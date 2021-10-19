/* $OpenBSD */

/*
 * Copyright (c) 2021 Mike Larkin <mlarkin@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <machine/bus.h>
#include <machine/vmmvar.h>
#include <sys/device.h>
#include <sys/types.h>
#include <uvm/uvm_extern.h>

#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>

const struct pci_matchid vmmfs_devices[] = {
	{ PCI_VENDOR_OPENBSD, PCI_PRODUCT_OPENBSD_VMMFS }
};

struct vmmfs_softc {
	struct device		sc_dev;

	paddr_t			sc_pa;
	vaddr_t			sc_va;

	bus_space_tag_t		sc_iot;
	bus_space_handle_t	sc_ioh;
	bus_size_t		sc_iosize;
};

struct vmmfs_softc *vmmfs_sc;

int	vmmfs_match(struct device *, void *, void *);
void	vmmfs_attach(struct device *, struct device *, void *);

struct cfattach vmmfs_ca = {
	sizeof(struct vmmfs_softc),
	vmmfs_match,
	vmmfs_attach,
	NULL
};

struct cfdriver vmmfs_cd = {
	NULL, "vmmfs", DV_DULL
};


int vmmfs_match(struct device *parent, void *match, void *aux)
{
	return (pci_matchbyid((struct pci_attach_args *)aux, vmmfs_devices,
	    sizeof(vmmfs_devices) / sizeof(vmmfs_devices[0])));
}

void
vmmfs_attach(struct device *parent, struct device *self, void *aux)
{
	struct vmmfs_softc *sc = (struct vmmfs_softc *)self;
	struct pci_attach_args *pa = (struct pci_attach_args *)aux;
	pcireg_t type;

	type = pci_mapreg_type(pa->pa_pc, pa->pa_tag, PCI_MAPREG_START);
	if (pci_mapreg_map(pa, PCI_MAPREG_START, type, 0,
	    &sc->sc_iot, &sc->sc_ioh, NULL, &sc->sc_iosize, 0)) {
		printf("%s: can't map i/o space\n", __func__);
		return;
	}

	sc->sc_va = (vaddr_t)km_alloc(PAGE_SIZE, &kv_page, &kp_zero,
	    &kd_waitok);
	sc->sc_pa = 0;

	if (!pmap_extract(pmap_kernel(), sc->sc_va, (paddr_t *)&sc->sc_pa)) {
		printf(" cannot allocate vmmfs buffer\n");
		return;
	}

	bus_space_write_4(sc->sc_iot, sc->sc_ioh, 0,
	    (uint32_t)(sc->sc_pa & 0x00000000FFFFFFFFULL));
	bus_space_write_4(sc->sc_iot, sc->sc_ioh, 4,
	    (uint32_t)(sc->sc_pa >> 32));

	printf(", %p", (void *)sc->sc_pa);

	vmmfs_sc = sc;

	printf("\n");
	return;
}

int
vmmfs_fsop_put(struct vm_fsop *op)
{
	uint64_t seq = op->seq;

	printf("%s: putting fsop opcode %d seq %lld\n", __func__, op->opcode,
	    op->seq);

	memcpy((void *)vmmfs_sc->sc_va, op, sizeof(struct vm_fsop));

	printf("%s: ringing doorbell for seq %lld\n", __func__, op->seq);
	bus_space_write_1(vmmfs_sc->sc_iot, vmmfs_sc->sc_ioh, 9, 0);
	memcpy(op, (void *)vmmfs_sc->sc_va, sizeof(struct vm_fsop));

	printf("%s: exiting, old seq = %lld new seq = %lld\n", __func__,
	    seq, op->seq);

	return 0;
}

int
vmmfs_fsop_get(struct vm_fsop *op)
{
	struct vm_fsop *cur = (struct vm_fsop *)vmmfs_sc->sc_va;

	printf("%s: checking for completion of seq %lld\n", __func__, op->seq);
	if (cur->seq == op->seq) {
		printf("%s: seq %lld not completed\n", __func__, op->seq);
		return EAGAIN;
	}

	printf("%s: seq %lld completed\n", __func__, op->seq);

	memcpy(op, cur, sizeof(struct vm_fsop));

	return 0;
}

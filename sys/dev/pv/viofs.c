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
#include <sys/timeout.h>
#include <machine/bus.h>
#include <sys/device.h>
#include <sys/pool.h>
#include <dev/pv/virtioreg.h>
#include <dev/pv/virtiovar.h>

#define VIOFS_DEBUG 0

struct viofs_softc {
	struct device		 sc_dev;
	struct virtio_softc	*sc_virtio;

	struct virtqueue         sc_hiprioq;
	struct virtqueue         sc_reqq;
};

int	viofs_match(struct device *, void *, void *);
void	viofs_attach(struct device *, struct device *, void *);
int	viofs_vq_done(struct virtqueue *);

struct cfattach viofs_ca = {
	sizeof(struct viofs_softc),
	viofs_match,
	viofs_attach,
	NULL
};

struct cfdriver viofs_cd = {
	NULL, "viofs", DV_DULL
};


int viofs_match(struct device *parent, void *match, void *aux)
{
	struct virtio_softc *va = aux;
	if (va->sc_childdevid == PCI_PRODUCT_VIRTIO_FS)
		return 1;
	return 0;
}

void
viofs_attach(struct device *parent, struct device *self, void *aux)
{
	struct viofs_softc *sc = (struct viofs_softc *)self;
	struct virtio_softc *vsc = (struct virtio_softc *)parent;

	vsc->sc_nvqs = 2;
	vsc->sc_config_change = 0;
	if (vsc->sc_child != NULL)
		panic("already attached to something else");
	vsc->sc_child = self;
	vsc->sc_ipl = IPL_BIO;
	sc->sc_virtio = vsc;

	virtio_negotiate_features(vsc, NULL);

/*
	sc->sc_buf = dma_alloc(VIORND_BUFSIZE, PR_NOWAIT|PR_ZERO);
	if (sc->sc_buf == NULL) {
		printf(": Can't alloc dma buffer\n");
		goto err;
	}
	if (bus_dmamap_create(sc->sc_virtio->sc_dmat, VIORND_BUFSIZE, 1,
	    VIORND_BUFSIZE, 0, BUS_DMA_NOWAIT|BUS_DMA_ALLOCNOW,
	    &sc->sc_dmamap)) {
		printf(": Can't alloc dmamap\n");
		goto err;
	}
	if (bus_dmamap_load(sc->sc_virtio->sc_dmat, sc->sc_dmamap,
	    sc->sc_buf, VIORND_BUFSIZE, NULL, BUS_DMA_NOWAIT|BUS_DMA_READ)) {
		printf(": Can't load dmamap\n");
		goto err2;
	}
*/
	if (virtio_alloc_vq(vsc, &sc->sc_hiprioq, 0, MAXPHYS, 1,
	    "hi prio") != 0) {
		printf(": Can't alloc hi prio virtqueue\n");
		goto err;
	}

	sc->sc_hiprioq.vq_done = viofs_vq_done;
	virtio_start_vq_intr(vsc, &sc->sc_hiprioq);

	if (virtio_alloc_vq(vsc, &sc->sc_reqq, 0, MAXPHYS, 1,
	    "request queue") != 0) {
		printf(": Can't alloc request virtqueue\n");
		goto err;
	}

	sc->sc_reqq.vq_done = viofs_vq_done;
	virtio_start_vq_intr(vsc, &sc->sc_reqq);

	printf("\n");
	return;
err:
	vsc->sc_child = VIRTIO_CHILD_ERROR;
/*
	if (sc->sc_buf != NULL) {
		dma_free(sc->sc_buf, VIORND_BUFSIZE);
		sc->sc_buf = NULL;
	}
*/
	return;
}

int
viofs_vq_done(struct virtqueue *vq)
{
	struct virtio_softc *vsc = vq->vq_owner;
//	struct viofs_softc *sc = (struct viofs_softc *)vsc->sc_child;
	int slot, len;

	if (virtio_dequeue(vsc, vq, &slot, &len) != 0)
		return 0;
/*
	bus_dmamap_sync(vsc->sc_dmat, sc->sc_dmamap, 0, VIORND_BUFSIZE,
	    BUS_DMASYNC_POSTREAD);
	if (len > VIORND_BUFSIZE) {
		printf("%s: inconsistent descriptor length %d > %d\n",
		    sc->sc_dev.dv_xname, len, VIORND_BUFSIZE);
		goto out;
	}
*/
	virtio_dequeue_commit(vq, slot);
	return 1;
}

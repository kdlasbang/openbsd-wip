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
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/fusebuf.h>
#include <dev/pv/virtioreg.h>
#include <dev/pv/virtiovar.h>

#define VIOFS_DEBUG 0

#define VIOFS_CONFIG_TAG 0

struct viofs_softc {
	struct device		 sc_dev;
	struct virtio_softc	*sc_virtio;

	struct virtqueue         sc_hiprioq;
	struct virtqueue         sc_reqq;
	bus_dmamap_t		 sc_dmamap;
	bus_dma_segment_t	 sc_hiprio_reqs;
	bus_dma_segment_t	 sc_req_reqs;

	struct fusebuf		*sc_fusebuf;

	char			 sc_tag[37];

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
	uint64_t tagb;

	vsc->sc_nvqs = 2;
	vsc->sc_config_change = 0;
	if (vsc->sc_child != NULL)
		panic("already attached to something else");
	vsc->sc_child = self;
	vsc->sc_ipl = IPL_BIO;
	sc->sc_virtio = vsc;

	virtio_negotiate_features(vsc, NULL);

	sc->sc_fusebuf = dma_alloc(MAXPHYS, PR_NOWAIT|PR_ZERO);
	if (sc->sc_fusebuf == NULL) {
		printf(": Can't alloc dma buffer\n");
		goto err;
	}
	if (bus_dmamap_create(sc->sc_virtio->sc_dmat, MAXPHYS, 1,
	    MAXPHYS, 0, BUS_DMA_NOWAIT|BUS_DMA_ALLOCNOW,
	    &sc->sc_dmamap)) {
		printf(": Can't alloc dmamap\n");
		goto err;
	}
	if (bus_dmamap_load(sc->sc_virtio->sc_dmat, sc->sc_dmamap,
	    sc->sc_fusebuf, MAXPHYS, NULL, BUS_DMA_NOWAIT|BUS_DMA_READ)) {
		printf(": Can't load dmamap\n");
		goto err;
	}

	if (virtio_alloc_vq(vsc, &sc->sc_hiprioq, 0, MAXPHYS, 1,
	    "hi prio") != 0) {
		printf(": Can't alloc hi prio virtqueue\n");
		goto err;
	}

	sc->sc_hiprioq.vq_done = viofs_vq_done;
	virtio_start_vq_intr(vsc, &sc->sc_hiprioq);

	if (virtio_alloc_vq(vsc, &sc->sc_reqq, 1, MAXPHYS, 1,
	    "request queue") != 0) {
		printf(": Can't alloc request virtqueue\n");
		goto err;
	}

	sc->sc_reqq.vq_done = viofs_vq_done;
	virtio_start_vq_intr(vsc, &sc->sc_reqq);


	memset(&sc->sc_tag, 0, 36);
	tagb = virtio_read_device_config_8(vsc, VIOFS_CONFIG_TAG);
	memcpy(&sc->sc_tag[0], &tagb, 8);
	tagb = virtio_read_device_config_8(vsc, VIOFS_CONFIG_TAG + 8);
	memcpy(&sc->sc_tag[8], &tagb, 8);
	tagb = virtio_read_device_config_8(vsc, VIOFS_CONFIG_TAG + 16);
	memcpy(&sc->sc_tag[16], &tagb, 8);
	tagb = virtio_read_device_config_8(vsc, VIOFS_CONFIG_TAG + 24);
	memcpy(&sc->sc_tag[24], &tagb, 8);
	tagb = virtio_read_device_config_4(vsc, VIOFS_CONFIG_TAG + 32);
	memcpy(&sc->sc_tag[32], &tagb, 4);

	sc->sc_tag[36] = 0;

	printf(": host tag %s\n", sc->sc_tag);
#if 0
	if (viofs_alloc_reqs(sc)) {
		printf(": Can't alloc request buffers\n");
		goto err;
	}
#endif

	printf("\n");
	return;
err:
	vsc->sc_child = VIRTIO_CHILD_ERROR;

	if (sc->sc_fusebuf != NULL) {
		dma_free(sc->sc_fusebuf, MAXPHYS);
		sc->sc_fusebuf = NULL;
	}

	return;
}

int
viofs_vq_done(struct virtqueue *vq)
{
	struct virtio_softc *vsc = vq->vq_owner;
	struct viofs_softc *sc = (struct viofs_softc *)vsc->sc_child;
	int slot, len;

	if (virtio_dequeue(vsc, vq, &slot, &len) != 0)
		return 0;

	bus_dmamap_sync(vsc->sc_dmat, sc->sc_dmamap, 0, MAXPHYS,
	    BUS_DMASYNC_POSTREAD);
	if (len > MAXPHYS) {
		printf("%s: invalid descriptor length %d > %d\n",
		    sc->sc_dev.dv_xname, len, MAXPHYS);
		goto out;
	}
out:
	virtio_dequeue_commit(vq, slot);
	return 1;
}

/*
int
viofs_alloc_reqs(struct viofs_softc *sc)
{
	int qsize, allocsize, r;

	qsize = sc->sc_hiprioq.vq_num;

	allocsize = FUSEBUFMAXSIZE * qsize;
	printf("%s: allocating %d hiprioq bytes\n", __func__);

	r = bus_dmamem_alloc(sc->sc_virtio->sc_dmat, allocsize, 0, 0,
	    &sc->sc_hiprioq_segs, 1, &rsegs, BUS_DMA_NOWAIT);
	if (r != 0) {
		printf("DMA memory allocation failed, size %d, error %d\n",
		    allocsize, r);
		goto err_none;
	}

	r = bus_dmamem_map(sc->sc_virtio->sc_dmat, &sc->sc_hiprioq_segs, 1,
	    allocsize, (caddr_t *)&vaddr, BUS_DMA_NOWAIT);
	if (r != 0) {
		printf("DMA memory map failed, error %d\n", r);
		goto err_dmamem_alloc;
	}
*/

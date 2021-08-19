/* $OpenBSD */
/*
 * Copyright (c) 2014 Mike Larkin <mlarkin@openbsd.org>
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
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/vmm.h>

/* #define VMM_DEBUG */

#ifdef VMM_DEBUG
#define DPRINTF(x...)	do { printf(x); } while(0)
#else
#define DPRINTF(x...)
#endif /* VMM_DEBUG */

int vmmioctl(dev_t, u_long, caddr_t, int, struct proc *);

/*
 * vmmioctl
 *
 * Main ioctl dispatch routine for /dev/vmm. Parses ioctl type and calls
 * appropriate lower level handler routine. Returns result to ioctl caller.
 */
int
vmmioctl(dev_t dev, u_long cmd, caddr_t data, int flag, struct proc *p)
{
	int ret;

	switch (cmd) {
	case VMM_IOC_CREATE:
		if ((ret = vmm_start()) != 0) {
			vmm_stop();
			break;
		}
		ret = vm_create((struct vm_create_params *)data, p);
		break;
	case VMM_IOC_INFO:
		ret = vm_get_info((struct vm_info_params *)data);
		break;
	case VMM_IOC_TERM:
		ret = vm_terminate((struct vm_terminate_params *)data);
		break;
	case VMM_IOC_INTR:
		ret = vm_intr_pending((struct vm_intr_params *)data);
		break;
	default:
		DPRINTF("%s: unknown ioctl code 0x%lx\n", __func__, cmd);
		ret = ENOTTY;
	}

	return (ret);
}

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

#ifndef _SYS_VMM_H_
#define _SYS_VMM_H_

#include <sys/proc.h>
#include <sys/types.h>

#define VMM_HV_SIGNATURE 	"OpenBSDVMM58"

#define VMM_MAX_MEM_RANGES	16
#define VMM_MAX_DISKS_PER_VM	4
#define VMM_MAX_PATH_DISK	128
#define VMM_MAX_PATH_CDROM	128
#define VMM_MAX_NAME_LEN	64
#define VMM_MAX_KERNEL_PATH	128
#define VMM_MAX_VCPUS_PER_VM	64
#define VMM_MAX_VM_MEM_SIZE	32768
#define VMM_MAX_NICS_PER_VM	4

struct vm_mem_range {
	paddr_t	vmr_gpa;
	vaddr_t vmr_va;
	size_t	vmr_size;
};

struct vm_create_params {
	/* Input parameters to VMM_IOC_CREATE */
	size_t			vcp_nmemranges;
	size_t			vcp_ncpus;
	size_t			vcp_ndisks;
	size_t			vcp_nnics;
	struct vm_mem_range	vcp_memranges[VMM_MAX_MEM_RANGES];
	char			vcp_disks[VMM_MAX_DISKS_PER_VM][VMM_MAX_PATH_DISK];
	char			vcp_cdrom[VMM_MAX_PATH_CDROM];
	char			vcp_name[VMM_MAX_NAME_LEN];
	char			vcp_kernel[VMM_MAX_KERNEL_PATH];
	uint8_t			vcp_macs[VMM_MAX_NICS_PER_VM][6];

	/* Output parameter from VMM_IOC_CREATE */
	uint32_t	vcp_id;
};

struct vm_info_result {
	/* Output parameters from VMM_IOC_INFO */
	size_t		vir_memory_size;
	size_t		vir_used_size;
	size_t		vir_ncpus;
	uint8_t		vir_vcpu_state[VMM_MAX_VCPUS_PER_VM];
	pid_t		vir_creator_pid;
	uint32_t	vir_id;
	char		vir_name[VMM_MAX_NAME_LEN];
};

struct vm_info_params {
	/* Input parameters to VMM_IOC_INFO */
	size_t			vip_size;	/* Output buffer size */

	/* Output Parameters from VMM_IOC_INFO */
	size_t			 vip_info_ct;	/* # of entries returned */
	struct vm_info_result	*vip_info;	/* Output buffer */
};

struct vm_terminate_params {
	/* Input parameters to VMM_IOC_TERM */
	uint32_t		vtp_vm_id;
};

struct vm_intr_params {
	/* Input parameters to VMM_IOC_INTR */
	uint32_t		vip_vm_id;
	uint32_t		vip_vcpu_id;
	uint16_t		vip_intr;
};

int vmm_start(void);
int vmm_stop(void);
int vm_create(struct vm_create_params *, struct proc *);
int vm_get_info(struct vm_info_params *);
int vm_terminate(struct vm_terminate_params *);
int vm_intr_pending(struct vm_intr_params *);

/* IOCTL definitions */
#define VMM_IOC_CREATE _IOWR('V', 1, struct vm_create_params) /* Create VM */
#define VMM_IOC_INFO _IOWR('V', 3, struct vm_info_params) /* Get VM Info */
#define VMM_IOC_TERM _IOW('V', 4, struct vm_terminate_params) /* Terminate VM */
#define VMM_IOC_INTR _IOW('V', 6, struct vm_intr_params) /* Intr pending */

#endif /* _SYS_VMM_H_ */

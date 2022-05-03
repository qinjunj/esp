// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef _ROTATEORDER2_VIVADO_H_
#define _ROTATEORDER2_VIVADO_H_

#ifdef __KERNEL__
#include <linux/ioctl.h>
#include <linux/types.h>
#else
#include <sys/ioctl.h>
#include <stdint.h>
#ifndef __user
#define __user
#endif
#endif /* __KERNEL__ */

#include <esp.h>
#include <esp_accelerator.h>

struct rotateorder2_vivado_access {
	struct esp_access esp;
	/* <<--regs-->> */
	unsigned nBatches;
	unsigned channels;
	unsigned nSamples;
	unsigned src_offset;
	unsigned dst_offset;
};

#define ROTATEORDER2_VIVADO_IOC_ACCESS	_IOW ('S', 0, struct rotateorder2_vivado_access)

#endif /* _ROTATEORDER2_VIVADO_H_ */

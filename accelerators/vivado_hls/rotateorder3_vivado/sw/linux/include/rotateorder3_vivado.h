// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef _ROTATEORDER3_VIVADO_H_
#define _ROTATEORDER3_VIVADO_H_

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

struct rotateorder3_vivado_access {
	struct esp_access esp;
	/* <<--regs-->> */
	unsigned nBatches;
	float cosA1;
	float cosA3;
	float cosA2;
	unsigned nChannels;
	float sinB3;
	float sinB2;
	float sinB1;
	float sinA2;
	float sinA3;
	float sinA1;
	float cosG3;
	float cosG2;
	float cosG1;
	unsigned nSamples;
	float sinG1;
	float sinG2;
	float sinG3;
	float cosB1;
	float cosB2;
	float cosB3;
	unsigned src_offset;
	unsigned dst_offset;
};

#define ROTATEORDER3_VIVADO_IOC_ACCESS	_IOW ('S', 0, struct rotateorder3_vivado_access)

#endif /* _ROTATEORDER3_VIVADO_H_ */

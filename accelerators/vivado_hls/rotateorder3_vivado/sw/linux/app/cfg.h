// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"
#include "rotateorder3_vivado.h"

typedef int32_t token_t;

/* <<--params-def-->> */
#define NBATCHES 1
#define COSA1 -4.37114e-08
#define COSA3 1.19249e-08
#define COSA2 -1
#define NCHANNELS 7
#define SINB3 -0.00318557
#define SINB2 -0.864962
#define SINB1 0.866556
#define SINA2 8.74228e-08
#define SINA3 1
#define SINA1 -1
#define COSG3 1.19249e-08
#define COSG2 -1
#define COSG1 -4.37114e-08
#define NSAMPLES 1024
#define SING1 1
#define SING2 -8.74228e-08
#define SING3 -1
#define COSB1 -0.49908
#define COSB2 -0.501838
#define COSB3 0.999995

/* <<--params-->> */
const int32_t nBatches = NBATCHES;
const int32_t cosA1 = COSA1;
const int32_t cosA3 = COSA3;
const int32_t cosA2 = COSA2;
const int32_t nChannels = NCHANNELS;
const int32_t sinB3 = SINB3;
const int32_t sinB2 = SINB2;
const int32_t sinB1 = SINB1;
const int32_t sinA2 = SINA2;
const int32_t sinA3 = SINA3;
const int32_t sinA1 = SINA1;
const int32_t cosG3 = COSG3;
const int32_t cosG2 = COSG2;
const int32_t cosG1 = COSG1;
const int32_t nSamples = NSAMPLES;
const int32_t sinG1 = SING1;
const int32_t sinG2 = SING2;
const int32_t sinG3 = SING3;
const int32_t cosB1 = COSB1;
const int32_t cosB2 = COSB2;
const int32_t cosB3 = COSB3;

#define NACC 1

struct rotateorder3_vivado_access rotateorder3_cfg_000[] = {
	{
		/* <<--descriptor-->> */
		.nBatches = NBATCHES,
		.cosA1 = COSA1,
		.cosA3 = COSA3,
		.cosA2 = COSA2,
		.nChannels = NCHANNELS,
		.sinB3 = SINB3,
		.sinB2 = SINB2,
		.sinB1 = SINB1,
		.sinA2 = SINA2,
		.sinA3 = SINA3,
		.sinA1 = SINA1,
		.cosG3 = COSG3,
		.cosG2 = COSG2,
		.cosG1 = COSG1,
		.nSamples = NSAMPLES,
		.sinG1 = SING1,
		.sinG2 = SING2,
		.sinG3 = SING3,
		.cosB1 = COSB1,
		.cosB2 = COSB2,
		.cosB3 = COSB3,
		.src_offset = 0,
		.dst_offset = 0,
		.esp.coherence = ACC_COH_NONE,
		.esp.p2p_store = 0,
		.esp.p2p_nsrcs = 0,
		.esp.p2p_srcs = {"", "", "", ""},
	}
};

esp_thread_info_t cfg_000[] = {
	{
		.run = true,
		.devname = "rotateorder3_vivado.0",
		.ioctl_req = ROTATEORDER3_VIVADO_IOC_ACCESS,
		.esp_desc = &(rotateorder3_cfg_000[0].esp),
	}
};

#endif /* __ESP_CFG_000_H__ */

// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"
#include "rotateorder2_vivado.h"

typedef int32_t token_t;

/* <<--params-def-->> */
#define NBATCHES 1
#define CHANNELS 5
#define NSAMPLES 1024

/* <<--params-->> */
const int32_t nBatches = NBATCHES;
const int32_t channels = CHANNELS;
const int32_t nSamples = NSAMPLES;

#define NACC 1

struct rotateorder2_vivado_access rotateorder2_cfg_000[] = {
	{
		/* <<--descriptor-->> */
		.nBatches = NBATCHES,
		.channels = CHANNELS,
		.nSamples = NSAMPLES,
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
		.devname = "rotateorder2_vivado.0",
		.ioctl_req = ROTATEORDER2_VIVADO_IOC_ACCESS,
		.esp_desc = &(rotateorder2_cfg_000[0].esp),
	}
};

#endif /* __ESP_CFG_000_H__ */

/* Copyright (c) 2011-2022 Columbia University, System Level Design Group */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdio.h>
#ifndef __riscv
#include <stdlib.h>
#endif

#include <esp_accelerator.h>
#include <esp_probe.h>
#include <fixed_point.h>

typedef int32_t token_t;

static unsigned DMA_WORD_PER_BEAT(unsigned _st)
{
        return (sizeof(void *) / _st);
}


#define SLD_ROTATEORDER3 0x060
#define DEV_NAME "sld,rotateorder3_vivado"

/* <<--params-->> */
const int32_t nBatches = 1;
const int32_t cosA1 = -4.37114e-08;
const int32_t cosA3 = 1.19249e-08;
const int32_t cosA2 = -1;
const int32_t nChannels = 7;
const int32_t sinB3 = -0.00318557;
const int32_t sinB2 = -0.864962;
const int32_t sinB1 = 0.866556;
const int32_t sinA2 = 8.74228e-08;
const int32_t sinA3 = 1;
const int32_t sinA1 = -1;
const int32_t cosG3 = 1.19249e-08;
const int32_t cosG2 = -1;
const int32_t cosG1 = -4.37114e-08;
const int32_t nSamples = 1024;
const int32_t sinG1 = 1;
const int32_t sinG2 = -8.74228e-08;
const int32_t sinG3 = -1;
const int32_t cosB1 = -0.49908;
const int32_t cosB2 = -0.501838;
const int32_t cosB3 = 0.999995;

static unsigned in_words_adj;
static unsigned out_words_adj;
static unsigned in_len;
static unsigned out_len;
static unsigned in_size;
static unsigned out_size;
static unsigned out_offset;
static unsigned mem_size;

/* Size of the contiguous chunks for scatter/gather */
#define CHUNK_SHIFT 20
#define CHUNK_SIZE BIT(CHUNK_SHIFT)
#define NCHUNK(_sz) ((_sz % CHUNK_SIZE == 0) ?		\
			(_sz / CHUNK_SIZE) :		\
			(_sz / CHUNK_SIZE) + 1)

/* User defined registers */
/* <<--regs-->> */
#define ROTATEORDER3_NBATCHES_REG 0x90
#define ROTATEORDER3_COSA1_REG 0x8c
#define ROTATEORDER3_COSA3_REG 0x88
#define ROTATEORDER3_COSA2_REG 0x84
#define ROTATEORDER3_NCHANNELS_REG 0x80
#define ROTATEORDER3_SINB3_REG 0x7c
#define ROTATEORDER3_SINB2_REG 0x78
#define ROTATEORDER3_SINB1_REG 0x74
#define ROTATEORDER3_SINA2_REG 0x70
#define ROTATEORDER3_SINA3_REG 0x6c
#define ROTATEORDER3_SINA1_REG 0x68
#define ROTATEORDER3_COSG3_REG 0x64
#define ROTATEORDER3_COSG2_REG 0x60
#define ROTATEORDER3_COSG1_REG 0x5c
#define ROTATEORDER3_NSAMPLES_REG 0x58
#define ROTATEORDER3_SING1_REG 0x54
#define ROTATEORDER3_SING2_REG 0x50
#define ROTATEORDER3_SING3_REG 0x4c
#define ROTATEORDER3_COSB1_REG 0x48
#define ROTATEORDER3_COSB2_REG 0x44
#define ROTATEORDER3_COSB3_REG 0x40


static int validate_buf(token_t *out, token_t *gold)
{
	int i;
	int j;
	unsigned errors = 0;

	for (i = 0; i < nBatches; i++)
		for (j = 0; j < nChannels*nSamples; j++)
			if (gold[i * out_words_adj + j] != out[i * out_words_adj + j])
				errors++;

	return errors;
}


static void init_buf (token_t *in, token_t * gold)
{
	int i;
	int j;

	for (i = 0; i < nBatches; i++)
		for (j = 0; j < nChannels*nSamples; j++)
			in[i * in_words_adj + j] = (token_t) j;

	for (i = 0; i < nBatches; i++)
		for (j = 0; j < nChannels*nSamples; j++)
			gold[i * out_words_adj + j] = (token_t) j;
}


int main(int argc, char * argv[])
{
	int i;
	int n;
	int ndev;
	struct esp_device *espdevs;
	struct esp_device *dev;
	unsigned done;
	unsigned **ptable;
	token_t *mem;
	token_t *gold;
	unsigned errors = 0;
	unsigned coherence;

	if (DMA_WORD_PER_BEAT(sizeof(token_t)) == 0) {
		in_words_adj = nChannels*nSamples;
		out_words_adj = nChannels*nSamples;
	} else {
		in_words_adj = round_up(nChannels*nSamples, DMA_WORD_PER_BEAT(sizeof(token_t)));
		out_words_adj = round_up(nChannels*nSamples, DMA_WORD_PER_BEAT(sizeof(token_t)));
	}
	in_len = in_words_adj * (nBatches);
	out_len = out_words_adj * (nBatches);
	in_size = in_len * sizeof(token_t);
	out_size = out_len * sizeof(token_t);
	out_offset  = 0;
	mem_size = (out_offset * sizeof(token_t)) + out_size;


	// Search for the device
	printf("Scanning device tree... \n");

	ndev = probe(&espdevs, VENDOR_SLD, SLD_ROTATEORDER3, DEV_NAME);
	if (ndev == 0) {
		printf("rotateorder3 not found\n");
		return 0;
	}

	for (n = 0; n < ndev; n++) {

		printf("**************** %s.%d ****************\n", DEV_NAME, n);

		dev = &espdevs[n];

		// Check DMA capabilities
		if (ioread32(dev, PT_NCHUNK_MAX_REG) == 0) {
			printf("  -> scatter-gather DMA is disabled. Abort.\n");
			return 0;
		}

		if (ioread32(dev, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size)) {
			printf("  -> Not enough TLB entries available. Abort.\n");
			return 0;
		}

		// Allocate memory
		gold = aligned_malloc(out_size);
		mem = aligned_malloc(mem_size);
		printf("  memory buffer base-address = %p\n", mem);

		// Alocate and populate page table
		ptable = aligned_malloc(NCHUNK(mem_size) * sizeof(unsigned *));
		for (i = 0; i < NCHUNK(mem_size); i++)
			ptable[i] = (unsigned *) &mem[i * (CHUNK_SIZE / sizeof(token_t))];

		printf("  ptable = %p\n", ptable);
		printf("  nchunk = %lu\n", NCHUNK(mem_size));

#ifndef __riscv
		for (coherence = ACC_COH_NONE; coherence <= ACC_COH_RECALL; coherence++) {
#else
		{
			/* TODO: Restore full test once ESP caches are integrated */
			coherence = ACC_COH_NONE;
#endif
			printf("  --------------------\n");
			printf("  Generate input...\n");
			init_buf(mem, gold);

			// Pass common configuration parameters

			iowrite32(dev, SELECT_REG, ioread32(dev, DEVID_REG));
			iowrite32(dev, COHERENCE_REG, coherence);

#ifndef __sparc
			iowrite32(dev, PT_ADDRESS_REG, (unsigned long long) ptable);
#else
			iowrite32(dev, PT_ADDRESS_REG, (unsigned) ptable);
#endif
			iowrite32(dev, PT_NCHUNK_REG, NCHUNK(mem_size));
			iowrite32(dev, PT_SHIFT_REG, CHUNK_SHIFT);

			// Use the following if input and output data are not allocated at the default offsets
			iowrite32(dev, SRC_OFFSET_REG, 0x0);
			iowrite32(dev, DST_OFFSET_REG, 0x0);

			// Pass accelerator-specific configuration parameters
			/* <<--regs-config-->> */
		iowrite32(dev, ROTATEORDER3_NBATCHES_REG, nBatches);
		iowrite32(dev, ROTATEORDER3_COSA1_REG, cosA1);
		iowrite32(dev, ROTATEORDER3_COSA3_REG, cosA3);
		iowrite32(dev, ROTATEORDER3_COSA2_REG, cosA2);
		iowrite32(dev, ROTATEORDER3_NCHANNELS_REG, nChannels);
		iowrite32(dev, ROTATEORDER3_SINB3_REG, sinB3);
		iowrite32(dev, ROTATEORDER3_SINB2_REG, sinB2);
		iowrite32(dev, ROTATEORDER3_SINB1_REG, sinB1);
		iowrite32(dev, ROTATEORDER3_SINA2_REG, sinA2);
		iowrite32(dev, ROTATEORDER3_SINA3_REG, sinA3);
		iowrite32(dev, ROTATEORDER3_SINA1_REG, sinA1);
		iowrite32(dev, ROTATEORDER3_COSG3_REG, cosG3);
		iowrite32(dev, ROTATEORDER3_COSG2_REG, cosG2);
		iowrite32(dev, ROTATEORDER3_COSG1_REG, cosG1);
		iowrite32(dev, ROTATEORDER3_NSAMPLES_REG, nSamples);
		iowrite32(dev, ROTATEORDER3_SING1_REG, sinG1);
		iowrite32(dev, ROTATEORDER3_SING2_REG, sinG2);
		iowrite32(dev, ROTATEORDER3_SING3_REG, sinG3);
		iowrite32(dev, ROTATEORDER3_COSB1_REG, cosB1);
		iowrite32(dev, ROTATEORDER3_COSB2_REG, cosB2);
		iowrite32(dev, ROTATEORDER3_COSB3_REG, cosB3);

			// Flush (customize coherence model here)
			esp_flush(coherence);

			// Start accelerators
			printf("  Start...\n");
			iowrite32(dev, CMD_REG, CMD_MASK_START);

			// Wait for completion
			done = 0;
			while (!done) {
				done = ioread32(dev, STATUS_REG);
				done &= STATUS_MASK_DONE;
			}
			iowrite32(dev, CMD_REG, 0x0);

			printf("  Done\n");
			printf("  validating...\n");

			/* Validation */
			errors = validate_buf(&mem[out_offset], gold);
			if (errors)
				printf("  ... FAIL\n");
			else
				printf("  ... PASS\n");
		}
		aligned_free(ptable);
		aligned_free(mem);
		aligned_free(gold);
	}

	return 0;
}

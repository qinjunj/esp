/* Copyright (c) 2011-2022 Columbia University, System Level Design Group */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdio.h>
#ifndef __riscv
#include <stdlib.h>
#endif

#include <esp_accelerator.h>
#include <esp_probe.h>
#include <fixed_point.h>
#include <math.h>

typedef int32_t token_t;

static unsigned DMA_WORD_PER_BEAT(unsigned _st)
{
        return (sizeof(void *) / _st);
}


static inline uint64_t get_counter()
{
	uint64_t counter;
	asm volatile (
		"li t0, 0;"
		// "rdcycle t0;"
		"csrr t0, mcycle;"
		"mv %0, t0"
		: "=r" ( counter )
		:
		: "t0"
	);

	// printf("counter: %x\n", counter);

	return counter;
}

static float my_abs(float a){
	if(a > 0)
		return a;
	else
		return -a;
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

uint64_t start_cnt_pc;
uint64_t end_cnt_pc;

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

	for (i = 0; i < nChannels; i++) {
        for (j = 0; j < nSamples; j++) {
            in[nSamples*i+j] = (token_t) (-1.0 + j/(float)(nSamples/2)); // random between -1 and 1 to mimic sin cos
        }
    }

	const unsigned kQ = 0, kO = 1, kM = 2, kK = 3, kL = 4, kN = 5, kP = 6;
	const float fSqrt3_2 = sqrt(3.f/2.f);
    const float fSqrt15 = sqrt(15.f);
    const float fSqrt5_2 = sqrt(5.f/2.f);

	float tmp[nChannels];
	float tmp2[nChannels];

	start_cnt_pc = get_counter(); 
	for (int niSample = 0; niSample < nSamples; niSample++) {
		// Alpha rotation
		tmp[kQ] = - in[niSample+nSamples*kP] * sinA3
							+ in[niSample+nSamples*kQ] * cosA3;
		tmp[kO] = - in[niSample+nSamples*kN] * sinA2
							+ in[niSample+nSamples*kO] * cosA2;
		tmp[kM] = - in[niSample+nSamples*kL] * sinA1
							+ in[niSample+nSamples*kM] * cosA1;
		tmp[kK] = in[niSample+nSamples*kK];
		tmp[kL] = in[niSample+nSamples*kL] * cosA1
							+ in[niSample+nSamples*kM] * sinA1;
		tmp[kN] = in[niSample+nSamples*kN] * cosA2
							+ in[niSample+nSamples*kO] * sinA2;
		tmp[kP] = in[niSample+nSamples*kP] * cosA3
							+ in[niSample+nSamples*kQ] * sinA3;

		// Beta rotation
		tmp2[kQ] = 0.125f * tmp[kQ] * (5.f + 3.f*cosB2)
					- fSqrt3_2 * tmp[kO] *cosB1 * sinB1
					+ 0.25f * fSqrt15 * tmp[kM] * pow(sinB1,2.0f);
		tmp2[kO] = tmp[kO] * cosB2
					- fSqrt5_2 * tmp[kM] * cosB1 * sinB1
					+ fSqrt3_2 * tmp[kQ] * cosB1 * sinB1;
		tmp2[kM] = 0.125f * tmp[kM] * (3.f + 5.f*cosB2)
					- fSqrt5_2 * tmp[kO] *cosB1 * sinB1
					+ 0.25f * fSqrt15 * tmp[kQ] * pow(sinB1,2.0f);
		tmp2[kK] = 0.25f * tmp[kK] * cosB1* (-1.f + 15.f*cosB2)
					+ 0.5f * fSqrt15 * tmp[kN] * cosB1 * pow(sinB1,2.f)
					+ 0.5f * fSqrt5_2 * tmp[kP] * pow(sinB1,3.f)
					+ 0.125f * fSqrt3_2 * tmp[kL] * (sinB1 + 5.f * sinB3);
		tmp2[kL] = 0.0625f * tmp[kL] * (cosB1 + 15.f * cosB3)
					+ 0.25f * fSqrt5_2 * tmp[kN] * (1.f + 3.f * cosB2) * sinB1
					+ 0.25f * fSqrt15 * tmp[kP] * cosB1 * pow(sinB1,2.f)
					- 0.125 * fSqrt3_2 * tmp[kK] * (sinB1 + 5.f * sinB3);
		tmp2[kN] = 0.125f * tmp[kN] * (5.f * cosB1 + 3.f * cosB3)
					+ 0.25f * fSqrt3_2 * tmp[kP] * (3.f + cosB2) * sinB1
					+ 0.5f * fSqrt15 * tmp[kK] * cosB1 * pow(sinB1,2.f)
					+ 0.125 * fSqrt5_2 * tmp[kL] * (sinB1 - 3.f * sinB3);
		tmp2[kP] = 0.0625f * tmp[kP] * (15.f * cosB1 + cosB3)
					- 0.25f * fSqrt3_2 * tmp[kN] * (3.f + cosB2) * sinB1
					+ 0.25f * fSqrt15 * tmp[kL] * cosB1 * pow(sinB1,2.f)
					- 0.5 * fSqrt5_2 * tmp[kK] * pow(sinB1,3.f);

		// Gamma rotation
		gold[kQ * nSamples + niSample] = - tmp2[kP] * sinG3
							+ tmp2[kQ] * cosG3;
		gold[kO* nSamples + niSample] = - tmp2[kN] * sinG2
							+ tmp2[kO] * cosG2;
		gold[kM* nSamples + niSample] = - tmp2[kL] * sinG1
							+ tmp2[kM] * cosG1;
		gold[kK* nSamples + niSample] = tmp2[kK];
		gold[kL* nSamples + niSample] = tmp2[kL] * cosG1
							+ tmp2[kM] * sinG1;
		gold[kN* nSamples + niSample] = tmp2[kN] * cosG2
							+ tmp2[kO] * sinG2;
		gold[kP* nSamples + niSample] = tmp2[kP] * cosG3
							+ tmp2[kQ] * sinG3;

    }
	

	// for (i = 0; i < nBatches; i++)
	// 	for (j = 0; j < nChannels*nSamples; j++)
	// 		in[i * in_words_adj + j] = (token_t) j;

	// for (i = 0; i < nBatches; i++)
	// 	for (j = 0; j < nChannels*nSamples; j++)
	// 		gold[i * out_words_adj + j] = (token_t) j;
	
	end_cnt_pc = get_counter(); 
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

	uint64_t start_cnt;
	uint64_t coh_cnt;
	uint64_t end_cnt;

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
	out_offset  = in_len;
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
			coherence = ACC_COH_NONE; // {ACC_COH_NONE = 0, ACC_COH_LLC, ACC_COH_RECALL, ACC_COH_FULL};
#endif
			//printf("  --------------------\n");
			//printf("  Generate input...\n");
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

			start_cnt = get_counter(); 

			// Flush (customize coherence model here)
			esp_flush(coherence);

			coh_cnt = get_counter(); 

			// Start accelerators
			//printf("  Start...\n");
			iowrite32(dev, CMD_REG, CMD_MASK_START);

			// Wait for completion
			done = 0;
			while (!done) {
				done = ioread32(dev, STATUS_REG);
				done &= STATUS_MASK_DONE;
			}
			iowrite32(dev, CMD_REG, 0x0);

			end_cnt = get_counter(); 

			//printf("  Done\n");
			//printf("  validating...\n");

			/* Validation */
			errors = validate_buf(&mem[out_offset], gold);
			if (errors)
				printf("  ... FAIL\n");
			else
				printf("  ... PASS\n");
		}

		printf("acc counter flush: %lu\n", coh_cnt-start_cnt);
		printf("acc counter: %lu\n", end_cnt-coh_cnt);
		printf("cpu  counter: %lu\n", end_cnt_pc-start_cnt_pc);

		aligned_free(ptable);
		aligned_free(mem);
		aligned_free(gold);
	}

	return 0;
}

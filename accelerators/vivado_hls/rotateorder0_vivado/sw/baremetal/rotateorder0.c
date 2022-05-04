/* Copyright (c) 2011-2022 Columbia University, System Level Design Group */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdio.h>
#include <unistd.h>
#include <math.h>
#ifndef __riscv
#include <stdlib.h>
#endif

#include <esp_accelerator.h>
#include <esp_probe.h>
#include <fixed_point.h>

typedef float token_t;

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

#define SLD_ROTATEORDER0 0x066
#define DEV_NAME "sld,rotateorder0_vivado"

/* <<--params-->> */
const int32_t nBatches = 1;
const int32_t nChannels = 3;
const int32_t nSamples = 1024;

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
#define ROTATEORDER0_NBATCHES_REG 0x48
#define ROTATEORDER0_NCHANNELS_REG 0x44
#define ROTATEORDER0_NSAMPLES_REG 0x40


static int validate_buf(token_t *out, token_t *gold)
{
	int i;
	int j;
	unsigned errors = 0;
	float epsilon = 0.00001f;
	for (i = 0; i < nBatches; i++)
		for (j = 0; j < nChannels * nSamples; j++) {
			float diff = fabs((float)gold[i * out_words_adj + j] - (float)out[i * out_words_adj + j]);
			if (diff > epsilon){
				errors++;
			}
		}

    printf("errors in baremetal: %d\n", errors);

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

    const float m_fSinGammaf = 1;
    const float m_fCosBetaf = -0.49908;
    const float m_fCosAlphaf = -4.37114e-08;
    const float m_fSinAlphaf = -1;
    const float m_fSinBetaf = 0.866556;
    const float m_fCosGammaf = -4.37114e-08;
    
    // const int kY = 0;
    // const int kZ = 1;
    // const int kX = 2;

    float m_pfTempSample[3] = {0};
	float m_pfTempSample2[3] = {0};

    start_cnt_pc = get_counter();
    for (int niSample = 0; niSample < nSamples; niSample++) {
        // Alpha rotation
        m_pfTempSample[0] = -in[2*nSamples+niSample] * m_fSinAlphaf
                            + in[0*nSamples+niSample] * m_fCosAlphaf;
        m_pfTempSample[1] = in[1*nSamples+niSample];
        m_pfTempSample[2] = in[2*nSamples+niSample] * m_fCosAlphaf
                            + in[0*nSamples+niSample] * m_fSinAlphaf;

        // Beta rotation
        m_pfTempSample2[0] = m_pfTempSample[0];
        m_pfTempSample2[1] = m_pfTempSample[1] * m_fCosBetaf
                            +  m_pfTempSample[2] * m_fSinBetaf;
        m_pfTempSample2[2] = m_pfTempSample[2] * m_fCosBetaf
                            - m_pfTempSample[1] * m_fSinBetaf;

        // Gamma rotation
        m_pfTempSample[0] = -m_pfTempSample2[2] * m_fSinGammaf
                            + m_pfTempSample2[0] * m_fCosGammaf;
        m_pfTempSample[1] = m_pfTempSample2[1];
        m_pfTempSample[2] = m_pfTempSample2[2] * m_fCosGammaf
                            + m_pfTempSample2[0] * m_fSinGammaf;

        gold[2*nSamples+niSample] = m_pfTempSample[2];
        gold[0*nSamples+niSample] = m_pfTempSample[0];
        gold[1*nSamples+niSample] = m_pfTempSample[1];
    }
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
	out_offset  = 0;
	mem_size = (out_offset * sizeof(token_t)) + out_size;


	// Search for the device
	printf("Scanning device tree... \n");

	ndev = probe(&espdevs, VENDOR_SLD, SLD_ROTATEORDER0, DEV_NAME);
	if (ndev == 0) {
		printf("rotateorder0 not found\n");
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
		iowrite32(dev, ROTATEORDER0_NBATCHES_REG, nBatches);
		iowrite32(dev, ROTATEORDER0_NCHANNELS_REG, nChannels);
		iowrite32(dev, ROTATEORDER0_NSAMPLES_REG, nSamples);
start_cnt = get_counter(); 
			// Flush (customize coherence model here)
			esp_flush(coherence);
end_cnt = get_counter(); 
printf("acc flush time: %lu\n\n", end_cnt-start_cnt);
			// Start accelerators
			printf("  Start...\n");
start_cnt = get_counter(); 
			iowrite32(dev, CMD_REG, CMD_MASK_START);

			// Wait for completion
			done = 0;
			while (!done) {
				done = ioread32(dev, STATUS_REG);
				done &= STATUS_MASK_DONE;
			}
			iowrite32(dev, CMD_REG, 0x0);
end_cnt = get_counter(); 
printf("acc time: %lu\n\n", end_cnt-start_cnt);
			printf("  Done\n");
			printf("  validating...\n");

			/* Validation */
			errors = validate_buf(&mem[out_offset], gold);
			if (errors)
				printf("  ... FAIL\n");
			else
				printf("  ... PASS\n");
		}
		printf("cpu counter : %lu\n", end_cnt_pc-start_cnt_pc);
		aligned_free(ptable);
		aligned_free(mem);
		aligned_free(gold);
	}

	return 0;
}

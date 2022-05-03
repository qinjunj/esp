/* Copyright (c) 2011-2022 Columbia University, System Level Design Group */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdio.h>
#include <math.h>
#ifndef __riscv
#include <stdlib.h>
#endif

#include <esp_accelerator.h>
#include <esp_probe.h>
#include <fixed_point.h>

typedef float  token_t;

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
#define SLD_ROTATEORDER2 0x054
#define DEV_NAME "sld,rotateorder2_vivado"

/* <<--params-->> */
const int32_t nBatches = 1;
const int32_t channels = 5;
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
#define ROTATEORDER2_NBATCHES_REG 0x48
#define ROTATEORDER2_CHANNELS_REG 0x44
#define ROTATEORDER2_NSAMPLES_REG 0x40


static int validate_buf(token_t *out, token_t *gold)
{
	int i;
	int j;
	int errors = 0;

	for (i = 0; i < nBatches; i++)
		for (j = 0; j < channels * nSamples; j++)
			if (my_abs(gold[i * out_words_adj + j] - out[i * out_words_adj + j]) > 0.00001)
				errors++;

    printf("errors in baremetal: %d\n", errors); 
	return errors;
}


static void init_buf (token_t *in, token_t * gold)
{
	int i;
	int j;

    for (i = 0; i < channels; i++) {
        for (j = 0; j < nSamples; j++) {
            in[nSamples*i+j] = (token_t) (-1.0 + j/(float)(nSamples/2)); // random between -1 and 1 to mimic sin cos
        }
    }
    const float m_fCosAlpha = -4.37114e-08;
    const float m_fSinAlpha = -1;
    const float m_fCosBeta = -0.49908;
    const float m_fSinBeta = 0.866556; 
    const float m_fCosGamma = -4.37114e-08; 
    const float m_fSinGamma = 1; 
    const float m_fCos2Alpha = -1;
    const float m_fSin2Alpha = 8.74228e-08;
    const float m_fCos2Beta = -0.501838;
    const float m_fSin2Beta = -0.864962;
    const float m_fCos2Gamma = -1;
    const float m_fSin2Gamma = -8.74228e-08;
    
    const int kV = 0;
    const int kT = 1;
    const int kR = 2;
    const int kS = 3;
    const int kU = 4; 

    float m_pfTempSample[5] = {0};
    const float fSqrt3 = sqrt(3.f);

    start_cnt_pc = get_counter(); 
    for (int niSample = 0; niSample < nSamples; niSample++) {
        // Alpha rotation 
        m_pfTempSample[kV] = - in[kU*nSamples+niSample] * m_fSin2Alpha
                            + in[kV*nSamples+niSample] * m_fCos2Alpha;
        m_pfTempSample[kT] = - in[kS*nSamples+niSample] * m_fSinAlpha
                            + in[kT*nSamples+niSample] * m_fCosAlpha;
        m_pfTempSample[kR] = in[kR*nSamples+niSample];
        m_pfTempSample[kS] = in[kS*nSamples+niSample] * m_fCosAlpha
                             + in[kT*nSamples+niSample] * m_fSinAlpha;
        m_pfTempSample[kU] = in[kU*nSamples+niSample] * m_fCos2Alpha
                             + in[kV*nSamples+niSample] * m_fSin2Alpha; 
        // Beta rotation
        gold[kV*nSamples+niSample] = -m_fSinBeta * m_pfTempSample[kT]
                                + m_fCosBeta * m_pfTempSample[kV];
        gold[kT*nSamples+niSample] = -m_fCosBeta * m_pfTempSample[kT]
                                + m_fSinBeta * m_pfTempSample[kV];
        gold[kR*nSamples+niSample] = (0.75f * m_fCos2Beta + 0.25f) * m_pfTempSample[kR]
                                + (0.5 * fSqrt3 * pow(m_fSinBeta,2.0) ) * m_pfTempSample[kU]
                                + (fSqrt3 * m_fSinBeta * m_fCosBeta) * m_pfTempSample[kS];
        gold[kS*nSamples+niSample] = m_fCos2Beta * m_pfTempSample[kS]
                                - fSqrt3 * m_fCosBeta * m_fSinBeta * m_pfTempSample[kR]
                                + m_fCosBeta * m_fSinBeta * m_pfTempSample[kU];
        gold[kU*nSamples+niSample] = (0.25f * m_fCos2Beta + 0.75f) * m_pfTempSample[kU]
                                - m_fCosBeta * m_fSinBeta * m_pfTempSample[kS]
                                +0.5 * fSqrt3 * pow(m_fSinBeta,2.0) * m_pfTempSample[kR]; 

        // Gamma rotation
        m_pfTempSample[kV] = - gold[kU*nSamples+niSample] * m_fSin2Gamma
                            + gold[kV*nSamples+niSample] * m_fCos2Gamma;
        m_pfTempSample[kT] = - gold[kS*nSamples+niSample] * m_fSinGamma
                             + gold[kT*nSamples+niSample] * m_fCosGamma;

        m_pfTempSample[kR] = gold[kR*nSamples+niSample];
        m_pfTempSample[kS] = gold[kS*nSamples+niSample] * m_fCosGamma
                             + gold[kT*nSamples+niSample] * m_fSinGamma;
        m_pfTempSample[kU] = gold[kU*nSamples+niSample] * m_fCos2Gamma
                             + gold[kV*nSamples+niSample] * m_fSin2Gamma; 

        gold[kR*nSamples+niSample] = m_pfTempSample[kR];
        gold[kS*nSamples+niSample] = m_pfTempSample[kS];
        gold[kT*nSamples+niSample] = m_pfTempSample[kT];
        gold[kU*nSamples+niSample] = m_pfTempSample[kU];
        gold[kV*nSamples+niSample] = m_pfTempSample[kV]; 

    }
    end_cnt_pc = get_counter(); 
    /*
    FILE *inputFile = fopen("/scratch/projects/qinjunj2/esp/accelerators/vivado_hls/rotateorder2_vivado/hw/tb/inputs_2.txt", "r");
    FILE *outputFile = fopen("/scratch/projects/qinjunj2/esp/accelerators/vivado_hls/rotateorder2_vivado/hw/tb/outputs_2.txt", "r");
	
    float val = 0.0;
    for(int i = 0; i < channels; i++) {
      for (int j = 0 ; j < nSamples; j++) {
          fscanf(inputFile,"%f",&val);
          in[nSamples*i+j] = (token_t) val;
          fscanf(outputFile,"%f",&val);
          gold[nSamples*i+j] = (token_t) val;
          // printf("%f, %f", outbuff_gold[1024*i+j], inbuff[1024*i+j]);
      }
      // printf(“\n”);
    }
    fclose(inputFile);
    fclose(outputFile);
    */ 
    /*
    for (i = 0; i < nBatches; i++)
		for (j = 0; j < channels * nSamples; j++)
			in[i * in_words_adj + j] = (token_t) j;

	for (i = 0; i < nBatches; i++)
		for (j = 0; j < channels * nSamples; j++)
			gold[i * out_words_adj + j] = (token_t) j;
    */
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
		in_words_adj = channels * nSamples;
		out_words_adj = channels * nSamples;
	} else {
		in_words_adj = round_up(channels * nSamples, DMA_WORD_PER_BEAT(sizeof(token_t)));
		out_words_adj = round_up(channels * nSamples, DMA_WORD_PER_BEAT(sizeof(token_t)));
	}
	in_len = in_words_adj * (nBatches);
	out_len = out_words_adj * (nBatches);
	in_size = in_len * sizeof(token_t);
	out_size = out_len * sizeof(token_t);
	out_offset  = 0;
	mem_size = (out_offset * sizeof(token_t)) + out_size;


	// Search for the device
	printf("Scanning device tree... \n");

	ndev = probe(&espdevs, VENDOR_SLD, SLD_ROTATEORDER2, DEV_NAME);
	if (ndev == 0) {
		printf("rotateorder2 not found\n");
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
			coherence = ACC_COH_RECALL;
#endif
            // enum accelerator_coherence {ACC_COH_NONE = 0, ACC_COH_LLC, ACC_COH_RECALL, ACC_COH_FULL, ACC_COH_AUTO};
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
		iowrite32(dev, ROTATEORDER2_NBATCHES_REG, nBatches);
		iowrite32(dev, ROTATEORDER2_CHANNELS_REG, channels);
		iowrite32(dev, ROTATEORDER2_NSAMPLES_REG, nSamples);
            
            printf("  Start...\n");
            start_cnt = get_counter(); 
			// Flush (customize coherence model here)
			esp_flush(coherence);

			// Start accelerators
			// printf("  Start...\n");
            // start_cnt = get_counter();
			iowrite32(dev, CMD_REG, CMD_MASK_START);

			// Wait for completion
			done = 0;
			while (!done) {
				done = ioread32(dev, STATUS_REG);
				done &= STATUS_MASK_DONE;
			}
			iowrite32(dev, CMD_REG, 0x0);
            end_cnt = get_counter(); 

			printf("  Done\n");
			printf("  validating...\n");

			/* Validation */
			errors = validate_buf(&mem[out_offset], gold);
			if (errors)
				printf("  ... FAIL\n");
			else
				printf("  ... PASS\n");
		}
        
        printf("acc counter: %lu\n", end_cnt-start_cnt);
		printf("cpu  counter: %lu\n", end_cnt_pc-start_cnt_pc);
        
		aligned_free(ptable);
		aligned_free(mem);
		aligned_free(gold);
	}

	return 0;
}

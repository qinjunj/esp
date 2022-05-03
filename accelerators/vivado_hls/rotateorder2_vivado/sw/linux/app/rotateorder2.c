// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "libesp.h"
#include "cfg.h"

static unsigned in_words_adj;
static unsigned out_words_adj;
static unsigned in_len;
static unsigned out_len;
static unsigned in_size;
static unsigned out_size;
static unsigned out_offset;
static unsigned size;

/* User-defined code */
static int validate_buffer(token_t *out, token_t *gold)
{
	int i;
	int j;
	unsigned errors = 0;

	for (i = 0; i < nBatches; i++)
		for (j = 0; j < channels * nSamples; j++)
			if (gold[i * out_words_adj + j] != out[i * out_words_adj + j])
				errors++;

	return errors;
}


/* User-defined code */
static void init_buffer(token_t *in, token_t * gold)
{
	int i;
	int j;

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

    /*
	for (i = 0; i < nBatches; i++)
		for (j = 0; j < channels * nSamples; j++)
			in[i * in_words_adj + j] = (token_t) j;

	for (i = 0; i < nBatches; i++)
		for (j = 0; j < channels * nSamples; j++)
			gold[i * out_words_adj + j] = (token_t) j;
    */
}


/* User-defined code */
static void init_parameters()
{
	if (DMA_WORD_PER_BEAT(sizeof(token_t)) == 0) {
		in_words_adj = channels * nSamples;
		out_words_adj = channels * nSamples;
	} else {
		in_words_adj = round_up(channels * nSamples, DMA_WORD_PER_BEAT(sizeof(token_t)));
		out_words_adj = round_up(channels * nSamples, DMA_WORD_PER_BEAT(sizeof(token_t)));
	}
	in_len = in_words_adj * (nBatches);
	out_len =  out_words_adj * (nBatches);
	in_size = in_len * sizeof(token_t);
	out_size = out_len * sizeof(token_t);
	out_offset = 0;
	size = (out_offset * sizeof(token_t)) + out_size;
}


int main(int argc, char **argv)
{
	int errors;

	token_t *gold;
	token_t *buf;

	init_parameters();

	buf = (token_t *) esp_alloc(size);
	cfg_000[0].hw_buf = buf;
    
	gold = malloc(out_size);

	init_buffer(buf, gold);

	printf("\n====== %s ======\n\n", cfg_000[0].devname);
	/* <<--print-params-->> */
	printf("  .nBatches = %d\n", nBatches);
	printf("  .channels = %d\n", channels);
	printf("  .nSamples = %d\n", nSamples);
	printf("\n  ** START **\n");

	esp_run(cfg_000, NACC);

	printf("\n  ** DONE **\n");

	errors = validate_buffer(&buf[out_offset], gold);

	free(gold);
	esp_free(buf);

	if (!errors)
		printf("+ Test PASSED\n");
	else
		printf("+ Test FAILED\n");

	printf("\n====== %s ======\n\n", cfg_000[0].devname);

	return errors;
}

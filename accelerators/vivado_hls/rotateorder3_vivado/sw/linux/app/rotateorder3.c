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
		for (j = 0; j < nChannels*nSamples; j++)
			if (gold[i * out_words_adj + j] != out[i * out_words_adj + j])
				errors++;

	return errors;
}


/* User-defined code */
static void init_buffer(token_t *in, token_t * gold)
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


/* User-defined code */
static void init_parameters()
{
	if (DMA_WORD_PER_BEAT(sizeof(token_t)) == 0) {
		in_words_adj = nChannels*nSamples;
		out_words_adj = nChannels*nSamples;
	} else {
		in_words_adj = round_up(nChannels*nSamples, DMA_WORD_PER_BEAT(sizeof(token_t)));
		out_words_adj = round_up(nChannels*nSamples, DMA_WORD_PER_BEAT(sizeof(token_t)));
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
	printf("  .cosA1 = %d\n", cosA1);
	printf("  .cosA3 = %d\n", cosA3);
	printf("  .cosA2 = %d\n", cosA2);
	printf("  .nChannels = %d\n", nChannels);
	printf("  .sinB3 = %d\n", sinB3);
	printf("  .sinB2 = %d\n", sinB2);
	printf("  .sinB1 = %d\n", sinB1);
	printf("  .sinA2 = %d\n", sinA2);
	printf("  .sinA3 = %d\n", sinA3);
	printf("  .sinA1 = %d\n", sinA1);
	printf("  .cosG3 = %d\n", cosG3);
	printf("  .cosG2 = %d\n", cosG2);
	printf("  .cosG1 = %d\n", cosG1);
	printf("  .nSamples = %d\n", nSamples);
	printf("  .sinG1 = %d\n", sinG1);
	printf("  .sinG2 = %d\n", sinG2);
	printf("  .sinG3 = %d\n", sinG3);
	printf("  .cosB1 = %d\n", cosB1);
	printf("  .cosB2 = %d\n", cosB2);
	printf("  .cosB3 = %d\n", cosB3);
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

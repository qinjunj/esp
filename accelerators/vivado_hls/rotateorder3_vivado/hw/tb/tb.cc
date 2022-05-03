// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {

    printf("****start*****\n");

    /* <<--params-->> */
	 const unsigned nBatches = 1;
	 const float cosA1 = -4.37114e-08;
	 const float cosA3 = 1.19249e-08;
	 const float cosA2 = -1;
	 const unsigned nChannels = 7;
	 const float sinB3 = -0.00318557;
	 const float sinB2 = -0.864962;
	 const float sinB1 = 0.866556;
	 const float sinA2 = 8.74228e-08;
	 const float sinA3 = 1;
	 const float sinA1 = -1;
	 const float cosG3 = 1.19249e-08;
	 const float cosG2 = -1;
	 const float cosG1 = -4.37114e-08;
	 const unsigned nSamples = 1024;
	 const float sinG1 = 1;
	 const float sinG2 = -8.74228e-08;
	 const float sinG3 = -1;
	 const float cosB1 = -0.49908;
	 const float cosB2 = -0.501838;
	 const float cosB3 = 0.999995;

    uint32_t in_words_adj;
    uint32_t out_words_adj;
    uint32_t in_size;
    uint32_t out_size;
    uint32_t dma_in_size;
    uint32_t dma_out_size;
    uint32_t dma_size;


    in_words_adj = round_up(nChannels*nSamples, VALUES_PER_WORD);
    out_words_adj = round_up(nChannels*nSamples, VALUES_PER_WORD);
    in_size = in_words_adj * (nBatches);
    out_size = out_words_adj * (nBatches);

    dma_in_size = in_size / VALUES_PER_WORD;
    dma_out_size = out_size / VALUES_PER_WORD;
    dma_size = 0 + dma_out_size;

    dma_word_t *mem=(dma_word_t*) malloc(dma_size * sizeof(dma_word_t));
    word_t *inbuff=(word_t*) malloc(in_size * sizeof(word_t));
    word_t *outbuff=(word_t*) malloc(out_size * sizeof(word_t));
    word_t *outbuff_gold= (word_t*) malloc(out_size * sizeof(word_t));
    dma_info_t load;
    dma_info_t store;

    // Prepare input data
    // for(unsigned i = 0; i < nBatches; i++)
    //     for(unsigned j = 0; j < nChannels*nSamples; j++)
    //         inbuff[i * in_words_adj + j] = (word_t) j;

	// Set golden output
    // for(unsigned i = 0; i < nBatches; i++)
    //     for(unsigned j = 0; j < nChannels*nSamples; j++)
    //         outbuff_gold[i * out_words_adj + j] = (word_t) j;
	FILE *inputFile = fopen("/scratch/projects/fg14/esp/accelerators/vivado_hls/rotateorder3_vivado/hw/tb/inputs_3.txt", "r");
    FILE *outputFile = fopen("/scratch/projects/fg14/esp/accelerators/vivado_hls/rotateorder3_vivado/hw/tb/outputs_3.txt", "r");
    float val = 0.0;
    for(int i = 0; i < nChannels; i++) {
        for (int j = 0 ; j < nSamples; j++) {
            fscanf(inputFile,"%f",&val);
            inbuff[i*nSamples+j] = (word_t) val;
            fscanf(outputFile,"%f",&val);
            outbuff_gold[i*nSamples+j] = (word_t) val;
            // printf("%f ?= %f\n", outbuff_gold[nSamples*i+j], inbuff[nSamples*i+j]);
        }
        //printf("\n");
    }
    fclose(inputFile);
    fclose(outputFile);

    for(unsigned i = 0; i < dma_in_size; i++)
	for(unsigned k = 0; k < VALUES_PER_WORD; k++)
	    mem[i].word[k] = inbuff[i * VALUES_PER_WORD + k];

	const float cosA = -4.37114e-08;

    // Call the TOP function
    top(mem, mem,
        /* <<--args-->> */
	 	 nBatches,
	 	 cosA1,
	 	 cosA3,
	 	 cosA2,
	 	 nChannels,
	 	 sinB3,
	 	 sinB2,
	 	 sinB1,
	 	 sinA2,
	 	 sinA3,
	 	 sinA1,
	 	 cosG3,
	 	 cosG2,
	 	 cosG1,
	 	 nSamples,
	 	 sinG1,
	 	 sinG2,
	 	 sinG3,
	 	 cosB1,
	 	 cosB2,
	 	 cosB3,
        load, store);

    // Validate
    uint32_t out_offset = 0;
    for(unsigned i = 0; i < dma_out_size; i++)
	for(unsigned k = 0; k < VALUES_PER_WORD; k++)
	    outbuff[i * VALUES_PER_WORD + k] = mem[out_offset + i].word[k];

    int errors = 0;
    for(unsigned i = 0; i < nBatches; i++)
        for(unsigned j = 0; j < nChannels*nSamples; j++)
		if (std::abs(outbuff[i * out_words_adj + j] - outbuff_gold[i * out_words_adj + j]) > 0.00001)
		        errors++;

    if (errors)
	std::cout << "Test FAILED with " << errors << " errors." << std::endl;
    else
	std::cout << "Test PASSED." << std::endl;
	
    // Free memory

    free(mem);
    free(inbuff);
    free(outbuff);
    free(outbuff_gold);

    return 0;
}

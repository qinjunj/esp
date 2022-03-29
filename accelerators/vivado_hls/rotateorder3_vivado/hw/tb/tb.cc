// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cmath>

int main(int argc, char **argv) {

    printf("****start*****\n");

    /* <<--params-->> */
    const unsigned nBatches = 1;
    const unsigned nChannels = 7;
	const unsigned nSamples = 1024;

    uint32_t in_words_adj;
    uint32_t out_words_adj;
    uint32_t in_size;
    uint32_t out_size;
    uint32_t dma_in_size;
    uint32_t dma_out_size;
    uint32_t dma_size;

    std::cout << "VALUES_PER_WORD: " << VALUES_PER_WORD << std::endl; 

    in_words_adj = round_up(nChannels * nSamples, VALUES_PER_WORD);
    out_words_adj = round_up(nChannels * nSamples, VALUES_PER_WORD);
    in_size = in_words_adj * (nBatches);
    out_size = out_words_adj * (nBatches);

    dma_in_size = in_size / VALUES_PER_WORD;
    dma_out_size = out_size / VALUES_PER_WORD;
    dma_size = dma_in_size + dma_out_size;

    dma_word_t *mem=(dma_word_t*) malloc(dma_size * sizeof(dma_word_t));
    word_t *inbuff=(word_t*) malloc(in_size * sizeof(word_t));
    word_t *outbuff=(word_t*) malloc(out_size * sizeof(word_t));
    word_t *outbuff_gold= (word_t*) malloc(out_size * sizeof(word_t));
    dma_info_t load;
    dma_info_t store;

    // Prepare input data
    printf("?-----%d, %d, %d, %d, %d, %d\n", VALUES_PER_WORD, in_words_adj, out_words_adj, in_size, out_size, dma_size);
    
    
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
        printf("\n");
    }
    fclose(inputFile);
    fclose(outputFile);

    // for(unsigned i = 0; i < 1; i++)
    //     for(unsigned j = 0; j < nSamples; j++)
    //         inbuff[i * in_words_adj + j] = (word_t) j;

    for(unsigned i = 0; i < dma_in_size; i++)
        for(unsigned k = 0; k < VALUES_PER_WORD; k++)
            mem[i].word[k] = inbuff[i * VALUES_PER_WORD + k];

    // // Set golden output
    // for(unsigned i = 0; i < 1; i++)
    //     for(unsigned j = 0; j < nSamples; j++)
    //         outbuff_gold[i * out_words_adj + j] = (word_t) j;


    // Call the TOP function
    top(mem, mem,
        /* <<--args-->> */
         nBatches,
         nChannels,
	 	 nSamples,
        load, store);

    // Validate
    uint32_t out_offset = dma_in_size;
    for(unsigned i = 0; i < dma_out_size; i++)
        for(unsigned k = 0; k < VALUES_PER_WORD; k++)
            outbuff[i * VALUES_PER_WORD + k] = mem[out_offset + i].word[k];

    int errors = 0;
    for(unsigned i = 0; i < nBatches; i++)
        for(unsigned j = 0; j < nChannels * nSamples; j++)
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

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

struct Sample {
    word_t kQ, kO, kM, kK, kL, kN, kP;
};

Sample unit_test_process_order_3(Sample sample) {
    float fSqrt3_2 = sqrt(3.f/2.f);
    float fSqrt15 = sqrt(15.f);
    float fSqrt5_2 = sqrt(5.f/2.f);

    float m_fCosAlpha = -4.37114e-08;
    float m_fSinAlpha = -1;
    float m_fCosBeta = -0.49908;
    float m_fSinBeta = 0.866556;
    float m_fCosGamma = -4.37114e-08;
    float m_fSinGamma = 1;

    float m_fCos2Alpha = -1;
    float m_fSin2Alpha = 8.74228e-08;
    float m_fCos2Beta = -0.501838;
    float m_fSin2Beta = -0.864962;
    float m_fCos2Gamma = -1;
    float m_fSin2Gamma = -8.74228e-08;

    float m_fCos3Alpha = 1.19249e-08;
    float m_fSin3Alpha = 1;
    float m_fCos3Beta = 0.999995;
    float m_fSin3Beta = -0.00318557;
    float m_fCos3Gamma = 1.19249e-08;
    float m_fSin3Gamma = -1;

    Sample tmp;

    // Alpha rotation
    tmp.kQ = - sample.kP * m_fSin3Alpha
                        + sample.kQ * m_fCos3Alpha;
    tmp.kO = - sample.kN * m_fSin2Alpha
                        + sample.kO * m_fCos2Alpha;
    tmp.kM = - sample.kL * m_fSinAlpha
                        + sample.kM * m_fCosAlpha;
    tmp.kK = sample.kK;
    tmp.kL = sample.kL * m_fCosAlpha
                        + sample.kM * m_fSinAlpha;
    tmp.kN = sample.kN * m_fCos2Alpha
                        + sample.kO * m_fSin2Alpha;
    tmp.kP = sample.kP * m_fCos3Alpha
                        + sample.kQ * m_fSin3Alpha;

    // Beta rotation
    sample.kQ = 0.125f * tmp.kQ * (5.f + 3.f*m_fCos2Beta)
                - fSqrt3_2 * tmp.kO *m_fCosBeta * m_fSinBeta
                + 0.25f * fSqrt15 * tmp.kM * pow(m_fSinBeta,2.0f);
    sample.kO = tmp.kO * m_fCos2Beta
                - fSqrt5_2 * tmp.kM * m_fCosBeta * m_fSinBeta
                + fSqrt3_2 * tmp.kQ * m_fCosBeta * m_fSinBeta;
    sample.kM = 0.125f * tmp.kM * (3.f + 5.f*m_fCos2Beta)
                - fSqrt5_2 * tmp.kO *m_fCosBeta * m_fSinBeta
                + 0.25f * fSqrt15 * tmp.kQ * pow(m_fSinBeta,2.0f);
    sample.kK = 0.25f * tmp.kK * m_fCosBeta * (-1.f + 15.f*m_fCos2Beta)
                + 0.5f * fSqrt15 * tmp.kN * m_fCosBeta * pow(m_fSinBeta,2.f)
                + 0.5f * fSqrt5_2 * tmp.kP * pow(m_fSinBeta,3.f)
                + 0.125f * fSqrt3_2 * tmp.kL * (m_fSinBeta + 5.f * m_fSin3Beta);
    sample.kL = 0.0625f * tmp.kL * (m_fCosBeta + 15.f * m_fCos3Beta)
                + 0.25f * fSqrt5_2 * tmp.kN * (1.f + 3.f * m_fCos2Beta) * m_fSinBeta
                + 0.25f * fSqrt15 * tmp.kP * m_fCosBeta * pow(m_fSinBeta,2.f)
                - 0.125 * fSqrt3_2 * tmp.kK * (m_fSinBeta + 5.f * m_fSin3Beta);
    sample.kN = 0.125f * tmp.kN * (5.f * m_fCosBeta + 3.f * m_fCos3Beta)
                + 0.25f * fSqrt3_2 * tmp.kP * (3.f + m_fCos2Beta) * m_fSinBeta
                + 0.5f * fSqrt15 * tmp.kK * m_fCosBeta * pow(m_fSinBeta,2.f)
                + 0.125 * fSqrt5_2 * tmp.kL * (m_fSinBeta - 3.f * m_fSin3Beta);
    sample.kP = 0.0625f * tmp.kP * (15.f * m_fCosBeta + m_fCos3Beta)
                - 0.25f * fSqrt3_2 * tmp.kN * (3.f + m_fCos2Beta) * m_fSinBeta
                + 0.25f * fSqrt15 * tmp.kL * m_fCosBeta * pow(m_fSinBeta,2.f)
                - 0.5 * fSqrt5_2 * tmp.kK * pow(m_fSinBeta,3.f);

    // Gamma rotation
    tmp.kQ = - sample.kP * m_fSin3Gamma
                        + sample.kQ * m_fCos3Gamma;
    tmp.kO = - sample.kN * m_fSin2Gamma
                        + sample.kO * m_fCos2Gamma;
    tmp.kM = - sample.kL * m_fSinGamma
                        + sample.kM * m_fCosGamma;
    tmp.kK = sample.kK;
    tmp.kL = sample.kL * m_fCosGamma
                        + sample.kM * m_fSinGamma;
    tmp.kN = sample.kN * m_fCos2Gamma
                        + sample.kO * m_fSin2Gamma;
    tmp.kP = sample.kP * m_fCos3Gamma
                        + sample.kQ * m_fSin3Gamma;

    return tmp;
}

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

    // check
    int k = 0;
    Sample check {
        // kQ, kO, kM, kK, kL, kN, kP;
        // 0.00193399,
        // 2.27074e-11,
        // -0.00893265,
        // -9.30881e-05,
        // -0.00573578,
        // -1.12019e-10,
        // -0.0135675,
        inbuff[0 * nSamples + k],
        inbuff[1 * nSamples + k],
        inbuff[2 * nSamples + k],
        inbuff[3 * nSamples + k],
        inbuff[4 * nSamples + k],
        inbuff[5 * nSamples + k],
        inbuff[6 * nSamples + k],
    };
    
    auto out = unit_test_process_order_3(check);
    printf("test ssadfsdf %f ?= %f", outbuff_gold[4 * nSamples + k], out.kL);

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
    // uint32_t out_offset = dma_in_size;
    // for(unsigned i = 0; i < dma_out_size; i++)
    //     for(unsigned k = 0; k < VALUES_PER_WORD; k++)
    //         outbuff[i * VALUES_PER_WORD + k] = mem[out_offset + i].word[k];

    uint32_t out_offset = 0;
    for(unsigned i = 0; i < dma_out_size; i++)
        for(unsigned k = 0; k < VALUES_PER_WORD; k++)
            outbuff[i * VALUES_PER_WORD + k] = mem[out_offset + i].word[k];

    int errors = 0;
    for(unsigned i = 0; i < nBatches; i++)
        for(unsigned j = 0; j < nChannels * nSamples; j++)
            if (std::abs(outbuff[i * out_words_adj + j] - outbuff_gold[i * out_words_adj + j]) > 0.00001) {
                printf("%f != %f\n", outbuff[i * out_words_adj + j], outbuff_gold[i * out_words_adj + j]);
                errors++;
            }
		        

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

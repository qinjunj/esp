// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>

void load(word_t _inbuff[SIZE_IN_CHUNK_DATA], dma_word_t *in1,
          /* <<--compute-params-->> */
	 const unsigned nBatches,
	 const float cosA1,
	 const float cosA3,
	 const float cosA2,
	 const unsigned nChannels,
	 const float sinB3,
	 const float sinB2,
	 const float sinB1,
	 const float sinA2,
	 const float sinA3,
	 const float sinA1,
	 const float cosG3,
	 const float cosG2,
	 const float cosG1,
	 const unsigned nSamples,
	 const float sinG1,
	 const float sinG2,
	 const float sinG3,
	 const float cosB1,
	 const float cosB2,
	 const float cosB3,
	  dma_info_t &load_ctrl, int chunk, int batch)
{
load_data:

    const unsigned length = round_up(nChannels*nSamples, VALUES_PER_WORD) / 1;
    const unsigned index = length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index = index / VALUES_PER_WORD;

    load_ctrl.index = dma_index;
    load_ctrl.length = dma_length;
    load_ctrl.size = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length; i++) {
    load_label0:for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
	    _inbuff[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
    	}
    }
}

void store(word_t _outbuff[SIZE_OUT_CHUNK_DATA], dma_word_t *out,
          /* <<--compute-params-->> */
	 const unsigned nBatches,
	 const float cosA1,
	 const float cosA3,
	 const float cosA2,
	 const unsigned nChannels,
	 const float sinB3,
	 const float sinB2,
	 const float sinB1,
	 const float sinA2,
	 const float sinA3,
	 const float sinA1,
	 const float cosG3,
	 const float cosG2,
	 const float cosG1,
	 const unsigned nSamples,
	 const float sinG1,
	 const float sinG2,
	 const float sinG3,
	 const float cosB1,
	 const float cosB2,
	 const float cosB3,
	   dma_info_t &store_ctrl, int chunk, int batch)
{
store_data:

    const unsigned length = round_up(nChannels*nSamples, VALUES_PER_WORD) / 1;
    const unsigned store_offset = round_up(nChannels*nSamples, VALUES_PER_WORD) * nBatches;
    const unsigned out_offset = 0;
    const unsigned index = out_offset + length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index = index / VALUES_PER_WORD;

    store_ctrl.index = dma_index;
    store_ctrl.length = dma_length;
    store_ctrl.size = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length; i++) {
    store_label1:for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
	    out[dma_index + i].word[j] = _outbuff[i * VALUES_PER_WORD + j];
	}
    }
}



void compute(word_t _inbuff[SIZE_IN_CHUNK_DATA],
             /* <<--compute-params-->> */
	 const unsigned nBatches,
	 const float cosA1,
	 const float cosA3,
	 const float cosA2,
	 const unsigned nChannels,
	 const float sinB3,
	 const float sinB2,
	 const float sinB1,
	 const float sinA2,
	 const float sinA3,
	 const float sinA1,
	 const float cosG3,
	 const float cosG2,
	 const float cosG1,
	 const unsigned nSamples,
	 const float sinG1,
	 const float sinG2,
	 const float sinG3,
	 const float cosB1,
	 const float cosB2,
	 const float cosB3,
             word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{
	const word_t fSqrt3_2 = sqrt(3.f/2.f);
    const word_t fSqrt15 = sqrt(15.f);
    const word_t fSqrt5_2 = sqrt(5.f/2.f);

	const int unrolling_factor = 64;

	word_t temp0[unrolling_factor] = {0};
    word_t temp1[unrolling_factor] = {0};
    word_t temp2[unrolling_factor] = {0};
    word_t temp3[unrolling_factor] = {0};
    word_t temp4[unrolling_factor] = {0}; 
	word_t temp5[unrolling_factor] = {0}; 
	word_t temp6[unrolling_factor] = {0}; 

    word_t t0[unrolling_factor] = {0}; 
    word_t t1[unrolling_factor] = {0};
    word_t t2[unrolling_factor] = {0};
    word_t t3[unrolling_factor] = {0};
    word_t t4[unrolling_factor] = {0}; 
	word_t t5[unrolling_factor] = {0};
	word_t t6[unrolling_factor] = {0};

    // for buffering _inbuff
    word_t buffer0[unrolling_factor] = {0};
    word_t buffer1[unrolling_factor] = {0}; 
    word_t buffer2[unrolling_factor] = {0};
    word_t buffer3[unrolling_factor] = {0};
    word_t buffer4[unrolling_factor] = {0};
	word_t buffer5[unrolling_factor] = {0};
	word_t buffer6[unrolling_factor] = {0};

	
    compute_label2:for (int c = 0; c < 1024/unrolling_factor; c++) {
		int disp = c*unrolling_factor; 
		// Alpha rotation
		for (int n = 0; n < unrolling_factor; n++) {
        #pragma HLS unroll			
			temp0[n] = - buffer6[n] * sinA3
								+ buffer0[n] * cosA3;
			temp1[n] = - buffer5[n] * sinA2
								+ buffer1[n] * cosA2;
			temp2[n] = - buffer4[n] * sinA1
								+ buffer2[n] * cosA1;
			temp3[n] = buffer3[n];
			temp4[n] = buffer4[n] * cosA1
								+ buffer2[n] * sinA1;
			temp5[n] = buffer5[n] * cosA2
								+ buffer1[n] * sinA2;
			temp6[n] = buffer6[n] * cosA3
								+ buffer0[n] * sinA3;
        }
	
		// Beta rotation
		for (int n = 0; n < unrolling_factor; n++) {
        #pragma HLS unroll
			t0[n] = 0.125f * temp0[n] * (5.f + 3.f*cosB2)
						- fSqrt3_2 * temp1[n] *cosB1 * sinB1
						+ 0.25f * fSqrt15 * temp2[n] * pow(sinB1,2.0f);
			t1[n] = temp1[n] * cosB2
						- fSqrt5_2 * temp2[n] * cosB1 * sinB1
						+ fSqrt3_2 * temp0[n] * cosB1 * sinB1;
			t2[n] = 0.125f * temp2[n] * (3.f + 5.f*cosB2)
						- fSqrt5_2 * temp1[n] *cosB1 * sinB1
						+ 0.25f * fSqrt15 * temp0[n] * pow(sinB1,2.0f);
			t3[n] = 0.25f * temp3[n] * cosB1* (-1.f + 15.f*cosB2)
						+ 0.5f * fSqrt15 * temp5[n] * cosB1 * pow(sinB1,2.f)
						+ 0.5f * fSqrt5_2 * temp6[n] * pow(sinB1,3.f)
						+ 0.125f * fSqrt3_2 * temp4[n] * (sinB1 + 5.f * sinB3);
			t4[n] = 0.0625f * temp4[n] * (cosB1 + 15.f * cosB3)
						+ 0.25f * fSqrt5_2 * temp5[n] * (1.f + 3.f * cosB2) * sinB1
						+ 0.25f * fSqrt15 * temp6[n] * cosB1 * pow(sinB1,2.f)
						- 0.125 * fSqrt3_2 * temp3[n] * (sinB1 + 5.f * sinB3);
			t5[n] = 0.125f * temp5[n] * (5.f * cosB1 + 3.f * cosB3)
						+ 0.25f * fSqrt3_2 * temp6[n] * (3.f + cosB2) * sinB1
						+ 0.5f * fSqrt15 * temp3[n] * cosB1 * pow(sinB1,2.f)
						+ 0.125 * fSqrt5_2 * temp4[n] * (sinB1 - 3.f * sinB3);
			t6[n] = 0.0625f * temp6[n] * (15.f * cosB1 + cosB3)
						- 0.25f * fSqrt3_2 * temp5[n] * (3.f + cosB2) * sinB1
						+ 0.25f * fSqrt15 * temp4[n] * cosB1 * pow(sinB1,2.f)
						- 0.5 * fSqrt5_2 * temp3[n] * pow(sinB1,3.f);
		}

		// Gamma rotation
		for (int n = 0; n < unrolling_factor; n++) {
        #pragma HLS unroll 
			_outbuff[disp+n] = - t6[n] * sinG3
								+ t0[n] * cosG3;
			_outbuff[1024+disp+n] = - t5[n] * sinG2
								+ t1[n] * cosG2;
			_outbuff[2048+disp+n] = - t4[n] * sinG1
								+ t2[n] * cosG1;
			_outbuff[3072+disp+n] = t3[n];
			_outbuff[4096+disp+n] = t4[n] * cosG1
								+ t2[n] * sinG1;
			_outbuff[5120+disp+n] = t5[n] * cosG2
								+ t1[n] * sinG2;
			_outbuff[6144+disp+n] = t6[n] * cosG3
								+ t0[n] * sinG3;
		}
    }
}


void top(dma_word_t *out, dma_word_t *in1,
         /* <<--params-->> */
	 const unsigned conf_info_nBatches,
	 const float conf_info_cosA1,
	 const float conf_info_cosA3,
	 const float conf_info_cosA2,
	 const unsigned conf_info_nChannels,
	 const float conf_info_sinB3,
	 const float conf_info_sinB2,
	 const float conf_info_sinB1,
	 const float conf_info_sinA2,
	 const float conf_info_sinA3,
	 const float conf_info_sinA1,
	 const float conf_info_cosG3,
	 const float conf_info_cosG2,
	 const float conf_info_cosG1,
	 const unsigned conf_info_nSamples,
	 const float conf_info_sinG1,
	 const float conf_info_sinG2,
	 const float conf_info_sinG3,
	 const float conf_info_cosB1,
	 const float conf_info_cosB2,
	 const float conf_info_cosB3,
	 dma_info_t &load_ctrl, dma_info_t &store_ctrl)
{

    /* <<--local-params-->> */
	//  const unsigned nBatches = conf_info_nBatches;
	//  const float cosA1 = conf_info_cosA1;
	//  const float cosA3 = conf_info_cosA3;
	//  const float cosA2 = conf_info_cosA2;
	//  const unsigned nChannels = conf_info_nChannels;
	//  const float sinB3 = conf_info_sinB3;
	//  const float sinB2 = conf_info_sinB2;
	//  const float sinB1 = conf_info_sinB1;
	//  const float sinA2 = conf_info_sinA2;
	//  const float sinA3 = conf_info_sinA3;
	//  const float sinA1 = conf_info_sinA1;
	//  const float cosG3 = conf_info_cosG3;
	//  const float cosG2 = conf_info_cosG2;
	//  const float cosG1 = conf_info_cosG1;
	//  const unsigned nSamples = conf_info_nSamples;
	//  const float sinG1 = conf_info_sinG1;
	//  const float sinG2 = conf_info_sinG2;
	//  const float sinG3 = conf_info_sinG3;
	//  const float cosB1 = conf_info_cosB1;
	//  const float cosB2 = conf_info_cosB2;
	//  const float cosB3 = conf_info_cosB3;

    // Batching
batching:
    for (unsigned b = 0; b < conf_info_nBatches; b++)
    {
        // Chunking
    go:
        for (int c = 0; c < 1; c++)
        {
            word_t _inbuff[SIZE_IN_CHUNK_DATA];
            word_t _outbuff[SIZE_OUT_CHUNK_DATA];

            load(_inbuff, in1,
                 /* <<--args-->> */
	 	 conf_info_nBatches,
	 	 conf_info_cosA1,
	 	 conf_info_cosA3,
	 	 conf_info_cosA2,
	 	 conf_info_nChannels,
	 	 conf_info_sinB3,
	 	 conf_info_sinB2,
	 	 conf_info_sinB1,
	 	 conf_info_sinA2,
	 	 conf_info_sinA3,
	 	 conf_info_sinA1,
	 	 conf_info_cosG3,
	 	 conf_info_cosG2,
	 	 conf_info_cosG1,
	 	 conf_info_nSamples,
	 	 conf_info_sinG1,
	 	 conf_info_sinG2,
	 	 conf_info_sinG3,
	 	 conf_info_cosB1,
	 	 conf_info_cosB2,
	 	 conf_info_cosB3,
                 load_ctrl, c, b);
            compute(_inbuff,
                    /* <<--args-->> */
	 	 conf_info_nBatches,
	 	 conf_info_cosA1,
	 	 conf_info_cosA3,
	 	 conf_info_cosA2,
	 	 conf_info_nChannels,
	 	 conf_info_sinB3,
	 	 conf_info_sinB2,
	 	 conf_info_sinB1,
	 	 conf_info_sinA2,
	 	 conf_info_sinA3,
	 	 conf_info_sinA1,
	 	 conf_info_cosG3,
	 	 conf_info_cosG2,
	 	 conf_info_cosG1,
	 	 conf_info_nSamples,
	 	 conf_info_sinG1,
	 	 conf_info_sinG2,
	 	 conf_info_sinG3,
	 	 conf_info_cosB1,
	 	 conf_info_cosB2,
	 	 conf_info_cosB3,
                    _outbuff);
            store(_outbuff, out,
                  /* <<--args-->> */
	 	 conf_info_nBatches,
	 	 conf_info_cosA1,
	 	 conf_info_cosA3,
	 	 conf_info_cosA2,
	 	 conf_info_nChannels,
	 	 conf_info_sinB3,
	 	 conf_info_sinB2,
	 	 conf_info_sinB1,
	 	 conf_info_sinA2,
	 	 conf_info_sinA3,
	 	 conf_info_sinA1,
	 	 conf_info_cosG3,
	 	 conf_info_cosG2,
	 	 conf_info_cosG1,
	 	 conf_info_nSamples,
	 	 conf_info_sinG1,
	 	 conf_info_sinG2,
	 	 conf_info_sinG3,
	 	 conf_info_cosB1,
	 	 conf_info_cosB2,
	 	 conf_info_cosB3,
                  store_ctrl, c, b);
        }
    }
}

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

	const unsigned kQ = 0, kO = 1, kM = 2, kK = 3, kL = 4, kN = 5, kP = 6;
	const float fSqrt3_2 = sqrt(3.f/2.f);
    const float fSqrt15 = sqrt(15.f);
    const float fSqrt5_2 = sqrt(5.f/2.f);

	word_t tmp[nChannels];
	word_t tmp2[nChannels];

    for(unsigned niSample = 0; niSample < nSamples; niSample++)
    {
	
      	// Alpha rotation
		tmp[kQ] = - _inbuff[niSample+nSamples*kP] * sinA3
							+ _inbuff[niSample+nSamples*kQ] * cosA3;
		tmp[kO] = - _inbuff[niSample+nSamples*kN] * sinA2
							+ _inbuff[niSample+nSamples*kO] * cosA2;
		tmp[kM] = - _inbuff[niSample+nSamples*kL] * sinA1
							+ _inbuff[niSample+nSamples*kM] * cosA1;
		tmp[kK] = _inbuff[niSample+nSamples*kK];
		tmp[kL] = _inbuff[niSample+nSamples*kL] * cosA1
							+ _inbuff[niSample+nSamples*kM] * sinA1;
		tmp[kN] = _inbuff[niSample+nSamples*kN] * cosA2
							+ _inbuff[niSample+nSamples*kO] * sinA2;
		tmp[kP] = _inbuff[niSample+nSamples*kP] * cosA3
							+ _inbuff[niSample+nSamples*kQ] * sinA3;

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
		_outbuff[kQ * nSamples + niSample] = - tmp2[kP] * sinG3
							+ tmp2[kQ] * cosG3;
		_outbuff[kO* nSamples + niSample] = - tmp2[kN] * sinG2
							+ tmp2[kO] * cosG2;
		_outbuff[kM* nSamples + niSample] = - tmp2[kL] * sinG1
							+ tmp2[kM] * cosG1;
		_outbuff[kK* nSamples + niSample] = tmp2[kK];
		_outbuff[kL* nSamples + niSample] = tmp2[kL] * cosG1
							+ tmp2[kM] * sinG1;
		_outbuff[kN* nSamples + niSample] = tmp2[kN] * cosG2
							+ tmp2[kO] * sinG2;
		_outbuff[kP* nSamples + niSample] = tmp2[kP] * cosG3
							+ tmp2[kQ] * sinG3;
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

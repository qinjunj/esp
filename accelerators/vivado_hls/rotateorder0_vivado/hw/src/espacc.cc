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
	 const unsigned nChannels,
	 const unsigned nSamples,
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
	 const unsigned nChannels,
	 const unsigned nSamples,
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
	 const unsigned nChannels,
	 const unsigned nSamples,
             word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{
    const word_t m_fSinGammaf = 1;
    const word_t m_fCosBetaf = -0.49908;
    const word_t m_fCosAlphaf = -4.37114e-08;
    const word_t m_fSinAlphaf = -1;
    const word_t m_fSinBetaf = 0.866556;
    const word_t m_fCosGammaf = -4.37114e-08;
    
    word_t temp0[64] = {0};
    word_t temp1[64] = {0};
    word_t temp2[64] = {0};

    word_t t0[64] = {0}; 
    word_t t1[64] = {0};
    word_t t2[64] = {0};

    word_t buffer0[64] = {0};
    word_t buffer1[64] = {0}; 
    word_t buffer2[64] = {0};

    compute_label2:for (int c = 0; c < 16; c++) {
        int disp = c*64; 
        for (int n = 0; n < 64; n++) {
            buffer0[n] = _inbuff[disp+n];
            buffer1[n] = _inbuff[1024+disp+n];
            buffer2[n] = _inbuff[2048+disp+n];
        }
        for (int n = 0; n < 64; n++) {
        #pragma HLS unroll
            temp0[n] = - buffer2[n] * m_fSinAlphaf
                      + buffer0[n] * m_fCosAlphaf;
            temp1[n] = buffer1[n];
            temp2[n] = buffer2[n] * m_fCosAlphaf
                      + buffer0[n] * m_fSinAlphaf;
        }
        for (int n = 0; n < 64; n++) {
        #pragma HLS unroll
            t0[n] = temp0[n];
            t1[n] = m_fCosBetaf * temp1[n]
                          + m_fSinBetaf * temp2[n];
            t2[n] = m_fCosBetaf * temp2[n]
                          - m_fSinBetaf * temp1[n];
        }
        for (int n = 0; n < 64; n++) {
        #pragma HLS unroll 
            _outbuff[disp+n] = - t2[n] * m_fSinGammaf
                               + t0[n] * m_fCosGammaf;
            _outbuff[1024+disp+n] = t1[n];
            _outbuff[2048+disp+n] = t2[n] * m_fCosGammaf
                                    + t0[n] * m_fSinGammaf;
        }
        
    }
}


void top(dma_word_t *out, dma_word_t *in1,
         /* <<--params-->> */
	 const unsigned conf_info_nBatches,
	 const unsigned conf_info_nChannels,
	 const unsigned conf_info_nSamples,
	 dma_info_t &load_ctrl, dma_info_t &store_ctrl)
{

    /* <<--local-params-->> */
	 const unsigned nBatches = conf_info_nBatches;
	 const unsigned nChannels = conf_info_nChannels;
	 const unsigned nSamples = conf_info_nSamples;

    // Batching
batching:
    for (unsigned b = 0; b < nBatches; b++)
    {
        // Chunking
    go:
        for (int c = 0; c < 1; c++)
        {
            word_t _inbuff[SIZE_IN_CHUNK_DATA];
            word_t _outbuff[SIZE_OUT_CHUNK_DATA];

            load(_inbuff, in1,
                 /* <<--args-->> */
	 	 nBatches,
	 	 nChannels,
	 	 nSamples,
                 load_ctrl, c, b);
            compute(_inbuff,
                    /* <<--args-->> */
	 	 nBatches,
	 	 nChannels,
	 	 nSamples,
                    _outbuff);
            store(_outbuff, out,
                  /* <<--args-->> */
	 	 nBatches,
	 	 nChannels,
	 	 nSamples,
                  store_ctrl, c, b);
        }
    }
}

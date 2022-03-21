// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>

void load(word_t _inbuff[SIZE_IN_CHUNK_DATA], dma_word_t *in1,
          /* <<--compute-params-->> */
	 const unsigned nSamples,
	  dma_info_t &load_ctrl, int chunk, int batch)
{
load_data:

    const unsigned length = round_up(3*nSamples, VALUES_PER_WORD) / 1;
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
	 const unsigned nSamples,
	   dma_info_t &store_ctrl, int chunk, int batch)
{
store_data:

    const unsigned length = round_up(3*nSamples, VALUES_PER_WORD) / 1;
    const unsigned store_offset = round_up(3*nSamples, VALUES_PER_WORD) * 1;
    const unsigned out_offset = store_offset;
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
	 const unsigned nSamples,
             word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{

    // TODO implement compute functionality
    float m_fSinGammaf = 1;
    float m_fCosBetaf = -0.49908;
    float m_fCosAlphaf = -4.37114e-08;
    float m_fSinAlphaf = -1;
    float m_fSinBetaf = 0.866556;
    float m_fCosGammaf = -4.37114e-08;
    float m_pfTempSample[3];
    float m_pfTempSample2[3];
    
    for (int in_rem = nSamples; in_rem > 0; in_rem -= SIZE_IN_CHUNK_DATA) {
        int in_len  = in_rem  > SIZE_IN_CHUNK_DATA  ? SIZE_IN_CHUNK_DATA  : in_rem;
        
        for(int niSample = 0; niSample < in_len; niSample++) {
            // Alpha rotation
            m_pfTempSample[0] = -_inbuff[2*nSamples+niSample] * m_fSinAlphaf
                                + _inbuff[0*nSamples+niSample] * m_fCosAlphaf;
            m_pfTempSample[1] = _inbuff[1*nSamples+niSample];
            m_pfTempSample[2] = _inbuff[2*nSamples+niSample] * m_fCosAlphaf
                                + _inbuff[0*nSamples+niSample] * m_fSinAlphaf;

            // Beta rotation
            //_inbuff[0*nSamples+niSample] = m_pfTempSample[0];
            m_pfTempSample2[0] = m_pfTempSample[0];
            //_inbuff[1*nSamples+niSample] = m_pfTempSample[1] * m_fCosBetaf
                                //+  m_pfTempSample[2] * m_fSinBetaf;
            m_pfTempSample2[1] = m_pfTempSample[1] * m_fCosBetaf
                                +  m_pfTempSample[2] * m_fSinBetaf;
            //_inbuff[2*nSamples+niSample] = m_pfTempSample[2] * m_fCosBetaf
                                //- m_pfTempSample[1] * m_fSinBetaf;
            m_pfTempSample2[2] = m_pfTempSample[2] * m_fCosBetaf
                                - m_pfTempSample[1] * m_fSinBetaf;

            // Gamma rotation
            m_pfTempSample[0] = -m_pfTempSample2[2] * m_fSinGammaf
                                + m_pfTempSample2[0] * m_fCosGammaf;
            m_pfTempSample[1] = m_pfTempSample2[1];
            m_pfTempSample[2] = m_pfTempSample2[2] * m_fCosGammaf
                                + m_pfTempSample2[0] * m_fSinGammaf;

            _outbuff[2*nSamples+niSample] = m_pfTempSample[2];
            _outbuff[0*nSamples+niSample] = m_pfTempSample[0];
            _outbuff[1*nSamples+niSample] = m_pfTempSample[1];
        }
    }
}


void top(dma_word_t *out, dma_word_t *in1,
         /* <<--params-->> */
	 const unsigned conf_info_nSamples,
	 dma_info_t &load_ctrl, dma_info_t &store_ctrl)
{

    /* <<--local-params-->> */
	 const unsigned nSamples = conf_info_nSamples;

    // Batching
batching:
    for (unsigned b = 0; b < 1; b++)
    {
        // Chunking
    go:
        for (int c = 0; c < 1; c++)
        {
            word_t _inbuff[SIZE_IN_CHUNK_DATA];
            word_t _outbuff[SIZE_OUT_CHUNK_DATA];

            load(_inbuff, in1,
                 /* <<--args-->> */
	 	 nSamples,
                 load_ctrl, c, b);
            compute(_inbuff,
                    /* <<--args-->> */
	 	 nSamples,
                    _outbuff);
            store(_outbuff, out,
                  /* <<--args-->> */
	 	 nSamples,
                  store_ctrl, c, b);
        }
    }
}

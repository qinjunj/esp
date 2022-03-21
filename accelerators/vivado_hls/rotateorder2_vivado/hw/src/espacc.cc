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
	 const unsigned channels,
	 const unsigned nSamples,
	  dma_info_t &load_ctrl, int chunk, int batch)
{
load_data:

    const unsigned length = round_up(channels * nSamples, VALUES_PER_WORD) / 1;
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
	 const unsigned channels,
	 const unsigned nSamples,
	   dma_info_t &store_ctrl, int chunk, int batch)
{
store_data:

    const unsigned length = round_up(channels * nSamples, VALUES_PER_WORD) / 1;
    const unsigned store_offset = round_up(channels * nSamples, VALUES_PER_WORD) * nBatches;
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
	 const unsigned channels,
	 const unsigned nSamples,
             word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{

    // TODO implement compute functionality

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
    /*
    const float m_fCos3Alpha = 1.19249e-08;
    const float m_fSin3Alpha = 1;
    const float m_fCos3Beta = 0.999995;
    const float m_fSin3Beta = -0.00318557;
    const float m_fCos3Gamma = 1.19249e-08;
    const float m_fSin3Gamma = -1;
    */ 

    const int kV = 0;
    const int kT = 1;
    const int kR = 2;
    const int kS = 3;
    const int kU = 4; 

    float m_pfTempSample[5] = {0};
    const float fSqrt3 = sqrt(3.f);

    for (int niSample = 0; niSample < nSamples; niSample++) {
        // Alpha rotation 
        m_pfTempSample[kV] = - _inbuff[kU*nSamples+niSample] * m_fSin2Alpha
                            + _inbuff[kV*nSamples+niSample] * m_fCos2Alpha;
        m_pfTempSample[kT] = - _inbuff[kS*nSamples+niSample] * m_fSinAlpha
                            + _inbuff[kT*nSamples+niSample] * m_fCosAlpha;
        m_pfTempSample[kR] = _inbuff[kR*nSamples+niSample];
        m_pfTempSample[kS] = _inbuff[kS*nSamples+niSample] * m_fCosAlpha
                             + _inbuff[kT*nSamples+niSample] * m_fSinAlpha;
        m_pfTempSample[kU] = _inbuff[kU*nSamples+niSample] * m_fCos2Alpha
                             + _inbuff[kV*nSamples+niSample] * m_fSin2Alpha;
        // Beta rotation
        _outbuff[kV*nSamples+niSample] = -m_fSinBeta * m_pfTempSample[kT]
                                + m_fCosBeta * m_pfTempSample[kV];
        _outbuff[kT*nSamples+niSample] = -m_fCosBeta * m_pfTempSample[kT]
                                + m_fSinBeta * m_pfTempSample[kV];
        _outbuff[kR*nSamples+niSample] = (0.75f * m_fCos2Beta + 0.25f) * m_pfTempSample[kR]
                                + (0.5 * fSqrt3 * pow(m_fSinBeta,2.0) ) * m_pfTempSample[kU]
                                + (fSqrt3 * m_fSinBeta * m_fCosBeta) * m_pfTempSample[kS];
        _outbuff[kS*nSamples+niSample] = m_fCos2Beta * m_pfTempSample[kS]
                                - fSqrt3 * m_fCosBeta * m_fSinBeta * m_pfTempSample[kR]
                                + m_fCosBeta * m_fSinBeta * m_pfTempSample[kU];
        _outbuff[kU*nSamples+niSample] = (0.25f * m_fCos2Beta + 0.75f) * m_pfTempSample[kU]
                                - m_fCosBeta * m_fSinBeta * m_pfTempSample[kS]
                                +0.5 * fSqrt3 * pow(m_fSinBeta,2.0) * m_pfTempSample[kR];

        // Gamma rotation
        m_pfTempSample[kV] = - _outbuff[kU*nSamples+niSample] * m_fSin2Gamma
                            + _outbuff[kV*nSamples+niSample] * m_fCos2Gamma;
        m_pfTempSample[kT] = - _outbuff[kS*nSamples+niSample] * m_fSinGamma
                             + _outbuff[kT*nSamples+niSample] * m_fCosGamma;

        m_pfTempSample[kR] = _outbuff[kR*nSamples+niSample];
        m_pfTempSample[kS] = _outbuff[kS*nSamples+niSample] * m_fCosGamma
                             + _outbuff[kT*nSamples+niSample] * m_fSinGamma;
        m_pfTempSample[kU] = _outbuff[kU*nSamples+niSample] * m_fCos2Gamma
                             + _outbuff[kV*nSamples+niSample] * m_fSin2Gamma;

        _outbuff[kR*nSamples+niSample] = m_pfTempSample[kR];
        _outbuff[kS*nSamples+niSample] = m_pfTempSample[kS];
        _outbuff[kT*nSamples+niSample] = m_pfTempSample[kT];
        _outbuff[kU*nSamples+niSample] = m_pfTempSample[kU];
        _outbuff[kV*nSamples+niSample] = m_pfTempSample[kV];

    }
}


void top(dma_word_t *out, dma_word_t *in1,
         /* <<--params-->> */
	 const unsigned conf_info_nBatches,
	 const unsigned conf_info_channels,
	 const unsigned conf_info_nSamples,
	 dma_info_t &load_ctrl, dma_info_t &store_ctrl)
{

    /* <<--local-params-->> */
	 const unsigned nBatches = conf_info_nBatches;
	 const unsigned channels = conf_info_channels;
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
	 	 channels,
	 	 nSamples,
                 load_ctrl, c, b);
            compute(_inbuff,
                    /* <<--args-->> */
	 	 nBatches,
	 	 channels,
	 	 nSamples,
                    _outbuff);
            store(_outbuff, out,
                  /* <<--args-->> */
	 	 nBatches,
	 	 channels,
	 	 nSamples,
                  store_ctrl, c, b);
        }
    }
}

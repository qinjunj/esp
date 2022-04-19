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

    // float m_pfTempSample[10] = {0};
    // float temp[10] = {0}; 
    const float fSqrt3 = sqrt(3.f);

    float temp[5*64] = {0};
    float temp2[5*64] = {0}; 
    compute_label2:for (int c = 0; c < 16; c++) {
        // int start = c*64;
        // int end = (c+1)*64; 
        int disp = c*64; 
        for (int n = 0; n < 64; n++) {
        #pragma HLS unroll
            temp[n] = - _inbuff[kU*nSamples+disp+n] * m_fSin2Alpha
                      + _inbuff[kV*nSamples+disp+n] * m_fCos2Alpha;
            temp[n+64] = - _inbuff[kS*nSamples+disp+n] * m_fSinAlpha
                         + _inbuff[kT*nSamples+disp+n] * m_fCosAlpha;
            temp[n+2*64] = _inbuff[kR*nSamples+disp+n];
            temp[n+3*64] = _inbuff[kS*nSamples+disp+n] * m_fCosAlpha
                             + _inbuff[kT*nSamples+disp+n] * m_fSinAlpha;
            temp[n+4*64] = _inbuff[kU*nSamples+disp+n] * m_fCos2Alpha
                             + _inbuff[kV*nSamples+disp+n] * m_fSin2Alpha;
        }
        for (int n = 0; n < 64; n++) {
        #pragma HLS unroll
            temp2[n] = -m_fSinBeta * temp[kT*64+n]
                       + m_fCosBeta * temp[kV*64+n];
            temp2[n+64] = -m_fCosBeta * temp[kT*64+n]
                                + m_fSinBeta * temp[kV*64+n];
            temp2[n+2*64] = (0.75f * m_fCos2Beta + 0.25f) * temp[kR*64+n]
                                + (0.5 * fSqrt3 * pow(m_fSinBeta,2.0) ) * temp[kU*64+n]
                                + (fSqrt3 * m_fSinBeta * m_fCosBeta) * temp[kS*64+n];
            temp2[n+3*64] = m_fCos2Beta * temp[kS*64+n]
                                - fSqrt3 * m_fCosBeta * m_fSinBeta * temp[kR*64+n]
                                + m_fCosBeta * m_fSinBeta * temp[kU*64+n];
            temp2[n+4*64] = (0.25f * m_fCos2Beta + 0.75f) * temp[kU*64+n]
                                - m_fCosBeta * m_fSinBeta * temp[kS*64+n]
                                +0.5 * fSqrt3 * pow(m_fSinBeta,2.0) * temp[kR*64+n];
        }
        for (int n = 0; n < 64; n++) {
        #pragma HLS unroll 
            _outbuff[kV*nSamples+disp+n] = - temp2[kU*64+n] * m_fSin2Gamma
                            + temp2[kV*64+n] * m_fCos2Gamma;
            _outbuff[kT*nSamples+disp+n] = - temp2[kS*64+n] * m_fSinGamma
                             + temp2[kT*64+n] * m_fCosGamma;

            _outbuff[kR*nSamples+disp+n] = temp2[kR*64+n];
            _outbuff[kS*nSamples+disp+n] = temp2[kS*64+n] * m_fCosGamma
                             + temp2[kT*64+n] * m_fSinGamma;
            _outbuff[kU*nSamples+disp+n] = temp2[kU*64+n] * m_fCos2Gamma
                             + temp2[kV*64+n] * m_fSin2Gamma;
        }
        
    }
/*
    compute_label2:for (int niSample = 0; niSample < nSamples; niSample++) {
        #pragma HLS unroll factor=2
        int s = niSample%2 == 0 ? 0 : 5; 
        // Alpha rotation 
        m_pfTempSample[0+s] = - _inbuff[kU*nSamples+niSample] * m_fSin2Alpha
                            + _inbuff[kV*nSamples+niSample] * m_fCos2Alpha;
        m_pfTempSample[1+s] = - _inbuff[kS*nSamples+niSample] * m_fSinAlpha
                            + _inbuff[kT*nSamples+niSample] * m_fCosAlpha;
        m_pfTempSample[2+s] = _inbuff[kR*nSamples+niSample];
        m_pfTempSample[3+s] = _inbuff[kS*nSamples+niSample] * m_fCosAlpha
                             + _inbuff[kT*nSamples+niSample] * m_fSinAlpha;
        m_pfTempSample[4+s] = _inbuff[kU*nSamples+niSample] * m_fCos2Alpha
                             + _inbuff[kV*nSamples+niSample] * m_fSin2Alpha;
        // Beta rotation
        temp[0] = -m_fSinBeta * m_pfTempSample[1]
                                + m_fCosBeta * m_pfTempSample[0];
        temp[1] = -m_fCosBeta * m_pfTempSample[1]
                                + m_fSinBeta * m_pfTempSample[0];
        temp[2] = (0.75f * m_fCos2Beta + 0.25f) * m_pfTempSample[2]
                                + (0.5 * fSqrt3 * pow(m_fSinBeta,2.0) ) * m_pfTempSample[4]
                                + (fSqrt3 * m_fSinBeta * m_fCosBeta) * m_pfTempSample[3];
        temp[3] = m_fCos2Beta * m_pfTempSample[3]
                                - fSqrt3 * m_fCosBeta * m_fSinBeta * m_pfTempSample[2]
                                + m_fCosBeta * m_fSinBeta * m_pfTempSample[4];
        temp[4] = (0.25f * m_fCos2Beta + 0.75f) * m_pfTempSample[4]
                                - m_fCosBeta * m_fSinBeta * m_pfTempSample[3]
                                +0.5 * fSqrt3 * pow(m_fSinBeta,2.0) * m_pfTempSample[2];

        // Gamma rotation
        _outbuff[0] = - temp[4] * m_fSin2Gamma
                            + temp[0] * m_fCos2Gamma;
        _outbuff[1] = - temp[3] * m_fSinGamma
                             + temp[1] * m_fCosGamma;

        _outbuff[2] = temp[2];
        _outbuff[3] = temp[3] * m_fCosGamma
                             + temp[1] * m_fSinGamma;
        _outbuff[4] = temp[4] * m_fCos2Gamma
                             + temp[0] * m_fSin2Gamma;
*/
/*
        _outbuff[kR*nSamples+niSample] = m_pfTempSample[kR*nSamples+niSample];
        _outbuff[kS*nSamples+niSample] = m_pfTempSample[kS*nSamples+niSample];
        _outbuff[kT*nSamples+niSample] = m_pfTempSample[kT*nSamples+niSample];
        _outbuff[kU*nSamples+niSample] = m_pfTempSample[kU*nSamples+niSample];
        _outbuff[kV*nSamples+niSample] = m_pfTempSample[kV*nSamples+niSample];
*/

//    }
    
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

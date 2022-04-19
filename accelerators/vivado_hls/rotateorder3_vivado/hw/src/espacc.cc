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

    const unsigned length = round_up(nSamples, VALUES_PER_WORD) / 1;
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

    const unsigned length = round_up(nSamples, VALUES_PER_WORD) / 1;
    const unsigned store_offset = round_up(nSamples, VALUES_PER_WORD) * 1;
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
     const unsigned nBatches,
	 const unsigned nChannels,
	 const unsigned nSamples,
             word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{
    
    const unsigned length = round_up(nSamples, VALUES_PER_WORD) / 1;
    const unsigned kQ = 0, kO = 1, kM = 2, kK = 3, kL = 4, kN = 5, kP = 6;
    
    word_t m_pfTempSample_kQ, m_pfTempSample_kO, m_pfTempSample_kM, m_pfTempSample_kK, m_pfTempSample_kL, m_pfTempSample_kN, m_pfTempSample_kP;
    word_t pBFSrcDst_m_ppfChannels_kQ, pBFSrcDst_m_ppfChannels_kO, pBFSrcDst_m_ppfChannels_kM, pBFSrcDst_m_ppfChannels_kK, 
           pBFSrcDst_m_ppfChannels_kL, pBFSrcDst_m_ppfChannels_kN, pBFSrcDst_m_ppfChannels_kP;
    
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
    
    // row major access
    #define IND(row,col) ((row)*nSamples+(col))

    for(unsigned niSample = 0; niSample < nSamples; niSample++)
    {
        // Alpha rotation
        m_pfTempSample_kQ = - _inbuff[IND(kP,niSample)] * m_fSin3Alpha
                            + _inbuff[IND(kQ,niSample)] * m_fCos3Alpha;
        
        m_pfTempSample_kO = - _inbuff[IND(kN,niSample)] * m_fSin2Alpha
                            + _inbuff[IND(kO,niSample)] * m_fCos2Alpha;
        m_pfTempSample_kM = - _inbuff[IND(kL,niSample)] * m_fSinAlpha
                            + _inbuff[IND(kM,niSample)] * m_fCosAlpha;
        m_pfTempSample_kK = _inbuff[IND(kK,niSample)];
        m_pfTempSample_kL = _inbuff[IND(kL,niSample)] * m_fCosAlpha
                            + _inbuff[IND(kM,niSample)] * m_fSinAlpha;
        m_pfTempSample_kN = _inbuff[IND(kN,niSample)] * m_fCos2Alpha
                            + _inbuff[IND(kO,niSample)] * m_fSin2Alpha;
        m_pfTempSample_kP = _inbuff[IND(kP,niSample)] * m_fCos3Alpha
                            + _inbuff[IND(kQ,niSample)] * m_fSin3Alpha;
        
        // Beta rotation
        pBFSrcDst_m_ppfChannels_kQ= 0.125f * m_pfTempSample_kQ * (5.f + 3.f*m_fCos2Beta)
                    - fSqrt3_2 * m_pfTempSample_kO *m_fCosBeta * m_fSinBeta
                    + 0.25f * fSqrt15 * m_pfTempSample_kM * pow(m_fSinBeta,2.0f);
        pBFSrcDst_m_ppfChannels_kO = m_pfTempSample_kO * m_fCos2Beta
                    - fSqrt5_2 * m_pfTempSample_kM * m_fCosBeta * m_fSinBeta
                    + fSqrt3_2 * m_pfTempSample_kQ * m_fCosBeta * m_fSinBeta;
        pBFSrcDst_m_ppfChannels_kM = 0.125f * m_pfTempSample_kM * (3.f + 5.f*m_fCos2Beta)
                    - fSqrt5_2 * m_pfTempSample_kO *m_fCosBeta * m_fSinBeta
                    + 0.25f * fSqrt15 * m_pfTempSample_kQ * pow(m_fSinBeta,2.0f);
        pBFSrcDst_m_ppfChannels_kK = 0.25f * m_pfTempSample_kK * m_fCosBeta * (-1.f + 15.f*m_fCos2Beta)
                    + 0.5f * fSqrt15 * m_pfTempSample_kN * m_fCosBeta * pow(m_fSinBeta,2.f)
                    + 0.5f * fSqrt5_2 * m_pfTempSample_kP * pow(m_fSinBeta,3.f)
                    + 0.125f * fSqrt3_2 * m_pfTempSample_kL * (m_fSinBeta + 5.f * m_fSin3Beta);
        pBFSrcDst_m_ppfChannels_kL = 0.0625f * m_pfTempSample_kL * (m_fCosBeta + 15.f * m_fCos3Beta)
                    + 0.25f * fSqrt5_2 * m_pfTempSample_kN * (1.f + 3.f * m_fCos2Beta) * m_fSinBeta
                    + 0.25f * fSqrt15 * m_pfTempSample_kP * m_fCosBeta * pow(m_fSinBeta,2.f)
                    - 0.125 * fSqrt3_2 * m_pfTempSample_kK * (m_fSinBeta + 5.f * m_fSin3Beta);
        pBFSrcDst_m_ppfChannels_kN = 0.125f * m_pfTempSample_kN * (5.f * m_fCosBeta + 3.f * m_fCos3Beta)
                    + 0.25f * fSqrt3_2 * m_pfTempSample_kP * (3.f + m_fCos2Beta) * m_fSinBeta
                    + 0.5f * fSqrt15 * m_pfTempSample_kK * m_fCosBeta * pow(m_fSinBeta,2.f)
                    + 0.125 * fSqrt5_2 * m_pfTempSample_kL * (m_fSinBeta - 3.f * m_fSin3Beta);
        pBFSrcDst_m_ppfChannels_kP = 0.0625f * m_pfTempSample_kP * (15.f * m_fCosBeta + m_fCos3Beta)
                    - 0.25f * fSqrt3_2 * m_pfTempSample_kN * (3.f + m_fCos2Beta) * m_fSinBeta
                    + 0.25f * fSqrt15 * m_pfTempSample_kL * m_fCosBeta * pow(m_fSinBeta,2.f)
                    - 0.5 * fSqrt5_2 * m_pfTempSample_kK * pow(m_fSinBeta,3.f);
        
        // Gamma rotation
        _outbuff[IND(kQ, niSample)] = - pBFSrcDst_m_ppfChannels_kP * m_fSin3Gamma
                            + pBFSrcDst_m_ppfChannels_kQ * m_fCos3Gamma;
        _outbuff[IND(kO, niSample)] = - pBFSrcDst_m_ppfChannels_kN * m_fSin2Gamma
                            + pBFSrcDst_m_ppfChannels_kO * m_fCos2Gamma;
        _outbuff[IND(kM, niSample)] = - pBFSrcDst_m_ppfChannels_kL * m_fSinGamma
                            + pBFSrcDst_m_ppfChannels_kM * m_fCosGamma;
        _outbuff[IND(kK, niSample)] = pBFSrcDst_m_ppfChannels_kK;
        _outbuff[IND(kL, niSample)] = pBFSrcDst_m_ppfChannels_kL * m_fCosGamma
                            + pBFSrcDst_m_ppfChannels_kM * m_fSinGamma;
        _outbuff[IND(kN, niSample)] = pBFSrcDst_m_ppfChannels_kN * m_fCos2Gamma
                            + pBFSrcDst_m_ppfChannels_kO * m_fSin2Gamma;
        _outbuff[IND(kP, niSample)] = pBFSrcDst_m_ppfChannels_kP * m_fCos3Gamma
                            + pBFSrcDst_m_ppfChannels_kQ * m_fSin3Gamma;
        
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

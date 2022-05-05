// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>

void load(word_t _inbuff0[1024], word_t _inbuff1[1024], word_t _inbuff2[1024], word_t _inbuff3[1024], word_t _inbuff4[1024], dma_word_t *in1,
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
    
    unsigned words_per_channel = 1024/VALUES_PER_WORD;
    unsigned offset2 =  words_per_channel * 2;
    unsigned offset3 =  words_per_channel * 3;
    unsigned offset4 =  words_per_channel * 4;

    for (unsigned k = 0; k < words_per_channel; k++) {
        load_label0:for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
                        _inbuff0[k * VALUES_PER_WORD + j] = in1[dma_index + k].word[j];
                        _inbuff1[k * VALUES_PER_WORD + j] = in1[dma_index + k + words_per_channel].word[j];
                        _inbuff2[k * VALUES_PER_WORD + j] = in1[dma_index + k + offset2].word[j];
                        _inbuff3[k * VALUES_PER_WORD + j] = in1[dma_index + k + offset3].word[j];
                        _inbuff4[k * VALUES_PER_WORD + j] = in1[dma_index + k + offset4].word[j];
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


void compute(word_t _inbuff0[1024], word_t _inbuff1[1024], word_t _inbuff2[1024], word_t _inbuff3[1024], word_t _inbuff4[1024],
             /* <<--compute-params-->> */
     const unsigned nBatches,
     const unsigned channels,
     const unsigned nSamples,
             word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{

    // TODO implement compute functionality

    const word_t m_fCosAlpha = -4.37114e-08;
    const word_t m_fSinAlpha = -1;
    const word_t m_fCosBeta = -0.49908;
    const word_t m_fSinBeta = 0.866556;
    const word_t m_fCosGamma = -4.37114e-08;
    const word_t m_fSinGamma = 1;
    const word_t m_fCos2Alpha = -1;
    const word_t m_fSin2Alpha = 8.74228e-08;
    const word_t m_fCos2Beta = -0.501838;
    const word_t m_fSin2Beta = -0.864962;
    const word_t m_fCos2Gamma = -1;
    const word_t m_fSin2Gamma = -8.74228e-08;
    /*
    const float m_fCos3Alpha = 1.19249e-08;
    const float m_fSin3Alpha = 1;
    const float m_fCos3Beta = 0.999995;
    const float m_fSin3Beta = -0.00318557;
    const float m_fCos3Gamma = 1.19249e-08;
    const float m_fSin3Gamma = -1;
    */
/*
    const int kV = 0;
    const int kT = 1;
    const int kR = 2;
    const int kS = 3;
    const int kU = 4;
*/

    // float m_pfTempSample[10] = {0};
    // float temp[10] = {0};
    const word_t fSqrt3 = sqrt(3.f);
/*
    compute_label2:for (int n = 0; n < nSamples; n++) {
    // #pragma HLS unroll
        _outbuff[kV*nSamples+n] = _inbuff[kV*nSamples+n] * fSqrt3;
            _outbuff[kT*nSamples+n] = _inbuff[kT*nSamples+n] * fSqrt3;
            _outbuff[kR*nSamples+n] = _inbuff[kR*nSamples+n] * fSqrt3;
            _outbuff[kS*nSamples+n] = _inbuff[kS*nSamples+n] * fSqrt3;
            _outbuff[kU*nSamples+n] = _inbuff[kU*nSamples+n] * fSqrt3;
    }
*/
    
    word_t temp0[64] = {0};
    word_t temp1[64] = {0};
    word_t temp2[64] = {0};
    word_t temp3[64] = {0};
    word_t temp4[64] = {0};

    word_t t0[64] = {0};
    word_t t1[64] = {0};
    word_t t2[64] = {0};
    word_t t3[64] = {0};
    word_t t4[64] = {0};

    // for buffering _inbuff
    word_t buffer0[64] = {0};
    word_t buffer1[64] = {0};
    word_t buffer2[64] = {0};
    word_t buffer3[64] = {0};
    word_t buffer4[64] = {0};

    compute_label2:for (int c = 0; c < 16; c++) {
        // #pragma HLS pipeline II = 1
        int disp = c*64;
        for (int n = 0; n < 64; n++) {
            buffer0[n] = _inbuff0[disp+n];
            buffer1[n] = _inbuff1[disp+n];
            buffer2[n] = _inbuff2[disp+n];
            buffer3[n] = _inbuff3[disp+n];
            buffer4[n] = _inbuff4[disp+n];
        }
        for (int n = 0; n < 64; n++) {
        #pragma HLS unroll
            temp0[n] = - buffer4[n] * m_fSin2Alpha
                      + buffer0[n] * m_fCos2Alpha;
            temp1[n] = - buffer3[n] * m_fSinAlpha
                       + buffer1[n] * m_fCosAlpha;
            temp2[n] = buffer2[n];
            temp3[n] = buffer3[n] * m_fCosAlpha
                       + buffer1[n] * m_fSinAlpha;
            temp4[n] = buffer4[n] * m_fCos2Alpha
                       + buffer0[n] * m_fSin2Alpha;
        }
        for (int n = 0; n < 64; n++) {
        #pragma HLS unroll
            t0[n] = -m_fSinBeta * temp1[n]
                       + m_fCosBeta * temp0[n];
            t1[n] = -m_fCosBeta * temp1[n]
                          + m_fSinBeta * temp0[n];
            t2[n] = (0.75f * m_fCos2Beta + 0.25f) * temp2[n]
                            + (0.5 * fSqrt3 * pow(m_fSinBeta,2.0) ) * temp4[n]
                            + (fSqrt3 * m_fSinBeta * m_fCosBeta) * temp3[n];
            t3[n] = m_fCos2Beta * temp3[n]
                            - fSqrt3 * m_fCosBeta * m_fSinBeta * temp2[n]
                            + m_fCosBeta * m_fSinBeta * temp4[n];
            t4[n] = (0.25f * m_fCos2Beta + 0.75f) * temp4[n]
                            - m_fCosBeta * m_fSinBeta * temp3[n]
                            + 0.5 * fSqrt3 * pow(m_fSinBeta,2.0) * temp2[n];
        }
        for (int n = 0; n < 64; n++) {
        #pragma HLS unroll
            _outbuff[disp+n] = - t4[n] * m_fSin2Gamma
                               + t0[n] * m_fCos2Gamma;
            _outbuff[1024+disp+n] = - t3[n] * m_fSinGamma
                                    + t1[n] * m_fCosGamma;
            _outbuff[2048+disp+n] = t2[n];
            _outbuff[3072+disp+n] = t3[n] * m_fCosGamma
                                    + t1[n] * m_fSinGamma;
            _outbuff[4096+disp+n] = t4[n] * m_fCos2Gamma
                                    + t0[n] * m_fSin2Gamma;
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
    for (unsigned b = 0; b < 1; b++)
    {
        // Chunking
    go:
        for (int c = 0; c < nBatches; c++)
        {
            // word_t _inbuff[SIZE_IN_CHUNK_DATA];
            word_t _outbuff[SIZE_OUT_CHUNK_DATA];

            word_t _inbuff0[1024];
            word_t _inbuff1[1024];
            word_t _inbuff2[1024];
            word_t _inbuff3[1024];
            word_t _inbuff4[1024];

            word_t _outbuff0[1024];
            word_t _outbuff1[1024];
            word_t _outbuff2[1024];
            word_t _outbuff3[1024];
            word_t _outbuff4[1024];

            #pragma HLS dataflow
            load(_inbuff0, _inbuff1, _inbuff2, _inbuff3, _inbuff4, in1, nBatches, channels, nSamples, load_ctrl, c, b);
            compute(_inbuff0, _inbuff1, _inbuff2, _inbuff3, _inbuff4, nBatches, channels, nSamples, _outbuff);
            store(_outbuff, out, nBatches, channels, nSamples, store_ctrl, c, b);
        }
    }
}

#ifndef DSP_MFCC_FEATURE_H
#define DSP_MFCC_FEATURE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ====================================================================================================
 * CONST Parameter
 * ==================================================================================================== */
#define DSP_MFCC_FRAME_SIZE  400
#define DSP_MFCC_FFT_SIZE    512
#define DSP_MFCC_MAG_SIZE    256
#define DSP_MFCC_MEL_SIZE    40
#define DSP_MFCC_COEF_SIZE   13
#define DSP_MFCC_COEF_FRAME  98
#define DSP_MFCC_HOP_SIZE    160

/* ====================================================================================================
 * DSP mfcc init
 * ==================================================================================================== */
int dsp_mfcc_init();

/* ====================================================================================================
 * Process One Frame : 400 bin -> 512 complex;
 * ==================================================================================================== */
int dsp_mfcc_fft_process(int16_t *input_pcm, float *output_fft);

/* ====================================================================================================
 * Process One Frame : 400 bin -> 13 coef;
 * ==================================================================================================== */
int dsp_mfcc_frame_process(int16_t *input_pcm, float *output_coef);

#ifdef __cplusplus
}
#endif

#endif /* DSP_MFCC_FEATURE_H */
#ifndef DSP_MFCC_MEL_H
#define DSP_MFCC_MEL_H

#include <stdint.h>

typedef struct
{
    uint16_t start_bin;
    uint16_t length;
    const float *weights;
} dsp_mel_filter_t;

extern const dsp_mel_filter_t dsp_mel_filters[40];

#endif

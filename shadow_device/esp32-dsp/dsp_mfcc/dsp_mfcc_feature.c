#include <stddef.h>
#include <string.h>
#include <math.h>
#include "dsps_fft2r.h"
#include "dsp_mfcc_feature.h"
#include "dsp_mfcc_hann.h"
#include "dsp_mfcc_mel.h"
#include "dsp_mfcc_dct.h"
#include "esp_fake.h"

static const char *TAG = "DSP_MFCC_PIPELINE";

__attribute__((aligned(16))) float mfcc_fft_data[DSP_MFCC_FFT_SIZE * 2];
static float mfcc_mag_output[DSP_MFCC_MAG_SIZE];
static float mfcc_mel_output[DSP_MFCC_MEL_SIZE];

int dsp_mfcc_init()
{
	int ret = dsps_fft2r_init_fc32(NULL, DSP_MFCC_FFT_SIZE);
    if (ret  != ESP_OK)
    {
        ESP_LOGE(TAG, "Not possible to initialize FFT2R. Error = %i", ret);
    }
    return ret;
}

int dsp_mfcc_frame_process(int16_t *input_pcm, float *output_coef)
{
    memset(mfcc_fft_data, 0, sizeof(mfcc_fft_data));
    for (int i = 0; i < DSP_MFCC_FRAME_SIZE; i++)
    {
        mfcc_fft_data[2*i] = (float)input_pcm[i] * hanning_window[i];
    }
    dsps_fft2r_fc32(mfcc_fft_data, DSP_MFCC_FFT_SIZE);
    dsps_bit_rev_fc32(mfcc_fft_data, DSP_MFCC_FFT_SIZE);

    for (int k = 0; k < DSP_MFCC_MAG_SIZE; k++)
    {
        mfcc_mag_output[k] = (mfcc_fft_data[2*k] * mfcc_fft_data[2*k] + mfcc_fft_data[2*k+1] * mfcc_fft_data[2*k+1]) / DSP_MFCC_FFT_SIZE;
    }
/*
    for (int m = 0; m < DSP_MFCC_MEL_SIZE; m++)
    {
        float sum = 0.0f;
        for (int k = 0; k < DSP_MFCC_MAG_SIZE; k++)
            sum += mfcc_mag_output[k] * mel_filterbank[m][k];
        mfcc_mel_output[m] = logf(sum + 1e-10f); // log-mel
    }
*/
    for (int m = 0; m < DSP_MFCC_MEL_SIZE; m++)
    {
        const dsp_mel_filter_t *f = &dsp_mel_filters[m];
        const float *w = f->weights;

        float sum = 0.0f;
        for (int i = 0; i < f->length; i++)
        {
            sum += mfcc_mag_output[f->start_bin + i] * w[i];
        }
        mfcc_mel_output[m] = logf(sum + 1e-10f);
    }

    for (int k = 0; k < DSP_MFCC_COEF_SIZE; k++)
    {
        float sum = 0.0f;        
        for (int n = 0; n < DSP_MFCC_MEL_SIZE; n++)
        {
            sum += mfcc_mel_output[n] * dct_cosine_table[k][n];
        }
        output_coef[k] = sum * ((k == 0) ? DSP_DCT_SCALE_0 : DSP_DCT_SCALE_N );
    }
    return 0;
}

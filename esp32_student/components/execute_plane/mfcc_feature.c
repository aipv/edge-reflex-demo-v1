#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_dsp.h"
#include "math.h"
#include "mel_filterbank.h"
#include "mfcc_feature.h"
#include "test_mfcc_feature.h"

static const char *TAG = "MFCC_FEATURE";

#define N_DCT MFCC_MEL_LENGTH

static float dct_cosine_table[MFCC_MEL_LENGTH][MFCC_MEL_LENGTH]; 
static float dct_scale_factors[MFCC_MEL_LENGTH]; 
static float mfcc_han_window[MFCC_N_FFT_SIZE];
static float mfcc_mel_output[MFCC_MEL_LENGTH];
static float mfcc_dct_output[MFCC_MEL_LENGTH];
static float mfcc_mag_output[MFCC_MAG_LENGTH];
static float mfcc_coef_output[MFCC_COEF_COUNT];
__attribute__((aligned(16))) float mfcc_fft_data[MFCC_FFT_BUFFER];

static void mfcc_init_dct_tables() {
    float inv_sqrt_n = 1.0f / sqrtf((float)N_DCT);
    float sqrt_2_n = sqrtf(2.0f / (float)N_DCT);

    // 计算正交归一化因子 c_k
    for (int k = 0; k < N_DCT; k++) {
        if (k == 0) {
            // C[0] 的 Ortho 缩放因子：sqrt(1/N)
            dct_scale_factors[k] = inv_sqrt_n; 
        } else {
            // C[k>0] 的 Ortho 缩放因子：sqrt(2/N)
            dct_scale_factors[k] = sqrt_2_n;
        }
    }

    // 生成余弦表 (cos(pi * k * (n + 0.5) / N))
    for (int k = 0; k < N_DCT; k++) {
        for (int n = 0; n < N_DCT; n++) {
            float arg = PI * k * (n + 0.5f) / (float)N_DCT;
            dct_cosine_table[k][n] = cosf(arg);
        }
    }
}

static void mfcc_calculate_dct_type2(const float *input_data, float *output_coef)
{
    for (int k = 0; k < N_DCT; k++)
    {
        float sum = 0.0f;        
        for (int n = 0; n < N_DCT; n++)
        {
            sum += input_data[n] * dct_cosine_table[k][n];
        }
        output_coef[k] = sum * dct_scale_factors[k];
    }
}

esp_err_t mfcc_feature_init()
{
    mfcc_init_dct_tables();
    dsps_wind_hann_f32(mfcc_han_window, MFCC_N_FFT_SIZE);
    int ret = dsps_fft2r_init_fc32(NULL, MFCC_N_FFT_SIZE);
    if (ret  != ESP_OK)
    {
        ESP_LOGE(TAG, "Not possible to initialize FFT2R. Error = %i", ret);
    }
    return ret;
}

esp_err_t mfcc_feature_frame_process(int16_t *input_pcm, float *output_coef)
{
    memset(mfcc_fft_data, 0, MFCC_FFT_BUFSIZE);
    for (int i = 0; i < MFCC_FRAME_SIZE; i++)
    {
        mfcc_fft_data[2*i] = (float)input_pcm[i] * mfcc_han_window[i];
    }

    dsps_fft2r_fc32(mfcc_fft_data, MFCC_N_FFT_SIZE);
    dsps_bit_rev_fc32(mfcc_fft_data, MFCC_N_FFT_SIZE);

    for (int k = 0; k < MFCC_MAG_LENGTH; k++)
    {
        mfcc_mag_output[k] = (mfcc_fft_data[2*k] * mfcc_fft_data[2*k] + mfcc_fft_data[2*k+1] * mfcc_fft_data[2*k+1]) / MFCC_N_FFT_SIZE;
    }

    for (int m = 0; m < MFCC_MEL_LENGTH; m++)
    {
        float sum = 0.0f;
        for (int k = 0; k < MFCC_MAG_LENGTH; k++)
            sum += mfcc_mag_output[k] * mel_filterbank[m][k];
        mfcc_mel_output[m] = logf(sum + 1e-6f); // log-mel
    }
    mfcc_calculate_dct_type2(mfcc_mel_output, mfcc_dct_output);

    for (int j = 0; j < MFCC_COEF_COUNT; j++)
    {
        output_coef[j] = mfcc_dct_output[j]; 
    }
    return ESP_OK;
}


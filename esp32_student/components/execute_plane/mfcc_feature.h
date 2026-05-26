#ifndef MFCC_FEATURE_H
#define MFCC_FEATURE_H

#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MFCC_FFT_BUFSIZE            4096
#define MFCC_FFT_BUFFER             1024
#define MFCC_N_FFT_SIZE             512
#define MFCC_FRAME_SIZE             400
#define MFCC_MAG_LENGTH             256
#define MFCC_HOP_LENGTH             160
#define MFCC_MEL_LENGTH             40
#define MFCC_COEF_COUNT             13
#define MFCC_COEF_FRAME             98
#define MFCC_COEF_TOTAL             1274

#ifndef PI 
#define PI 3.14159265358979f
#endif

esp_err_t mfcc_feature_init(void);
esp_err_t mfcc_feature_frame_process(int16_t *input_pcm, float *output_coef);

#endif // MFCC_FEATURE_H
#ifndef TEST_MFCC_FEATURE_H
#define TEST_MFCC_FEATURE_H

#include <stdio.h>
#include "esp_log.h"

void print_float_40(float *values);
void print_float_256(float *values);
void print_float_512(float *values);
void print_int16_512(int16_t *values);
void print_int8_1274(int8_t *values);
void test_mfcc_process(void);
void test_tflite_process(void);
int test_pcm_voice_predict(int16_t *input_pcm);

#endif // TEST_MFCC_FEATURE_H
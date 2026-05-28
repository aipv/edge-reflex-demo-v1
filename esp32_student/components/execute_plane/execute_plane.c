#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2s_audio.h"
#include "gpio_button.h"
#include "network_socket.h"
#include "support_plane.h"
#include "observe_plane.h"
#include "execute_plane.h"
#include "dsp_mfcc_feature.h"
#include "test_mfcc_feature.h"

static const char *TAG = "EXECUTE_PLANE";

static size_t count = 16000;
static char data_buffer[64000];
static char send_buffer[37096];
static int32_t *pcm_data = (int32_t *)(data_buffer);
static int16_t *pcm16_data = (int16_t *)(send_buffer);

typedef struct {
    int32_t    count;
    float      min;
    float      max;
    float      sum;
    double     sum2;
} CoefStatistic;
CoefStatistic   Counter[13];

void counter_init()
{
    for (int i = 0; i < 13; i++)
    {
        Counter[i].count = 0;
        Counter[i].min = 0;
        Counter[i].max = 0;
        Counter[i].sum = 0;
        Counter[i].sum2 = 0;
    }
}

void counter_update(float *data)
{
    for (int i = 0; i < 98; i++)
    {
        for (int j = 0; j < 13; j++)
        {
            float coef = data[i * 13 + j];
            Counter[j].min = (coef < Counter[j].min) ? coef : Counter[j].min;
            Counter[j].max = (coef > Counter[j].max) ? coef : Counter[j].max;
            Counter[j].sum += coef;
            Counter[j].sum2 += coef * coef;
            Counter[j].count++;
        }
    }
}

void counter_print()
{
    for (int i = 0; i < 13; i++)
    {
        ESP_LOGI(TAG, "%d : %d, %f, %f, %f, %lf", i, Counter[i].count, Counter[i].min, Counter[i].max, Counter[i].sum, Counter[i].sum2);
    }
}

esp_err_t pcm16_mfcc_preprocess(int16_t *input_pcm, int count)
{
    int16_t *iptr = input_pcm;
    float *optr = (float *)(input_pcm + count);

    for (int i = 0; i < DSP_MFCC_COEF_FRAME; i++)
    {
        dsp_mfcc_frame_process(iptr, optr);
        iptr += DSP_MFCC_HOP_SIZE;
        optr += DSP_MFCC_COEF_SIZE;
    }
    return ESP_OK;
}

void application_button_boot_callback(uint8_t gpio_num)
{
    ESP_LOGW(TAG, ">>> Button Boot (GPIO %d) Pressed! - Executing action A.", gpio_num);
    i2s_audio_read_data(pcm_data, count);
    ESP_LOGI(TAG, "Success read %d samples!", count);
    i2s_audio_play_data(pcm_data, count);
    ESP_LOGI(TAG, "Success play %d samples!", count);
    i2s_audio_convert_data(pcm_data, pcm16_data, count);
    ESP_LOGI(TAG, "Success convert %d samples!", count);

    pcm16_mfcc_preprocess(pcm16_data, count);
    ESP_LOGI(TAG, "Success MFCC feature %d samples!", count);

    network_socket_data_publish(pcm16_data, 37096);
    ESP_LOGI(TAG, "Success Publish feature %d samples!", count);
}

void application_button_up_callback(uint8_t gpio_num)
{
    ESP_LOGW(TAG, ">>> Button Up (GPIO %d) Pressed! - Executing action B.", gpio_num);
    counter_init();
    counter_print();
    if (network_socket_init() < 0)
    {
        ESP_LOGE(TAG, "Failed to connect to host.");
        return;
    }
    for (int i = 0; i < 600; i++)
    {
        i2s_audio_read_data(pcm_data, count);
        i2s_audio_convert_data(pcm_data, pcm16_data, count);
        pcm16_mfcc_preprocess(pcm16_data, count);
        counter_update((float *)(pcm16_data + count));
        if (((i + 1) % 60) == 0) counter_print();
        int bytes_sent = network_socket_send(pcm16_data, 37096);
        if (bytes_sent != 37096)
        {
            ESP_LOGE(TAG, "Transmission failed or incomplete!");
        }
    }

    network_socket_close();
    counter_print();
    ESP_LOGI(TAG, "Success Stream 100 seconds data!");
}

void application_button_down_callback(uint8_t gpio_num)
{
    ESP_LOGW(TAG, ">>> Button Down (GPIO %d) Pressed! - Executing action C.", gpio_num);
    i2s_audio_stop_stream();
}

esp_err_t execute_plane_init(void)
{
    gpio_button_set_callback_func(0, application_button_boot_callback);
    gpio_button_set_callback_func(1, application_button_up_callback);
    gpio_button_set_callback_func(2, application_button_down_callback);

    test_mfcc_process();
    return ESP_OK;
}

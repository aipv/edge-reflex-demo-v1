#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2s_audio.h"
#include "gpio_button.h"
#include "network_socket.h"
#include "support_plane.h"
#include "observe_plane.h"
#include "execute_plane.h"
#include "mfcc_feature.h"
#include "test_mfcc_feature.h"

static const char *TAG = "EXECUTE_PLANE";

static size_t count = 16000;
static char data_buffer[64000];
static char send_buffer[37096];
static int32_t *pcm_data = (int32_t *)(data_buffer);
static int16_t *pcm16_data = (int16_t *)(send_buffer);

esp_err_t pcm16_mfcc_preprocess(int16_t *input_pcm, int count)
{
    int16_t *iptr = input_pcm;
    float *optr = (float *)(input_pcm + count);

    for (int i = 0; i < MFCC_COEF_FRAME; i++)
    {
        mfcc_feature_frame_process(iptr, optr);
        iptr += MFCC_HOP_LENGTH;
        optr += MFCC_COEF_COUNT;
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
    i2s_audio_stream_data(1);
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

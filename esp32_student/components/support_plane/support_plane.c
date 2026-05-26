#include "support_plane.h"

static const char *TAG = "MAIN";

static void check_esp_err(esp_err_t err, const char* msg)
{
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s failed: %s (0x%x)", msg, esp_err_to_name(err), err);
        abort();
    }
}

void support_plane_init(void)
{
    check_esp_err(gpio_button_init(), "gpio_button_init()");
    check_esp_err(i2s_audio_mic_init(), "i2s_audio_mic_init()");
    check_esp_err(i2s_audio_spk_init(), "i2s_audio_spk_init()");
    check_esp_err(wifi_station_init(), "wifi_station_init()");

    check_esp_err(gpio_button_start(), "gpio_button_start()");
    mqtt_client_start();
}
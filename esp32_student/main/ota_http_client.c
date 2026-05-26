#include "esp_crt_bundle.h" 
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "esp_log.h"

static const char *TAG = "EFV_HTTP_OTA";

esp_err_t ota_start_http_update(const char *url)
{
    esp_http_client_config_t config = {
        .url = url,

        .skip_cert_common_name_check = true,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    ESP_LOGI(TAG, "Starting OTA from: %s", url);

    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA success, restarting...");
        esp_restart();
    } else {
        ESP_LOGE(TAG, "OTA failed: %s", esp_err_to_name(ret));
    }

    return ret;
}

void ota_print_running_partition(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    if (running) {
        ESP_LOGI(TAG, "Running partition: %s, offset: 0x%lx",
                 running->label, running->address);
    }
}

void ota_confirm_after_start(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t state;

    if (esp_ota_get_state_partition(running, &state) == ESP_OK) {
        if (state == ESP_OTA_IMG_PENDING_VERIFY) {
            ESP_LOGI(TAG, "OTA pending verify, marking valid...");

            esp_err_t err = esp_ota_mark_app_valid_cancel_rollback();
            if (err == ESP_OK) {
                ESP_LOGI(TAG, "OTA marked valid");
            } else {
                ESP_LOGE(TAG, "Failed to mark OTA valid: %s", esp_err_to_name(err));
            }
        } else {
            ESP_LOGI(TAG, "OTA state=%d, no need to confirm", state);
        }
    }
}

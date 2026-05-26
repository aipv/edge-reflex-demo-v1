#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "network_socket.h"
#include "observe_plane.h"

static const char *TAG = "OBSERVE_PLANE";

esp_err_t observe_plane_init(void)
{
    return ESP_OK;
}

esp_err_t observe_plane_data_report(void *data, int data_type, int data_count)
{
    if (data_type == OBSERVE_DATA_PCM16)
    {
        network_socket_data_publish(data, data_count * OBSERVE_PCM16_DATA_SIZE);
    }
    ESP_LOGI(TAG, "Success observe_plane_data_report(%d, %d)", data_type, data_count);
    return ESP_OK;
}

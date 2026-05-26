/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "wifi_station.h"
#include "mqtt_client_app.h"
#include "ota_http_client.h"

static const char *TAG = "MAIN_APP";

void app_main(void)
{
    ota_confirm_after_start();

    wifi_station_init();
    ESP_LOGI(TAG, ">>> End of initiation, starting tasks...");

    mqtt_client_start();
    ESP_LOGI(TAG, ">>> End of starting tasks, system running...");

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

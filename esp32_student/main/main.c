/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "support_plane.h"
#include "execute_plane.h"

static const char *TAG = "MAIN_APP";

void app_main(void)
{
    support_plane_init();
    ESP_LOGI(TAG, "support_plane_init() Done!");

    execute_plane_init();
    ESP_LOGI(TAG, "support_plane_init() Done!");

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

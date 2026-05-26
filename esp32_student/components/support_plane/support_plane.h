#ifndef SUPPORT_PLANE_H
#define SUPPORT_PLANE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2s_audio.h"
#include "gpio_button.h"
#include "wifi_station.h"
#include "network_socket.h"
#include "mqtt_client_app.h"
#include "ota_http_client.h"

void support_plane_init(void);

#endif // SUPPORT_PLANE_H
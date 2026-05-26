#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "mqtt_client.h"
#include "mqtt_client_app.h"
#include "ota_http_client.h"

static const char *TAG = "MQTT_CLIENT_APP";

static esp_mqtt_client_handle_t client = NULL;

static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {

    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT connected");
        ESP_LOGI(TAG, "Subscribe Topic : %s", APP_MQTT_OTA_TOPIC);
        ESP_LOGI(TAG, "Publish Topic : %s", APP_MQTT_HELLO_TOPIC);
        esp_mqtt_client_subscribe(client, APP_MQTT_OTA_TOPIC, 0);
        esp_mqtt_client_publish(client, APP_MQTT_HELLO_TOPIC, "hello from esp32", 0, 1, 0);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT disconnected");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "Subscribed, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "Published, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "Received topic: %.*s", event->topic_len, event->topic);
        ESP_LOGI(TAG, "Received data: %.*s", event->data_len, event->data);
        if (strncmp(event->topic, APP_MQTT_OTA_TOPIC, event->topic_len) == 0)
        {
            char url[256] = {0};
            int len = event->data_len < sizeof(url) - 1 ? event->data_len : sizeof(url) - 1;
            memcpy(url, event->data, len);
            ota_start_http_update(url);
        }
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT error");
        break;

    default:
        break;
    }
}

/* ================= 初始化 MQTT ================= */
void mqtt_client_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = APP_MQTT_BROKER,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    esp_mqtt_client_start(client);
}

void mqtt_client_publish(const char *topic, const char *data)
{
    if (client) {
        esp_mqtt_client_publish(client, topic, data, 0, 1, 0);
    }
}

void mqtt_log_i(const char *tag, const char *format, ...)
{
    char buffer[256];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    ESP_LOGI(tag, "%s", buffer);

    mqtt_client_publish(APP_MQTT_LOG_TOPIC, buffer);
}
#ifndef MQTT_CLIENT_APP_H
#define MQTT_CLIENT_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#define APP_MQTT_BROKER          CONFIG_APP_MQTT_BROKER
#define APP_MQTT_TEST_TOPIC      CONFIG_APP_MQTT_TEST_TOPIC
#define APP_MQTT_LOG_TOPIC       CONFIG_APP_MQTT_LOG_TOPIC
#define APP_MQTT_OTA_TOPIC       CONFIG_APP_MQTT_OTA_TOPIC
#define APP_MQTT_HELLO_TOPIC     CONFIG_APP_MQTT_HELLO_TOPIC

void mqtt_client_start(void);
void mqtt_client_publish(const char *topic, const char *data);
void mqtt_log_i(const char *tag, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* MQTT_CLIENT_APP_H */
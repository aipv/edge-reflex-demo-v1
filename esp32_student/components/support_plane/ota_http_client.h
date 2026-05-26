#ifndef OTA_HTTP_CLIENT_H
#define OTA_HTTP_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

int ota_start_http_update(const char *image_url);
void ota_print_running_partition(void);
void ota_confirm_after_start(void);

#ifdef __cplusplus
}
#endif

#endif /* OTA_HTTP_CLIENT_H */
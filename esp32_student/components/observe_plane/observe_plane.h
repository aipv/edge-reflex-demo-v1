#ifndef OBSERVE_PLANE_H
#define OBSERVE_PLANE_H

#include "esp_err.h"

#define OBSERVE_DATA_PCM32              1
#define OBSERVE_DATA_PCM16              2
#define OBSERVE_DATA_MFCC               3

#define OBSERVE_PCM16_DATA_SIZE         2

esp_err_t observe_plane_init(void);
esp_err_t observe_plane_data_report(void *data, int data_type, int data_count);

#endif // OBSERVE_PLANE_H
#ifndef GPIO_BUTTON_H
#define GPIO_BUTTON_H

#include "esp_err.h"

#define GPIO_BUTTON_0       GPIO_NUM_0
#define GPIO_BUTTON_1       GPIO_NUM_38
#define GPIO_BUTTON_2       GPIO_NUM_39
#define GPIO_BUTTON_NUM     3

#define BUTTON_PRESSED      0
#define DEBOUNCE_TIME_MS    50

typedef struct {
    uint8_t gpio_num;
    uint32_t press_time_ms;
} button_event_t;

typedef void (*button_callback_t)(uint8_t gpio_num);

esp_err_t gpio_button_init(void);
esp_err_t gpio_button_start(void);
esp_err_t gpio_button_set_callback_func(int index, button_callback_t cbFunc);

#endif // GPIO_BUTTON_H
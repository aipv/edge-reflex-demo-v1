#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "gpio_button.h"

static const char *TAG = "GPIO_BUTTON";

void gpio_button_0_default_callback(uint8_t gpio_num)
{
    ESP_LOGW(TAG, ">>> Button 0 (GPIO %d) Pressed! - Executing action A.", gpio_num);
}

void gpio_button_1_default_callback(uint8_t gpio_num)
{
    ESP_LOGW(TAG, ">>> Button 1 (GPIO %d) Pressed! - Executing action B.", gpio_num);
}

void gpio_button_2_default_callback(uint8_t gpio_num)
{
    ESP_LOGW(TAG, ">>> Button 2 (GPIO %d) Pressed! - Executing action C.", gpio_num);
}

static QueueHandle_t button_event_queue = NULL;
static const gpio_num_t button_gpios[GPIO_BUTTON_NUM] = {GPIO_BUTTON_0, GPIO_BUTTON_1, GPIO_BUTTON_2};
static uint64_t last_press_time_ms[GPIO_BUTTON_NUM] = {0};
static button_callback_t gpio_callback_func[GPIO_BUTTON_NUM] =
{gpio_button_0_default_callback, gpio_button_1_default_callback, gpio_button_2_default_callback};

/**
 * @brief GPIO Interrupt Service Routine (ISR)
 * NOTE: This runs in an interrupt context (high priority), keep it short!
 * It only queues the event; actual processing is done in the task.
 */
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    
    // Check if the current reading is 'pressed' (Active-Low)
    if (gpio_get_level((gpio_num_t)gpio_num) == BUTTON_PRESSED) {
        
        button_event_t event = {
            .gpio_num = (uint8_t)gpio_num,
            // Use time since boot for simple tracking
            .press_time_ms = (uint32_t)(esp_timer_get_time() / 1000)
        };
        
        // Send the event to the queue from the ISR
        xQueueSendFromISR(button_event_queue, &event, NULL);
    }
}

/**
 * @brief Dedicated FreeRTOS task to handle button events and debounce.
 */
static void gpio_button_task(void* arg)
{
    button_event_t event;
    int button_index = -1;
    uint64_t current_time_ms;
    
    ESP_LOGI(TAG, "Button processing task started.");

    while (1) {
        // Wait forever (or a timeout) for a button event from the ISR
        if (xQueueReceive(button_event_queue, &event, portMAX_DELAY) == pdPASS) {
            
            // 1. Find the index of the button that generated the interrupt
            for(int i = 0; i < GPIO_BUTTON_NUM; i++) {
                if(button_gpios[i] == event.gpio_num) {
                    button_index = i;
                    break;
                }
            }

            if (button_index == -1) continue; // Should not happen

            // 2. Perform Software Debouncing Check
            current_time_ms = esp_timer_get_time() / 1000;
            
            // Check if enough time has passed since the last stable press for this button
            if (current_time_ms - last_press_time_ms[button_index] > DEBOUNCE_TIME_MS)
            {
                // 3. Final Check (Reading the GPIO again in the task context)
                // This confirms the state is still active after the delay.
                if (gpio_get_level(event.gpio_num) == BUTTON_PRESSED)
                {
                    gpio_callback_func[button_index](event.gpio_num);
                    // Update the stable press time
                    last_press_time_ms[button_index] = current_time_ms;
                }
            }
        }
    }
}

/**
 * @brief Initializes the GPIOs, ISR, Queue, and Task
 */
esp_err_t gpio_button_init(void)
{
    // 1. Create the queue before the ISR is installed
    button_event_queue = xQueueCreate(10, sizeof(button_event_t));
    if (button_event_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create button event queue.");
        return ESP_FAIL;
    }

    // 2. Install the global GPIO ISR handler
    gpio_install_isr_service(0);

    // 3. Configure each button GPIO
    for (int i = 0; i < GPIO_BUTTON_NUM; i++) {
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_NEGEDGE; // Trigger on falling edge (press)
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = (1ULL << button_gpios[i]);
        io_conf.pull_down_en = 0;
        io_conf.pull_up_en = 1; // Use internal pull-up resistor
        gpio_config(&io_conf);

        // 4. Hook the ISR to the specific GPIO pin
        gpio_isr_handler_add(button_gpios[i], gpio_isr_handler, (void*) button_gpios[i]);
    }
    return ESP_OK;
}

esp_err_t gpio_button_set_callback_func(int index, button_callback_t cbFunc)
{
    ESP_LOGI(TAG, "gpio_button_set_callback_func(%d).", index);
    if ((index >= 0) && (index < GPIO_BUTTON_NUM))
    {
        gpio_callback_func[index] = cbFunc;
    }
    else
    {
        ESP_LOGW(TAG, "Invalid index when gpio_button_set_callback_func(%d).", index);
    }
    return ESP_OK;
}


esp_err_t gpio_button_start(void)
{
    xTaskCreate(gpio_button_task, "button_task", 4096, NULL, 10, NULL);
    
    return ESP_OK;
}
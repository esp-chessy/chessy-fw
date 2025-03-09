#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_err.h"
#include "led_strip.h"
#include "driver/gpio.h"

#define HALL_COL_SW1 GPIO_NUM_39
#define HALL_COL_SW2 GPIO_NUM_40
#define HALL_COL_SW3 GPIO_NUM_41
#define HALL_COL_SW4 GPIO_NUM_42
#define HALL_COL_SW5 GPIO_NUM_4
#define HALL_COL_SW6 GPIO_NUM_5
#define HALL_COL_SW7 GPIO_NUM_6
#define HALL_COL_SW8 GPIO_NUM_7
#define HALL_ROW1 GPIO_NUM_14
#define HALL_ROW2 GPIO_NUM_13
#define HALL_ROW3 GPIO_NUM_21
#define HALL_ROW4 GPIO_NUM_8
#define HALL_ROW5 GPIO_NUM_15
#define HALL_ROW6 GPIO_NUM_16
#define HALL_ROW7 GPIO_NUM_17
#define HALL_ROW8 GPIO_NUM_18
#define LED_DATA_IN1 GPIO_NUM_38

#define COL_NUM 4
#define ROW_NUM 8

// static const gpio_num_t colPins[COL_NUM] = {
//     HALL_COL_SW1,
//     HALL_COL_SW2,
//     HALL_COL_SW3,
//     HALL_COL_SW4
// };

// static const gpio_num_t rowPins[ROW_NUM] = {
//     HALL_ROW1,
//     HALL_ROW2,
//     HALL_ROW3,
//     HALL_ROW4,
//     HALL_ROW5,
//     HALL_ROW6,
//     HALL_ROW7,
//     HALL_ROW8
// };

static led_strip_handle_t led_strip;
static const char *TAG = "GPIO_MATRIX";

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_DATA_IN1,
        .max_leds = 64, // at least one LED on board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

void app_main(void)
{
    configure_led();
    uint32_t led_index = 0;
    while (1) {
        // Turn on current LED (using green color)
        led_strip_set_pixel(led_strip, led_index, 100, 100, 100); // RGB values
        led_strip_refresh(led_strip);
        
        // Keep LED on for 500ms
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // Turn off current LED
        led_strip_set_pixel(led_strip, led_index, 0, 0, 0);
        led_strip_refresh(led_strip);
        
        // Move to next LED
        led_index = (led_index + 1) % 64;
    }
}

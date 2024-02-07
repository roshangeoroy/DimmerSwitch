/*
This is a copy of the RMT based led_strip example provided by esperrif IDF
*/
#include "ring_led_driver.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "led_strip_encoder.h"
#include "esp_log.h"

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define LED_STRIP_GPIO_NUM 10
#define LED_NUMBERS 24

static const char *ring_led = "example";
static uint8_t led_strip_pixels[LED_NUMBERS * 3];
rmt_channel_handle_t led_chan = NULL;
rmt_encoder_handle_t led_encoder = NULL;
rmt_transmit_config_t tx_config;

uint32_t red = 0;
uint32_t green = 0;
uint32_t blue = 0;
uint32_t hue = 0;
uint32_t saturation = 0;
uint32_t value = 0;
uint8_t head = 0; // head of the led_strip array

static void led_strip_hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b);

void initialize_ring_led(void)
{
    ESP_LOGI(ring_led, "Create RMT TX channel");

    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
        .gpio_num = LED_STRIP_GPIO_NUM,
        .mem_block_symbols = 64, // increase the block size can make the LED less flickering
        .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
        .trans_queue_depth = 4, // set the number of transactions that can be pending in the background
    };

    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));
    ESP_LOGI(ring_led, "Install led strip encoder");

    led_strip_encoder_config_t encoder_config = {
        .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
    };
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

    ESP_LOGI(ring_led, "Enable RMT TX channel");
    ESP_ERROR_CHECK(rmt_enable(led_chan));
    tx_config.loop_count = 0, // no transfer loop
   

    // Add functions to increase and decrease leds and bind it to knob
    led_strip_hsv2rgb(100, 90, 50, &red, &green, &blue);


    // Testing
}

static void led_strip_hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b)
{
    h %= 360; // h -> [0,360]
    uint32_t rgb_max = v * 2.55f;
    uint32_t rgb_min = rgb_max * (100 - s) / 100.0f;

    uint32_t i = h / 60;
    uint32_t diff = h % 60;

    // RGB adjustment amount by hue
    uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

    switch (i)
    {
    case 0:
        *r = rgb_max;
        *g = rgb_min + rgb_adj;
        *b = rgb_min;
        break;
    case 1:
        *r = rgb_max - rgb_adj;
        *g = rgb_max;
        *b = rgb_min;
        break;
    case 2:
        *r = rgb_min;
        *g = rgb_max;
        *b = rgb_min + rgb_adj;
        break;
    case 3:
        *r = rgb_min;
        *g = rgb_max - rgb_adj;
        *b = rgb_max;
        break;
    case 4:
        *r = rgb_min + rgb_adj;
        *g = rgb_min;
        *b = rgb_max;
        break;
    default:
        *r = rgb_max;
        *g = rgb_min;
        *b = rgb_max - rgb_adj;
        break;
    }
}

void add_pixel(uint8_t number_of_pixels)
{
    uint8_t temp = head * 3;
    for (int i = 0; i < number_of_pixels; i++)
    {
        // updating the led_array
        led_strip_pixels[head * 3] = green;
        led_strip_pixels[head * 3 + 1] = red;
        led_strip_pixels[head * 3 + 2] = blue;

        head++;
        head = head % LED_NUMBERS; // head should be within maximum number of leds
        ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
        ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void remove_pixel(uint8_t number_of_pixels)
{

    for (int i = 0; i < number_of_pixels; i++)
    {
        // updating the led_array
        if (head != 0)
        {
            led_strip_pixels[head * 3 - 1] = 0;
            led_strip_pixels[head * 3 - 2] = 0;
            led_strip_pixels[head * 3 - 3] = 0;

            head--;
            head = head % LED_NUMBERS; // head should be within maximum number of leds
            ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
            ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}
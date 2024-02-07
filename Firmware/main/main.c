/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
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
#include "knob_driver.h"
#include "ring_led_driver.h"



static const char *TAG = "example";


void app_main(void)
{

    initialize_knob();
    initialize_ring_led();

  
}

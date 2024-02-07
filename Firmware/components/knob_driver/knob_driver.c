/*
This driver will contain the following functions
    -intialize knob : this will intialize gpio pins with interrupt
    -initialize functions to increase, decrease and button press
    -knob task : task running continously to check if there is change in GPIOs

The state machine and logic used is based on https://www.pinteric.com/rotary.html and 
https://github.com/RalphBacon/226-Better-Rotary-Encoder---no-switch-bounce
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h" 
#include "knob_driver.h"
#include "ring_led_driver.h"

#define GPIO_KNOB_PIN_CLK 8
#define GPIO_KNOB_PIN_DT 9
#define GPIO_KNOB_PIN_SW 0
#define GPIO_KNOB_PIN_SEL  ((1ULL<<GPIO_KNOB_PIN_CLK) | (1ULL<<GPIO_KNOB_PIN_DT))
#define ESP_INTR_FLAG_DEFAULT 0

static QueueHandle_t gpio_evt_queue = NULL;
static void knob_task(void* arg);
static void rotary(uint32_t io_num);

uint8_t lrmem = 3;
int lrsum = 0;
int8_t num = 0;
int8_t l, r;
static const char *KNOB = "knob_driver";


static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void initialize_knob()
{
    //Initializing the GPIO pins
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_ANYEDGE; //interrupt to any edge
    io_conf.pin_bit_mask = GPIO_KNOB_PIN_SEL; //bit mask
    io_conf.mode = GPIO_MODE_INPUT; //input mode
    io_conf.pull_up_en = 1; //pull up enabled
    gpio_config(&io_conf);

 
    gpio_evt_queue = xQueueCreate(50, sizeof(uint32_t)); //Queue to handle GPIO event from ISR

    xTaskCreate(knob_task, "knob_task", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pins
    gpio_isr_handler_add(GPIO_KNOB_PIN_CLK, gpio_isr_handler, (void*) GPIO_KNOB_PIN_CLK);
    gpio_isr_handler_add(GPIO_KNOB_PIN_DT, gpio_isr_handler, (void*) GPIO_KNOB_PIN_DT);
 
}


static void knob_task(void* arg)
{
    uint32_t io_num; //received interrupt gpio number
    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {

        rotary(io_num);

        }
    }
}

static void rotary(uint32_t io_num)
{
    static int8_t TRANS[] = {0,-1,1,14,1,0,14,-1,-1,14,0,1,14,1,-1,0};
    if(io_num == GPIO_KNOB_PIN_DT)
        r = gpio_get_level(GPIO_KNOB_PIN_DT);
    else
        l = gpio_get_level(GPIO_KNOB_PIN_CLK);
    
   lrmem = ((lrmem & 0x03) << 2) + 2*l + r;
   lrsum = lrsum + TRANS[lrmem];
   /* encoder not in the neutral state */
   if(lrsum % 4 != 0)
    return;
   /* encoder in the neutral state */
   if (lrsum == 4)
      {
      lrsum=0;
      add_pixel(3);
      }
   if (lrsum == -4)
      {
      lrsum=0;
      remove_pixel(3);
      }
   /* lrsum > 0 if the impossible transition */
   lrsum=0;
   return;
    
}

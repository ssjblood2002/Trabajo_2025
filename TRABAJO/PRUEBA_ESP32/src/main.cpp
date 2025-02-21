#include <Arduino.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"

gpio_num_t push = GPIO_NUM_22;

#define led0 GPIO_NUM_21
#define led1 GPIO_NUM_18
#define led2 GPIO_NUM_17
#define led3 GPIO_NUM_4
#define led4 GPIO_NUM_25
#define led5 GPIO_NUM_16
#define led6 GPIO_NUM_27
#define led7 GPIO_NUM_19

#define led8 GPIO_NUM_15

#define GPIO_OUTS ((1<<led0)|(1<<led1)|(1<<led2)|(1<<led3)|(1<<led4)|(1<<led5)|(1<<led6)|(1<<led7));

void setup() {
gpio_config_t io_config={};
io_config.mode=GPIO_MODE_OUTPUT;
io_config.pin_bit_mask=GPIO_OUTS;
io_config.intr_type=GPIO_INTR_DISABLE;
io_config.pull_down_en= GPIO_PULLDOWN_DISABLE;
io_config.pull_up_en=GPIO_PULLUP_DISABLE;
gpio_config(&io_config);

io_config.mode=GPIO_MODE_INPUT;
io_config.pin_bit_mask=(1<<push);
io_config.intr_type=GPIO_INTR_DISABLE;
io_config.pull_down_en= GPIO_PULLDOWN_ENABLE;           //bajo pasa a alto 0 -> 1
io_config.pull_up_en=GPIO_PULLUP_DISABLE;
gpio_config(&io_config);

gpio_set_direction(led8,GPIO_MODE_OUTPUT);
}

void loop() {
  int status_push=gpio_get_level(push);
  printf("El estado del pulsador es:");
  printf("%d\n",status_push);

  gpio_set_level(led8,1);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  gpio_set_level(led8,0);
  vTaskDelay(1000/portTICK_PERIOD_MS);

}
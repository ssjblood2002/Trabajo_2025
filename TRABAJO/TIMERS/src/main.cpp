#include <Arduino.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
//#include <esp_timer.h>


// Declaración de variables
esp_timer_handle_t timer;
gpio_num_t pin = GPIO_NUM_2;
bool led = false;


// Función de interrupción del temporizador
void IRAM_ATTR interrupcionTemporizador(void* arg) {
    if(led){
            gpio_set_level(GPIO_NUM_2, 1); // Alternar el estado del LED
        }
        else{
            gpio_set_level(GPIO_NUM_2, 0); // Alternar el estado del LED
        }
        led = led^1;
}

void setup() {

    // Configurar el temporizador
    esp_timer_create_args_t config = {
        .callback = interrupcionTemporizador,
        .name = "mi_temporizador"
    };
    esp_timer_create(&config, &timer);
    esp_timer_start_periodic(timer, 300000); // Interrupción cada 1 segundo

    // Configurar el LED incorporado
    gpio_pad_select_gpio(GPIO_NUM_2);
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);

    
}

void loop() {
        
        //aqui el procesador esta realizando un ciclo oscioso de 50 segundos
        delay(50000);
        
    }

    
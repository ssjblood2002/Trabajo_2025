#include <Arduino.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define A_mas   GPIO_NUM_4
#define A_menos GPIO_NUM_16
#define B_mas   GPIO_NUM_2
#define B_menos GPIO_NUM_15
#define BTN     GPIO_NUM_35
#define SW      GPIO_NUM_34
#define LED     GPIO_NUM_17
#define TIME_M  1000  //tiempo de rotacion de los leds
#define DECIMA  100 //decima de segundo
#define CLK_X    25
#define CLK_Y    25

// put your setup code here, to run once:
// Configura el número de pin GPIO que deseas utilizar
gpio_num_t pin = GPIO_NUM_2;
unsigned long ti,tf,dt;
unsigned long tam,tpm; //para medir el tiempo del motor
unsigned long ta_clk,tp_clk; //para medir el tiempo del motor

gpio_num_t pines_motor[] = {A_mas, B_mas, A_menos, B_menos};
u_int8_t conteo_motor = 0;
u_int8_t decimas,segundos,minutos;
bool estado_led = false;
bool count = true; //esta variable controla si puedo contar o no

void mover_motor(void);
void pines_en_cero(void);
void conf_motor(void);
void calculo_matematico(void);
void conf_sw_btn(void);
void mostrar_decimas(void);

   void gpio_isr_handler(void* arg) {
    if(estado_led==true){
      gpio_set_level(LED,1);
      estado_led = false;
    }
    else {
      gpio_set_level(LED,0);
      estado_led = true;
    }
}

void start_stop(void* arg) {
    count = count^1;
}


void setup() {
  Serial.begin(9600);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);
  display.display();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(15,0);             // Start at top-left corner
  display.println(F("Cronometro PIAI"));
  display.drawLine(0,10,127,10,SSD1306_WHITE);
  display.display();

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(CLK_X,CLK_Y);             // Start at top-left corner
  display.println(F("00"));
  display.setCursor(CLK_X+25,CLK_Y);             // Start at top-left corner
  display.println(F(":"));
  display.setCursor(CLK_X+38,CLK_Y);             // Start at top-left corner
  display.println(F("00"));
  display.setCursor(CLK_X+62,CLK_Y);             // Start at top-left corner
  display.println(F(":"));
  display.setCursor(CLK_X+77,CLK_Y);             // Start at top-left corner
  display.println(F("0"));

  display.display();

  conf_motor();
  conf_sw_btn();
  tam=tpm=0;
  decimas=segundos=minutos=0;
}

void loop() {
    tam=millis();
    if(tam-tpm >= TIME_M){
        mover_motor();
        Serial.print("paso No");
        Serial.println(conteo_motor);
        tpm = tam;
    }
    if(count){
    ta_clk=millis();
    if(ta_clk-tp_clk >= DECIMA){
        ++decimas;
        if(decimas==10){
          decimas=0;
          ++segundos;
          if(segundos==60){
            segundos=0;
            ++minutos;
            minutos = 60 ? 0 : minutos;
          }
        }
        mostrar_decimas();
        tp_clk = ta_clk;
    }
    }
   
}

void conf_motor(void){
  // Configura la estructura de configuración del GPIO
  gpio_config_t config_motor;
  config_motor.pin_bit_mask = (1ULL << A_mas | 1ULL << A_menos | 1ULL << B_mas | 1ULL << B_menos | 1ULL << LED | 1ULL << GPIO_NUM_5 | 1ULL << GPIO_NUM_18 | 1ULL << GPIO_NUM_19); // Máscara de bits para el pin
  config_motor.mode = GPIO_MODE_OUTPUT;      // Configura el pin como salida
  config_motor.pull_up_en = GPIO_PULLUP_DISABLE; // Desactiva el pull-up interno
  config_motor.pull_down_en = GPIO_PULLDOWN_DISABLE; // Desactiva el pull-down interno
  config_motor.intr_type =  GPIO_INTR_DISABLE;
  // Configura el GPIO con la estructura de configuración
  gpio_config(&config_motor);
  gpio_set_level(LED,0);
  gpio_set_level(GPIO_NUM_5,0);
  gpio_set_level(GPIO_NUM_18,0); //pone el resto de leds en 0
  gpio_set_level(GPIO_NUM_19,0);
}

void conf_sw_btn(void){
  // Configurar los pines 34 y 35 como entradas
  gpio_config_t config_btn_sw;
  config_btn_sw.pin_bit_mask = (1ULL << SW | 1ULL << BTN ); // Máscara de bits para el pin
  config_btn_sw.mode = GPIO_MODE_INPUT;      // Configura el pin como entrada
  config_btn_sw.pull_up_en = GPIO_PULLUP_ENABLE; // habilita el pull-up interno
  config_btn_sw.pull_down_en = GPIO_PULLDOWN_DISABLE; // Desactiva el pull-down interno
  config_btn_sw.intr_type =  GPIO_INTR_NEGEDGE; //esto es para flanco de bajada
  // Configura el GPIO con la estructura de configuración
  gpio_config(&config_btn_sw);

 // Esta función se utiliza para instalar el servicio de gestión de interrupciones para los pines GPIO.
 // Es esencial llamar a esta función antes de utilizar interrupciones en los pines GPIO.
  gpio_install_isr_service(0);
  gpio_isr_handler_add(BTN, start_stop, NULL);
  gpio_isr_handler_add(SW, gpio_isr_handler, NULL);
}

void mover_motor(void){
  for(u_int8_t i=0; i<=3; ++i)
    gpio_set_level(pines_motor[i],0);//pines apagados
  conteo_motor++;
  conteo_motor = (conteo_motor > 3) ? 0 : conteo_motor;//verifica conteo entre 0 y 3
  gpio_set_level(pines_motor[conteo_motor],1); //activa nuevo pin
}

void calculo_matematico(void){
  int j;
  double total=0;
  double sinh_value[1000];
  for(j=0;j<1000;++j){
    sinh_value[j] = sinh(j);
  }
  for(j=0;j<1000;++j){
    total = sinh_value[j] + total;
  }
  for(j=0;j<1000;++j){
    sinh_value[j] = sinh(j);
  }
  for(j=0;j<1000;++j){
    total = sinh_value[j] + total;
  }
}

void mostrar_decimas(void){
  display.fillRect(CLK_X+77,CLK_Y,10,15,SSD1306_BLACK);
  display.setCursor(CLK_X+77,CLK_Y);             // Start at top-left corner
  display.println(decimas);
  display.display();
}
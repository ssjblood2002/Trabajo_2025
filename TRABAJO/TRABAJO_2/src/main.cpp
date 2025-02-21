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
#include <Keypad.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {33, 25, 26, 27}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {14, 12, 13, 23}; //connect to the column pinouts of the keypad

Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
char customkey;


gpio_num_t pines_leds[8]={GPIO_NUM_15,GPIO_NUM_2,GPIO_NUM_4,GPIO_NUM_16,GPIO_NUM_17,GPIO_NUM_5,GPIO_NUM_18,GPIO_NUM_19};

gpio_num_t btn_led=GPIO_NUM_34;
int sec_led=1;
int indice=0;
unsigned long tim=0;
unsigned long timf=0;
unsigned long tim_btn_sec=0;
unsigned long tim_btn_sec_f=0;

gpio_num_t btn_motor=GPIO_NUM_35;
bool ban_mot=false;
unsigned long tim_btn_mot=0;
unsigned long tim_btn_mot_f=0;
hw_timer_t * temporizador = NULL;//apuntador al timer
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
gpio_num_t pin_mot= GPIO_NUM_32;
int pwm_mot=0;
int ciclo=0;
String velo="";

void configurar(void);
void sec_1(void);
void sec_2(void);
void sec_3(void);
void sec_4(void);
void sec_5(void);
void imprimir_pantalla(void);
void teclado(void);

void IRAM_ATTR timer() { 
  portENTER_CRITICAL_ISR(&timerMux); 
  ciclo++;
   if(ciclo<pwm_mot  && ban_mot==true){
    gpio_set_level(pin_mot,1);
   }else{
    gpio_set_level(pin_mot,0);
   }
  if(ciclo>=100){
    ciclo=0;
  }
  portEXIT_CRITICAL_ISR(&timerMux); 
}

void IRAM_ATTR fun_secuencia(void* arg) {
  tim_btn_sec_f=millis();
  if(tim_btn_sec_f-tim_btn_sec>=400){
    sec_led++;
    if(sec_led>=6){
      sec_led=1;
    }
    for(int j=0;j<8;j++){
      gpio_set_level(pines_leds[j],0);
      }
    indice=0;
    tim_btn_sec=tim_btn_sec_f;
  }
}

void IRAM_ATTR fun_velocidad(void* arg) {
  tim_btn_mot_f=millis();
  if(tim_btn_mot_f-tim_btn_mot>=400){
    tim_btn_mot=tim_btn_mot_f;
    if(ban_mot==true){
         ban_mot=false;
    }else{
         ban_mot=true;
    }
  }
}

void setup() {
  configurar();
  tim=timf=0;
}
void loop() {
  timf=millis();
  if(timf-tim>=300){
    tim=timf;
    imprimir_pantalla();
    if(sec_led==1){
      sec_1();
    }else if(sec_led==2){
      sec_2();
    }else if(sec_led==3){
      sec_3();
    }else if(sec_led==4){
      sec_4();
    }else if(sec_led==5){
      sec_5();
    }
  }
  teclado();
}
void configurar(){
Serial.begin(115200);
Wire.begin();//para el odjeto wire
oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);//
oled.display();
oled.clearDisplay();
oled.setTextColor(WHITE);
oled.setTextSize(1);//nivel de escala de 6*8 tamaño texto
imprimir_pantalla();
for (int i=0;i<8;i++){
  gpio_pad_select_gpio(pines_leds[i]);
  gpio_set_direction(pines_leds[i], GPIO_MODE_OUTPUT);
}

gpio_config_t io_config={};
io_config.mode=GPIO_MODE_INPUT;
io_config.pin_bit_mask=((1ULL<<btn_led)| (1ULL<<btn_motor));
io_config.intr_type=GPIO_INTR_LOW_LEVEL; ;
io_config.pull_down_en= GPIO_PULLDOWN_DISABLE;           //bajo pasa a alto 0 -> 1
io_config.pull_up_en=GPIO_PULLUP_DISABLE;
gpio_config(&io_config);

gpio_install_isr_service(0);
gpio_isr_handler_add(btn_led, fun_secuencia, (void*) btn_led);
gpio_isr_handler_add(btn_motor, fun_velocidad, (void*) btn_motor);

gpio_pad_select_gpio(pin_mot);
gpio_set_direction(pin_mot, GPIO_MODE_OUTPUT);

temporizador = timerBegin(0, 80, true);// 80mhz/80-> 1mhz
timerAttachInterrupt(temporizador, &timer, true);
timerAlarmWrite(temporizador, 40/*4000*/, true);
timerAlarmEnable(temporizador);
// periodo=1/frecuencia-> 1/250hz=0.004;
// pulsos = frecuencia * periodo -> 1.000.000*0.04=4000;
// periodo=1/frecuencia-> 1/250(ciclo util)*100(pulsos por ciclo util)hz=0.0004
// pulsos = frecuencia * periodo -> 1.000.000*0.00004=40;
}
void sec_1(){
  if(indice<8){
   gpio_set_level(pines_leds[indice],1);
  }else if(indice<16){
    gpio_set_level(pines_leds[indice-8],0);
  }else{
    indice=-1;
  }
  indice++;
}
void sec_2(){
  if(indice==0){
    for(int j=0;j<8;j++){
      gpio_set_level(pines_leds[j],1);
    }
    indice=1;
  }else{
    for(int j=0;j<8;j++){
      gpio_set_level(pines_leds[j],0);
    }
    indice=0;
  }
}
void sec_3(){
  if(indice%2==0){
    gpio_set_level(pines_leds[indice],1);
    indice+=2;
    if(indice==8){
      indice=1;
    }
  }else{
    gpio_set_level(pines_leds[indice],1);
    indice+=2;
    if(indice==9){
       indice=0;
      for(int j=0;j<8;j++){
        gpio_set_level(pines_leds[j],0);
      }
    }
  }
}
void sec_4(){
  if(indice<8){
  gpio_set_level(pines_leds[indice],1);
  }
  if(indice!=0){
    gpio_set_level(pines_leds[indice-1],0);
  }
  indice++;
  if(indice==9){
    indice=0;
  }
}
void sec_5(){
  if(indice>=4){
    indice=0;
    for(int j=0;j<8;j++){
      gpio_set_level(pines_leds[j],0);
    }
  }
  gpio_set_level(pines_leds[indice],1);//0-3
  gpio_set_level(pines_leds[indice+4],1);//4-7
  indice++;
}
void imprimir_pantalla(){
  oled.clearDisplay();
  oled.setCursor(4,4);
  oled.print("Teclado:");
  oled.setCursor(54,4);
  oled.print(velo);
  oled.setCursor(4,16);
  oled.print("Pwm:");
  oled.setCursor(30,16);
  oled.print(pwm_mot);
  oled.setCursor(4,28);
  oled.print("Motor:");
  oled.setCursor(40,28);
  oled.print(String(ban_mot));
  oled.setCursor(4,40);
  oled.print("Secuencia:");
  oled.setCursor(68,40);
  oled.print(String(sec_led));
  oled.display();
}
void teclado(){
  customkey= customKeypad.getKey();
  if(customkey){
    switch (customkey){
    case 'A': 
      if(velo.toInt()>=0 && velo.toInt()<=100){
          pwm_mot=velo.toInt();
        }
      velo="";
      break;
    default:
      velo+=customkey; 
      break;
    }
  }
}
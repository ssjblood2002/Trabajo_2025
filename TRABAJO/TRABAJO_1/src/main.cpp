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
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include <Keypad.h>

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


gpio_num_t leds[8]={GPIO_NUM_15,GPIO_NUM_2,GPIO_NUM_4,GPIO_NUM_16,GPIO_NUM_17,GPIO_NUM_5,GPIO_NUM_18,GPIO_NUM_19};

#define GPIO_OUTS ((1ULL<<leds[0])|(1ULL<<leds[1])|(1ULL<<leds[2])|(1ULL<<leds[3])|(1ULL<<leds[4])|(1ULL<<leds[5])|(1ULL<<leds[6])|(1ULL<<leds[7]));

gpio_num_t btn_led=GPIO_NUM_34;
int sec_led=2;
int i=0;
unsigned long tim=0;
unsigned long timf=0;
unsigned long tim_l=0;
unsigned long tim_lf=0;

gpio_num_t btn_motor=GPIO_NUM_35;
bool ban_mot=false;
unsigned long tim_m=0;
unsigned long tim_mf=0;
hw_timer_t * temporizador = NULL;//apuntador al timer
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
gpio_num_t mot= GPIO_NUM_32;
int vel=250;
int ciclo=0;
String velo="";

unsigned long tim_p=0;
unsigned long tim_pf=0;

void confInicial(void);
void secuencia(void);
void secuencia1(void);
void secuencia2(void);
void secuencia3(void);
void secuencia4(void);
void secuencia5(void);
void imprimirlss(void);
void imprimir(int a,int b,String c);
void teclado(void);

void IRAM_ATTR fun_led(void* arg) {
  tim_lf=millis();
  if(tim_lf-tim_l>=300){
    sec_led++;
    if(sec_led>=6){
      sec_led=1;
    }
    for(int j=0;j<8;j++){
      gpio_set_level(leds[j],0);
      }
    i=0;
    tim=timf=millis();
    tim_l=tim_lf;
  }
}

void IRAM_ATTR fun_motor(void* arg) {
  tim_mf=millis();
  if(tim_mf-tim_m>=300){
    tim_m=tim_mf;
    ban_mot=!ban_mot;
  }
}
void IRAM_ATTR timer() { 
  portENTER_CRITICAL_ISR(&timerMux); 
  ciclo++;
  if(ban_mot==true){
   if(ciclo<vel){
    gpio_set_level(mot,1);
   }else{
    gpio_set_level(mot,0);
   }
  }else{
    gpio_set_level(mot,0);
  }
  if(ciclo>=250){
    ciclo=0;
  }
  portEXIT_CRITICAL_ISR(&timerMux); 
}
void setup() {
  confInicial();
  tim=timf=0;
  tim_p=tim_pf=0;
}
void loop() {
  secuencia();
  teclado();
  tim_pf=millis();
  if(tim_pf-tim_p>=200){
    tim_p=tim_pf;
      imprimirlss();
  }
}
void confInicial(){
Serial.begin(115200);
Wire.begin();//para el odjeto wire
oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);//
oled.display();
oled.clearDisplay();
oled.setTextColor(WHITE);
oled.setTextSize(1);//nivel de escala de 6*8 tamaño texto
imprimirlss();
gpio_config_t io_config={};
io_config.mode=GPIO_MODE_OUTPUT;
io_config.pin_bit_mask=GPIO_OUTS;
io_config.intr_type=GPIO_INTR_DISABLE;
io_config.pull_down_en= GPIO_PULLDOWN_DISABLE;
io_config.pull_up_en=GPIO_PULLUP_DISABLE;
gpio_config(&io_config);

io_config.mode=GPIO_MODE_INPUT;
io_config.pin_bit_mask=((1ULL<<btn_led)| (1ULL<<btn_motor));
io_config.intr_type=GPIO_INTR_LOW_LEVEL; ;
io_config.pull_down_en= GPIO_PULLDOWN_DISABLE;           //bajo pasa a alto 0 -> 1
io_config.pull_up_en=GPIO_PULLUP_DISABLE;
gpio_config(&io_config);

gpio_install_isr_service(0);
gpio_isr_handler_add(btn_led, fun_led, (void*) btn_led);
gpio_isr_handler_add(btn_motor, fun_motor, (void*) btn_motor);

io_config.mode=GPIO_MODE_OUTPUT;
io_config.pin_bit_mask=(1ULL<<mot);
io_config.intr_type=GPIO_INTR_DISABLE;
io_config.pull_down_en= GPIO_PULLDOWN_DISABLE;
io_config.pull_up_en=GPIO_PULLUP_DISABLE;
gpio_config(&io_config);

temporizador = timerBegin(0, 80, true);//80mhz /80= 1mhz
timerAttachInterrupt(temporizador, &timer, true);
timerAlarmWrite(temporizador, 16/*4000*/, true);
timerAlarmEnable(temporizador);
// periodo=1/frecuencia-> 1/250hz=0.004;
// pulsos = frecuencia * periodo -> 1.000.000*0.04=4000;
// periodo=1/frecuencia-> 1/250(ciclo util)*250(pulsos por ciclo util)hz=0.000016
// pulsos = frecuencia * periodo -> 1.000.000*0.000016=16;
}
void secuencia(){
  Serial.println(sec_led);
  timf=millis();
  switch(sec_led){
    case 1:
      if(timf-tim>=300){
        tim=timf;
        secuencia1();
      }
    break;
    case 2:
      if(timf-tim>=500){
        tim=timf;
        secuencia2();
      }
    break;
    case 3:
      if(timf-tim>=200){
        tim=timf;
        secuencia3();
      }
    break;
    case 4:
      if(timf-tim>=350){
       tim=timf;
      secuencia4();
      }
    break;
    case 5:
      if(timf-tim>=600){
        tim=timf;
        secuencia5();
      }
    break;
  }
}
void secuencia1(){
  if(i>=8){
    i=0;
    for(int j=0;j<8;j++){
      gpio_set_level(leds[j],0);
    }
  }
  gpio_set_level(leds[i],1);
  i++;
}
void secuencia2(){
  if(i<=-1){
    i=7;
    for(int j=0;j<8;j++){
      gpio_set_level(leds[j],0);
    }
  }
  gpio_set_level(leds[i],1);
  i--;
}
void secuencia3(){
  if(i>=4){
    i=0;
    for(int j=0;j<8;j++){
      gpio_set_level(leds[j],0);
    }
  }
  gpio_set_level(leds[i],1);//0 - 3
  gpio_set_level(leds[i+4],1); //4- 7
  i++;
}
void secuencia4(){
  if(i<=-1){
    i=3;
    for(int j=0;j<8;j++){
      gpio_set_level(leds[j],0);
    }
  }
  gpio_set_level(leds[i],1);//7-4
  gpio_set_level(leds[i+4],1);//3-0
  i--;
}
void secuencia5(){
  if(i%2==0){
    gpio_set_level(leds[i],1);
    i+=2;
    if(i==8){
      i=1;
    }
  }else{
    gpio_set_level(leds[i],1);
    i+=2;
    if(i==9){
        i=0;
    for(int j=0;j<8;j++){
      gpio_set_level(leds[j],0);
    }
    }
  }
}
void imprimirlss(){
  oled.clearDisplay();
  imprimir(4,4,"SECUENCIA:");
  imprimir(68,4,String(sec_led));
  imprimir(4,15,"MOTOR:");
  imprimir(40,15,String(ban_mot));
  imprimir(4,26,"PWM:");
  imprimir(30,26,String(vel));
  imprimir(4,37,"VALOR:");
  imprimir(40,37,velo);
  oled.display();
}
void imprimir(int a, int b, String c){
  oled.setCursor(a,b);
  oled.print(c);
}
void teclado(){
  customkey= customKeypad.getKey();
  if(customkey){
    if(customkey!='D'){
      velo+=customkey;
    }else if (customkey=='D'){
      if(velo.toInt()>=0 && velo.toInt()<=100){
         vel=velo.toInt();
         vel=map(vel,0,100,0,250);
      }else{
        Serial.println("VALOR NO VALIDO");
      }
      velo="";
    }
  }
}
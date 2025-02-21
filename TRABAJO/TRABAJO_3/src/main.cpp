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

// Definición de pines para el motor DC y los interruptores
#define PIN_MOTOR_DC GPIO_NUM_32
#define PIN_INTERRUPTOR_1 GPIO_NUM_34
#define PIN_INTERRUPTOR_2 GPIO_NUM_35

// Definición de pines para los LEDs
#define PIN_LED_1 GPIO_NUM_19
#define PIN_LED_2 GPIO_NUM_18
#define PIN_LED_3 GPIO_NUM_5
#define PIN_LED_4 GPIO_NUM_17
#define PIN_LED_5 GPIO_NUM_16
#define PIN_LED_6 GPIO_NUM_4
#define PIN_LED_7 GPIO_NUM_2
#define PIN_LED_8 GPIO_NUM_15

int secuenciaActual=3;
int indice=0;
unsigned long tim=0;
unsigned long timf=0;
unsigned long TIM_PIN_1=0;
unsigned long TIM_PIN_1_F=0;


bool estadoMotor=false;
unsigned long TIM_PIN_2=0;
unsigned long TIM_PIN_2_F=0;
hw_timer_t * temporizador = NULL;//apuntador al timer
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
int pwm_mot=0;
int cicloUtil=0;
String velo="";

void secuenciaLuces(void);
void secuencia1(void);
void secuencia2(void);
void secuencia3(void);
void secuencia4(void);
void secuencia5(void);
void actualizarLCD(void);
void leerTeclado(void);
void encender(int a,int b,int c,int d,int e,int f,int g,int h);

void IRAM_ATTR timer() { 
  portENTER_CRITICAL_ISR(&timerMux); 
  cicloUtil++;//50
   if(cicloUtil<pwm_mot  && estadoMotor==true){
    gpio_set_level(PIN_MOTOR_DC,1);
   }else{
    gpio_set_level(PIN_MOTOR_DC,0);
   }
  if(cicloUtil>=100){
    cicloUtil=0;
  }
  portEXIT_CRITICAL_ISR(&timerMux); 
}

void IRAM_ATTR fun_secuencia(void* arg) {
  TIM_PIN_1_F=millis();
  if(TIM_PIN_1_F-TIM_PIN_1>=400){
    secuenciaActual++;
    if(secuenciaActual>=5){
      secuenciaActual=0;
    }
    encender(0,0,0,0,0,0,0,0);
    indice=0;
    TIM_PIN_1=TIM_PIN_1_F;
  }
}

void IRAM_ATTR fun_velocidad(void* arg) {
  TIM_PIN_2_F=millis();
  if(TIM_PIN_2_F-TIM_PIN_2>=400){
    TIM_PIN_2=TIM_PIN_2_F;
    if(estadoMotor==true){
         estadoMotor=false;
    }else{
         estadoMotor=true;
    }
  }
}

void setup() {
  Serial.begin(115200);
  // Configurar pines de salida para los LEDs y el Motor
  gpio_config_t io_config={};
  io_config.mode=GPIO_MODE_OUTPUT;
  io_config.pin_bit_mask=((1ULL<<PIN_LED_1) | (1ULL<<PIN_LED_2) | (1ULL<<PIN_LED_3) | (1ULL<<PIN_LED_4) | (1ULL<<PIN_LED_5) | (1ULL<<PIN_LED_6) | (1ULL<<PIN_LED_7) |(1ULL<<PIN_LED_8) | (1ULL<<PIN_MOTOR_DC));
  io_config.intr_type=GPIO_INTR_DISABLE; ;
  io_config.pull_down_en= GPIO_PULLDOWN_DISABLE;
  io_config.pull_up_en=GPIO_PULLUP_DISABLE;
  gpio_config(&io_config);

  // Configurar pines de entrada/salida según corresponda
  io_config.mode=GPIO_MODE_INPUT;
  io_config.pin_bit_mask=((1ULL<<PIN_INTERRUPTOR_1) | (1ULL<<PIN_INTERRUPTOR_2));
  io_config.intr_type=GPIO_INTR_LOW_LEVEL; ;
  io_config.pull_down_en= GPIO_PULLDOWN_DISABLE;           //bajo pasa a alto 0 -> 1
  io_config.pull_up_en=GPIO_PULLUP_DISABLE;
  gpio_config(&io_config);

// Configurar interrupciones para los interruptores
  gpio_install_isr_service(0);
  gpio_isr_handler_add(PIN_INTERRUPTOR_1, fun_secuencia, (void*) PIN_INTERRUPTOR_1);
  gpio_isr_handler_add(PIN_INTERRUPTOR_2, fun_velocidad, (void*) PIN_INTERRUPTOR_2);

  // Inicializar el LCD
  Wire.begin();//para el odjeto wire
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);//
  oled.display();
  oled.clearDisplay();
  oled.setTextColor(WHITE);
  oled.setTextSize(1);//nivel de escala de 6*8 tamaño texto
  actualizarLCD();
  tim=timf=0;

  //inizizalizar timer
  temporizador = timerBegin(0, 40, true);// 80mhz/40-> 2mhz
  timerAttachInterrupt(temporizador, &timer, true);
  timerAlarmWrite(temporizador, 80/*4000*/, true);
  timerAlarmEnable(temporizador);;
  // periodo=1/frecuencia-> 1/250(ciclo util)*100(pulsos por ciclo util)hz=0.0004
  // pulsos = frecuencia * periodo -> 2.000.000*0.00004=80;
}
void loop() {
  // Paso 1: Leer el leerTeclado alfanumérico para ajustar el ciclo útil del PWM
  leerTeclado();
  timf=millis();
  if(timf-tim>=300){
    tim=timf;
    actualizarLCD();
    secuenciaLuces();
  }
}
// Función para leer el teclado alfanumérico
void leerTeclado(){
  customkey= customKeypad.getKey();
  if(customkey){
    switch (customkey){
    case 'B': 
      if(velo.toInt()>=0 && velo.toInt()<=100){
          pwm_mot=velo.toInt();
        }
      velo="";
      break;
    default:
      velo+=customkey; //10B
      break;
    }
  }
}
// Función para actualizar el LCD
void actualizarLCD() {
    // Limpiar el contenido actual del LCD
    oled.clearDisplay();

    // Escribir el ciclo útil del PWM en la primera línea del LCD
    oled.setCursor(4, 4); // Establecer la posición del cursor en la primera líne
    oled.print("Sec. de leds: ");
    oled.print(secuenciaActual+1);

   // Escribir el estado del motor en la segunda línea del LCD
    oled.setCursor(4, 16); // Establecer la posición del cursor en la segunda línea
    oled.print("Est. motor: ");
    oled.print(estadoMotor ? "ON" : "OFF");

    // Escribir el porcentaje del motor en la tercera línea del LCD
    oled.setCursor(4, 28); // Establecer la posición del cursor en la segunda línea
    oled.print("Por. moto: ");
    oled.print(pwm_mot);

     // Escribir el teclado en la cuarta línea del LCD
    oled.setCursor(4, 40); // Establecer la posición del cursor en la segunda línea
    oled.print("Cam. motor: ");
    oled.print(velo);
    oled.display();
}

// Función para implementar las secuencias de luces con los LEDs
void secuenciaLuces() {
    // Ejecutar la secuencia seleccionada
    switch (secuenciaActual) {
        case 0:
            secuencia1();
            break;
        case 1:
            secuencia2();
            break;
        case 2:
            secuencia3();
            break;
        case 3:
            secuencia4();
            break;
        case 4:
            secuencia5();
            break;
    }
}
// Implementación de las secuencias de luces
void secuencia1() {
    // Encender y apagar los LEDs uno por uno en secuencia
    int matriz[8];
    for(int i=0;i<8;i++){
      if(i!=indice){
        matriz[i]=0;
      }else{
        matriz[i]=1;
      }
    }
  encender(matriz[0],matriz[1],matriz[2],matriz[3],matriz[4],matriz[5],matriz[6],matriz[7]);
  indice++;
  if(indice==9){
    indice=0;
  }
}

void secuencia2() {
    // Encender y apagar todos los LEDs simultáneamente
    if(indice==0){
      indice=1;
      encender(1,1,1,1,1,1,1,1);
    }else{
      indice=0;
      encender(0,0,0,0,0,0,0,0);
    }
}

void secuencia3() {
    // Encender y apagar los LEDs impares y pares
  if(indice==0){
     encender(1,0,1,0,1,0,1,0);
  }
  if(indice==1){
    encender(0,1,0,1,0,1,0,1);
  }
  indice++;
  if(indice==2){
    indice=0;
  }
}

void secuencia4() {
    // Encender y apagar los LEDs en secuencia, uno a uno en direcciones opuestas
    int matriz[8];
    for (int i = 0; i<8; i++) {// 0 7 - 1 6 - 2 5 - 3 4
        if(indice==i || indice ==(7-i)){
            Serial.println(indice);
            matriz[i]=1;
        }else{
          matriz[i]=0;
        } 
    }
    encender(matriz[0],matriz[1],matriz[2],matriz[3],matriz[4],matriz[5],matriz[6],matriz[7]);
    indice++;
    if(indice==8){
      indice=0;
    }
}

void secuencia5() {
    // Encender y apagar los LEDs en secuencia inversa
    int matriz[8];
    for(int i=0;i<8;i++){
      if(i!=indice){
        matriz[i]=0;
      }else{
        matriz[i]=1;
      }
    }
  encender(matriz[0],matriz[1],matriz[2],matriz[3],matriz[4],matriz[5],matriz[6],matriz[7]);
  indice--;
  if(indice==-1){
    indice=7;
  }
}
void encender(int a,int b,int c,int d,int e,int f,int g,int h){
  gpio_set_level(PIN_LED_1,a?1:0);
  gpio_set_level(PIN_LED_2,b?1:0);
  gpio_set_level(PIN_LED_3,c?1:0);
  gpio_set_level(PIN_LED_4,d?1:0);
  gpio_set_level(PIN_LED_5,e?1:0);
  gpio_set_level(PIN_LED_6,f?1:0);
  gpio_set_level(PIN_LED_7,g?1:0);
  gpio_set_level(PIN_LED_8,h?1:0);
}
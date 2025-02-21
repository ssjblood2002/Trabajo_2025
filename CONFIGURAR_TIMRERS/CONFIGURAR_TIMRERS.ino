hw_timer_t * temporizador = NULL;//apuntador al timer
hw_timer_t * temporizador2 = NULL;
//modificador de variables
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
int cont=0;
int cont2=0;
bool ban_timer0=false;
bool ban_timer2=false;
unsigned long tiempo=0;
unsigned long tiempo2=0;

//funcion que llega cuando se dispara timer
void IRAM_ATTR timer() { 
  portENTER_CRITICAL_ISR(&timerMux); 
  ban_timer0=true;
  tiempo=millis();
  portEXIT_CRITICAL_ISR(&timerMux); 
 
}
void IRAM_ATTR timer2() { 
  portENTER_CRITICAL_ISR(&timerMux); 
  ban_timer2=true;
  tiempo2=millis();
  portEXIT_CRITICAL_ISR(&timerMux); 
 
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");
  //hw_timer_t = iniziliza(timer 0,80MHZ/80,conteo hacia arriba)
  temporizador = timerBegin(0, 40, true);
  //cuando se dispara(hw_timer_t,nombre de funcion,por borde)
  timerAttachInterrupt(temporizador, &timer, true);
  //parametros   (hw_timer_t,preescalodor 1 sg, volver a contar)
  timerAlarmWrite(temporizador, 1000000, true);
  //habilitar interrupcion
  timerAlarmEnable(temporizador);
  temporizador2 = timerBegin(1, 40, true);
  timerAttachInterrupt(temporizador2, &timer2, true);
  timerAlarmWrite(temporizador2, 2000000, true);
  timerAlarmEnable(temporizador2);
}

void loop() {
 if(ban_timer0==true){
   ban_timer0=false;
   cont++;
   Serial.println("TIEMPO 1 SG");
   Serial.println(cont);
   Serial.println(tiempo);
 }
 if(ban_timer2==true){
   ban_timer2=false;
   cont2++;
   Serial.println("TIEMPO 2 SG");
   Serial.println(cont2);
   Serial.println(tiempo2);
 }
}

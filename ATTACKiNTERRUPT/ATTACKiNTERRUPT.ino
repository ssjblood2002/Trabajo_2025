int pushButton_pin1=   5;
int LED_pin =  12;
int  pushButton_pin2=   26;
int  LED_pin2=   13;
int c=0;
bool bandera=false;
unsigned long reb1=0,reb11=0;
int c2=0;
bool bandera2=false;
unsigned long reb2=0,reb21=0;
void IRAM_ATTR pulsos1()
{
  reb11=millis();
  //si no hay pasado mas de 100ms desde la ultima interrupcion,
  //entonces es un rebote del pulsador
  if(reb11-reb1>150){
    bandera =true;
       c++;
    reb1=millis();
  }
}
void IRAM_ATTR pulsos2(){
 reb21=millis();
  if(reb21-reb2>150){
    bandera2 =true;
       c2++;
    reb2=millis();
  }
}
void setup()
{
  Serial.begin(115200);
  pinMode(LED_pin, OUTPUT);
  pinMode(pushButton_pin1, INPUT);
  pinMode(LED_pin2, OUTPUT);
  pinMode(pushButton_pin2, INPUT);
  attachInterrupt(pushButton_pin1, pulsos1, RISING);
  attachInterrupt(pushButton_pin2, pulsos2, RISING);
} 
void loop()
{
  if(bandera==true){
    bandera=false;
    digitalWrite(LED_pin, !digitalRead(LED_pin));
     Serial.print("CONTROL 1: ");
    Serial.println(c);
  }
  if(bandera2==true){
    bandera2=false;
    digitalWrite(LED_pin2, !digitalRead(LED_pin2));
    Serial.print("CONTROL 2: ");
    Serial.println(c2);
  }
}
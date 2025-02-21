int led=14;//pin de salida
int fre=1000;//fecuencia
int res=9;//resolucion bits
int canal=0;//canal de comunicacion

void setup() {
  Serial.begin(115200);
  pinMode(led,OUTPUT);
  ledcSetup(canal,fre,res);//inizialzar parametros
  ledcAttachPin(led, canal);//inizializar canal
}

void loop() {
  for(int i=0;i<511;i+=10){
    ledcWrite(canal, i);
    Serial.println(i);
    delay(200);
  }
   for(int i=511;i>0;i-=10){
    ledcWrite(canal, i);
    Serial.println(i);
     delay(200);
  }
}

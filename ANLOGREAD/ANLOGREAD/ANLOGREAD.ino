int led=12;
int fre=1000;
int res=8;
int canal=1;

void setup() {
  Serial.begin(115200);
 pinMode(led, OUTPUT);
 ledcSetup(canal,fre,res);
 ledcAttachPin(led,canal);
}

void loop() {
  int x=analogRead(2);
  Serial.println(x);
  float m=x*0.0008058;//   3,3/4096
  Serial.println(m);
  float pwm=x*0.06227106;//    255/4096
  ledcWrite(canal,pwm);
}


int led=12;
int pul=5;

void setup() {
  Serial.begin(115200);
 pinMode(led, OUTPUT);
 pinMode(pul, INPUT);
 
}

void loop() {
  if(digitalRead(pul)==HIGH){
    digitalWrite(led, HIGH);
  }else{
    digitalWrite(led, LOW);
  }
 
}

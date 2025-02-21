int gpioPin=12;
void setup() {
  pinMode(2,INPUT);
  if (gpioPin < 32) {
    GPIO.enable_w1ts =0B00000000001;
  } else {
    GPIO.enable1_w1ts.val = (1 << (gpioPin - 32));
  }

}

void loop() {
 int inputValue1 = (GPIO.in >> gpioPin) & 0x1;

}

#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

String MACadd = "AA:BB:CC:11:22:33";
uint8_t address[6]  = {0xAA, 0xBB, 0xCC, 0x11, 0x22, 0x33};
String name = "SSJ_BLOOD";
char *pin = "1234";
bool connected;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("SSJ_BLOOD_ESP32", true); 
  Serial.println("The device started in master mode, make sure remote BT device is on!");
  connected = SerialBT.connect(name);
  
  if(connected) {
    Serial.println("Conectado");
  } else {
    while(!SerialBT.connected(10000)) {
      Serial.println("No se pudo conectar"); 
    }
  }
  if (SerialBT.disconnect()) {
    Serial.println("Desconectado");
  }
  SerialBT.connect();
  Serial.println("CONEXION EXISTOSA"); 
}

void loop() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    Serial.write(SerialBT.read());
  }
  delay(20);
}

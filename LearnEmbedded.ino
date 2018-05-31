#include "LE.h"

LE le;

void setup() {
  Serial.begin(9600);// serial baudrate 9600
  //Serial.begin(115000); //for debug ESP8266
  le.setupRFID();
  //le.setupIRRemote();
  //le.setupAutoLight();
  //le.setupESP8266();
  //le.setupNRF24L01();
}

void loop() {
  le.readUID();
  //le.recvIRRemote();
  //le.autoTurnLight();
  //le.serverESP8266();
  //le.transData(); // NRF24L01-1
  //le.recvData();  // NRF24L01-2
}


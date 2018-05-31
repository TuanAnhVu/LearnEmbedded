#ifndef LE_H
#define LE_H

//#define RFID_MFRC522
//#define RFID_OPTIONS1
//#define CLONE_REMOTE
//#define AUTO_LIGHT
//#define TRIGGER_OFF
//#define ESP8266
//#define NRF24L01
//#define NRF_TRANS
//#define NRF_RECV

class LE {
  private:

  public:
    //RFID
    void setupRFID();
    void readUID();

    //Thu phát hồng ngoại
    void setupIRRemote();
    void recvIRRemote();

    //Sensor
    void setupAutoLight();
    void autoTurnLight();

    //ESP8266
    void setupESP8266();
    void serverESP8266();
    void initESP8266();
    void bufferingRequest(char c);
    void procedure(char command);
    void deliverMessage(char msg, int dt);

    //NRF24L01
    void setupNRF24L01();
    void transData();
    void recvData();
};
#endif



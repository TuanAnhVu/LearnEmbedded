#include "LE.h"
#include <DirectIO.h>
#include "Arduino.h"
#include <SPI.h>

//#include "RF24.h"

using namespace std;
/*Options Run program*/
//#define RFID_MFRC522
//#define RFID_OPTIONS1
//#define CLONE_REMOTE
//#define AUTO_LIGHT
//#define TRIGGER_OFF
//#define ESP8266
//#define NRF24L01
//#define NRF_TRANS
//#define NRF_RECV


Output<2> RELAY1;
Output<3> RELAY2;

#ifdef RFID_MFRC522
#include <MFRC522.h> // thư viện "RFID MFRC522".

/*
  Chân kết nối với Arduino Uno R3
  Pin layout should be as follows:
  Pin            Pin
  Arduino Uno    MFRC522 board
  ------------------------------------------------------------
  9              RST
  10             SDA
  11             MOSI
  12             MISO
  13             SCK
*/
#define SS_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN);
unsigned long uidDec, uidDecTemp; // Hiển thị số UID dạng thập phân

void LE::setupRFID() {
  RELAY1 = 0; // Đèn tắt
  RELAY2 = 0;
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Waiting for card...");
}

void LE::readUID() {
  // Tìm thẻ mới
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Đọc thẻ
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  uidDec = 0;
  // Hiển thị UID
  Serial.print("Card UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidDecTemp = mfrc522.uid.uidByte[i];
    uidDec = uidDec * 256 + uidDecTemp;
  }
  Serial.println(uidDec);
  /*==================================
      Tùy chọn chế độ dùng thẻ bật/ tắt đèn
    ===================================*/
#ifdef  RFID_OPTIONS1
  // Tùy chọn 1: 1 thẻ 1 = 1 chức năng
  if (uidDec == 1441981283 || uidDec == 3311855971) {
    RELAY1 = 0;
    RELAY2 = 0;
    Serial.println("Led OFF");
  } else {
  }
  //--------------------------------
  if (uidDec == 2504979902) {
    RELAY1 = 1;//Đèn bật
    RELAY2 = 1;
    Serial.println("Led ON");
  } else {
  }
#else
  // Tùy chọn 2: 1 thẻ = 2 chức năng bật & tắt
  if (uidDec == 1441981283 || uidDec == 3311855971 || uidDec == 2504979902) {
    RELAY1 = !digitalRead(2);
    RELAY2 = !digitalRead(3);
    delay(1);
    if (RELAY1 == HIGH && RELAY2 == HIGH) {
      Serial.println("Led OFF");
    } else Serial.println("Led ON");
  } else {
  }
#endif
  // Hiển thị loại thẻ
  byte piccType = mfrc522.PICC_GetType(mfrc522.uid.sak); // Định dạng thẻ
  Serial.print("Card type: ");

  Serial.println(mfrc522.PICC_GetTypeName(piccType));
  if (piccType != MFRC522::PICC_TYPE_MIFARE_UL) { // Nếu không dùng định dạng thẻ
    Serial.print("Type: ");
    Serial.print(piccType);
    Serial.println("KB");
    Serial.println("===============");
    mfrc522.PICC_HaltA();  // Dừng lại
    return;
    delay(3000);
  }
}
#endif

#ifdef CLONE_REMOTE
#include <IRremote.h> // thư viện hỗ trợ IR remote
const int receiverPin = 8; // chân digital 8 dùng để đọc tín hiệu
IRrecv irrecv(receiverPin); // tạo đối tượng IRrecv mới
decode_results results;// lưu giữ kết quả giải mã tín hiệu

void LE::setupIRRemote() {
  irrecv.enableIRIn(); // stat recv
}

void LE::recvIRRemote() {
  if (irrecv.decode(&results)) // nếu nhận được tín hiệu
  {
    int de_tempt_code = results.value;
    Serial.print(de_tempt_code); // in ra Serial Monitor
    if (de_tempt_code == -22380) {
      RELAY1 = 0;
      RELAY2 = 0;
      Serial.println(" - LIGHT OFF");
    } else if (de_tempt_code == -25962) {
      RELAY1 = 1;
      RELAY2 = 1;
      Serial.println(" - LIGHT ON");
    } else {
      Serial.println(" - NO CHANGE");
    }
    delay(500);
    irrecv.resume(); // nhận giá trị tiếp theo
  }
}
#endif

#ifdef AUTO_LIGHT
const int trig      = 7;  // chân trig của HC-SR04
const int echo      = 6;  // chân echo của HC-SR04
const int quangtro  = A5; // chân analog đọc quang trở
const int led       = 13; // turn led out put
const int node      = 8;  // công tắc

void LE::setupAutoLight() {
  pinMode(trig, OUTPUT);  // chân trig sẽ phát tín hiệu
  pinMode(echo, INPUT);   // chân echo sẽ nhận tín hiệu
#ifdef TRIGGER_OFF
  pinMode(node, INPUT);
#endif
}

void LE::autoTurnLight() {
  unsigned long duration; // biến đo thời gian
  int distance;           // biến lưu khoảng cách
  static int first_distance;
  int giatriQuangtro = analogRead(quangtro);// đọc giá trị quang trở
  Serial.print("Giá trị quang trở: ");
  Serial.println(giatriQuangtro); // Xuất giá trị ra Serial Monitor
  /* Phát xung từ chân trig */
  digitalWrite(trig, 0);  // tắt chân trig
  delayMicroseconds(2);
  digitalWrite(trig, 1);  // phát xung từ chân trig
  delayMicroseconds(5);   // xung có độ dài 5 microSeconds
  digitalWrite(trig, 0);  // tắt chân trig

  /* Tính toán thời gian */
  // Đo độ rộng xung HIGH ở chân echo.
  duration = pulseIn(echo, HIGH);
  // Tính khoảng cách đến vật.
  distance = int(duration / 2 / 29.412);

  /* Kiểm tra đầu vào của công tắc */
  if (distance > 0) {
    if (first_distance <= 0) {
      first_distance = distance;
      Serial.print("Khoảng cách ban đầu đo được: ");
      Serial.print(first_distance);
      Serial.println("cm");
    } else {
    }
  }
  Serial.print("Khoảng cách: ");
  Serial.print(distance);
  Serial.println("cm");
#ifdef TRIGGER_OFF
  digitalWrite(node, LOW);
  int ct = digitalRead(node);
  Serial.println(ct);
  if (ct == 1) {
    RELAY1 = 0;
    RELAY2 = 0;
  }
#endif
  // Đèn tắt nếu đủ tối và khoảng cách thay đổi so với khoảng cách đo được ban đầu.
  if (giatriQuangtro >= 170 && distance <= (first_distance - 20) ) {
    RELAY1 = 1;
    RELAY2 = 1;
    delay(1000);
  } else {
    RELAY1 = 0;
    RELAY2 = 0;
  }
  delay(1000);
}
#endif

/*================================
  Chân kết nối với Arduino Uno R3
  ESP8266       Uno
  -----------------
  TX            RX
  RX            TX
  CH_PD         3,3v
  (VCC:3,3/5v)
  ===================================*/
#ifdef ESP8266
#define CMD_SEND_BEGIN  "AT+CIPSEND=0"
#define CMD_SEND_END    "AT+CIPCLOSE=0"

#define PROTOCOL_HTTP     80
#define PROTOCOL_HTTPS    443
#define PROTOCOL_FTP      21
#define PROTOCOL_CURRENT  PROTOCOL_HTTP

#define CHAR_CR     0x0D
#define CHAR_LF     0x0A

#define STRING_EMPTY  ""

#define DELAY_SEED  1000
#define DELAY_1X    (1*DELAY_SEED)
#define DELAY_2X    (2*DELAY_SEED)
#define DELAY_3X    (3*DELAY_SEED)
#define DELAY_4X    (4*DELAY_SEED)
#define DELAY_5X    (5*DELAY_SEED)
LE le;
bool hasRequest = false;

void LE::setupESP8266() {
  delay(DELAY_5X);
  Serial.begin(115200);
  RELAY1 = 0;
  RELAY2 = 0;
  delay(DELAY_5X); // Chờ để khởi động ESP8266.
  le.initESP8266();
}

void LE::serverESP8266 () {
  while (Serial.available()) {
    le.bufferingRequest(Serial.read());
  }
  if (hasRequest == true) {
    String htmlResponse = 	"<!doctype html>"
                            "<html>"
                            "<head>"
                            "<title>DEMO WEBSERVER CONTROL LIGHT</title>"
                            "</head>"
                            "<style>"
                            ".button{"
                            "background: #F0FFFF;"
                            "border: 5px #00CCFF outset;"
                            "padding: 4px 10px 4px;"
                            "font: bold 12px Verdana;"
                            "color:#FF0000;"
                            "}"

                            ".button:hover {"
                            "border-style: inset;"
                            "background: #0000FF;"
                            "padding: 4px 10px 4px;"
                            "color:E6E6FA;"
                            "}"
                            "</style>"
                            "<body style='background:lightblue'>"
                            "<h1 style='color:red';>ESP8266 DEMO TURN ON/OFF THE LIGHT</h1>"
                            "<form action='' method='GET'>"
                            "<button class='button' type='submit' name='LIGHT' value='ON'/><h4> LIGHT_ON </h4></button>"
                            "<button class='button' type='submit' name='LIGHT' value='OFF'/><h4> LIGHT_OFF </h4></button><br/>"
                            "</form>"
                            "</body>"
                            "</html>";

    String beginSendCmd = String(CMD_SEND_BEGIN) + "," + htmlResponse.length();
    le.deliverMessage(beginSendCmd, DELAY_1X);
    le.deliverMessage(htmlResponse, DELAY_1X);
    le.deliverMessage(CMD_SEND_END, DELAY_1X);
    hasRequest = false;
  }
}

void LE::initESP8266() {
  le.deliverMessage("AT", DELAY_1X);
  le.deliverMessage("AT+RST", DELAY_2X);
  le.deliverMessage("AT+CWMODE=1", DELAY_3X);
  le.deliverMessage("AT+CWSAP=\"VTA_ESP8266\",\"22226789\",4,2", DELAY_5X);
  le.deliverMessage("AT+CIFSR", DELAY_1X);
  le.deliverMessage("AT+CIPMUX=1", DELAY_2X);
  le.deliverMessage(String("AT+CIPSERVER=1,") + PROTOCOL_CURRENT, DELAY_3X);
}

void LE::bufferingRequest(char c) {
  static String bufferData = STRING_EMPTY;
  switch (c) {
    case CHAR_CR:
      break;
    case CHAR_LF:
      {
        le.procedure(bufferData);
        bufferData = STRING_EMPTY;
      }
      break;
    default:
      bufferData += c;
  }
}

void LE::procedure(const String& command) {
  hasRequest = command.startsWith("+IPD,");
  if (command.indexOf("OFF") != -1) {
    RELAY1 = 0;
    RELAY2 = 0;
  }
  else if (command.indexOf("ON") != -1) {
    RELAY1 = 1;
    RELAY2 = 1;
  }
}

void LE::deliverMessage(const String& msg, int dt) {
  Serial.println(msg);
  delay(dt);
}
#endif

/*==================================
  Chân kết nối với Arduino Uno R3
  NRF24L01     Uno
  -----------------
  CE            D9
  CSN           D10
  SCK           D13
  MOSI          D11
  MISO          D12
  (VCC:3,3/5v)
  ====================================*/
#ifdef NRF24L01
#include <SPI.h>
#include "RF24.h"

const uint64_t pipe = 0xE8E8F0F0E1LL; // địa chỉ phát
RF24 radio(9, 10); //thay 10 thành 53 với mega
byte msg[3];
#ifdef NRF_RECV
int led_st = 0;
#endif

#ifdef NRF_TRANS
const int sensor = A5; // Chân nhận dữ liệu cảm biến ánh sáng
int value = 0;
#endif

void LE::setupNRF24L01() {
  radio.begin();
  radio.setAutoAck(1);
#ifdef NRF_RECV
  radio.setDataRate(RF24_1MBPS);    // Tốc độ dữ liệu
  radio.setChannel(10);               // Đặt kênh
  radio.openReadingPipe(1, pipe);
  radio.startListening();
#else
  radio.setRetries(1, 1);
  radio.setDataRate(RF24_1MBPS);    // Tốc độ truyền
  radio.setPALevel(RF24_PA_MAX);      // Dung lượng tối đa
  radio.setChannel(10);               // Đặt kênh
  radio.openWritingPipe(pipe);        // mở kênh
  pinMode(sensor, INPUT);
#endif
}

//Truyền dữ liệu cảm biến ánh sáng
#ifdef NRF_TRANS
void LE::transData() {
  value = analogRead(sensor);
  msg[0] = value / 4;
  radio.write(&msg, sizeof(msg)); // gửi
  Serial.print("Giá trị quang trở: ");
  Serial.println(value);
  delay(1000); //cứ 0,1s gửi dữ liệu 1 lần
}
#endif
//Nhận dữ liệu cảm biến ánh sáng
#ifdef NRF_RECV
void LE::recvData() {
  if (radio.available()) {
    while (radio.available()) {
      radio.read(&msg, sizeof(msg)); // nhận
      Serial.println(msg[0]);
      //analogWrite(led, msg[0]);
      if (msg[0] >= B10010110) {
        RELAY1 = 1;
        RELAY2 = 1;
        Serial.println("LED ON");
      } else {
        RELAY1 = 0;
        RELAY2 = 0;
        Serial.println("LED OF");
      }
    }
  }
}
#endif

#endif

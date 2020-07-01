#include <SPI.h>
#include <RH_RF95.h>
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#define SF         7
#define BANDWIDTH  250
#define TXPOWER    14

#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2
#define RF95_FREQ 434.0
#define I2C_ADDRESS 0x3C
#define LED LED_BUILTIN

uint8_t input_voltage_pin = A0;
uint8_t input_relay_status_pin = A1;
uint8_t node_select_pin = A3;
uint8_t node = 0;
uint16_t packetnum = 0;
String node1 = "";
String node2 = "";

RH_RF95 rf95(RFM95_CS, RFM95_INT);
SSD1306AsciiWire oled;

void setup(){
  pinMode(input_voltage_pin, INPUT);
  pinMode(input_relay_status_pin, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  while (!Serial);
  Serial.begin(115200);
  delay(100);

  Serial.println("RFM98W LoRa Shield RX");

// Set Node Number
  pinMode(node_select_pin, INPUT_PULLUP);
  node = !digitalRead(node_select_pin);  // 0 or 1
  node++;  // 1 or 2
  // Serial.print("Node Number: ");
  // Serial.println(node);

// OLED display
  Wire.begin();
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  // oled.set400kHz();
  oled.setFont(Adafruit5x7);

  oled.clear();
  oled.println("LoRa Shield RX");
  
// RFM98W manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

// Init RFM98W
  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK");

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  rf95.setTxPower(TXPOWER, false);
  rf95.setSpreadingFactor(SF);
  rf95.setSignalBandwidth(BANDWIDTH);
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  Serial.println("LoRa radio init OK");

  delay(1000);
}


void loop(){
  if (rf95.available()){
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    
    if (rf95.recv(buf, &len)){
      digitalWrite(LED, HIGH);
      RH_RF95::printBuffer("Received:", buf, len);
      Serial.print("packet:");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);

      String packet = (char*)buf;
      packet.trim();
      if(packet.startsWith("#")){  // 정상적인 패킷
        packetnum++;

        String message = packet.substring(1);
        uint8_t node = message.substring(0,message.indexOf("#")).toInt();
        message = message.substring(message.indexOf("#")+1);
        float temp = message.substring(0,message.indexOf("#")).toInt()/10.0;
        message = message.substring(message.indexOf("#")+1);
        int humd = message.substring(0,message.indexOf("#")).toInt();
        message = message.substring(message.indexOf("#")+1);
        uint8_t relay = message.substring(0,message.indexOf("#")).toInt();
        message = message.substring(message.indexOf("#")+1);
        float voltage = message.substring(0,message.indexOf("#")).toInt()/10.0;

        Serial.print("Packet number: "); Serial.println(packetnum);
        Serial.print("Node: "); Serial.println(node);
        Serial.print("Temp: "); Serial.println(temp, 1);
        Serial.print("Humd: "); Serial.println(humd);
        Serial.print("Relay: "); Serial.println(relay);
        Serial.print("Voltage: "); Serial.println(voltage, 1);
        Serial.print("RSSI: "); Serial.print(rf95.lastRssi(), DEC); Serial.println("dBm");

        oled.clear();
        oled.print("Packet number: "); oled.println(packetnum);
        oled.print("Node: "); oled.println(node);
        oled.print("Temp: "); oled.println(temp, 1 );
        oled.print("Humd: "); oled.println(humd);
        oled.print("Relay: "); oled.println(relay);
        oled.print("Voltage: "); oled.println(voltage, 1);
        oled.print("RSSI: "); oled.print(rf95.lastRssi(), DEC); oled.println("dBm");

      }else{  // 비정상적인 패킷 수신
      }
    }
    else{
      Serial.println("Receive failed");
    }

  Serial.println();
  }
}
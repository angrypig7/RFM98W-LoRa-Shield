#include <SPI.h>
#include <RH_RF95.h>
#include <Wire.h>
#include "DFRobot_SHT20.h"
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
// #include "fonts.h"  //Open_Sans_Hebrew_Condensed_14/18

#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2
#define RF95_FREQ 434.0
#define I2C_ADDRESS 0x3C

uint8_t input_voltage_pin = A0;
uint8_t input_relay_status_pin = A1;
uint8_t node_select_pin = A3;
uint8_t node = 0;
uint16_t packetnum = 0;
uint16_t packet_interval = 5000;  // 5000ms delay

RH_RF95 rf95(RFM95_CS, RFM95_INT);
DFRobot_SHT20 sht20;
SSD1306AsciiWire oled;

void setup(){
  pinMode(input_voltage_pin, INPUT);
  pinMode(input_relay_status_pin, INPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  while (!Serial);
  Serial.begin(115200);
  delay(100);

  Serial.println("RFM98W LoRa Shield TX");

// Set Node Number
  pinMode(node_select_pin, INPUT_PULLUP);
  node = !digitalRead(node_select_pin);  // 0 or 1
  node++;  // 1 or 2
  Serial.print("Node Number: ");
  Serial.println(node);

// OLED display
  Wire.begin();
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  // oled.set400kHz();
  oled.setFont(Adafruit5x7);

  oled.clear();
  oled.println("LoRa Shield TX");
  oled.set2X();
  oled.print("Node: ");
  oled.println(node);
  oled.set1X();

// Init SHT20
	sht20.initSHT20(); // Init SHT20 Sensor
	delay(100);
	sht20.checkSHT20(); // Check SHT20 Sensor

// RFM98W manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  rf95.setTxPower(20, false);
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  Serial.println("LoRa radio init OK");

  delay(1000);
}


void loop(){
  float input_voltage = analogRead(input_voltage_pin) * 12 * 0.0049;
  // input_voltage = round(input_voltage*10)/10.0;
  uint8_t input_relay_status = digitalRead(input_relay_status_pin);

	float temp = sht20.readTemperature();
  // temp = round(temp*10)/10.0;
	uint16_t humd = round(sht20.readHumidity()*10)/10.0;

	Serial.print("Temperature:");
	Serial.print(temp, 1);
	Serial.print("C");
	Serial.print("       Humidity:");
	Serial.print(humd);
	Serial.print("%");
	Serial.println();

  oled.clear();
  oled.print("Temperature: ");
  oled.println(temp, 1);
  oled.print("Humidity: ");
  oled.print(humd);
  oled.println("%");
  oled.print("relay: ");
  oled.println(input_relay_status);
  oled.print("voltage: ");
  oled.print(input_voltage, 1);
  oled.println("V");

  // Transmit
  String radiopacket = "#";
  radiopacket.concat(node);
  radiopacket.concat("#");
  radiopacket.concat((int)round(temp*10));
  radiopacket.concat("#");
  radiopacket.concat(humd);
  radiopacket.concat("#");
  radiopacket.concat(input_relay_status);
  radiopacket.concat("#");
  radiopacket.concat((int)round(input_voltage*10));
  radiopacket.concat("#");
  
  Serial.print("Radio Packet: ");
  Serial.println(radiopacket);

  char radioChar[radiopacket.length()+1] = {0};
  radiopacket.toCharArray(radioChar, radiopacket.length());
  radioChar[radiopacket.length()] = 0;
  rf95.send(radioChar, radiopacket.length()+1);
  rf95.waitPacketSent();
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  oled.println();
  if (rf95.waitAvailableTimeout(1000)){
    if (rf95.recv(buf, &len)){
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);

      oled.print("Reply: ");
      oled.println((char*)buf);
      oled.print("RSSI:");
      oled.set2X();
      oled.print(rf95.lastRssi());
      oled.println("dBm");
      oled.set1X();
    }
    else{
      Serial.println("Receive failed");
      oled.println("Receive failed");
    }
  }
  else{
    Serial.println("No ACK, no reponse from gateway?");
      oled.println("No gateway");
  }

  rf95.sleep();
  delay(packet_interval);
}
#include <SPI.h>
#include <RH_RF95.h>
#include <Wire.h>
#include "DFRobot_SHT20.h"

#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2
#define RF95_FREQ 434.0

uint8_t input_voltage_pin = A0;
uint8_t input_relay_status_pin = A1;
uint8_t node_select_pin = A3;
uint8_t node = 0;

RH_RF95 rf95(RFM95_CS, RFM95_INT);
int16_t packetnum = 0;

DFRobot_SHT20 sht20;

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
  Serial.println("LoRa radio init OK");

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  rf95.setTxPower(20, false);

  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
}


void loop(){
  uint8_t input_voltage = analogRead(input_voltage_pin) * 12;
  uint8_t input_relay_status = digitalRead(input_relay_status_pin);

	float humd = sht20.readHumidity();
	float temp = sht20.readTemperature();

  String radiopacket = "#";
  radiopacket.concat(node);
  radiopacket.concat("#");
  // char radiopacket[] = "#";

  // char radiopacket[20] = "Hello World #      ";
  // itoa(packetnum++, radiopacket+13, 10);
  // Serial.print("Packet: "); Serial.println(radiopacket);
  // radiopacket[19] = 0;
  
  // Serial.println("Sending..."); delay(10);
  // rf95.send((uint8_t *)radiopacket, 20);

  // delay(10);
  // rf95.waitPacketSent();
	// Serial.println("Transmission complete"); 
  

	// Wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  Serial.println("Waiting for reply..."); delay(10);
  if (rf95.waitAvailableTimeout(1000)){ 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len)){
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);    
    }
    else{
      Serial.println("Receive failed");
    }
  }
  else{
    Serial.println("No reply, is there a listener around?");
  }
  delay(1000);
}
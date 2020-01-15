#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2
#define RF95_FREQ 434.0
#define LED LED_BUILTIN

uint8_t input_voltage_pin = A0;
uint8_t input_relay_status_pin = A1;
uint8_t node_select_pin = A3;
uint8_t node = 0;
uint16_t packetnum = 0;

RH_RF95 rf95(RFM95_CS, RFM95_INT);

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
  Serial.print("Node Number: ");
  Serial.println(node);
  
// RFM98W manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

// Init SHT20
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
  if (rf95.available()){
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    
    if (rf95.recv(buf, &len)){
      digitalWrite(LED, HIGH);
      RH_RF95::printBuffer("Received: ", buf, len);
      Serial.print("Got: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
      
      // Send a reply
      // uint8_t data[] = "Copy";
      uint8_t data[len] = {0};
      for(uint8_t i=0; i<len; i++){
        data[i] = buf[i];
      }
      data[len] = 0;
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();

      digitalWrite(LED, LOW);
      Serial.println("Sent a reply");
    }
    else{
      Serial.println("Receive failed");
    }

  Serial.println();
  }
}
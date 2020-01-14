#include <Wire.h>
#include "DFRobot_SHT20.h"

DFRobot_SHT20 sht20;

void setup(){
	Serial.begin(115200);
	Serial.println("SHT20 Example!");
	sht20.initSHT20();
	delay(100);
	sht20.checkSHT20();
}

void loop(){
	float humd = sht20.readHumidity();
	float temp = sht20.readTemperature();

	Serial.print("Time:");
	Serial.print(millis());
	Serial.print(" Temperature:");
	Serial.print(temp, 1);
	Serial.print("C");
	Serial.print(" Humidity:");
	Serial.print(humd, 1);
	Serial.print("%");
	Serial.println();
	
	delay(1000);
}
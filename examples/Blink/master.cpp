#include <Arduino.h>
#include "SerialManager.h"

byte sendData = 0;
byte recieveData = 0;
SerialManager<byte, byte> manager(Serial, recieveData, sendData, NONE, SECURE);

void setup() {
	pinMode(13, INPUT);
	pinMode(LED_BUILTIN, OUTPUT);
	Serial.begin(115200);
	delay(500);
}

void loop() {

	if (HIGH == digitalRead(13)) {
		digitalWrite(LED_BUILTIN, HIGH);
		sendData = 1;
	}
	else {
		digitalWrite(LED_BUILTIN, LOW);
		sendData = 0;
	}
	manager.send();
	delay(500);
}
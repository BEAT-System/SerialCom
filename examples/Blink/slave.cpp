#include <Arduino.h>
#include "SerialManager.h"

byte sendData = 0;
byte recieveData = 0;
SerialManager<byte, byte> manager(Serial, recieveData, sendData, SECURE, NONE);

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	Serial1.begin(115200);
	delay(500);
}

void loop() {

	if (recieveData == 1) {
		digitalWrite(LED_BUILTIN, HIGH);
	}
	else {
		digitalWrite(LED_BUILTIN, LOW);
	}
	manager.update();
}
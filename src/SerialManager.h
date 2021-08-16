#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H
#include <Arduino.h>
#include "Communication.h"

const uint16_t start_byte = 0xAAAA;
const uint16_t end_byte = 0xEEEE;

template<typename recieveT, typename sendT>
class SerialManager {
private:
	HardwareSerial& _serial;
	recieveT& _indata;
	sendT& _outdata;
	enum Communication _input;
	enum Communication _output;

public:

	/*
	 * Init the SerialManager.
	 *
	 * @param serial        -   HardwareSerial used for serial communication (for most cases: Serial) also determine the Pins
	 * @param recieveData   -   Data, which is overwite each time data is recieved
	 * @param sendData      -   Data, which is sended every time the update() or send()-method is called
	 * @param recieve       -   Communication, determines with which communication protocoll the data is recieved
	 * @param send          -   Communication, determines with which communication protocoll the data is send
	 */
	SerialManager(HardwareSerial& serial, recieveT& recieveData, sendT& sendData, enum Communication recieve, enum Communication send);

	/*
	 * Check for recieved data.
	 *
	 * @param time in [ms] to wait to recieve data
	 */
	void update(unsigned long timeout = 1000);

	/*
	 * Check for recieved data.
	 *
	 * @param time in [ms] till timeout
	 * @return  0  - success
	 * @return -1  - no communication match
	 * @return -2  - time out data
	 * @return -3  - read data failed
	 * @return -4  - time out by recieving checksum
	 * @return -5  - read checksum failed
	 * @return -6  - checksum did not match
	 */
	int8_t recieve(unsigned long timeout = 1000);
	void send();
};

template<typename T>
class SerialSecurity {
public:
	static uint8_t checkSum(T& data) {
		uint8_t pack[sizeof(T)];
		memcpy(pack, &data, sizeof(T));
		uint8_t sum = 0;
		for (uint32_t i = 0; i < sizeof(T); i++) {
			sum ^= pack[i];//bitwise XOR-OPERATION
		}

		return -sum;
	};
};

template<typename recieveT, typename sendT>
SerialManager<recieveT, sendT>::SerialManager(HardwareSerial& serial, recieveT& recieveData, sendT& sendData, enum Communication recieve, enum Communication send) :
	_serial(serial),
	_indata(recieveData),
	_outdata(sendData),
	_input(recieve),
	_output(send)
{

}

template<typename recieveT, typename sendT>
void SerialManager<recieveT, sendT>::update(unsigned long timeout) {
	if (_input != NONE) {
		recieve(timeout);
	}
	if (_output != NONE) {
		send();
	}
}


template<typename recieveT, typename sendT>
int8_t SerialManager<recieveT, sendT>::recieve(unsigned long timeout) {
	unsigned long startTime = millis();
	int recieve_size;
	if (_input == FAST)
	{
		recieve_size = (int)sizeof(recieveT) + 4;
	}
	if (_input == FAST)
	{
		recieve_size = (int)sizeof(recieveT) + 5;
	}

	while ((_serial.available() < recieve_size) && (millis() - startTime < timeout)) {}
	if (millis() - startTime >= timeout)
	{
		return -2;
	}

	uint8_t single_check;
	uint16_t block_check;
	bool check_start = false;
	bool check_end = false;
	uint8_t start_counter = 0;
	uint8_t end_counter = 0;

	recieveT buffer_data;
	char buffer_check;

	//Read till start or end
	if (_serial.available() < 2) { return -2; }
	if (sizeof(uint16_t) != _serial.readBytes((char*)&block_check, sizeof(uint16_t))) { return -3;}
	if (block_check == start_byte) { 
		check_start = true;
	}
	if (block_check == end_byte) { 
		return recieve(timeout);
	}

	while (!check_start)//clear buffer
	{
		if (_serial.available() < 1) { return -2; }
		if (sizeof(uint8_t) != _serial.readBytes((char*)&single_check, sizeof(uint8_t))) { return -3;}
		if (single_check == 0xEE) { 
			++end_counter;
		}
		else{
			end_counter = 0;
		}
		if (end_counter == 2) { 
			check_start = false;
			return -10;
		}
	}
	
	if (_serial.available() < (int)sizeof(recieveT))
	{
		return -2;
	}
	if (sizeof(recieveT) != _serial.readBytes((char*)&buffer_data, sizeof(recieveT)))
	{
		return -3;
	}

	if (_input == SECURE)
	{
		if (_serial.available() < (int)1)
		{
			return -2;
		}
		if (1 != _serial.readBytes((char*)&buffer_check, 1))
		{
			return -5;
		}
	}

	while (!check_end)
	{
		if (_serial.available() < 2) { return -2; }
		if (sizeof(uint16_t) != _serial.readBytes((char*)&block_check, sizeof(uint16_t))) { return -3; }
		if (block_check == end_byte) {
			check_end = true;
				break;
		}
	}

//	Post-Processing
	if (check_start && check_end) {
		if (_input == FAST)
		{
			_indata = buffer_data;
			return 0;
		}
		if (_input == SECURE)
		{
			uint8_t checksum = SerialSecurity<recieveT>::checkSum(buffer_data);
			if (buffer_check != checksum)
			{
				return -6;
			}
			_indata = buffer_data;
			return 0;
		}
	}
	return -1;
}

template<typename recieveT, typename sendT>
void SerialManager<recieveT, sendT>::send() {
	
	_serial.write((char*)&start_byte, sizeof(uint16_t));
	if (_output == FAST)
	{
		_serial.write((char*)&_outdata, sizeof(sendT));
	}
	if (_output == SECURE)
	{
		_serial.write((char*)&_outdata, sizeof(sendT));
		uint8_t check = SerialSecurity<sendT>::checkSum(_outdata);
		_serial.write((char*)&check, 1);
	}
	_serial.write((char*)&end_byte, sizeof(uint16_t));
}

#endif
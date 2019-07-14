#ifndef at24c32_h
#define at24c32_h

#include "Arduino.h"
#include "Wire.h"

class AT24C32{
	public:
		void writeByte(char eepromAddress, int dataAddress, char data);
		char readByte(char eepromAddress, int dataAddress);
	
	private:
		char getLowerAddress(int address);
		char getUpperAddress(int address);
};

#endif
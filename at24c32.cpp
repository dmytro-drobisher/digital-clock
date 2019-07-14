#include "Arduino.h"
#include "at24c32.h"
#include "Wire.h"
#define maxMemorySize 4096

void AT24C32::writeByte(char eepromAddress, int dataAddress, char data){
  //Wire.begin();
  Wire.beginTransmission(eepromAddress);
  
  Wire.write(getUpperAddress(dataAddress)); //first address byte
  Wire.write(getLowerAddress(dataAddress)); //second address byte
  Wire.write(data);

  Wire.endTransmission();
  delay(10);
}

char AT24C32::readByte(char eepromAddress, int dataAddress){
  //Wire.begin();
  Wire.beginTransmission(eepromAddress);

  Wire.write(getUpperAddress(dataAddress)); //first address byte
  Wire.write(getLowerAddress(dataAddress)); //second address byte
  
  Wire.endTransmission();
  delay(10);

  Wire.requestFrom(eepromAddress, 1);
  
  char data = Wire.read();
  return data;
}

char AT24C32::getLowerAddress(int address){
  //Serial.println(address - ((address >> 8) << 8));
  return address - ((address >> 8) << 8);
}

char AT24C32::getUpperAddress(int address){
  //Serial.println(address >> 8);
  return address >> 8;
}

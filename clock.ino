#define numberOne 2
#define numberTwo 3
#define numberThree 4
#define numberFour 5

#define latchPin 12
#define clockPin 11
#define dataPin 10

#include "TimerOne.h"
#include <Wire.h>
#include "DS3231.h"

RTClib RTC;
DS3231 Clock;

byte segments[13];  //values to put on 7seg
byte number[4];  //digits/characters to display
bool decimal[4];  //decimal point location

bool isTime = true; //switch between time and temperature display
byte count = 1; //count to switch between displays

void updateTime(){
  DateTime now = RTC.now();
  byte minute = now.minute();
  byte hour = now.hour();
  number[0] = minute % 10;
  number[1] = floor(minute / 10);
  number[2] = hour % 10;
  number[3] = floor(hour / 10);
}

void updateTemperature(){
  byte temperature = round(Clock.getTemperature());
  number[0] = 12;
  number[1] = 11;
  number[2] = temperature % 10;

  byte tempTens = floor(temperature / 10);
  if(tempTens == 0){
    number[3] = 10;
  } else {
    number[3] = tempTens;  
  }
}

void displayTime() {
  digitalWrite(latchPin, LOW);
  PORTD = B11011100;
  shiftOut(dataPin, clockPin, LSBFIRST, segments[number[1]] | decimal[1]);
  digitalWrite(latchPin, HIGH);
  PORTD = B11111100;

  digitalWrite(latchPin, LOW);
  PORTD = B11101100;
  shiftOut(dataPin, clockPin, LSBFIRST, segments[number[2]] | decimal[2]);
  digitalWrite(latchPin, HIGH);
  PORTD = B11111100;

  digitalWrite(latchPin, LOW);
  PORTD = B11110100;
  shiftOut(dataPin, clockPin, LSBFIRST, segments[number[3]] | decimal[3]);
  digitalWrite(latchPin, HIGH);
  PORTD = B11111100;

  digitalWrite(latchPin, LOW);
  PORTD = B11111000;
  shiftOut(dataPin, clockPin, LSBFIRST, segments[number[0]] | decimal[0]);
  digitalWrite(latchPin, HIGH);
  PORTD = B11111100;
}

void setup() {
  segments[0] = B10000000;  //10 digits
  segments[1] = B11101100;
  segments[2] = B01000010;
  segments[3] = B01001000;
  segments[4] = B00101100;
  segments[5] = B00011000;
  segments[6] = B00010000;
  segments[7] = B11001100;
  segments[8] = B00000000;
  segments[9] = B00001000;
  
  segments[10] = B11111110; //empty display
  segments[11] = B00001110; //degree symbol
  segments[12] = B10010010; //letter "C"

  decimal[0] = true;
  decimal[1] = true;
  decimal[2] = false;
  decimal[3] = true;

  number[0] = 0;
  number[1] = 0;
  number[2] = 0;
  number[3] = 0;

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(numberOne, OUTPUT);
  pinMode(numberTwo, OUTPUT);
  pinMode(numberThree, OUTPUT);
  pinMode(numberFour, OUTPUT);
  Wire.begin();
  //updateTime();

  Timer1.initialize(1000);
  Timer1.attachInterrupt(displayTime);

  digitalWrite(numberOne, HIGH);
  digitalWrite(numberTwo, HIGH);
  digitalWrite(numberThree, HIGH);
  digitalWrite(numberFour, HIGH);
}

void loop() {
  if(count == 0){
    isTime = !isTime;
    decimal[2] = !decimal[2];
  }
  
  if(isTime){
    updateTime();
  } else {
    updateTemperature();
  }

  count = (count + 1) % 5;
  delay(1000);
}


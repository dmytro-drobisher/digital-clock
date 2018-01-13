//pins to control transistors driving individual displays
#define numberOne 2
#define numberTwo 3
#define numberThree 4
#define numberFour 5

//pins to send data to the shift register
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

//update digits of each display to show their respective unit of time
void updateTime(){
  DateTime now = RTC.now();   //get current time
  byte minute = now.minute(); //get minute
  byte hour = now.hour();     //get hour
  number[0] = minute % 10;    //minute units
  number[1] = floor(minute / 10); //minute tens
  number[2] = hour % 10;          //hour units
  number[3] = floor(hour / 10);   //hour tens
}

//update digits to show current temperature
void updateTemperature(){
  byte temperature = round(Clock.getTemperature()); //get temperature from the RTC
  number[0] = 12;   //set last digit to letter "C"
  number[1] = 11;   //set third digit to degree symbol
  number[2] = temperature % 10; //set second digit to degree units

  byte tempTens = floor(temperature / 10);
  if(tempTens == 0){  //if temperature is < 10 then leave first display blank
    number[3] = 10;
  } else {
    number[3] = tempTens;  //else set it to degree tens
  }
}

//multiplexes the display on timer interrupt
void displayTime() {
  //enable the digit
  //shift out the number
  //latch the number in the shift register
  //disable the display
  //uses PNP transistors so HIGH input to turn off the digit
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
  //shift register is a current sink in this configuration so HIGH output to turn off the segment
  segments[0] = B10000000;  //number 0
  segments[1] = B11101100;  //number 1
  segments[2] = B01000010;  //number 2
  segments[3] = B01001000;  //number 3
  segments[4] = B00101100;  //number 4
  segments[5] = B00011000;  //number 5
  segments[6] = B00010000;  //number 6
  segments[7] = B11001100;  //number 7
  segments[8] = B00000000;  //number 8
  segments[9] = B00001000;  //number 9
  
  segments[10] = B11111110; //empty display
  segments[11] = B00001110; //degree symbol
  segments[12] = B10010010; //letter "C"

  //toggle decimal point for each digit
  decimal[0] = true;
  decimal[1] = true;
  decimal[2] = false;
  decimal[3] = true;

  //initialse every digit to 0
  number[0] = 0;
  number[1] = 0;
  number[2] = 0;
  number[3] = 0;

  //set pins to putput, their description is above
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(numberOne, OUTPUT);
  pinMode(numberTwo, OUTPUT);
  pinMode(numberThree, OUTPUT);
  pinMode(numberFour, OUTPUT);
  Wire.begin(); //begin I2C communication

  //setup timer interrupt to 1000ns
  Timer1.initialize(1000);
  Timer1.attachInterrupt(displayTime);

  //disable all displays
  digitalWrite(numberOne, HIGH);
  digitalWrite(numberTwo, HIGH);
  digitalWrite(numberThree, HIGH);
  digitalWrite(numberFour, HIGH);
}

void loop() {
  //every five seconds the display changes from time to temperature
  if(count == 0){
    isTime = !isTime; //toggle display mode if time (TRUE) set to temperature (FALSE) and vice versa
    decimal[2] = !decimal[2]; //toggle decimal point on the second display if ON -> OFF and vice versa
  }
  
  if(isTime){ //if the current mode is time then display time, switch to temperature otherwise
    updateTime();
  } else {
    updateTemperature();
  }

  count = (count + 1) % 5;
  delay(1000);  //1 second delay
}


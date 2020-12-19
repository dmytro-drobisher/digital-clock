//PIR interrupt
#define pirInterrupt 2

//pins to control transistors driving individual displays
#define numberOne 3
#define numberTwo 4
#define numberThree 5
#define numberFour 6

//pins to send data to the shift register
#define latchPin 12
#define clockPin 11
#define dataPin 10

#include "TimerOne.h"
#include <Wire.h>
#include "DS3231.h"
#include "at24c32.h"

AT24C32 eeprom;
RTClib RTC;
DS3231 Clock;

byte segments[13];  //values to put on 7seg
byte number[4];  //digits/characters to display
bool decimal[4];  //decimal point location

byte BST = 0; //summer time offset
bool isTime = true; //switch between time and temperature display
byte count = 1; //count to switch between displays
volatile bool displaysEnabled = true;

//update digits of each display to show their respective unit of time
void updateTime() {
  DateTime now = RTC.now();   //get current time
  byte minute = now.minute(); //get minute
  byte hour = (now.hour() + BST) % 24;     //get hour with BST offset
  number[0] = minute % 10;    //minute units
  number[1] = floor(minute / 10); //minute tens
  number[2] = hour % 10;          //hour units
  number[3] = floor(hour / 10);   //hour tens
}

//update digits to show current temperature
void updateTemperature() {
  byte temperature = round(Clock.getTemperature()); //get temperature from the RTC
  number[0] = 12;   //set last digit to letter "C"
  number[1] = 11;   //set third digit to degree symbol
  number[2] = temperature % 10; //set second digit to degree units

  byte tempTens = floor(temperature / 10);
  if (tempTens == 0) { //if temperature is < 10 then leave first display blank
    number[3] = 10;
  } else {
    number[3] = tempTens;  //else set it to degree tens
  }
}

//multiplexes the display on timer interrupt
void displayTime() {
  if(displaysEnabled){
    //shift out the number
    //latch the number in the shift register
    //enable the digit
    //delay
    //disable the display
    //uses PNP transistors so HIGH input to turn off the digit
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, segments[number[0]] | decimal[0]);
    digitalWrite(latchPin, HIGH);
    PORTD = B10111100;
    delayMicroseconds(75);
    PORTD = B11111100;
    
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, segments[number[1]] | decimal[1]);
    digitalWrite(latchPin, HIGH);
    PORTD = B11011100;
    delayMicroseconds(75);
    PORTD = B11111100;
    
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, segments[number[2]] | decimal[2]);
    digitalWrite(latchPin, HIGH);
    PORTD = B11101100;
    delayMicroseconds(75);
    PORTD = B11111100;

    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, segments[number[3]] | decimal[3]);
    digitalWrite(latchPin, HIGH);
    PORTD = B11110100;
    delayMicroseconds(75);
    PORTD = B11111100;
  }
  
}

//get command from serial interface: set/get
byte getCommand(String input) {
  String command = input.substring(0, 3);

  if (command.compareTo("set") == 0) {
    return 1;
  } else if (command.compareTo("get") == 0) {
    return 2;
  } else {
    return 0;
  }
}

//get modifier of a command from serial interface: hour/minute/second/day/month/year/bst
byte getModifier(String input) {
  String modifier = input.substring(4, input.length());
  modifier = modifier.substring(0, modifier.indexOf(" "));

  if (modifier.compareTo("hour") == 0) {
    return 1;
  } else if (modifier.compareTo("minute") == 0) {
    return 2;
  } else if (modifier.compareTo("second") == 0) {
    return 3;
  } else if (modifier.compareTo("day") == 0) {
    return 4;
  } else if (modifier.compareTo("month") == 0) {
    return 5;
  } else if (modifier.compareTo("year") == 0) {
    return 6;
  } else if (modifier.compareTo("bst") == 0){
    return 7;
  } else {
    return 0;
  }
}

// get value field from instruction
int getValue(String input){
  String value = input.substring(4, input.length());
  value = value.substring(value.indexOf(" ") + 1, value.length());
  return value.toInt();
}

// PIR sensor interrupt routine, Pin Change
void toggleDisplays(){
  if(digitalRead(pirInterrupt) == HIGH){
    displaysEnabled = true; //enable displays if received signal is high
  } else {
    displaysEnabled = false;  //disable otherwise
  }
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

  //initialise every digit to 0
  number[0] = 0;
  number[1] = 0;
  number[2] = 0;
  number[3] = 0;

  Serial.begin(9600);
  Wire.begin(); //begin I2C communication
  BST = eeprom.readByte(0x57, 0); //add bst correction

  //set interrupt pin for PIR sensor
  pinMode(pirInterrupt, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pirInterrupt), toggleDisplays, CHANGE);

  //set pins to putput, their description is above
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(numberOne, OUTPUT);
  pinMode(numberTwo, OUTPUT);
  pinMode(numberThree, OUTPUT);
  pinMode(numberFour, OUTPUT);

  //setup timer interrupt to 1000us
  Timer1.initialize(1000);
  Timer1.attachInterrupt(displayTime);

  //disable all displays
  digitalWrite(numberOne, HIGH);
  digitalWrite(numberTwo, HIGH);
  digitalWrite(numberThree, HIGH);
  digitalWrite(numberFour, HIGH);
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readString();
    byte modifier = getModifier(input);
    int value = getValue(input);
    
    switch (getCommand(input)) {
      //set data
      case 1:
        switch (modifier) {
          case 1:
            Clock.setHour(value % 24);
            break;
          case 2:
            Clock.setMinute(value % 60);
            break;
          case 3:
            Clock.setSecond(value % 60);
            break;
          case 4:
            Clock.setDate(value);
            break;
          case 5:
            Clock.setMonth(value);
            break;
          case 6:
            Clock.setYear(value);
            break;
          case 7:
            eeprom.writeByte(0x57, 0, value);
            BST = value;
            break;
          default:
            Serial.println("UNDEFINED");
            break;
        }
        break;

      //get data
      case 2:
        switch (getModifier(input)) {
          case 1:
            Serial.println(RTC.now().hour());
            break;
          case 2:
            Serial.println(RTC.now().minute());
            break;
          case 3:
            Serial.println(RTC.now().second());
            break;
          case 4:
            Serial.println(RTC.now().day());
            break;
          case 5:
            Serial.println(RTC.now().month());
            break;
          case 6:
            Serial.println(RTC.now().year());
            break;
          case 7:
            Serial.println(BST);
            break;
          default:
            Serial.println("UNDEFINED");
            break;
        }
        break;
      default:
        Serial.println("UNDEFINED");
        break;
    }

    Serial.flush();
  }
  //every five seconds the display changes from time to temperature
  if (count == 0) {
    isTime = !isTime; //toggle display mode if time (TRUE) set to temperature (FALSE) and vice versa
    decimal[2] = !decimal[2]; //toggle decimal point on the second display if ON -> OFF and vice versa
  }

  if (isTime) { //if the current mode is time then display time, switch to temperature otherwise
    updateTime();
  } else {
    updateTemperature();
  }

  count = (count + 1) % 5;
  delay(1000);  //1 second delay
}

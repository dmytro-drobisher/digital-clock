# Arduino digital clock

A simple digital clock running on Arduino Nano. This started as my first electronics project and has been running for over a year at this point. System components:
1. The time and date are maintained by a temperature compensated DS3231 RTC module with a backup battery, the time is therefore not reset when the power is switched off
2. The clock includes a BST switch programmable via serial, is is stored on AT24C32 EEPROM chip that comes with the RTC module
3. Arduino Nano is used to retrieve time from the RTC and display it on the four 7-segment displays using SN74HC595 shift register for multiplexing

##### Principle of operation
* main loop reads time/temperature from the RTC over I2C bus
* timer1 of arduino is used for multiplexing routine: set latch pin to LOW, enable digit, shift out the number, set latch to HIGH, disable displays (this sequence of operations has slight ghosting issues)
* PIR module in retrigger mode with pin change interrupt on pin 2 is set to disable the display when noone is nearby to save power.

##### Requirements

* Arduino Nano ATmega328P (should be able to run on other as well)
* SN74HC595 shift register
* DS3231 device
* AT24C32 device
* PIR module
* 4 7-segment displays (common anode, 5011BS)
* 4 PNP transistors (2N3906)
* 4 2.2k resistors
* 8 220R resistors
* powerbank/power supply - this will run on 2600mAh powerbank for about 48 hours 
* libraries: DS3231, Wire, RTClib, TimerOne

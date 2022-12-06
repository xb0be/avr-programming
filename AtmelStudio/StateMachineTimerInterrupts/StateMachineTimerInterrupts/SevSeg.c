/*
 * DHT22_OnSevSeg
 *
 * Code snippet was taken from internetz.
 * 7-segment display was harvested from old darts board and "reverse engineered".
 *
 * I used Atmel Studio for coding and building. Because I didn't manage to make USBTiny to work
 * in virtual Windoze machine, I use avrdude on Linux host to flash HEX file to uC like this:
 *
ATtiny4313 burn command:
sudo avrdude -v -v -v -c usbtiny -p t4313 -U flash:w:sevenSegmentStaticDisplay.hex:i
ATmega328p burn command:
sudo avrdude -v -v -c usbtiny -p ATmega328P -U flash:w:DHT22_OnSevSeg.hex:i -U lfuse:w:0xe2:m -U hfuse:w:0xd9:m -U efuse:w:0xff:m

 *
 * ATtiny 4313 pinout: https://colinkeef.com/images/attiny_x313/attiny_x313_pinout.jpg
 *
 * Created: 8/29/2022 4:45:29 AM
 * Author : rludvik
 *
 * Schematic:
			  ______________                  ___________________
			 | Attiny 4313  |                | 7-segment display |
			 |              |                |                   |
			 |          PB7 | -------------> | dp                |
			 |          PB6 | -------------> | A                 |
			 |          PB5 | -------------> | B                 |
			 |          PB4 | -------------> | C                 |
			 |          PB3 | -------------> | D                 |
			 |          PB2 | -------------> | E                 |
			 |          PB1 | -------------> | F                 |
			 |          PB0 | -------------> | G                 |
			 |              |                |                   |
			 |              |                |                   |
			 |          PD2 | -------------> | D1                |
			 |          PD1 | -------------> | D2                |
			 |          PD0 | -------------> | D3                |
			 |______________|                |___________________|
 *
 */

#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>

#include "SevSeg.h"

void ssdDisplay(int numToDisplay) {
	// Array of chars to display (0..9)
	//ATtiny4313
	   //char seg_code[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};	// just numbers
	//char seg_code_dp[]={0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};	// numbers with DP on 
	//ATmega328p
	   char seg_code[]={0x81,0xcf,0x92,0x86,0xcc,0xa4,0xa0,0x8f,0x80,0x84};	// just numbers
	char seg_code_dp[]={0x01,0x4f,0x12,0x06,0x4c,0x24,0x20,0x0f,0x00,0x04};	// numbers with DP on 
	int num, temp, i;
	//Set registers as output
	DIGIT_CONTROL_DDR = (1<<PD0) | (1<<PD1) | (1<<PD2);
	DATA_DDR = 0xff;
	//Loop 100 times and show each number
	for (i = 0; i < 300; i++)
	{
		num = numToDisplay;
		temp = num / 100;
		num = num % 100;
		DIGIT_CONTROL_PORT = SegOne;				// Select digit 1
		DATA_PORT = seg_code[temp];					// Display corresponding char from array
		_delay_ms(1);												// And the same for other 3 digits below

		temp = num / 10;
		num = num % 10;
		DIGIT_CONTROL_PORT = SegTwo;
		DATA_PORT = seg_code[temp];
		_delay_ms(1);

		temp = num % 10;
		PORTD = SegThree;
		PORTB = seg_code_dp[temp];
		_delay_ms(1);
	}

}

/*
 * sevenSegmentStaticDisplay.c
 *
 * First C program! Code snippet was taken from internetz.
 * 7-segment display was harvested form old darts board and "reverse engineered".
 * See photos for more info.
 * Reason for doing it: son bought ATtiny 4313, which is not supported in Arduino,
 * so I decided to make a use of it in some other way. I bought USBtinyISP programmer
 * and made it happen.
 *
 * I used Atmel Studio for coding and building. Because I didn't manage to make USBTiny to work
 * in virtual Windoze machine, I use avrdude on Linux host to flash HEX file to uC like this:
 *
 * $ sudo avrdude -v -v -v -c usbtiny -p t4313 -U flash:w:sevenSegmentStaticDisplay.hex:i
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

#define F_CPU 1000000UL		// MCU frequency at 1 MHz
#include <avr/io.h>
#include <util/delay.h>

#include "ssd.h"

int ssdDisplay(int numToDisplay) {
	// Array of chars to display (0..9)
	char seg_code[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};	// just numbers
	//char seg_code_dp[]={0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};	// numbers with DP on
	int num, temp, i;
	//Set registers as output
	DIGIT_CONTROL_DDR = 0xff;
	DATA_DDR = 0xff;
	//Loop 100 times and show each number
	for (i = 0; i < 100; i++)
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
		PORTB = seg_code[temp];
		_delay_ms(1);
	}

}
/*
int main() {
	// Array of chars to display (0 .. 9, A .. F)
	char seg_code[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e};
	int cnt, num, temp, i;

	// Configure the ports as output
	DDRB = 0xff; // Data lines
	DDRD = 0xff; // Digit select PORTD0 - PORTD3

while (1)
{
	for (cnt = 0; cnt <= 9999; cnt++) // loop to display 0-9999
	{
		for (i = 0; i < 100; i++)
		{
			num = cnt;
			temp = num / 1000;
			num = num % 1000;
			PORTD = SegOne;				// Select digit 1
			PORTB = seg_code[temp];		// Display corresponding char from array
			_delay_ms(1);				// And the same for other 3 digits below

			temp = num / 100;
			num = num % 100;
			PORTD = SegTwo;
			PORTB = seg_code[temp];
			_delay_ms(1);

			temp = num / 10;
			PORTD = SegThree;
			PORTB = seg_code[temp];
			_delay_ms(1);

			temp = num % 10;
			PORTD = SegFour;
			PORTB = seg_code[temp];
			_delay_ms(1);
		}
	}
}
}
*/

/*
 * ssd.c
 * Created: 8/29/2022 4:45:29 AM
 * Author : rludvik
 *
 */
// 7-segment display, harvested from old darts board, "reverse engineered".
// Common-anode type => LOW (or 0) lights up the LED.
// So to light up certain segment, set it to LOW (or 0).
// 4 digits (well, A+B+DP of digit 4 + 3 full digits :)
// Look from above, decimal points are on top!
//        D1 G  F  DP A  B
//   _____|__|__|__|__|__|_____
//  |                          |
//  |                          |
//  |_____ __ __ __ __ __ _____|
//        |  |  |  |  |  |
//        E  D4 D3 C  D  D2
//

#define F_CPU 1000000UL
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
		DIGIT_CONTROL_PORT = SegOne;			// Select digit 1
		DATA_PORT = seg_code[temp];				// Display corresponding char from array
		_delay_ms(1);							// And the same for other 3 digits below

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

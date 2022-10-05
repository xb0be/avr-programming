/* Copyright rludvik <r@aufbix.org> 2022
 *
 * This file uses Moreto's original DHT22int.c file, and changed version of his DHT22int.h file
 * (DHT22int_4313.h). See Copyright section in those files.
 * 
 * This file is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This file (main.c) comes WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 * 
 * Please consult the GNU General Public License at http://www.gnu.org/licenses/.
 */
/*
 * TemperatureSensor.c
 *
 * See well documented DHT22int.c and DHT22int_4313.h files for detail on how this works.
 *
 * Make use of DHT22 temperature and humidity sensor and display values on 7-segment display.
 *
 * See lines in DHT22_int_4313.h commented with "//rludvik attiny4313" for changes made!
 *
 * Author : rludvik, September 2022
 * 
 * Schematic:
 
*	 _______                  ______________                  ___________________
*	| DHT22 |                | Attiny 4313  |                | 7-segment display |
*	|       |                |              |                |                   |
*	|       |                |          PB6 | -------------> | A                 |
*	|     2 | <------------> | PD2      PB5 | -------------> | B                 |
*	|       |       |        |          PB4 | -------------> | C                 |
*	|       |      _|_       |          PB3 | -------------> | D                 |
*	|_______|     |4k7|      |          PB2 | -------------> | E                 |
*	              |___|      |          PB1 | -------------> | F                 |
*	                |        |          PB0 | -------------> | G                 |
*	                |        |              |                |                   |
*	               Vcc       |          PD0 | -------------> | D1                |
*	                         |          PD1 | -------------> | D2                |
*	                         |          PD3 | -------------> | D3                |
*	                         |              |                |                   |
*	                         |______________|                |___________________|
 *
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
 */ 

#define F_CPU 1000000L
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "DHT22int_4313.h"

#define SegOne 0x01
#define SegTwo 0x02
#define SegThree 0x08


int main(void)
{
/*
* 7-segment display things
*/

// Array of chars to display (0..9)
char seg_code[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};	// "ordinary" numbers
char seg_code_dp[]={0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};	// Numbers with DP on
int temp_integral_tens, temp_integral_ones, temp_decimal_tens;
DDRB = 0xff;			// Output to 7-segment display
DDRD |= ~(1<<PIND0);	// Select digit pins
DDRD |= ~(1<<PIND1);
DDRD |= ~(1<<PIND3);

/*
* DHT22 + main things
*/

DHT22_STATE_t state;
DHT22_DATA_t sensor_data;
DHT22_Init();
sei();

    while (1) 
    {
		state = DHT22_StartReading();
		// Check the state if you want to confirm that the state machine has started.
		//  In your main loop, check periodically when the data is available and in
		//  case of available, do something:
		state = DHT22_CheckStatus(&sensor_data);
		if (state == DHT_DATA_READY){
			// Do something with the data.
			
		/* 12.34 -> 12.3 (because we only have 3-digit display basically). D4 will not be used
			Example:
				sensor_data.temperature_integral = 12
				sensor_data.temperature_decima = 34
			translates to:
			D3 = 1
			D2 = 2 + DP
			D1 = 3
		*/
			temp_integral_tens = sensor_data.temperature_integral / 10;
			temp_integral_ones = sensor_data.temperature_integral % 10;
			PORTD = SegOne;
			PORTB = seg_code[temp_integral_tens];
			_delay_ms(1);
			PORTD = SegTwo;
			PORTB = seg_code_dp[temp_integral_ones];
			_delay_ms(1);
			temp_decimal_tens = sensor_data.temperature_decimal / 10;
			PORTD = SegThree;
			PORTB = seg_code[temp_decimal_tens];	
			_delay_ms(1);
			// sensor_data.humidity_integral
			// sensor_data.humidity_decimal
		}
		else if (state == DHT_ERROR_CHECKSUM){
			// Do something if there is a Checksum error
			// Display "CS.E" = CheckSum.Error
			// C = 0xC6
			// S. = 0x12
			// E = 0x86
			PORTD = SegThree;
			PORTB = 0xc6;
			_delay_ms(1);
			PORTD = SegTwo;
			PORTB = 0x12;
			_delay_ms(1);
			PORTD = SegOne;
			PORTB = 0x86;
			_delay_ms(1);
			
		}
		else if (state == DHT_ERROR_NOT_RESPOND){
			// Do something if the sensor did not respond
			// Display "DG.E" = DataGather.Error
			// D = 0 = 0xc0
			// G. 6. = 0x02
			// E = 0x86
			PORTD = SegThree;
			PORTB = 0xc0;
			_delay_ms(1);
			PORTD = SegTwo;
			PORTB = 0x02;
			_delay_ms(1);
			PORTD = SegOne;
			PORTB = 0x86;
			_delay_ms(1);
		}
		//_delay_ms(10000);
    }
}


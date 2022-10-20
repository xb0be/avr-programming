#ifndef SSD_H
#define SSD_H

/*
 * 7-segment display for Attiny2313/4313
 *
 * Author      : Robert Ludvik
 * Description : 7-segment Library for harvested 3-digit SSD
 */

#include <stdio.h>
#include <avr/io.h>

//functions
extern ssdDisplay(int numToDisplay);

//Registers used
#define DIGIT_CONTROL_DDR   DDRD
#define DIGIT_CONTROL_PORT  PORTD
#define DATA_DDR            DDRB
#define DATA_PORT           PORTB

#define SegOne   0x01		// Digit select pins on DIGIT_CONTROL_DDR
#define SegTwo   0x02
#define SegThree 0x04

// Array of chars to display (0..9)
//char seg_code[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};	// just numbers
//char seg_code_dp[]={0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};	// numbers with DP on
extern char seg_code[];	// just numbers
extern char seg_code_dp[];	// numbers with DP on

#endif      //SSD_H

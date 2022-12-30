#ifndef settings_H
#define settings_H

/* Set up LEDs
RF signal sending	- blue LED
opening				- green LED
closing				- white LED
stopping motor		- red LED
*/
#define OPEN_LED_PIN		PC0		//Port for Open LED
#define CLOSE_LED_PIN		PC1		//Port for Close LED
#define MOTOR_STOP_LED_PIN	PC2		//Port for Stop motor LED
#define RF_LED_PIN			PC3		//Port for RF LED

//Button definition
//On which pins are push buttons to trigger opening or closing the door
#define OPEN_BTN_PIN		PD0		//
#define CLOSE_BTN_PIN		PD1		//
#define MOTOR_STOP_BTN_PIN	PD4		//Emergency push button to stop the motor running

#define BOUNCETIME			30		//Period for button bounce in ms

//UART RF settings
#define BAUDRATE		9600				//Set desired baud rate
#define UBRRVAL ((F_CPU/(BAUDRATE*8UL))-1)	//Calculate UBRR value
//Define receive parameters
#define SYNC			0xBB				//Synchronization signal
#define RADDR			0x55				//Receiver address
//Define commands
#define MOTOR_STOP_CMD	0x69				//Command to stop the motor
#define OPEN_CMD		0xA0				//Command to open the door
#define CLOSE_CMD		0xF0				//Command to close the door

#define WAIT_TIME		3000				//Time to wait after a button is press

//States of state machine
#define IDLE		1
#define OPENING		2
#define CLOSING		3
#define STOPPING	4

#endif
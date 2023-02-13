#ifndef settings_H
#define settings_H

/* Set up LEDs
RF signal sending	- blue LED
*/
/* Define input and output registers and ports */
#define INPUT_REG			DDRB	//Register for inputs
#define OUTPUT_REG			DDRC	//Register for outputs
#define INPUT_PORT			PORTB	//Port for inputs
#define OUTPUT_PORT			PORTC	//Port for outputs
#define INPUT_PIN			PINB	//Pin for inputs
#define OUTPUT_PIN			PORTC	//Pin for outputs

//#define OPEN_LED_PIN		PC0		//Port for Open LED
//#define CLOSE_LED_PIN		PC1		//Port for Close LED
//#define MOTOR_STOP_LED_PIN	PC2		//Port for Stop motor LED
#define RF_LED_PIN			PC3		//Port for RF LED

//Button definition
//On which pins are push buttons to trigger opening or closing the door
#define OPEN_BTN_PIN		PB0		//
#define CLOSE_BTN_PIN		PB1		//
#define MOTOR_STOP_BTN_PIN	PB2		//Emergency push button to stop the motor running

#define BOUNCETIME			30		//Period for button bounce in ms

//UART RF settings
#define BAUDRATE			9600				//Set desired baud rate
//#define UBRRVAL ((F_CPU/(BAUDRATE*8UL))-1)	//Calculate UBRR value
#define UBRRVAL 51
//Define receive parameters
#define SYNC				0xBB				//Synchronization signal
#define RADDR				0x55				//Receiver address
//Define commands
#define MOTOR_STOP_CMD		0x69				//Command to stop the motor
#define MOTOR_OPEN_CMD		0xA0				//Command to open the door
#define MOTOR_CLOSE_CMD		0x25				//Command to close the door

//#define WAIT_TIME		500				//Time to wait after a button is press

//States of state machine
#define IDLE		1
#define OPENING		2
#define CLOSING		3
#define STOPPING	4

#endif
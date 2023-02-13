#ifndef settings_H
#define settings_H

/* Define input and output registers and ports */
#define INPUT_REG			DDRB	//Register for inputs
#define OUTPUT_REG			DDRC	//Register for outputs
#define INPUT_PORT			PORTB	//Port for inputs
#define OUTPUT_PORT			PORTC	//Port for outputs
#define INPUT_PIN			PINB	//Pin for inputs
#define OUTPUT_PIN			PORTC	//Pin for outputs

/* Buttons		 						- ################ INPUTS ################
 * Pins for push buttons to trigger opening or closing the door
 */
#define OPEN_BTN_PIN		PB0		//Brown wire
#define CLOSE_BTN_PIN		PB1		//Green wire

/* Pins for switches that signal that door was fully closed or fully open */
#define OPEN_SWITCH_PIN		PB2		//Red wire
#define CLOSE_SWITCH_PIN	PB6		//White wire

/* Emergency push button to stop the motor running */
#define EMERGENCY_BTN_PIN	PB7

/* LEDs, motor					- ################ OUTPUTS ################ */
#define OPEN_LED_PIN		PC0		//Port for Open LED
#define CLOSE_LED_PIN		PC1		//Port for Close LED
#define LOCKED_LED_PIN		PC2		//Port for Alarm LED
#define MOTOR_IN1_PIN		PC3		//To motor's IN1
#define MOTOR_IN2_PIN		PC4		//To motor's IN2
#define RF_LED_PIN			PC5		//Just for diagnostic if we get something via UART

/* States definition. Define all states of the machine */
#define CLOSED		1
#define CLOSING		2
#define OPEN		3
#define OPENING		4
#define LOCKED		6
#define ONE			7
#define TWO			8
#define THREE		9
#define IDLE		10
#define STARTING	11
#define PRE_IDLE	12
#define PRE_OPENING	13
#define PRE_CLOSING	14

/* Period for de-bounce in ms */
#define BOUNCETIME	30

//UART RF settings - WORK IN PROGRESS
#define BAUDRATE	9600						//set desired baud rate
//#define UBRRVAL ((F_CPU/(BAUDRATE*8UL))-1)	//calculate UBRR value
#define UBRRVAL		51

#define openingmsg	"Going to OPENING!\n"
#define alarmmsg	"Going to LOCKED!\n"
#define closingmsg	"Going to CLOSING!\n"
#define startingmsg	"Starting ...\n"
#define idlemsg		"Going to IDLE!\n"

#endif
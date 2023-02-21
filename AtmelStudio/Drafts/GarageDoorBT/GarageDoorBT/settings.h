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
#define POWER_LED_PIN		PC2		//Port for Alarm LED
#define MOTOR_IN1_PIN		PC3		//To motor's IN1
#define MOTOR_IN2_PIN		PC4		//To motor's IN2
//#define RELAY_PIN			PC5		//Relay to power up the ATX

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
#define PRE_LOCKED	15

/* Period for de-bounce in ms */
#define BOUNCETIME	30

#define BAUDRATE	9600
//#define UBRRVAL ((F_CPU/(BAUDRATE*16))-1)	//calculate UBRR value
#define UBRRVAL		51

#endif
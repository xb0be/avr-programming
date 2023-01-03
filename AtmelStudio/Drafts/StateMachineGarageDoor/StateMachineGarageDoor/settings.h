#ifndef settings_H
#define settings_H

//Define input and output registers and ports
#define INPUT_REG			DDRB	//Register for inputs
#define OUTPUT_REG			DDRC	//Register for outputs
#define INPUT_PORT			PORTB	//Port for inputs
#define OUTPUT_PORT			PORTC	//Port for outputs
#define INPUT_PIN			PINB	//Pin for inputs
#define OUTPUT_PIN			PORTC	//Pin for outputs

//Buttons		 					- ################ INPUTS ################
//Pins for push buttons to trigger opening or closing the door
#define OPEN_BTN_PIN		PB0		//Brown wire
#define CLOSE_BTN_PIN		PB1		//Green wire

//Pins for switches that signal that door was fully closed or fully open
#define OPEN_SWITCH_PIN		PB2		//Red wire
#define CLOSE_SWITCH_PIN	PB3		//White wire

//Emergency push button to stop the motor running
//#define MOTOR_STOP_PIN		PB4

/*LEDs, motor, lock					- ################ OUTPUTS ################
opening, closing	- green and white LEDs on
open				- green LED on
closed				- white LED on
alarm				- red LED on
RF signal			- blue LED on
*/
#define OPEN_LED_PIN		PC0		//Port for Open LED
#define CLOSE_LED_PIN		PC1		//Port for Close LED
#define ALARM_LED_PIN		PC2		//Port for Alarm LED
//#define RF_LED_PIN			PC3		//Port for RF LED
#define MOTOR_IN1_PIN		PC4		//To motor's IN1
#define MOTOR_IN2_PIN		PC5		//To motor's IN2
//#define LOCK_PIN			PC6		//Pin for electro magnetic lock (relay for it)

//States definition
//Define all states of the machine
#define CLOSED		1
#define CLOSING		2
#define OPEN		3
#define OPENING		4
#define ALARM		5

#define BOUNCETIME	30						//Period for bounce in ms

//UART RF settings
//#define BAUDRATE 9600						//set desired baud rate
//#define UBRRVAL ((F_CPU/(BAUDRATE*8UL))-1)	//calculate UBRR value
////Define receive parameters
//#define SYNC 0xBB							//synchronization signal
//#define RADDR 0x55							//receiver address
////Define commands
//#define MOTOR_STOP_CMD 0x69					//Command to stop the motor
//#define MOTOR_OPEN_CMD 0xA0					//Command to open the door
//#define MOTOR_CLOSE_CMD 0xF0				//Command to close the door

#endif
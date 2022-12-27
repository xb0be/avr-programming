#ifndef settings_H
#define settings_H

/* set up LEDs
opening - green LED blinking
closing - white LED blinking
open - green LED on
closed - white LED on
alarm - red LED on
*/
#define OPEN_LED_PIN  PC0
#define CLOSE_LED_PIN PC1
#define ALARM_LED_PIN PC2

/* Button definition
On which pins are buttons to trigger opening or closing the door
*/
#define OPEN_BTN_PIN  PD0		//Brown wire
#define CLOSE_BTN_PIN PD1		//Green wire

/*Switch (touchguard) definition
On  which pins are switches that signal that door was closed or fully open
*/
#define OPEN_SWITCH_PIN   PD2	//Red wire
#define CLOSE_SWITCH_PIN  PD3	//White wire

/*States definition
Define all states of the machine
*/
#define CLOSED		1
#define CLOSING		2
#define OPEN		3
#define OPENING		4
#define ALARM		5
#define LED_ON		6
#define LED_OFF		7

#define BOUNCETIME	30						//Period for bounce in ms

#endif
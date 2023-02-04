/*
 * StateMachineGarageDoor.c
 *
 * Created: 12/11/2022 2:22:03 PM
 * Author : rludvik
 * Version: 0.1
 * For the full schematic see the schematics_VNH2SP30_schem.png
 * Timer calculator: https://www.ee-diary.com/2021/07/programming-atmega328p-in-ctc-mode.html
 */ 

#define F_CPU 8000000UL
#include "settings.h"
#include <avr/io.h>
#include <avr/interrupt.h>
//#include <stdbool.h>
#include <util/delay.h>


/* Variables list:
 * state: initial state of the state machine
 * cntXXX: counter for button de-bouncing, used in ISR
 * cntTimeout: counter for timeout, used in ISR
 * chkLimit: setting for ms for button de-bouncing
 * timeoutLimit: setting for ms for timeout
 */
volatile char state = LOCKED;
volatile uint8_t cntOpenButton, cntCloseButton, cntOpenSwitch, cntCloseSwitch, cntEmergencyButton = 0;
volatile uint16_t cntTimeout = 0;
uint8_t chkLimit = 30;
uint16_t timeoutLimit = 10000;

/* Declarations */
void debounceTimerStart();
void USART_Init();
void motorOpen();
void motorStop();
void motorClose();
void lock_solenoid();
void unlock_solenoid();

/* Turn all the LEDs off */
void turnOffLEDs() {
	OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
	OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
	OUTPUT_PORT &= ~(1 << LOCKED_LED_PIN);
}

/* Main code begins here */
int main(void) {
	OUTPUT_REG = 0xff; 							//LEDs and motor (output)
	INPUT_REG &= ~(1 << OPEN_BTN_PIN);			//set OPEN_BTN_PIN as input for the button
	INPUT_PORT |= (1 << OPEN_BTN_PIN);			//enable pull-up resistor on button input
	INPUT_REG &= ~(1 << CLOSE_BTN_PIN);			//set CLOSE_BTN_PIN as input for the button
	INPUT_PORT |= (1 << CLOSE_BTN_PIN);			//enable pull-up resistor on button input
	INPUT_REG &= ~(1 << OPEN_SWITCH_PIN);		//set OPEN_SWITCH_PIN as input for the button
	INPUT_PORT |= (1 << OPEN_SWITCH_PIN);		//enable pull-up resistor on button input
	INPUT_REG &= ~(1 << CLOSE_SWITCH_PIN);		//set CLOSE_SWITCH_PIN as input for the button
	INPUT_PORT |= (1 << CLOSE_SWITCH_PIN);		//enable pull-up resistor on button input
	INPUT_REG &= ~(1 << EMERGENCY_BTN_PIN);		//set MOTOR_STOP_PIN as input for the button
	INPUT_PORT |= (1 << EMERGENCY_BTN_PIN);		//enable pull-up resistor on button input	
	
	debounceTimerStart();
	USART_Init();
	sei();
	
	while(1)
	{
		switch (state)
		{
			case LOCKED:
				turnOffLEDs();
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);
				OUTPUT_PORT |= (1 << CLOSE_LED_PIN);
				OUTPUT_PORT |= (1 << LOCKED_LED_PIN);
				motorStop();
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & (cntOpenButton > chkLimit)) {
					state = ONE;
				}
				break;
				
			case ONE:
				turnOffLEDs();
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);
				
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & (cntCloseButton > chkLimit)) {
					state = TWO;
				}
				break;
				
			case TWO:
				turnOffLEDs();
				OUTPUT_PORT |= (1 << CLOSE_LED_PIN);
				
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & (cntOpenButton > chkLimit)) {
					state = THREE;
				}
				break;
				
			case THREE:
				turnOffLEDs();
				OUTPUT_PORT |= (1 << LOCKED_LED_PIN);
				
				if ((!(INPUT_PIN & (1 << EMERGENCY_BTN_PIN))) & (cntEmergencyButton > chkLimit)) {
					state = IDLE;
				}				
				break;
									
			case IDLE:
				turnOffLEDs();
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);
				OUTPUT_PORT |= (1 << LOCKED_LED_PIN);
				
				/* If the Open door switch was pressed */
				if ((!(INPUT_PIN & (1 << OPEN_SWITCH_PIN))) & (cntOpenSwitch > chkLimit)) {
					state = OPEN;
				}
				
				/* If the Closed door switch was pressed */
				if ((!(INPUT_PIN & (1 << CLOSE_SWITCH_PIN))) & (cntCloseSwitch > chkLimit)) {
					state = CLOSED;
				}
				
				/* If the Open button was pressed */
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & (cntOpenButton > chkLimit)) {
					OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
					state = OPENING;
				}
				
				/* If the Close button was pressed */
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & (cntCloseButton > chkLimit)) {
					OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
					state = CLOSING;
				}
				break;

			case CLOSED:
				turnOffLEDs();
				OUTPUT_PORT |= (1 << CLOSE_LED_PIN);

				/* If the Open button was pressed */
				if (((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & (cntOpenButton > chkLimit))) {
					OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
					state = OPENING;
				}
				
				if ((!(INPUT_PIN & (1 << EMERGENCY_BTN_PIN))) & (cntEmergencyButton > chkLimit)) {
					state = LOCKED;
				}				
				break;
			case OPENING:
				/* If the timeout happened */
				if (cntTimeout > timeoutLimit) {
					OUTPUT_PORT ^= (1 << LOCKED_LED_PIN);
					_delay_ms(250);
					OUTPUT_PORT ^= (1 << LOCKED_LED_PIN);
					_delay_ms(250);
					state = LOCKED;
				}
				
				motorOpen();
				
				/* If the Emergency button was pressed */
				if ((!(INPUT_PIN & (1 << EMERGENCY_BTN_PIN))) & (cntEmergencyButton > chkLimit)) {
					state = LOCKED;
				}
				
				/* If the Close button was pressed */
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & (cntCloseButton > chkLimit)) {
					state = CLOSING;
				}
				
				/* If the Open door switch was hit */
				if ((!(INPUT_PIN & (1 << OPEN_SWITCH_PIN))) & (cntOpenSwitch > chkLimit)) {
					motorStop();
					state = OPEN;
				}
				break;

			case OPEN:
				turnOffLEDs();
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);
				
				/* If the Close button was pressed */
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & (cntCloseButton > chkLimit)) {
					OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
					state = CLOSING;
				}
				
				/* If the Emergency button was pressed */
				if ((!(INPUT_PIN & (1 << EMERGENCY_BTN_PIN))) & (cntEmergencyButton > chkLimit)) {
					state = LOCKED;
				}				
				break;
			
			case CLOSING:
				/* If the timeout happened */
				if (cntTimeout > timeoutLimit) {
					OUTPUT_PORT ^= (1 << LOCKED_LED_PIN);
					_delay_ms(500);
					OUTPUT_PORT ^= (1 << LOCKED_LED_PIN);
					_delay_ms(500);
					state = LOCKED;
				}
						
				motorClose();
				
				/* If the Emergency button was pressed */
				if ((!(INPUT_PIN & (1 << EMERGENCY_BTN_PIN))) & (cntEmergencyButton > chkLimit)) {
					state = LOCKED;
				}
				
				/* If the Open button was pressed */
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & (cntOpenButton > chkLimit)) {
					state = OPENING;
				}
				
				/* If the Closed door switch was hit */
				if ((!(INPUT_PIN & (1 << CLOSE_SWITCH_PIN))) & (cntCloseSwitch > chkLimit)) {
					motorStop();
					state = CLOSED;
				}
				
				/* if photo-eye blocked - TBD */
				break;
			
			default:
				break;
		} //end switch
	} //end while
} //end main

/*
 * This one is a little bit clumsy :/
 * Compare vector for button debounce on 8-bit timer.
 * cntlimit value: 1 means 1 ms => 10000 is 10 seconds
 * timeoutLimit value: 15000 is 15 seconds => used only for OPENING and CLOSING states
 */
ISR(TIMER0_COMPA_vect)
{
	static uint8_t cntLimit = 50;
	static uint16_t timeoutLimit = 15000;

	if (!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) {
		cntCloseButton++;
		if (cntCloseButton > cntLimit) {
			cntCloseButton = 0;
		}	
	} else {
		if (cntCloseButton > 0) {
			cntCloseButton--;
		}
	}
	if (!(INPUT_PIN & (1 << OPEN_BTN_PIN))) {
		cntOpenButton++;
		if (cntOpenButton > cntLimit) {
			cntOpenButton = 0;
		}
	} else {
		if (cntOpenButton > 0) {
			cntOpenButton--;
		}
	}
	if (!(INPUT_PIN & (1 << OPEN_SWITCH_PIN))) {
		cntOpenSwitch++;
		if (cntOpenSwitch > cntLimit) {
			cntOpenSwitch = 0;
		}
	} else {
		if (cntOpenSwitch > 0) {
			cntOpenSwitch--;
		}
	}
	if (!(INPUT_PIN & (1 << CLOSE_SWITCH_PIN))) {
		cntCloseSwitch++;
		if (cntCloseSwitch > cntLimit) {
			cntCloseSwitch = 0;
		}
	} else {
		if (cntCloseSwitch > 0) {
			cntCloseSwitch--;
		}
	}
	if (!(INPUT_PIN & (1 << EMERGENCY_BTN_PIN))) {
		cntEmergencyButton++;
		if (cntEmergencyButton > cntLimit) {
			cntEmergencyButton = 0;
		}
	} else {
		if (cntEmergencyButton > 0) {
			cntEmergencyButton--;
		}
	}
	if ((state == 2) | (state == 4)) {		/* Opening or Closing state */
		cntTimeout++;
		if (cntTimeout > timeoutLimit) {
			cntTimeout = 0;
		}
	} else {
		if (cntTimeout > 0) {
			cntTimeout--;
		}		
	}
}

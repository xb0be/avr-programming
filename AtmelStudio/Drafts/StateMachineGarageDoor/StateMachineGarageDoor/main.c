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
 * bpXXXX: true, if a certain button is pressed and debounced
 * RF: flag, if we are coming from RF (remote control) - TBD
 */
volatile char state = LOCKED;
volatile uint8_t extraTime, alarmextraTime = 0;
volatile uint8_t cntOpenButton, cntCloseButton, cntOpenSwitch, cntCloseSwitch, cntEmergencyButton = 0;
uint8_t chkLimit = 30;
//2023-01-31extern uint8_t doOpen;

/* Declarations */
void debounceTimerStart();
//2023-01-31void initTimer();
//2023-01-31void startTimer();
//2023-01-31void stopTimer();
//2023-01-31void restartTimer();
void USART_Init();
void motorOpen();
void motorStop();
void motorClose();
void lock_solenoid();
void unlock_solenoid();
//void turnOffLEDs();

void turnOffLEDs() {
	OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
	OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
	OUTPUT_PORT &= ~(1 << LOCKED_LED_PIN);
}

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
	//2023-01-31initTimer();
	USART_Init();
	sei();
	
	while(1)
	{
		switch (state)
		{
			/*
			 * In LOCKED state:
			 * - turn on all the LEDs (just for testing purpose)
			 * - go through the ONE, TWO, and THREE states only after certain
			 *   combination of pushbuttons was pressed (currently: Open, Close, Open, Close)
			 * - keep restarting Timeout timer in each state
			 */
			case LOCKED:
				turnOffLEDs();
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);
				OUTPUT_PORT |= (1 << CLOSE_LED_PIN);
				OUTPUT_PORT |= (1 << LOCKED_LED_PIN);
				motorStop();
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & (cntOpenButton > chkLimit)) {
					//2023-01-31restartTimer();
					state = ONE;
				}
				break;
			case ONE:
				turnOffLEDs();
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);
				
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & (cntCloseButton > chkLimit)) {
					//2023-01-31restartTimer();
					state = TWO;
				}
				break;
			case TWO:
				turnOffLEDs();
				OUTPUT_PORT |= (1 << CLOSE_LED_PIN);
				
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & (cntOpenButton > chkLimit)) {
					//2023-01-31restartTimer();
					state = THREE;
				}
				break;
			case THREE:
				turnOffLEDs();
				OUTPUT_PORT |= (1 << LOCKED_LED_PIN);
				
				if ((!(INPUT_PIN & (1 << EMERGENCY_BTN_PIN))) & (cntEmergencyButton > chkLimit)) {
					//2023-01-31stopTimer();
					state = IDLE;
				}				
				break;
			/*
			 * In IDLE state:
			 * - turn on the Power LED (TBD), signaling that the system in unlocked and can be used
			 * - check the status of the switches => if something weird is going on, go to the ALARM state.
			 */							
			case IDLE:
				turnOffLEDs();
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);
				OUTPUT_PORT |= (1 << LOCKED_LED_PIN);
				
				/* If the Open door switch is pressed */
				if ((!(INPUT_PIN & (1 << OPEN_SWITCH_PIN))) & (cntOpenSwitch > chkLimit)) {
					state = OPEN;
				}
				
				/* If the Closed door switch is pressed */
				if ((!(INPUT_PIN & (1 << CLOSE_SWITCH_PIN))) & (cntCloseSwitch > chkLimit)) {
					//2023-01-31restartTimer();
					state = CLOSED;
				}
				
				/* If the Open button was pressed */
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & (cntOpenButton > chkLimit)) {
					OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
					//unlock_solenoid();
					//2023-01-31restartTimer();
					state = OPENING;
				}
				
				/* If the Close button was pressed */
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & (cntCloseButton > chkLimit)) {
					OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
					//2023-01-31restartTimer();
					state = CLOSING;
				}
				break;
			/*
			 * In CLOSED state:
			 * - turn on only the Closed LED, signaling the door is fully closed
			 * - lock the Solenoid
			 *   the state shall change to LOCKED
			 * - if the Open pushbutton was pressed:
			 *   +  turn off the Closed LED
			 *   + unlock the Solenoid
			 *   + restart the Timeout timer
			 *   + change the state to OPENING
			 */
			case CLOSED:
				turnOffLEDs();
				OUTPUT_PORT |= (1 << CLOSE_LED_PIN);

				if (((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & (cntOpenButton > chkLimit))) {
					
					OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
					//2023-01-31restartTimer();
					state = OPENING;
				}
				
				if ((!(INPUT_PIN & (1 << EMERGENCY_BTN_PIN))) & (cntEmergencyButton > chkLimit)) {
					//2023-01-31stopTimer();
					state = LOCKED;
				}				
				break;
			case OPENING:
				/* If the timeout appears, interrupt will handle it
				 * timeout timer was disabled on 2023-02-02!
				 */
				
				motorOpen();
				
				/* If the Emergency button was pressed */
				if ((!(INPUT_PIN & (1 << EMERGENCY_BTN_PIN))) & (cntEmergencyButton > chkLimit)) {
					//2023-01-31restartTimer();
					state = LOCKED;
				}
				
				/* If the Close button was pressed, change state to CLOSING */
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & (cntCloseButton > chkLimit)) {
					//2023-01-31restartTimer();
					state = CLOSING;
				}
				
				/* If the Open door switch was hit */
				if ((!(INPUT_PIN & (1 << OPEN_SWITCH_PIN))) & (cntOpenSwitch > chkLimit)) {
					//2023-01-31stopTimer();
					motorStop();
					state = OPEN;
				}
				
				/* If the open door switch was NOT hit TBD */
				break;
			/*
			 * In OPEN state:
			 * - turn on only the Open LED, signaling the door is fully open
			 * - if the Close pushbutton was pressed:
			 *   - turn off the Open LED
			 *   //- start the motor to close the door
			 *   - restart the Timeout timer => if there's no Open switch hit after timeout,
			 *     the state will change to ALARM
			 *   - change the state to CLOSING
			 */
			case OPEN:
				turnOffLEDs();
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);
				
				/* If the Close button was pressed */
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & (cntCloseButton > chkLimit)) {
					OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
					//2023-01-31restartTimer();
					state = CLOSING;
				}
				
				if ((!(INPUT_PIN & (1 << EMERGENCY_BTN_PIN))) & (cntEmergencyButton > chkLimit)) {
					//2023-01-31stopTimer();
					state = LOCKED;
				}				
				break;
			/*
			 * In CLOSING state:
			 * - start the motor to close the door
			 * - if the Emergency pushbutton was pressed:
			 *	//+ stop the motor => this is done in ALARM
			 *	+ stop the Timeout timer
			 *	+ go to the ALARM state
			 * - if the Open pushbutton was pressed:
			 *	+ restart the Timeout timer
			 *	+ go to the OPENING state
			 * - if the Closed switch was hit:
			 *	+ stop the motor
			 *	+ stop the Timeout timer
			 *	+ go to the CLOSED state
			 */				
			case CLOSING:
				/* If the timeout appears, interrupt will handle it
				 * timeout timer was disabled on 2023-02-02!
				 */
		
				motorClose();
				
				/* Simple way for 16-bit timer with 1024 pre-scaler and overflow vector
				 * which gives us 8 seconds.
				 * On entry to this state, the timer is started
				 * on every exit it's stopped.
				 */
				//TIMSK1 |= (1 << TOIE0);
				//TCCR1B |= (1 << CS12) | (1 << CS10);			// Start timer
				
				/* If the Emergency button was pressed */
				if ((!(INPUT_PIN & (1 << EMERGENCY_BTN_PIN))) & (cntEmergencyButton > chkLimit)) {
					//TCCR1B &= ~((1 << CS12) | (1 << CS10));		// Stop timer
					state = LOCKED;
				}
				
				/* If the Open button was pressed, change state to OPENING */
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & (cntOpenButton > chkLimit)) {
					//TCCR1B &= ~((1 << CS12) | (1 << CS10));		// Stop timer
					state = OPENING;
				}
				
				/* If the Closed door switch was hit */
				if ((!(INPUT_PIN & (1 << CLOSE_SWITCH_PIN))) & (cntCloseSwitch > chkLimit)) {
					motorStop();
					//TCCR1B &= ~((1 << CS12) | (1 << CS10));		// Stop timer
					state = CLOSED;
				}
				
				/* if photo-eye blocked */
				
				break;
			/*
			 * In ALARM state:
			 * - turn on only the Alarm LED, signaling there's something wrong
			 * - stop the motor
			 * - if the Close pushbutton was pressed:
			 *   - turn off the Alarm LED
			 *   //- start the motor to close the door
			 *   - restart the Timeout timer => if there's no Closed switch hit after timeout,
			 *     the state will change back to ALARM
			 *   - change the state to CLOSING
			 * And similar for the Open pushbutton
			 */
			//case ALARM:
				//OUTPUT_PORT |= (1 << ALARM_LED_PIN);
				//OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
				//OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
				//motorStop();
				//
				///* If the Close button was pressed */
				//if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & (cntCloseButton > chkLimit)) {
					//OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);
					////2023-01-31restartTimer();
					//state = CLOSING;
				//}
				//
				///* If the Open button was pressed */
				//if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & (cntOpenButton > chkLimit)) {	
					//OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);
					////2023-01-31restartTimer();
					//state = OPENING;
				//}
				//break;			
			default:
				break;
		} //end switch
	} //end while
} //end main

/*
 * Overflow vector for timeouts on 16-bit register.
 * Changed from 1024 pre-scaler to 256 (8 seconds -> 2 second).
 * Idea is to use this one also for LED blinking when in Opening or Closing state. (TBD)
 * Added alarmextraTime variable to change to the LOCKED state after some more time.
 */

//ISR(TIMER1_OVF_vect) {
	//state = LOCKED;
//}

//ISR(TIMER1_OVF_vect)
//{
	////static uint8_t extraTime, alarmextraTime = 0;
	//extraTime++;
	//alarmextraTime++;
	//if (extraTime > 5) {
		//if (alarmextraTime > 10) {
			//alarmextraTime = 0;
			//extraTime = 0;
			//motorStop();
			////2023-01-31stopTimer();
			//state = LOCKED;
		//} else {
			//motorStop();
			//state = ALARM;
		//}
	//}
//}
/*
 * This one is a little bit clumsy :/
 * Compare vector for button debounce on 8-bit timer.
 * With limit = 100, we get 50 ms.
 * Because of change in timetrs.c, where frequency is now 1kHz,
 * the limit value has to be adjusted. So 1 means 1 ms.
 */
ISR(TIMER0_COMPA_vect)
{
	static uint8_t cntLimit = 50;

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
}

/* Work in progress */
//ISR(TIMER0_COMPB_vect)
//{
	//static uint8_t lckCount;
	//static uint8_t lckcntLimit = 255;
	//lckCount++;
	//if (lckCount >= lckcntLimit) {
		//lock_solenoid();
		//lckCount = 0;
	//}
//}


/*
 * When we press the switch, the input pin is pulled to ground. Thus, we�re
 * waiting for the pin to go low.
 * The button is pressed when BUTTON1 bit is clear: if (!(PINB & (1<<BUTTON1)))
 
 
Debouncing is pretty straightforward, not sure why you're messing around so much.

Make it easy:
set up a timer irq that fires 200 times a sec
in the irq check switch A
if it is pressed:
	increment countA, limit the count to 40 max
else
	decrement countA, don't let it go below zero

Do the same thing for switch B (countB)
 
in your main code  if countA is > 20 then the switch is pressed & do what you want likewise for B
 */
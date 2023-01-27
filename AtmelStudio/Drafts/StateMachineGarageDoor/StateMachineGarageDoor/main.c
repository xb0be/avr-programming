/*
 * StateMachineGarageDoor.c
 *
 * Created: 12/11/2022 2:22:03 PM
 * Author : rludvik
 * Version: 0.1
 
Simple schematic. Left here just for fun.
For the full schematic see the schematics_VNH2SP30_schem.png

                       _____________
                      | ATmega328p  |
              _       |             |
 GND -------. | .---> | PB0     PC0 | ----> 330 Ohms --- Green LED (open)
              _       |             |
 GND -------. | .---> | PB1     PC1 | ----> 330 Ohms --- White LED (close)
                      |             | 
                      |         PC2 | ----> 330 Ohms --- Red LED (alarm)
                      |             | 					  
                      |         PC6 | ----> 330 Ohms --- Blue LED (RF)				  
 GND -------./ .----> | PB2         |
                      |         PC3 | ----> Motor IN1
 GND -------./ .----> | PB3     PC4 | ----> Motor IN2
             _        |             |
 GND ------. | .----> | PB4     PC5 | ----> Relay or MOSFET --- Electro magnetic lock (SOLENOID)
                      |_____________|
 Legend:
     _
 --. | .--      == Push button
 --./ .--       == Switch (touchguard)

Timer calculator: https://www.ee-diary.com/2021/07/programming-atmega328p-in-ctc-mode.html

 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
//#include <util/delay.h>
#include "settings.h"

/* Variables list:
 * state: initial state of the state machine
 * bpXXXX: true, if a certain button is pressed and debounced
 * RF: flag, if we are coming from RF (remote control) - TBD
 */
volatile char state = LOCKED;
volatile uint8_t extraTime, alarmextraTime = 0;
volatile uint8_t cntOpenButton, cntCloseButton, cntOpenSwitch, cntCloseSwitch, cntEmergencyButton = 0;
uint8_t chkLimit = 30;
//volatile char RF; //TBD

/* Declarations */
void debounceTimerStart();
void initTimer();
void startTimer();
void stopTimer();
void restartTimer();
void USART_Init();
void motorOpen();
void motorStop();
void motorClose();
void lock_solenoid();
void unlock_solenoid();

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
	initTimer();
	//Part for receiver over UART - code is in receiver.c TBD
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
				//OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
				//OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
				//OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);
				OUTPUT_PORT |= (1 << CLOSE_LED_PIN);
				OUTPUT_PORT |= (1 << ALARM_LED_PIN);
				motorStop();
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & (cntOpenButton > chkLimit)) {
					restartTimer();
					state = ONE;
				}
				break;
			case ONE:
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);
				OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
				OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);
				
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & (cntCloseButton > chkLimit)) {
					restartTimer();
					state = TWO;
				}
				break;
			case TWO:
				OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
				OUTPUT_PORT |= (1 << CLOSE_LED_PIN);
				OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);
				
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & (cntOpenButton > chkLimit)) {
					restartTimer();
					state = THREE;
				}
				break;
			case THREE:
				OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
				OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
				OUTPUT_PORT |= (1 << ALARM_LED_PIN);
				
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & (cntCloseButton > chkLimit)) {
					restartTimer();
					state = IDLE;
				}
				break;
			/*
			 * In IDLE state:
			 * - turn on the Power LED (TBD), signaling that the system in unlocked and can be used
			 * - check the status of the switches => if something weird is going on, go to the ALARM state.
			 */							
			case IDLE:
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);
				OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
				OUTPUT_PORT |= (1 << ALARM_LED_PIN);
				
				/* If the Open door switch is pressed */
				if ((!(INPUT_PIN & (1 << OPEN_SWITCH_PIN))) & (cntOpenSwitch > chkLimit)) {
					state = OPEN;
				}
				
				/* If the Closed door switch is pressed */
				if ((!(INPUT_PIN & (1 << CLOSE_SWITCH_PIN))) & (cntCloseSwitch > chkLimit)) {
					restartTimer();
					state = CLOSED;
				}
				
				/* If the Open button was pressed */
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & (cntOpenButton > chkLimit)) {
					OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
					//unlock_solenoid();		// TBD - SHALL BE ADDED HERE???
					restartTimer();
					state = OPENING;
				}
				
				/* If the Close button was pressed */
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & (cntCloseButton > chkLimit)) {
					OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
					restartTimer();
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
				OUTPUT_PORT |= (1 << CLOSE_LED_PIN);
				OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
				OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);

				/* If the Open button was pressed */
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & (cntOpenButton > chkLimit)) {
					OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
					restartTimer();
					state = OPENING;
				}
				break;
			case OPENING:
				//unlock_solenoid();			/* This also Enables Output Compare Match B Interrupt */
				
				/* If the timeout appears, interrupt will handle it */
				
				/* Right now just turn on both LEDs (opening + closing) to get a visual signal */
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);
				OUTPUT_PORT |= (1 << CLOSE_LED_PIN);
				OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);
				
				/* Start opening only if the Open door switch is NOT pressed ??? */
	//			if (((INPUT_PIN & (1 << OPEN_SWITCH_PIN))) & (cntOpenSwitch > chkLimit)) {
					motorOpen();
//				}
				
				/* If the Emergency button was pressed */
				if ((!(INPUT_PIN & (1 << EMERGENCY_BTN_PIN))) & (cntEmergencyButton > chkLimit)) {
					restartTimer();
					state = ALARM;
				}
				
				/* If the Close button was pressed, change state to CLOSING */
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & (cntCloseButton > chkLimit)) {
					restartTimer();
					state = CLOSING;
				}
				
				/* If the Open door switch was hit */
				if ((!(INPUT_PIN & (1 << OPEN_SWITCH_PIN))) & (cntOpenSwitch > chkLimit)) {
					stopTimer();
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
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);
				OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
				OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);
				
				/* If the Close button was pressed */
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & (cntCloseButton > chkLimit)) {
					OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
					restartTimer();
					state = CLOSING;
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
				motorClose();
				/* If the timeout appears, interrupt will handle it */
				
				/* Right now just turn on both LEDs (opening + closing) to get a visual signal */
				OUTPUT_PORT |= (1 << CLOSE_LED_PIN);
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);
				OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);
												
				/* If the Emergency button was pressed */
				if ((!(INPUT_PIN & (1 << EMERGENCY_BTN_PIN))) & (cntEmergencyButton > chkLimit)) {
					stopTimer();
					state = ALARM;
				}
				
				/* If the Open button was pressed, change state to OPENING */
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & (cntOpenButton > chkLimit)) {
					restartTimer();
					state = OPENING;
				}
				
				/* If the Closed door switch was hit */
				if ((!(INPUT_PIN & (1 << CLOSE_SWITCH_PIN))) & (cntCloseSwitch > chkLimit)) {
					motorStop();
					restartTimer();				/* In CLOSED, go to the LOCKED state after some time */
					state = CLOSED;
				}
				
				/* if photo-eye blocked */
					
				/* If the Closed door switch was NOT hit */

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
			case ALARM:
				OUTPUT_PORT |= (1 << ALARM_LED_PIN);
				OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
				OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
				motorStop();
				
				/* If the Close button was pressed */
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & (cntCloseButton > chkLimit)) {
					OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);
					restartTimer();
					state = CLOSING;
				}
				
				/* If the Open button was pressed */
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & (cntOpenButton > chkLimit)) {	
					OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);
					restartTimer();
					state = OPENING;
				}
				break;			
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
ISR(TIMER1_OVF_vect)
{
	//static uint8_t extraTime, alarmextraTime = 0;
	extraTime++;
	alarmextraTime++;
	if (extraTime > 5) {
		if (alarmextraTime > 10) {
			alarmextraTime = 0;
			extraTime = 0;
			motorStop();
			stopTimer();
			state = LOCKED;
		} else {
			motorStop();
			state = ALARM;
		}
	}
}
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
 * When we press the switch, the input pin is pulled to ground. Thus, we’re
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
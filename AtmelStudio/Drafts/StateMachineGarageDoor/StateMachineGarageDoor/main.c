/*
 * StateMachineGarageDoor.c
 *
 * Created: 12/11/2022 2:22:03 PM
 * Author : rludvik
 * Version: 0.1
 
Simple schematics:

                       _____________
                      | ATmega328p  |
              _       |             |
 GND -------. | .---> | PB0     PC0 | ----> 4k7 --- Green LED (open)
              _       |             |
 GND -------. | .---> | PB1     PC1 | ----> 4k7 --- White LED (close)
                      |             | 
                      |         PC2 | ----> 4k7 --- Red LED (alarm)
                      |             | 					  
                      |         PC3 | ----> 4k7 --- Blue LED (RF)				  
 GND -------./ .----> | PB2         |
                      |         PC4 | ----> Motor IN1
 GND -------./ .----> | PB3     PC5 | ----> Motor IN2
             _        |             |
 GND ------. | .----> | PB4     PC6 | ----> Relay --- Electro magnetic lock (SOLENIDE)
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
#include <util/delay.h>
#include "settings.h"

volatile uint8_t extraTime;					//Init extraTime
volatile uint8_t numOfAlarms;				//Number of alarms triggered
volatile char state = LOCKED;				//Initial state is Closed
volatile uint8_t count;						//Init of count variable for button deboucing
volatile uint8_t buttonPressed;				//Flag for pressed button
volatile uint8_t limit = 100;				//Used in button debounce ISR
//volatile char RF;							//Flag, if we are coming to state machine from RF


/* Declarations */
void USART_Init();
void motorOpen();
void motorStop();
void motorClose();
void lock_solenoid();
void unlock_solenoid();
void debounceTimerStart();
void initTimer();
void startTimer();
void stopTimer();
void restartTimer();


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
	INPUT_REG &= ~(1 << MOTOR_STOP_PIN);		//set MOTOR_STOP_PIN as input for the button
	INPUT_PORT |= (1 << MOTOR_STOP_PIN);		//enable pull-up resistor on button input	
	
	debounceTimerStart();
	initTimer();
	//Part for receiver over UART - code is in receiver.c
	USART_Init();
	sei();
	
	while(1)
	{
		switch (state)
		{
			/*
			 * In LOCKED state:
			 * - turn off all the LEDs
			 * - go through the ONE, TWO, and THREE states only after certain
			 *   combination of pushbuttons was pressed (currently: Open, Open, Close, Close)
			 */
			case LOCKED:
				OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
				OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
				OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & buttonPressed) {
					startTimer();
					state = ONE;
				}
				break;
			case ONE:
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & buttonPressed) {
					restartTimer();
					state = TWO;
				}
				break;
			case TWO:
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & buttonPressed) {
					restartTimer();
					state = THREE;
				}
				break;
			case THREE:
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & buttonPressed) {
					restartTimer();
					state = IDLE;
				}
				break;
			/*
			 * In IDLE state:
			 * - turn on the Power LED (TBD), signaling that system in unlocked and can be used
			 * - stop the Timeout timer
			 * - check the status of switches => if osmething weird is going on, go to ALARM state.
			 */							
			case IDLE:
				stopTimer();
				/* If the Open door switch is pressed */
				if ((!(INPUT_PIN & (1 << OPEN_SWITCH_PIN))) & buttonPressed) {
					state = OPEN;
				}
				/* If the Closed door switch is pressed */
				else if ((!(INPUT_PIN & (1 << CLOSE_SWITCH_PIN))) & buttonPressed) {
					state = CLOSED;
				}
				/* Anything else is an error => Alarm */
				else {
					state = ALARM;
				}
				break;
			/*
			 * In CLOSED state:
			 * - turn on only the Closed LED, signaling the door is fully closed
			 * - lock the Solenoid
			 * - start the Timeout timer => if there's no Open pushbutton pressed,
			 *   the state shall change to LOCKED (currently going through ALARM state twice)
			 * - if the Open pushbutton was pressed
			 *   +  turn off the Closed LED
			 *   + unlock the Solenoid
			 *   + start the motor to open the door
			 *   + start the Timeout timer
			 *   + change the state to OPENING
			 */
			case CLOSED:
				OUTPUT_PORT |= (1 << CLOSE_LED_PIN);
				OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
				OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);
				lock_solenoid();
				startTimer();
				/* If the Open button was pressed */
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & buttonPressed) {
						OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
						unlock_solenoid();
						motorOpen();
						startTimer();
						state = OPENING;
				}
				break;
			case OPENING:
				/* If the timeout appears, interrupt will handle it */
				
				/* Right now just turn on both LEDs (opening + closing) to get a visual signal */
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);
				OUTPUT_PORT |= (1 << CLOSE_LED_PIN);
				OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);
												
				/* If the Emergency button was pressed */
				if ((!(INPUT_PIN & (1 << MOTOR_STOP_PIN))) & buttonPressed) {
						motorStop();
						stopTimer();
						state = ALARM;
				}
				
				/* If the Close button was pressed, change state to CLOSING */
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & buttonPressed) {
						motorClose();
						restartTimer();
						state = CLOSING;
				}
				
				/* If the Open door switch was hit */
				if ((!(INPUT_PIN & (1 << OPEN_SWITCH_PIN))) & buttonPressed) {
						motorStop();
						stopTimer();
						state = OPEN;
				}
				
				/* If the open door switch was NOT hit TBD */
				break;
			/*
			 * In OPEN state:
			 * - turn on only the Open LED, signaling the door is fully open
			 * - if the Close pushbutton is pressed
			 *   - turn off the Open LED
			 *   - start the motor to close the door
			 *   - start the Timeout timer => if there's no Open switch hit after timeout,
			 *     the state will change to ALARM
			 *   - change the state to CLOSING
			 */
			case OPEN:
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);
				OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
				OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);
				
				//If the Close button was pressed
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & buttonPressed) {
						OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
						motorClose();
						startTimer();
						state = CLOSING;
				}
				break;
			case CLOSING:
				/* If the timeout appears, interrupt will handle it */
				
				/* Right now just turn on both LEDs (opening + closing) to get a visual signal */
				OUTPUT_PORT |= (1 << CLOSE_LED_PIN);
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);
				OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);
												
				/* If the Emergency button was pressed */
				if ((!(INPUT_PIN & (1 << MOTOR_STOP_PIN))) & buttonPressed) {
						motorStop();
						stopTimer();
						state = ALARM;
				}
				
				/* If the Open button was pressed, change state to OPENING */
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & buttonPressed) {
						motorOpen();
						restartTimer();
						state = OPENING;
				}
				
				/* If the Closed door switch was hit */
				if ((!(INPUT_PIN & (1 << CLOSE_SWITCH_PIN))) & buttonPressed) {
						motorStop();
						stopTimer();
						state = CLOSED;
				}
				
				/* if photo-eye blocked */
					
				/* If the Closed door switch was NOT hit */

				break;
			case ALARM:
				OUTPUT_PORT |= (1 << ALARM_LED_PIN);
				OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
				OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
				restartTimer();
				
				//If the Close button was pressed
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & buttonPressed) {
						OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);	//Turn off ALARM_LED_PIN
						motorClose();
						restartTimer();
						state = CLOSING;
				}
				
				//If the Open button was pressed
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & buttonPressed) {	
						OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);	//Turn off ALARM_LED_PIN
						motorOpen();
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
 * Added numOfAlarms variable - if the overflow gets triggered more then once, the state
 * machine goes to LOCKED state.
 */
ISR(TIMER1_OVF_vect)
{
	extraTime++;
	if (extraTime > 5) {
		extraTime = 0;
		numOfAlarms++;
		if (numOfAlarms <= 2) {
			state = ALARM;
		} else {
			state = LOCKED;
		}
	}
}
/*
 * Compare vector for button de-bounce on 8-bit timer.
 * With limit = 100, we get 50 ms.
 * Implemented for:
 * - Open and Close push buttons
 * - Fully open and fully closed switches
 * - Emergency motor stop
 */
ISR(TIMER0_COMPA_vect)
{
	if (!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) {
		count++;
		if(count > limit) {
			count = 0;
			buttonPressed = 1;
		} else {
			buttonPressed = 0;
		}	
	}
	if (!(INPUT_PIN & (1 << OPEN_BTN_PIN))) {
		count++;
		if(count > limit) {
			count = 0;
			buttonPressed = 1;
			} else {
			buttonPressed = 0;
		}
	}
	if (!(INPUT_PIN & (1 << OPEN_SWITCH_PIN))) {
		count++;
		if(count > limit) {
			count = 0;
			buttonPressed = 1;
			} else {
			buttonPressed = 0;
		}
	}
	if (!(INPUT_PIN & (1 << CLOSE_SWITCH_PIN))) {
		count++;
		if(count > limit) {
			count = 0;
			buttonPressed = 1;
			} else {
			buttonPressed = 0;
		}
	}
	if (!(INPUT_PIN & (1 << MOTOR_STOP_PIN))) {
		count++;
		if(count > limit) {
			count = 0;
			buttonPressed = 1;
			} else {
			buttonPressed = 0;
		}
	}	
}

/*
When we press the switch, the input pin is pulled to ground. Thus, we’re
waiting for the pin to go low.
The button is pressed when BUTTON1 bit is clear: if (!(PINB & (1<<BUTTON1)))

IN1		IN2		Motor Status
-------------------------------
LOW		LOW		Stops
LOW		HIGH	Clockwise
HIGH	LOW		Anti-Clockwise
HIGH	HIGH	Stops
*/
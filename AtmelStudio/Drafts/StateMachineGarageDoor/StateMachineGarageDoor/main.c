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

/* Set up 8-bit timer for debounce interrupt
 * The formula to calculate value of count to be loaded in OCR0A register (or OCR0B register) is:
 * C=(Fclk/2*N*Fw)?1 where:
 * - Fclk is the microcontroller CPU clock frequency
 * - N is the pre-scalar value and
 * - Fw is the output wave frequency
 * 
 * The above equation can be rewritten in the following form where we use the output square wave time period(Tw)
 * instead of the frequency(Fw).
 * 
 * C=(Fclk*Tw/2*N)?1
 * For example, if we want to create square wave signal with time period of Tw=100 us, we get:
 * C=(8000000*0.0001/2*8)-1 = 49 
 * 
 * So for 500 us time period, we get:
 * C=(8000000*0.0005/2*8)-1 = 249
 */
void debounceTimerStart() {
	OCR0A = 249;						//See formula above
	TCCR0A |= (1 << WGM01); 			//Set CTC mode
	TCCR0B = (1 << CS01);				//Set 8 prescaler
	TIMSK0 = (1 << OCIE0A);				//Timer/Counter0 Output Compare Match A Interrupt Enable
	//sei();							//Will be set in next (initTimer) function
}

/* Initialization of 16-bit timer, used for timeouts
Each overflow for 1024 prescaler at 8MHz lasts ~8.38 seconds
When the TOIE0 bit is written to one, and the I-bit in the status register is set (this is done by sei();),
the Timer/Counter overflow interrupt is enabled.
*/
void initTimer() {
	TIMSK1 |= (1 << TOIE0);
	//sei();
}

/*
 * Start the 16-bit timer.
 * Each overflow for 1024 pre-scaler at 8MHz lasts app 8 seconds.
 * Each overflow for 256 pre-scaler at 8MHz lasts app 2 seconds.
 */
void startTimer() {
	TCNT1 = 0;
	//TCCR1B |= (1 << CS12) | (1 << CS10);			//Start the timer. To stop it, just write 0 to these bits.
	TCCR1B |= (1 << CS12);							//256 pre-scaler => 2 seconds per overflow
}

/*
 * Stop the timer.
 */
void stopTimer() {
	//TCCR1B &= ~((1 << CS12) | (1 << CS10));
	TCCR1B &= ~(1 << CS12);
}

/*
 * Shortcut for stop+start timer.
 */
void restartTimer() {
	stopTimer();
	startTimer();
}

/*
Stop the motor.
*/
void motorStop() {								//Setting both pins to 0 makes the motor stop.
	OUTPUT_PORT &= ~(1 << MOTOR_IN1_PIN);
	OUTPUT_PORT &= ~(1 << MOTOR_IN2_PIN);
}

/*
Start turning the motor to open direction.		//Check directions in practice!!!!!!!!!!!!!!!!!!!!!!
*/
void motorOpen() {
	OUTPUT_PORT |= (1 << MOTOR_IN1_PIN);
	OUTPUT_PORT &= ~(1 << MOTOR_IN2_PIN);
}

/*
Start turning the motor to close direction.
*/
void motorClose() {
	OUTPUT_PORT &= ~(1 << MOTOR_IN1_PIN);
	OUTPUT_PORT |= (1 << MOTOR_IN2_PIN);
}

/*
Electro magnetic lock. Shall be released when the state switches to OPENING
and held back when the state switches to CLOSED.
*/

void unlock() {
	OUTPUT_PORT |= (1 << LOCK_PIN);
	_delay_ms(200);								//Wait a little bit before starting a motor
}

void lock() {
	_delay_ms(200);								//Wait a little bit before locking
	OUTPUT_PORT &= ~(1 << LOCK_PIN);
}

void USART_Init();								//Just a declaration

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
			case LOCKED:
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & buttonPressed) {		//If the Open button was pressed
					startTimer();							//Timer for timeout
					state = ONE;
				}
				break;
			case ONE:
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & buttonPressed) {		//If the Open button was pressed
					restartTimer();
					state = TWO;
				}
				break;
			case TWO:
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & buttonPressed) {		//If the Close button was pressed
					restartTimer();
					state = THREE;
				}
				break;
			case THREE:
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & buttonPressed) {		//If the Close button was pressed
					restartTimer();
					state = IDLE;
				}
				break;
			case IDLE:
				stopTimer();
				//If the Open door switch is pressed
				if ((!(INPUT_PIN & (1 << OPEN_SWITCH_PIN))) & buttonPressed) {
					state = OPEN;
				}
				//If the Closed door switch is pressed
				else if ((!(INPUT_PIN & (1 << CLOSE_SWITCH_PIN))) & buttonPressed) {
					state = CLOSED;
				}
				//Anything else is an error - Alarm
				else {
					state = ALARM;
				}
				break;
			case CLOSED:
				OUTPUT_PORT |= (1 << CLOSE_LED_PIN);			//Turn on Closed LED, signaling the door is fully closed
				OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);			//Turn off Open LED
				OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);			//Turn off Alarm LED
				lock();
				
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & buttonPressed) {		//If the Open button was pressed
						OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);	//Turn off CLOSE_LED_PIN
						unlock();							//Release the lock
						motorOpen();							//Start the motor to open the door
						startTimer();							//Timer for timeout
						state = OPENING;
				}
				break;
			case OPENING:
				//If the timeout appears, interrupt will handle it
				
				//Timer for blinking LED in the future version?
				//Right now I just turn on both LEDs (opening + closing ones)
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);
				OUTPUT_PORT |= (1 << CLOSE_LED_PIN);
				OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);
												
				//If the Emergency button was pressed
				if ((!(INPUT_PIN & (1 << MOTOR_STOP_PIN))) & buttonPressed) {
						motorStop();
						stopTimer();
						state = ALARM;
				}
				
				//If the Close button was pressed, change state to CLOSING
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & buttonPressed) {
						motorClose();					//Start the motor to close the door
						restartTimer();					//Direction was changed - reset the timeout timer
						state = CLOSING;
				}
				
				//If the Open door switch was hit
				if ((!(INPUT_PIN & (1 << OPEN_SWITCH_PIN))) & buttonPressed) {
						motorStop();					//Stop the motor
						stopTimer();					//Stop the timeout timer - there was no timeout!
						state = OPEN;
				}
				
				//If the open door switch was NOT hit
				//if (INPUT_PIN & (1 << OPEN_SWITCH_PIN)){
					//from = 4; //OPENING;
					//state = LED_ON;						//Switch the state to LED_ON
				//}
				break;
			case OPEN:
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);			//Turn on just OPEN_LED_PIN, signaling the door is open
				OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
				OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);
				
				//If the Close button was pressed
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & buttonPressed) {
						OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);	//Turn off OPEN_LED_PIN
						motorClose();					//Start the motor to close the door
						startTimer();					//Timer for timeout
						state = CLOSING;
				}
				break;
			case CLOSING:
				//If the timeout appears, interrupt will handle it
				
				//Timer for blinking LED in the future version?
				//Right now I just turn on both LEDs (opening + closing ones)
				OUTPUT_PORT |= (1 << CLOSE_LED_PIN);
				OUTPUT_PORT |= (1 << OPEN_LED_PIN);
				OUTPUT_PORT &= ~(1 << ALARM_LED_PIN);	//Turn off ALARM_LED_PIN
												
				//If the Emergency button was pressed
				if ((!(INPUT_PIN & (1 << MOTOR_STOP_PIN))) & buttonPressed) {
						motorStop();
						stopTimer();
						state = ALARM;
				}
				
				//If the Open button was pressed, change state to OPENING
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & buttonPressed) {
						motorOpen();
						restartTimer();
						state = OPENING;
				}
				
				//If the Closed door switch was hit
				if ((!(INPUT_PIN & (1 << CLOSE_SWITCH_PIN))) & buttonPressed) {
						motorStop();
						stopTimer();
						state = CLOSED;
				}
				
				//if photo-eye blocked
					////open door-start motor+
					//restartTimer();
					//state = OPENING;
					
				//If the Closed door switch was NOT hit
				//if (INPUT_PIN & (1 << CLOSE_SWITCH_PIN)){
					//from = 2; //CLOSING;
					//state = LED_ON;							//Switch the state to LED_ON
				//}
				break;
			case ALARM:
				OUTPUT_PORT |= (1 << ALARM_LED_PIN);			//Turn on just a red LED
				OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
				OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
				startTimer();
				
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
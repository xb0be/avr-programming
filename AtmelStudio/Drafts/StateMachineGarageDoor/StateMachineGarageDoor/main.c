/*
 * StateMachineGarageDoor.c
 *
 * Created: 12/11/2022 2:22:03 PM
 * Author : rludvik
 
Simple schematics:

                       _____________
                      | ATmega328p  |
              _       |             |
 GND -------. | .---- | PD0     PC0 | ----- 4k7 --- Green LED
              _       |             |
 GND -------. | .---- | PD1     PC1 | ----- 4k7 --- White LED
                      |             | 
                      |         PC0 | ----- 4k7 --- Red LED
 GND -------./ .----- | PD2         |
                      |             |
 GND -------./ .----- | PD3         |
                      |_____________|
 Legend:
     _
 --. | .--      == Push button
 --./ .--       == Switch (touchguard)
  
    
When we press the switch, the input pin is pulled to ground. Thus, we’re
waiting for the pin to go low.
The button is pressed when BUTTON1 bit is clear: if (!(PINB & (1<<BUTTON1)))
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include "settings.h"

volatile int extraTime = 0;					//Init extraTime
volatile char state = CLOSED;				//Initial state is Closed
char from;									//From which state we came for LED_ON - LED_OFF states

void initLEDs(){
	PORTC &= ~(1 << OPEN_LED_PIN);
	PORTC &= ~(1 << CLOSE_LED_PIN);
	PORTC &= ~(1 << ALARM_LED_PIN);
}

/* 16-bit timer, used for timeouts
Each overflow for 1024 prescaler at 8MHz lasts 8.388736 seconds
When the TOIE0 bit is written to one, and the I-bit in the status register is set (this is done by sei();),
the Timer/Counter overflow interrupt is enabled.
*/
void initTimer(){
	TIMSK1 = (1 << TOIE0);
	sei();
}

void startTimer(){
	TCCR1B = (1 << CS12) | (1 << CS10);			//Start the timer. To stop it, just write 0 to these bits.
}

void stopTimer(){
	TCCR1B &= ~((1 << CS12) | (1 << CS10));
}

void restartTimer(){
	stopTimer();
	startTimer();
}

void triggerAlarm(){
	initLEDs();								//Turn off all LEDs
	PORTC |= (1 << ALARM_LED_PIN);			//Turn on red LED
}

int main(void) {
	DDRC = 0xff; 							//LEDs and motors on PORTC as output
	DDRD &= ~(1 << OPEN_BTN_PIN);			//set OPEN_BTN_PIN as input for the button
	PORTD |= (1 << OPEN_BTN_PIN);			//enable pull-up resistor on button input
	DDRD &= ~(1 << CLOSE_BTN_PIN);			//set CLOSE_BTN_PIN as input for the button
	PORTD |= (1 << CLOSE_BTN_PIN);			//enable pull-up resistor on button input
	DDRD &= ~(1 << OPEN_SWITCH_PIN);		//set OPEN_SWITCH_PIN as input for the button
	PORTD |= (1 << OPEN_SWITCH_PIN);		//enable pull-up resistor on button input
	DDRD &= ~(1 << CLOSE_SWITCH_PIN);		//set CLOSE_SWITCH_PIN as input for the button
	PORTD |= (1 << CLOSE_SWITCH_PIN);		//enable pull-up resistor on button input

	initLEDs();
	initTimer();
	
	while(1)
	{
		switch (state)
		{
			case CLOSED:
				PORTC |= (1 << CLOSE_LED_PIN);			//Turn on CLOSE_LED_PIN, signaling the door is closed
				if (!(PIND & (1 << OPEN_BTN_PIN))){		//If the Open button was pressed
					_delay_ms(BOUNCETIME);
					if (!(PIND & (1 << OPEN_BTN_PIN))){
						//start motor to open the door
						startTimer();
						PORTC &= ~(1 << CLOSE_LED_PIN);	//Turn off CLOSE_LED_PIN
						state = OPENING;
					}
				}
				break;
			case OPENING:
				//If the timeout appears, interrupt will handle it
				
				//We don't want to wait for fully open door to be able to start closing it
				
				if (!(PIND & (1 << CLOSE_BTN_PIN))){	//If the Close button was pressed
					_delay_ms(BOUNCETIME);
					if (!(PIND & (1 << CLOSE_BTN_PIN))){
						//start motor to close the door
						restartTimer();
						state = CLOSING;
					}
				}
				if (!(PIND & (1 << OPEN_SWITCH_PIN))){	//If the Open door switch was hit
					_delay_ms(BOUNCETIME);
					if (!(PIND & (1 << OPEN_SWITCH_PIN))){
						//stop motor
						stopTimer();
						state = OPEN;
					}
				}
				if (PIND & (1 << OPEN_SWITCH_PIN)){		//If the open door switch was NOT hit
					from = 4; //OPENING;
					state = LED_ON;						//Switch the state to LED_ON
				}
				break;
			case OPEN:
				PORTC |= (1 << OPEN_LED_PIN);			//Turn on OPEN_LED_PIN, signaling the door is open
				if (!(PIND & (1 << CLOSE_BTN_PIN))){	//If the Close button was pressed
					_delay_ms(BOUNCETIME);
					if (!(PIND & (1 << CLOSE_BTN_PIN))){
						//start motor to close the door
						startTimer();
						PORTC &= ~(1 << OPEN_LED_PIN);	//Turn off OPEN_LED_PIN
						state = CLOSING;
					}
				}
				break;
			case CLOSING:
				//If the timeout appears, interrupt will handle it
				
				//We don't want to wait for fully closed door to be able to start opening it
				if (!(PIND & (1 << OPEN_BTN_PIN))){		//If the Open button was pressed
					_delay_ms(BOUNCETIME);
					if (!(PIND & (1 << OPEN_BTN_PIN))){
						//start motor to open the door
						restartTimer();
						state = OPENING;
					}
				}
				if (!(PIND & (1 << CLOSE_SWITCH_PIN))){	//If the Closed door switch was hit
					_delay_ms(BOUNCETIME);
						if (!(PIND & (1 << CLOSE_SWITCH_PIN))){
						//stop motor
						stopTimer();
						state = CLOSED;
					}
				}
				//if photo-eye blocked
					////open door-start motor+
					//restartTimer();
					//state = OPENING;
				if (PIND & (1 << CLOSE_SWITCH_PIN)){	//If the Closed door switch was NOT hit
					from = 2; //CLOSING;
					state = LED_ON;						//Switch the state to LED_ON
				}
				break;
			case ALARM:
				triggerAlarm();
				if (!(PIND & (1 << CLOSE_BTN_PIN))){	//If the Close button was pressed
					_delay_ms(BOUNCETIME);
					if (!(PIND & (1 << CLOSE_BTN_PIN))){
						//start motor to close the door
						startTimer();
						PORTC &= ~(1 << ALARM_LED_PIN);	//Turn off OPEN_LED_PIN
						state = CLOSING;
					}
				}
				if (!(PIND & (1 << OPEN_BTN_PIN))){		//If the Open button was pressed
					_delay_ms(BOUNCETIME);
					if (!(PIND & (1 << OPEN_BTN_PIN))){
						//start motor to open the door
						startTimer();
						PORTC &= ~(1 << ALARM_LED_PIN);	//Turn off CLOSE_LED_PIN
						state = OPENING;
					}
				}
				break;
			case LED_ON:
				if ((from == 2)){					//Switch the appropriate LED on
					PORTC |= (1 << CLOSE_LED_PIN);
				} else {
					PORTC |= (1 << OPEN_LED_PIN);
				}
				_delay_ms(100);
				state = LED_OFF;
				break;
			case LED_OFF:
				if ((from == 2)){					//Switch the appropriate LED off
					PORTC &= ~(1 << CLOSE_LED_PIN);
					} else {
					PORTC &= ~(1 << OPEN_LED_PIN);
				}
				_delay_ms(100);
				state = from;
				break;							
			default:
				break;
		} //end switch
	} //end while
} //end main

ISR(TIMER1_OVF_vect)
{
	extraTime++;
	if(extraTime > 1){					//2 * 8 seconds = 16 seconds
		//stop the motor
		triggerAlarm();
		state = ALARM;		
	}
	extraTime = 0;						//reset extraTime
}

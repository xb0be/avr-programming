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
 GND -------. | .---- | PD0     PC0 | ----- 4k7 --- Green LED (open)
              _       |             |
 GND -------. | .---- | PD1     PC1 | ----- 4k7 --- White LED (close)
                      |             | 
                      |         PC2 | ----- 4k7 --- Red LED (alarm)
                      |             | 					  
                      |         PC3 | ----- 4k7 --- Blue LED (RF)				  
 GND -------./ .----- | PD2         |
                      |             |
 GND -------./ .----- | PD3         |
                      |             |
 GND -------./ .----- | PD4         |
                      |_____________|
 Legend:
     _
 --. | .--      == Push button
 --./ .--       == Switch (touchguard)
  
    
When we press the switch, the input pin is pulled to ground. Thus, we’re
waiting for the pin to go low.
The button is pressed when BUTTON1 bit is clear: if (!(PINB & (1<<BUTTON1)))

RF with UART: https://www.youtube.com/watch?v=4TPwvxCTS4I
https://drive.google.com/drive/folders/1G8QLIVCWlWAjIYiDAR0_j9Lhmxq-QTzu
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
volatile char RF;						//Flag, if we are coming to state machine from RF


/*
Turn off all the LEDs.
*/
void initLEDs(){
	PORTC &= ~(1 << OPEN_LED_PIN);
	PORTC &= ~(1 << CLOSE_LED_PIN);
	PORTC &= ~(1 << ALARM_LED_PIN);
	PORTC &= ~(1 << RF_LED_PIN);
}

/* Initialization of 16-bit timer, used for timeouts
Each overflow for 1024 prescaler at 8MHz lasts 8.388736 seconds
When the TOIE0 bit is written to one, and the I-bit in the status register is set (this is done by sei();),
the Timer/Counter overflow interrupt is enabled.
*/
void initTimer(){
	TIMSK1 = (1 << TOIE0);
	sei();
}

/*
Sartp the timer running.
*/
void startTimer(){
	TCCR1B = (1 << CS12) | (1 << CS10);			//Start the timer. To stop it, just write 0 to these bits.
}

/*
Stop the timer running.
*/
void stopTimer(){
	TCCR1B &= ~((1 << CS12) | (1 << CS10));
}

/*
Shortcut for stop+start.
*/
void restartTimer(){
	stopTimer();
	startTimer();
}

/*
Stop the motor.
*/
void motorStop(){
	
}

/*
Start turning the motor to open direction.
*/
void motorOpen(){
	
}

/*
Start turning the motor to close direction.
*/
void motorClose(){

}

/*
If the alarm is triggered, first stop the motor, then turn off all the LEDs and turn on the red LED.
*/
void triggerAlarm(){
	motorStop();							//Turn off the motor
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
	DDRD &= ~(1 << MOTOR_STOP_PIN);			//set CLOSE_SWITCH_PIN as input for the button
	PORTD |= (1 << MOTOR_STOP_PIN);			//enable pull-up resistor on button input	
	
	initLEDs();
	initTimer();
	//Part for receiver over UART - code is in receiver.c
	Main_Init();
	USART_Init();
	
	while(1)
	{
		switch (state)
		{
			case CLOSED:
				PORTC |= (1 << CLOSE_LED_PIN);			//Turn on CLOSE_LED_PIN, signaling the door is closed
				if (!(PIND & (1 << OPEN_BTN_PIN))){		//If the Open button was pressed
					_delay_ms(BOUNCETIME);
					if (!(PIND & (1 << OPEN_BTN_PIN))){
						motorOpen();					//Start the motor to open the door
						startTimer();
						PORTC &= ~(1 << CLOSE_LED_PIN);	//Turn off CLOSE_LED_PIN
						state = OPENING;
					}
				}
				break;
			case OPENING:
				//If the timeout appears, interrupt will handle it
				
				//If the Emergency button was pressed
				if (!(PIND & (1 << MOTOR_STOP_PIN))){
					_delay_ms(BOUNCETIME);
					if (!(PIND & (1 << MOTOR_STOP_PIN))){
						stopTimer();
						state = ALARM;
					}
				}
				//If the Close button was pressed, change state to CLOSING
				if (!(PIND & (1 << CLOSE_BTN_PIN))){
					_delay_ms(BOUNCETIME);
					if (!(PIND & (1 << CLOSE_BTN_PIN))){
						motorClose();					//Start the motor to close the door
						restartTimer();
						state = CLOSING;
					}
				}
				//If the Open door switch was hit
				if (!(PIND & (1 << OPEN_SWITCH_PIN))){
					_delay_ms(BOUNCETIME);
					if (!(PIND & (1 << OPEN_SWITCH_PIN))){
						motorStop();					//Stop the motor
						stopTimer();
						state = OPEN;
					}
				}
				//If the open door switch was NOT hit
				if (PIND & (1 << OPEN_SWITCH_PIN)){
					from = 4; //OPENING;
					state = LED_ON;						//Switch the state to LED_ON
				}
				break;
			case OPEN:
				PORTC |= (1 << OPEN_LED_PIN);			//Turn on OPEN_LED_PIN, signaling the door is open
				//If the Close button was pressed
				if (!(PIND & (1 << CLOSE_BTN_PIN))){
					_delay_ms(BOUNCETIME);
					if (!(PIND & (1 << CLOSE_BTN_PIN))){
						motorClose();					//Start the motor to close the door
						startTimer();
						PORTC &= ~(1 << OPEN_LED_PIN);	//Turn off OPEN_LED_PIN
						state = CLOSING;
					}
				}
				break;
			case CLOSING:
				//If the timeout appears, interrupt will handle it
				
				//If the Emergency button was pressed
				if (!(PIND & (1 << MOTOR_STOP_PIN))){
					_delay_ms(BOUNCETIME);
					if (!(PIND & (1 << MOTOR_STOP_PIN))){
						stopTimer();
						state = ALARM;
					}
				}
				//If the Open button was pressed, change state to OPENING
				if (!(PIND & (1 << OPEN_BTN_PIN))){
					_delay_ms(BOUNCETIME);
					if (!(PIND & (1 << OPEN_BTN_PIN))){
						motorOpen();				//Start the motor to open the door
						restartTimer();
						state = OPENING;
					}
				}
				//If the Closed door switch was hit
				if (!(PIND & (1 << CLOSE_SWITCH_PIN))){
					_delay_ms(BOUNCETIME);
						if (!(PIND & (1 << CLOSE_SWITCH_PIN))){
						motorStop();				//Stop the motor
						stopTimer();
						state = CLOSED;
					}
				}
				//if photo-eye blocked
					////open door-start motor+
					//restartTimer();
					//state = OPENING;
				//If the Closed door switch was NOT hit
				if (PIND & (1 << CLOSE_SWITCH_PIN)){
					from = 2; //CLOSING;
					state = LED_ON;						//Switch the state to LED_ON
				}
				break;
			case ALARM:
				triggerAlarm();
				//If the Close button was pressed
				if (!(PIND & (1 << CLOSE_BTN_PIN))){
					_delay_ms(BOUNCETIME);
					if (!(PIND & (1 << CLOSE_BTN_PIN))){
						motorClose();					//Start motor to close the door
						startTimer();
						PORTC &= ~(1 << ALARM_LED_PIN);	//Turn off OPEN_LED_PIN
						state = CLOSING;
					}
				}
				//If the Open button was pressed
				if (!(PIND & (1 << OPEN_BTN_PIN))){	
					_delay_ms(BOUNCETIME);
					if (!(PIND & (1 << OPEN_BTN_PIN))){
						motorOpen();					//Start motor to open the door
						startTimer();
						PORTC &= ~(1 << ALARM_LED_PIN);	//Turn off CLOSE_LED_PIN
						state = OPENING;
					}
				}
				break;
			case LED_ON:
				if ((RF == 1)){							//Signal came from RF
					PORTC |= (1 << RF_LED_PIN);			//Switch the blue LED off
				}
				if ((from == 2)){						//Switch the appropriate LED on
					PORTC |= (1 << CLOSE_LED_PIN);
				} else {
					PORTC |= (1 << OPEN_LED_PIN);
				}
				_delay_ms(100);
				state = LED_OFF;
				break;
			case LED_OFF:
				if ((RF == 1)){							//Signal came from RF
					PORTC &= ~(1 << RF_LED_PIN);		//Switch the blue LED off
				}
				if ((from == 2)){						//Switch the appropriate LED off
					PORTC &= ~(1 << CLOSE_LED_PIN);
					} else {
					PORTC &= ~(1 << OPEN_LED_PIN);
				}
				_delay_ms(100);
				RF = 0;									//Reset RF flag
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
		triggerAlarm();
		state = ALARM;		
	}
	extraTime = 0;						//reset extraTime
}


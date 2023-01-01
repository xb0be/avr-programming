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
Timer calculator: https://www.ee-diary.com/2021/07/programming-atmega328p-in-ctc-mode.html

 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include "settings.h"

volatile uint8_t extraTime = 0;				//Init extraTime
volatile char state = CLOSED;				//Initial state is Closed
volatile uint8_t count = 0;					//Init of count variable for button deboucing
volatile uint8_t buttonPressed = 0;			//Flag for pressed button
volatile uint8_t limit = 100;				//Used in button debounce ISR
//volatile char RF;							//Flag, if we are coming to state machine from RF

/* Set up 8-bit timer for debounce interrupt
The formula to calculate value of count to be loaded in OCR0A register (or OCR0B register) is:
C=(Fclk/2*N*Fw)?1 where:
- Fclk is the microcontroller CPU clock frequency
- N is the pre-scalar value and
- Fw is the output wave frequency

The above equation can be rewritten in the following form where we use the output square wave time period(Tw)
instead of the frequency(Fw).

C=(Fclk*Tw/2*N)?1
For example, if we want to create square wave signal with time period of Tw=100 us, we get:
C=(8000000*0.0001/2*8)-1 = 49 

So for 500 us time period, we get:
C=(8000000*0.0005/2*8)-1 = 249

*/
void debounceTimerStart() {
	OCR0A = 249;						//See formula
	TCCR0A |= (1 << WGM01); 			//Set CTC mode
	TCCR0B = (1 << CS01);				//Set 8 prescaler
	TIMSK0 = (1 << OCIE0A);				//Timer/Counter0 Output Compare Match A Interrupt Enable
	//sei();							//Will be set in next (initTimer) function
}

/* Initialization of 16-bit timer, used for timeouts
Each overflow for 1024 prescaler at 8MHz lasts 8.388736 seconds
When the TOIE0 bit is written to one, and the I-bit in the status register is set (this is done by sei();),
the Timer/Counter overflow interrupt is enabled.
*/
void initTimer(){
	TIMSK1 |= (1 << TOIE0);
	sei();
}

/*
Start the 16-bit timer.
Each overflow for 1024 prescaler at 8MHz lasts 8.388736 seconds
*/
void startTimer(){
	TCNT1 = 0;
	TCCR1B |= (1 << CS12) | (1 << CS10);			//Start the timer. To stop it, just write 0 to these bits.
}

/*
Stop the timer.
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
	//DDRD &= ~(1 << MOTOR_STOP_PIN);			//set MOTOR_STOP_PIN as input for the button
	//PORTD |= (1 << MOTOR_STOP_PIN);			//enable pull-up resistor on button input	
	
	debounceTimerStart();
	initTimer();							//sei() is included here
	//Part for receiver over UART - code is in receiver.c
//	Main_Init();
//	USART_Init();
	
	while(1)
	{
		switch (state)
		{
			case CLOSED:
				PORTC |= (1 << CLOSE_LED_PIN);			//Turn on CLOSE_LED_PIN, signaling the door is closed
				PORTC &= (~1 << OPEN_LED_PIN);
				PORTC &= ~(1 << ALARM_LED_PIN);			//Turn off ALARM_LED_PIN
				
				if ((!(PIND & (1 << OPEN_BTN_PIN))) & buttonPressed) {		//If the Open button was pressed
						PORTC &= ~(1 << CLOSE_LED_PIN);	//Turn off CLOSE_LED_PIN
						motorOpen();					//Start the motor to open the door
						startTimer();
						state = OPENING;
				}
				break;
			case OPENING:
				//If the timeout appears, interrupt will handle it
				
				//Timer for blinking LED?
				PORTC |= (1 << OPEN_LED_PIN);
				PORTC |= (1 << CLOSE_LED_PIN);
				PORTC &= ~(1 << ALARM_LED_PIN);	//Turn off ALARM_LED_PIN
												
				//If the Emergency button was pressed
				//if ((!(PIND & (1 << MOTOR_STOP_PIN))) & buttonPressed) {
						//stopTimer();
						//state = ALARM;
				//}
				
				//If the Close button was pressed, change state to CLOSING
				if ((!(PIND & (1 << CLOSE_BTN_PIN))) & buttonPressed) {
						motorClose();					//Start the motor to close the door
						restartTimer();
						state = CLOSING;
				}
				
				//If the Open door switch was hit
				if ((!(PIND & (1 << OPEN_SWITCH_PIN))) & buttonPressed) {
						motorStop();					//Stop the motor
						stopTimer();
						state = OPEN;
				}
				
				//If the open door switch was NOT hit
				//if (PIND & (1 << OPEN_SWITCH_PIN)){
					//from = 4; //OPENING;
					//state = LED_ON;						//Switch the state to LED_ON
				//}
				break;
			case OPEN:
				PORTC |= (1 << OPEN_LED_PIN);			//Turn on OPEN_LED_PIN, signaling the door is open
				PORTC &= ~(1 << CLOSE_LED_PIN);
				PORTC &= ~(1 << ALARM_LED_PIN);	//Turn off ALARM_LED_PIN
				
				//If the Close button was pressed
				if ((!(PIND & (1 << CLOSE_BTN_PIN))) & buttonPressed) {
						PORTC &= ~(1 << OPEN_LED_PIN);	//Turn off OPEN_LED_PIN
						motorClose();					//Start the motor to close the door
						startTimer();
						state = CLOSING;
				}
				break;
			case CLOSING:
				//If the timeout appears, interrupt will handle it
				
				//Timer for blinking LED?
				PORTC |= (1 << CLOSE_LED_PIN);
				PORTC |= (1 << OPEN_LED_PIN);
				PORTC &= ~(1 << ALARM_LED_PIN);	//Turn off ALARM_LED_PIN
												
				//If the Emergency button was pressed
				//if ((!(PIND & (1 << MOTOR_STOP_PIN))) & buttonPressed) {
						//stopTimer();
						//state = ALARM;
				//}
				
				//If the Open button was pressed, change state to OPENING
				if ((!(PIND & (1 << OPEN_BTN_PIN))) & buttonPressed) {
						motorOpen();				//Start the motor to open the door
						restartTimer();
						state = OPENING;
				}
				
				//If the Closed door switch was hit
				if ((!(PIND & (1 << CLOSE_SWITCH_PIN))) & buttonPressed) {
						motorStop();				//Stop the motor
						stopTimer();
						state = CLOSED;
				}
				
				//if photo-eye blocked
					////open door-start motor+
					//restartTimer();
					//state = OPENING;
					
				//If the Closed door switch was NOT hit
				//if (PIND & (1 << CLOSE_SWITCH_PIN)){
					//from = 2; //CLOSING;
					//state = LED_ON;						//Switch the state to LED_ON
				//}
				break;
			case ALARM:
				PORTC |= (1 << ALARM_LED_PIN);			//Turn on red LED
				PORTC &= ~(1 << OPEN_LED_PIN);	//Turn off OPEN_LED_PIN
				PORTC &= ~(1 << CLOSE_LED_PIN);	//Turn off OPEN_LED_PIN
				stopTimer();
				
				//If the Close button was pressed
				if ((!(PIND & (1 << CLOSE_BTN_PIN))) & buttonPressed) {
						PORTC &= ~(1 << ALARM_LED_PIN);	//Turn off ALARM_LED_PIN
						motorClose();					//Start motor to close the door
						startTimer();
						state = CLOSING;
				}
				
				//If the Open button was pressed
				if ((!(PIND & (1 << OPEN_BTN_PIN))) & buttonPressed) {	
						PORTC &= ~(1 << ALARM_LED_PIN);	//Turn off ALARM_LED_PIN
						motorOpen();					//Start motor to open the door
						startTimer();
						state = OPENING;
				}
				break;			
			default:
				break;
		} //end switch
	} //end while
} //end main

ISR(TIMER1_OVF_vect)
{
	extraTime++;
	if (extraTime > 0) {				//0 = 8 seconds, 1 = 16 seconds etc.
		extraTime = 0;					//reset extraTime
		state = ALARM;
	}
}

ISR(TIMER0_COMPA_vect)
{
	if (!(PIND & (1 << CLOSE_BTN_PIN))) {
		count++;
		if(count > limit) {				//Triggers every 500 us, so with 100 we get 50 ms 
			count = 0;
			buttonPressed = 1;			//The button is pressed
		} else {
			buttonPressed = 0;			//The button is not pressed (yet)
		}	
	}
	if (!(PIND & (1 << OPEN_BTN_PIN))) {
		count++;
		if(count > limit) {				//Triggers every 500 us, so with 100 we get 50 ms
			count = 0;
			buttonPressed = 1;			//The button is pressed
			} else {
			buttonPressed = 0;			//The button is not pressed (yet)
		}
	}
	if (!(PIND & (1 << OPEN_SWITCH_PIN))) {
		count++;
		if(count > limit) {				//Triggers every 500 us, so with 100 we get 50 ms
			count = 0;
			buttonPressed = 1;			//The button is pressed
			} else {
			buttonPressed = 0;			//The button is not pressed (yet)
		}
	}
	if (!(PIND & (1 << CLOSE_SWITCH_PIN))) {
		count++;
		if(count > limit) {				//Triggers every 500 us, so with 100 we get 50 ms
			count = 0;
			buttonPressed = 1;			//The button is pressed
			} else {
			buttonPressed = 0;			//The button is not pressed (yet)
		}
	}		
}
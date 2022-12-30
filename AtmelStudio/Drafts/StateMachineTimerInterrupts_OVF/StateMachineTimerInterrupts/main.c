/*
 * StateMachineTimerInterrupts.c
 *
 * Created: 12/5/2022 6:42:53 PM
 * Author : rludvik
 
 Just a simple, timer based state machine on ATmega328p. There are 2 states.
 In state 1 LED1 is on X seconds.
 In state 2 LED2 is on X/2 seconds.
 
 Schematics:
 ATmega
    PC0 ----> 4k7 resistor ---> LED1 ---> GND
	PC1 ----> 4k7 resistor ---> LED2 ---> GND
 
 Resources:
 https://www.youtube.com/watch?v=cAui6116XKc&list=PLA6BB228B08B03EDD&index=6 (timers)
 https://sites.google.com/site/ka7ehkengineeringsite/home/statemachines (state machine)
 https://eleccelerator.com/avr-timer-calculator/ (timer calculator)
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

#define firstLedOn 1
#define secondLedOn 2

int extraTime = 0;
char state = firstLedOn;

int main(void)
{
	DDRC = 0xff; 							//2 LEDs on PORTC as output
	/* 8-bit timer
	TIMSK0 = (1 << TOIE0);
	sei();
	TCCR0B = (1 << CS02) | (1 << CS00);	
	//TCCR0B = (1 << CS01);					
	//TCCR0B = (1 << CS00);					
	*/
	
	/* 16-bit timer
	Each overflow for 1024 prescaler at 8MHz lasts 8.388736 seconds
	*/
	TIMSK1 = (1 << TOIE0);
	sei();
	TCCR1B = (1 << CS12) | (1 << CS10);
	//TCCR0B = (1 << CS11);					
	//TCCR0B = (1 << CS10);					
    while (1)
    {
		//just make interrupts do everything
    }
}

ISR(TIMER1_OVF_vect)
{
	extraTime++;
	if(extraTime > 0)							
	{
		switch (state) {
			case firstLedOn:
				PORTC |= (1 << PC0);
				PORTC &= ~(1 << PC1);
				//OCR0A = 200;					
				state = secondLedOn;
				break;
			case secondLedOn:
				PORTC &= ~(1 << PC0);
				PORTC |= (1 << PC1);
				//OCR0A = 100;					
				state = firstLedOn;
				break;
			default:
			break;
		}		
		extraTime = 0;						//reset extraTime
	}
}


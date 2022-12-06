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
 
 */ 
#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

#define firstLedOn 1
#define secondLedOn 2

//init
int extraTime = 0;
int state = firstLedOn;

int main(void)
{
	DDRC = 0xff; 							//2 LEDs on PORTC as output
	TCCR0A = (1 << WGM01); 					//Set CTC bit
	OCR0A = 100;							//Interrupt on every 100th => 100 us
	TIMSK0 = (1 << OCIE0A);
	sei();
	//TCCR0B = (1 << CS02) | (1 << CS00);	//start at 1024 prescaler
	//TCCR0B = (1 << CS00);					//no prescaler => 125 us
	TCCR0B = (1 << CS01);					//8 prescaler => 1 us
		
    while (1)
    {
		//just make interrupts do everything
    }
}

ISR(TIMER0_COMPA_vect)
{
	extraTime++;
	if(extraTime > 10000)					//10000 * 1 us = 10 ms		
	{
		switch (state) {
			case firstLedOn:
				PORTC |= (1 << PC0);
				PORTC &= ~(1 << PC1);
				OCR0A = 200;					//10000 * 200 (from OCR0A) = 2000000 * 1 us => 2 s
				state = secondLedOn;
				break;
			case secondLedOn:
				PORTC &= ~(1 << PC0);
				PORTC |= (1 << PC1);
				OCR0A = 100;					//10000 * 100 (from OCR0A) = 1000000 * 1 us => 1 s
				state = firstLedOn;
				break;
			default:
				PORTC |= (1 << PC0);			//if in any case we come here, switch on both LEDs
				PORTC |= (1 << PC1);
				state = firstLedOn;
				break;
		}		
		extraTime = 0;						//reset extraTime
	}

}


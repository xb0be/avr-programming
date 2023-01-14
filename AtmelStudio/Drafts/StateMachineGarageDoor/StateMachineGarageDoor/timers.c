
#define F_CPU 8000000UL
#include <avr/io.h>
#include "settings.h"


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
	//sei();							//Will be set in main
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

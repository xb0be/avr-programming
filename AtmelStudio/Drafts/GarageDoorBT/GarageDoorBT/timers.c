
#define F_CPU 8000000UL
#include <avr/io.h>
#include "settings.h"


/* Set up 8-bit timer
 * OCR0A is used for debounce interrupt for all buttons
 * OCR0B is used for releasing the lock (after it's held in transition from CLOSED to OPENING
 * so I don't need another switch/signal for when to release it).
 * Because it has to be kept unlocked for some time, so the door starts to open and do some progress.
 
 * The formula to calculate value of count to be loaded in OCR0A register (or OCR0B register) is:
 * C=(Fclk/2*N*Fw)-1 where:
 * - Fclk is the microcontroller CPU clock frequency
 * - N is the pre-scalar value and
 * - Fw is the output wave frequency
 * 
 * The above equation can be rewritten in the following form where we use the output square wave time period(Tw)
 * instead of the frequency(Fw).
 * 
 * C=(Fclk*Tw/2*N)-1
 * For example, if we want to create square wave signal with time period of Tw=100 us, we get:
 * C=(8000000*0.0001/2*8)-1 = 49 
 * 
 * So for 500 us time period, we get:
 * C=(8000000*0.0005/2*8)-1 = 249
 *
 * So with 256 pre-scaler_
 * OCR0A = 15 => we get 1 ms square wave time period
 */
void debounceTimerStart() {
	OCR0A = 15;
	TCCR0A |= (1 << WGM01); 			//Set CTC mode
	TCCR0B = (1 << CS02);				//Set 256 prescaler
	TIMSK0 = (1 << OCIE0A);				//Timer/Counter0 Output Compare Match A Interrupt Enable
}

/*
*		OBSOLETE 16-BIT TIMER CODE BELOW
*/

/* Initialization of 16-bit timer, used for timeouts
Each overflow for 1024 prescaler at 8MHz lasts ~8.38 seconds
When the TOIE0 bit is written to one, and the I-bit in the status register is set (this is done by sei();),
the Timer/Counter overflow interrupt is enabled.
*/
void initTimer() {
	//2023-01-31TIMSK1 |= (1 << TOIE0);
}

/*
 * Start the 16-bit timer.
 * Each overflow for 1024 pre-scaler at 8MHz lasts app 8 seconds.
 * Each overflow for 256 pre-scaler at 8MHz lasts app 2 seconds.
 */
void startTimer() {
	//2023-01-31TIMSK1 |= (1 << TOIE0);
	//2023-01-312023-01-31TCCR1B |= (1 << CS12);							//256 pre-scaler => 2 seconds per overflow
	//TCNT1 = 0;
}

/*
 * Stop the timer.
 */
void stopTimer() {
	//2023-01-31TCCR1B &= ~(1 << CS12);
}

/*
 * Shortcut for stop+start timer.
 */
void restartTimer() {
	//extern uint8_t extraTime, alarmextraTime;
	//stopTimer();
	//startTimer();
	//extraTime = 0;
	//alarmextraTime = 0;
}

/* cmds for 1024 pre-scaler
 * 	//TCCR1B |= (1 << CS12) | (1 << CS10);			//from startTimer
 * 	//TCCR1B &= ~((1 << CS12) | (1 << CS10));		//from StopTimer
 */
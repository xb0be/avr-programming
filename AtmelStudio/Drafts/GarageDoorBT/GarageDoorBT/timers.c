
/* Timer calculator: https://www.ee-diary.com/2021/07/programming-atmega328p-in-ctc-mode.html */

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include "settings.h"
#include <avr/io.h>

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

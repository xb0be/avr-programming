/*
 * Electro magnetic lock. Shall be released (unlock) when the state switches to OPENING
 * and held back (locked) when the state switches to CLOSED.
 * 
 * There is a short delay introduced after it's unlocked and the motor is started
 * and before the real locking
 */

#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "settings.h"

void unlock_solenoid() {
	OUTPUT_PORT |= (1 << LOCK_PIN);
	TIMSK0 = (1 << OCIE0B);				/* Enable Output Compare Match B Interrupt */
}

void lock_solenoid() {
	OUTPUT_PORT &= ~(1 << LOCK_PIN);
}
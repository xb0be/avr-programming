
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "settings.h"

/*
Electro magnetic lock. Shall be released when the state switches to OPENING
and held back when the state switches to CLOSED.
*/

void unlock_solenoid() {
	OUTPUT_PORT |= (1 << LOCK_PIN);
	_delay_ms(200);								//Wait a little bit before starting a motor
}

void lock_solenoid() {
	_delay_ms(200);								//Wait a little bit before locking
	OUTPUT_PORT &= ~(1 << LOCK_PIN);
}
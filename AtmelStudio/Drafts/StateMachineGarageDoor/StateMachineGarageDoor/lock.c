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
	//_delay_ms(200);					/* Solve this in main. Wait a little bit before starting a motor */
}

void lock_solenoid() {
	//_delay_ms(200);					/* Solve this in main. Wait a little bit before locking */
	OUTPUT_PORT &= ~(1 << LOCK_PIN);
}
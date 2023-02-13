/* 
 * IN1		IN2		Motor Status
 * -------------------------------
 * LOW		LOW		Stops
 * LOW		HIGH	Clockwise
 * HIGH		LOW		Anti-Clockwise
 * HIGH		HIGH	Stops
 */

#ifndef F_CPU
#define F_CPU 8000000UL
#endif
#include <avr/io.h>
#include "settings.h"

/* Stop the motor. */
void motorStop() {
	OUTPUT_PORT &= ~(1 << MOTOR_IN1_PIN);
	OUTPUT_PORT &= ~(1 << MOTOR_IN2_PIN);
}

/* Start turning the motor to close direction */
void motorOpen() {
	OUTPUT_PORT &= ~(1 << MOTOR_IN1_PIN);
	OUTPUT_PORT |= (1 << MOTOR_IN2_PIN);
}

/* Start turning the motor to open direction. Check directions in practice!!! */
void motorClose() {
	OUTPUT_PORT |= (1 << MOTOR_IN1_PIN);
	OUTPUT_PORT &= ~(1 << MOTOR_IN2_PIN);
}


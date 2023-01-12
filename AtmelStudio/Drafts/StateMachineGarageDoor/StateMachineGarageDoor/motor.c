
#define F_CPU 8000000UL
#include <avr/io.h>
#include "settings.h"

/*
Stop the motor.
*/
void motorStop() {								//Setting both pins to 0 makes the motor stop.
	OUTPUT_PORT &= ~(1 << MOTOR_IN1_PIN);
	OUTPUT_PORT &= ~(1 << MOTOR_IN2_PIN);
}

/*
Start turning the motor to open direction.		//Check directions in practice!!!!!!!!!!!!!!!!!!!!!!
*/
void motorOpen() {
	OUTPUT_PORT |= (1 << MOTOR_IN1_PIN);
	OUTPUT_PORT &= ~(1 << MOTOR_IN2_PIN);
}

/*
Start turning the motor to close direction.
*/
void motorClose() {
	OUTPUT_PORT &= ~(1 << MOTOR_IN1_PIN);
	OUTPUT_PORT |= (1 << MOTOR_IN2_PIN);
}

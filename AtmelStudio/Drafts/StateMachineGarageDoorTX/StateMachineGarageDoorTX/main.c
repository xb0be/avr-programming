/*
 * StateMachineGarageDoorTX.c
 *
 * Created: 12/29/2022 1:24:27 PM
 * Author : rludvik
 */ 
//Code for Transmitter Model: FS1000A, Transmitting Frequency: 433.92MHz

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "settings.h"

volatile uint8_t cntOpenButton, cntCloseButton, cntEmergencyButton = 0;
uint8_t chkLimit = 30;
char state = IDLE;

/* Declarations */
void debounceTimerStart();
void initTimer();
void startTimer();
void stopTimer();
void restartTimer();
//void USART_Init();


//Initializing UART
void USART_Init(void) {
	//Setting the baud rate is done by writing to the UBRR0H and UBRR0L registers
	UBRR0H = (UBRRVAL >> 8);				//high byte
	UBRR0L = (uint8_t)UBRRVAL;				//low byte
	
	//Set data frame format: asynchronous mode, no parity, 1 stop bit, 8 bit size
	UCSR0C = (0 << UMSEL01) | (0 << UMSEL00) | (0 << UPM01) | (0 << UPM00) | (0 << USBS0) | (0 << UCSZ02) | (1 << UCSZ01) | (1 << UCSZ00);
	//Enable Receiver Interrupt on receive complete
	UCSR0B = (1 << RXEN0) | (1 << RXCIE0);
}

// Transmit data(byte) function
void USART_vSendByte(uint8_t u8Data) {
	//Wait for transmit buffer to be empty
	while((UCSR0A & (1 << UDRE0)) == 0);
	//Transmit data - load data into transmit register
	UDR0 = u8Data;
}

// Send packet of data 
void Send_Packet(uint8_t addr, uint8_t cmd)
{
	USART_vSendByte(SYNC);					//Send synchronization byte
	USART_vSendByte(addr);					//Send receiver address
	USART_vSendByte(cmd);					//Send increment command
	USART_vSendByte((addr+cmd));			//Send checksum
}

int main(void)
{
	OUTPUT_REG = 0xff; 							//LEDs on PORTC as output
	INPUT_REG &= ~(1 << OPEN_BTN_PIN);			//set OPEN_BTN_PIN as input for the button
	INPUT_PORT |= (1 << OPEN_BTN_PIN);			//enable pull-up resistor on button input
	INPUT_REG &= ~(1 << CLOSE_BTN_PIN);			//set CLOSE_BTN_PIN as input for the button
	INPUT_PORT |= (1 << CLOSE_BTN_PIN);			//enable pull-up resistor on button input
	INPUT_REG &= ~(1 << MOTOR_STOP_BTN_PIN);			//set CLOSE_SWITCH_PIN as input for the button
	INPUT_PORT |= (1 << MOTOR_STOP_BTN_PIN);			//enable pull-up resistor on button input

	debounceTimerStart();
	USART_Init();
	sei();
	
	while(1)
	{
		switch (state){
			case IDLE:
				OUTPUT_PORT &= ~(1 << RF_LED_PIN);			//turn off the LEDs and go to IDLE state				
				if ((!(INPUT_PIN & (1 << OPEN_BTN_PIN))) & (cntOpenButton > chkLimit)) {
					Send_Packet(RADDR, MOTOR_OPEN_CMD);		//send Open cmd
					state = OPENING;					//switch state
				}
				if ((!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) & (cntCloseButton > chkLimit)) {				
					Send_Packet(RADDR, MOTOR_CLOSE_CMD);		//send Close cmd
					state = CLOSING;					//switch state					
				}				
				if ((!(INPUT_PIN & (1 << MOTOR_STOP_BTN_PIN))) & (cntEmergencyButton > chkLimit)) {
					Send_Packet(RADDR, MOTOR_STOP_CMD);	//send Stop motor cmd
					state = STOPPING;					//switch state					
				}				
				break;
			case OPENING:
				OUTPUT_PORT |= (1 << RF_LED_PIN);			//turn off the LEDs and go to IDLE state
				state = IDLE;
				break;
			case CLOSING:
				OUTPUT_PORT |= (1 << RF_LED_PIN);			//turn off the LEDs and go to IDLE state			
				//_delay_ms(WAIT_TIME);						//wait a little bit
				//OUTPUT_PORT &= ~(1 << RF_LED_PIN);			//turn off the LEDs and go to IDLE state				
				state = IDLE;
				break;
			case STOPPING:
				OUTPUT_PORT |= (1 << RF_LED_PIN);			//turn off the LEDs and go to IDLE state			
				//_delay_ms(WAIT_TIME);						//wait a little bit
				//OUTPUT_PORT &= ~(1 << RF_LED_PIN);			//turn off the LEDs and go to IDLE state
				state = IDLE;
				break;
			default:
				break;			
		} //end switch
	}
	return 0;
}

ISR(TIMER0_COMPA_vect)
{
	static uint8_t cntLimit = 50;

	if (!(INPUT_PIN & (1 << CLOSE_BTN_PIN))) {
		cntCloseButton++;
		if (cntCloseButton > cntLimit) {
			cntCloseButton = 0;
		}
	} else {
		if (cntCloseButton > 0) {
			cntCloseButton--;
		}
	}
	if (!(INPUT_PIN & (1 << OPEN_BTN_PIN))) {
		cntOpenButton++;
		if (cntOpenButton > cntLimit) {
			cntOpenButton = 0;
		}
	} else {
		if (cntOpenButton > 0) {
			cntOpenButton--;
		}
	}
	if (!(INPUT_PIN & (1 << MOTOR_STOP_BTN_PIN))) {
		cntEmergencyButton++;
		if (cntEmergencyButton > cntLimit) {
			cntEmergencyButton = 0;
		}
	} else {
		if (cntEmergencyButton > 0) {
			cntEmergencyButton--;
		}
	}
}

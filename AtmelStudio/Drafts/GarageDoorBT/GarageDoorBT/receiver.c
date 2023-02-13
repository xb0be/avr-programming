/*
Code taken from:
https://www.youtube.com/watch?v=4TPwvxCTS4I
 https://drive.google.com/drive/folders/1G8QLIVCWlWAjIYiDAR0_j9Lhmxq-QTzu
Code for Receiver Model: XY-MK-5V Transmitting Frequency: 433.92MHz
*/
#ifndef F_CPU
#define F_CPU 8000000UL
#endif
#include "settings.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

extern volatile char state;						//Needed to update the state machine in main.c
extern void turnOffLEDs();

//Initializing UART
void USART_Init(void) {
	//Setting the baud rate is done by writing to the UBRR0H and UBRR0L registers
	UBRR0H = (UBRRVAL >> 8);				//high byte
	UBRR0L = (uint8_t)UBRRVAL;				//low byte
	
	//Set data frame format: asynchronous mode, no parity, 1 stop bit, 8 bit size
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	//Enable Receiver, Transmitter, and  Receiver Interrupt on receive complete
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
}

//Read the value out of the UART buffer
uint8_t USART_vReceiveByte(void) {
		while((UCSR0A & (1 << RXC0)) == 0);		//Wait until a byte has been received
		return UDR0;							//Return received data 
}

/* Send data to remote */
void sendbyte(unsigned char MSG) {
	while((UCSR0A&(1<<UDRE0)) == 0);     // Wait if a byte is being transmitted
	UDR0 = MSG;
}

void sendstr(unsigned char *s) {
	unsigned char i = 0;
	while(s[i] != '\0'){
		sendbyte(s[i]);
		i++;
	}
}

//USART Receiver interrupt service routine
ISR(USART_RX_vect)
{
	uint8_t data;
	data = USART_vReceiveByte();
	OUTPUT_PORT |= (1 << RF_LED_PIN);
	_delay_ms(250);
	OUTPUT_PORT &= ~(1 << RF_LED_PIN);
	_delay_ms(250);
	if (data == 'a') {
		sendstr(alarmmsg);
		state = LOCKED;
	}
	else if (data =='o') {
		state = PRE_OPENING;
	}
	else if (data == 'c') {
		state = PRE_CLOSING;
	}
	else {
	}
} //end ISR

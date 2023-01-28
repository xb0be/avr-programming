/*
Code taken from:
https://www.youtube.com/watch?v=4TPwvxCTS4I
 https://drive.google.com/drive/folders/1G8QLIVCWlWAjIYiDAR0_j9Lhmxq-QTzu
Code for Receiver Model: XY-MK-5V Transmitting Frequency: 433.92MHz
*/
#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
//#include <util/delay.h>
#include "settings.h"

extern volatile char state;						//Needed to update the state machine in main.c
volatile uint8_t RF = 0;						//Flag, if we are coming to state machine from RF
volatile uint8_t doOpen = 0;

extern void restartTimer();

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

//Read the value out of the UART buffer
uint8_t USART_vReceiveByte(void) {
	while((UCSR0A & (1 << RXC0)) == 0);		//Wait until a byte has been received
	return UDR0;							//Return received data 
}

//USART Receiver interrupt service routine
ISR(USART_RX_vect)
{
	uint8_t raddress, data, chk;			//define variables
	raddress = USART_vReceiveByte();		//receive destination address
	data = USART_vReceiveByte();			//receive data
	chk = USART_vReceiveByte();				//receive checksum
	
	if(chk == (raddress+data)) {			//compare received checksum with calculated
		if(raddress == RADDR) {				//compare transmitter address
			RF = 1;
			switch (data) {
				case MOTOR_STOP_CMD:
					state = ALARM;
					break;
				case MOTOR_OPEN_CMD:
					restartTimer();
					state = OPENING;
					//just set a variable and check it in main???
					//doOpen = 1;
					break;
				case MOTOR_CLOSE_CMD:
					restartTimer();
					state = CLOSING;
					break;
				default:
					break; 
			} //end switch
		} //end if
	} //end if
	doOpen = 0;
	//USART_vReceiveByte();
	//USART_vReceiveByte();
	//USART_vReceiveByte();
} //end ISR

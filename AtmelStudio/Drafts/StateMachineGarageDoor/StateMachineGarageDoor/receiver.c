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
//extern void restartTimer();
extern void turnOffLEDs();

//Initializing UART
void USART_Init(void) {
	//Setting the baud rate is done by writing to the UBRR0H and UBRR0L registers
	UBRR0H = (UBRRVAL >> 8);				//high byte
	UBRR0L = (uint8_t)UBRRVAL;				//low byte
	
	//Set data frame format: asynchronous mode, no parity, 1 stop bit, 8 bit size
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
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
	uint8_t rsync, raddress, data, chk;			//define variables
	rsync = USART_vReceiveByte();				//receive SYNC
	raddress = USART_vReceiveByte();			//receive destination address
	data = USART_vReceiveByte();				//receive data
	chk = USART_vReceiveByte();					//receive checksum
	
	OUTPUT_PORT ^= (1 << RF_LED_PIN);
	
	if (rsync == SYNC) {
		if(chk == (raddress+data)) {			//compare received checksum with calculated
			if(raddress == RADDR) {				//compare transmitter address
				//OUTPUT_PORT ^= (1 << LOCKED_LED_PIN);
				//_delay_ms(500);
				//OUTPUT_PORT ^= (1 << LOCKED_LED_PIN);
				//_delay_ms(500);
					switch (data) {
						case EMERGENCY_STOP_CMD:
							turnOffLEDs();
							state = LOCKED;
							break;
						case MOTOR_OPEN_CMD:
							state = PRE_OPENING;
							break;
						case MOTOR_CLOSE_CMD:
							state = PRE_CLOSING;
							break;
						default:
							break; 
					} //end switch
			} //end if raddress
		} //end if chk
	} //end if rsync
} //end ISR

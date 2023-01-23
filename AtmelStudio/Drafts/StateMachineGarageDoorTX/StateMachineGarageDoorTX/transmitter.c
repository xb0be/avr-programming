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
#include "settings.h"

//char state;

// Initializing UART
void USART_Init(void)
{
	//Setting the baud rate is done by writing to the UBRR0H and UBRR0L registers
	UBRR0H = (UBRRVAL >> 8);				//high byte
	UBRR0L = (uint8_t)UBRRVAL;				//low byte
	//Set data frame format: asynchronous mode,no parity, 1 stop bit, 8 bit size
	UCSR0C = (0 << UMSEL01) | (0 << UMSEL00) | (0 << UPM01) | (0 << UPM00) | (0 << USBS0) | (0 << UCSZ02) | (1 << UCSZ01) | (1 << UCSZ00);
	UCSR0B = (1 << TXEN0);					//Enable Transmitter 
}

// Transmit data(byte) function
void USART_vSendByte(uint8_t u8Data)
{
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

// The function returns a boolean value indicating whether or not the button was pressed
unsigned char buttonPressed(unsigned char BUTTON){
	if (!(INPUT_PIN & (1 << BUTTON)))			//The button is pressed when BUTTON bit is clear 
	{
		_delay_ms(BOUNCETIME);				//Time to wait while "de-bouncing" button
		if (!(INPUT_PIN & (1 << BUTTON))){		//Check the state of the button again
			return 1;
		}
	}
	return 0;
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

//	char state = IDLE;						//Initial state is IDLE
	USART_Init();

	while(1)
	{
		//switch (state){
			//case IDLE:
				if (buttonPressed(OPEN_BTN_PIN)){		//if Open button was pressed
					OUTPUT_PORT |= (1 << RF_LED_PIN);			//turn on the RF LED
					OUTPUT_PORT |= (1 << OPEN_LED_PIN);		//turn on the Open LED
					Send_Packet(RADDR, OPEN_CMD);		//send Open cmd
					//state = OPENING;					//switch state
				}
				if (buttonPressed(CLOSE_BTN_PIN)){		//if Close button was pressed
					OUTPUT_PORT |= (1 << RF_LED_PIN);			//turn on the RF LED
					OUTPUT_PORT |= (1 << CLOSE_LED_PIN);		//turn on the Close LED					
					Send_Packet(RADDR, CLOSE_CMD);		//send Close cmd
					//state = CLOSING;					//switch state					
				}				
				if (buttonPressed(MOTOR_STOP_BTN_PIN)){		//if Stop motor button was pressed
					OUTPUT_PORT |= (1 << RF_LED_PIN);			//turn on the RF LED
					OUTPUT_PORT |= (1 << MOTOR_STOP_LED_PIN);	//turn on the Stop motor LED					
					Send_Packet(RADDR, MOTOR_STOP_CMD);	//send Stop motor cmd
					//state = STOPPING;					//switch state					
				}				
				break;
			case OPENING:
				_delay_ms(WAIT_TIME);					//wait a little bit
				OUTPUT_PORT &= ~(1 << RF_LED_PIN);			//turn off the LEDs and go to IDLE state
				OUTPUT_PORT &= ~(1 << OPEN_LED_PIN);
				state = IDLE;
				break;
			case CLOSING:
				_delay_ms(WAIT_TIME);					//wait a little bit
				OUTPUT_PORT &= ~(1 << RF_LED_PIN);			//turn off the LEDs and go to IDLE state
				OUTPUT_PORT &= ~(1 << CLOSE_LED_PIN);
				state = IDLE;
				break;
			case STOPPING:
				_delay_ms(WAIT_TIME);					//wait a little bit
				OUTPUT_PORT &= ~(1 << RF_LED_PIN);			//turn off the LEDs and go to IDLE state
				OUTPUT_PORT &= ~(1 << MOTOR_STOP_LED_PIN);
				state = IDLE;
				break;
			default:
				break;			
		} //end switch
	}
	return 0;
}

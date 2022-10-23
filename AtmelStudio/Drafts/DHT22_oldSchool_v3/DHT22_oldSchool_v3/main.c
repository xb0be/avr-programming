/*****************************************************
Taken from https://forum.allaboutcircuits.com/threads/dht22-shows-check-sum-error.156857/

*****************************************************/

#include <avr/io.h>
#include <util/delay.h>

#include "ssd.h"

#define F_CPU			1000000UL
#define DHT_PORT        PORTD
#define DHT_DDR         DDRD
#define DHT_PIN         PIND
#define DHT11_PIN		3

uint8_t c=0,I_RH,D_RH,I_Temp,D_Temp,CheckSum;
int q;
float RH;
unsigned char data [5];

void Request()                /* Microcontroller send start pulse/request */
{
	DHT_DDR |= (1<<DHT11_PIN);
	DHT_PORT &= ~(1<<DHT11_PIN);    /* set to low pin */
	_delay_ms(20);            /* wait for 20ms */
	DHT_PORT |= (1<<DHT11_PIN);    /* set to high pin */
	//ssdDisplay(9);
}

void Response()                /* receive response from DHT11 */
{
	DHT_DDR &= ~(1<<DHT11_PIN);
	//ssdDisplay(1);
	DHT_PORT |= (1<<DHT11_PIN);    /* set to high pin */
	while(DHT_PIN & (1<<DHT11_PIN));
	ssdDisplay(2);					//HERE IT STOPS
	while((DHT_PIN & (1<<DHT11_PIN))==0);
	ssdDisplay(3);
	while(DHT_PIN & (1<<DHT11_PIN));
	ssdDisplay(4);
}

uint8_t Receive_data()            /* receive data */
{
	for (q=0; q<8; q++)
	{
		while((DHT_PIN & (1<<DHT11_PIN)) == 0);  /* check received bit 0 or 1 */
		_delay_us(30);
		if(DHT_PIN & (1<<DHT11_PIN))/* if high pulse is greater than 30ms */
		c = (c<<1)|(0x01);    /* then its logic HIGH */
		else            /* otherwise its logic LOW */
		c = (c<<1);
		while(DHT_PIN & (1<<DHT11_PIN));
	}
	return c;
}


void main(void)
{
//	DDRA = 0x00;						//Obsolete (Port A0 as input)
	DIGIT_CONTROL_DDR = 0xff;			//Digit select register as output
	DATA_DDR = 0xff;					//Whole register as output for 7-segment display

	while (1)
	{
		Request();        /* send start pulse */
		Response();        /* receive response */
		I_RH=Receive_data();    /* store first eight bit in I_RH */
		D_RH=Receive_data();    /* store next eight bit in D_RH */
		I_Temp=Receive_data();    /* store next eight bit in I_Temp */
		D_Temp=Receive_data();    /* store next eight bit in D_Temp */
		CheckSum=Receive_data();/* store next eight bit in CheckSum */

		if ((I_RH + D_RH + I_Temp + D_Temp) != CheckSum)
		{
			//Checksum not OK, display error
		}
		else
		{
			//All good, display data
		}
		_delay_ms(2000);
	}
}

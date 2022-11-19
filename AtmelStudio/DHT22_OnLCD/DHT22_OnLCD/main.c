/*****************************************************
Used code:
https://forum.allaboutcircuits.com/threads/dht22-shows-check-sum-error.156857/
https://www.programming-electronics-diy.xyz/2016/08/lcd-interfacing-library-4-and-8-bit-mode.html
All under GNU GPL v3, so is this one.

Simple schematic:
ATmega328p <--> LCD:
PD4 ---------> D4
PD5 ---------> D5
PD6 ---------> D6
PD7 ---------> D7

PC0 ---------> RS
PC1 ---------> RW
PC2 ---------> E

LCD:
VSS ---------> GND
VDD ---------> +5V
V0 ---------> GND
A ---------> +5V
K ---------> 4k7 resistor  ---------> GND

Burn command:
sudo avrdude -v -v -c usbtiny -p ATmega328P -U flash:w:DHT22_OnLCD.hex:i -U lfuse:w:0xe2:m -U hfuse:w:0xd9:m -U efuse:w:0xff:m

*****************************************************/

#define F_CPU			8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "OnLCDLib.h"

#define DHT_PORT        PORTC
#define DHT_DDR         DDRC
#define DHT_PIN         PINC
#define DHT22_PIN		5

uint8_t c=0,I_RH,D_RH,I_Temp,D_Temp,CheckSum;
int q;
float RH, T;
unsigned char data [5];

void Request()                /* Microcontroller send start pulse/request */
{
	DHT_DDR |= (1<<DHT22_PIN);
	DHT_PORT &= ~(1<<DHT22_PIN);    /* set to low pin */
	_delay_ms(20);            /* wait for 20ms */
	DHT_PORT |= (1<<DHT22_PIN);    /* set to high pin */
	//ssdDisplay(9);
}

void Response()                /* receive response from DHT11 */
{
	DHT_DDR &= ~(1<<DHT22_PIN);
	DHT_PORT |= (1<<DHT22_PIN);    /* set to high pin */
	while(DHT_PIN & (1<<DHT22_PIN));
	while((DHT_PIN & (1<<DHT22_PIN))==0);
	while(DHT_PIN & (1<<DHT22_PIN));
}

uint8_t Receive_data()            /* receive data */
{
	for (q=0; q<8; q++)
	{
		while((DHT_PIN & (1<<DHT22_PIN)) == 0);  /* check received bit 0 or 1 */
		_delay_us(39);
		if(DHT_PIN & (1<<DHT22_PIN))/* if high pulse is greater than 30ms */
		c = (c<<1)|(0x01);    /* then its logic HIGH */
		else            /* otherwise its logic LOW */
		c = (c<<1);
		while(DHT_PIN & (1<<DHT22_PIN));
	}
	return c;
}

int main(void)
{
	// Initialize the LCD
	LCDSetup(LCD_CURSOR_NONE);
	_delay_ms(2000);
	while (1)
	{
		Request();        /* send start pulse */
		Response();        /* receive response */
		I_RH=Receive_data();    /* store first eight bit in I_RH */
		D_RH=Receive_data();    /* store next eight bit in D_RH */
		I_Temp=Receive_data();    /* store next eight bit in I_Temp */
		D_Temp=Receive_data();    /* store next eight bit in D_Temp */
		CheckSum=Receive_data();/* store next eight bit in CheckSum */

		if (((I_RH + D_RH + I_Temp + D_Temp) & 255) != CheckSum)
		{
			//Checksum error
			LCDHome();
			LCDWriteString("Checksum error");
		}
		else // All good, display values
		{
			LCDHome();
            T= (float) I_Temp * 256.0 + D_Temp ;
            T=T/10.0;
            LCDWriteFloat(T,0,2);			
			LCDGotoXY(1,2);
            RH= (float) I_RH * 256.0 + D_RH ;
            RH=RH/10.0;			
			LCDWriteFloat(RH,0,2);
		}
		_delay_ms(2000);
	}
}

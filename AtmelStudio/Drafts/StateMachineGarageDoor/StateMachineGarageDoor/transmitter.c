// Code for Transmitter Model: FS1000A, Transmitting Frequency: 433.92MHz

#ifndef F_CPU
//define CPU clock speed if not defined
#define F_CPU 16000000UL // AVR ATMega328P 16MHz
#endif

#include <avr/io.h> // enable data flow control over pins
#include <util/delay.h> // enable delay function in program

//set desired baud rate
#define BAUDRATE 9600
//calculate UBRR value
#define UBRRVAL ((F_CPU/(BAUDRATE*16UL))-1)

//define communicate parameters
#define SYNC 0xAA // synchronization signal
#define RADDR 0x44 // receiver address
#define LED_TOGGLE 0x11// LED toggle command

#define BUTTON 0 // button switch connected to port C pin 0
#define DEBOUNCE_TIME 25 // time to wait while "de-bouncing" button
#define LOCK_INPUT_TIME 300 // time to wait after a button press

// Initializing UART
void USART_Init(void)
{
	// Setting the baud rate is done by writing to the UBRR0H and UBRR0L registers
	UBRR0H=(UBRRVAL>>8);	//high byte
	UBRR0L=(uint8_t)UBRRVAL;//low byte
	
	//Set data frame format: asynchronous mode,no parity, 1 stop bit, 8 bit size
	UCSR0C=(0<<UMSEL01)|(0<<UMSEL00)|(0<<UPM01)|(0<<UPM00)|(0<<USBS0)|(0<<UCSZ02)|(1<<UCSZ01)|(1<<UCSZ00);
	//Enable Transmitter 
	UCSR0B=(1<<TXEN0);
}

// Transmit data(byte) function
void USART_vSendByte(uint8_t u8Data)
{
	// Wait for transmit buffer to be empty
	while((UCSR0A&(1<<UDRE0)) == 0);
	// Transmit data - load data into transmit register
	UDR0 = u8Data;
}

// Send packet of data 
void Send_Packet(uint8_t addr, uint8_t cmd)
{
	USART_vSendByte(SYNC);//send synchronization byte
	USART_vSendByte(addr);//send receiver address
	USART_vSendByte(cmd);//send increment command
	USART_vSendByte((addr+cmd));//send checksum
}

// The function returns a boolean value indicating whether or not the button was pressed
unsigned char button_state()
{
	if (!(PINC & (1<<BUTTON)))// the button is pressed when BUTTON bit is clear 
	{
		_delay_ms(DEBOUNCE_TIME);// time to wait while "de-bouncing" button
		if (!(PINC & (1<<BUTTON))) return 1;// check the state of the button again
	}
	return 0;
}

// Main initialization
void Main_Init(void)
{
	DDRC=0xFFu; //	Set all pins of the PORTC as output.
	DDRC &= ~(1<<BUTTON);//Makes  pin 0 of the PORTC as Input
	PORTC = 0xFF;  // Set all pins of the PORTC as HIGH. Internal Pull Up resistor is enable.
}

// main function - entry point of the program
int main(void)
{
	USART_Init();
	Main_Init();

	// main endless loop			
	while(1)
	{		
		if (button_state()) // If the button is pressed, 
		{
		 	Send_Packet(RADDR, LED_TOGGLE); //send command to increment delay of led blink
		 	_delay_ms(LOCK_INPUT_TIME); // time to lock button
		 }
	}
	
	return 0;
}

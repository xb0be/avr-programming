/*

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

*/

/****************************************
 INCLUDES
*****************************************/
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "DHT11.h"
#include "OnLCDLib.h"



/****************************************
 MAIN FUNCTION
*****************************************/

int main(void){
    // Initialise the LCD
    LCDSetup(LCD_CURSOR_NONE);

    int8_t DHTreturnCode;
 
    while(1){
        DHTreturnCode = DHT11ReadData();

        if(DHTreturnCode == 1){
            LCDHome();
            DHT11DisplayTemperature();
            LCDGotoXY(1,2);
            DHT11DisplayHumidity();
        }else{
            if(DHTreturnCode == -1){
                LCDHome();
                LCDWriteString("Checksum Error");
            }else{
                LCDHome();
                LCDWriteString("Unknown Error");
            }
        }
    }
}
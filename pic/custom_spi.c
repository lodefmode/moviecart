#include <xc.h>

void 
SPI1_Open(void)
{
    // Set the SPI1 module to the options selected in the User Interface
		// SSP active high; SDOP active high; FST disabled; SMP Middle; CKP Idle:Low, Active:High; CKE Active to idle; SDIP active high; 
		SPI1CON1 = 0x40;
		// SSET disabled; RXR suspended if the RxFIFO is full; TXR required for a transfer; 
		SPI1CON2 = 0x03;
		// BAUD 0; 
        
#if 0        
255 125 kHz
127 250 kHz
 79 400 kHz
 31   1 MHz
 15   2 MHz
  9 3.2 MHz
  7   4 MHz    // works
  6            // works
  5            // works
  4 6.4 MHz    // fail
  3   8 MHz    // fail
  2 10.66 MHz
  1  16 MHz    // fail
  0  32 MHz
#endif        

		SPI1BAUD = 7; // 7; // 4 MHz

		// CLKSEL HFINTOSC; 
		SPI1CLK = 0x01;
		// BMODE last byte; LSBF MSb first; EN enabled; MST bus master; 
		SPI1CON0 = 0x82;
    
    //Do this once!
    //One byte transfer count
//    SPI1TCNTL = 1;

}

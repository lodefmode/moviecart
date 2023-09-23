
/*

Runs an interrupt on address change that feed the buffer to the screen, and updates frameNumber
Main thread needs only maintain the buffer.

*/

#include <stdint.h>
#include <stdbool.h>

#include "defines.h"

// I/O mappings
#define SET_DATA(X)     { LATB = (X); }
#define READ_DATA()     PORTB
#define DATA_OUTPUT     TRISB = 0x0000;
#define DATA_INPUT      TRISB = 0xffff;
#define DATA_OUTPUT_DIR IO_RA3_SetLow();
#define DATA_INPUT_DIR  IO_RA3_SetHigh();
#define LO_ADDRESS      (PORTC & 0x1FF)
#define TESTA0_LOW		 IO_RA0_SetLow();
#define TESTA0_HIGH		 IO_RA0_SetHigh();
#define TESTA1_LOW		 IO_RA1_SetLow();
#define TESTA1_HIGH		 IO_RA1_SetHigh();
#define TESTA2_LOW		 IO_RA2_SetLow();
#define TESTA2_HIGH		 IO_RA2_SetHigh();

#define EMULATE_DONE \
    { \
    CNFC = 0; /* clear all flags */     \
    IFS1bits.CNCIF = 0; /* clear flag */    \
    return; \
    }
#define RAM_UNITIALIZED __attribute__((persistent))


extern void			coreInit();
extern struct frameInfo	r_frameInfo;
extern struct frameInfo	mr_frameInfo1, mr_frameInfo2;


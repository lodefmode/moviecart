
#include <stdbool.h>
#include <stdint.h>
#include "mcc_generated_files/mcc.h"
#include "../output/core_locations.h"

#define nullptr NULL

////////////////////////////////////////////////


// round field to nearest 512 byte boundary
#define BLOCK_SIZE 512
//#define FIELD_SIZE_RAW 2377 /*(3 + 262 + 192 * (5 + 6)) */
//#define FIELD_PAD ((FIELD_SIZE_RAW % BLOCK_SIZE) ?  (BLOCK_SIZE - (FIELD_SIZE_RAW % BLOCK_SIZE)):0)
#define FIELD_SIZE 2560 /* (FIELD_SIZE_RAW + FIELD_PAD) */
#define NUM_BLOCKS 5 /* (FIELD_SIZE / BLOCK_SIZE) */
#define FULL_NUM_BLOCKS 8 /* (FIELD_SIZE / BLOCK_SIZE) */

#define FRAME_SIZE (2 * FIELD_SIZE)
#define BLANK_LINE_SIZE	(28+3+37) // 68, +2 prev


////////////////////////////////////////////////


#define ADDR_HI		LATD  // 3 bits

#define SPI1_SS     LATB0

#define TEST_LED_ON		{ LATB5 = 0; }
#define TEST_LED_OFF	{ LATB5 = 1; }

#define A7_RAW      PORTAbits.RA7
//#define A12_RAW     PORTDbits.RD6
#define A11_RAW     PORTDbits.RD5

#define A10_RAW         PORTBbits.RB4
#define A10_COUNT		TMR0L


//#define TRANSPORT_PORT PORTB

#define ADDR_LO         LATA  // 7 bits

#define DATA_BUS_OUT    LATC  // 8 bits
#define DATA_BUS_IN     PORTC  // 8 bits
#define DATA_TRIS       TRISC


#define SRAM_OE     LATE0
#define SRAM2_CE    LATE1        
#define SRAM_WE     LATE2

#define LO7_BYTE(x) ((uint8_t)(((x)) & 127))
#define HI3_BYTE(x) ((uint8_t)(((x)>>7) & 7))

        
#define LO8_BYTE(X) (uint8_t)((X)&0xff)
#define HI8_BYTE(X) (uint8_t)(((X)>>8)&0xff)
        
#define VECTOR_SIZE 6

#define	BEGIN_WRITE() \
    { \
	SRAM_OE = 1; 		/* disable output */ \
	DATA_TRIS = 0;		/* output */ \
    }

#define END_WRITE() \
    { \
	DATA_TRIS = 0xff;  /* input */ \
	SRAM_OE = 0;      /* enable our output */ \
    }

#define	SET_WRITE_PAGE(X)	\
    { \
    ADDR_HI = (ADDR_HI & ~7) | HI3_BYTE(X); \
    }

#define	WRITE_DATA(X, D)	\
    { \
    ADDR_LO = LO7_BYTE(X); \
    DATA_BUS_OUT = D; \
    SRAM_WE = 0; \
    SRAM_WE = 1; \
    }

#define STATE_LOOP    {goto again;}

extern void WHILE_A7(void);
extern void WHILE_NOT_A7(void);

extern void test_flash(uint8_t num);


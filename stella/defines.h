
#include <stdint.h>
#include "../output/core_locations.h"

#define BEGIN_WRITE()  {} 
#define END_WRITE()  {} 

#define	SET_WRITE_PAGE(X)	\
	writePage = ((X >> 7) & 0x07);

#define	WRITE_DATA(X, D)	\
	writeSramLo((uint8_t)(X & 127), D);

#define WHILE_NOT_A7()	{ if (!a7) break; }
#define WHILE_A7()	{ if (a7) break; }

//round field to nearest 512 byte boundary
#define FIELD_SIZE 2560
#define BLANK_LINE_SIZE (28+3+37) // 68

//#define STATE_LOOP	{goto again;}
#define STATE_LOOP	{}

extern uint8_t		writePage;
extern bool			a7;
extern void			writeSramLo(uint8_t lo, uint8_t data);
extern uint8_t		A10_COUNT;

extern uint8_t stream_buffer1[];
extern uint8_t stream_buffer2[];



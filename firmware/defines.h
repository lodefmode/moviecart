
/*

   Format description

*/

#ifndef __DEFINES__
#define __DEFINES__

#include <stdint.h>
#include <stdbool.h>

#define FIELD_NUM_BLOCKS	8
#define FIELD_MAX_BLOCKS	6	// pal is 6, but ntsc is only 5, anything larger will require device with more RAM

// 3K
#define FIELD_SIZE			(512*FIELD_MAX_BLOCKS) 

#define TESTA0_LOW		IO_RA0_SetLow();
#define TESTA0_HIGH		IO_RA0_SetHigh();
#define TESTA1_LOW		IO_RA1_SetLow();
#define TESTA1_HIGH		IO_RA1_SetHigh();
#define TESTA2_LOW		IO_RA2_SetLow();
#define TESTA2_HIGH		IO_RA2_SetHigh();
#endif

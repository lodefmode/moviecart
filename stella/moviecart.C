
/////////////////////////////////////////////////////////////////////////////////////////////
//
//  Stella stuff.
//

#include "defines.h"
#include <iostream>
#include <bitset>
#include "../output/core_locations.h"
#include "../output/core_data.h"

// static ram
uint8_t		sram[1024];
extern int32_t frameNumber; // signed

// address lines
bool 		a12 = false;
bool 		a11 = false;
bool 		a10 = false;
bool 		a7 = false;
uint8_t		A10_COUNT = 0;

uint8_t		writePage = 0;		// 0..7

void
writeSramLo(uint8_t lo, uint8_t data)	// lo is lower 7 bits of sram address
{
	uint16_t addr = (writePage << 7) | lo;
	sram[addr & 1023] = data;
}

uint8_t stream_buffer1[FIELD_SIZE];
uint8_t stream_buffer2[FIELD_SIZE];


void runStateMachine();
bool initState();

#define TITLE_CYCLES 1000000

void
process_addr(uint16_t addr)
{
	// Make sure its in title screen
	static bool outTitleScreen = false;
	static int outTitleTotal = 0;
	static int totalCycles = 0;


	if (addr >= 0xfffa)	// reset vectors, cpu was reset
	{
		outTitleScreen = false;
		outTitleTotal = 0;
		totalCycles = 0;

		memcpy(sram, core_data, 1024);
		initState();
	}


	a12 = (addr & (1 << 12)) ? 1:0;		// don't need to latch
	a11 = (addr & (1 << 11)) ? 1:0;		// don't need to latch

	bool a10i = (addr & (1 << 10));		// don't need to latch
	if (a10i && !a10)
		A10_COUNT++;
	a10 = a10i;


	// don't latch a7 on a12/a11, instead look for steady count

	static uint8_t a7_on = 0;
	static uint8_t a7_off = 0;

	bool a7_raw = (addr & (1 << 7));		// each 128

	// latched
	if (a11)	// a12
		a7 = a7_raw;  // why does !a7_raw work??

	totalCycles++;
	if (totalCycles == TITLE_CYCLES)
	{
		// stop title screen
		BEGIN_WRITE()
			SET_WRITE_PAGE(addr_title_loop)
			WRITE_DATA(addr_title_loop + 0, 0x18);	// clear carry, one bit difference from 0x38 sec
		END_WRITE()
	}

	if (totalCycles > TITLE_CYCLES)
	{
		if ((addr_title_loop & 1023) == (addr & 1023))
			outTitleScreen = true;

		if (outTitleScreen && outTitleTotal < 8)
			outTitleTotal++;

		if (outTitleTotal >= 8)
			runStateMachine();
	}
}

bool
movie_peek(uint16_t addr, uint8_t &result)
{
	process_addr(addr);

	result = sram[addr & 1023];

	return a12;
}

bool
movie_poke(uint16_t addr, uint8_t value)
{
	process_addr(addr);

	return a12;
}


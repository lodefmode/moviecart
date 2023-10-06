
/*

Runs an interrupt on address change that feed the buffer to the screen, and updates frameNumber
Main thread needs only maintain the buffer.

*/

#ifndef __CORE__
#define __CORE__

#include <stdint.h>
#include <stdbool.h>

#include "defines.h"
#include "frame.h"

struct coreInfo
{
	// mr_* used by both main + interrupt

	volatile bool			mr_endFrame;
	volatile bool			mr_bufferIndex;
	volatile uint_fast8_t	mr_swcha;
	volatile uint_fast8_t	mr_swchb;
	volatile uint_fast8_t	mr_inpt4;
	volatile uint_fast8_t	mr_inpt5;	// not used
	struct frameInfo		mr_frameInfo1 , mr_frameInfo2;

	// following only used by interrupt code

	uint_fast8_t	peekBus;
	volatile uint_fast8_t*	storeAddress;
	uint_fast8_t	breakLoops;
	uint_fast8_t	lines;

	uint_fast8_t	hiAddress;
	uint_fast8_t	vblankState;
	uint_fast8_t	vsyncState;
	uint_fast8_t	endState;
	uint_fast8_t	nextLineJump;
	uint_fast8_t	data;

	bool			audioPushed;
	uint_fast8_t	audioVal;

	struct frameInfo	frameInfo;
};

extern void			coreInit();

#endif

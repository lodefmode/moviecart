


#ifndef __UPDATE__
#define __UPDATE__

#include "defines.h"
#include "frame.h"

#define STATE_PLAYING	0x01
#define STATE_MUTE0		0x02
#define STATE_MUTE1		0x04
#define STATE_END		0x08

struct stateVars
{
	int32_t io_frameNumber;
	uint32_t i_numFrames;

	uint8_t i_swcha;
	uint8_t i_swchb;
	uint8_t i_inpt4;
	uint8_t i_inpt5;

	uint8_t io_bits;
};

extern void updateTransport(struct stateVars* state);
extern void updateBuffer(struct stateVars* state, struct frameInfo* fInfo);

#endif

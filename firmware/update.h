

#include "defines.h"

struct stateVars
{
	int32_t io_frameNumber;
	bool io_playing;

	uint32_t i_numFrames;
	bool	i_numFramesInit;

	uint8_t i_swcha;
	uint8_t i_swchb;
	uint8_t i_inpt4;
	uint8_t i_inpt5;
};

extern void updateTransport(struct stateVars* state);
extern void updateBuffer(struct stateVars* state, struct frameInfo* fInfo);


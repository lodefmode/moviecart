
#include <string.h> // memset
#include "defines.h"
#include "osd.h"
#include "update.h"

// transport controls

#define OSD_FRAMES		180
#define BACK_SECONDS	10

#define COLOR_BLACK		0x00
#define COLOR_BLUE		0x9A

#define LABEL_HEIGHT	14
#define LEVEL_HEIGHT	8
#define	TIMECODE_HEIGHT	12

// state machine
#define MODE_VOLUME     0
#define MODE_BRIGHT     1
#define MODE_TIME       2
#define MODE_LAST       2

#define MAX_LEVEL       11
#define DEFAULT_LEVEL   6


// scale adjustments, automatically generated
const uint8_t scale0[16] = { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 /*0.0000 */ };
const uint8_t scale1[16] = { 6, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 9, 9 /*0.1667 */ };
const uint8_t scale2[16] = { 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10 /*0.3333 */ };
const uint8_t scale3[16] = { 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11 /*0.5000 */ };
const uint8_t scale4[16] = { 3, 3, 4, 5, 5, 6, 7, 7, 8, 9, 9, 10, 11, 11, 12, 13 /*0.6667 */ };
const uint8_t scale5[16] = { 1, 2, 3, 4, 5, 5, 6, 7, 8, 9, 10, 10, 11, 12, 13, 14 /*0.8333 */ };
const uint8_t scale6[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 /*1.0000 */ };
const uint8_t scale7[16] = { 0, 0, 0, 1, 3, 4, 5, 7, 8, 10, 11, 12, 14, 15, 15, 15 /*1.3611 */ };
const uint8_t scale8[16] = { 0, 0, 0, 0, 1, 3, 5, 7, 8, 10, 12, 14, 15, 15, 15, 15 /*1.7778 */ };
const uint8_t scale9[16] = { 0, 0, 0, 0, 0, 2, 4, 6, 9, 11, 13, 15, 15, 15, 15, 15 /*2.2500 */ };
const uint8_t scale10[16] = { 0, 0, 0, 0, 0, 1, 3, 6, 9, 12, 14, 15, 15, 15, 15, 15 /*2.7778 */ };
//const uint8_t *scaleList[11] = { scale0, scale1, scale2, scale3, scale4, scale5, scale6, scale7, scale8, scale9, scale10 };
const uint8_t * const scaleList[11] = { scale0, scale1, scale2, scale3, scale4, scale5, scale6, scale7, scale8, scale9, scale10 };

struct updateInfo
{
	uint_fast8_t    mode;
	uint_fast8_t    drawLevelBars; // expressed in frames
	uint_fast8_t    drawTimeCode; // expressed in frames
	uint_fast8_t    volume;
	uint_fast8_t    bright;
	uint_fast8_t    joyRepeat;
	bool            right;
	bool            left;
	uint_fast8_t    lastMainMode;
	int8_t          speed;    // signed
	int8_t          step; // signed
	bool            lswcha_off;
	uint_fast8_t    linpt4;
	uint_fast8_t    lswchb;
};

__attribute__((section(".updateInfo"))) struct updateInfo           uInfo;

void
updateInit()
{
	uInfo.mode = MODE_VOLUME;
	uInfo.drawLevelBars = 0; // expressed in frames
	uInfo.drawTimeCode = 0; // expressed in frames
	uInfo.volume = DEFAULT_LEVEL;
	uInfo.bright = DEFAULT_LEVEL;
	uInfo.joyRepeat = 0x01;
	uInfo.right = false;
	uInfo.left = false;
	uInfo.lastMainMode = 0xff;
	uInfo.speed = 1;    // signed
	uInfo.step = 1; // signed
	uInfo.lswcha_off = true;
	uInfo.linpt4 = 0xff;
	uInfo.lswchb = 0xff;
}

void
updateTransport(struct stateVars *state)
{
	uInfo.lastMainMode = uInfo.mode;
	uInfo.right = !(state->i_swcha & 0x80);
	uInfo.left = !(state->i_swcha & 0x40);

	// reset
	if (!(state->i_swchb & 0x01))
	{
		state->io_frameNumber = (state->io_frameNumber) ? 0 : 1;
		state->io_bits |= STATE_PLAYING;
		state->io_bits |= STATE_MUTE0;
		uInfo.drawTimeCode = OSD_FRAMES;
		return;
	}

	// select
	if (!(state->i_swchb & 0x02) && (uInfo.lswchb & 0x02))
	{
		uInfo.drawTimeCode = OSD_FRAMES;
		state->io_frameNumber -= 60 *BACK_SECONDS;
	}

	uInfo.lswchb = state->i_swchb;

	// fire
	if (!(state->i_inpt4 & 0x80) && (uInfo.linpt4 & 0x80))
		state->io_bits ^= STATE_PLAYING;
	uInfo.linpt4 = state->i_inpt4;

	if (!(state->i_swcha & 0x10))	// up
	{
		if (uInfo.lswcha_off)
		{
			if (uInfo.mode == 0)
				uInfo.mode = MODE_LAST;
			else
				uInfo.mode--;
		}
	}

	if (!(state->i_swcha & 0x20))	// down
	{
		if (uInfo.lswcha_off)
		{
			if (uInfo.mode == MODE_LAST)
				uInfo.mode = 0;
			else
				uInfo.mode++;
		}
	}

	if (uInfo.left || uInfo.right)
	{
		uInfo.joyRepeat++;
		uInfo.joyRepeat &= 0x0f;
	}
	else
	{
		uInfo.joyRepeat = 0x01;
		uInfo.speed = 1;
	}

	if (!uInfo.joyRepeat)
	{
		if (uInfo.mode == MODE_TIME)
		{
			uInfo.drawTimeCode = OSD_FRAMES;
			uInfo.speed += 4;
			if (uInfo.speed < 0)
				uInfo.speed -= 4;
		}
	}

	if (!uInfo.joyRepeat)
	{
		if (uInfo.mode == MODE_VOLUME)
		{
			uInfo.drawLevelBars = OSD_FRAMES;
			if (uInfo.left)
			{
				if (uInfo.volume)
					uInfo.volume--;
			}
			else
			{
				uInfo.volume++;
				if (uInfo.volume >= MAX_LEVEL)
					uInfo.volume--;
			}
		}
	}

	if (!uInfo.joyRepeat)
	{
		if (uInfo.mode == MODE_BRIGHT)
		{
			uInfo.drawLevelBars = OSD_FRAMES;
			if (uInfo.left)
			{
				if (uInfo.bright)
					uInfo.bright--;
			}
			else
			{
				uInfo.bright++;
				if (uInfo.bright >= MAX_LEVEL)
					uInfo.bright--;
			}
		}
	}

	switch (uInfo.mode)
	{
		case MODE_TIME:
			if (uInfo.lastMainMode != uInfo.mode)
				uInfo.drawTimeCode = OSD_FRAMES;
			break;

		case MODE_BRIGHT:
		case MODE_VOLUME:
		default:
			if (uInfo.lastMainMode != uInfo.mode)
				uInfo.drawLevelBars = OSD_FRAMES;
			break;
	}

	// just draw one
	if (uInfo.drawLevelBars > uInfo.drawTimeCode)
		uInfo.drawTimeCode = 0;
	else
		uInfo.drawLevelBars = 0;

	// update frame
	uInfo.step = (state->io_frameNumber & 1) ? -1 : 1; // signed

	if (!(state->io_bits & STATE_PLAYING))	// step while paused
	{
		if (uInfo.mode == MODE_TIME)
		{
			if (uInfo.lswcha_off)
			{
				if (uInfo.right)
					uInfo.step = 2;
				else if (uInfo.left)
					uInfo.step = -2;
			}
		}
	}

	if (state->io_bits & STATE_PLAYING)
	{
		uInfo.step = 1;
		if (uInfo.mode == MODE_TIME)
		{
			if (uInfo.right)
				uInfo.step = uInfo.speed;
			else if (uInfo.left)
				uInfo.step = -uInfo.speed;
		}
	}

	uInfo.lswcha_off = !(~state->i_swcha & 0xff);
	state->io_frameNumber += uInfo.step;

	// check negative before unsigned comparison below
	if (state->io_frameNumber < 0)
	{
		state->io_frameNumber = (state->io_frameNumber & 1) ? 1 : 0;
		state->io_bits |= STATE_MUTE0;
		state->io_bits |= STATE_MUTE1;
	}

	if (state->i_numFrames && (state->io_frameNumber >= state->i_numFrames))
	{
		state->io_frameNumber = (state->io_frameNumber & 1) ? state->i_numFrames-1 : state->i_numFrames-2;
		state->io_bits |= STATE_MUTE0;
		state->io_bits |= STATE_MUTE1;
	};
}

void
updateVolume(struct stateVars* state, struct frameInfo* fInfo)
{
	const uint8_t*  volumeScale;

	if (state->io_bits & STATE_MUTE0)
	{
		volumeScale = scaleList[0];
		state->io_bits &= ~STATE_MUTE0;
	}
	else if (state->io_bits & STATE_MUTE1)
	{
		volumeScale = scaleList[0];
		state->io_bits &= ~STATE_MUTE1;
	}
	else
	{
		if (state->io_bits & STATE_PLAYING)
			volumeScale = scaleList[uInfo.volume];
		else
			volumeScale = scaleList[0];
	}


	uint_fast16_t	t = fInfo->totalLines;
	uint8_t*		dst = fInfo->audioBuf;

	while(t)
	{
		*dst = volumeScale[*dst & 0x0F];
		dst++;
		t--;
	};
}

// lower bit is ignored anyways
const uint8_t shiftBright[16 + MAX_LEVEL - 1] = { 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15, 15, 15, 15 };

void
updateColor(struct stateVars* state, struct frameInfo* fInfo)
{
	uint8_t		colorMask;
	uint_fast16_t	t;
	uint8_t*		dst;

	// color
	if (state->i_swchb & 0x08)
		colorMask = 0xff;
	else
		colorMask = 0x0f;

	uint8_t	lines = fInfo->visibleLines;

	const uint_fast16_t	offset22 = (fInfo->visibleLines - (LABEL_HEIGHT+LEVEL_HEIGHT))*5;
	const uint_fast16_t	offsetTC = (fInfo->visibleLines - TIMECODE_HEIGHT)*5;

	if (uInfo.drawLevelBars)
	{
		uInfo.drawLevelBars--;
		lines -= (LABEL_HEIGHT + LEVEL_HEIGHT);

		uint8_t levelValue;
		const uint8_t*	srcLabel;
		const uint8_t*	srcLevel;

		if (uInfo.mode == MODE_BRIGHT)
		{
			levelValue = uInfo.bright;
			if (fInfo->odd)
				srcLabel = brightLabelOdd;
			else
				srcLabel = brightLabelEven;
		}
		else
		{
			levelValue = uInfo.volume;
			if (fInfo->odd)
				srcLabel = volumeLabelOdd;
			else
				srcLabel = volumeLabelEven;
		}

		if (fInfo->odd)
			srcLevel = &levelBarsOddData[levelValue * 40];
		else
			srcLevel = &levelBarsEvenData[levelValue * 40];

		const uint_fast16_t	offset1 = (fInfo->visibleLines - (LABEL_HEIGHT+LEVEL_HEIGHT))*1;

		// background
		memset(fInfo->colorBuf + offset22, COLOR_BLUE, (LABEL_HEIGHT+LEVEL_HEIGHT)*5);
		memset(fInfo->colorBKBuf + offset1, COLOR_BLACK, (LABEL_HEIGHT+LEVEL_HEIGHT)*1);

		// label
		memcpy(fInfo->graphBuf + offset22, srcLabel, LABEL_HEIGHT*5);

		// level
		const uint_fast16_t offsetG = (fInfo->visibleLines - LEVEL_HEIGHT)*5;
		memcpy(fInfo->graphBuf + offsetG, srcLevel,  LEVEL_HEIGHT*5);
	}
	else if (uInfo.drawTimeCode)
	{
		uInfo.drawTimeCode--;
		lines -= TIMECODE_HEIGHT;

		const uint_fast16_t	offset1 = (fInfo->visibleLines - TIMECODE_HEIGHT)*1;

		memcpy(fInfo->graphBuf + offsetTC, fInfo->timecodeBuf, TIMECODE_HEIGHT*5);
		memset(fInfo->colorBuf + offsetTC, COLOR_BLUE, TIMECODE_HEIGHT*5);
		memset(fInfo->colorBKBuf + offset1, COLOR_BLACK, TIMECODE_HEIGHT*1);
	}

	// foreground
	dst = fInfo->colorBuf;
	t = lines * 5;

	while(t)
	{
		uint8_t r_data = *dst;
		*dst++ = ((r_data & 0xf0) | shiftBright[(r_data & 0x0f) + uInfo.bright]) & colorMask;
		t--;
	};

	// background
	t = lines * 1;
	dst = fInfo->colorBKBuf;
	while(t)
	{
		uint8_t r_data = *dst;
		*dst++ = ((r_data & 0xf0) | shiftBright[(r_data & 0x0f) + uInfo.bright]) & colorMask;
		t--;
	};

	// square off edges
	const uint_fast16_t	offsetF = (fInfo->visibleLines*5);
	if (fInfo->odd)
	{
		// bottom line
		fInfo->colorBuf[offsetF - 5] = COLOR_BLACK;
		fInfo->colorBuf[offsetF - 4] = COLOR_BLACK;
		fInfo->colorBuf[offsetF - 3] = COLOR_BLACK;
		fInfo->colorBuf[offsetF - 2] = COLOR_BLACK;
		fInfo->colorBuf[offsetF - 1] = COLOR_BLACK;

	}
	else
	{
		// top line
		fInfo->colorBuf[0] = COLOR_BLACK;
		fInfo->colorBuf[1] = COLOR_BLACK;
		fInfo->colorBuf[2] = COLOR_BLACK;
		fInfo->colorBuf[3] = COLOR_BLACK;
		fInfo->colorBuf[4] = COLOR_BLACK;
	}

	fInfo->colorBKBuf[0] = 0;

	// flat black line above time code
	if (uInfo.drawTimeCode)
	{
		uint8_t	*d = fInfo->odd ? (fInfo->graphBuf + offsetTC - 5) : (fInfo->graphBuf + offsetTC - 0);
		*d++ = 0;
		*d++ = 0;
		*d++ = 0;
		*d++ = 0;
		*d++ = 0;
	}

	// flat black line above label
	if (uInfo.drawLevelBars)
	{
		uint8_t	*d = fInfo->odd ? (fInfo->graphBuf + offset22 - 5) : (fInfo->graphBuf + offset22 - 0);
		*d++ = 0;
		*d++ = 0;
		*d++ = 0;
		*d++ = 0;
		*d++ = 0;
	}

}

void
updateBuffer(struct stateVars* state, struct frameInfo* fInfo)
{
	updateVolume(state, fInfo);
	updateColor(state, fInfo);
}

/**
 End of File
*/


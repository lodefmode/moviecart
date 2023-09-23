
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


uint_fast8_t    m_mode = MODE_VOLUME;
uint_fast8_t    m_drawLevelBars = 0; // expressed in frames
uint_fast8_t    m_drawTimeCode = 0; // expressed in frames
uint_fast8_t    m_volume = DEFAULT_LEVEL;
uint_fast8_t    m_bright = DEFAULT_LEVEL;
uint_fast8_t    m_joyRepeat = 0x01;
bool            m_right = false;
bool            m_left = false;
uint_fast8_t    m_lastMainMode = 0xff;
int8_t          m_speed = 1;    // signed
int8_t          m_step = 1; // signed
bool            m_lswcha_off = true;
uint_fast8_t    m_linpt4 = 0xff;
uint_fast8_t    m_lswchb = 0xff;

void
updateTransport(struct stateVars *state)
{
	m_lastMainMode = m_mode;
	m_right = !(state->i_swcha & 0x80);
	m_left = !(state->i_swcha & 0x40);

	// reset
	if (!(state->i_swchb & 0x01))
	{
		state->io_frameNumber = 1;
		state->io_playing = true;
		m_drawTimeCode = OSD_FRAMES;
	}

	// select
	if (!(state->i_swchb & 0x02) && (m_lswchb & 0x02))
	{
		m_drawTimeCode = OSD_FRAMES;
		state->io_frameNumber -= 60 *BACK_SECONDS + 1;
	}

	m_lswchb = state->i_swchb;

	// fire
	if (!(state->i_inpt4 & 0x80) && (m_linpt4 & 0x80))
		state->io_playing = !state->io_playing;
	m_linpt4 = state->i_inpt4;

	if (!(state->i_swcha & 0x10))	// up
	{
		if (m_lswcha_off)
		{
			if (m_mode == 0)
				m_mode = MODE_LAST;
			else
				m_mode--;
		}
	}

	if (!(state->i_swcha & 0x20))	// down
	{
		if (m_lswcha_off)
		{
			if (m_mode == MODE_LAST)
				m_mode = 0;
			else
				m_mode++;
		}
	}

	if (m_left || m_right)
	{
		m_joyRepeat++;
		m_joyRepeat &= 0x0f;
	}
	else
	{
		m_joyRepeat = 0x01;
		m_speed = 1;
	}

	if (!m_joyRepeat)
	{
		if (m_mode == MODE_TIME)
		{
			m_drawTimeCode = OSD_FRAMES;
			m_speed += 4;
			if (m_speed < 0)
				m_speed -= 4;
		}
	}

	if (!m_joyRepeat)
	{
		if (m_mode == MODE_VOLUME)
		{
			m_drawLevelBars = OSD_FRAMES;
			if (m_left)
			{
				if (m_volume)
					m_volume--;
			}
			else
			{
				m_volume++;
				if (m_volume >= MAX_LEVEL)
					m_volume--;
			}
		}
	}

	if (!m_joyRepeat)
	{
		if (m_mode == MODE_BRIGHT)
		{
			m_drawLevelBars = OSD_FRAMES;
			if (m_left)
			{
				if (m_bright)
					m_bright--;
			}
			else
			{
				m_bright++;
				if (m_bright >= MAX_LEVEL)
					m_bright--;
			}
		}
	}

	switch (m_mode)
	{
		case MODE_TIME:
			if (m_lastMainMode != m_mode)
				m_drawTimeCode = OSD_FRAMES;
			break;

		case MODE_BRIGHT:
		case MODE_VOLUME:
		default:
			if (m_lastMainMode != m_mode)
				m_drawLevelBars = OSD_FRAMES;
			break;
	}

	// just draw one
	if (m_drawLevelBars > m_drawTimeCode)
		m_drawTimeCode = 0;
	else
		m_drawLevelBars = 0;

	// update frame
	m_step = (state->io_frameNumber & 1) ? -1 : 1; // signed

	if (!state->io_playing)	// step while paused
	{
		if (m_mode == MODE_TIME)
		{
			if (m_lswcha_off)
			{
				if (m_right)
					m_step = 2;
				else if (m_left)
					m_step = -2;
			}
		}
	}

	if (state->io_playing)
	{
		m_step = 1;
		if (m_mode == MODE_TIME)
		{
			if (m_right)
				m_step = m_speed;
			else if (m_left)
				m_step = -m_speed;
		}
	}

	m_lswcha_off = !(~state->i_swcha & 0xff);
	state->io_frameNumber += m_step;

	if (state->i_numFramesInit && (state->io_frameNumber >= state->i_numFrames))
	{
		state->io_frameNumber -= 2;
		m_joyRepeat = 0;
		state->io_playing = false;
	};

	if (state->io_frameNumber < 1)
	{
		state->io_frameNumber = 1;
		m_speed = 1;
	}

}

void
updateVolume(struct stateVars* state, struct frameInfo* fInfo)
{
	const uint8_t*  volumeScale;

	if (state->io_playing)
		volumeScale = scaleList[m_volume];
	else
		volumeScale = scaleList[0];

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

	if (m_drawLevelBars)
	{
		m_drawLevelBars--;
		lines -= (LABEL_HEIGHT + LEVEL_HEIGHT);

		uint8_t levelValue;
		const uint8_t*	srcLabel;
		const uint8_t*	srcLevel;

		if (m_mode == MODE_BRIGHT)
		{
			levelValue = m_bright;
			if (fInfo->odd)
				srcLabel = brightLabelOdd;
			else
				srcLabel = brightLabelEven;
		}
		else
		{
			levelValue = m_volume;
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
	else if (m_drawTimeCode)
	{
		m_drawTimeCode--;
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
		*dst++ = ((r_data & 0xf0) | shiftBright[(r_data & 0x0f) + m_bright]) & colorMask;
		t--;
	};

	// background
	t = lines * 1;
	dst = fInfo->colorBKBuf;
	while(t)
	{
		uint8_t r_data = *dst;
		*dst++ = ((r_data & 0xf0) | shiftBright[(r_data & 0x0f) + m_bright]) & colorMask;
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
	if (m_drawTimeCode)
	{
		uint8_t	*d = fInfo->odd ? (fInfo->graphBuf + offsetTC - 5) : (fInfo->graphBuf + offsetTC - 0);
		*d++ = 0;
		*d++ = 0;
		*d++ = 0;
		*d++ = 0;
		*d++ = 0;
	}

	// flat black line above label
	if (m_drawLevelBars)
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


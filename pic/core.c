/////////////////////////////////////////////////////////////////////////////////////////////
// 
//  COMMON
//

#include "defines.h"

#define LO_JUMP_BYTE(X) ((X) & 0xff)
#define HI_JUMP_BYTE(X) ((((X) & 0xff00) >> 8) | 0x10)

extern bool sd_openStream(void);
extern void sd_swapField(bool index, bool odd);
extern void sd_blankPartialLines(bool index);
extern void sd_readField(uint32_t fr, bool index);
extern void sd_runReadState(void);

extern uint8_t *sd_ptr_version;
extern uint8_t *sd_ptr_frame;
extern uint8_t *sd_ptr_audio;
extern uint8_t *sd_ptr_graph;
extern uint8_t *sd_ptr_timecode;
extern uint8_t *sd_ptr_color;
extern uint8_t *sd_ptr_colorbk;

const uint8_t   *sd_ptr_graph_override = 0;


uint8_t 	lines = 0;
int32_t		frameNumber = 1;
bool		play = true;
uint8_t 	state = 0;
uint8_t		firstAudVal = 0;
bool		odd = true;
bool		bufferIndex = false;

#define TIMECODE_HEIGHT		12
#define OSD_FRAMES			180
#define BACK_SECONDS		10

#define MAX_LEVEL 			11
#define DEFAULT_LEVEL 		6


const uint8_t  scale0[16] = { 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8   /* 0.0000 */ };
const uint8_t  scale1[16] = { 6,  6,  7,  7,  7,  7,  7,  7,  8,  8,  8,  8,  8,  8,  9,  9   /* 0.1667 */ };
const uint8_t  scale2[16] = { 5,  5,  6,  6,  6,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10, 10   /* 0.3333 */ };
const uint8_t  scale3[16] = { 4,  4,  5,  5,  6,  6,  7,  7,  8,  8,  9,  9, 10, 10, 11, 11   /* 0.5000 */ };
const uint8_t  scale4[16] = { 3,  3,  4,  5,  5,  6,  7,  7,  8,  9,  9, 10, 11, 11, 12, 13   /* 0.6667 */ };
const uint8_t  scale5[16] = { 1,  2,  3,  4,  5,  5,  6,  7,  8,  9, 10, 10, 11, 12, 13, 14   /* 0.8333 */ };
const uint8_t  scale6[16] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15   /* 1.0000 */ };
const uint8_t  scale7[16] = { 0,  0,  0,  1,  3,  4,  5,  7,  8, 10, 11, 12, 14, 15, 15, 15   /* 1.3611 */ };
const uint8_t  scale8[16] = { 0,  0,  0,  0,  1,  3,  5,  7,  8, 10, 12, 14, 15, 15, 15, 15   /* 1.7778 */ };
const uint8_t  scale9[16] = { 0,  0,  0,  0,  0,  2,  4,  6,  9, 11, 13, 15, 15, 15, 15, 15   /* 2.2500 */ };
const uint8_t scale10[16] = { 0,  0,  0,  0,  0,  1,  3,  6,  9, 12, 14, 15, 15, 15, 15, 15   /* 2.7778 */ };
const uint8_t *scales[11] = { scale0, scale1, scale2, scale3, scale4, scale5, scale6, scale7, scale8, scale9, scale10 };

uint8_t mainVolume = DEFAULT_LEVEL;
const uint8_t	*volumeScale = 0;


uint8_t	mainBright = DEFAULT_LEVEL;
// lower bit is ignored anyways
const uint8_t  shiftBright[16 + MAX_LEVEL - 1] = {  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 15, 15, 15, 15};

#define MODE_VOLUME	0
#define MODE_BRIGHT	1
#define MODE_TIME	2
#define MAX_MODE	2

uint8_t	mainMode = MODE_TIME;

#include "../output/osd_data.h"


const uint8_t	*levelBarsOdd = 0;
const uint8_t	*levelBarsEven = 0;

///////////////////////////////////////////////////////////////////


#define TRANSPORT_RIGHT		0x10
#define TRANSPORT_LEFT		0x08
#define TRANSPORT_DOWN		0x04
#define TRANSPORT_UP		0x02
#define TRANSPORT_UNUSED1	0x01	// Right-2

#define TRANSPORT_BW		0x10
#define TRANSPORT_UNUSED2	0x08
#define TRANSPORT_SELECT	0x04
#define TRANSPORT_RESET		0x02
#define TRANSPORT_BUTTON	0x01

uint8_t		transport_direction = 0;
uint8_t		transport_buttons = 0;

uint8_t		transport_direction_last = 0;
uint8_t		transport_buttons_last = 0;

uint8_t		joy_repeat = 0;
int8_t		jumpStep = 1;	// signed

uint8_t		forceColor = 0;
bool		forceBWMode = false;

// expressed in frames
uint8_t		drawLevelBars = 0;
uint8_t		drawTimeCode = 0;

#define		COLOR_BLUE		0x9A
#define		COLOR_WHITE		0x0E

void
updateTransport()
{
	sd_ptr_graph_override = nullptr;

	uint8_t		transport_count = A10_COUNT;
	A10_COUNT = 0;
    
    // have to cut rate in half, to remove glitches...todo..
    {
        if (bufferIndex == true)
        {
			uint8_t	temp = ~(transport_count & 0x1e) & 0x1e;

            if (temp == transport_direction_last)
                transport_direction = temp;
            transport_direction_last = temp;
        }
        else
        {
			uint8_t temp = ~(transport_count & 0x17) & 0x17;

            if (temp == transport_buttons_last)
                transport_buttons = temp;
            transport_buttons_last = temp;
        }
    }

	bool right = transport_direction & TRANSPORT_RIGHT;
	bool left = transport_direction & TRANSPORT_LEFT;
	bool up = transport_direction & TRANSPORT_UP;
	bool down = transport_direction & TRANSPORT_DOWN;

	bool bw = transport_buttons & TRANSPORT_BW;
	bool button = transport_buttons & TRANSPORT_BUTTON;
	bool select = transport_buttons & TRANSPORT_SELECT;
	bool reset = transport_buttons & TRANSPORT_RESET;

	static bool last_button = false;
	static bool last_select = true;
	static bool last_right = false;
	static bool last_left = false;
	static bool last_down = false;
	static bool last_up = false;

	int8_t				step = 1;

	forceBWMode = bw;

	if (reset)
	{
		frameNumber = 1;
		play = true;
		drawTimeCode = OSD_FRAMES;

		goto update_stream;
	}

	uint8_t	lastMainMode = mainMode;
	if (up && !last_up)
	{
		if (mainMode == 0)
			mainMode = MAX_MODE;
		else
			mainMode--;
	}
	else if (down && !last_down)
	{
		if (mainMode == MAX_MODE)
			mainMode = 0;
		else
			mainMode++;
	}


	if (left || right)
	{
		joy_repeat++;
	}
	else
	{
		joy_repeat = 0;
		jumpStep = 1;
	}


	if (joy_repeat & 16)
	{
		joy_repeat = 0;

		if (left || right)
		{
			if (mainMode == MODE_TIME)
			{
				drawTimeCode = OSD_FRAMES;
				jumpStep += 4;
				if (jumpStep < 0)
					jumpStep -= 4;
			}
			else if (mainMode == MODE_VOLUME)
			{
				drawLevelBars = OSD_FRAMES;
				if (left)
				{
					if (mainVolume)
						mainVolume--;
				}
				else
				{
					mainVolume++;
					if (mainVolume >= MAX_LEVEL)
						mainVolume--;
				}
			}
			else if (mainMode == MODE_BRIGHT)
			{
				drawLevelBars = OSD_FRAMES;
				if (left)
				{
					if (mainBright)
						mainBright--;
				}
				else
				{
					mainBright++;
					if (mainBright >= MAX_LEVEL)
						mainBright--;
				}
			}
		}
	}


	if (select && !last_select)
	{
		drawTimeCode = OSD_FRAMES;
		frameNumber -= 60 * BACK_SECONDS + 1; // back 10 seconds
		goto update_stream;
	}

	if (button && !last_button)
		play = !play;

	switch(mainMode)
	{
		case MODE_TIME:
			if(lastMainMode != mainMode)
				drawTimeCode = OSD_FRAMES;
			break;

		case MODE_BRIGHT:
		case MODE_VOLUME:
		default:
			if(lastMainMode != mainMode)
				drawLevelBars = OSD_FRAMES;
			break;
	}
	
	// just draw one
	if (drawLevelBars > drawTimeCode)
		drawTimeCode = 0;
	else
		drawLevelBars = 0;

	if (play)
		volumeScale = scales[mainVolume];
	else
		volumeScale = scales[0];

	// update frame

	if (!play)  // step while paused
	{
		if (mainMode == MODE_TIME)
		{
			if (right && !last_right)
				step = 2;
			else if (left && !last_left)
				step = -2;
			else
				step = (frameNumber & 1) ? -1 : 1;
		}
		else
		{
			step = (frameNumber & 1) ? -1 : 1;
		}
	}
	else
	{
		if (mainMode == MODE_TIME)
		{
			if (right)
				step = jumpStep;
			else if (left)
				step = -jumpStep;
		}
		else
		{
			step = 1;
		}
	}

	frameNumber += step;
	if (frameNumber < 1)
	{
		frameNumber = 1;
		jumpStep = 1;
	}

update_stream:

	last_button = button;
	last_select = select;
	last_right = right;
	last_left = left;
	last_down = down;
	last_up = up;
}

//////////////////////////////////////////////////////////////////////////////

uint8_t	streamByte = 0;
uint8_t	colorByte = 0;

#define READ_FRAME()    \
{ \
	streamByte = *sd_ptr_frame++; \
}

#define READ_VERSION()    \
{ \
	streamByte = *sd_ptr_version++; \
}

#define PROCESS_COLOR() \
			\
			streamByte = colorByte; \
			streamByte &= 0x0f; \
			streamByte += mainBright; \
			streamByte = shiftBright[streamByte]; \
			\
			colorByte &= 0xf0; \
			streamByte |= colorByte; \
			\
		  if (forceColor) \
			streamByte = forceColor; \
		  if (forceBWMode) \
			streamByte &= 0x0f;


#define WRITE_COLOR(X)    \
    { \
		colorByte = *sd_ptr_color++; \
		PROCESS_COLOR() \
		WRITE_DATA(X, streamByte) \
    }

#define WRITE_COLOR_BK(X)    \
    { \
		colorByte = *sd_ptr_colorbk++; \
		PROCESS_COLOR() \
		WRITE_DATA(X, streamByte) \
    }

#define WRITE_AUDIO_DATA(X, Y)    \
    { \
	streamByte = volumeScale[Y]; \
	WRITE_DATA(X, streamByte) \
    }

#define WRITE_AUDIO(X)    \
    { \
	streamByte = *sd_ptr_audio++; \
	WRITE_AUDIO_DATA(X, streamByte ) \
    }

#define WRITE_GRAPH(X)    \
    { \
    if (sd_ptr_graph_override) \
        streamByte = *sd_ptr_graph_override++; \
    else \
        streamByte = *sd_ptr_graph++; \
	WRITE_DATA(X, streamByte) \
    }

void
fill_addr_right_line()
{
	SET_WRITE_PAGE(addr_right_line)

	WRITE_AUDIO(addr_set_aud_right + 1);

	WRITE_GRAPH(addr_set_gdata5 + 1);
	WRITE_GRAPH(addr_set_gdata6 + 1);
	WRITE_GRAPH(addr_set_gdata7 + 1);
	WRITE_GRAPH(addr_set_gdata8 + 1);
	WRITE_GRAPH(addr_set_gdata9 + 1);

	WRITE_COLOR(addr_set_gcol5 + 1);
	WRITE_COLOR(addr_set_gcol6 + 1);
	WRITE_COLOR(addr_set_gcol7 + 1);
	WRITE_COLOR(addr_set_gcol8 + 1);
	WRITE_COLOR(addr_set_gcol9 + 1);

	// alternate between background color and playfield color
    if (forceColor)
    {
        WRITE_DATA(addr_set_colubk_r + 1, 0);
    }
    else
    {
        WRITE_COLOR_BK(addr_set_colubk_r + 1);
    }

}

void
fill_addr_left_line(bool again)
{
	SET_WRITE_PAGE(addr_left_line)

	WRITE_AUDIO(addr_set_aud_left + 1);

	WRITE_GRAPH(addr_set_gdata0 + 1);
	WRITE_GRAPH(addr_set_gdata1 + 1);
	WRITE_GRAPH(addr_set_gdata2 + 1);
	WRITE_GRAPH(addr_set_gdata3 + 1);
	WRITE_GRAPH(addr_set_gdata4 + 1);

	WRITE_COLOR(addr_set_gcol0 + 1);
	WRITE_COLOR(addr_set_gcol1 + 1);
	WRITE_COLOR(addr_set_gcol2 + 1);
	WRITE_COLOR(addr_set_gcol3 + 1);
	WRITE_COLOR(addr_set_gcol4 + 1);

	// alternate between background color and playfield color
	if (forceColor)
	{
		WRITE_DATA(addr_set_colupf_l + 1, 0);
	}
	else
	{
		WRITE_COLOR_BK(addr_set_colupf_l + 1);
	}


	// addr_pick_line_end = 0x0ee;
	//		jmp right_line
	//		jmp end_lines
	if (again)
	{
		WRITE_DATA(addr_pick_continue + 1, LO_JUMP_BYTE(addr_right_line))
		WRITE_DATA(addr_pick_continue + 2, HI_JUMP_BYTE(addr_right_line))
	}
	else
	{
		WRITE_DATA(addr_pick_continue + 1, LO_JUMP_BYTE(addr_end_lines))
		WRITE_DATA(addr_pick_continue + 2, HI_JUMP_BYTE(addr_end_lines))
	}
}


void
fill_addr_end_lines()
{
	SET_WRITE_PAGE(addr_end_lines)

	WRITE_AUDIO(addr_set_aud_endlines + 1);

	if (!odd)
		firstAudVal = *sd_ptr_audio++;

	// keep at overscan=29, vblank=36
	//      or overscan=30, vblank=36 + 1 blank line

	if (odd)
	{
		WRITE_DATA(addr_set_overscan_size + 1, 29);
		WRITE_DATA(addr_set_vblank_size + 1, 36);

		WRITE_DATA(addr_pick_extra_lines + 1, 0);
	}
	else
	{
		WRITE_DATA(addr_set_overscan_size + 1, 30);
		WRITE_DATA(addr_set_vblank_size + 1, 36);

		// extra line after vblank
		WRITE_DATA(addr_pick_extra_lines + 1, 1);
	}

	if (bufferIndex == false)
	{
		WRITE_DATA(addr_pick_transport + 1, LO_JUMP_BYTE(addr_transport_direction));
		WRITE_DATA(addr_pick_transport + 2, HI_JUMP_BYTE(addr_transport_direction));
	}
	else
	{
		WRITE_DATA(addr_pick_transport + 1, LO_JUMP_BYTE(addr_transport_buttons));
		WRITE_DATA(addr_pick_transport + 2, HI_JUMP_BYTE(addr_transport_buttons));
	}

}

void
fill_addr_blank_lines()
{
	uint8_t	i;

	// version number
	READ_VERSION();
	READ_VERSION();
	READ_VERSION();
	READ_VERSION();

	READ_FRAME();    // frame number
	READ_FRAME();    // frame number
	READ_FRAME();    // frame number
    
    if (streamByte & 4) // debug flash during play
    {
        TEST_LED_ON;
    }
    else
    {
        TEST_LED_OFF;
    }
	
	// make sure we're in sync with frame data
	odd = (streamByte & 1);

	SET_WRITE_PAGE(addr_audio_bank)

	// 30 overscan
	// 3 vsync
	// 37 vblank

	if (odd)
	{
		WRITE_AUDIO_DATA(addr_audio_bank + 0, firstAudVal);
		for (i = 1; i < (BLANK_LINE_SIZE + 1); i++)
			WRITE_AUDIO(addr_audio_bank + i);
	}
	else
	{
		for (i = 0; i < (BLANK_LINE_SIZE -1); i++)
			WRITE_AUDIO(addr_audio_bank + i);
	}
}

void
stopTitleScreen()
{
	BEGIN_WRITE()
		SET_WRITE_PAGE(addr_title_loop)
		WRITE_DATA(addr_title_loop + 0, 0x18);  // clear carry, one bit difference from 0x38 sec
	END_WRITE()
}

bool
initState()
{
	// core already filled
			
	state = 3;
    play = 1;
	odd = 1;

	if (!sd_openStream())
		return false;
    
    volumeScale = scales[DEFAULT_LEVEL];

	sd_swapField(true, odd);

	A10_COUNT = 0;

	return true;
}

void
runStateMachine()
{   

again:

	switch(state)
	{
		case 1:
			WHILE_NOT_A7();
			sd_runReadState();
		
			if (lines == (TIMECODE_HEIGHT-1))
			{
				if (drawTimeCode)
				{
					drawTimeCode--;
					forceColor = COLOR_BLUE;
					sd_ptr_graph = sd_ptr_timecode;
				}
			}

			// label = 12, bars = 7
			if (lines == 21)
			{
				if (drawLevelBars)
				{
					drawLevelBars--;
					forceColor = COLOR_BLUE;

					switch(mainMode)
					{
						case MODE_TIME:
							sd_ptr_graph_override = nullptr;
							break;

						case MODE_BRIGHT:
							if(odd)
								sd_ptr_graph_override = brightLabelOdd;
							else
								sd_ptr_graph_override = brightLabelEven;
							break;

						case MODE_VOLUME:
						default:
							if(odd)
								sd_ptr_graph_override = volumeLabelOdd;
							else
								sd_ptr_graph_override = volumeLabelEven;
							break;
					}
				}
			}

			if (lines == 7)
			{
				if (drawLevelBars)
				{
					uint8_t levelValue = 0;

					switch(mainMode)
					{
						case MODE_TIME:
							levelValue = 0;
							break;

						case MODE_BRIGHT:
							levelValue = mainBright;
							break;

						case MODE_VOLUME:
						default:
							levelValue = mainVolume;
							break;
					}

					if(odd)
						sd_ptr_graph_override = (&levelBarsOddData[levelValue * 40]);
					else
						sd_ptr_graph_override = (&levelBarsEvenData[levelValue * 40]);
				}
			}


			BEGIN_WRITE()
			fill_addr_right_line();
			END_WRITE()

			lines -= 1;
			state = 2;
			break;


		case 2:
			WHILE_A7();
			sd_runReadState();

			if(odd)
			{
				if(drawTimeCode)
				{
					if (lines == (TIMECODE_HEIGHT - 0))
						sd_blankPartialLines(true);
				}
				if(drawLevelBars)
				{
					if(lines == 22)
						sd_blankPartialLines(true);
				}
			}

			if (lines >= 1)
			{
				BEGIN_WRITE()
				fill_addr_left_line(1);
				END_WRITE()

				lines -= 1;
				state = 1;
			}
			else
			{
				BEGIN_WRITE()
					fill_addr_left_line(0);
					fill_addr_end_lines();
				END_WRITE()

				sd_swapField(bufferIndex, odd);
				sd_blankPartialLines(odd);

				bufferIndex = !bufferIndex;
				updateTransport();

				BEGIN_WRITE()
					fill_addr_blank_lines();
				END_WRITE()

				state = 3;
			}
			break;

		case 3:
 			WHILE_NOT_A7();
			sd_readField(frameNumber, bufferIndex);
			forceColor = 0;
			lines = 191;
			state = 1;
			break;

		default:
			break;
	}       

	STATE_LOOP
}


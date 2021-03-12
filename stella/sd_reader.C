

#include <windows.h>
#include <stdio.h>
#include "defines.h"

#define FRAME_DATA_OFFSET		1
#define AUDIO_DATA_OFFSET		4
#define GRAPH_DATA_OFFSET		266
#define TIMECODE_DATA_OFFSET	1226
#define COLOR_DATA_OFFSET		1286
#define END_DATA_OFFSET			2246


uint8_t		*sd_ptr_audio = 0;
uint8_t		*sd_ptr_graph = 0;
uint8_t		*sd_ptr_timecode = 0;
uint8_t		*sd_ptr_color = 0;
uint8_t		*sd_ptr_data = 0;

static bool initStream = true;
static FILE *disk_stream = nullptr;

void
sd_runReadState()
{
}

void
sd_swapField(bool index)
{
	if (index == true)
	{
		sd_ptr_data  = stream_buffer1;
		sd_ptr_audio = stream_buffer1 + AUDIO_DATA_OFFSET;
		sd_ptr_graph = stream_buffer1 + GRAPH_DATA_OFFSET;
		sd_ptr_timecode= stream_buffer1 + TIMECODE_DATA_OFFSET;
		sd_ptr_color = stream_buffer1 + COLOR_DATA_OFFSET;
	}
	else
	{
		sd_ptr_data  = stream_buffer2;
		sd_ptr_audio = stream_buffer2 + AUDIO_DATA_OFFSET;
		sd_ptr_graph = stream_buffer2 + GRAPH_DATA_OFFSET;
		sd_ptr_timecode = stream_buffer2 + TIMECODE_DATA_OFFSET;
		sd_ptr_color = stream_buffer2 + COLOR_DATA_OFFSET;
	}
}


bool
sd_openStream()
{
	if (initStream)
	{
		initStream = false;
		disk_stream = fopen("output_cronkite.dat", "rb");	
	}

	return disk_stream ? true:false;
}

void
sd_readField(uint32_t fnum, bool index)
{
	fseek(disk_stream, (fnum + 0) * (8 * 512), SEEK_SET) ;

	if (index == true)
	{
		fread((void *)stream_buffer1, 1, FIELD_SIZE, disk_stream);

		// sanity check
		while (sd_ptr_graph >= stream_buffer1 && sd_ptr_graph <= (stream_buffer1 + FIELD_SIZE))
		{
		}
	}
	else
	{
		fread((void *)stream_buffer2, 1, FIELD_SIZE, disk_stream);

		// sanity check
		while (sd_ptr_graph >= stream_buffer2 && sd_ptr_graph <= (stream_buffer2 + FIELD_SIZE))
		{
		}
	}
}


#include "frame.h"

struct FrameFormat
{
	uint8_t version[4];   // ('M', 'V', 'C', 0)
	uint8_t format;       // ( 1-------)
	uint8_t timecode[4];  // (hour, minute, second, fame)
	uint8_t vsync;        // eg 3
	uint8_t vblank;       // eg 37
	uint8_t overscan;     // eg 30
	uint8_t visible;      // eg 192
	uint8_t rate;         // eg 60
    
//    dataStart;

	// sound[vsync+blank+overscan+visible]
	// graph[5 * visible]
	// color[5 * visible]
	// bkcolor[1 * visible]
	// timecode[60]
	// padding
};

void
frameInit(struct frameInfo* fInfo)
{
	uint8_t*	dst = fInfo->buffer;
	struct 		FrameFormat* ff = (struct FrameFormat* )dst;
	uint16_t	headerSize;


	if (ff->format & 0x80)
	{
		headerSize = sizeof(*ff);

		fInfo->vsyncLines = ff->vsync;
		fInfo->blankLines = ff->vblank;
		fInfo->overscanLines = ff->overscan;
		fInfo->visibleLines = ff->visible;
        fInfo->odd = !(ff->timecode[3] & 1);
		fInfo->totalLines = fInfo->vsyncLines + fInfo->blankLines + fInfo->overscanLines + fInfo->visibleLines;

        fInfo->audioBuf    = dst + headerSize;        
		fInfo->graphBuf    = fInfo->audioBuf + fInfo->totalLines;
		fInfo->colorBuf    = ((uint8_t*)fInfo->graphBuf) + 5 * fInfo->visibleLines;
		fInfo->colorBKBuf  = fInfo->colorBuf + 5 * fInfo->visibleLines;
		fInfo->timecodeBuf = fInfo->colorBKBuf + 1 * fInfo->visibleLines;
	}
	else
	{
		// old format
		// 'M', 'V', 'C', 0, f2, f1, f0
		headerSize = (4 + 3);

		fInfo->vsyncLines = 3;
		fInfo->blankLines = 37;
		fInfo->overscanLines = 30;
		fInfo->visibleLines = 192;
		fInfo->odd = (dst[4 + 3 -1] & 1);
		fInfo->totalLines = fInfo->vsyncLines + fInfo->blankLines + fInfo->overscanLines + fInfo->visibleLines;

		fInfo->audioBuf    = dst + headerSize;
		fInfo->graphBuf    = fInfo->audioBuf + fInfo->totalLines;
		fInfo->timecodeBuf = ((uint8_t*)fInfo->graphBuf) + 5*fInfo->visibleLines;
		fInfo->colorBuf    = ((uint8_t*)fInfo->timecodeBuf) + 60;
		fInfo->colorBKBuf  = fInfo->colorBuf + 5*fInfo->visibleLines;
	}

	if (fInfo->odd)
		fInfo->colorBKBuf++;


	// Calculate total blocks needed

	// sound[vsync+blank+overscan+visible]
	// graph[5 * visible]
	// color[5 * visible]
	// bkcolor[1 * visible]
	// timecode[60]
	// padding

	uint16_t		totalSize = fInfo->totalLines + fInfo->visibleLines*(5 + 5 + 1) + (5*12) + headerSize;

	fInfo->numBlocks = (totalSize >> 9);	// 512 block chunks
	if (totalSize & (512-1))
		fInfo->numBlocks++;

	fInfo->buffer = dst;
}


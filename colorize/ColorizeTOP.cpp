/* Shared Use License: This file is owned by Derivative Inc. (Derivative) and
 * can only be used, and/or modified for use, in conjunction with 
 * Derivative's TouchDesigner software, and only if you are a licensee who has
 * accepted Derivative's TouchDesigner license or assignment agreement (which
 * also govern the use of this file).  You may share a modified version of this
 * file with another authorized licensee of Derivative's TouchDesigner software.
 * Otherwise, no redistribution or sharing of this file, with or without
 * modification, is permitted.
 */

#include "ColorizeTOP.h"

#include <stdio.h>
#include <string.h>


// These functions are basic C function, which the DLL loader can find
// much easier than finding a C++ Class.
// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
// you are creating
extern "C"
{

DLLEXPORT
void
FillTOPPluginInfo(TOP_PluginInfo *info)
{
	// This must always be set to this constant
	info->apiVersion = TOPCPlusPlusAPIVersion;

	// Change this to change the executeMode behavior of this plugin.
	info->executeMode = TOP_ExecuteMode::CPUMemWriteOnly;

	// The opType is the unique name for this TOP. It must start with a 
	// capital A-Z character, and all the following characters must lower case
	// or numbers (a-z, 0-9)
	info->customOPInfo.opType->setString("Colorize");

	// The opLabel is the text that will show up in the OP Create Dialog
	info->customOPInfo.opLabel->setString("Colorize");

	// Will be turned into a 3 letter icon on the nodes
	info->customOPInfo.opIcon->setString("COL");

	// Information about the author of this OP
	info->customOPInfo.authorName->setString("Lodef Mode");
	info->customOPInfo.authorEmail->setString("lodef.mode@gmail.com");

	// This TOP works with 0 or 1 inputs connected
	info->customOPInfo.minInputs = 1;
	info->customOPInfo.maxInputs = 1;
}

DLLEXPORT
TOP_CPlusPlusBase*
CreateTOPInstance(const OP_NodeInfo* info, TOP_Context* context)
{
	// Return a new instance of your class every time this is called.
	// It will be called once per TOP that is using the .dll
	return new CPUMemoryTOP(info);
}

DLLEXPORT
void
DestroyTOPInstance(TOP_CPlusPlusBase* instance, TOP_Context *context)
{
	// Delete the instance here, this will be called when
	// Touch is shutting down, when the TOP using that instance is deleted, or
	// if the TOP loads a different DLL
	delete (CPUMemoryTOP*)instance;
}

};


CPUMemoryTOP::CPUMemoryTOP(const OP_NodeInfo* info) 
{
	myResultWidth = 0;
	myResultHeight = 0;
	myResultGraph = nullptr;
	myResultColor = nullptr;
	myResultBK = nullptr;
}

CPUMemoryTOP::~CPUMemoryTOP()
{
	if (myResultGraph)
		delete [] myResultGraph;
	if (myResultColor)
		delete [] myResultColor;
	if (myResultBK)
		delete [] myResultBK;
}

void
CPUMemoryTOP::getGeneralInfo(TOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
    ginfo->memPixelType = OP_CPUMemPixelType::RGBA32Float;
}

bool
CPUMemoryTOP::getOutputFormat(TOP_OutputFormat* format, const OP_Inputs* inputs, void* reserved1)
{
	// In this function we could assign variable values to 'format' to specify
	// the pixel format/resolution etc that we want to output to.
	// If we did that, we'd want to return true to tell the TOP to use the settings we've
	// specified.
	// In this example we'll return false and use the TOP's settings
	return false;
}




float
colorDist(const float a[3], const float b[3])
{
	float dist =
		(a[0] - b[0])*(a[0] - b[0]) * 0.3f +
		(a[1] - b[1])*(a[1] - b[1]) * 0.6f +
		(a[2] - b[2])*(a[2] - b[2]) * 0.1f;
	
	return dist;
}

unsigned int
rubik_palette[6*3] =
{
  0x00, 0x9b, 0x48,
  0xff, 0xff, 0xff,
  0xb7, 0x12, 0x34,
  0xff, 0xd5, 0x00,
  0x00, 0x46, 0xad,
  0xff, 0x58, 0x00
};

// stella uInt32 Console::ourNTSCPalette[128] = 
unsigned int
atari2600ntsc_palette[128*3] =
{
  0x00, 0x00, 0x00,
  0x4a, 0x4a, 0x4a,
  0x6f, 0x6f, 0x6f,
  0x8e, 0x8e, 0x8e,
  0xaa, 0xaa, 0xaa,
  0xc0, 0xc0, 0xc0,
  0xd6, 0xd6, 0xd6,
  0xec, 0xec, 0xec,
  0x48, 0x48, 0x00,
  0x69, 0x69, 0x0f,
  0x86, 0x86, 0x1d,
  0xa2, 0xa2, 0x2a,
  0xbb, 0xbb, 0x35,
  0xd2, 0xd2, 0x40,
  0xe8, 0xe8, 0x4a,
  0xfc, 0xfc, 0x54,
  0x7c, 0x2c, 0x00,
  0x90, 0x48, 0x11,
  0xa2, 0x62, 0x21,
  0xb4, 0x7a, 0x30,
  0xc3, 0x90, 0x3d,
  0xd2, 0xa4, 0x4a,
  0xdf, 0xb7, 0x55,
  0xec, 0xc8, 0x60,
  0x90, 0x1c, 0x00,
  0xa3, 0x39, 0x15,
  0xb5, 0x53, 0x28,
  0xc6, 0x6c, 0x3a,
  0xd5, 0x82, 0x4a,
  0xe3, 0x97, 0x59,
  0xf0, 0xaa, 0x67,
  0xfc, 0xbc, 0x74,
  0x94, 0x00, 0x00, 
  0xa7, 0x1a, 0x1a,
  0xb8, 0x32, 0x32,
  0xc8, 0x48, 0x48,
  0xd6, 0x5c, 0x5c,
  0xe4, 0x6f, 0x6f,
  0xf0, 0x80, 0x80,
  0xfc, 0x90, 0x90,
  0x84, 0x00, 0x64,
  0x97, 0x19, 0x7a,
  0xa8, 0x30, 0x8f,
  0xb8, 0x46, 0xa2,
  0xc6, 0x59, 0xb3,
  0xd4, 0x6c, 0xc3,
  0xe0, 0x7c, 0xd2,
  0xec, 0x8c, 0xe0,
  0x50, 0x00, 0x84,
  0x68, 0x19, 0x9a,
  0x7d, 0x30, 0xad,
  0x92, 0x46, 0xc0,
  0xa4, 0x59, 0xd0,
  0xb5, 0x6c, 0xe0,
  0xc5, 0x7c, 0xee,
  0xd4, 0x8c, 0xfc,
  0x14, 0x00, 0x90,
  0x33, 0x1a, 0xa3,
  0x4e, 0x32, 0xb5,
  0x68, 0x48, 0xc6,
  0x7f, 0x5c, 0xd5,
  0x95, 0x6f, 0xe3,
  0xa9, 0x80, 0xf0,
  0xbc, 0x90, 0xfc,
  0x00, 0x00, 0x94,
  0x18, 0x1a, 0xa7,
  0x2d, 0x32, 0xb8,
  0x42, 0x48, 0xc8,
  0x54, 0x5c, 0xd6,
  0x65, 0x6f, 0xe4,
  0x75, 0x80, 0xf0,
  0x84, 0x90, 0xfc,
  0x00, 0x1c, 0x88,
  0x18, 0x3b, 0x9d,
  0x2d, 0x57, 0xb0,
  0x42, 0x72, 0xc2,
  0x54, 0x8a, 0xd2,
  0x65, 0xa0, 0xe1,
  0x75, 0xb5, 0xef,
  0x84, 0xc8, 0xfc,
  0x00, 0x30, 0x64,
  0x18, 0x50, 0x80,
  0x2d, 0x6d, 0x98,
  0x42, 0x88, 0xb0,
  0x54, 0xa0, 0xc5,
  0x65, 0xb7, 0xd9,
  0x75, 0xcc, 0xeb,
  0x84, 0xe0, 0xfc,
  0x00, 0x40, 0x30,
  0x18, 0x62, 0x4e,
  0x2d, 0x81, 0x69,
  0x42, 0x9e, 0x82,
  0x54, 0xb8, 0x99,
  0x65, 0xd1, 0xae,
  0x75, 0xe7, 0xc2,
  0x84, 0xfc, 0xd4,
  0x00, 0x44, 0x00,
  0x1a, 0x66, 0x1a,
  0x32, 0x84, 0x32,
  0x48, 0xa0, 0x48,
  0x5c, 0xba, 0x5c,
  0x6f, 0xd2, 0x6f,
  0x80, 0xe8, 0x80,
  0x90, 0xfc, 0x90,
  0x14, 0x3c, 0x00,
  0x35, 0x5f, 0x18,
  0x52, 0x7e, 0x2d,
  0x6e, 0x9c, 0x42,
  0x87, 0xb7, 0x54,
  0x9e, 0xd0, 0x65,
  0xb4, 0xe7, 0x75,
  0xc8, 0xfc, 0x84,
  0x30, 0x38, 0x00,
  0x50, 0x59, 0x16,
  0x6d, 0x76, 0x2b,
  0x88, 0x92, 0x3e,
  0xa0, 0xab, 0x4f,
  0xb7, 0xc2, 0x5f,
  0xcc, 0xd8, 0x6e,
  0xe0, 0xec, 0x7c,
  0x48, 0x2c, 0x00,
  0x69, 0x4d, 0x14,
  0x86, 0x6a, 0x26,
  0xa2, 0x86, 0x38,
  0xbb, 0x9f, 0x47,
  0xd2, 0xb6, 0x56,
  0xe8, 0xcc, 0x63,
  0xfc, 0xe0, 0x70,
};

// random terrain
unsigned int
atari2600randomterrain_palette[128*3] =
{
	0x00,0x00,0x00,
	0x1A,0x1A,0x1A,
	0x39,0x39,0x39,
	0x5B,0x5B,0x5B,
	0x7E,0x7E,0x7E,
	0xA2,0xA2,0xA2,
	0xC7,0xC7,0xC7,
	0xED,0xED,0xED,
	0x19,0x02,0x00,
	0x3A,0x1F,0x00,
	0x5D,0x41,0x00,
	0x82,0x64,0x00,
	0xA7,0x88,0x00,
	0xCC,0xAD,0x00,
	0xF2,0xD2,0x19,
	0xFE,0xFA,0x40,
	0x37,0x00,0x00,
	0x5E,0x08,0x00,
	0x83,0x27,0x00,
	0xA9,0x49,0x00,
	0xCF,0x6C,0x00,
	0xF5,0x8F,0x17,
	0xFE,0xB4,0x38,
	0xFE,0xDF,0x6F,
	0x47,0x00,0x00,
	0x73,0x00,0x00,
	0x98,0x13,0x00,
	0xBE,0x32,0x16,
	0xE4,0x53,0x35,
	0xFE,0x76,0x57,
	0xFE,0x9C,0x81,
	0xFE,0xC6,0xBB,
	0x44,0x00,0x08,
	0x6F,0x00,0x1F,
	0x96,0x06,0x40,
	0xBB,0x24,0x62,
	0xE1,0x45,0x85,
	0xFE,0x67,0xAA,
	0xFE,0x8C,0xD6,
	0xFE,0xB7,0xF6,
	0x2D,0x00,0x4A,
	0x57,0x00,0x67,
	0x7D,0x05,0x8C,
	0xA1,0x22,0xB1,
	0xC7,0x43,0xD7,
	0xED,0x65,0xFE,
	0xFE,0x8A,0xF6,
	0xFE,0xB5,0xF7,
	0x0D,0x00,0x82,
	0x33,0x00,0xA2,
	0x55,0x0F,0xC9,
	0x78,0x2D,0xF0,
	0x9C,0x4E,0xFE,
	0xC3,0x72,0xFE,
	0xEB,0x98,0xFE,
	0xFE,0xC0,0xF9,
	0x00,0x00,0x91,
	0x0A,0x05,0xBD,
	0x28,0x22,0xE4,
	0x48,0x42,0xFE,
	0x6B,0x64,0xFE,
	0x90,0x8A,0xFE,
	0xB7,0xB0,0xFE,
	0xDF,0xD8,0xFE,
	0x00,0x00,0x72,
	0x00,0x1C,0xAB,
	0x03,0x3C,0xD6,
	0x20,0x5E,0xFD,
	0x40,0x81,0xFE,
	0x64,0xA6,0xFE,
	0x89,0xCE,0xFE,
	0xB0,0xF6,0xFE,
	0x00,0x10,0x3A,
	0x00,0x31,0x6E,
	0x00,0x55,0xA2,
	0x05,0x79,0xC8,
	0x23,0x9D,0xEE,
	0x44,0xC2,0xFE,
	0x68,0xE9,0xFE,
	0x8F,0xFE,0xFE,
	0x00,0x1F,0x02,
	0x00,0x43,0x26,
	0x00,0x69,0x57,
	0x00,0x8D,0x7A,
	0x1B,0xB1,0x9E,
	0x3B,0xD7,0xC3,
	0x5D,0xFE,0xE9,
	0x86,0xFE,0xFE,
	0x00,0x24,0x03,
	0x00,0x4A,0x05,
	0x00,0x70,0x0C,
	0x09,0x95,0x2B,
	0x28,0xBA,0x4C,
	0x49,0xE0,0x6E,
	0x6C,0xFE,0x92,
	0x97,0xFE,0xB5,
	0x00,0x21,0x02,
	0x00,0x46,0x04,
	0x08,0x6B,0x00,
	0x28,0x90,0x00,
	0x49,0xB5,0x09,
	0x6B,0xDB,0x28,
	0x8F,0xFE,0x49,
	0xBB,0xFE,0x69,
	0x00,0x15,0x01,
	0x10,0x36,0x00,
	0x30,0x59,0x00,
	0x53,0x7E,0x00,
	0x76,0xA3,0x00,
	0x9A,0xC8,0x00,
	0xBF,0xEE,0x1E,
	0xE8,0xFE,0x3E,
	0x1A,0x02,0x00,
	0x3B,0x1F,0x00,
	0x5E,0x41,0x00,
	0x83,0x64,0x00,
	0xA8,0x88,0x00,
	0xCE,0xAD,0x00,
	0xF4,0xD2,0x18,
	0xFE,0xFA,0x40,
	0x38,0x00,0x00,
	0x5F,0x08,0x00,
	0x84,0x27,0x00,
	0xAA,0x49,0x00,
	0xD0,0x6B,0x00,
	0xF6,0x8F,0x18,
	0xFE,0xB4,0x39,
	0xFE,0xDF,0x70,
};

unsigned int
bw2_palette[2*3] =
{
	0, 0, 0,
	255, 255, 255
};

unsigned int
bw4_palette[4*3] =
{
	0, 0, 0,
	85, 85, 85,
	170, 170, 170,
	255, 255, 255
};

unsigned int
rgb_palette[4*3] =
{
	0, 0, 0,
	0, 0, 255,
	0, 255, 0,
	255, 0, 0
};


inline void
findClosestInPalette(float cellColor[4], unsigned int *pal, int palSize)
{
	float	mindist = -1;
	int		minIndex = 0;

	for (int i=0; i<palSize; i++)
	{
		float color[4];

		color[0] = pal[i*3 + 0] / 255.0f;
		color[1] = pal[i*3 + 1] / 255.0f;
		color[2] = pal[i*3 + 2] / 255.0f;
		color[3] = (float)i;

		float dist = colorDist(cellColor, color);
		if (mindist < 0 || dist < mindist)
		{
			mindist = dist;
			minIndex = i;
		}
	}

	cellColor[0] = pal[minIndex*3 + 0] / 255.0f;
	cellColor[1] = pal[minIndex*3 + 1] / 255.0f;
	cellColor[2] = pal[minIndex*3 + 2] / 255.0f;
	cellColor[3] = (float)minIndex;
}

inline int
findClosest(float cellColor[4], const float *selectColor, const float *backColor)
{
	float distBlack = colorDist(cellColor, backColor);
	float distWhite = colorDist(cellColor, selectColor);

	if (distBlack < distWhite)
	{
		memcpy(cellColor, backColor, sizeof(float)*4);
		return 0;
	}
	else
	{
		memcpy(cellColor, selectColor, sizeof(float)*4);
		return 1;
	}
}

inline void
distributeError(int width, int height, float *mem,
				int x, int y, float *quantError, float ratio)
{
	if (x >=0 && x<width && y>=0 && y<height)
	{
		float* npixel = &mem[4 * (y*width + x)];
		npixel[0] += quantError[0] * ratio;
		npixel[1] += quantError[1] * ratio;
		npixel[2] += quantError[2] * ratio;

		// clamp
		for (int j=0; j<3; j++)
		{
			if (npixel[j] < 0)
				npixel[j] = 0;
			else if (npixel[j] > 1)
				npixel[j] = 1;
		}

	}
}

#define max(a,b)  ((a)>(b) ? (a):(b))
#define min(a,b)  ((a)<(b) ? (a):(b))

float
RGBtoHue(float r, float g, float b)
{
	float maxcol, mincol;
	float h = 0;

	maxcol = max(r, max(g, b));
	mincol = min(r, min(g, b));

	if (maxcol == mincol)
	{                       /* achromatic case */
		h = 0;
	}
	else
	{
		float diff = maxcol - mincol;

		/* find hue */
		float rc = (maxcol - r) / diff;
		float gc = (maxcol - g) / diff;
		float bc = (maxcol - b) / diff;

		if (r == maxcol)
			h = bc - gc;
		else if (g == maxcol)
			h = rc - bc;
		else
			h = gc - rc;
	}

	return h;
}

float
RGBtoLum(float r, float g, float b)
{
	float maxcol, mincol;

	maxcol = max(r, max(g, b));
	mincol = min(r, min(g, b));

	return r;

	return (maxcol + mincol) / 2.0f;
}

enum
{
	Palette_Atari2600NTSC = 0,
	Palette_BW2 = 1,
	Palette_BW4 = 2,
	Palette_RGB = 3,
	Palette_Atari2600RandomTerrain = 4,
	Palette_Rubik = 5,
};

enum
{
	Matrix_FloydSteinberg = 0,
	Matrix_JIN = 1,
	Matrix_Atkinson = 2
};

void
getPalette(int palette, unsigned int *&pal, int &palSize)
{
	switch(palette)
	{
		case Palette_Atari2600NTSC:
		default:
			pal = atari2600ntsc_palette;
			palSize = 128;
			break;

		case Palette_BW2:
			pal = bw2_palette;
			palSize = 2;
			break;

		case Palette_BW4:
			pal = bw4_palette;
			palSize = 4;
			break;

		case Palette_RGB:
			pal = rgb_palette;
			palSize = 4;
			break;

		case Palette_Atari2600RandomTerrain:
			pal = atari2600randomterrain_palette;
			palSize = 128;
			break;

		case Palette_Rubik:
			pal = rubik_palette;
			palSize = 6;
			break;
	}
}


void
ditherLine(int bidx, int y, bool finalB, int width, int height, int cellSize,
	float *curY, unsigned int *pal, int palSize, float bleed, int matrix,
	float *mem, bool dither, float *curError)
{
	float	cellColor[4] = { 1, 1, 1, 0 };
	float	backColor[4];

	backColor[0] = pal[bidx*3 + 0] / 255.0f;
	backColor[1] = pal[bidx*3 + 1] / 255.0f;
	backColor[2] = pal[bidx*3 + 2] / 255.0f;
	backColor[3] = (float)bidx;


	// background backup
	float	mem4[2048 * 4];

	// sanity check
	if (width > 2048)
		width = 2048;

	*curError = 0.0f;

	// if intermediate result, work on copy instead
	if (!finalB)
	{
		memcpy(mem4, curY, width*4 * sizeof(float));
		curY = mem4;
	}

	for (int x0 = 0; x0 < width; x0++)
	{
		int xpix = x0%cellSize;

		int x = x0;
		float* pixel = &curY[4*x];


		// determine cell color

		if (xpix == 0)	// first pixel of cell
		{
			float	sum[3] = { 0,0,0 };
			float	total_weight = 0.0f;

			for (int x2=0; x2<cellSize; x2++)
			{
				int x3 = x + x2;

				float* npixel = &curY[4*x3];
			
				float r = npixel[0];
				float g = npixel[1];
				float b = npixel[2];

				float weight = r*0.3f + g*0.6f + b*0.1f;
				//
				// weigh background minimally
				{
					float	dist = colorDist(npixel, backColor);
					weight *= dist;
				}

				sum[0] += r*weight;
				sum[1] += g*weight;
				sum[2] += b*weight;

				total_weight += weight;
			}

			if (total_weight)
			{
				for (int i = 0; i < 3; i++)
					cellColor[i] = sum[i] / total_weight;
			}
			else
			{
				for (int i = 0; i < 3; i++)
					cellColor[i] = 0.0f;
			}

			findClosestInPalette(cellColor, pal, palSize);
		}

		// now dither
		if (dither)
		{
			float current[3];
			current[0] = pixel[0];
			current[1] = pixel[1];
			current[2] = pixel[2];

			findClosest(pixel, cellColor, backColor);

			if (bleed > 0)
			{
				float quantError[3];
				
				for (int i = 0; i < 3; i++)
				{
					quantError[i] = (current[i] - pixel[i]) * bleed;
					*curError += fabs(quantError[i]);
				}

				switch(matrix)
				{
					case Matrix_FloydSteinberg:
					default:
						distributeError(width, height, curY, x+1, 0, quantError, 7.0f / 16.0f);

						if (finalB)
						{
							distributeError(width, height, mem, x-1, y+1, quantError, 3.0f / 16.0f);
							distributeError(width, height, mem, x+0, y+1, quantError, 5.0f / 16.0f);
							distributeError(width, height, mem, x+1, y+1, quantError, 1.0f / 16.0f);
						}
						break;

					case Matrix_JIN:
					#if 0
							 -   -   X   7   5 
							 3   5   7   5   3
							 1   3   5   3   1
					 #endif
						distributeError(width, height, curY, x+1, 0, quantError, 7.0f / 48.0f);
						distributeError(width, height, curY, x+2, 0, quantError, 5.0f / 48.0f);

						if (finalB)
						{
							distributeError(width, height, mem, x-2, y+1, quantError, 3.0f / 48.0f);
							distributeError(width, height, mem, x-1, y+1, quantError, 5.0f / 48.0f);
							distributeError(width, height, mem, x+0, y+1, quantError, 7.0f / 48.0f);
							distributeError(width, height, mem, x+1, y+1, quantError, 5.0f / 48.0f);
							distributeError(width, height, mem, x+2, y+1, quantError, 3.0f / 48.0f);

							distributeError(width, height, mem, x-2, y+2, quantError, 1.0f / 48.0f);
							distributeError(width, height, mem, x-1, y+2, quantError, 3.0f / 48.0f);
							distributeError(width, height, mem, x+0, y+2, quantError, 5.0f / 48.0f);
							distributeError(width, height, mem, x+1, y+2, quantError, 3.0f / 48.0f);
							distributeError(width, height, mem, x+2, y+2, quantError, 1.0f / 48.0f);
						}

						break;

					case Matrix_Atkinson: // (partial error distribution 6/8)

					#if 0
						-   X   1   1 
						1   1   1
						-   1
				   #endif

						distributeError(width, height, curY, x+1, 0, quantError, 1.0f / 8.0f);
						distributeError(width, height, curY, x+2, 0, quantError, 1.0f / 8.0f);

						if (finalB)
						{
							distributeError(width, height, mem, x-1, y+1, quantError, 1.0f / 8.0f);
							distributeError(width, height, mem, x+0, y+1, quantError, 1.0f / 8.0f);
							distributeError(width, height, mem, x+1, y+1, quantError, 1.0f / 8.0f);

							distributeError(width, height, mem, x+0, y+2, quantError, 1.0f / 8.0f);
						}
						
						break;

				}

			}
		}
		else
		{
			memcpy(pixel, cellColor, 4*sizeof(float));
		}

	}
}


void
CPUMemoryTOP::execute(TOP_OutputFormatSpecs* outputFormat,
						const OP_Inputs* inputs,
						TOP_Context *context,
						void* reserved1)
{
	bool active = inputs->getParInt("Active") ? true:false;
	int	palette = inputs->getParInt("Palette");
	int	cellSize = inputs->getParInt("Cellsize");

	bool dither = inputs->getParInt("Dither") ? true:false;
	float bleed = (float)inputs->getParDouble("Bleed");

	int matrix = inputs->getParInt("Matrix");

	bool background = inputs->getParInt("Background") ? true:false;

	unsigned int	*pal;
	int				palSize;
	getPalette(palette, pal, palSize);


	if (!active)
		return;

	OP_TOPInputDownloadOptions dlOptions;
	dlOptions.downloadType = OP_TOPInputDownloadType::Instant;

	const OP_TOPInput	*topInput = inputs->getInputTOP(0);
	const uint8_t	*topMem = topInput ? (uint8_t *)inputs->getTOPDataInCPUMemory(topInput, &dlOptions) : nullptr;


	int textureMemoryLocation = 0;
    float* mem = (float*)outputFormat->cpuPixelData[textureMemoryLocation];


	int width = outputFormat->width;
	int height = outputFormat->height;

	setupStorage(width, outputFormat->height, cellSize);

	// copy
	if (topMem)
	{
		const uint8_t	*topCopy = topMem;
		float			*dstPixel = mem;
		for (int y = 0; y < outputFormat->height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				// BRGA (8bit) to  RGBA  (float)

				dstPixel[0] = topCopy[2] / 255.0f;
				dstPixel[1] = topCopy[1] / 255.0f;
				dstPixel[2] = topCopy[0] / 255.0f;
				dstPixel[3] = 1.0f;

				topCopy += 4;
				dstPixel += 4;
			}
		}
	}

	if (!background)
	{
		bool	final = true;

		for (int y = 0; y < outputFormat->height; y++)
		{
			float* curY = &mem[4 * (y*width)];
			
			float	error;

			ditherLine(0, y, final, width, height, cellSize, curY,
				pal, palSize, bleed, matrix, mem, dither, &error);

			for (int i = 0; i < 4; i++)
				myResultBK[y * 4 + i] = 0.0f;
		}
	}
	else
	{
		// exhaustive search of each background color

		for (int y = 0; y < outputFormat->height; y++)
		{
			float* curY = &mem[4 * (y*width)];
			
			int		bestB = 0;
			float	bestError = -1.0f;


			for (int bidx=0; bidx<palSize; bidx++)
			{
				float	curError;
				bool	final = false;

				ditherLine(bidx, y, final, width, height, cellSize, curY, pal, palSize, bleed, matrix, mem, dither, &curError);

				if ((curError < bestError) || (bestError < 0))
				{
					bestError = curError;
					bestB = bidx;
				}
			}

			// redo best color
			{
				bool	final = true;
				float	error;
				int		bidx = bestB;

				ditherLine(bestB, y, final, width, height, cellSize, curY, pal, palSize, bleed, matrix, mem, dither, &error);

				float	backColor[4];
				backColor[0] = pal[bidx*3 + 0] / 255.0f;
				backColor[1] = pal[bidx*3 + 1] / 255.0f;
				backColor[2] = pal[bidx*3 + 2] / 255.0f;
				backColor[3] = (float)bidx;

				for (int i = 0; i < 4; i++)
					myResultBK[y * 4 + i] = backColor[i];
			}

		}
	}

	storeResults(width, height, mem, cellSize);

    outputFormat->newCPUPixelDataLocation = textureMemoryLocation;
    textureMemoryLocation = !textureMemoryLocation;
}

void
CPUMemoryTOP::setupStorage(int outputWidth, int outputHeight, int cellSize)
{
	outputWidth /= cellSize;
	if (outputWidth < 1)
		outputWidth = 1;

	if (myResultWidth != outputWidth || (myResultHeight != outputHeight))
	{
		if (myResultGraph)
			delete[] myResultGraph;
		if (myResultColor)
			delete[] myResultColor;
		if (myResultBK)
			delete[] myResultBK;

		myResultWidth = outputWidth;
		myResultHeight = outputHeight;
		myResultGraph = new uint8_t[myResultWidth * myResultHeight];
		myResultColor = new uint8_t[myResultWidth * myResultHeight];
		myResultBK = new float[myResultHeight * 4];
	}

}

void
CPUMemoryTOP::storeResults(int outputWidth, int outputHeight, float *mem, int cellSize)
{
	outputWidth /= cellSize;
	if (outputWidth < 1)
		outputWidth = 1;


	int		cnt = 0;

	for (int y = 0; y<outputHeight; y++)
	{
		int	bidx = (int)myResultBK[y*4 + 3];

		for (int x=0; x<outputWidth; x++, cnt++)
		{
			float* pixel = &mem[4 * cellSize * (y*outputWidth + x)];

			// take next cellSize rgb bits for dither
			uint8_t val = 0;
			uint8_t col = 0;
			float *npixel = pixel;
			for (int i = 0; i < cellSize; i++, npixel += 4)
			{
				val <<= 1;

				// only store non-backcolor in this array
				if (npixel[3] != bidx)
				{
					val |= 1;
					if (npixel[3])
						col = (uint8_t)npixel[3];

					// set alpha to solid
					npixel[3] = 1.0f;
				}
				else
				{
					// turn off background

					npixel[0] = 0.0f;
					npixel[1] = 0.0f;
					npixel[2] = 0.0f;
					npixel[3] = 0.0f;
				}

			}

			myResultGraph[cnt] = val;
			myResultColor[cnt] = col;
		}
	}
}

int32_t
CPUMemoryTOP::getNumInfoCHOPChans(void *reserved1)
{
	return 0;
}

void
CPUMemoryTOP::getInfoCHOPChan(int32_t index, OP_InfoCHOPChan* chan, void* reserved1)
{
}

bool		
CPUMemoryTOP::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
{
	infoSize->rows = myResultHeight;
	infoSize->cols = myResultWidth + myResultWidth + 4;		// graph, color,  bkground

	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void
CPUMemoryTOP::getInfoDATEntries(int32_t index,
								int32_t nCols,
								OP_InfoDATEntries* entries,
								void *reserved1)
{
	// It's safe to use static buffers here because Touch will make it's own
	// copies of the strings immediately after this call returns
	// (so the buffers can be reuse for each column/row)
	static char intBuffer[256][4];
	static bool first = true;
	if (first)
	{
		for (int i=0; i<256; i++)
#ifdef _WIN32
			sprintf_s(intBuffer[i], "%d", i);
#else
			snprintf(intBuffer[i], 4, "%d", i);
#endif
		first = false;
	}

	int y = myResultHeight - index - 1; // reverse
	int cntBase = (y*myResultWidth);

	// graph
	for (int i=0; i<myResultWidth; i++)
	{
		int x = i;
		int v = myResultGraph[cntBase + x];

		entries->values[i]->setString(intBuffer[v]);
	}

	// color

	for (int i=0; i<myResultWidth; i++)
	{
		int x = i;
		int v = myResultColor[cntBase + x];

		// top 7 bits only
		v <<= 1;

		entries->values[i + myResultWidth]->setString(intBuffer[v]);
	}


	// color bk

	{
		int v = (int)myResultBK[y*4 + 3];

		// top 7 bits only
		v <<= 1;

		entries->values[2*myResultWidth + 0]->setString(intBuffer[v]);

		for (int i=0; i<3; i++)
		{
			float	f = myResultBK[y*4 + i];
			char	fltBuffer[64];

#ifdef _WIN32
			sprintf_s(fltBuffer, "%g", f);
#else
			snprintf(fltBuffer, 4, "%g", f);
#endif

			entries->values[2*myResultWidth + i + 1]->setString(fltBuffer);
		}
	}


}

void
CPUMemoryTOP::setupParameters(OP_ParameterManager* manager, void *reserved)
{
	{
		OP_NumericParameter  sp;

		sp.name = "Active";
		sp.label = "Active";
		sp.defaultValues[0] = 1;

		manager->appendToggle(sp);
	}

	{
		OP_StringParameter  sp;

		sp.name = "Palette";
		sp.label = "Palette";

		const char *names[6] = { "Atari2600ntsc", "Bw2", "Bw4", "Rgb","Atario2600randomterrain", "Rubik" };
		const char *labels[6] = { "Atari 2600 NTSC", "B/W 2", "B/W 4", "RGB", "Atari 2600 Random Terrain", "Rubik" };

		manager->appendMenu(sp, 6, names, labels);
	}
	
	{
		OP_NumericParameter  sp;

		sp.name = "Cellsize";
		sp.label = "Cell Size";

		sp.defaultValues[0] = 8;

		sp.minValues[0] = 1;
		sp.clampMins[0] = true;

		sp.minSliders[0] = 1;
		sp.maxSliders[0] = 16;

		manager->appendInt(sp);
	}


	{
		OP_NumericParameter  sp;

		sp.name = "Dither";
		sp.label = "Dither";

		manager->appendToggle(sp);
	}

	{
		OP_NumericParameter  sp;

		sp.name = "Background";
		sp.label = "Background";

		manager->appendToggle(sp);
	}

	{
		OP_NumericParameter  sp;

		sp.name = "Bleed";
		sp.label = "Bleed";

		sp.defaultValues[0] = 1;

		manager->appendFloat(sp);
	}

	{
		OP_StringParameter  sp;

		sp.name = "Matrix";
		sp.label = "Matrix";

		const char *names[3] = { "Floydsteinberg", "Jin", "Atkinson" };
		const char *labels[3] = { "Floyd/Steinberg", "JIN", "Atkinson" };

		manager->appendMenu(sp, 3, names, labels);
	}

}

void
CPUMemoryTOP::pulsePressed(const char* name, void *reserved1)
{
}


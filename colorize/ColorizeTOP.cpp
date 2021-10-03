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
	myLastPal = nullptr;
	myMem = nullptr;
	myMemBackup = nullptr;
}

CPUMemoryTOP::~CPUMemoryTOP()
{
	releaseStorage();
}

void
CPUMemoryTOP::getGeneralInfo(TOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
    ginfo->memPixelType = OP_CPUMemPixelType::BGRA8Fixed;
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

const float	colorScales[3] = {0.299f, 0.587f, 0.114f};

float
colorDist(const float a[3], const float b[3])
{
	float dist =
		(a[0] - b[0])*(a[0] - b[0]) * colorScales[0] +
		(a[1] - b[1])*(a[1] - b[1]) * colorScales[1] +
		(a[2] - b[2])*(a[2] - b[2]) * colorScales[2];
	
	return dist;
}

/////////////////////////////////////////////////////////
/* Adapted from: https://rosettacode.org/wiki/K-d_tree */

 
inline float
dist(struct kd_node_t *a, struct kd_node_t *b)
{
	return colorDist(a->val, b->val);
}

inline void
swap_nodes(struct kd_node_t *x, struct kd_node_t *y) 
{
	kd_node_t	tmp;

	tmp.val[0] = x->val[0];
	tmp.val[1] = x->val[1];
	tmp.val[2] = x->val[2];
	tmp.index = x->index;

	x->val[0] = y->val[0];
	x->val[1] = y->val[1];
	x->val[2] = y->val[2];
	x->index = y->index;

	y->val[0] = tmp.val[0];
	y->val[1] = tmp.val[1];
	y->val[2] = tmp.val[2];
	y->index = tmp.index;
}
 
 
/* see quickselect method */
static struct kd_node_t*
find_median(struct kd_node_t *start, struct kd_node_t *end, int idx)
{
    if (end <= start)
		return nullptr;

    if (end == start + 1)
        return start;
 
    struct kd_node_t *p, *store, *md = start + (end - start) / 2;

    while (1) 
	{
		float pivot = md->val[idx];
 
        swap_nodes(md, end - 1);
        for (store = p = start; p < end; p++) 
		{
            if (p->val[idx] < pivot) 
			{
                if (p != store)
                    swap_nodes(p, store);
                store++;
            }
        }
        swap_nodes(store, end - 1);
 
        /* median has duplicate values */
        if (store->val[idx] == md->val[idx])
            return md;
 
        if (store > md)
			end = store;
        else
			start = store;
    }
}
 
static struct kd_node_t*
make_tree(struct kd_node_t *t, int len, int i)
{
    struct kd_node_t *n;
 
    if (!len)
		return 0;
 
    if ((n = find_median(t, t + len, i))) 
	{
        i = (i + 1) % MAX_DIM;
        n->left  = make_tree(t, n - t, i);
        n->right = make_tree(n + 1, t + len - (n + 1), i);
    }
    return n;
}
 
 
static void
nearest(struct kd_node_t *root, struct kd_node_t *nd, int i,
        struct kd_node_t **best, float *best_dist)
{
    float d, dx, dx2;
 
    if (!root)
		return;

    d = dist(root, nd);
    dx = root->val[i] - nd->val[i];

    dx2 = dx * dx;

	dx2 *= colorScales[i];

    if (!*best || d < *best_dist) 
	{
        *best_dist = d;
        *best = root;
    }
 
    /* if chance of exact match is high */
    if (!*best_dist)
		return;
 
    if (++i >= MAX_DIM)
		i = 0;
 
    nearest(dx > 0 ? root->left : root->right, nd, i, best, best_dist);

    if (dx2 >= *best_dist)
		return;

    nearest(dx > 0 ? root->right : root->left, nd, i, best, best_dist);
}
 
void
CPUMemoryTOP::setup_kdtree(const float *fpal, int palSize)
{
	if (palSize > 256)
		palSize = 256;

    for (int i=0; i<palSize; i++)
    {
		kd_node_t	&n = kdtree[i];

        n.val[0] = fpal[i*3 + 0];
        n.val[1] = fpal[i*3 + 1];
        n.val[2] = fpal[i*3 + 2];
		n.index = i;
	}

    kdtree_root = make_tree(kdtree, palSize, 0);
}

static int
search_kdtree(float r, float g, float b, kd_node_t* root)
{
	kd_node_t	n;

	n.val[0] = r;
	n.val[1] = g;
	n.val[2] = b;

    struct kd_node_t *found = nullptr;
    float best_dist;

    nearest(root, &n, 0, &found, &best_dist);

	if (found)
		return found->index;

	return -1;
}

/*              end of kd -tree                        */
/////////////////////////////////////////////////////////

void
CPUMemoryTOP::buildColourMap() 
{
    for (int r = 0; r < 256; r++) 
	{
        for (int g = 0; g < 256; g++) 
		{
            for (int b = 0; b < 256; b++) 
			{
                float rr = r / 255.0f;
                float gg = g / 255.0f;
                float bb = b / 255.0f;
                                
				int		minIndex = search_kdtree(rr, gg, bb, kdtree_root);

                myColorLookup[r][g][b] = minIndex;
            }
        }
    }
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


/*
 GIMP Palette
 Name: HW Atari 2600 (PAL)
 Columns: 16
 # https://en.wikipedia.org/wiki/List_of_video_game_console_palettes#Atari_2600
   0   0   0    0+1+14+15, 0
  40  40  40    0+1+14+15, 2
  80  80  80    0+1+14+15, 4
 116 116 116    0+1+14+15, 6
 148 148 148    0+1+14+15, 8
 180 180 180    0+1+14+15, 10
 208 208 208    0+1+14+15, 12
 236 236 236    0+1+14+15, 14
 128  88   0    2, 0
 148 112  32    2, 2
 168 132  60    2, 4
 188 156  88    2, 6
 204 172 112    2, 8
 220 192 132    2, 10
 236 208 156    2, 12
 252 224 176    2, 14
  68  92   0    3, 0
  92 120  32    3, 2
 116 144  60    3, 4
 140 172  88    3, 6
 160 192 112    3, 8
 176 212 132    3, 10
 196 232 156    3, 12
 212 252 176    3, 14
 112  52   0    4, 0
 136  80  32    4, 2
 160 104  60    4, 4
 180 132  88    4, 6
 200 152 112    4, 8
 220 172 132    4, 10
 236 192 156    4, 12
 252 212 176    4, 14
   0 100  20    5, 0
  32 128  52    5, 2
  60 152  80    5, 4
  88 176 108    5, 6
 112 196 132    5, 8
 132 216 156    5, 10
 156 232 180    5, 12
 176 252 200    5, 14
 112   0  20    6, 0
 136  32  52    6, 2
 160  60  80    6, 4
 180  88 108    6, 6
 200 112 132    6, 8
 220 132 156    6, 10
 236 156 180    6, 12
 252 176 200    6, 14
   0  92  92    7, 0
  32 116 116    7, 2
  60 140 140    7, 4
  88 164 164    7, 6
 112 184 184    7, 8
 132 200 200    7, 10
 156 220 220    7, 12
 176 236 236    7, 14
 112   0  92    8, 0
 132  32 116    8, 2
 148  60 136    8, 4
 168  88 156    8, 6
 180 112 176    8, 8
 196 132 192    8, 10
 208 156 208    8, 12
 224 176 224    8, 14
   0  60 112    9, 0
  28  88 136    9, 2
  56 116 160    9, 4
  80 140 180    9, 6
 104 164 200    9, 8
 124 184 220    9, 10
 144 204 236    9, 12
 164 224 252    9, 14
  88   0 112    10, 0
 108  32 136    10, 2
 128  60 160    10, 4
 148  88 180    10, 6
 164 112 200    10, 8
 180 132 220    10, 10
 196 156 236    10, 12
 212 176 252    10, 14
   0  32 112    11, 0
  28  60 136    11, 2
  56  88 160    11, 4
  80 116 180    11, 6
 104 136 200    11, 8
 124 160 220    11, 10
 144 180 236    11, 12
 164 200 252    11, 14
  60   0 128    12, 0
  84  32 148    12, 2
 108  60 168    12, 4
 128  88 188    12, 6
 148 112 204    12, 8
 168 132 220    12, 10
 184 156 236    12, 12
 200 176 252    12, 14
   0   0 136    13, 0
  32  32 156    13, 2
  60  60 176    13, 4
  88  88 192    13, 6
 112 112 208    13, 8
 132 132 224    13, 10
 156 156 236    13, 12
 176 176 252    13, 14
 
 */

/*
 GIMP Palette
 Name: HW Atari 2600 (SECAM)
 Columns: 8
 # https://en.wikipedia.org/wiki/List_of_video_game_console_palettes#Atari_2600
   0   0   0     0
  33  33 255     2
 240  60 121     4
 255  80 255     6
 127 255   0     8
 127 255 255     10
 255 255  63     12
 255 255 255     14
 */

unsigned int atari2600pal_palette[128*3] = 
{
      0,   0,   0,
     40,  40,  40,
     80,  80,  80,
    116, 116, 116,
    148, 148, 148,
    180, 180, 180,
    208, 208, 208,
    236, 236, 236,

    0,   0,   0,
     40,  40,  40,
     80,  80,  80,
    116, 116, 116,
    148, 148, 148,
    180, 180, 180,
    208, 208, 208,
    236, 236, 236,
    
    128,  88,   0,
    148, 112,  32,
    168, 132,  60,
    188, 156,  88,
    204, 172, 112,
    220, 192, 132,
    236, 208, 156,
    252, 224, 176,
    
    68,  92,   0,
     92, 120,  32,
    116, 144,  60,
    140, 172,  88,
    160, 192, 112,
    176, 212, 132,
    196, 232, 156,
    212, 252, 176,
    
    112,  52,   0,
    136,  80,  32,
    160, 104,  60,
    180, 132,  88,
    200, 152, 112,
    220, 172, 132,
    236, 192, 156,
    252, 212, 176,
    
    0, 100,  20,
     32, 128,  52,
     60, 152,  80,
     88, 176, 108,
    112, 196, 132,
    132, 216, 156,
    156, 232, 180,
    176, 252, 200,
    
    112,   0,  20,
    136,  32,  52,
    160,  60,  80,
    180,  88, 108,
    200, 112, 132,
    220, 132, 156,
    236, 156, 180,
    252, 176, 200,
    
    0,  92,  92,
     32, 116, 116,
     60, 140, 140,
     88, 164, 164,
    112, 184, 184,
    132, 200, 200,
    156, 220, 220,
    176, 236, 236,
    
    112,   0,  92,
    132,  32, 116,
    148,  60, 136,
    168,  88, 156,
    180, 112, 176,
    196, 132, 192,
    208, 156, 208,
    224, 176, 224,
    
    0,  60, 112,
     28,  88, 136,
     56, 116, 160,
     80, 140, 180,
    104, 164, 200,
    124, 184, 220,
    144, 204, 236,
    164, 224, 252,
    
    88,   0, 112,
    108,  32, 136,
    128,  60, 160,
    148,  88, 180,
    164, 112, 200,
    180, 132, 220,
    196, 156, 236,
    212, 176, 252,
    
    0,  32, 112,
     28,  60, 136,
     56,  88, 160,
     80, 116, 180,
    104, 136, 200,
    124, 160, 220,
    144, 180, 236,
    164, 200, 252,
    
    60,   0, 128,
     84,  32, 148,
    108,  60, 168,
    128,  88, 188,
    148, 112, 204,
    168, 132, 220,
    184, 156, 236,
    200, 176, 252,
    
    0,   0, 136,
     32,  32, 156,
     60,  60, 176,
     88,  88, 192,
    112, 112, 208,
    132, 132, 224,
    156, 156, 236,
    176, 176, 252,
    
    0,   0,   0,
     40,  40,  40,
     80,  80,  80,
    116, 116, 116,
    148, 148, 148,
    180, 180, 180,
    208, 208, 208,
    236, 236, 236,
    
    0,   0,   0,
     40,  40,  40,
     80,  80,  80,
    116, 116, 116,
    148, 148, 148,
    180, 180, 180,
    208, 208, 208,
    236, 236, 236,
};

unsigned int atari2600secam_palette[8*3] = 
{
      0,   0,   0,
     33,  33, 255,
    240,  60, 121,
    255,  80, 255,
    127, 255,   0,
    127, 255, 255,
    255, 255,  63,
    255, 255, 255,
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
lookupClosestInPalette(float cellColor[4], const float *fpal, const uint8_t lookup[256][256][256]) 
{
    int r = (int)(cellColor[0] * 255.0f);
    int g = (int)(cellColor[1] * 255.0f);
    int b = (int)(cellColor[2] * 255.0f);
    
    int minIndex = lookup[r][g][b];

    // stuff it all back in.

    cellColor[0] = fpal[minIndex * 3 + 0];
    cellColor[1] = fpal[minIndex * 3 + 1];
    cellColor[2] = fpal[minIndex * 3 + 2];
    cellColor[3] = (float)minIndex;
}

inline const float *
findClosest(float cellColor[4], const float *selectColor, const float *backColor)
{
    float distBlack = colorDist(cellColor, backColor);
    float distWhite = colorDist(cellColor, selectColor);

    return distBlack <= distWhite ? backColor : selectColor;
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


enum
{
	Palette_Atari2600NTSC = 0,
	Palette_BW2 = 1,
	Palette_BW4 = 2,
	Palette_RGB = 3,
	Palette_Atari2600RandomTerrain = 4,
	Palette_Rubik = 5,
	Palette_Atari2600PAL = 6,
	Palette_Atari2600SECAM = 7,
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

		case Palette_Atari2600PAL:
			pal = atari2600pal_palette;
			palSize = 128;
			break;

		case Palette_Atari2600SECAM:
			pal = atari2600secam_palette;
			palSize = 8;
			break;

		case Palette_Atari2600RandomTerrain:
			pal = atari2600randomterrain_palette;
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


		case Palette_Rubik:
			pal = rubik_palette;
			palSize = 6;
			break;
	}
}


void
ditherLine(int bidx, int y, bool finalB, int width, int height, int cellSize,
	float *curY, const float *fpal, int palSize, float bleed, int matrix,
	float *mem, bool dither, float *curError,
	const uint8_t lookup[256][256][256],
	float bestError)
{
	float	cellColor[4] = { 1, 1, 1, 0 };
	float	backColor[4];

	backColor[0] = fpal[bidx*3 + 0];
	backColor[1] = fpal[bidx*3 + 1];
	backColor[2] = fpal[bidx*3 + 2];
	backColor[3] = (float)bidx;


	*curError = 0.0f;

	for (int x0 = 0; x0 < width; x0++)
	{
		int xpix = x0%cellSize;

		int x = x0;
		float* pixel = &curY[4*x];


		// determine cell color

		if (xpix == 0)	// first pixel of cell
		{
			float	total_weight = 0.0f;

			cellColor[0] = 0.0f;
			cellColor[1] = 0.0f;
			cellColor[2] = 0.0f;

			for (int x2=0; x2<cellSize; x2++)
			{
				int x3 = x + x2;

				float* npixel = &curY[4*x3];
			
				float r = npixel[0];
				float g = npixel[1];
				float b = npixel[2];

				float weight = r*colorScales[0] + g*colorScales[1] + b*colorScales[2];
				//
				// weigh background minimally
				{
					float	dist = colorDist(npixel, backColor);
					weight *= dist;
				}

				cellColor[0] += r*weight;
				cellColor[1] += g*weight;
				cellColor[2] += b*weight;

				total_weight += weight;
			}

			if (total_weight)
			{
				cellColor[0] /= total_weight;
				cellColor[1] /= total_weight;
				cellColor[2] /= total_weight;
			}

			lookupClosestInPalette(cellColor, fpal, lookup);
		}

		// now dither
		if (dither)
		{
            
            float current[3];
            memcpy(current, pixel, 3 * sizeof(float));

			// copy cellColor or backColor into pixel
			memcpy(pixel, findClosest(pixel, cellColor, backColor), sizeof(float) * 4);

			*curError += colorDist(current, pixel);
			if (!finalB && *curError >= bestError)
				return;

			if (bleed > 0)
			{
				float quantError[3];
				
				for (int i = 0; i < 3; i++)
					quantError[i] = (current[i] - pixel[i]) * bleed;

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
	bool bleedSearch = inputs->getParInt("Bleedsearch") ? true:false;

	int matrix = inputs->getParInt("Matrix");

	bool background = inputs->getParInt("Background") ? true:false;

	unsigned int	*pal;
	int				palSize;
	getPalette(palette, pal, palSize);

	if (pal != myLastPal)
	{
		myLastPal = pal;

		for (int i=0; i<palSize; i++)
		{
			myFPal[3*i + 0] = pal[3*i + 0] / 255.0f;
			myFPal[3*i + 1] = pal[3*i + 1] / 255.0f;
			myFPal[3*i + 2] = pal[3*i + 2] / 255.0f;
		}

		setup_kdtree(myFPal, palSize);
		buildColourMap();
	}

	if (!active)
		return;

	OP_TOPInputDownloadOptions dlOptions;
	dlOptions.downloadType = OP_TOPInputDownloadType::Instant;
	dlOptions.cpuMemPixelType = OP_CPUMemPixelType::RGBA32Float;

	const OP_TOPInput	*topInput = inputs->getInputTOP(0);
	const uint8_t	*topMem = topInput ? (uint8_t *)inputs->getTOPDataInCPUMemory(topInput, &dlOptions) : nullptr;


	int textureMemoryLocation = 0;
	int width = outputFormat->width;
	int height = outputFormat->height;

	setupStorage(width, outputFormat->height, cellSize);

	// copy, will already be float format
	if (topMem)
		memcpy(myMem, topMem, width * height * 4 * sizeof(float));

	if (!background)
	{
		bool	finalB = true;

		for (int y = 0; y < outputFormat->height; y++)
		{
			float* curY = &myMem[4 * (y*width)];
			
			float	error;

			ditherLine(0, y, finalB, width, height, cellSize, curY,
				myFPal, palSize, bleed, matrix, myMem, dither, &error, myColorLookup, HUGE_VAL);

			for (int i = 0; i < 4; i++)
				myResultBK[y * 4 + i] = 0.0f;
		}
	}
	else
	{
		// exhaustive search of each background color

		// don't bleed during search unless specified.
		float lbleed = bleedSearch ? bleed : 0.0f;

		for (int y = 0; y < outputFormat->height; y++)
		{
			float* curY = &myMem[4 * (y*width)];
			
			int		bestB = 0;
			float	bestError = HUGE_VAL;

			// start with best color from previous frame
			int		startB = (int)(myResultBK[y*4 + 3]);

			for (int b=0; b<palSize; b++)
			{
				float	curError;
				bool	finalB = false;
				int		bidx = (startB + b) % palSize;

				memcpy(myMemBackup, curY, width*4 * sizeof(float));
				ditherLine(bidx, y, finalB, width, height, cellSize, myMemBackup, myFPal, palSize, lbleed, matrix, myMem, dither, &curError, myColorLookup, bestError);

				if (curError < bestError)
				{
					bestError = curError;
					bestB = bidx;
				}
			}

			// redo best color
			{
				bool	finalB = true;
				float	error;
				int		bidx = bestB;

				ditherLine(bestB, y, finalB, width, height, cellSize, curY, myFPal, palSize, bleed, matrix, myMem, dither, &error, myColorLookup, HUGE_VAL);

				float	backColor[4];
				backColor[0] = myFPal[bidx*3 + 0];
				backColor[1] = myFPal[bidx*3 + 1];
				backColor[2] = myFPal[bidx*3 + 2];
				backColor[3] = (float)bidx;

				for (int i = 0; i < 4; i++)
					myResultBK[y * 4 + i] = backColor[i];
			}

		}
	}

	{
		uint8_t* destMem = (uint8_t*)outputFormat->cpuPixelData[textureMemoryLocation];
		storeResults(width, height, myMem, destMem, cellSize);
	}

    outputFormat->newCPUPixelDataLocation = textureMemoryLocation;
    textureMemoryLocation = !textureMemoryLocation;
}

void
CPUMemoryTOP::releaseStorage()
{
	if (myResultGraph)
		delete[] myResultGraph;
	if (myResultColor)
		delete[] myResultColor;
	if (myResultBK)
		delete[] myResultBK;
	if (myMem)
		delete[] myMem;
	if (myMemBackup)
		delete[] myMemBackup;

	myResultGraph = nullptr;
	myResultColor = nullptr;
	myResultBK = nullptr;
	myMem = nullptr;
	myMemBackup = nullptr;
}

void
CPUMemoryTOP::setupStorage(int outputWidth, int outputHeight, int cellSize)
{
	outputWidth /= cellSize;
	if (outputWidth < 1)
		outputWidth = 1;

	if (myResultWidth != outputWidth || (myResultHeight != outputHeight))
	{
		releaseStorage();

		myResultWidth = outputWidth;
		myResultHeight = outputHeight;
		myResultGraph = new uint8_t[myResultWidth * myResultHeight];
		myResultColor = new uint8_t[myResultWidth * myResultHeight];

		// will be read each frame, initialize to zero
		int bkSize = (myResultHeight * 4);
		myResultBK = new float[bkSize];
		memset(myResultBK, 0, bkSize * sizeof(float));

		myMem = new float[myResultWidth * cellSize * myResultHeight * 4];
		myMemBackup = new float[myResultWidth * cellSize * 4];
	}

}

void
CPUMemoryTOP::storeResults(int outputWidth, int outputHeight, const float *srcMem, uint8_t *destMem, int cellSize)
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
			const float* pixel = &srcMem[4 * cellSize * (y*outputWidth + x)];

			// take next cellSize rgb bits for dither
			uint8_t val = 0;
			uint8_t col = 0;

			uint8_t* destPixel = &destMem[4 * cellSize * (y*outputWidth + x)];


			for (int i = 0; i < cellSize; i++, destPixel += 4, pixel +=4)
			{
				destPixel[0] = uint8_t(pixel[2] * 255.0f);
				destPixel[1] = uint8_t(pixel[1] * 255.0f);
				destPixel[2] = uint8_t(pixel[0] * 255.0f);
				destPixel[3] = uint8_t(pixel[3]);

				val <<= 1;

				// only store non-backcolor in this array
				if (destPixel[3] != bidx)
				{
					val |= 1;
					if (destPixel[3])
						col = (uint8_t)destPixel[3];

					// set alpha to solid
					destPixel[3] = 255;
				}
				else
				{
					// turn off background

					destPixel[0] = 0;
					destPixel[1] = 0;
					destPixel[2] = 0;
					destPixel[3] = 0;
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

		const char *names[8] = 
		{
			"Atari2600ntsc",
			"Bw2",
			"Bw4",
			"Rgb",
			"Atari2600randomterrain",
			"Rubik",
			"Atari2600pal",
			"Atari2600secam"
		};

		const char *labels[8] = 
		{
			"Atari 2600 NTSC",
			"B/W 2",
			"B/W 4",
			"RGB",
			"Atari 2600 Random Terrain",
			"Rubik",
			"Atari 2600 PAL",
			"Atari 2600 SECAM"
		};

		manager->appendMenu(sp, 8, names, labels);
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
		OP_NumericParameter  sp;

		sp.name = "Bleedsearch";
		sp.label = "Bleed During Search";
		sp.defaultValues[0] = 0;

		manager->appendToggle(sp);
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


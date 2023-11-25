/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "TOP_CPlusPlusBase.h"

using namespace TD;


// kd-tree structures
/* Adapted from: https://rosettacode.org/wiki/K-d_tree */
#define MAX_DIM 3

struct kd_node_t
{
    float   val[MAX_DIM];
    int     index;
    struct  kd_node_t *left, *right;
};

template<class T>
class Array2D
{
public:

	Array2D()
	{
		width = height = 0;
		mem = nullptr;
	}

	~Array2D()
	{
		setSize(0, 0);
	}

	void
	setSize(int w, int h)
	{
		if (w != width || h != height)
		{
			width = w;
			height = h;

			if (mem)
				delete [] mem;

			if (w || h)
				mem = new T[width * height];
			else
				mem = nullptr;

			zero();
		}
	}

	void
	zero()
	{
		if (mem)
			memset(mem, 0, width*height*sizeof(T));
	}

	T&
	operator()(int x, int y)
	{
		return mem[y*width + x];
	}

	T&
	operator()(int x, int y) const
	{
		return mem[y*width + x];
	}

	T*
	getData()
	{
		return mem;
	}

	int
	getWidth() const
	{
		return width;
	}

	int
	getHeight() const
	{
		return height;
	}

private:

	T*			mem;
	int			width;
	int			height;

};

class ColorizeTOP : public TOP_CPlusPlusBase
{
public:
	ColorizeTOP(const OP_NodeInfo *info, TOP_Context* context);
	virtual ~ColorizeTOP();

	virtual void		getGeneralInfo(TOP_GeneralInfo *, const OP_Inputs*, void*) override;

	virtual void		execute(TOP_Output*,
							const OP_Inputs*,
							void* reserved1) override;

	virtual int32_t		getNumInfoCHOPChans(void *reserved1) override;
	virtual void		getInfoCHOPChan(int32_t index,
								OP_InfoCHOPChan *chan, void* reserved1) override;

	virtual bool		getInfoDATSize(OP_InfoDATSize *infoSize, void *reserved1) override;
	virtual void		getInfoDATEntries(int32_t index,
									int32_t nEntries,
									OP_InfoDATEntries *entries,
									void *reserved1) override;

	virtual void		setupParameters(OP_ParameterManager *manager, void *reserved1) override;
	virtual void		pulsePressed(const char *name, void *reserved1) override;

private:

	TOP_Context*		myContext;

    void                setupStorage(int outputWidth, int outputHeight, int cellSize);
    void                storeResults(uint8_t *destPixel, int cellSize);

    Array2D<float[4]>	myMem;
    Array2D<float[4]>	myMemBackup;
    Array2D<uint8_t>	myResultGraph;
    Array2D<uint8_t>	myResultColor;
    Array2D<float[4]>	myResultBK;
    Array2D<float[3]>	myFPal;

    unsigned char *myLastPal;
    uint8_t				myColorLookup[256][256][256];
    void				buildColourMap();

    void				ditherLine(int bidx, int y, bool finalB, int width, int height, int cellSize,
							float *curY, int palSize, float bleed, int matrix,
							bool dither, float *curError, float bestError,
							bool searchForeground, int foregroundInc);

    // k-d tree data
    struct kd_node_t	kdtree[256];
    struct kd_node_t*	kdtree_root{nullptr};
    void				setup_kdtree(Array2D<float[3]>& fpal, int palSize);

};

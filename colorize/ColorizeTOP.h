/* Shared Use License: This file is owned by Derivative Inc. (Derivative) and
 * can only be used, and/or modified for use, in conjunction with 
 * Derivative's TouchDesigner software, and only if you are a licensee who has
 * accepted Derivative's TouchDesigner license or assignment agreement (which
 * also govern the use of this file).  You may share a modified version of this
 * file with another authorized licensee of Derivative's TouchDesigner software.
 * Otherwise, no redistribution or sharing of this file, with or without
 * modification, is permitted.
 */

#include "TOP_CPlusPlusBase.h"

class CPUMemoryTOP : public TOP_CPlusPlusBase
{
public:
    CPUMemoryTOP(const OP_NodeInfo *info);
    virtual ~CPUMemoryTOP();

    virtual void		getGeneralInfo(TOP_GeneralInfo *, const OP_Inputs*, void*) override;
    virtual bool		getOutputFormat(TOP_OutputFormat*, const OP_Inputs*, void*) override;


    virtual void		execute(TOP_OutputFormatSpecs*,
							const OP_Inputs*,
							TOP_Context* context,
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

	void				setupStorage(int outputWidth, int outputHeight, int cellSize);
	void				storeResults(int outputWidth, int outputHeight, float *mem, int cellSize);

	int                  myResultWidth;
	int                  myResultHeight;
	uint8_t             *myResultGraph;
	uint8_t             *myResultColor;
	float               *myResultBK;

	unsigned int		*myLastPal;

};

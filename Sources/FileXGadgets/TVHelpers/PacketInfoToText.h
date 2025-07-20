#pragma once
#include <gadgets\gadbase.h>

class PacketInfoToText :
    public CFilterGadget
{
public:
    PacketInfoToText(void);
    virtual void ShutDown();
    virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	DECLARE_RUNTIME_GADGET(PacketInfoToText);
};

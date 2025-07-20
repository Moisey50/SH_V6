#pragma once
#include <gadgets\gadbase.h>

class QuantityToText :
    public CFilterGadget
{
public:
    QuantityToText(void);
    virtual void ShutDown();
    virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	DECLARE_RUNTIME_GADGET(QuantityToText);
};

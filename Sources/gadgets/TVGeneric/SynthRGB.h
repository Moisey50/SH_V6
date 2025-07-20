#pragma once
#include <gadgets\gadbase.h>

class SynthRGB : public CFilterGadget
{
protected:
	FXStaticQueue<CVideoFrame*> m_Stack;
	FXLockObject m_Lock;
private:
	void ClearStack();
public:
	SynthRGB();
	virtual void ShutDown();
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);

  DECLARE_RUNTIME_GADGET(SynthRGB);
};

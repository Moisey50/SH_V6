#pragma once
#include <gadgets\gadbase.h>

class ArrayToFigure : public CFilterGadget
{
public:
	virtual void ShutDown();
	bool PrintProperties(FXString& text);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
private:
	ArrayToFigure();
	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	DECLARE_RUNTIME_GADGET(ArrayToFigure);
};

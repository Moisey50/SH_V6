// SquareFilter.h : Declaration of the SquareFilter

#pragma once
#include <Gadgets\gadbase.h>
#include <imageproc\fstfilter.h>

class SquareFilter : public CFilterGadget
{
    boxsize m_Box;
public:
    SquareFilter(void);
    void ShutDown();
//
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool PrintProperties(FXString& text);
	bool ScanSettings(FXString& text);

    DECLARE_RUNTIME_GADGET(SquareFilter);
};

// CustomFilter.h : Declaration of the CustomFilter

#pragma once
#include <Gadgets\gadbase.h>
#include <imageproc/customfilters.h>

class CustomFilter : public CFilterGadget
{
    CCustomFilterMatrix m_Matrix;
    int                 m_Offset;
public:
    CustomFilter(void);
    void ShutDown();
//
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool PrintProperties(FXString& text);
	bool ScanSettings(FXString& text);

    DECLARE_RUNTIME_GADGET(CustomFilter);
};

// MODIOCR.h : Declaration of the MODIOCR class

#pragma once
#include <gadgets\gadbase.h>

class MODIOCR : public CFilterGadget
{
protected:
    BOOL    m_OverwriteDPI;
    int     m_X_DPI, m_Y_DPI;
public:
    MODIOCR(void);
    void ShutDown();
//
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool PrintProperties(FXString& text);
	bool ScanSettings(FXString& text);

    DECLARE_RUNTIME_GADGET(MODIOCR);
};

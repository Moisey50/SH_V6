#pragma once
#include <gadgets\gadbase.h>

class TextFormat :
    public CFilterGadget
{
private:
    FXString m_InputFormat, m_OutputFormat;
protected:
    TextFormat(void);
public:
    virtual void ShutDown();
	virtual bool PrintProperties(FXString& text);
	virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
    virtual bool ScanSettings(FXString& text);
private:
    virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
    DECLARE_RUNTIME_GADGET(TextFormat);
};

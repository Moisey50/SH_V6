// Pseudocolor2.h : Declaration of the Pseudocolor2 class

#pragma once
#include <gadgets\gadbase.h>

class Pseudocolor2 : public CFilterGadget
{
protected:
    int m_Palette;
private:
    Pseudocolor2(void);
    void ShutDown();
public:
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool PrintProperties(FXString& text);
	bool ScanSettings(FXString& text);

	DECLARE_RUNTIME_GADGET(Pseudocolor2);
};

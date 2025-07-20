// LineProfile.h : Declaration of the LineProfile

#pragma once
#include <Gadgets\gadbase.h>

class LineProfile : public CFilterGadget
{
protected:
    CDuplexConnector*	m_pControl;
    CRect               m_LineSel;
    FXLockObject         m_TransactionLock;
public:
    LineProfile(void);
    void ShutDown();
//
	virtual int GetDuplexCount();
	virtual CDuplexConnector* GetDuplexConnector(int n);
    virtual void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

    CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool PrintProperties(FXString& text);

    DECLARE_RUNTIME_GADGET(LineProfile);
};

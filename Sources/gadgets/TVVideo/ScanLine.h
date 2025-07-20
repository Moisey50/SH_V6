// ScanLine.h: interface for the ScanLine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCANLINE_H__7B339C45_D99A_4437_B3AF_8B9DB8D5483E__INCLUDED_)
#define AFX_SCANLINE_H__7B339C45_D99A_4437_B3AF_8B9DB8D5483E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Gadgets\gadbase.h>

class ScanLine : public CFilterGadget  
{
protected:
    int m_Line, m_Row;
    CDuplexConnector*	m_pControl;
    FXLockObject         m_Lock;
public:
	ScanLine();
	virtual void ShutDown();
	virtual int GetDuplexCount();
	virtual CDuplexConnector* GetDuplexConnector(int n);
    virtual void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
    virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);    
    DECLARE_RUNTIME_GADGET(ScanLine);
};

#endif // !defined(AFX_SCANLINE_H__7B339C45_D99A_4437_B3AF_8B9DB8D5483E__INCLUDED_)

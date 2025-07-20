// TVRotate.h : Declaration of the Rotate

#pragma once
#include <Gadgets\gadbase.h>

enum WorkingMode
{
  WM_ROTATE ,
  WM_HSKEW ,
  WM_VSKEW
};


class Rotate : public CFilterGadget
{
protected:
    double m_AngleDeg;
    bool   m_RotateRgn;
    WorkingMode    m_Mode ;
    CRect  m_Rect;
    CDuplexConnector*	m_pControlPin;
public:
    Rotate(void);
    void ShutDown();
//
    int		    GetDuplexCount() { return 1; }
    CDuplexConnector* GetDuplexConnector(int n) { return (n==0)?m_pControlPin:NULL; }
	void		AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pDataFrame);
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	bool        ScanProperties(LPCTSTR text, bool& Invalidate);
	bool        PrintProperties(FXString& text);
	bool        ScanSettings(FXString& text);

    DECLARE_RUNTIME_GADGET(Rotate);
};

// TVHistogram.h : Declaration of the TVHistogram

#pragma once
#include <Gadgets\shkernel.h>
#include <math\Intf_sup.h>

class TVHistogram : public CFilterGadget
{
private:
    CDuplexConnector*	m_pControl;
    CRect             m_RectSel;
    int               m_iLastImageDepth ;
    FXLockObject      m_TransactionLock;
    int               m_iRangeBegin ;
    int               m_iRangeEnd ;
    int               m_iCutLevelx10_perc ;
    BOOL              m_bViewCut ;
    cmplx             m_RectForView[ 5 ] ;
public:
    TVHistogram(void);
    void ShutDown();
//
	virtual int GetDuplexCount();
	virtual CDuplexConnector* GetDuplexConnector(int n);
  virtual void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

  CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
  virtual bool ScanProperties( LPCTSTR text , bool& Invalidate );
  virtual bool PrintProperties( FXString& text );
  virtual bool ScanSettings( FXString& text );

  DECLARE_RUNTIME_GADGET(TVHistogram);
};

// ScanByCamera.h : Declaration of the ScanByCamera class

#pragma once
#include <gadgets\gadbase.h>
#include <video\TVFrame.h>
#include <helpers\FramesHelper.h>
class ScanByCamera : public CFilterGadget
{
protected:
	CDuplexConnector * m_pDuplexConnector ;

private:
    ScanByCamera(void);
    void ShutDown();
public:
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool PrintProperties(FXString& text);
	bool ScanSettings(FXString& text);
	int GetDuplexCount();
	CDuplexConnector* GetDuplexConnector(int n);
	virtual void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

  int m_iLineNumber ;
  int m_iImageHeight ;
  pTVFrame m_pOutputTVFrame ;
  int      m_iLastLine ;

	DECLARE_RUNTIME_GADGET(ScanByCamera);
};

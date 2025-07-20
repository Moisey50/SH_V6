// WaterFall.h : Declaration of the WaterFall class

#pragma once
#include <gadgets\gadbase.h>
#include <video\TVFrame.h>
#include <helpers\FramesHelper.h>
#include <gadgets\arrayframe.h>




class WaterFall : public CFilterGadget
{
protected:
	CDuplexConnector * m_pDuplexConnector ;

private:
    WaterFall(void);
    void ShutDown();
public:
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool PrintProperties(FXString& text);
	bool ScanSettings(FXString& text);
	int GetDuplexCount();
	CDuplexConnector* GetDuplexConnector(int n);
	virtual void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

  int m_iImageWidth ;
  int m_iImageHeight ;
  bool m_bSizeChanged ;
  pTVFrame m_pCurrentState ;
  int  m_iDecimationInterval ;
  double m_dGain_dB ;
  int    m_Law ;
  BOOL m_bAutoGain ;
  int  m_iDecimationCounter ;
  int  m_iCutLevel ;


	DECLARE_RUNTIME_GADGET(WaterFall);
};

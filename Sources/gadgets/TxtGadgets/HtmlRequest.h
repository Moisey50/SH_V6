#pragma once
#include <gadgets\gadbase.h>
#include <afxinet.h>
#include <fxfc\fxfc.h>

class HtmlRequest : public CCaptureGadget
{
protected:
	CInputConnector*	m_pInput;
	FXString			m_URL;
	FXString				m_Request;
	BOOL				m_bCmdPin;
	BOOL				m_bRun;
    double              m_FrameRate;
	CInternetSession	m_InetSession;
	CHttpConnection*	m_InetConnect;
    DWORD               m_frameID;
	HtmlRequest();
public:
	virtual void ShutDown();
	virtual void OnStart();
	virtual void OnStop();
	void OnInput(CDataFrame* pFrame);
	virtual int DoJob();
	void SendData(CDataFrame* pFrame=NULL, bool bEOS = false);
	bool ScanSettings(FXString& text);
	virtual bool ScanProperties( LPCTSTR text, bool& Invalidate) ;
	virtual bool PrintProperties(FXString& text) ;
	virtual int GetInputsCount() { return (m_pInput) ? 1 : 0; } ;
	CInputConnector* GetInputConnector(int n) { return (n == 0) ? m_pInput : NULL ; } ;
	friend void CALLBACK fnInput(CDataFrame* pFrame, void* Gadget, CConnector* pInput)
	{
		((HtmlRequest*)Gadget)->OnInput(pFrame);
	}
	DECLARE_RUNTIME_GADGET(HtmlRequest);
};

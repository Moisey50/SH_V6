#pragma once

#include <gadgets\gadbase.h>
#include <helpers\PrxyWnd.h>

class ScrollCtrl :
    public CCtrlGadget
{
protected:
    CPrxyWnd          m_Proxy;
    CSliderCtrl       m_Scroll;
    int               m_sMin, m_sMax;
    FXString          m_Format;
    bool              m_Vertical;
	CInputConnector*  m_pInput;
	FXLockObject      m_Lock;
	int				  m_CurPos;
public:
    ScrollCtrl(void);
	void ShutDown();

    void        Attach(CWnd* pWnd);
    void        Detach();
    CWnd*       GetRenderWnd() { return &m_Proxy; }
    void        GetDefaultWndSize (RECT& rc);
    void        OnStart();
	void        OnStop();

	virtual int GetInputsCount() { return (m_pInput) ? 1 : 0;}
	virtual CInputConnector* GetInputConnector(int n)   { return (n > 0) ? NULL : m_pInput; }

    bool ScanSettings(FXString& text);
    bool ScanProperties(LPCTSTR text, bool& Invalidate);
    bool PrintProperties(FXString& text);

    void OnCommand(UINT nID, int nCode, void* pParam);

protected:
	void SendCurPos();
	friend void CALLBACK ScrollCtrl_AsyncFn(CDataFrame* lpData, void* lpParam, CConnector* lpInput)
	{
		ScrollCtrl* pCtrl = (ScrollCtrl*)lpParam;
		FXAutolock al(pCtrl->m_Lock);
		pCtrl->SendCurPos();
		if (lpData)
			lpData->Release(lpData);
	}
    DECLARE_RUNTIME_GADGET(ScrollCtrl);
};

// Scan.h : Declaration of the Scan class

#pragma once
#include <Gadgets\shkernel.h>
#include <helpers\PrxyWnd.h>
#include <helpers\Acquire.h>

class Scan : public CCtrlGadget
{
private:
    CPrxyWnd          m_Proxy;
    CButton           m_Button;
	CAcquire*		  m_AcquireSource;

    Scan(void);
    void ShutDown();
public:
    void Attach(CWnd* pWnd);
    void Detach();
    CWnd*GetRenderWnd() { return &m_Proxy; }
    virtual void GetDefaultWndSize (RECT& rc) { rc.left=rc.top=0; rc.right=4*DEFAULT_GADGET_WIDTH/3; rc.bottom=DEFAULT_GADGET_HEIGHT/2; }

    virtual void OnStart();
	virtual void OnStop();

	void ShowSetupDialog(CPoint& point);

	void OnCommand(UINT nID, int nCode);
	void TwainData(LPBYTE Data, int Length);
    DECLARE_RUNTIME_GADGET(Scan);
};

// CheckBoxCtrl.h: interface for the CheckBoxCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHECKBOXCTRL_H__818D814A_63AB_48EB_A829_1BC136FFE826__INCLUDED_)
#define AFX_CHECKBOXCTRL_H__818D814A_63AB_48EB_A829_1BC136FFE826__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include <helpers\PrxyWnd.h>

class CheckBoxCtrl : public CCtrlGadget  
{
protected:
    CPrxyWnd          m_Proxy;
    CButton           m_Button;
    FXString          m_ButtonName;
    bool              m_Set;
public:
	CheckBoxCtrl();
	virtual void ShutDown();
    void        Attach(CWnd* pWnd);
    void        Detach();
    CWnd*       GetRenderWnd() { return &m_Proxy; }
    void        GetDefaultWndSize (RECT& rc) { rc.left=rc.top=0; rc.right=2*DEFAULT_GADGET_WIDTH; rc.bottom=DEFAULT_GADGET_HEIGHT/2; }

    virtual void OnStart();
	virtual void OnStop();


    bool ScanSettings(FXString& text);
    bool ScanProperties(LPCTSTR text, bool& Invalidate);
    bool PrintProperties(FXString& text);

    void OnCommand(UINT nID, int nCode);

    DECLARE_RUNTIME_GADGET(CheckBoxCtrl);
};

#endif // !defined(AFX_CHECKBOXCTRL_H__818D814A_63AB_48EB_A829_1BC136FFE826__INCLUDED_)

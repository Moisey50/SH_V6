// ControlPinTester.h: interface for the ControlPinTester class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONTROLPINTESTER_H__D5CBC34F_CBB8_4011_BD86_7CE0ED5CC88F__INCLUDED_)
#define AFX_CONTROLPINTESTER_H__D5CBC34F_CBB8_4011_BD86_7CE0ED5CC88F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include "CtrlPinTestDlg.h"

class ControlPinTester : public CCtrlGadget  
{
protected:
    CDuplexConnector*	m_pControl;
    CCtrlPinTestDlg     m_Dialog;
public:
	         ControlPinTester();
	virtual void        ShutDown();
	int                 GetDuplexCount();
	CDuplexConnector*   GetDuplexConnector(int n);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

    virtual void OnStart();
	virtual void OnStop();

    void    Attach(CWnd* pWnd);
    void    Detach();
    CWnd*   GetRenderWnd() { return &m_Dialog; }
    void GetDefaultWndSize (RECT& rc) 
    { 
        m_Dialog.GetClientRect(&rc);
    }
    bool    IsCtrlReady() 
    { 
        return (m_Dialog.GetSafeHwnd()!=NULL); 
    }
    // 
    void SendText(FXString& str);
	void OnCommand(UINT nID, int nCode);
	DECLARE_RUNTIME_GADGET(ControlPinTester);
};

#endif // !defined(AFX_CONTROLPINTESTER_H__D5CBC34F_CBB8_4011_BD86_7CE0ED5CC88F__INCLUDED_)

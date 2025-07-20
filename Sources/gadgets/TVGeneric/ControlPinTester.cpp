// ControlPinTester.cpp: implementation of the ControlPinTester class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ControlPinTester.h"
#include <gadgets\TextFrame.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IMPLEMENT_RUNTIME_GADGET_EX(ControlPinTester, CCtrlGadget, LINEAGE_DIAGNOSTIC, TVDB400_PLUGIN_NAME);

void __stdcall ControlPinTesterCallback(UINT nID, int nCode, void* cbParam, void* pParam)
{
    ((ControlPinTester*)cbParam)->OnCommand(nID, nCode);
}

ControlPinTester::ControlPinTester():
    m_Dialog()
{
    SetMonitor(SET_INPLACERENDERERMONITOR);
	m_pControl = new CDuplexConnector(this, text, text);
    SetTicksIdle(100);
}

void ControlPinTester::ShutDown()
{
    Detach();
    CCtrlGadget::ShutDown();
	delete m_pControl;
	m_pControl = NULL;
}

int ControlPinTester::GetDuplexCount()
{
    return (m_pControl)?1:0;
}

CDuplexConnector* ControlPinTester::GetDuplexConnector(int n)
{
	return ((!n) ? m_pControl : NULL);
}

void ControlPinTester::SendText(FXString& str)
{
    CTextFrame* tF=CTextFrame::Create(str);
    tF->ChangeId(NOSYNC_FRAME);
    if (!m_pControl->Put(tF))
        tF->RELEASE(tF);
}

void ControlPinTester::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
	ASSERT(pParamFrame);
    CTextFrame* TextFrame = pParamFrame->GetTextFrame(DEFAULT_LABEL);
    if (TextFrame)
    {
        FXPropertyKit pk=TextFrame->GetString();
        if (IsCtrlReady())
            m_Dialog.SetText(pk);
    }
    pParamFrame->RELEASE(pParamFrame);
}

void ControlPinTester::Attach(CWnd* pWnd)
{
    VERIFY(m_Dialog.Create(IDD_CTRLPIN_TESTDLG,NULL));
    m_Dialog.SetParent(pWnd);
    m_Dialog.Invalidate();
    m_Dialog.Init(ControlPinTesterCallback,this);
    //m_Dialog.GetDlgItem(IDC_SEND)->EnableWindow(FALSE);
	m_Dialog.GetDlgItem(IDC_SEND)->EnableWindow(TRUE);
}

void ControlPinTester::Detach()
{
	if (m_Dialog.GetSafeHwnd())
		m_Dialog.DestroyWindow();
}

void ControlPinTester::OnCommand(UINT nID, int nCode)
{
    FXString tmpS; 
    if (m_Dialog.GetDlgItem(nID))
    {
        m_Dialog.GetDlgItem(nID)->GetWindowText(tmpS.GetBufferSetLength(MAX_PATH), MAX_PATH);
        SendText(tmpS);
    }
}

void ControlPinTester::OnStart()
{
//    if (m_Dialog.GetSafeHwnd()!=NULL)
//        m_Dialog.GetDlgItem(IDC_SEND)->EnableWindow(TRUE);
    CCtrlGadget::OnStart();
}

void ControlPinTester::OnStop()
{
//    if (m_Dialog.GetSafeHwnd()!=NULL)
//        m_Dialog.GetDlgItem(IDC_SEND)->EnableWindow(FALSE);
    CCtrlGadget::OnStop();
}

// CheckBoxCtrl.cpp: implementation of the CheckBoxCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CheckBoxCtrl.h"
#include <gadgets\QuantityFrame.h>

IMPLEMENT_RUNTIME_GADGET_EX(CheckBoxCtrl, CCtrlGadget, LINEAGE_CTRL, TVDB400_PLUGIN_NAME);

void __stdcall CheckBoxCtrlCallback(UINT nID, int nCode, void* cbParam, void* pParam)
{
    ((CheckBoxCtrl*)cbParam)->OnCommand(nID, nCode);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CheckBoxCtrl::CheckBoxCtrl():
    m_Set(false)
{
    SetMonitor(SET_INPLACERENDERERMONITOR);
    m_ButtonName= "Check me!";
    m_pOutput   = new COutputConnector(logical);
    //Suspend();
    SetTicksIdle(100);
}

void CheckBoxCtrl::ShutDown()
{
    CCtrlGadget::ShutDown();
	delete m_pOutput;
}

void CheckBoxCtrl::Attach(CWnd* pWnd)
{
  Detach();

  CCtrlGadget::Create();
  CCtrlGadget::Attach(pWnd);

  CRect rc;
  pWnd->GetClientRect(rc);
  m_Proxy.Create(pWnd);
  m_Button.Create(m_ButtonName,WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, rc, &m_Proxy, 13); 
  m_Proxy.Init(&m_Button,CheckBoxCtrlCallback,this);
  m_Button.SetCheck((m_Set)?1:0);
  m_Button.EnableWindow(FALSE);
  if ((m_pStatus) && (m_pStatus->GetStatus()==CExecutionStatus::RUN))
      OnStart();
}

void CheckBoxCtrl::Detach()
{
	if (m_Button.GetSafeHwnd())
		m_Button.DestroyWindow();
	if (m_Proxy.GetSafeHwnd())
		m_Proxy.DestroyWindow();
}

void CheckBoxCtrl::OnCommand(UINT nID, int nCode)
{
    if ((nID==13) && (nCode==BN_CLICKED) && (m_pStatus) && (m_pStatus->GetStatus()==CExecutionStatus::RUN) )
    {
        m_Set=(m_Button.GetCheck()==1);
        CBooleanFrame* pDataFrame = CBooleanFrame::Create(m_Set);
        //pDataFrame->GetString().Format("%s(BN_CLICKED)",m_ButtonName);
        pDataFrame->ChangeId(0);
        pDataFrame->SetTime( GetGraphTime() * 1.e-3 );
        if ((!m_pOutput) || (!m_pOutput->Put(pDataFrame)))
            pDataFrame->Release();
    }
}

bool CheckBoxCtrl::ScanSettings(FXString& text)
{
    text.Format("template(EditBox(ButtonName))");
    return true;
}


bool CheckBoxCtrl::ScanProperties(LPCTSTR text, bool& Invalidate)
{
    CCtrlGadget::ScanProperties(text,Invalidate);
    FXPropertyKit pk(text);
    pk.GetString("ButtonName",m_ButtonName);
    pk.GetBool("Set",m_Set);
    if (m_Button.GetSafeHwnd())
        m_Button.SetWindowText(m_ButtonName);
    return true;
}

bool CheckBoxCtrl::PrintProperties(FXString& text)
{
    FXPropertyKit pk;
    CCtrlGadget::PrintProperties(text);
    pk.WriteString("ButtonName",m_ButtonName);
    pk.WriteBool("Set",m_Set);
    text+=pk;
    return true;
}

void CheckBoxCtrl::OnStart()
{
    if (m_Button.GetSafeHwnd())
    {
        m_Button.EnableWindow(TRUE);
        OnCommand(13,BN_CLICKED);
    }
    CCtrlGadget::OnStart();
}

void CheckBoxCtrl::OnStop()
{
	if (m_Button.GetSafeHwnd())
		m_Button.EnableWindow(FALSE);
    CCtrlGadget::OnStop();
}


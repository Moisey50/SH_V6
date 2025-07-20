#include "stdafx.h"
#include "ScrollGadget.h"
#include <gadgets\TextFrame.h>

#define THIS_MODULENAME "ScrollCtrl"

IMPLEMENT_RUNTIME_GADGET_EX(ScrollCtrl, CCtrlGadget, LINEAGE_CTRL, TVDB400_PLUGIN_NAME);

void __stdcall ScrollCtrlCallback(UINT nID, int nCode, void* cbParam, void* pParam)
{
    ((ScrollCtrl*)cbParam)->OnCommand(nID, nCode, pParam);
}


ScrollCtrl::ScrollCtrl(void):
        m_sMin(0), 
        m_sMax(100),
        m_Vertical(true),
		m_pInput(NULL),
		m_CurPos(0)
{
    SetMonitor(SET_INPLACERENDERERMONITOR);
    m_pOutput   = new COutputConnector(text);
    SetTicksIdle(100);
}

void ScrollCtrl::ShutDown()
{
    CCtrlGadget::ShutDown();
	FXAutolock al(m_Lock);
	if (m_pInput)
		delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
}

void        ScrollCtrl::Attach(CWnd* pWnd)
{
    Detach();

    CCtrlGadget::Create();
    CCtrlGadget::Attach(pWnd);

    CRect rc;
    pWnd->GetClientRect(rc);
    m_Proxy.Create(pWnd);
    m_Scroll.Create(WS_CHILD|WS_VISIBLE|WS_BORDER|((m_Vertical)?TBS_VERT:TBS_HORZ)|TBS_LEFT, rc, &m_Proxy, 14); 
    m_Proxy.Init(&m_Scroll,ScrollCtrlCallback,this);
    m_Scroll.SetRange(m_sMin, m_sMax);
    m_Scroll.EnableWindow(FALSE);
    if ((m_pStatus) && (m_pStatus->GetStatus()==CExecutionStatus::RUN))
      OnStart();
}

void        ScrollCtrl::Detach()
{
	if (m_Scroll.GetSafeHwnd())
		m_Scroll.DestroyWindow();
	if (m_Proxy.GetSafeHwnd())
		m_Proxy.DestroyWindow();
}

void ScrollCtrl::OnStart()
{
    if (m_Scroll.GetSafeHwnd())
    {
        m_Scroll.EnableWindow(TRUE);
    }
    CCtrlGadget::OnStart();
}

void ScrollCtrl::OnStop()
{
	if (m_Scroll.GetSafeHwnd())
    {
		m_Scroll.EnableWindow(FALSE);
    }
    CCtrlGadget::OnStop();
}

void ScrollCtrl::GetDefaultWndSize (RECT& rc) 
{ 
    rc.left=rc.top=0; 
    if (m_Vertical)
    {
        rc.right=2*DEFAULT_GADGET_WIDTH/3; rc.bottom=DEFAULT_GADGET_HEIGHT*2; 
    }
    else
    {
        rc.right=DEFAULT_GADGET_WIDTH*4; rc.bottom=2*DEFAULT_GADGET_HEIGHT/3; 
    }
}


bool ScrollCtrl::ScanSettings(FXString& text)
{
    text.Format("template(Spin(Min,%d,%d),Spin(Max,%d,%d),ComboBox(Vertical(False(%d),True(%d))),EditBox(MessageFormat),ComboBox(SynchroPin(False(%d),True(%d))))",MININT16,MAXINT16,MININT16,MAXINT16,false,true, false, true);
    return true;
}

bool ScrollCtrl::ScanProperties(LPCTSTR text, bool& Invalidate)
{
    CCtrlGadget::ScanProperties(text, Invalidate);
    FXPropertyKit pk(text);
    pk.GetInt("Min",m_sMin);
    pk.GetInt("Max",m_sMax);
    pk.GetInt("Vertical",(int&)m_Vertical);
    if (::IsWindow(m_Scroll.m_hWnd))
    {
        RECT rc;
        GetDefaultWndSize(rc);
        m_Scroll.ModifyStyle(TBS_VERT,((m_Vertical)?TBS_VERT:TBS_HORZ));
        ::SendMessage(m_hParentWnd,UM_GLIPH_CHANGESIZE,SIZE_RESTORED,MAKELONG(rc.right,rc.bottom));
    }
    pk.GetString("MessageFormat",m_Format);
    if (::IsWindow(m_Scroll.m_hWnd))
        m_Scroll.SetRange(m_sMin, m_sMax);
	BOOL bPin;
	if (pk.GetInt("SynchroPin", (int&)bPin))
	{
		if (bPin && !m_pInput)
		{
			FXAutolock al(m_Lock);
			m_pInput = new CInputConnector(transparent, ScrollCtrl_AsyncFn, this);
		}
		else if (!bPin && m_pInput)
		{
			m_pInput->Disconnect();
			FXAutolock al(m_Lock);
			delete m_pInput;
			m_pInput = NULL;
		}
	}
	if (m_CurPos < m_sMin)
		m_CurPos = m_sMin;
	if (m_CurPos > m_sMax)
		m_CurPos = m_sMax;
	Status().WriteBool(STATUS_REDRAW, true);
    return true;
}

bool ScrollCtrl::PrintProperties(FXString& text)
{
    CCtrlGadget::PrintProperties(text);
    FXPropertyKit pk;
    pk.WriteInt("Min",m_sMin);
    pk.WriteInt("Max",m_sMax);
    pk.WriteInt("Vertical",m_Vertical);
    pk.WriteString("MessageFormat",m_Format);
	BOOL bPin = (m_pInput != NULL);
	pk.WriteInt("SynchroPin", bPin);
    text+=pk;
    return true;
}

void ScrollCtrl::OnCommand(UINT nID, int nCode, void* pParam)
{
    if ((nID==14) && ((nCode==WM_VSCROLL) || (nCode==WM_HSCROLL)) && (m_pStatus) && (m_pStatus->GetStatus()==CExecutionStatus::RUN) )
    {
        pScrollCmd psCmd=(pScrollCmd)pParam;
        int pos=m_Scroll.GetPos();
        switch (psCmd->nSBCode)
        {
        case SB_LINEDOWN:
        //case SB_LINERIGHT:
            pos++; if (pos>=m_sMax) pos=m_sMax-1;
            break;
        case SB_LINEUP:
        //case SB_LINELEFT:
            pos--; if (pos<m_sMin) pos=m_sMin;
            break;
        }
        m_Scroll.SetPos(pos);
		m_CurPos = pos;
		FXAutolock al(m_Lock);
		if (!m_pInput)
			SendCurPos();
	}
}

void ScrollCtrl::SendCurPos()
{
        FXString tmpS;
        bool res=true;
        try
        {
		tmpS.Format(m_Format, m_CurPos);
        }
        catch(...)
        {
            res=false;
        }
        if (!res)
        {
            SENDERR_1("Error: Wrong format message string '%s'", m_Format);
        }
        else
        {
            CTextFrame* pDataFrame = CTextFrame::Create(tmpS);
            pDataFrame->ChangeId(0);
            pDataFrame->SetTime( GetGraphTime() * 1.e-3 );
            if ((!m_pOutput) || (!m_pOutput->Put(pDataFrame)))
                pDataFrame->Release();
        }
}
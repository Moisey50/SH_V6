// Scan.h : Implementation of the Scan class



#include "StdAfx.h"
#include "Scan.h"
#include <files\imgfiles.h>
#include <gadgets\VideoFrame.h>
#include <video\shvideo.h>

IMPLEMENT_RUNTIME_GADGET_EX(Scan, CCtrlGadget, LINEAGE_VIDEO".capture", TVDB400_PLUGIN_NAME);

void __stdcall ScanCallback(UINT nID, int nCode, void* cbParam, void* pParam)
{
    ((Scan*)cbParam)->OnCommand(nID, nCode);
}

void TWAINCallBack(LPVOID pParam, LPBYTE Data, int Length)
{
    ((Scan*)pParam)->TwainData(Data,Length);
}

Scan::Scan(void):
			m_AcquireSource(NULL)
{
    SetMonitor(SET_INPLACERENDERERMONITOR);
    m_pOutput = new COutputConnector(vframe);
    SetTicksIdle(100);
}

void Scan::ShutDown()
{
	Detach();
    CCtrlGadget::ShutDown();
	delete m_pOutput;
	m_pOutput = NULL;
}

void Scan::Attach(CWnd* pWnd)
{
	Detach();

	CCtrlGadget::Create();
	CCtrlGadget::Attach(pWnd);

	CRect rc;
	pWnd->GetClientRect(rc);
	m_Proxy.Create(pWnd);
	m_Button.Create("Scan",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, rc, &m_Proxy, 13); 
	m_Proxy.Init(&m_Button,ScanCallback,this);
	m_Button.EnableWindow(FALSE);
	m_AcquireSource = new CAcquire;
	m_AcquireSource->Create(&m_Proxy,TWAINCallBack,(LPVOID)this);

	if ((m_pStatus) && (m_pStatus->GetStatus()==CExecutionStatus::RUN))
	  OnStart();
}

void Scan::Detach()
{
	if (m_AcquireSource)
		delete m_AcquireSource; m_AcquireSource=NULL;
	if (m_Button.GetSafeHwnd())
		m_Button.DestroyWindow();
	if (m_Proxy.GetSafeHwnd())
		m_Proxy.DestroyWindow();
}

void Scan::OnStart()
{
    if (m_Button.GetSafeHwnd())
        m_Button.EnableWindow(TRUE);
    CCtrlGadget::OnStart();
}

void Scan::OnStop()
{
    if (m_Button.GetSafeHwnd())
        m_Button.EnableWindow(FALSE);
    CCtrlGadget::OnStop();
}

void Scan::OnCommand(UINT nID, int nCode)
{
	if ((nID==13) && (nCode==BN_CLICKED) && (m_pStatus) && (m_pStatus->GetStatus()==CExecutionStatus::RUN) && m_AcquireSource)
    {
		m_AcquireSource->DoIt();
	}
}

void Scan::TwainData(LPBYTE Data, int Length)
{
	TVFrame *tvframe=(TVFrame*)malloc(sizeof(TVFrame));
	memset(tvframe,0,sizeof(TVFrame));
    tvframe->lpBMIH=(LPBITMAPINFOHEADER)malloc(Length);
    memcpy(tvframe->lpBMIH,Data,Length);
    tvframe->lpData=NULL;

    if (makeYUV9(tvframe))
    {
		CVideoFrame* vf=CVideoFrame::Create(tvframe);
		vf->ChangeId(++m_FrameCounter);
		vf->SetTime(GetGraphTime() * 1.e-3 );
	    if ((!m_pOutput) || (!m_pOutput->Put(vf)))
			    vf->RELEASE(vf);
	}
	else
	{
		if (tvframe->lpBMIH)
			free(tvframe->lpBMIH);
		free(tvframe);
	}
}


void Scan::ShowSetupDialog(CPoint& point)
{
	m_AcquireSource->SelectSource();
}

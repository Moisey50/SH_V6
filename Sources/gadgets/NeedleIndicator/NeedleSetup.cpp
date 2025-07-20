// NeedleSetup.cpp : implementation file
//

#include "stdafx.h"
#include "needleindicator.h"
#include "NeedleSetup.h"
#include "NeedleDetector.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "CNeedleSetup"

/////////////////////////////////////////////////////////////////////////////
// CNeedleSetup dialog

bool NeedleDrawExFunc(HDC hdc,RECT& rc,CDIBViewBase* view, LPVOID lParam)
{
	CNeedleSetup* ld=(CNeedleSetup*)lParam;
	return ld->Draw(hdc,rc,view);
}

CNeedleSetup::CNeedleSetup(NeedleDetector* pGadget, CWnd* pParent):
CGadgetSetupDialog(pGadget, CNeedleSetup::IDD, pParent),
	m_rPen(PS_SOLID,1,RGB(255,0,0)),
	m_gPen(PS_SOLID,1,RGB(0,255,0)),
	m_FrameSize(-1,-1)
{
	FXPropertyKit pc;
	m_pGadget->PrintProperties(pc);
    pc.GetInt("ScanLinePos",m_LinePos);
	pc.GetDouble("FstPointValue",m_FstPointValue);
	pc.GetInt("FstPointX",m_FstPointX);
	pc.GetDouble("SdPointValue",m_SdPointValue);
	pc.GetInt("SdPointX",m_SdPointX);
}

void CNeedleSetup::DoDataExchange(CDataExchange* pDX)
{
	CGadgetSetupDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNeedleSetup)
	DDX_Text(pDX, IDC_EDIT_FSTPOINT_VALUE, m_FstPointValue);
	DDX_Text(pDX, IDC_EDIT_FSTPOINT_X, m_FstPointX);
	DDX_Text(pDX, IDC_EDIT_LINEPOSITION, m_LinePos);
	DDX_Text(pDX, IDC_EDIT_SDPOINT_VALUE, m_SdPointValue);
	DDX_Text(pDX, IDC_EDIT_SDPOINT_X, m_SdPointX);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNeedleSetup, CGadgetSetupDialog)
	//{{AFX_MSG_MAP(CNeedleSetup)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_EN_CHANGE(IDC_EDIT_FSTPOINT_VALUE, OnChangeEditFstpointValue)
	ON_EN_CHANGE(IDC_EDIT_FSTPOINT_X, OnChangeEditFstpointX)
	ON_EN_CHANGE(IDC_EDIT_LINEPOSITION, OnChangeEditLineposition)
	ON_EN_CHANGE(IDC_EDIT_SDPOINT_VALUE, OnChangeEditSdpointValue)
	ON_EN_CHANGE(IDC_EDIT_SDPOINT_X, OnChangeEditSdpointX)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNeedleSetup message handlers
bool CNeedleSetup::Show(CPoint point, LPCTSTR uid)
{
    FXString DlgHead;
    DlgHead.Format("%s Setup Dialog", uid);
    if (!m_hWnd)
    {
        FX_UPDATERESOURCE fur(pThisDll->m_hResource);
        if (!Create(IDD_NEEDLE_SETUP, NULL))
        {
            SENDERR_0("Failed to create Setup Dialog");
            return false;
        }
    }
    SetWindowText(DlgHead);
    SetWindowPos(NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    ShowWindow(SW_SHOWNORMAL);
	FXPropertyKit pc;
	m_pGadget->PrintProperties(pc);
    pc.GetInt("ScanLinePos",m_LinePos);
	pc.GetDouble("FstPointValue",m_FstPointValue);
	pc.GetInt("FstPointX",m_FstPointX);
	pc.GetDouble("SdPointValue",m_SdPointValue);
	pc.GetInt("SdPointX",m_SdPointX);
    UpdateData(FALSE);
    return true;
}

BOOL CNeedleSetup::OnInitDialog() 
{
	CGadgetSetupDialog::OnInitDialog();
	m_View.Create(GetDlgItem(IDC_VIEW));
	m_View.SetScale(-1);
	m_View.SetDrawExFunc(NeedleDrawExFunc,this);
	return TRUE;  
}

void CNeedleSetup::OnDestroy() 
{
	m_View.DestroyWindow();
	CGadgetSetupDialog::OnDestroy();
}

void CNeedleSetup::LoadFrame(const pTVFrame frame)
{
	if (!IsWindowVisible()) return;
	m_FrameSize.cx=frame->lpBMIH->biWidth;
	m_FrameSize.cy=frame->lpBMIH->biHeight;
	m_View.LoadFrame(frame);
}

bool CNeedleSetup::Draw(HDC hdc,RECT& rc,CDIBViewBase* view)
{
	int w=m_FrameSize.cx;
	int h=m_FrameSize.cy;

	if ((w<0) || (h<0)) 
		return true;

	CPoint sP(0,m_LinePos);
	CPoint eP(w,m_LinePos);
	view->Pic2Scr(sP);
	view->Pic2Scr(eP);
	HGDIOBJ oldPen=::SelectObject(hdc,m_rPen);
	
	::MoveToEx(hdc,sP.x,sP.y,NULL);
	::LineTo(hdc,eP.x,eP.y);

	::SelectObject(hdc,m_gPen);
	sP=CPoint(m_FstPointX,0);
	eP=CPoint(m_FstPointX,h);
	view->Pic2Scr(sP);
	view->Pic2Scr(eP);
	::MoveToEx(hdc,sP.x,sP.y,NULL);
	::LineTo(hdc,eP.x,eP.y);

	sP=CPoint(m_SdPointX,0);
	eP=CPoint(m_SdPointX,h);
	view->Pic2Scr(sP);
	view->Pic2Scr(eP);
	::MoveToEx(hdc,sP.x,sP.y,NULL);
	::LineTo(hdc,eP.x,eP.y);

	::SelectObject(hdc,oldPen); 

	return true;
}

void CNeedleSetup::OnSize(UINT nType, int cx, int cy) 
{
	CGadgetSetupDialog::OnSize(nType, cx, cy);
	if (::IsWindow(m_View.GetSafeHwnd()))
	{
		CWnd* Frame = GetDlgItem(IDC_VIEW);
		CRect rc;
		Frame->GetWindowRect(rc);
		ScreenToClient(rc);
		rc.right = cx - rc.left;
		rc.bottom = cy - rc.left;
		Frame->MoveWindow(rc);
		Frame->GetClientRect(rc);
		m_View.MoveWindow(rc);
	}
}

void CNeedleSetup::UploadParams()
{
	if (m_pGadget)
	{
        FXPropertyKit pc;
        bool Invalidate=false;
		pc.WriteInt("ScanLinePos",m_LinePos);
		pc.WriteDouble("FstPointValue",m_FstPointValue);
		pc.WriteInt("FstPointX",m_FstPointX);
		pc.WriteDouble("SdPointValue",m_SdPointValue);
		pc.WriteInt("SdPointX",m_SdPointX);
        m_pGadget->ScanProperties(pc,Invalidate);
	}
    CGadgetSetupDialog::UploadParams();
}

void CNeedleSetup::OnChangeEditFstpointValue() 
{
	UpdateData();
	UploadParams();
}

void CNeedleSetup::OnChangeEditFstpointX() 
{
	UpdateData();
	UploadParams();
}

void CNeedleSetup::OnChangeEditLineposition() 
{
	UpdateData();
	UploadParams();
}

void CNeedleSetup::OnChangeEditSdpointValue() 
{
	UpdateData();
	UploadParams();
}

void CNeedleSetup::OnChangeEditSdpointX() 
{
	UpdateData();
	UploadParams();
}

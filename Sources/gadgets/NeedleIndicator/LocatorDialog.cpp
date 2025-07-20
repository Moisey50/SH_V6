// LOcatorDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Locator.h"
#include "LocatorDialog.h"
#include "NeedleIndicator.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "LocatorDialog"

bool LocatorDrawExFunc(HDC hdc,RECT& rc,CDIBViewBase* view, LPVOID lParam)
{
	LocatorDialog* ld=(LocatorDialog*)lParam;
	return ld->DrawPoint(hdc,rc,view);
}

void LocatorDipPPEvent(int Event, void *Data, void *pParam, CDIBViewBase* wParam)
{
	if (Event==DIBVE_LBUTTONDOWN)
	{
		LocatorDialog* ld=(LocatorDialog*)pParam;
		CPoint pt=*((POINT*)Data);
		ld->OnChangePoint(pt,wParam);
	}
}

/////////////////////////////////////////////////////////////////////////////
// LocatorDialog dialog
LocatorDialog::LocatorDialog(Locator* pGadget, CWnd* pParent) :
CGadgetSetupDialog(pGadget, LocatorDialog::IDD, pParent),
m_pParent(pGadget)
{
	m_XPos = 0;
	m_YPos = 0;
}

void LocatorDialog::DoDataExchange(CDataExchange* pDX)
{
    CGadgetSetupDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(LocatorDialog)
    DDX_Text(pDX, IDC_EDIT_XPOS, m_XPos);
    DDX_Text(pDX, IDC_EDIT_YPOS, m_YPos);
    //}}AFX_DATA_MAP
    DDX_Control(pDX, IDC_ANGLE, m_Angle);
    DDX_Control(pDX, IDC_EDIT_RADIUS, m_Radius);
}


BEGIN_MESSAGE_MAP(LocatorDialog, CGadgetSetupDialog)
	//{{AFX_MSG_MAP(LocatorDialog)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_EN_CHANGE(IDC_EDIT_XPOS, OnChangeEditXpos)
	ON_EN_CHANGE(IDC_EDIT_YPOS, OnChangeEditYpos)
	//}}AFX_MSG_MAP
    ON_CBN_SELCHANGE(IDC_ANGLE, &LocatorDialog::OnCbnSelchangeAngle)
    ON_EN_KILLFOCUS(IDC_EDIT_RADIUS, &LocatorDialog::OnEnKillfocusEditRadius)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// LocatorDialog message handlers
bool LocatorDialog::Show(CPoint point, LPCTSTR uid)
{
    FXString DlgHead;
    DlgHead.Format("%s Setup Dialog", uid);
    if (!m_hWnd)
    {
        FX_UPDATERESOURCE fur(pThisDll->m_hResource);
        if (!Create(IDD_VIDEO_LOCATOR, NULL))
        {
            SENDERR_0("Failed to create Setup Dialog");
            return false;
        }
    }
    SetWindowText(DlgHead);
    SetWindowPos(NULL, point.x, point.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    ShowWindow(SW_SHOWNORMAL);
    // place initialization here if required
    // UpdateData(FALSE);
    return true;
}

BOOL LocatorDialog::OnInitDialog() 
{
	CGadgetSetupDialog::OnInitDialog();
    m_Angle.AddString("180°");
    m_Angle.AddString("360°");
	m_View.Create(GetDlgItem(IDC_VIEW));
	m_View.SetScale(-1);
	m_View.SetDrawExFunc(LocatorDrawExFunc,this);
	m_View.SetCallback(LocatorDipPPEvent,this);
    
    // Set initial values
	FXPropertyKit pc;
	m_pParent->PrintProperties(pc);
	pc.GetLong("X",m_XPos);
	pc.GetLong("Y",m_YPos);
    pc.GetInt("Angle",m_AngleInt);
    pc.GetInt("Radius",m_RadiusInt);
    m_Angle.SetCurSel(m_AngleInt);
    CString tmpS; tmpS.Format("%d",m_RadiusInt);
    m_Radius.SetWindowText(tmpS);
    UpdateData(FALSE);
    return TRUE;  
}

void LocatorDialog::OnDestroy() 
{
	m_View.DestroyWindow();
	CGadgetSetupDialog::OnDestroy();
}

void LocatorDialog::LoadFrame(const pTVFrame frame)
{
	if (!IsWindowVisible()) return;
	m_View.LoadFrame(frame);
}

bool LocatorDialog::DrawPoint(HDC hdc,RECT& rc,CDIBViewBase* view)
{
	CPoint selPnt(m_XPos,m_YPos);
	view->Pic2Scr(selPnt);
	HGDIOBJ oldPen=::SelectObject(hdc,m_rPen);
	
	::MoveToEx(hdc,selPnt.x-5,selPnt.y,NULL);
	::LineTo(hdc,selPnt.x+5,selPnt.y);

	::MoveToEx(hdc,selPnt.x,selPnt.y-5,NULL);
	::LineTo(hdc,selPnt.x,selPnt.y+5);

	::SelectObject(hdc,oldPen);
	return true;
}

void LocatorDialog::OnChangePoint(CPoint& pt,  CDIBViewBase* wParam)
{
	m_XPos=pt.x;
	m_YPos=pt.y;
	UpdateData(FALSE);
	UploadParams();
    Invalidate();
}

void LocatorDialog::UploadParams()
{
	if (m_pParent)
	{
        FXPropertyKit pc;
        bool Invalidate=false;
        pc.WriteInt("X",m_XPos);
        pc.WriteInt("Y",m_YPos);
        m_AngleInt=m_Angle.GetCurSel();
        pc.WriteInt("Angle",m_AngleInt);
        CString tmpS;
        m_Radius.GetWindowText(tmpS);
        m_RadiusInt=atoi(tmpS);
        pc.WriteInt("Radius",m_RadiusInt);
        m_pParent->ScanProperties(pc,Invalidate);
	}
    CGadgetSetupDialog::UploadParams();
}

void LocatorDialog::OnSize(UINT nType, int cx, int cy) 
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

void LocatorDialog::OnChangeEditXpos() 
{
	UpdateData(TRUE);
	UploadParams();
    Invalidate();
}

void LocatorDialog::OnChangeEditYpos() 
{
	UpdateData(TRUE);
	UploadParams();
    Invalidate();
}

void LocatorDialog::OnCbnSelchangeAngle()
{
    m_AngleInt=m_Angle.GetCurSel();
    UploadParams();
}

void LocatorDialog::OnEnKillfocusEditRadius()
{
    CString tmpS;
    m_Radius.GetWindowText(tmpS);
    m_RadiusInt=atoi(tmpS);
    if (m_RadiusInt<-1) m_RadiusInt=-1;
    tmpS.Format("%d",m_RadiusInt);
    m_Radius.SetWindowText(tmpS); 
    UploadParams();
}

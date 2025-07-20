// TVToolTip.cpp : implementation file
//

#include "stdafx.h"
#include "TVToolTip.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTVToolTip dialog


CTVToolTip::CTVToolTip(CWnd* pParent /*=NULL*/)
	: CDialog(CTVToolTip::IDD, pParent),
    m_Active(false)
{
	//{{AFX_DATA_INIT(CTVToolTip)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_Brush.CreateSolidBrush(RGB(255, 255, 236));
}


void CTVToolTip::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTVToolTip)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTVToolTip, CDialog)
	//{{AFX_MSG_MAP(CTVToolTip)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CTVToolTip::OnInitDialog() 
{
	CDialog::OnInitDialog();
    m_Timer=SetTimer(CTVToolTip::IDD,100,NULL);	
	return TRUE;  
}

void CTVToolTip::OnDestroy() 
{
    if (m_Timer) KillTimer(m_Timer);
	CDialog::OnDestroy();
}


void CTVToolTip::Activate(LPCTSTR text, CPoint& pt)
{
	CString tip;
    m_StartTime=GetTickCount();
	GetDlgItem(IDC_TIP)->GetWindowText(tip);
	
    if (tip == text) return; // do not redisplay last tooltip

	CDC* pDC = GetDC();
	CSize szText = pDC->GetTextExtent(text, (int)strlen(text));
	ReleaseDC(pDC);
	SetWindowPos(NULL, pt.x - 1, pt.y - 2 * szText.cy - 1, szText.cx + 2, szText.cy + 2, SWP_NOZORDER);
	GetDlgItem(IDC_TIP)->SetWindowPos(NULL, 2, 2, szText.cx, szText.cy, SWP_NOZORDER);
	GetDlgItem(IDC_TIP)->SetWindowText(text);
	ShowWindow(SW_SHOW);
    m_Active=true;
	if (GetParent())
		GetParent()->SetFocus();
}

void CTVToolTip::Pop()
{
	if (::IsWindow(GetSafeHwnd()))
		ShowWindow(SW_HIDE);
    m_Active=false;
}

/////////////////////////////////////////////////////////////////////////////
// CTVToolTip message handlers


HBRUSH CTVToolTip::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);	
	pDC->SetBkMode(TRANSPARENT);
	hbr = m_Brush;
	return hbr;
}

void CTVToolTip::OnTimer(UINT_PTR nIDEvent) 
{
    if ((nIDEvent==CTVToolTip::IDD) && (m_Active))
    {
        if ((GetTickCount()-m_StartTime)>TIMETOSHOWTOOLTIP)
        {
            Pop();
        }
    }
	CDialog::OnTimer(nIDEvent);
}


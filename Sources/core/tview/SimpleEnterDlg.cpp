// SimpleEnterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SimpleEnterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSimpleEnterDlg dialog


CSimpleEnterDlg::CSimpleEnterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSimpleEnterDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSimpleEnterDlg)
	m_Enter = _T("");
	m_Title = _T("Rename");
	//}}AFX_DATA_INIT
}


void CSimpleEnterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSimpleEnterDlg)
	DDX_Text(pDX, IDC_ENTER, m_Enter);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSimpleEnterDlg, CDialog)
	//{{AFX_MSG_MAP(CSimpleEnterDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSimpleEnterDlg message handlers

BOOL CSimpleEnterDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CRect rc;
	GetWindowRect(rc);
	MoveWindow(m_rc.left, m_rc.top, rc.Width(), rc.Height());
	SetWindowText(m_Title);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

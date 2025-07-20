// RightButtonDlg.cpp : implementation file
//

#include "stdafx.h"
#include "imcntl.h"
#include "RightButtonDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRightButtonDlg dialog


CRightButtonDlg::CRightButtonDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRightButtonDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRightButtonDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRightButtonDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRightButtonDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRightButtonDlg, CDialog)
	//{{AFX_MSG_MAP(CRightButtonDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRightButtonDlg message handlers

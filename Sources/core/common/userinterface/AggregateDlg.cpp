// AggregateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AggregateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAggregateDlg dialog


CAggregateDlg::CAggregateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAggregateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAggregateDlg)
	m_BlockName = _T("");
	m_bAddToLibrary = FALSE;
	//}}AFX_DATA_INIT
}


void CAggregateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAggregateDlg)
	DDX_Text(pDX, IDC_BLOCK_NAME, m_BlockName);
	DDX_Check(pDX, IDC_ADD, m_bAddToLibrary);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAggregateDlg, CDialog)
	//{{AFX_MSG_MAP(CAggregateDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAggregateDlg message handlers

// FImProcParamDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ImageCNTL.h"
#include "FImProcParamDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFImProcParamDialog dialog


CFImProcParamDialog::CFImProcParamDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CFImProcParamDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFImProcParamDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CFImProcParamDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFImProcParamDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFImProcParamDialog, CDialog)
	//{{AFX_MSG_MAP(CFImProcParamDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFImProcParamDialog message handlers

//  $File : DispDlg.cpp : implementation file
//  (C) Copyright HomeBuilt Group and File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)

#include "stdafx.h"
#include "tuner.h"
#include "DispDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDispDlg dialog


CDispDlg::CDispDlg( UINT nIDTemplate, CWnd* pParentWnd)
	: CDialog (nIDTemplate, pParentWnd),
    m_Template(nIDTemplate)
{
	//{{AFX_DATA_INIT(CDispDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDispDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog ::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDispDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDispDlg, CDialog )
	//{{AFX_MSG_MAP(CDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDispDlg message handlers

BOOL CDispDlg::Create(CWnd* pParentWnd) 
{
	return CDialog ::Create(m_Template, pParentWnd);
}

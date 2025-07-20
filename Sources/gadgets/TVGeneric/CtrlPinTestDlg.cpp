// CtrlPinTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "tvgeneric.h"
#include "CtrlPinTestDlg.h"
#include "ControlPinTester.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCtrlPinTestDlg dialog


CCtrlPinTestDlg::CCtrlPinTestDlg(CWnd* pParent)
    : CDialog(CCtrlPinTestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCtrlPinTestDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCtrlPinTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCtrlPinTestDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCtrlPinTestDlg, CDialog)
	//{{AFX_MSG_MAP(CCtrlPinTestDlg)
	ON_BN_CLICKED(IDC_SEND, OnSend)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCtrlPinTestDlg message handlers

void CCtrlPinTestDlg::SetText(LPCTSTR str)
{
    CString tmpS(str);
    tmpS.Replace(";",";\r\n");
    GetDlgItem(IDC_ANSWER)->SetWindowText(tmpS);
}

void CCtrlPinTestDlg::OnSend() 
{
    if (m_Callback)
        m_Callback(IDC_EDIT_CMD, BN_CLICKED, m_UserParam,0);
}



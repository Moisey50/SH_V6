// ConnectToDlg.cpp : implementation file
//

#include "stdafx.h"
#include "tvdb400.h"
#include "ConnectToDlg.h"


// CConnectToDlg dialog

IMPLEMENT_DYNAMIC(CConnectToDlg, CDialog)

CConnectToDlg::CConnectToDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConnectToDlg::IDD, pParent)
	, m_Host(_T(""))
	, m_Port(0)
{

}

CConnectToDlg::~CConnectToDlg()
{
}

void CConnectToDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_HOST, m_Host);
	DDX_Text(pDX, IDC_PORT, m_Port);
}


BEGIN_MESSAGE_MAP(CConnectToDlg, CDialog)
END_MESSAGE_MAP()


// CConnectToDlg message handlers

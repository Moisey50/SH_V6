// LogPP.cpp : implementation file
//

#include "stdafx.h"
#include "tvdb400.h"
#include "LogPP.h"
#include <helpers\BrowseFolder.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

LPCTSTR _LogLevelNames[] =
{
    "MSG_INFO_LEVEL",
    "MSG_INFO_LEVEL+1",
	"MSG_DEBUG_LEVEL",
	"MSG_DEBUG_LEVEL+1",
    "MSG_WARNING_LEVEL",
    "MSG_WARNING_LEVEL+1",
    "MSG_ERROR_LEVEL",
    "MSG_ERROR_LEVEL+1",
    "MSG_CRITICAL_LEVEL",
    "MSG_SYSTEM_LEVEL"
};

/////////////////////////////////////////////////////////////////////////////
// CLogPP property page

IMPLEMENT_DYNCREATE(CLogPP, CPropertyPage)

CLogPP::CLogPP() : CPropertyPage(CLogPP::IDD)
{
    m_SetLogLevel=1;
    m_WriteFile=false;
	//{{AFX_DATA_INIT(CLogPP)
	//}}AFX_DATA_INIT
}

CLogPP::~CLogPP()
{
}

void CLogPP::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLogPP)
	DDX_Control(pDX, IDC_LOGLEVEL, m_LogLevel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLogPP, CPropertyPage)
	//{{AFX_MSG_MAP(CLogPP)
	ON_WM_CREATE()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK_WRITEFILE, OnCheckWritefile)
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BROWSE, &CLogPP::OnBnClickedBrowse)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogPP message handlers

int CLogPP::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPropertyPage::OnCreate(lpCreateStruct) == -1)
		return -1;
	ShowWindow(SW_SHOW);
	return 0;
}

BOOL CLogPP::OnSetActive() 
{
    m_LogLevel.SetRange(MSG_INFO_LEVEL, MSG_SYSTEM_LEVEL,TRUE);
    m_LogLevel.SetPos(m_SetLogLevel);
    GetDlgItem(IDC_LOGLEVEL_COMMENT)->SetWindowText(_LogLevelNames[m_SetLogLevel-MSG_INFO_LEVEL]);
    SetWriteFile(m_WriteFile);
    SetLogFileName(m_FileName);
	return CPropertyPage::OnSetActive();
}

void CLogPP::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    if (m_LogLevel.m_hWnd==pScrollBar->m_hWnd)
    {
       m_SetLogLevel=m_LogLevel.GetPos();
       GetDlgItem(IDC_LOGLEVEL_COMMENT)->SetWindowText(_LogLevelNames[m_SetLogLevel-MSG_INFO_LEVEL]);
    }
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CLogPP::SetWriteFile(bool set)
{
    m_WriteFile=set;
    if ((GetSafeHwnd()) && (GetDlgItem(IDC_CHECK_WRITEFILE)))
    {
        GetDlgItem(IDC_CHECK_WRITEFILE)->SendMessage(BM_SETCHECK,(m_WriteFile)?BST_CHECKED:BST_UNCHECKED);
        EnableFileNamesEdit(m_WriteFile);
    }
}

void CLogPP::OnCheckWritefile() 
{
   m_WriteFile = (GetDlgItem(IDC_CHECK_WRITEFILE)->SendMessage(BM_GETCHECK)==BST_CHECKED);
   EnableFileNamesEdit(m_WriteFile);
}

void CLogPP::SetLogFileName(LPCTSTR name)
{
    m_FileName = name;
    if ((GetSafeHwnd()) && (GetDlgItem(IDC_CHECK_WRITEFILE)))
    {
        GetDlgItem(IDC_EDIT_FILENAME)->SetWindowText(m_FileName);
    }
}


void CLogPP::EnableFileNamesEdit(bool enable)
{
    if (GetSafeHwnd())
    {
        GetDlgItem(IDC_EDIT_FILENAME)->EnableWindow(enable);
        GetDlgItem(IDC_BROWSE)->EnableWindow(enable);
    }
}

void CLogPP::OnOK() 
{
    GetDlgItem(IDC_EDIT_FILENAME)->GetWindowText(m_FileName.GetBufferSetLength(MAX_PATH),MAX_PATH);
	CPropertyPage::OnOK();
}

void CLogPP::OnBnClickedBrowse()
{
    CBrowseFolder bf(this);
    bf.m_StartFolder=FxExtractPath(m_FileName);//FxGetAppPath();
    bf.m_Title=_T("Select folder for log file");
    if (bf.DoModal()==IDOK)
    {
        FXString newPath=FXString(bf.GetPath())+"\\SHStudio.log";
        SetLogFileName(newPath);
    }
}


// totalrenameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "totalrename.h"
#include "totalrenameDlg.h"
#include <shlobj.h>
#include "FileScanner.h"
#include "FilenameToLowercase.h"
#include "SetROFlag.h"
#include "CClearRIOFlag.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////

INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData) 
{
    switch(uMsg) 
    {
    case BFFM_INITIALIZED: 
        {
            FXString apppath=FxGetAppPath();
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)(LPCTSTR)apppath);
            break;
        }
    case BFFM_SELCHANGED: 
        {
            TCHAR szDir[MAX_PATH];
            if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir))
                SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
            break;
        }
    }
    return 0;
}


// CtotalrenameDlg dialog

CtotalrenameDlg::CtotalrenameDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CtotalrenameDlg::IDD, pParent)
    , m_FilePath(_T(""))
    , m_RenameMode(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CtotalrenameDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_PATH, m_FilePath);
    DDX_Radio(pDX, IDC_RENAMEMODE, m_RenameMode);
}

BEGIN_MESSAGE_MAP(CtotalrenameDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_START, &CtotalrenameDlg::OnBnClickedStart)
    ON_BN_CLICKED(IDC_SELECT_PATH, &CtotalrenameDlg::OnBnClickedSelectPath)
END_MESSAGE_MAP()


// CtotalrenameDlg message handlers

BOOL CtotalrenameDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	FXString apppath=FxGetAppPath();
    SetRootPath(apppath);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CtotalrenameDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CtotalrenameDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CtotalrenameDlg::OnBnClickedStart()
{
    CWaitCursor wc;
    UpdateData(TRUE);
    FXString path=m_FilePath;
    path.TrimRight(_T('\\'));
    m_FilePath=path;
    UpdateData(FALSE);
    if (!FxIsDirectory(path))
    {
        FxMessageBox("Error: Path is not directory!",MB_ICONSTOP);
        return;
    }
    CFileScanner *fs=NULL;

    switch (m_RenameMode)
    {
    case 0:
        fs=new CFilenameToLowercase;
        break;
    case 1:
        fs=new CSetROFlag;
        break;
    case 2:
        fs=new CClearRIOFlag;
        break;
    }

    int processed=fs->Scan(path);
    delete fs;

    FXString mes;
    mes.Format(_T("Processed %d files"), processed);
    FxMessageBox(mes);
}

void CtotalrenameDlg::OnBnClickedSelectPath()
{
    BROWSEINFO bi;
    TCHAR szDir[MAX_PATH];
    LPITEMIDLIST pidl;

    CoInitialize(NULL);
    ZeroMemory(&bi,sizeof(bi));

    bi.hwndOwner=this->m_hWnd;
    bi.pidlRoot=NULL;
    bi.lpszTitle=_T("Select directory to process");
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_USENEWUI;
    bi.lpfn = BrowseCallbackProc;
    pidl = SHBrowseForFolder(&bi);
    if (pidl)
    {
        if (SHGetPathFromIDList(pidl,szDir))
            SetRootPath(szDir);
    }
    CoTaskMemFree(pidl);
    CoUninitialize();
}

bool CtotalrenameDlg::SetRootPath(LPCTSTR path)
{
    if (!FxIsDirectory(path)) return FALSE;
    m_FilePath=path;
    UpdateData(FALSE);
    return true;
}

// FLWPlayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gadgets\stdsetup.h"
#include "FLWPlayer.h"
#include "FLWPlayerDlg.h"
#include "AboutDlg.h"
#include <gadgets\textframe.h>
#include <gadgets\videoframe.h>
#include <files\futils.h>
#include <gadgets\tvinspect.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define REGISTRY_SANDBOX HKEY_LOCAL_MACHINE
#define REGISTRY_SANDBOX HKEY_CURRENT_USER

CFLWPlayerDlg* thisDlg=NULL;

LPCTSTR g_wszAppID = _T("FileX.FLWPlayer");
LPCTSTR g_wszProgID = _T("FileX.FLWPlayerID");


bool WriteRegString ( HKEY hkeyParent, LPCTSTR szSubkey, LPCTSTR szValueName, LPCTSTR szData)
{
    HKEY hKey;
    LONG lRet;

    lRet = RegCreateKeyEx (hkeyParent, szSubkey, 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, NULL);

    if ( ERROR_SUCCESS != lRet )
        return false;
    
    lRet=RegSetValueEx(hKey,szValueName,0,REG_SZ,(LPBYTE)szData,(DWORD)strlen(szData));
    
    return ERROR_SUCCESS == lRet;
}

typedef struct tagRegEntry
{
    LPCTSTR sKey; 
    LPCTSTR szValue;
    LPCTSTR szData;
}RegEntry;

bool RegisterAsHandler()
{
    CString sIconPath, sCommandLine, sProgIDKey;
    TCHAR szModulePath[MAX_PATH] = {0};
    GetModuleFileName ( NULL, szModulePath, MAX_PATH );
    sIconPath.Format ( _T("\"%s\",-%d"), szModulePath, IDR_MAINFRAME);
    sCommandLine.Format ( _T("\"%s\" \"%%1\""), szModulePath );
    sProgIDKey.Format ( _T("software\\classes\\%s"), g_wszProgID );
    CString DefaultIcon=sProgIDKey + _T("\\DefaultIcon");
    CString CurVer=sProgIDKey + _T("\\CurVer");
    CString SheelCommand=sProgIDKey + _T("\\shell\\open\\command");
    RegEntry aEntries[] =
    {
        { _T("software\\classes\\.flw"), NULL, g_wszProgID },
        { _T("software\\classes\\.flw\\OpenWithProgIDs"), g_wszProgID, _T("") },
        { sProgIDKey, _T("FriendlyTypeName"), _T("FLW video player") },
        { sProgIDKey, _T("AppUserModelID"), g_wszAppID },
        { DefaultIcon, NULL, sIconPath },
        { CurVer, NULL, g_wszProgID },
        { SheelCommand, NULL, sCommandLine }
    }; 
    for ( int i = 0; i <sizeof(aEntries)/sizeof(RegEntry); i++ )
    {
        if ( !WriteRegString ( REGISTRY_SANDBOX, aEntries[i].sKey, aEntries[i].szValue, aEntries[i].szData) )
            return false;
    }
    return true;
}

void __stdcall PrintMsg(int msgLevel, LPCTSTR src, int msgId, LPCTSTR msgText)
{
    CString mes;
    mes.Format("+++ %d %s %d %s\n",msgLevel, src, msgId, msgText);
	if (thisDlg)
		thisDlg->PrintMessage(mes);
	else
	{
		TRACE(mes);
	}
}

DlgItem DI[]=
{
    {IDC_OPEN,BOTTOM_MOVE,{0,0,0,0},(DWORD)-1},
    {IDC_RUN,BOTTOM_MOVE,{0,0,0,0},(DWORD)-1},
    {IDC_STOP,BOTTOM_MOVE,{0,0,0,0},(DWORD)-1},
    {IDC_SETUP,BOTTOM_MOVE,{0,0,0,0},(DWORD)-1},
    {IDOK,BOTTOM_MOVE|RIGHT_MOVE,{0,0,0,0},(DWORD)-1},
    {IDC_VIDEOPOS,BOTTOM_MOVE|RIGHT_ALIGN,{0,0,0,0},(DWORD)-1},
    {IDC_FRRATE,BOTTOM_MOVE,{0,0,0,0},(DWORD)-1},
    {IDC_VIEWFRAME,BOTTOM_ALIGN|RIGHT_ALIGN,{0,0,0,0},(DWORD)-1},
	{IDC_MESSAGE,BOTTOM_MOVE,{0,0,0,0},(DWORD)-1},
	{IDC_VIEW_GRAPH,BOTTOM_MOVE,{0,0,0,0},(DWORD)-1},
};

void CALLBACK CFLWPlayerDlg_Callback(CDataFrame* lpData, void* lpParam)
{
    ((CFLWPlayerDlg*)lpParam)->OnAsyncTransaction(lpData);
}

BOOL CALLBACK CFLWPlayerDlg_OutputCallback(CDataFrame*& lpData, FXString& idPin, void* lpParam)
{
	if (idPin == "FLWCaptureGadget1>>0" || idPin == "FLWCaptureGadget1<>0")
	{
		((CFLWPlayerDlg*)lpParam)->OnAsyncTransaction(lpData);
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CFLWPlayerDlg dialog

CFLWPlayerDlg::CFLWPlayerDlg(CWnd* pParent /*=NULL*/)
	: CResizebleDialog(CFLWPlayerDlg::IDD, pParent),
	m_PrintMessage(false),
//	m_pDebugWnd(NULL),
    m_RangeChangeed(false)
{
	//{{AFX_DATA_INIT(CFLWPlayerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_hOpenFile = AfxGetApp()->LoadIcon(IDC_OPEN);
    m_hRun  = AfxGetApp()->LoadIcon(IDC_RUN);
    m_hStop =  AfxGetApp()->LoadIcon(IDC_STOP);
    m_hSetup =  AfxGetApp()->LoadIcon(IDC_SETUP);
	m_hViewGraph = AfxGetApp()->LoadIcon(IDC_VIEW_GRAPH);
}

void CFLWPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizebleDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFLWPlayerDlg)
	DDX_Control(pDX, IDC_VIDEOPOS, m_VideoPos);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFLWPlayerDlg, CResizebleDialog)
	//{{AFX_MSG_MAP(CFLWPlayerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_RUN, OnRun)
	ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_BN_CLICKED(IDC_SETUP, OnSetup)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_OPEN, OnOpen)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_VIEW_GRAPH, &CFLWPlayerDlg::OnViewGraph)
	ON_WM_PARENTNOTIFY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFLWPlayerDlg message handlers

BOOL CFLWPlayerDlg::OnInitDialog()
{
	CResizebleDialog::OnInitDialog(DI, sizeof(DI)/sizeof(DlgItem));
    thisDlg=this;
    RegisterAsHandler();
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
    GetDlgItem(IDC_RUN)-> SendMessage(BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_hRun);
    GetDlgItem(IDC_OPEN)->SendMessage(BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_hOpenFile);
    GetDlgItem(IDC_STOP)->SendMessage(BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_hStop);
    GetDlgItem(IDC_SETUP)->SendMessage(BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_hSetup);
	GetDlgItem(IDC_VIEW_GRAPH)->SendMessage(BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_hViewGraph);

	m_pBuilder = Tvdb400_CreateBuilder();
    FxInitMsgQueues(PrintMsg);
	m_PluginLoader=m_pBuilder->GetPluginLoader();
    m_PluginLoader->RegisterPlugins(m_pBuilder);

	//There are 2 options to load the graph as follow:
	//Option #1 - build graph from the '.tvg' file (e.g: @".\Resources\flwplay.tvg")
    CString tvgFileName="flwplay.tvg";
	

	//Option #2 - build graph from the script (e.g: flwplay.bin)
	// load tvg file from resource
    HRSRC hSrc=FindResource(NULL,MAKEINTRESOURCE(IDR_FLWPLAY),"TVGFILES");

    HGLOBAL hFLW=LoadResource(NULL,hSrc); 
    if (m_pBuilder->Load(NULL,(char*)hFLW)==MSG_ERROR_LEVEL) 
    {
        AfxMessageBox("Error find resource file FLWPLAY");
        EndDialog((DWORD)-1);
        return 1;
    }
    DeleteObject(hFLW);

    CGadget* gdgt=m_pBuilder->GetGadget("FLWCaptureGadget1");
    if (gdgt)
    {
        bool Invalidate=false;
        FXPropertyKit prp; prp.WriteString("FileName",m_FLWName);
        gdgt->ScanProperties(prp,Invalidate);
    }
    m_pBuilder->ConnectRendererAndMonitor(CString("VideoRenderGadget1"),GetDlgItem(IDC_VIEWFRAME),"Float window 1",m_RenderGadget);

	m_pBuilder->SetOutputCallback("FLWCaptureGadget1<>0", CFLWPlayerDlg_OutputCallback, this);
    
    m_pBuilder->SetOutputCallback("FLWCaptureGadget1>>0", CFLWPlayerDlg_OutputCallback, this);
    
    EnableFitting();
    FitDialog();
    ShowWindow(SW_SHOW);
    if (m_FLWName.GetLength()!=0)
    {
        OnRun();
    }
    SetTimer(IDD_FLWPLAYER_DIALOG,100,NULL);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFLWPlayerDlg::OnDestroy() 
{
/*	if (m_pDebugWnd && ::IsWindow(m_pDebugWnd->GetSafeHwnd()))
		m_pDebugWnd->DestroyWindow();
	m_pDebugWnd = NULL;*/

	m_pBuilder->SetOutputCallback("FLWCaptureGadget1<>0", NULL, NULL);
	m_pBuilder->SetOutputCallback("FLWCaptureGadget1>>0", NULL, NULL);

	OnStop();
    Sleep(500);
    m_pBuilder->Release(); m_pBuilder = NULL;
    FxExitMsgQueues();
	CResizebleDialog::OnDestroy();
}


void CFLWPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CResizebleDialog::OnSysCommand(nID, lParam);
	}
}

void CFLWPlayerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		CResizebleDialog::OnPaint();
	}
}

HCURSOR CFLWPlayerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CFLWPlayerDlg::OnRun() 
{
    CString cmd("getmax");
    CTextFrame* tf=CTextFrame::Create(cmd);
	if (!m_pBuilder->SendDataFrame(tf, "FLWCaptureGadget1<>0"))
		tf->Release(tf);
    m_LastFrameTime=-1;
    m_pBuilder->Start();
}

void CFLWPlayerDlg::OnStop() 
{
	m_pBuilder->Stop();		
}

void CFLWPlayerDlg::OnSetup() 
{
  Tvdb400_RunSetupDialog(m_pBuilder);
  if (!m_pBuilder->Save(m_TVGFileName))
	  AfxMessageBox("Failed to save graph!");
}


void CFLWPlayerDlg::OnOpen() 
{
	CFileDialog fd(TRUE, "flw", "", OFN_FILEMUSTEXIST, "FLW files (*.flw)|*.flw|all files (*.*)|*.*||", this);
	if (fd.DoModal() == IDOK)
	{
        FXPropertyKit pk;
        pk.WriteString("FileName",fd.GetPathName());
        bool Invalidate=false;
        m_pBuilder->GetGadget("FLWCaptureGadget1")->ScanProperties(pk,Invalidate);
	}
}

void CFLWPlayerDlg::SetFileName(LPCTSTR name)
{
	m_FLWName = name;
}

void CFLWPlayerDlg::OnSize(UINT nType, int cx, int cy) 
{
	CResizebleDialog::OnSize(nType, cx, cy);
    if (m_Updated)
    {
        CWnd* wnd=m_RenderGadget->GetRenderWnd();
        if (wnd)
        {
            CRect rc;
            GetDlgItem(IDC_VIEWFRAME)->GetClientRect(rc);
            wnd->MoveWindow(rc);
        }
    }
}

void CFLWPlayerDlg::OnAsyncTransaction(CDataFrame* pParamFrame)
{
    CVideoFrame*vf=pParamFrame->GetVideoFrame(DEFAULT_LABEL);
    if (vf)
    {
        TRACE("Got video frame\n");
        m_SliderPos=vf->GetId();
        if (m_LastFrameTime==-1)
            m_LastFrameTime=GetHRTickCount();
        else
        {
            m_FrameRate.Format("%.2f",1000.0/(GetHRTickCount()-m_LastFrameTime));
            m_LastFrameTime=GetHRTickCount();
        }
    }
    else
    {
        CTextFrame* tf=pParamFrame->GetTextFrame(DEFAULT_LABEL);
        if (tf)
        {
            if (tf->GetString().CompareNoCase("OK")==0) 
            {
                pParamFrame->Release(pParamFrame);
                return;
            }

            m_NewRange=atoi(tf->GetString());
            m_RangeChangeed=true;
        }
    }
	pParamFrame->Release(pParamFrame);
}

void CFLWPlayerDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CResizebleDialog::OnHScroll(nSBCode, nPos, pScrollBar);
    if (pScrollBar->m_hWnd==m_VideoPos.m_hWnd)
    {
        TRACE("new pos = %d\n",m_VideoPos.GetPos( ));
        CString cmd; cmd.Format("seek %d",m_VideoPos.GetPos( ));
        CTextFrame* tf=CTextFrame::Create(cmd);
		if (!m_pBuilder->SendDataFrame(tf, "FLWCaptureGadget1<>0"))
			tf->Release(tf);
    }
}

void CFLWPlayerDlg::OnTimer(UINT_PTR nIDEvent) 
{
	CResizebleDialog::OnTimer(nIDEvent);
    m_VideoPos.SetPos(m_SliderPos);    
	if (m_PrintMessage)
	{
		GetDlgItem(IDC_MESSAGE)->SetWindowText(m_Message);
		m_PrintMessage=false;
	}
    if (m_RangeChangeed)
    {
        m_VideoPos.SetRange(0,m_NewRange);
    }
    GetDlgItem(IDC_FRRATE)->SetWindowText(m_FrameRate);
}

void	CFLWPlayerDlg::PrintMessage(LPCTSTR message)
{
	m_Message=message;
	m_PrintMessage=true;
}

void CFLWPlayerDlg::OnViewGraph()
{
	if (m_pBuilder)
	{
//		CDebugWnd* pWnd = new CDebugWnd(m_pBuilder);
//		pWnd->Create(this);
//		pWnd->ShowWindow(SW_SHOW);
        m_RunBeforeInspect=(m_pBuilder->IsRuning()!=FALSE);
		StartInspect(m_pBuilder, this);
		FinishInspect();
	}
}

void CFLWPlayerDlg::OnParentNotify(UINT message, LPARAM lParam)
{
	if ((message == WM_DESTROY) && ((HWND)lParam == GetSafeHwnd())) // this must be a message from debug window
	{
		ASSERT(m_pBuilder);
		m_pBuilder->ConnectRendererAndMonitor(CString("VideoRenderGadget1"),GetDlgItem(IDC_VIEWFRAME),"Float window 1",m_RenderGadget);
        if (m_RunBeforeInspect)
            m_pBuilder->Start();
	}
	else
		CResizebleDialog::OnParentNotify(message, lParam);
}

// ImCNTLDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ImCNTL.h"
#include "ImCNTLDlg.h"
#include "DlgProxy.h"
#include "helpers\Registry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HWND hw;
extern int iExist;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImCNTLDlg dialog

IMPLEMENT_DYNAMIC(CImCNTLDlg, CDialog);

CImCNTLDlg::CImCNTLDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImCNTLDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImCNTLDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pAutoProxy = NULL;
  /*
  m_Version = _T("Version 1.0");
  */
  m_Version = _T("Ver 6.0.0 - (5.3.3Fix)"); //01.06.10 Moisey
  m_Version += 
#ifdef _DEBUG
    "DEB "  ;
#else
    "REL "  ;
#endif

	m_Version += __DATE__ ;
	m_Version += " " ;
	m_Version += __TIME__ ;

  m_ViewFrame = NULL ;
  
}

CImCNTLDlg::~CImCNTLDlg()
{
	// If there is an automation proxy for this dialog, set
	//  its back pointer to this dialog to NULL, so it knows
	//  the dialog has been deleted.
	if (m_pAutoProxy != NULL)
		m_pAutoProxy->m_pDialog = NULL;
	if(!IApp()->m_iViewExist)
	 delete m_ViewFrame;
}

void CImCNTLDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	hw=GetSafeHwnd();
	//{{AFX_DATA_MAP(CImCNTLDlg)
	DDX_Text(pDX, IDC_VERSION, m_Version);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CImCNTLDlg, CDialog)
	//{{AFX_MSG_MAP(CImCNTLDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_MY_MSG,OnRecMessage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImCNTLDlg message handlers

BOOL CImCNTLDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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
	ShowWindow( SW_MINIMIZE ) ;

	m_ViewFrame = new CImageView ;
  m_ViewFrame->LoadFrame( IDR_MAINFRAME ) ;
  HICON hViewIcon = AfxGetApp()->LoadIcon(IDI_RENDER);
  m_ViewFrame->SetIcon(hViewIcon, TRUE);			// Set big icon
  m_ViewFrame->SetIcon(hViewIcon, FALSE);		// Set small icon

  m_ViewFrame->ShowWindow( SW_SHOW ) ;

  m_ViewFrame->m_pDialog = this ;

  m_ViewFrame->SelectView() ;
#define BAD_PIXELS_FIND_EVENT   11
#define STOP_GRAB_EVENT         12
#define GRAB_EVENT_OUT          13
#define MEASURE_BLOB            14
#define GET_BLOB_PARAM          15


  //m_ViewFrame->SetTimer( BAD_PIXELS_FIND_EVENT , 2000 , NULL ) ;
	m_ViewFrame-> SetTimer( STOP_GRAB_EVENT , 5000 , NULL ) ;
// 	m_ViewFrame-> SetTimer(GRAB_EVENT_OUT,200,NULL);
//	m_ViewFrame-> SetTimer( MEASURE_BLOB , 400 , NULL ) ;
//	m_ViewFrame-> SetTimer(GET_BLOB_PARAM ,600,NULL);

  //m_pAutoProxy->m_pDialog = this ;
  //m_pAutoProxy->m_View = m_ViewFrame ;

 //   m_ViewFrame->m_ExposureControl.m_iAsynchronousMode = 0;
//   m_ViewFrame->m_wndToolBar.SetButtonInfo
//       (11, ID_ASYNCHRONE, TBBS_BUTTON, 20);
//     m_ViewFrame->SetWindowPos( &CWnd::wndTopMost  , 0, 0, 0, 0, 
//       SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW ) ;
//     m_ViewFrame->SetWindowPos( &CWnd::wndTop  , 0, 0, 0, 0, 
//       SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW ) ;
//    m_ViewFrame->ShowWindow( SW_HIDE ) ;
    CRegistry Reg("File Company\\OpticJig");
    CString AsString = Reg.GetRegiString("Positions", "ImageDlgPos", "");
    if (AsString.GetLength() > 5)
    {
      CRect WindowPos;

      Reg.GetRegiIntSerie("Positions", "ImageDlgPos", (int *)&WindowPos, 4);
      m_ViewFrame->SetWindowPos(NULL, WindowPos.left, WindowPos.top,
        WindowPos.right, WindowPos.bottom, SWP_NOZORDER | SWP_SHOWWINDOW);
    }
    m_ViewFrame->ShowWindow( SW_SHOW ) ;
 

  //SetWindowText((LPCTSTR) status);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CImCNTLDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CImCNTLDlg::OnDestroy()
{
//  if ( m_ViewFrame )
//    m_ViewFrame->Clean() ;
	
  WinHelp(0L, HELP_QUIT);
	CDialog::OnDestroy();
	
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CImCNTLDlg::OnPaint() 
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
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CImCNTLDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// Automation servers should not exit when a user closes the UI
//  if a controller still holds on to one of its objects.  These
//  message handlers make sure that if the proxy is still in use,
//  then the UI is hidden but the dialog remains around if it
//  is dismissed.

void CImCNTLDlg::OnClose() 
{
	if (CanExit())
		{
			IApp()->m_iViewExist = 0;
			CDialog::OnClose();
		}
}

void CImCNTLDlg::OnOK() 
{
	if (CanExit())
		{
		  IApp()->m_iViewExist = 0;
			CDialog::OnOK();
		}
}

void CImCNTLDlg::OnCancel() 
{
	if (CanExit())
		{
			IApp()->m_iViewExist = 0;
		  CDialog::OnCancel();
		}
		
}

BOOL CImCNTLDlg::CanExit()
{
	// If the proxy object is still around, then the automation
	//  controller is still holding on to this application.  Leave
	//  the dialog around, but hide its UI.
	if (m_pAutoProxy != NULL)
	{
		ShowWindow(SW_HIDE);
		return FALSE;
	}

	return TRUE;
}

int CImCNTLDlg::GetSubBackgroundMode()
{
  return m_ViewFrame->m_bSubBackground ;
}

LRESULT CImCNTLDlg::OnRecMessage(WPARAM pstr, LPARAM par)
{	 
	 /********************************************************************
		created:	2005/11/09
		class: CImCNTLDlg
		author:	Michael Son
		
		purpose: output of receiving massage     
	*********************************************************************/
	int iPart = (int)par;
	switch(iPart)
		{
		 	case 1:
		   m_ViewFrame->LogMessage((const char *) pstr );
			 break;
			case 2:
       m_ViewFrame->LogMessage2((const char *) pstr );
			 break;
			case 3:
       m_ViewFrame->LogMessage3((const char *) pstr );
			 break;
		}
	return 0;
}				 



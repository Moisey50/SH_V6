
// GraphDeployDlg.cpp : implementation file
//

#include "stdafx.h"
#include "framework.h"
#include "GraphDeploy.h"
#include "GraphDeployDlg.h"
#include <helpers/propertykitEx.h>
#include <fxfc/FXRegistry.h>
#include "afxdialogex.h"
#include <iostream>
#include <filesystem>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CGraphDeployDlg* thisDlg = NULL;

static UINT LAN_RECEIVED_MSG = RegisterWindowMessage( "LAN_RECEIVED_MSG" ) ;
static const UINT VM_TVDB400_SET_GADGET_PROPERTIES = ::RegisterWindowMessage( _T( "Tvdb400_SetGadgetProperties" ) );

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

void __stdcall PrintMsg( int msgLevel , LPCTSTR src , int msgId , LPCTSTR msgText )
{
  CString mes;
  mes.Format( "+++ %d %s %d %s\n" , msgLevel , src , msgId , msgText );
  if ( thisDlg )
    thisDlg->PrintMessage( mes );
  else
  {
    TRACE( mes );
  }
}


bool FAR __stdcall LANReceived( const char * pMsg , LPVOID lParam )
{
  char * pMsgCopy = strdup( pMsg ) ;
  CGraphDeployDlg * pHost = (CGraphDeployDlg*) lParam ;
  return pHost->PostMessageA( LAN_RECEIVED_MSG , 0 , (LPARAM) pMsgCopy ) ;
}

// CGraphDeployDlg dialog

CGraphDeployDlg::CGraphDeployDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GRAPHDEPLOY_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
  m_bPrintMessage = false ;
  m_pDebugWnd = NULL ;
  m_bAutoStart = false ;
  m_iLANPort = 0 ;
  m_bMinimize = false ;
}

void CGraphDeployDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange( pDX );
  DDX_Control( pDX , IDC_STATUS , m_StatusLED );
  DDX_Control( pDX , IDC_LAN_RECEIVE_STATUS , m_LANStatusLed );
}

BEGIN_MESSAGE_MAP(CGraphDeployDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
  ON_BN_CLICKED( ID_LOAD , &CGraphDeployDlg::OnBnClickedStart )
  ON_BN_CLICKED( ID_BROWSE , &CGraphDeployDlg::OnBnClickedBrowse )
  ON_BN_CLICKED( ID_UNLOAD , &CGraphDeployDlg::OnBnClickedUnload )
  ON_BN_CLICKED( ID_VIEW_GRAPH , &CGraphDeployDlg::OnBnClickedViewGraph )
  ON_REGISTERED_MESSAGE( LAN_RECEIVED_MSG , OnLANMessage )
  ON_REGISTERED_MESSAGE( VM_TVDB400_SET_GADGET_PROPERTIES , OnSetGadgetProperties )
  ON_WM_PARENTNOTIFY()
  ON_BN_CLICKED( IDCANCEL , &CGraphDeployDlg::OnBnClickedCancel )
  ON_WM_MOVE()
END_MESSAGE_MAP()


// CGraphDeployDlg message handlers

BOOL CGraphDeployDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	//SetIcon(m_hIcon, FALSE);		// Set small icon
  m_BlackLed.LoadBitmap( IDB_BLACK_LED ) ;
  m_BlueLed.LoadBitmap( IDB_BLUE_LED ) ;
  m_YellowLed.LoadBitmap( IDB_YELLOW_LED ) ;
  m_RedLed.LoadBitmap( IDB_RED_LED ) ;
  m_GreenLed.LoadBitmap( IDB_GREEN_LED ) ;

  CCommandLineInfo cmdInfo; //cmdInfo.m_nShellCommand=CCommandLineInfo::FileNothing;
  if ( ParseCommandLine( cmdInfo ) )
  {
    if ( (cmdInfo.m_strFileName.GetLength() != 0) && (cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen) )
    {
      if ( LoadAndRunGraph( cmdInfo.m_strFileName , m_bAutoStart ) )
      {
        return TRUE;
      }
    }
  }
  m_LANControlGraph.Init( "LANControl.tvg" , PrintMsg ) ; 
  if ( m_LANControlGraph.IsInitialized() )
  {
    m_LANControlGraph.SetTextCallBack( "LANControl>>0" , LANReceived , this ) ;
    if ( m_iLANPort )
      m_LANControlGraph.SetProperty( "LANControl" , "Port" , m_iLANPort ) ;
    else
      m_LANControlGraph.GetProperty( "LANControl" , "Port" , m_iLANPort ) ;
    FXString AsString ;
    AsString.Format( "Port: %d" , m_iLANPort ) ;
    SetDlgItemText( IDC_LAN_PORT_VIEW , AsString ) ;
    m_LANControlGraph.StartGraph() ;
  }
  else
    SetDlgItemText( IDC_LAN_PORT_VIEW , "No LAN" ) ;

  FXRegistry Reg( "TheFileX\\GraphDeploy" );
  FXString RegiName = FxGetFileName( m_TVGFileName.c_str() )
    + _T( "_DlgPos" );
  cmplx Pos( 100. , 100. ) ;
  Reg.GetRegiCmplx( "GraphDeploy" , RegiName , Pos , Pos ) ;
  m_LastPosition.x = ROUND( Pos.real() ) ;
  m_LastPosition.y = ROUND( Pos.imag() ) ;

  SetWindowPos( &wndTopMost ,
    m_LastPosition.x , m_LastPosition.y , 0 , 0 , SWP_NOSIZE ) ;
 
  if ( m_bMinimize )
    ShowWindow( SW_MINIMIZE ) ;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CGraphDeployDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGraphDeployDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGraphDeployDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void	CGraphDeployDlg::PrintMessage( LPCTSTR message )
{
  m_LogMutex.lock() ;
  m_Message += message;
  m_bPrintMessage = true;
  m_LogMutex.unlock() ;
}



void CGraphDeployDlg::OnBnClickedStart()
{
  TCHAR Buf[ 256 ] ;
  GetDlgItemText( ID_LOAD , Buf , 255 ) ;
  if ( _tcscmp( Buf , "Start" ) == 0 )
  {
    if ( !m_Graph.IsInitialized() )
    {
      GetDlgItemText( IDC_GRAPH_NAME , Buf , 255 ) ;
      LoadAndRunGraph( Buf , true ) ;
      if ( !m_Graph.IsRunning() )
      {
        m_StatusLED.SetBitmap( m_BlackLed ) ;
        return ;
      }
    }
    else
    {
      m_Graph.StartGraph() ;
      SetDlgItemText( ID_LOAD , "Stop" ) ;
    }
  }
  else
  {
    m_Graph.StopGraph() ;
    SetDlgItemText( ID_LOAD , "Start" ) ;
  }

  m_StatusLED.SetBitmap( (m_Graph.IsRunning()) ? m_GreenLed : m_RedLed ) ;
}


void CGraphDeployDlg::OnBnClickedBrowse()
{
  CFileDialog fd( TRUE , "tvg" , "" , OFN_FILEMUSTEXIST , "TVG files (*.tvg)|*.tvg|all files (*.*)|*.*||" , this );
  if ( fd.DoModal() == IDOK )
  {
    LoadAndRunGraph( (LPCTSTR) fd.GetPathName() , false ) ;
  }
}

int CGraphDeployDlg::LoadAndRunGraph( LPCTSTR pGraphName , bool bStart )
{
  if ( m_TVGFileName != pGraphName )
  {
    m_StatusLED.SetBitmap( m_RedLed ) ;
    if ( m_Graph.IsRunning() )
    {
      m_Graph.StopGraph() ;
      m_StatusLED.SetBitmap( m_YellowLed ) ;
      Sleep( 200 ) ;
      m_Graph.Disconnect() ;
    }
    if ( m_Graph.Init( pGraphName , PrintMsg , bStart ) )
    {
      m_TVGFileName = pGraphName ;
      m_StatusLED.SetBitmap( (bStart)? m_GreenLed : m_RedLed ) ;
      string Caption( "Graph Deploy: " ) ;
      Caption += m_TVGFileName ;
      SetWindowText( Caption.c_str() ) ;
      SetDlgItemText( IDC_GRAPH_NAME , m_TVGFileName.c_str() ) ;
      GetDlgItem( ID_UNLOAD )->EnableWindow( TRUE ) ;
      GetDlgItem( ID_VIEW_GRAPH )->EnableWindow( TRUE ) ;
      SetDlgItemText( ID_LOAD , bStart ? "Stop" : "Start" ) ;
    }
    else
    {
      m_TVGFileName.clear() ;
      m_StatusLED.SetBitmap( m_BlackLed ) ;
      SetWindowText( "Graph Deploy: NO RUNNING GRAPH" ) ;
      GetDlgItem( ID_UNLOAD )->EnableWindow( FALSE ) ;
      GetDlgItem( ID_VIEW_GRAPH )->EnableWindow( FALSE ) ;
    }
  }
  return 0;
}

void CGraphDeployDlg::OnBnClickedUnload()
{
  m_Graph.Disconnect() ;
  m_StatusLED.SetBitmap( m_BlackLed ) ;
  GetDlgItem( ID_UNLOAD )->EnableWindow( FALSE ) ;
  GetDlgItem( ID_VIEW_GRAPH )->EnableWindow( FALSE ) ;
  SetDlgItemText( ID_LOAD , "Start" ) ;
  m_TVGFileName.clear() ;
}


void CGraphDeployDlg::OnBnClickedViewGraph()
{
  m_Graph.ViewGraph( this ) ;
  GetDlgItem( ID_VIEW_GRAPH )->EnableWindow( FALSE ) ;
}


void CGraphDeployDlg::OnParentNotify( UINT message , LPARAM lParam )
{
  CDialogEx::OnParentNotify( message , lParam );

  switch ( message & 0xffff )
  {
  case WM_DESTROY:
    if ( (CWnd*)lParam == m_pDebugWnd )
    {
      GetDlgItem( ID_VIEW_GRAPH )->EnableWindow( TRUE ) ;
      m_pDebugWnd = NULL ;
    }
    break ;
  case WM_CREATE:
    m_pDebugWnd = (CWnd*) lParam ;
    break ;
  }
}

bool CGraphDeployDlg::ParseCommandLine( CCommandLineInfo& rCmdInfo )
{
  bool setup = false;
  int iNArg = __argc ;
  for ( int i = 1; i < __argc; i++ )
  {
    LPCTSTR pszParam = __targv[ i ];
    BOOL bFlag = FALSE;
    if ( pszParam[ 0 ] == '-' || pszParam[ 0 ] == '/' )
    {
      // remove flag specifier
      bFlag = TRUE;
      ++pszParam;
      if ( _tcscmp( pszParam , "i" ) == 0 )
        setup = true;
      else if ( _tcscmp( pszParam , "r" ) == 0 )
        m_bAutoStart = true;
      else if ( _tcscmp( pszParam , "lp" ) == 0 )
      {
        if ( i < __argc - 1 )
        {
          pszParam = __targv[ ++i ] ;
          m_iLANPort = (int) ConvToBinary( pszParam ) ;
        }
        if ( i < __argc - 1 )
          continue ;
        else
          bFlag = FALSE ;
      }
      else if ( _tcscmp( pszParam , "min" ) == 0 )
        m_bMinimize = true ;
    }
    BOOL bLast = ((i + 1) == __argc);
    rCmdInfo.ParseParam( pszParam , bFlag , bLast );
  }
  if ( rCmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen && !rCmdInfo.m_strFileName.IsEmpty() )
  {
    return true ;
  }
  return false; // false means continue
}


LRESULT CGraphDeployDlg::OnLANMessage( WPARAM wParam , LPARAM lParam )
{
  FXString Msg( (LPCTSTR) lParam ) ;
  free( (void*)lParam ) ;
  m_LANAccumulator += Msg ;
  SetDlgItemText( IDC_FROM_LAN , Msg ) ;
  ProcessLANCommand() ;
  m_LANStatusLed.SetBitmap( m_YellowLed ) ;
  return 1;
}


bool CGraphDeployDlg::SendToLAN( LPCTSTR pMsg )
{
  SetDlgItemText( IDC_TO_LAN , pMsg ) ;
  m_LANStatusLed.SetBitmap( m_RedLed ) ;
  return m_LANControlGraph.SendText( "LANControl<<0" , pMsg ) ;
}


// Every  LF will lead command processing; the rest will be hold up to next LF received
int CGraphDeployDlg::ProcessLANCommand()
{
  FXSIZE iLFPos = 0  ;
  m_LANAccumulator.Trim( " \t,;[{(:" ) ;
  FXString ForProcessing( m_LANAccumulator ) ;
  ForProcessing.Replace( "\\n" , "\x0a" ) ;
  FXString CommandLine = ForProcessing.Tokenize( "\n" , iLFPos ) ;
  FXString RectAsText ;
  HWND hWnd = NULL ;
  while ( !CommandLine.IsEmpty() ) 
  {
    FXSIZE iTokenEnd = 0 ;
    FXString Token = CommandLine.Tokenize( " \t" , iTokenEnd ) ;
    FXString Gadget = CommandLine.Tokenize( " \t" , iTokenEnd ) ;
    if ( Token.CompareNoCase( "setview" ) == 0 )
    {
      FXString Mode = CommandLine.Tokenize( " \t(" , iTokenEnd ) ;
      FXString hWndAsString ;
      if ( Mode.CompareNoCase( "Handle" ) == 0 )
      {
        FXString Param = CommandLine.Mid( iTokenEnd ) ;
        Param.Trim() ;
        hWndAsString = Param ;
        hWnd = (HWND)ConvToBinary( hWndAsString ) ;
      }
      else if ( Mode.CompareNoCase( "Point" ) == 0 )
      {
        FXString X = CommandLine.Tokenize( ", \t" , iTokenEnd ) ;
        FXString Y = CommandLine.Tokenize( " ,\t)" , iTokenEnd ) ;
        CPoint PtInDesktop( atoi( X ) , atoi( Y ) ) ;
        hWnd = ::WindowFromPoint( PtInDesktop ) ;
        if ( hWnd != m_hWnd )
          hWndAsString.Format( "0x%x" , hWnd ) ;
      }
      else if ( Mode.CompareNoCase( "Reset" ) == 0 )
        hWndAsString = "0" ;
      if ( !hWndAsString.IsEmpty() )
      {
        if ( m_Graph.SetProperty( Gadget , "hTargetWindow" , hWndAsString ) )
        {
          if ( hWnd )
          {
            CRect WR ;
            ::GetWindowRect( hWnd , &WR ) ;
            RectAsText.Format( "LT=(%d,%d) W=%d H=%d" , 
              WR.left , WR.top , WR.Width() , WR.Height() ) ;
          }
          FXString Msg ;
          Msg.Format( "OK, Handle=0x%x, %s" , hWnd , (LPCTSTR) RectAsText ) ;
          SendToLAN( Msg ) ;
        }
        else
        {
          FXString Msg ;
          Msg.Format( "ERROR: Can't set Gad=%s Prop=%s Val=%s" , 
            (LPCTSTR)Gadget , "hTargetWindow" , (LPCTSTR) hWndAsString ) ;
          SendToLAN( Msg ) ;
        }
      }
      else
        SendToLAN( "ERROR: Undefined window handle" ) ;
    }
    else if ( Token.CompareNoCase( "setproperty" ) == 0 )
    {
      FXString PropertyName = CommandLine.Tokenize( " \t(" , iTokenEnd ) ;
      FXString Value = CommandLine.Tokenize( " \t(" , iTokenEnd ) ;
      m_Graph.SetProperty( Gadget , PropertyName , (LPCTSTR) Value ) ;
    }
    
    
    
    CommandLine = ForProcessing.Tokenize( "\n" , iLFPos ) ;
  }
  int iLastLF = (int)m_LANAccumulator.ReverseFind( '\n' ) ;
 
  if ( (iLastLF > 0) && (iLastLF < m_LANAccumulator.GetLength() - 1) )
    m_LANAccumulator = m_LANAccumulator.Mid( iLastLF + 1 ) ;
  else
    m_LANAccumulator.Empty() ;
  return 0;
}


void CGraphDeployDlg::OnBnClickedCancel()
{
  FXRegistry Reg( "TheFileX\\GraphDeploy" );

  CRect WinPos ;
  GetWindowRect( &WinPos ) ;

  FXString RegiName = FxGetFileName( m_TVGFileName.c_str() ) 
    + _T("_DlgPos");

  cmplx Pos( m_LastPosition.x , m_LastPosition.y ) ;

  Reg.WriteRegiCmplx( "GraphDeploy" , RegiName , Pos ) ;
  CDialogEx::OnCancel();
}

LRESULT CGraphDeployDlg::OnSetGadgetProperties( WPARAM wParam , LPARAM lParam )
{
  SetPropertyGlobal Method = (SetPropertyGlobal) wParam ;
  switch ( Method )
  {
  case SPG_ByString:
    {
      FXPropKit2 Params( (LPCTSTR) lParam ) ;
      delete (LPCTSTR) lParam ;
      FXString GadgetName , Properties ;
      if ( ((FXPropertyKit) Params).GetString( "Gadget" , GadgetName )
        && Params.GetStringWithBrackets( "Properties" , Properties ) )
      {
        return (m_Graph.ScanProperties( GadgetName , Properties ) == true) ;
      }
    }
    break ;
  default:
    break ;
  }
  return 0;
}


void CGraphDeployDlg::OnMove( int x , int y )
{
  CDialogEx::OnMove( x , y );

  if ( (x > -30000) && (y > -30000) )
  {
    m_LastPosition.x = x ;
    m_LastPosition.y = y ;
  }
  // TODO: Add your message handler code here
}

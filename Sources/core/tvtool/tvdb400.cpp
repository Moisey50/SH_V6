// tvdb400.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "tvdb400.h"
#include <winver.h>
#include "mainfrm.h"
#include <security\basesecurity.h>
#include <shbase\shbase.h>
#include <gadgets\stdsetup.h>
#include <string.h>
#include <fxfc/FXRegistry.h>

//guardant start
#include "guardant\include\grddongle.h"
#define GrdDC_LP       0x5e617fb9  // Licence public code
#define GrdDC_JUNK     0x123456789  // Junk
#define GrdDC_LR       0x7cf838f4  // Licence private read code
//guardant end

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define REGISTRY_SANDBOX HKEY_LOCAL_MACHINE
#define REGISTRY_SANDBOX HKEY_CURRENT_USER

LPCTSTR g_wszAppID = _T( "FileX.SHStudio" );
LPCTSTR g_wszProgID = _T( "FileX.SHStudioProgID" );


bool WriteRegString( HKEY hkeyParent , LPCTSTR szSubkey , LPCTSTR szValueName , LPCTSTR szData )
{
  HKEY hKey;
  LONG lRet;

  lRet = RegCreateKeyEx( hkeyParent , szSubkey , 0 , REG_NONE , REG_OPTION_NON_VOLATILE , KEY_SET_VALUE , NULL , &hKey , NULL );

  if ( ERROR_SUCCESS != lRet )
    return false;

  lRet = RegSetValueEx( hKey , szValueName , 0 , REG_SZ ,
    (LPBYTE) szData , (DWORD) _tcsclen( szData ) );
  RegCloseKey( hKey );
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
  CString sIconPath , sCommandLine , sProgIDKey;
  TCHAR szModulePath[ MAX_PATH ] = {0};
  GetModuleFileName( NULL , szModulePath , MAX_PATH );
  sIconPath.Format( _T( "\"%s\",-%d" ) , szModulePath , IDI_ICON_DOC );
  sCommandLine.Format( _T( "\"%s\" \"%%1\"" ) , szModulePath );
  sProgIDKey.Format( _T( "software\\classes\\%s" ) , g_wszProgID );
  CString DefaultIcon = sProgIDKey + _T( "\\DefaultIcon" );
  CString CurVer = sProgIDKey + _T( "\\CurVer" );
  CString SheelCommand = sProgIDKey + _T( "\\shell\\open\\command" );
  RegEntry aEntries[] =
  {
      { _T( "software\\classes\\.tvg" ), NULL, g_wszProgID },
      { _T( "software\\classes\\.tvg\\OpenWithProgIDs" ), g_wszProgID, _T( "" ) },
      { sProgIDKey, _T( "FriendlyTypeName" ), _T( "SHStudio project file" ) },
      { sProgIDKey, _T( "AppUserModelID" ), g_wszAppID },
      { DefaultIcon, NULL, sIconPath },
      { CurVer, NULL, g_wszProgID },
      { SheelCommand, NULL, sCommandLine }
  };
  for ( int i = 0; i < sizeof( aEntries ) / sizeof( RegEntry ); i++ )
  {
    if ( !WriteRegString( REGISTRY_SANDBOX , aEntries[ i ].sKey , aEntries[ i ].szValue , aEntries[ i ].szData ) )
      return false;
  }
  return true;
}
/////////////////////////////////////////////////////////////////////////////
// CTvdb400App

BEGIN_MESSAGE_MAP( CTvdb400App , CWinApp )
  //{{AFX_MSG_MAP(CTvdb400App)
  ON_COMMAND( ID_APP_ABOUT , OnAppAbout )
  ON_COMMAND( ID_HELP , OnHelp )
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTvdb400App construction

CTvdb400App::CTvdb400App() :
  m_DLLLoader( "TheFileX\\SHStudio" ) ,
  m_AutoStart( false ) ,
  m_bStartMinimized( false )
{}

/////////////////////////////////////////////////////////////////////////////
// The one and only CTvdb400App object

CTvdb400App theApp;
CMainFrame* pFrame = NULL;

/////////////////////////////////////////////////////////////////////////////
// CTvdb400App initialization
__forceinline bool CheckDllVersions()
{
  FXString version = FxGetProductVersion( "SHStudio.exe" );
  if ( version.GetLength() == 0 ) return false;
  FXString fxfcver = FxGetProductVersion( FXFC_DLL_NAME );
  if ( version != fxfcver ) return false;
  FXString gadbase = FxGetProductVersion( GADBASE_DLL_NAME );
  if ( version != gadbase ) return false;
  FXString shdbase = FxGetProductVersion( SHDBASE_DLL_NAME );
  if ( version != shdbase ) return false;
  FXString shkernel = FxGetProductVersion( SHKERNEL_DLL_NAME );
  if ( version != shkernel ) return false;
  FXString shstdsetup = FxGetProductVersion( STDSETUP_DLL_NAME );
  if ( version != shstdsetup ) return false;
  FXString tview = FxGetProductVersion( TVVIEW_DLL_NAME );
  if ( version != tview ) return false;
  FXString shvideo = FxGetProductVersion( SHVIDEO_DLL_NAME );
  if ( version != shvideo ) return false;
  return true;
}

BOOL CTvdb400App::InitInstance()
{
  if ( !CheckDllVersions() )
  {
    AfxMessageBox( "Application can not be started:\n"
      "Incompatible SH DLLs Versions." );
    return FALSE;
  }
  // Standard initialization
#ifdef _AFXDLL
  Enable3dControls();			// Call this when using MFC in a shared DLL
#else
  Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

  // Change the registry key under which our settings are stored.

  SetRegistryKey( _T( "TheFileX" ) );
  VERIFY( RegisterAsHandler() );

  FXRegistry Reg( _T( "TheFileX\\SHStudio" ) ) ;
  BOOL bShowSplash = Reg.GetRegiInt( "root" , "ShowSplash" , 1 ) ;
  CSplashWnd::EnableSplashScreen( bShowSplash != FALSE );
  CCommandLineInfo cmdInfo; //cmdInfo.m_nShellCommand=CCommandLineInfo::FileNothing;
  if ( ParseCommandLine( cmdInfo ) )
  {
    return FALSE; // No plrogram starting required...
  }
  CString owner;

  bool bLocalMachine = false ;
  char cIsRestricted = isevaluation( bLocalMachine ) ;
  if ( cIsRestricted == EOL_NoLicense )
  {
    MessageBox( NULL , "This software is not registered, do registration" ,
      "REGISTRATION PROBLEM" , MB_ICONSTOP | MB_OK ) ;
    return FALSE ;
  }
  else if ( !bLocalMachine && (cIsRestricted >= EOL_Evaluation ) )
    getowner( owner );

  if ( owner.IsEmpty() )
    owner = _T( "Unknown" );

  CSplashWnd::ShowSplashScreen( true ); // Value is not important

  if ( cIsRestricted == EOL_Evaluation )
  {
    TRACE( "It's remain %d days before end of evaluation. Registered company name is %s\n" , daysremain() , owner );
  }
  else if ( cIsRestricted == EOL_License )
  {
    TRACE( "It's remain %d days before end of license. Registered company name is %s\n" , daysremain() , owner );
  }
  else // no restriction
  {
    TRACE( "This software is registered to \"%s\"\n" , owner );
  }

  if ( cIsRestricted && (isexpired()) )
  {
    //CSplashWnd::WaitEnd();
    CSplashWnd::WaitEnd();
    AfxMessageBox( ( cIsRestricted == EOL_Evaluation ) ?
      "Evaluation period expired." : "License expired.", MB_ICONSTOP );

    return FALSE;
  }

#ifndef GUARDANT_DEBUG
  //guardant start
  int ret = 0;
  char pExe[ 32 ];
  memset( pExe , 0 , 32 );
  DWORD ID;
  CGrdDongle grdDongle;
  grdDongle.Create( GrdDC_LP , GrdDC_LR , 0 , 0 );
  do
  {
    ret = grdDongle.Find( GrdF_Next , &ID );
    ret = grdDongle.Login();
    if ( ret != 0 )
    {
      //CSplashWnd::WaitEnd();
      CSplashWnd::WaitEnd();
      AfxMessageBox( "No valid HWKey found" , MB_ICONSTOP );
      return FALSE;
    }
    ret = grdDongle.PI_Read( 0 , 0 , 32 , pExe );
    if ( ret != 0 )
    {
      grdDongle.Logout();
      continue;
    }
    char    szAppPath[ MAX_PATH ] = "";
    CString strAppName;
    ::GetModuleFileName( 0 , szAppPath , sizeof( szAppPath ) - 1 );
    strAppName = szAppPath;
    strAppName = strAppName.Right( strAppName.GetLength() - strAppName.ReverseFind( '\\' ) - 1 );
    for ( int i = 0; i < min( 32 , strAppName.GetLength() ); i++ )
      ret += pExe[ i ] - strAppName[ i ];
    if ( abs( ret ) > 0 )
      grdDongle.Logout();
    else
      break;
  } while ( true );
  //guardant end
#endif

  pFrame = new CMainFrame;
  m_pMainWnd = pFrame;

  // create and load the frame with its resources

    //CSplashWnd::WaitEnd();
  CSplashWnd::WaitEnd();
  if ( !pFrame->LoadFrame( IDR_MAINFRAME , WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE , NULL , NULL ) )
  {
    AfxMessageBox( "Failed to create frame!" );
    return FALSE;
  }

  if ( (cmdInfo.m_strFileName.GetLength() != 0) && (cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen) )
  {
    if ( !pFrame->LoadGraphFile( cmdInfo.m_strFileName , m_AutoStart ) )
    {
      return TRUE;
    }
  }
  pFrame->UpdateWindow();
  pFrame->SetActiveWindow();

  if (m_bStartMinimized)
    pFrame->PostMessage(WM_SYSCOMMAND, SC_MINIMIZE, NULL);  //ShowWindow(SW_MINIMIZE);

//   SystemIDs IDs ;
//   IDs.GetThisSystemInfo() ;
//   IDs.SaveSystemInfo( false ) ;

  return TRUE;
}

int CTvdb400App::ExitInstance()
{
  return CWinApp::ExitInstance();
}

bool CTvdb400App::ParseCommandLine( CCommandLineInfo& rCmdInfo )
{
  bool setup = false;
  for ( int i = 1; i < __argc; i++ )
  {
    LPCTSTR pszParam = __targv[ i ];
    BOOL bFlag = FALSE;
    BOOL bLast = ((i + 1) == __argc);
    if ( pszParam[ 0 ] == '-' || pszParam[ 0 ] == '/' )
    {
      // remove flag specifier
      bFlag = TRUE;
      ++pszParam;
      if (_tcscmp(pszParam, "i") == 0)
        setup = true;
      else if (_tcscmp(pszParam, "r") == 0)
        m_AutoStart = true;
      else if (_tcscmp(pszParam, "min") == 0)
        m_bStartMinimized = true;
      else if ( _tcscmp( pszParam , "l" ) == 0 ) // Set or remove gadget license
      {
        bFlag = FALSE ;
        if ( __argc > i + 2 )
        {
          CString Name = __targv[ i + 1 ] ;
          int iDays = atoi( __targv[ i + 2 ] ) ;
          if ( !Name.IsEmpty() && (iDays >= 0) )
          {
            Tvdb400_AddRemoveLicense( ( LPCTSTR ) Name , iDays ) ;
            int iNRestDays = Tvdb400_CheckLicense( Name ) ;
          }
          i += 2 ;
        }
      }
    }
    else if ( (setup) && (_tcsicoll( pszParam , "eval" ) == 0) )
    {
      if ( !register_eval() )
      {
        AfxMessageBox( "Error: Can't setup for evaluation.- probably evaluation period already expired!" );
      }
      return true; // true means exit
    }
    else
      rCmdInfo.ParseParam( pszParam , bFlag , bLast );
  }
  if ( rCmdInfo.m_nShellCommand == CCommandLineInfo::FileNew && !rCmdInfo.m_strFileName.IsEmpty() )
    rCmdInfo.m_nShellCommand = CCommandLineInfo::FileOpen;
  return false; // false means continue
}


/////////////////////////////////////////////////////////////////////////////
// CTvdb400App message handlers





/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
  CAboutDlg();

  // Dialog Data
    //{{AFX_DATA(CAboutDlg)
  enum
  {
    IDD = IDD_ABOUTBOX
  };
  //}}AFX_DATA

  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CAboutDlg)
protected:
  virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  //{{AFX_MSG(CAboutDlg)
  virtual BOOL OnInitDialog();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog( CAboutDlg::IDD )
{
  //{{AFX_DATA_INIT(CAboutDlg)
  //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange( CDataExchange* pDX )
{
  CDialog::DoDataExchange( pDX );
  //{{AFX_DATA_MAP(CAboutDlg)
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP( CAboutDlg , CDialog )
  //{{AFX_MSG_MAP(CAboutDlg)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CTvdb400App::OnAppAbout()
{
  CAboutDlg aboutDlg;
  aboutDlg.DoModal();
}

void CTvdb400App::OnHelp()
{
  ShowHelp();
}

/////////////////////////////////////////////////////////////////////////////
// CTvdb400App message handlers

BOOL CAboutDlg::OnInitDialog()
{
  CDialog::OnInitDialog();
  CString owner;

  CString fname = CString( AfxGetApp()->m_pszExeName ) + ".exe";
  FXString version = FxGetProductVersion( fname );
  GetDlgItem( IDC_VERSION )->SetWindowText( version );
  bool bLocalMachine = true ;
  char EvaluationOrLicense = isevaluation( bLocalMachine ) ;
  if ( EvaluationOrLicense )
  {
    CString days;
    if ( bLocalMachine )
    {
      days.Format( "%d (by HKLM)" , daysremain() );
      owner = "Unknown" ;
    }
    else
    {
      if ( ( int ) EvaluationOrLicense >= EOL_Evaluation )
        days.Format( "%d (by HKCU)" , daysremain() );
      else
        days = "Unlimited" ;
      if ( EvaluationOrLicense >= EOL_License || EvaluationOrLicense >= EOL_NoRestriction )
        getowner( owner ) ;
    }
    GetDlgItem( IDC_DAYSREMAINTITLE )->ShowWindow( SW_SHOW );
    GetDlgItem( IDC_DAYSREMAIN )->SetWindowText( days );
  }
  else
  {
    GetDlgItem( IDC_DAYSREMAINTITLE )->ShowWindow( SW_HIDE );
    if ( bLocalMachine )
      getowner( owner ) ;
  }
  if ( getowner( owner ) )
    GetDlgItem( IDC_LICENSEDTO )->SetWindowText( owner );

  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}

// PressureMeterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gadgets\stdsetup.h"
#include "PressureMeter.h"
#include "PressureMeterDlg.h"
#include <gadgets\textframe.h>
#include "DataBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_MY_TRAY_NOTIFICATION WM_USER+0
#define MESUREMENT_INTERVAL 600000

void __stdcall PrintMsg( int msgLevel , LPCTSTR src , int msgId , LPCTSTR msgText )
{
  TRACE( "+++ %d %s %d %s\n" , msgLevel , src , msgId , msgText );
}

BOOL CALLBACK CPressureMeterDlg_OutputCallback( CDataFrame*& lpData , FXString& idPin , void* lpParam )
{
  return ((CPressureMeterDlg*) lpParam)->OnData( lpData );
}


/////////////////////////////////////////////////////////////////////////////
// CPressureMeterDlg dialog

CPressureMeterDlg::CPressureMeterDlg( CWnd* pParent /*=NULL*/ )
  : CDialog( CPressureMeterDlg::IDD , pParent ) ,
  m_trayIcon( NULL ) ,
  m_TVGFileName( "pressure.tvg" ) ,
  m_data( 0.0 ) ,
  m_count( 0 )

{
  //{{AFX_DATA_INIT(CPressureMeterDlg)
    // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
  m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
}

CPressureMeterDlg::~CPressureMeterDlg()
{
  if ( m_trayIcon ) delete m_trayIcon;
}

void CPressureMeterDlg::DoDataExchange( CDataExchange* pDX )
{
  CDialog::DoDataExchange( pDX );
  //{{AFX_DATA_MAP(CPressureMeterDlg)
    // NOTE: the ClassWizard will add DDX and DDV calls here
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP( CPressureMeterDlg , CDialog )
  //{{AFX_MSG_MAP(CPressureMeterDlg)
  ON_WM_QUERYDRAGICON()
  ON_MESSAGE( WM_MY_TRAY_NOTIFICATION , OnTrayNotification )
  ON_COMMAND( ID_TRAY_RESTORE , OnTrayRestore )
  ON_WM_SIZE()
  ON_WM_DESTROY()
  ON_BN_CLICKED( IDC_SETUP , OnSetup )
  ON_WM_TIMER()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPressureMeterDlg message handlers

BOOL CPressureMeterDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  SetIcon( m_hIcon , TRUE );
  SetIcon( m_hIcon , FALSE );
  m_pBuilder = Tvdb400_CreateBuilder();
  FxInitMsgQueues( PrintMsg );
  m_PluginLoader = m_pBuilder->GetPluginLoader();
  m_PluginLoader->RegisterPlugins( m_pBuilder );
  if ( m_pBuilder->Load( m_TVGFileName ) == MSG_ERROR_LEVEL )
  {
    CString mes;
    mes.Format( "Can't find '%s'" , m_TVGFileName );
    AfxMessageBox( mes );
    EndDialog( -1 );
    return FALSE;
  }
  if ( !m_pBuilder->SetOutputCallback( "NeedleDetector1>>1" , CPressureMeterDlg_OutputCallback , this ) )
  {
    CString mes;
    mes.Format( "Can't find pin '%s'" , "NeedleDetector1>>1" );
    AfxMessageBox( mes );
    EndDialog( -1 );
    return FALSE;
  }
  m_pBuilder->Start();
  if ( !m_pBuilder->IsRuning() )
  {
    AfxMessageBox( "Can't start routine" );
    EndDialog( -1 );
    return FALSE;
  }
  m_Timer = SetTimer( IDD_PRESSUREMETER_DIALOG , MESUREMENT_INTERVAL , NULL );
  return TRUE;
}

void CPressureMeterDlg::OnDestroy()
{
  m_pBuilder->SetOutputCallback( "NeedleDetector1>>1" , NULL , NULL ); // disconnect
  KillTimer( m_Timer );
  if ( m_pBuilder )
    m_pBuilder->Stop();
  m_pBuilder->Release(); m_pBuilder = NULL;

  FxExitMsgQueues();
  CDialog::OnDestroy();
}

void CPressureMeterDlg::OnSetup()
{
  Tvdb400_RunSetupDialog( m_pBuilder );
  if ( !m_pBuilder->Save( m_TVGFileName ) )
    AfxMessageBox( "Failed to save graph!" );
}

BOOL CPressureMeterDlg::OnData( CDataFrame* lpData )
{

  CTextFrame* tf = lpData->GetTextFrame();
  if ( tf )
  {
    CString tmpS = tf->GetString();
    {
      FXAutolock al( m_Lock );
      m_data += atof( tmpS );
      m_count++;
    }
    GetDlgItem( IDC_RESULT )->SetWindowText( tmpS );
  }
  lpData->Release( lpData );
  return TRUE;
}

void CPressureMeterDlg::OnTimer( UINT_PTR nIDEvent )
{
  double result = 0;
  if ( m_count )
  {
    FXAutolock al( m_Lock );
    result = m_data / m_count;
    m_data = 0;
    m_count = 0;
  }
  if ( result != 0 )
  {
    CDataBase db;
    db.AddValue( result );
  }
  CDialog::OnTimer( nIDEvent );
}


/// Tray functionality

HCURSOR CPressureMeterDlg::OnQueryDragIcon()
{
  return (HCURSOR) m_hIcon;
}

void CPressureMeterDlg::OnSize( UINT nType , int cx , int cy )
{
  if ( nType == SIZE_MINIMIZED )
  {
    ShowWindow( SW_HIDE );
    m_trayIcon = new CTrayIcon( IDR_TRAYICON );
    m_trayIcon->SetNotificationWnd( this , WM_MY_TRAY_NOTIFICATION );
    m_trayIcon->SetIcon( m_hIcon , "_Your application name_" );
  }
  else
  {
    CDialog::OnSize( nType , cx , cy );
  }
}

// Handle notification from tray icon: display a message.
//
LRESULT CPressureMeterDlg::OnTrayNotification( WPARAM uID , LPARAM lEvent )
{
  if ( m_trayIcon )
    return m_trayIcon->OnTrayNotification( uID , lEvent );
  else
    return(0);
}

void CPressureMeterDlg::OnTrayRestore()
{
  delete m_trayIcon;
  m_trayIcon = NULL;
  ShowWindow( SW_RESTORE );
}


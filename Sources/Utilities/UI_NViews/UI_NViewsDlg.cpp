
// UI_NViewsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "framework.h"
#include "UI_NViews.h"
#include "UI_NViewsDlg.h"
#include "DlgProxy.h"
#include "afxdialogex.h"
#include <habitat.h>
#include <gadgets/stdsetup.h>
#include <helpers/propertykitEx.h>
#include <fxfc/FXRegistry.h>
#include <gadgets/quantityframe.h>
#include <gadgets/videoframe.h>
#include <fxfc/CSystemMonitorsEnumerator.h>
#include "gadgets/TectoMsgs.h"
#include "ShellAPI.h"
#include "video/shvideo.h"
#include <psapi.h>
#include <process.h>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CUI_NViews * CUI_NViews::m_pCurrentDlg = NULL;
CUI_NViews * g_thisDlg = NULL ;
static const UINT VM_TVDB400_SHOWVOSETUPDLG = ::RegisterWindowMessage(
  _T( "Tvdb400_ShowVOSetupDialog" ) );
static const UINT WM_CAMERA_CALLBACK = ::RegisterWindowMessage(
  _T( "CallBackFromCamera" ) );

static const UINT WM_REQUEST_FROM_SHARED_MEM = ::RegisterWindowMessage(
  _T( "RequestFromSharedMem" ) );

static const UINT WM_RESULT_CALLBACK_FOR_TECTO = ::RegisterWindowMessage(
  _T( "ResultCallBackForTecto" ) );

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
  virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support

// Implementation
protected:
  DECLARE_MESSAGE_MAP()
public:
};

CAboutDlg::CAboutDlg() : CDialogEx( IDD_ABOUTBOX )
{
  EnableActiveAccessibility();
}

void CAboutDlg::DoDataExchange( CDataExchange* pDX )
{
  CDialogEx::DoDataExchange( pDX );
}

BEGIN_MESSAGE_MAP( CAboutDlg , CDialogEx )
END_MESSAGE_MAP()


void TColorText::setTransparent( bool ATransparent )
{
  MTransparent = ATransparent;
  Invalidate();
}

void TColorText::SetBackgroundColor( COLORREF AColor )
{
  MBackgroundColor = AColor;
  MTransparent = false;
  Invalidate();
}

void TColorText::SetTextColor( COLORREF AColor )
{
  MTextColor = AColor;
  Invalidate();
}

BEGIN_MESSAGE_MAP( TColorText , CStatic )
  ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

HBRUSH TColorText::CtlColor( CDC* pDC , UINT nCtlColor )
{
  pDC->SetTextColor( MTextColor );
  pDC->SetBkMode( OPAQUE );  // we do not want to draw background when drawing text. 
                                  // background color comes from drawing the control background.
  if ( MTransparent )
    return nullptr;  // return nullptr to indicate that the parent object 
                     // should supply the brush. it has the appropriate background color.
  else
    return ( HBRUSH ) CreateSolidBrush( MBackgroundColor );  // color for the empty area of the control
}


void __stdcall PrintGraphMessage(
  int msgLevel , const char * src , int msgId , const char * msgText )
{
  char buf[ 2000 ] ;
  const char * pPt = strchr( src , '.' ) ;

  sprintf_s( buf , 2000 , "%s: %d %s: %s\n" ,
    ( LPCTSTR ) GetTimeAsString_ms() , msgLevel ,
    pPt ? pPt + 1 : src , msgText ) ;
  if ( g_thisDlg && ( msgLevel >= 0 ) )
  {
    g_thisDlg->PrintMessage( buf );
  }
  else
  {
    TRACE( buf );
  }
}

bool __stdcall rcbCam1( CallBackDataA& Data , void* wParam )
{
  CUI_NViews* pWnd = ( CUI_NViews* ) wParam ;
  if ( pWnd && IsWindow( pWnd->m_hWnd ) )
    return ( ( ( CUI_NViews* ) wParam )->PostMessageA( WM_CAMERA_CALLBACK , 1 ) != FALSE ) ;
  return false ;
}
bool __stdcall rcbCam2( CallBackDataA& Data , void* wParam )
{
  CUI_NViews* pWnd = ( CUI_NViews* ) wParam ;
  if ( pWnd && IsWindow( pWnd->m_hWnd ) )
    return ( ( ( CUI_NViews* ) wParam )->PostMessageA( WM_CAMERA_CALLBACK , 2 ) != FALSE ) ;
  return false ;
}

bool FAR __stdcall ResultForTectoCallBack( const char * pMsg , LPVOID lParam )
{
  char * pMsgCopy = strdup( pMsg ) ;
  CUI_NViews * pHost = ( CUI_NViews* ) lParam ;
  return pHost->PostMessageA( WM_RESULT_CALLBACK_FOR_TECTO , 0 , ( LPARAM ) pMsgCopy ) ;
}

bool FAR __stdcall ImageForTectoCallBack0( CallBackDataA& Data , void* wParam )
{
  CUI_NViews * pHost = ( CUI_NViews* ) wParam ;
  return pHost->SendMessageA( USER_WM_IMAGE_SIMULATION_0 , 0 , ( LPARAM ) &Data ) ;
}
bool FAR __stdcall ImageForTectoCallBack1( CallBackDataA& Data , void* wParam )
{
  CUI_NViews * pHost = ( CUI_NViews* ) wParam ;
  return pHost->SendMessageA( USER_WM_IMAGE_SIMULATION_0 , 1 , ( LPARAM ) &Data ) ;
}
bool FAR __stdcall ImageForTectoCallBack2( CallBackDataA& Data , void* wParam )
{
  CUI_NViews * pHost = ( CUI_NViews* ) wParam ;
  return pHost->SendMessageA( USER_WM_IMAGE_SIMULATION_0 , 2 , ( LPARAM ) &Data ) ;
}
bool FAR __stdcall ImageForTectoCallBack3( CallBackDataA& Data , void* wParam )
{
  CUI_NViews * pHost = ( CUI_NViews* ) wParam ;
  return pHost->SendMessageA( USER_WM_IMAGE_SIMULATION_0 , 3 , ( LPARAM ) &Data ) ;
}

void ReceiveFromShMemFunc( CUI_NViews * pViews )
{
  TRACE( "\nReceiveFromShMemFunc started" ) ;
  // This application always works as server, i.e. provides reactions on
  // messages in receive section
  HANDLE hEvent = pViews->m_pShMemControl->GetInEventHandle() ;
  while ( !pViews->m_bFinishReceivengShMemMsgs )
  {
    DWORD Res = WaitForSingleObject( hEvent , 50 ) ;
    if ( pViews->m_bFinishReceivengShMemMsgs )
      break ;
    switch ( Res )
    {
      case WAIT_OBJECT_0:
        pViews->ProcessDataFromShMem() ;
        break ;
      case WAIT_TIMEOUT: break ;
      default: pViews->ProcessErrorFromShMem( Res ) ; break ;
    }
  }
  TRACE( "\nReceiveFromShMemFunc exit" ) ;
}

LPCTSTR GetWorkingModeName( GadgetWorkinMode Mode )
{
  switch ( Mode )
  {
    case GWM_Reject: return "Reject" ;
    case GWM_Transmit: return "Transmit" ;
    case GWM_Process: return "Process" ;
  }
  return "Unknown" ;
}



// CUI_NViews dialog


IMPLEMENT_DYNAMIC( CUI_NViews , CDialogEx );

CUI_NViews::CUI_NViews( CWnd* pParent /*=nullptr*/ )
  : CDialogEx( IDD_UI_NViews_DIALOG , pParent )
  , m_bCam1CallBackPassed( false )
  , m_bCam2CallBackPassed( false )
  , m_DataCam1( DATA_IMAGE )
  , m_DataCam2( DATA_IMAGE )
{
  EnableActiveAccessibility();
  m_hIcon = AfxGetApp()->LoadIcon( IDI_FLOWERS );
  m_pAutoProxy = nullptr;
//  FXRegistry Reg( "TheFileX\\Micropoint" );
}

CUI_NViews::~CUI_NViews()
{
  // If there is an automation proxy for this dialog, set
  //  its back pointer to this dialog to null, so it knows
  //  the dialog has been deleted.
  if ( m_pAutoProxy != nullptr )
    m_pAutoProxy->m_pDialog = nullptr;
  m_bAppClosing = TRUE ;
}

void CUI_NViews::DoDataExchange( CDataExchange* pDX )
{
  CDialogEx::DoDataExchange( pDX );
  DDX_Control(pDX, IDC_CAM1_STATUS, m_Cam1StatusLED);
  DDX_Control(pDX, IDC_CAM2_STATUS, m_Cam2StatusLED);
  DDX_Control( pDX , IDC_LOG_LIST , m_LogView );

//   DDX_Text( pDX , IDC_HORIZ_DIST , m_dHorizontalLineDist_mm );
//   DDX_Text( pDX , IDC_VERT_DIST , m_dVerticalLineDist_mm );
}

BEGIN_MESSAGE_MAP( CUI_NViews , CDialogEx )
  ON_WM_SYSCOMMAND()
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_MESSAGE( USER_WM_LOG_MSG , &CUI_NViews::OnLogMsg )
  ON_WM_TIMER()
  ON_WM_CTLCOLOR()
  ON_WM_PARENTNOTIFY()
  ON_BN_CLICKED( IDC_CLOSE , &CUI_NViews::OnBnClickedClose )
  ON_BN_CLICKED( IDC_VIEW_SETUP , &CUI_NViews::OnBnClickedViewSetup )
  ON_BN_CLICKED( IDC_VIEW_GRAPH , &CUI_NViews::OnBnClickedViewGraph )
  ON_BN_CLICKED( IDC_SAVE_GRAPH , &CUI_NViews::OnBnClickedSaveGraph )
  ON_EN_KILLFOCUS( IDC_HORIZ_DIST , &CUI_NViews::OnEnKillFocusHorizDist )
  ON_EN_KILLFOCUS( IDC_VERT_DIST , &CUI_NViews::OnEnKillfocusVertDist )
  ON_BN_CLICKED( IDC_TECHNICIAN , &CUI_NViews::OnBnClickedTechnician )
  ON_EN_KILLFOCUS( IDC_SKEW_CALIB_UM , &CUI_NViews::OnEnKillfocusSkewCalibUm )
  ON_REGISTERED_MESSAGE( WM_REQUEST_FROM_SHARED_MEM , OnShMemMessage )
  ON_REGISTERED_MESSAGE( WM_RESULT_CALLBACK_FOR_TECTO , OnAnswerForTectoLib )
  ON_MESSAGE( USER_WM_IMAGE_SIMULATION_0 , OnSimuImageArrived )
  ON_MESSAGE( USER_WM_IMAGE_SIMULATION_1 , OnSimuImageArrived )
  ON_MESSAGE( USER_WM_IMAGE_SIMULATION_2 , OnSimuImageArrived )
  ON_MESSAGE( USER_WM_IMAGE_SIMULATION_3 , OnSimuImageArrived )
  ON_BN_CLICKED( IDC_SIDE_PROC_SETUP , &CUI_NViews::OnBnClickedSideProcSetup )
  ON_BN_CLICKED( IDC_FRONT_PROC_SETUP , &CUI_NViews::OnBnClickedFrontProcSetup )
  ON_BN_CLICKED( IDC_SAVE_TO_REGISTRY , &CUI_NViews::OnBnClickedSaveToRegistry )
  ON_EN_CHANGE( IDC_FLOWER_NAME , &CUI_NViews::OnEnChangeFlowerName )
  ON_EN_KILLFOCUS( IDC_FLOWER_NAME , &CUI_NViews::OnEnKillfocusFlowerName )
  ON_EN_UPDATE( IDC_FLOWER_NAME , &CUI_NViews::OnEnUpdateFlowerName )
  ON_BN_CLICKED( IDC_SET_FLOWER , &CUI_NViews::OnBnClickedSetFlower )
  ON_BN_CLICKED( IDC_GET_NEXT , &CUI_NViews::OnBnClickedGetNext )
  ON_BN_CLICKED( IDC_SAVE_SRC , &CUI_NViews::OnBnClickedSaveSrc )
  ON_BN_CLICKED( IDC_SAVE_RESULT , &CUI_NViews::OnBnClickedSaveResult )
  ON_BN_CLICKED( IDC_RESULT1 , &CUI_NViews::OnBnClickedResult1 )
  ON_BN_CLICKED( IDC_SRC1 , &CUI_NViews::OnBnClickedSrc1 )
  ON_BN_CLICKED( IDC_RESULT2 , &CUI_NViews::OnBnClickedResult2 )
  ON_BN_CLICKED( IDC_SRC2 , &CUI_NViews::OnBnClickedSrc2 )
  ON_BN_CLICKED( IDC_RESULT1_ON_2 , &CUI_NViews::OnBnClickedResult1On2 )
  ON_BN_CLICKED( IDC_BATCH , &CUI_NViews::OnBnClickedBatch )
END_MESSAGE_MAP()

// CUI_NViews message handlers

BOOL CUI_NViews::OnInitDialog()
{
  CDialogEx::OnInitDialog();

  // Add "About..." menu item to system menu.

  // IDM_ABOUTBOX must be in the system command range.
  ASSERT( ( IDM_ABOUTBOX & 0xFFF0 ) == IDM_ABOUTBOX );
  ASSERT( IDM_ABOUTBOX < 0xF000 );

  CMenu* pSysMenu = GetSystemMenu( FALSE );
  if ( pSysMenu != nullptr )
  {
    BOOL bNameValid;
    CString strAboutMenu;
    bNameValid = strAboutMenu.LoadString( IDS_ABOUTBOX );
    ASSERT( bNameValid );
    if ( !strAboutMenu.IsEmpty() )
    {
      pSysMenu->AppendMenu( MF_SEPARATOR );
      pSysMenu->AppendMenu( MF_STRING , IDM_ABOUTBOX , strAboutMenu );
    }
  }

  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog
  //SetIcon(hIcon, FALSE);		// Set small icon

  m_BlackLed.LoadBitmap( IDB_BLACK_LED ) ;
  m_BlueLed.LoadBitmap( IDB_BLUE_LED ) ;
  m_YellowLed.LoadBitmap( IDB_YELLOW_LED ) ;
  m_RedLed.LoadBitmap( IDB_RED_LED ) ;
  m_GreenLed.LoadBitmap( IDB_GREEN_LED ) ;

  m_Cam1StatusLED.SetBitmap( m_BlackLed ) ;
  m_Cam2StatusLED.SetBitmap( m_BlackLed ) ;

  g_thisDlg = this ;

  CRect OriginalDlgWindowPos , OriginalDlgClientRect , CloseRect , ViewSetupRect , ViewGraphRect , SaveGraphRect ;
  GetWindowRect( OriginalDlgWindowPos ) ;
  GetClientRect( &OriginalDlgClientRect ) ;

  LoadPresentationParamsFromRegistry() ;
  bool bRes = m_Graph.Init( m_GraphName , PrintGraphMessage , false ) ;
  if ( !bRes )
  {

    FXString Content( "Can't load graph: " ) ;
    Content += ( m_Graph.m_EvaluationMsg.IsEmpty() ) ? m_LastErrorMsg : m_Graph.m_EvaluationMsg ;
    MessageBox( ( LPCTSTR ) Content , m_GraphName , MB_ICONERROR | MB_OK ) ;

    PostQuitMessage( -5 ) ;
    return FALSE ;
  }


//   GetDlgItem( IDC_CLOSE )->GetWindowRect( &CloseRect ) ;
//   GetDlgItem( IDC_VIEW_SETUP )->GetWindowRect( &ViewSetupRect )  ;
//   GetDlgItem( IDC_VIEW_GRAPH )->GetWindowRect( &ViewGraphRect )  ;
//   GetDlgItem( IDC_SAVE_GRAPH )->GetWindowRect( &SaveGraphRect )  ;
  const CSystemMonitorsEnumerator * pEnumerator = CSystemMonitorsEnumerator::GetMonitorEnums() ;

  const CRect MonitorRect = pEnumerator->GetMonitorRect( m_iMonitorNumber ) ;

  CSize DlgWindowSize( ROUND( MonitorRect.Width() * m_csDialogSize.real() ) ,
    ROUND( MonitorRect.Height() * m_csDialogSize.imag() ) ) ;

  FXRegistry Reg( "TheFileX\\UI_NViews" );
  cmplx cNormWinPosLT ;
  Reg.GetRegiCmplx( m_ConfigName , "DialogLTPos" , cNormWinPosLT , cmplx( 0.05 , 0.05 ) ) ;

  CPoint DlgLeftTop( MonitorRect.left + ROUND( MonitorRect.Width() * cNormWinPosLT.real() ) ,
    MonitorRect.top + ROUND( MonitorRect.Height() * cNormWinPosLT.imag() ) ) ;

  SetWindowPos( NULL , DlgLeftTop.x , DlgLeftTop.y ,
    DlgWindowSize.cx , DlgWindowSize.cy , SWP_SHOWWINDOW | SWP_DRAWFRAME ) ;

  CRect DlgClientArea ;
  GetClientRect( &DlgClientArea ) ;

  CSize LogSize( ROUND( DlgClientArea.Width() * m_dRelLogWidth ) - 20 ,
    ROUND( DlgClientArea.Height() * m_dRelLogHeight ) - 10 ) ;
  CPoint LogPosition( 10 , DlgClientArea.Height() - LogSize.cy - 10 ) ;
  GetDlgItem( IDC_LOG_LIST )->SetWindowPos( NULL ,
    LogPosition.x , LogPosition.y , LogSize.cx , LogSize.cy , SWP_SHOWWINDOW ) ;

  // Common controls positions
  CRect Edges( -1 , LogPosition.y + ( int ) ( LogSize.cy * 0.4 ) ,
    -1 , LogPosition.y + LogSize.cy ) ;
  SetNewRectPosition( OriginalDlgClientRect , IDC_CLOSE , &Edges ) ;

  Edges.top = LogPosition.y ;
  Edges.bottom = Edges.top + ( int ) ( LogSize.cy * 0.34 ) ;
  SetNewRectPosition( OriginalDlgClientRect , IDC_VIEW_SETUP , &Edges ) ;
  Edges.top = LogPosition.y + ( int ) ( LogSize.cy * 0.35 ) ;
  Edges.bottom = Edges.top + ( int ) ( LogSize.cy * 0.34 ) ;
  SetNewRectPosition( OriginalDlgClientRect , IDC_VIEW_GRAPH , &Edges ) ;
  Edges.top = LogPosition.y + ( int ) ( LogSize.cy * 0.7 ) ;
  Edges.bottom = Edges.top + ( int ) ( LogSize.cy * 0.34 ) ;
  SetNewRectPosition( OriginalDlgClientRect , IDC_SAVE_GRAPH , &Edges ) ;

  // Technician
  Edges.top = LogPosition.y - 5 ;
  Edges.bottom = Edges.top + ( int ) ( LogSize.cy * 0.4 ) ;
  GetDlgItem( IDC_TECHNICIAN )->SendMessage( WM_SETFONT , ( WPARAM ) m_TechnicianFont.GetSafeHandle() )  ;
  SetCheck( IDC_TECHNICIAN , true ) ;
  ShowCalibrationControls( TRUE ) ;
  SetNewRectPosition( OriginalDlgClientRect , IDC_TECHNICIAN , &Edges ) ;

  GetDlgItem( IDC_VIEW_SETUP )->SendMessage( WM_SETFONT , ( WPARAM ) m_ButtonsFont.GetSafeHandle() )  ;
  GetDlgItem( IDC_VIEW_GRAPH )->SendMessage( WM_SETFONT , ( WPARAM ) m_ButtonsFont.GetSafeHandle() )  ;
  GetDlgItem( IDC_SAVE_GRAPH )->SendMessage( WM_SETFONT , ( WPARAM ) m_ButtonsFont.GetSafeHandle() )  ;

  int iNRenderCount = 0 ;
  for ( size_t Row = 0 ; Row < m_uNRows ; Row++ )
  {
    size_t uNColumns = m_NColumnsPerRow[ Row ] ;
    for ( size_t Column = 0 ; Column < uNColumns ; Column++ , iNRenderCount++ )
    {
      CWnd * pNewRenderer = new CWnd ;
      CPoint RenderLT(
        ROUND( m_RenderPositions[ iNRenderCount ].real() * DlgClientArea.Width() ) ,
        ROUND( m_RenderPositions[ iNRenderCount ].imag() * DlgClientArea.Height() ) ) ;
      CSize RenderSize(
        ROUND( m_RenderSizes[ iNRenderCount ].real() * DlgClientArea.Width() ) ,
        ROUND( m_RenderSizes[ iNRenderCount ].imag() * DlgClientArea.Height() ) ) ;
      CRect RendererRect( RenderLT , RenderSize ) ;
      pNewRenderer->Create( _T( "STATIC" ) , "" ,
        WS_CHILD | WS_VISIBLE | WS_BORDER ,
        RendererRect , this , ( UINT ) ( 10000 * ( Row + 1 ) + Column ) ) ;
      pNewRenderer->ShowWindow( SW_SHOW ) ;
      m_Renderers.push_back( pNewRenderer ) ;

      BOOL bRes = m_Graph.ConnectRendererToWindow( pNewRenderer ,
        m_OnScreenRenderNames[ iNRenderCount ].c_str() ,
        m_RenderNames[ iNRenderCount ].c_str() ) ;

      ASSERT( bRes ) ;

      RendererRect.top -= 20 ;
      RendererRect.bottom = RendererRect.top + 19 ;
      pNewRenderer = new CWnd ;
      pNewRenderer->Create( _T( "STATIC" ) ,
        m_OnScreenRenderNames[ iNRenderCount ].c_str() ,
        WS_CHILD | WS_VISIBLE ,
        RendererRect , this , ( UINT ) ( 10000 * ( Row + 1 ) + Column ) ) ;
      pNewRenderer->ShowWindow( SW_SHOW ) ;
      m_Statics.push_back( pNewRenderer ) ;
    }
  }


  if ( m_ConfigName == "SkewMeter" )   // Set controls for Skew Meter
  {
    m_hIcon = LoadIcon( AfxGetInstanceHandle() , MAKEINTRESOURCE( IDI_FLOWERS ) );
//     m_hIcon = LoadIcon( AfxGetInstanceHandle() , MAKEINTRESOURCE( IDI_SKEW_METER ) );
    SetIcon( m_hIcon , TRUE );			// Set big icon
    GetDlgItem( IDC_HORIZ_DIST )->SendMessage( WM_SETFONT , ( WPARAM ) m_LogFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_VERT_DIST )->SendMessage( WM_SETFONT , ( WPARAM ) m_LogFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_SKEW_CALIB_UM )->SendMessage( WM_SETFONT , ( WPARAM ) m_LogFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_STATIC_HOR_DIST )->SendMessage( WM_SETFONT , ( WPARAM ) m_LogFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_STATIC_VERT_DIST )->SendMessage( WM_SETFONT , ( WPARAM ) m_LogFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_STATIC_SKEW_CALIB )->SendMessage( WM_SETFONT , ( WPARAM ) m_LogFont.GetSafeHandle() )  ;
    Edges.top = LogPosition.y /*+ ( int ) ( LogSize.cy * 0.15 )*/ ;
    Edges.bottom = Edges.top + ( int ) ( LogSize.cy * 0.3 ) ;
    SetNewRectPosition( OriginalDlgClientRect , IDC_STATIC_HOR_DIST , &Edges ) ;

    Edges.top = LogPosition.y + ( int ) ( LogSize.cy * 0.35 );
    Edges.bottom = Edges.top + ( int ) ( LogSize.cy * 0.3 ) ;
    SetNewRectPosition( OriginalDlgClientRect , IDC_STATIC_VERT_DIST , &Edges ) ;

    Edges.top = LogPosition.y + ( int ) ( LogSize.cy * 0.7 );
    Edges.bottom = Edges.top + ( int ) ( LogSize.cy * 0.3 ) ;
    SetNewRectPosition( OriginalDlgClientRect , IDC_STATIC_SKEW_CALIB , &Edges ) ;

    Edges.top = LogPosition.y /*+ ( int ) ( LogSize.cy * 0.1 )*/ ;
    Edges.bottom = Edges.top + ( int ) ( LogSize.cy * 0.3 ) ;
    SetNewRectPosition( OriginalDlgClientRect , IDC_HORIZ_DIST , &Edges ) ;

    Edges.top = LogPosition.y + ( int ) ( LogSize.cy * 0.35 ) ;
    Edges.bottom = Edges.top + ( int ) ( LogSize.cy * 0.3 ) ;
    SetNewRectPosition( OriginalDlgClientRect , IDC_VERT_DIST , &Edges ) ;

    Edges.top = LogPosition.y + ( int ) ( LogSize.cy * 0.7 ) ;
    Edges.bottom = Edges.top + ( int ) ( LogSize.cy * 0.3 ) ;
    SetNewRectPosition( OriginalDlgClientRect , IDC_SKEW_CALIB_UM , &Edges ) ;
  }
  else if ( m_ConfigName == "Tecto" ) // Set controls for Tecto
  {
    m_AppMode = AM_Tecto ;
    CString Caption = Reg.GetRegiString( m_ConfigName , "Caption" , "Tecto IMaging Engine" ) ;
    Caption += ". Ver ";
    Caption += __TIMESTAMP__ ;
    SetWindowText( Caption ) ;

    m_hIcon = LoadIcon( AfxGetInstanceHandle() , MAKEINTRESOURCE( IDI_FLOWERS ) );
    SetIcon( m_hIcon , TRUE );			// Set big icon
    
    CRect rcViewSetup , rcTechnician , rcViewGraph , rcr ;
    GetDlgItem( IDC_VIEW_SETUP )->GetWindowRect( &rcViewSetup ) ;
    ScreenToClient( &rcViewSetup ) ;
    GetDlgItem( IDC_VIEW_GRAPH )->GetWindowRect( &rcViewGraph ) ;
    ScreenToClient( &rcViewGraph ) ;
    GetDlgItem( IDC_TECHNICIAN )->GetWindowRect( &rcTechnician ) ;
    ScreenToClient( &rcTechnician ) ;

    GetDlgItem( IDC_SIDE_PROC_SETUP )->SetWindowPos( NULL ,
      rcr.left = rcTechnician.right - rcViewSetup.Width() ,
      rcr.top = rcTechnician.top - 3 * rcViewSetup.Height() ,
      rcViewSetup.Width() , rcViewSetup.Height() , SWP_SHOWWINDOW ) ;
    rcr.right = rcr.left + rcViewSetup.Width() ;
    rcr.bottom = rcr.top + rcViewSetup.Height() ;
    GetDlgItem( IDC_FRONT_PROC_SETUP )->SetWindowPos( NULL ,
      rcr.left , rcr.top + rcViewSetup.Height() + 6 ,
      rcViewSetup.Width() , rcViewSetup.Height() , SWP_SHOWWINDOW ) ;


    CRect rcSideProcSetup , rcFrontSetup , rcSaveToReg , rcGetNext ;
//     Edges.top = LogPosition.y ;
//     Edges.bottom = Edges.top + ( int ) ( LogSize.cy * 0.32 ) ;
//     SetNewRectPosition( OriginalDlgClientRect , IDC_SIDE_PROC_SETUP , &Edges ) ;
//     Edges.top = LogPosition.y + ( int ) ( LogSize.cy * 0.35 ) ;
//     Edges.bottom = Edges.top + ( int ) ( LogSize.cy * 0.32 ) ;
//     SetNewRectPosition( OriginalDlgClientRect , IDC_FRONT_PROC_SETUP , &Edges ) ;
    Edges.top = LogPosition.y ;
    Edges.bottom = Edges.top + ( int ) ( LogSize.cy * 0.32 ) ;
    SetNewRectPosition( OriginalDlgClientRect , IDC_SAVE_TO_REGISTRY , &Edges ) ;

    GetDlgItem( IDC_SIDE_PROC_SETUP )->GetWindowRect( &rcSideProcSetup ) ;
    ScreenToClient( &rcSideProcSetup ) ;
    GetDlgItem( IDC_GET_NEXT )->SetWindowPos( NULL ,
      rcSideProcSetup.left ,
      rcSideProcSetup.top - rcSideProcSetup.Height() - 6 ,
      rcSideProcSetup.Width() , rcSideProcSetup.Height() , SWP_SHOWWINDOW ) ;

    GetDlgItem( IDC_GET_NEXT )->GetWindowRect( &rcGetNext ) ;
    ScreenToClient( &rcGetNext ) ;
    GetDlgItem(  IDC_SAVE_RESULT)->SetWindowPos( NULL ,
      rcGetNext.left ,
      rcGetNext.top - rcGetNext.Height() - 10 ,
      rcGetNext.Width() , rcGetNext.Height() , SWP_SHOWWINDOW ) ;
    GetDlgItem( IDC_SAVE_SRC )->SetWindowPos( NULL ,
      rcGetNext.left ,
      rcGetNext.top - (rcGetNext.Height() + 10) * 2 ,
      (rcGetNext.Width()*4)/5 , rcGetNext.Height()  , SWP_SHOWWINDOW ) ;

    GetDlgItem( IDC_CAM1_STATUS )->SetWindowPos( NULL ,
      rcGetNext.left ,
      rcGetNext.top - ( rcGetNext.Height() + 10 ) * 3 ,
      rcGetNext.Width()/3 , rcGetNext.Width() / 3 , SWP_SHOWWINDOW ) ;
    GetDlgItem( IDC_CAM2_STATUS )->SetWindowPos( NULL ,
      rcGetNext.right - rcGetNext.Width() / 3 ,
      rcGetNext.top - ( rcGetNext.Height() + 10 ) * 3 ,
      rcGetNext.Width() / 3 , rcGetNext.Width() / 3 , SWP_SHOWWINDOW ) ;
    GetDlgItem( IDC_BATCH )->SetWindowPos( NULL ,
      rcGetNext.left ,
      rcGetNext.top - ( rcGetNext.Height() + 10 ) * 4 ,
      ( rcGetNext.Width() * 4 ) / 5 , rcGetNext.Height() , SWP_SHOWWINDOW ) ;

    CRect rcUpperView , rcLowerView ;
    m_Renderers[ 0 ]->GetWindowRect( rcUpperView ) ;
    ScreenToClient( &rcUpperView ) ;
    m_Renderers[ 1 ]->GetWindowRect( rcLowerView ) ;
    ScreenToClient( rcLowerView ) ;

    GetDlgItem( IDC_RESULT1 )->SetWindowPos( NULL ,
      rcGetNext.left , rcUpperView.top , rcGetNext.Width() , ( rcGetNext.Height() * 2 ) / 3 , SWP_SHOWWINDOW ) ;
    GetDlgItem( IDC_SRC1 )->SetWindowPos( NULL ,
      rcGetNext.left , rcUpperView.top + rcGetNext.Height() ,
      rcGetNext.Width() , ( rcGetNext.Height() * 2 ) / 3 , SWP_SHOWWINDOW ) ;

    GetDlgItem( IDC_RESULT2 )->SetWindowPos( NULL ,
      rcGetNext.left , rcLowerView.top , rcGetNext.Width() , ( rcGetNext.Height() * 2 ) / 3 , SWP_SHOWWINDOW ) ;
    GetDlgItem( IDC_SRC2 )->SetWindowPos( NULL ,
      rcGetNext.left , rcLowerView.top + rcGetNext.Height() ,
      rcGetNext.Width() , ( rcGetNext.Height() * 2 ) / 3 , SWP_SHOWWINDOW ) ;
    GetDlgItem( IDC_RESULT1_ON_2 )->SetWindowPos( NULL ,
      rcGetNext.left , rcLowerView.top + (rcGetNext.Height() * 2) ,
      rcGetNext.Width() , ( rcGetNext.Height() * 2 ) / 3 , SWP_SHOWWINDOW ) ;

    GetDlgItem( IDC_FRONT_PROC_SETUP )->GetWindowRect( &rcFrontSetup ) ;
    ScreenToClient( &rcFrontSetup ) ;

    GetDlgItem( IDC_SAVE_TO_REGISTRY )->SetWindowPos( NULL ,
      rcViewSetup.left - rcViewSetup.Width()  - 10 , 
      rcViewSetup.top , rcViewSetup.Width() , rcViewSetup.Height() , SWP_SHOWWINDOW ) ;

    GetDlgItem( IDC_SET_FLOWER )->SetWindowPos( NULL ,
      rcr.left = rcViewSetup.left - rcViewSetup.Width() - 10 , 
      rcViewGraph.top , rcViewGraph.Width() , rcViewGraph.Height() , SWP_SHOWWINDOW ) ;

    GetDlgItem( IDC_STATIC_FLOWER )->SetWindowPos( NULL ,
      rcr.left -= (rcViewSetup.Width() + 10) , 
      rcr.top = rcViewGraph.top + rcViewGraph.Height() + 10 , 
      rcViewGraph.Width() * 2 / 3 , 
      rcViewGraph.Height() * 2 / 3, SWP_SHOWWINDOW ) ;
    rcr.right = rcr.left + (rcViewGraph.Width() * 2 / 3) + 4 ;
    GetDlgItem( IDC_FLOWER_NAME )->SetWindowPos( NULL ,
      rcr.right , rcr.top ,
      rcViewGraph.Width() * 3/2, rcViewGraph.Height() * 2 / 3 , SWP_SHOWWINDOW ) ;

    GetDlgItem( IDC_STATIC_NSOURCES )->SetWindowPos( NULL ,
      rcr.left , rcViewSetup.top ,
      rcViewGraph.Width() , rcViewGraph.Height() * 2 / 3 , SWP_SHOWWINDOW ) ;

    GetDlgItem( IDC_NSOURCES )->SetWindowPos( NULL ,
      rcr.left /*+ rcViewGraph.Width() / 4*/ , 
      rcViewSetup.top + (rcViewGraph.Height() * 2 / 3) + 3 ,
      rcViewGraph.Width()/2 , rcViewGraph.Height() * 2 / 3 , SWP_SHOWWINDOW ) ;


    GetDlgItem( IDC_SIDE_PROC_SETUP )->SendMessage( WM_SETFONT , ( WPARAM ) m_ButtonsFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_FRONT_PROC_SETUP )->SendMessage( WM_SETFONT , ( WPARAM ) m_ButtonsFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_SAVE_TO_REGISTRY )->SendMessage( WM_SETFONT , ( WPARAM ) m_ButtonsFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_STATIC_FLOWER )->SendMessage( WM_SETFONT , ( WPARAM ) m_ButtonsFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_FLOWER_NAME )->SendMessage( WM_SETFONT , ( WPARAM ) m_ButtonsFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_SET_FLOWER )->SendMessage( WM_SETFONT , ( WPARAM ) m_ButtonsFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_STATIC_NSOURCES )->SendMessage( WM_SETFONT , ( WPARAM ) m_ButtonsFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_NSOURCES )->SendMessage( WM_SETFONT , ( WPARAM ) m_ButtonsFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_SAVE_RESULT )->SendMessage( WM_SETFONT , ( WPARAM ) m_ButtonsFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_SAVE_SRC )->SendMessage( WM_SETFONT , ( WPARAM ) m_ButtonsFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_RESULT1 )->SendMessage( WM_SETFONT , ( WPARAM ) m_ButtonsFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_SRC1 )->SendMessage( WM_SETFONT , ( WPARAM ) m_ButtonsFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_RESULT2 )->SendMessage( WM_SETFONT , ( WPARAM ) m_ButtonsFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_SRC2 )->SendMessage( WM_SETFONT , ( WPARAM ) m_ButtonsFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_RESULT1_ON_2 )->SendMessage( WM_SETFONT , ( WPARAM ) m_ButtonsFont.GetSafeHandle() )  ;
    GetDlgItem( IDC_BATCH )->SendMessage( WM_SETFONT , ( WPARAM ) m_ButtonsFont.GetSafeHandle() )  ;

    GetDlgItem( IDC_RESULT1 )->EnableWindow( 0 )  ;
    GetDlgItem( IDC_SRC1 )->EnableWindow( 0 )   ;
    GetDlgItem( IDC_RESULT2 )->EnableWindow( 0 )   ;
    GetDlgItem( IDC_SRC2 )->EnableWindow( 0 )   ;
    GetDlgItem( IDC_RESULT1_ON_2 )->EnableWindow( 0 ) ;

  }

  // Read saved values from registry

  m_iViewTimeIndex = Reg.GetRegiInt( m_ConfigName , "ViewTimeIndex" , 1 ) ;
  if ( ( 0 <= m_iViewTimeIndex ) 
    && ( m_iViewTimeIndex < ( FXSIZE ) m_Statics.size() ) )
  {
    CWnd * pCaption = m_Statics[ m_iViewTimeIndex ] ;
    CString Caption( m_OnScreenRenderNames[ m_iViewTimeIndex ].c_str() ) ;
    Caption += "       " ;
    Caption += GetTimeAsString( "Time: " ) ;
    m_dStartTime_ms = GetHRTickCount() ;

    CString RunTimeAsString ;
    double dRunTime_ms = GetHRTickCount() - m_dStartTime_ms ;
    __int64 i64RunTime_ms = ( __int64 ) dRunTime_ms ;
    RunTimeAsString.Format( "    Run Time: %02ddays %02dhr %02dmin %02d.%03dsec" ,
      ( int ) ( i64RunTime_ms / ( 24 * 3600000 ) ) , ( int ) ( i64RunTime_ms / 3600000 ) ,
      ( int ) ( i64RunTime_ms / 60000 ) , ( int ) ( i64RunTime_ms / 1000 ) ,
      ( int ) ( i64RunTime_ms % 1000 ) ) ;
    Caption += RunTimeAsString ;
    pCaption->SetWindowText( Caption ) ;
  }

  m_Graph.StartGraph();

  SetTimer( EvDrawTimer , 200 , NULL ) ;
  m_LogView.SetFont( &m_LogFont ) ;
  m_bAppClosing = FALSE ;

  if ( m_ConfigName == "Tecto" )
  {
    FXRegistry RegT( "TheFileX\\UI_NViews\\Tecto" );
    m_pSHMemReceiverThread = new std::thread( ReceiveFromShMemFunc , this ) ;
    m_CurrentFlowerName = RegT.GetRegiString( 
      "Common" , "LastFlowerName" , m_CurrentFlowerName ) ;
    m_iNSources = RegT.GetRegiInt( "Common" , "NSources" , 1 ) ;
#define MAX_NUM_OF_CAMERAS 4
    for ( int i = 0 ; i < m_iNSources ; i++ )
    {
      CString TectoGadgetName ;
      TectoGadgetName.Format( "Tecto%d" , i + 1 ) ;
      if ( !m_Graph.SetProperty( TectoGadgetName , "FlowerName" , m_CurrentFlowerName ) )
        break ; // no more Tecto gadgets
    }
    SetDlgItemText( IDC_FLOWER_NAME , m_CurrentFlowerName ) ;
    SetDlgItemInt( IDC_NSOURCES , m_iNSources ) ;
    SetFlowerAndDirectories( m_iNSources ) ;
    if ( !m_Graph.SetTextCallBack( "Tecto1>>1" , ResultForTectoCallBack , this ) )
    {
      char Msg[ 100 ] ;
      sprintf( Msg , "Can't assign callback from 'Tecto1>>1' pin" ) ;
      m_LogView.InsertString( 0 , Msg ) ;
    }

  }
  else if ( m_ConfigName == "SkewMeter" )
  {
    m_dHorizontalLineDist_mm = Reg.GetRegiDouble( m_ConfigName ,
      "dHorizDist_mm" , m_dHorizontalLineDist_mm ) ;
    m_dVerticalLineDist_mm = Reg.GetRegiDouble( m_ConfigName ,
      "dVertDist_mm" , m_dVerticalLineDist_mm ) ;
    m_dDistAllowedVariation_mm = Reg.GetRegiDouble( m_ConfigName ,
      "dDistVariation_mm" , m_dDistAllowedVariation_mm ) ;
    m_dHorizontalLineDistNominal_mm = Reg.GetRegiDouble( m_ConfigName ,
      "dHorizDist_mm" , m_dHorizontalLineDistNominal_mm ) ;
    m_dVerticalLineDistNominal_mm = Reg.GetRegiDouble( m_ConfigName ,
      "dVertDist_mm" , m_dVerticalLineDistNominal_mm ) ;
    m_dSkewCalib_um = Reg.GetRegiDouble( m_ConfigName ,
      "dSkewCalib_um" , m_dSkewCalib_um ) ;
    // Write default values to graph
    m_Graph.SetProperty( "SkewMeterLU_LEL" ,
      "BaseVertDist_mm" , m_dVerticalLineDist_mm ) ;
    m_Graph.SetProperty( "SkewMeterLU_LEL" ,
      "BaseHorDist_mm" , m_dHorizontalLineDist_mm ) ;
    m_Graph.SetProperty( "SkewMeterLU_LEL" ,
      "DistCorrectionB" , m_dSkewCalib_um ) ;

    CString AsText ;
    AsText.Format( "%.03f" , m_dHorizontalLineDist_mm ) ;
    SetDlgItemText( IDC_HORIZ_DIST , AsText ) ;
    AsText.Format( "%.03f" , m_dVerticalLineDist_mm ) ;
    SetDlgItemText( IDC_VERT_DIST , AsText ) ;
    AsText.Format( "%.03f" , m_dSkewCalib_um ) ;
    SetDlgItemText( IDC_SKEW_CALIB_UM , AsText ) ;
  }
  return TRUE;  // return TRUE  unless you set the focus to a control
}

void CUI_NViews::OnSysCommand( UINT nID , LPARAM lParam )
{
  if ( ( nID & 0xFFF0 ) == IDM_ABOUTBOX )
  {
    CAboutDlg dlgAbout;
    dlgAbout.DoModal();
  }
  else if ( nID == SC_CLOSE )
  {
    if ( CanExit() )
    {
      m_Graph.StopGraph() ;
      Sleep( 500 ) ;
      CDialogEx::OnClose();
      PostQuitMessage( 0 ) ;
    }
  }
  else
  {
    CDialogEx::OnSysCommand( nID , lParam );
  }
}

void CUI_NViews::OnPaint()
{
  if ( IsIconic() )
  {
    CPaintDC dc( this ); // device context for painting

    SendMessage( WM_ICONERASEBKGND , reinterpret_cast< WPARAM >( dc.GetSafeHdc() ) , 0 );

    // Center icon in client rectangle
    int cxIcon = GetSystemMetrics( SM_CXICON );
    int cyIcon = GetSystemMetrics( SM_CYICON );
    CRect rect;
    GetClientRect( &rect );
    int x = ( rect.Width() - cxIcon + 1 ) / 2;
    int y = ( rect.Height() - cyIcon + 1 ) / 2;

    // Draw the icon
    dc.DrawIcon( x , y , m_hIcon );
  }
  else
  {
    CDialogEx::OnPaint();
  }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CUI_NViews::OnQueryDragIcon()
{
  return static_cast< HCURSOR >( m_hIcon );
}

// Automation servers should not exit when a user closes the UI
//  if a controller still holds on to one of its objects.  These
//  message handlers make sure that if the proxy is still in use,
//  then the UI is hidden but the dialog remains around if it
//  is dismissed.

BOOL CUI_NViews::CanExit()
{
  // If the proxy object is still around, then the automation
  //  controller is still holding on to this application.  Leave
  //  the dialog around, but hide its UI.
  if ( m_pAutoProxy != nullptr )
  {
    ShowWindow( SW_HIDE );
    return FALSE;
  }
  m_bAppClosing = TRUE ;
  if ( m_pSHMemReceiverThread )
  {
    m_bFinishReceivengShMemMsgs = true ;
    SetEvent( m_pShMemControl->GetInEventHandle() ) ;
    m_pSHMemReceiverThread->join() ;
    delete m_pSHMemReceiverThread ;
    m_pSHMemReceiverThread = NULL ;
  }
  if ( m_AppMode == AM_Tecto )
  {
    m_Graph.SendText( "Tecto1<<0" , "disable" , "process" ) ;
    m_Graph.SendText( "Tecto2<<0" , "disable" , "process" ) ;
    Sleep( 100 ) ;
  }

  CRect MonitorRect( 0 , 0 , 0 , 0 ) ;
  m_iMonitorNumber = 0 ;
  CRect rcDialogPos ;
  GetWindowRect( &rcDialogPos ) ;
  const CSystemMonitorsEnumerator * pEnumerator =
    CSystemMonitorsEnumerator::GetMonitorEnums() ;
  do
  {
    MonitorRect = pEnumerator->GetMonitorRect( m_iMonitorNumber ) ;
    if ( MonitorRect.Width() == 0 )
    {             // Did not find monitor, set monitor 0 near left top corner
      FXRegistry Reg( "TheFileX\\UI_NViews" );
      cmplx cLTNorm( 0.05 , 0.05 ) ;
      Reg.WriteRegiInt( m_ConfigName , "Monitor#ForView" , 0 ) ;
      Reg.WriteRegiCmplx( m_ConfigName , "DialogLTPos" , cLTNorm ) ;
      break ;
    }
    if ( MonitorRect.PtInRect( rcDialogPos.TopLeft() ) )
    {
      cmplx cLTNorm( ( double ) ( rcDialogPos.left - MonitorRect.left ) / ( double ) MonitorRect.Width() ,
        ( double ) ( rcDialogPos.top - MonitorRect.top ) / ( double ) MonitorRect.Height() ) ;
      FXRegistry Reg( "TheFileX\\UI_NViews" );
      Reg.WriteRegiCmplx( m_ConfigName , "DialogLTPos" , cLTNorm ) ;
      Reg.WriteRegiInt( m_ConfigName , "Monitor#ForView" , m_iMonitorNumber ) ;
      break ;
    }
  } while ( ++m_iMonitorNumber < ( int ) pEnumerator->m_rcMonitors.size() );


  return TRUE;
}


void CUI_NViews::LoadPresentationParamsFromRegistry()
{
  FXRegistry Reg( "TheFileX\\UI_NViews" );

  m_ConfigName = Reg.GetRegiString( "Configuration" , "ConfigName" ,
    "SkewMeter" ) ;
  m_GraphName = Reg.GetRegiString( m_ConfigName , "GraphName" ,
    "S:/Dev/SH_20231003/Sources/FileXGadgets/FileX/res/SkewMeterBMP_Station3.tvg" ) ;
  m_uNRows = Reg.GetRegiInt( m_ConfigName , "NRowsOfImages" , 2 );
  m_uNDefaultColumnsPerRow = Reg.GetRegiInt( m_ConfigName , "NRowsOfImages" , 2 );
  m_iMonitorNumber = Reg.GetRegiInt( m_ConfigName , "Monitor#ForView" , 0 ) ;
  Reg.GetRegiCmplx( m_ConfigName , "DialogRelSize" , m_csDialogSize ,
    cmplx( 0.8 , 0.9 ) ) ;
  m_dRelLogHeight = Reg.GetRegiDouble( m_ConfigName , "RelLogHeight" , 0.1 ) ;
  m_dRelLogWidth = Reg.GetRegiDouble( m_ConfigName , "RelLogWidth" , 1.0 ) ;
  cmplx csDefaultRenderSize(
    ( ( 1. - ( ( m_uNDefaultColumnsPerRow + 1 ) * X_REL_SPACE_BETWEEN_RENDERERS ) ) / m_uNDefaultColumnsPerRow ) ,
    ( ( 1. - m_dRelLogHeight - ( ( m_uNRows + 1 ) * Y_REL_SPACE_BETWEEN_RENDERERS ) ) / m_uNRows ) ) ;
  for ( size_t i = 0 ; i < m_uNRows ; i++ )
  {
    FXString RegName ;
    RegName.Format( "NColumns%d" , i + 1 ) ;
    int iNColumnsInRow = Reg.GetRegiInt( m_ConfigName , RegName , ( int& ) m_uNDefaultColumnsPerRow );
    m_NColumnsPerRow.push_back( iNColumnsInRow ) ;
    for ( int j = 0 ; j < iNColumnsInRow ; j++ )
    {
      RegName.Format( "Renderer_%d_%d" , i + 1 , j + 1 ) ;
      string RendererName = Reg.GetRegiString( m_ConfigName , RegName , RegName ) ;
      m_RenderNames.push_back( RendererName ) ;
      RegName.Format( "OnScreenRendererName_%d_%d" , i + 1 , j + 1 ) ;
      string OnScreenRendererName = Reg.GetRegiString( m_ConfigName , RegName , RegName ) ;
      m_OnScreenRenderNames.push_back( OnScreenRendererName ) ;
      RegName.Format( "RenderRelPos_%d_%d" , i + 1 , j + 1 ) ;
      cmplx RenderRelPos(
        X_REL_SPACE_BETWEEN_RENDERERS
        + ( csDefaultRenderSize.real() + X_REL_SPACE_BETWEEN_RENDERERS ) * j ,
        Y_REL_SPACE_BETWEEN_RENDERERS
        + ( csDefaultRenderSize.imag() + Y_REL_SPACE_BETWEEN_RENDERERS ) * i ) ;
      Reg.GetRegiCmplx( m_ConfigName , RegName , RenderRelPos ,
        RenderRelPos ) ;
      m_RenderPositions.push_back( RenderRelPos ) ;

      RegName.Format( "RenderRelSize_%d_%d" , i + 1 , j + 1 ) ;
      cmplx RenderRelSize ;
      Reg.GetRegiCmplx( m_ConfigName , RegName , RenderRelSize ,
        csDefaultRenderSize ) ;
      m_RenderSizes.push_back( RenderRelSize ) ;
    }
  }
  int iFontSize_pts = Reg.GetRegiInt( m_ConfigName , "LogFontSizePts" , 10 );
  m_LogFont.CreatePointFont( iFontSize_pts , "System" ) ;
  iFontSize_pts = Reg.GetRegiInt( m_ConfigName , "ButtonsFontSizePts" , 8 );
  m_ButtonsFont.CreatePointFont( iFontSize_pts , "Arial" ) ;
  m_TechnicianFont.CreatePointFont( iFontSize_pts , "Arial" ) ;
  FXString NotShowErrorsAsString = Reg.GetRegiString( m_ConfigName , "NotShowErrors" , "31" ) ;
  m_NotShowErrors.resize( 30 ) ;
  int NValues = Reg.GetRegiIntSerie( m_ConfigName , "NotShowErrors" ,
    m_NotShowErrors.data() , ( int ) m_NotShowErrors.size() ) ;
  m_NotShowErrors.resize( NValues ) ;
  m_iNRemovedReset = Reg.GetRegiInt( m_ConfigName , "NRemovedSilentlyMesages" , 400 ) ;
  m_bEnableGraphSave = Reg.GetRegiInt( m_ConfigName , "GraphSaveEnable" , FALSE ) ;

  if ( m_ConfigName == "Tecto" )
  {
    m_ShMemAreaName = Reg.GetRegiString( m_ConfigName , "ShMemAreaName" , "TectoSharedArea" ) ;
    m_InEventName = Reg.GetRegiString( m_ConfigName , "InEventName" , "TectoInEvent" ) ;
    m_OutEventName = Reg.GetRegiString( m_ConfigName , "OutEventName" , "TectoOutEvent" ) ;
    m_iShMemInSize_KB = Reg.GetRegiInt( m_ConfigName , "ShMemInSize_KB" , 4096 ) ;
    m_iShMemOutSize_KB = Reg.GetRegiInt( m_ConfigName , "ShMemOutSize_KB" , 4096 ) ;
    if ( m_pShMemControl )
    {
      delete m_pShMemControl ;
      m_pShMemControl = NULL ;
    }
    m_pShMemControl = new CShMemControl( 1024 , 1024 + m_iShMemOutSize_KB * 1024 ,
      1024 + ( 1024 * ( m_iShMemInSize_KB + m_iShMemOutSize_KB ) ) , m_ShMemAreaName ,
      m_InEventName , m_OutEventName ) ;

    CString Msg ;
    if ( !m_pShMemControl )
      Msg.Format( "Can't allocate shared memory. Size In=%dKB Out=%dKB" , m_iShMemInSize_KB , m_iShMemOutSize_KB ) ;
    else
      Msg.Format( "Shared memory allocated. Size In=%dKB Out=%dKB" , m_iShMemInSize_KB , m_iShMemOutSize_KB ) ;

    m_LogView.InsertString( 0 , Msg ) ;
  }


}

void	CUI_NViews::PrintMessage( LPCTSTR message )
{
  if ( !m_bAppClosing )
  {
    m_LastErrorMsg = message ;
    if ( m_LastErrorMsg.Find( "Expired" ) >= 0 )
      m_EvaluationMsg = m_LastErrorMsg ;
    FXAutolock Lock( m_Protect , "CUI_NViews::PrintMessage" ) ;
    ASSERT( m_Protect.IsLockedByThisThread() ) ;
    m_Message.Add( message );
    m_bPrintMessage = true;
    PostMessage( USER_WM_LOG_MSG ) ;
  }
}

LRESULT CUI_NViews::OnLogMsg( WPARAM wParam , LPARAM lParam )
{
  if ( m_bPrintMessage )
  {
    m_Protect.Lock() ;
    while ( m_LogView.GetCount() > 100 )
      m_LogView.DeleteString( m_LogView.GetCount() - 1 ) ;
    while ( m_Message.GetCount() )
    {
      bool bPrint = true ;
      CString Msg = m_Message[ m_Message.GetUpperBound() ] ;
      int iOpenPos = Msg.ReverseFind( '(' ) ;
      if ( iOpenPos > 0 )
      {
        int iClosePos = Msg.Find( ')' , iOpenPos + 1 ) ;
        int iInside = iClosePos - iOpenPos - 1 ;
        if ( ( 0 < iInside ) && ( iInside < 3 ) )
        {
          int iErrorNum = atoi( ( LPCTSTR ) Msg + iOpenPos + 1 ) ;
          for ( size_t i = 0 ; i < m_NotShowErrors.size() ; i++ )
          {
            if ( iErrorNum == m_NotShowErrors[ i ] )
            {
              if ( m_iNRemovedReset > 0 )
              {
                if ( ++m_iNRemoved >= m_iNRemovedReset )
                {
                  CString RemovedMsg ;
                  RemovedMsg.Format( "%d messages removed" , m_iNRemoved ) ;
                  m_iNRemoved = 0 ;
                  int iLen ;
                  while ( ( iLen = RemovedMsg.GetLength() ) &&
                    ( ( RemovedMsg[ iLen - 1 ] == '\n' )
                    || ( RemovedMsg[ iLen - 1 ] == '\r' ) ) )
                  {
                    RemovedMsg.Delete( iLen - 1 ) ;
                  }
                  m_LogView.InsertString( 0 , RemovedMsg /*+ "\r\n"*/ ) ;
                  FXRegistry Reg( "TheFileX\\UI_NViews" );

                  FXString ConfigName = Reg.GetRegiString( "Configuration" , "ConfigName" ,
                    "SkewMeter" ) ;
                  m_iNRemovedReset = Reg.GetRegiInt( ConfigName , "NRemovedSilentlyMesages" , 400 ) ;
                }
                else
                  bPrint = false ;
              }
              break ;
            }
          }
        }
      }
      if ( bPrint )
      {
        int iLen ;
        while ( ( iLen = Msg.GetLength() ) && ( ( Msg[ iLen - 1 ] == '\n' )
          || ( Msg[ iLen - 1 ] == '\r' ) ) )
        {
          Msg.Delete( iLen - 1 ) ;
        }
        m_LogView.InsertString( 0 , Msg /*+ "\r\n" */ ) ;
      }
      m_Message.RemoveAt( m_Message.GetCount() - 1 ) ;
    }
    // SetDlgItemText( IDC_STATUS , m_Message ) ;
    m_bPrintMessage = false ;
    m_Protect.Unlock() ;
  }
  //     double dCurrentTime = m_Graph.GetTime() ;
  //     FXString FPS ;
  //     FPS.Format( "%5.1f FPS" ,   
  //       (  ( (m_dLastFrameTime > m_dLastViewFPS) 
  //       || (( dCurrentTime - m_dLastFrameTime ) < 1000.) )
  //       && ( m_dLastFrameInterval > 1.) ) ? 
  //       1000. / m_dLastFrameInterval : 0 );
  //     SetDlgItemText( IDC_FPS_VIEW , FPS ) ;
  //     m_dLastViewFPS = dCurrentTime ;
  return 0;
}

HBRUSH CUI_NViews::OnCtlColor( CDC* pDC , CWnd *pWnd , UINT nCtlColor )
{
  switch ( nCtlColor )
  {
    case CTLCOLOR_STATIC:
      if ( pWnd == GetDlgItem( IDC_SIDE_VIEW ) )
      {
  //       pDC->SetTextColor( m_SideResultView.MTextColor );
  //       pDC->SetBkColor( m_SideResultView.MBackgroundColor ) ;
  //       pDC->SetBkMode( OPAQUE );
        return ( HBRUSH ) GetStockObject( NULL_BRUSH );
      }
      else if ( pWnd == GetDlgItem( IDC_FRONT_VIEW ) )
      {
  //       pDC->SetTextColor( m_FrontResultView.MTextColor );
  //       pDC->SetBkColor( m_FrontResultView.MBackgroundColor ) ;
  //       pDC->SetBkMode( OPAQUE );
        return ( HBRUSH ) GetStockObject( NULL_BRUSH );
      }
    default:
      return CDialog::OnCtlColor( pDC , pWnd , nCtlColor );
  }
}

void CUI_NViews::OnTimer( UINT_PTR nIDEvent )
{
  switch ( nIDEvent )
  {
    case EvDrawTimer:
    {
      if ( m_ConfigName == "SkewMeter" )
      {
        if ( m_bCam1CallBackPassed != m_bLastDrawnCam1 )
        {
          m_bLastDrawnCam1 = m_bCam1CallBackPassed ;
          m_Cam1StatusLED.SetBitmap( m_bLastResult1 ? m_GreenLed : m_RedLed ) ;
        }
        else
          m_Cam1StatusLED.SetBitmap( m_BlackLed ) ;

        if ( m_bCam2CallBackPassed != m_bLastDrawnCam2 )
        {
          m_bLastDrawnCam2 = m_bCam2CallBackPassed ;
          m_Cam2StatusLED.SetBitmap( m_bLastResult2 ? m_GreenLed : m_RedLed ) ;
        }
        else
          m_Cam1StatusLED.SetBitmap( m_BlackLed ) ;

        m_bCam1CallBackPassed = m_bCam2CallBackPassed = false ;
      }
      else if ( m_ConfigName == "Tecto" )
      {
        if ( m_iRestToViewCam1 > 0 )
        {
          m_Cam1StatusLED.SetBitmap( m_bLastResult1 ? m_GreenLed : m_RedLed ) ;
          m_iRestToViewCam1-- ;
        }
        else
          m_Cam1StatusLED.SetBitmap( m_BlackLed ) ;

        if ( m_iRestToViewCam2 )
        {
          m_Cam2StatusLED.SetBitmap( m_bLastResult2 ? m_GreenLed : m_RedLed ) ;
          m_iRestToViewCam2-- ;
        }
        else
          m_Cam2StatusLED.SetBitmap( m_BlackLed ) ;
      }
    } ;
    if ( ( 0 <= m_iViewTimeIndex ) && ( m_iViewTimeIndex < ( FXSIZE ) m_Statics.size() ) )
    {
      CWnd * pCaption = m_Statics[ m_iViewTimeIndex ] ;
      CString Caption( m_OnScreenRenderNames[ m_iViewTimeIndex ].c_str() ) ;
      Caption += "             " ;
      Caption += GetTimeAsString( "Time: " ) ;

      CString RunTimeAsString ;
      double dRunTime_ms = GetHRTickCount() - m_dStartTime_ms ;
      __int64 i64RunTime_ms = ( __int64 ) dRunTime_ms ;
      RunTimeAsString.Format( " Run %dd %02d:%02d:%02d.%03d #(%d) OK(%d) Bad(%d) Empty(%d)" ,
        ( int ) ( i64RunTime_ms / ( 24 * 3600000 ) ) , ( int ) ( i64RunTime_ms / 3600000 ) % 24 ,
        ( int ) ( i64RunTime_ms / 60000 ) % 60 , ( int ) ( i64RunTime_ms / 1000 ) % 60 ,
        ( int ) ( i64RunTime_ms % 1000 ) ,
        m_iSideImagesWithFlowerCount , m_iGoodCount , m_iBadCount , m_iEmptyTrayCount ) ;
      Caption += RunTimeAsString ;

      DWORD PID = GetCurrentProcessId() ;
      PROCESS_MEMORY_COUNTERS Counters ;
      HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
        PROCESS_VM_READ ,
        FALSE , PID );

      if ( GetProcessMemoryInfo( hProcess , &Counters , sizeof( Counters ) ) )
      {
        CString Mem ;
        Mem.Format( "  Mem=%.2f" , (Counters.PeakWorkingSetSize * 1.e-6) ) ;
        Caption += Mem ;
      };
      CloseHandle( hProcess ) ;
      pCaption->SetWindowText( Caption ) ;
    }

    break ;
    case EvLockModeOn:
      GetDlgItem( IDC_WORK_MODE )->EnableWindow( FALSE ) ;
      KillTimer( EvLockModeOn ) ;
      SetCheck( IDC_PROTECT , false ) ;
      break ;
    case EvViewErrors:
    {
      FXPropertyKit Prop ;
    }
    break ;
    case EvWritingTimer:
    {
      int iRestFront = GetDlgItemInt( IDC_FRONT_VIDEO_REST_TIME );
      if ( ( iRestFront <= 1 ) && ( iRestFront >= 0 ) )
      {
        m_Graph.SendQuantity( "Switch1<>0" , false ); // Front write disable
        m_Graph.SendText( "FrontAvi<>0" , "close(1);" );
        SetDlgItemInt( IDC_FRONT_VIDEO_REST_TIME , iRestFront = -1 );
      }
      else
      {
        SetDlgItemInt( IDC_FRONT_VIDEO_REST_TIME ,
          ( iRestFront < 0 ) ? iRestFront = -1 : --iRestFront );
      }
      int iRestSide = GetDlgItemInt( IDC_SIDE_VIDEO_REST_TIME );
      if ( ( iRestSide <= 1 ) && ( iRestSide >= 0 ) )
      {
        m_Graph.SendQuantity( "Switch6<>0" , false ); // Side write disable
        m_Graph.SendText( "SideAvi<>0" , "close(1);" );
        SetDlgItemInt( IDC_SIDE_VIDEO_REST_TIME , iRestSide = -1 );
      }
      else
      {
        SetDlgItemInt( IDC_SIDE_VIDEO_REST_TIME ,
          ( iRestSide < 0 ) ? iRestSide = -1 : --iRestSide );

      }
      if ( ( iRestSide < 0 ) && ( iRestFront < 0 ) )
      {
        KillTimer( EvWritingTimer );
        m_iWritingTimer = 0;
      }
    }
    break ;
    case EvRestoreRendering:
    {

      CRect DlgClientArea ;
      GetClientRect( &DlgClientArea ) ;
      for ( size_t iNRenderCount = 0 ;
        iNRenderCount < m_RenderNames.size() ; iNRenderCount++ )
      {
        BOOL bRes = m_Graph.ConnectRendererToWindow( m_Renderers[ iNRenderCount ] ,
          m_OnScreenRenderNames[ iNRenderCount ].c_str() ,
          m_RenderNames[ iNRenderCount ].c_str() ) ;
      }
      SetWindowPos( &wndTop , 0 , 0 , 0 , 0 , SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW ) ;
      KillTimer( EvRestoreRendering ) ;
    }
    break ;
    case EvGetNextImage:
      KillTimer( EvGetNextImage ) ;
      ReadOrRepeatImage( false ) ;
      break ;
  }
  CDialogEx::OnTimer( nIDEvent );
}

WM_Green CUI_NViews::GetWorkingMode()
{
  CString Caption ;
  GetDlgItemText( IDC_WORK_MODE , Caption ) ;
  if ( Caption.Find( "Free" ) >= 0 )
    return WM_Green_Measure ;
  else   if ( Caption.Find( "Lock" ) >= 0 )
    return WM_Green_Lock ;
  else   if ( Caption.Find( "Manual" ) >= 0 )
    return WM_Green_Manual ;
  return WM_Green_Unknown ;
}

void CUI_NViews::OnParentNotify( UINT message , LPARAM lParam )
{
  // this must be a message from debug window
  if ( ( message == WM_DESTROY )/* && ( ( HWND ) lParam == GetSafeHwnd() )*/ )
  {
    for ( size_t i = 0 ; i < m_Renderers.size() ; i++ )
    {
      m_Graph.ConnectRendererToWindow( NULL , NULL , m_RenderNames[ i ].c_str() ) ;
    }
    SetTimer( EvRestoreRendering , 1000 , NULL ) ;
  }
  else
    CDialog::OnParentNotify( message , lParam );
}

void CUI_NViews::OnBnClickedSaveGraph()
{
  m_Graph.SaveGraph() ;
}

void CUI_NViews::OnBnClickedClose()
{
  if ( CanExit() )
  {
/*
    CRect MonitorRect = CSystemMonitorsEnumerator::GetMonitorEnums()
      ->GetMonitorRect(m_iMonitorNumber);

    CSize DlgWindowSize(ROUND(MonitorRect.Width() * m_csDialogSize.real()),
      ROUND(MonitorRect.Height() * m_csDialogSize.imag()));

    FXRegistry Reg("TheFileX\\UI_NViews");
    cmplx cNormWinPosLT;
    Reg.GetRegiCmplx(m_ConfigName, "DialogLTPos", cNormWinPosLT, cmplx(0.05, 0.05));
    */
    m_Graph.StopGraph() ;
    Sleep( 500 ) ;

    CDialogEx::OnClose();
    PostQuitMessage( 0 ) ;
  }
}

void CUI_NViews::OnBnClickedViewSetup()
{
  m_Graph.ViewSetup( this ) ;
}

void CUI_NViews::OnBnClickedViewGraph()
{
  m_Graph.ViewGraph( this ) ;
}



CRect CUI_NViews::SetNewRectPosition( CRect OriginalDlgClientRect , DWORD ID ,
  CRect * pEdges )
{
  CRect ControlRect , ScaledDlgRect ;
  GetClientRect( &ScaledDlgRect ) ;
  GetDlgItem( ID )->GetWindowRect( &ControlRect ) ;
  ScreenToClient( &ControlRect ) ;

  double dXRatio = ( double ) ScaledDlgRect.Width() / ( double ) OriginalDlgClientRect.Width() ;
  double dYRatio = ( double ) ScaledDlgRect.Height() / ( double ) OriginalDlgClientRect.Height() ;
  CRect NewCloseRect ;
  cmplx LT = GetCmplx( ControlRect.TopLeft() ) ;
  cmplx RB = GetCmplx( ControlRect.BottomRight() ) ;

  LT._Val[ _RE ] *= dXRatio ;
  LT._Val[ _IM ] *= dYRatio ;
  RB._Val[ _RE ] *= dXRatio ;
  RB._Val[ _IM ] *= dYRatio ;

  CRect NewRect( GetCPoint( LT ) , GetCPoint( RB ) ) ;
  int iNewWidth = NewRect.Width() ;
  int iNewHeight = NewRect.Height() ;
  if ( pEdges )
  {
    if ( ( pEdges->left >= 0 ) && ( NewRect.left < pEdges->left ) )
    {
      NewRect.left = pEdges->left ;
      NewRect.right = NewRect.left + iNewWidth ;
      if ( ( pEdges->right >= 0 ) && ( pEdges->Width() < NewRect.Width() ) )
        NewRect.right = pEdges->right ;
    }
    if ( ( pEdges->right >= 0 ) && ( NewRect.right < pEdges->right ) )
    {
      NewRect.right = pEdges->right ;
      NewRect.left = NewRect.right - iNewWidth ;
      if ( ( pEdges->left >= 0 ) && ( pEdges->Width() < NewRect.Width() ) )
        NewRect.left = pEdges->left ;
    }
    if ( ( pEdges->top >= 0 ) && ( NewRect.top < pEdges->top ) )
    {
      NewRect.top = pEdges->top ;
      NewRect.bottom = NewRect.top + iNewHeight ;
      if ( ( pEdges->bottom >= 0 ) && ( pEdges->Height() < NewRect.Height() ) )
        NewRect.bottom = pEdges->bottom ;
    }
    if ( ( pEdges->bottom >= 0 ) && ( NewRect.bottom > pEdges->bottom ) )
    {
      NewRect.bottom = pEdges->bottom ;
      NewRect.top = NewRect.bottom - iNewHeight ;
      if ( ( pEdges->top >= 0 ) && ( pEdges->Height() < NewRect.Height() ) )
        NewRect.top = pEdges->top ;
    }

  }

  GetDlgItem( ID )->SetWindowPos( NULL ,
    NewRect.left , NewRect.top , NewRect.Width() , NewRect.Height() , SWP_SHOWWINDOW ) ;

  return NewRect;
}

void CUI_NViews::OnEnKillFocusHorizDist()
{
  CString sHorizDist_mm ;
  if ( GetDlgItemText( IDC_HORIZ_DIST , sHorizDist_mm ) )
  {
    double dHorizDist_mm = atof( sHorizDist_mm ) ;
    if ( ( m_dHorizontalLineDistNominal_mm - m_dDistAllowedVariation_mm ) > dHorizDist_mm )
      dHorizDist_mm = ( m_dHorizontalLineDistNominal_mm - m_dDistAllowedVariation_mm ) ;
    if ( dHorizDist_mm > ( m_dHorizontalLineDistNominal_mm + m_dDistAllowedVariation_mm ) )
      dHorizDist_mm = ( m_dHorizontalLineDistNominal_mm + m_dDistAllowedVariation_mm ) ;
    m_dHorizontalLineDist_mm = dHorizDist_mm ;
    FXRegistry Reg( "TheFileX\\UI_NViews" );
    Reg.WriteRegiDouble( m_ConfigName , "dHorizDist_mm" , m_dHorizontalLineDist_mm ) ;
    m_Graph.SetProperty( "SkewMeterLU_LEL" , "BaseHorDist_mm" , m_dHorizontalLineDist_mm ) ;
    sHorizDist_mm.Format( "%.03f" , dHorizDist_mm ) ;
    SetDlgItemText( IDC_HORIZ_DIST , sHorizDist_mm ) ;
  }
}

void CUI_NViews::OnEnKillfocusVertDist()
{
  CString sVertDist_mm ;
  if ( GetDlgItemText( IDC_VERT_DIST , sVertDist_mm ) )
  {
    double dVertDist_mm = atof( sVertDist_mm ) ;
    if ( ( m_dVerticalLineDistNominal_mm - m_dDistAllowedVariation_mm ) > dVertDist_mm )
      dVertDist_mm = m_dVerticalLineDistNominal_mm - m_dDistAllowedVariation_mm ;
    if ( dVertDist_mm > ( m_dVerticalLineDistNominal_mm + m_dDistAllowedVariation_mm ) )
      dVertDist_mm = ( m_dVerticalLineDistNominal_mm + m_dDistAllowedVariation_mm )  ;
    m_dVerticalLineDist_mm = dVertDist_mm ;
    FXRegistry Reg( "TheFileX\\UI_NViews" );
    Reg.WriteRegiDouble( m_ConfigName , "dVertDist_mm" , m_dVerticalLineDist_mm ) ;
    m_Graph.SetProperty( "SkewMeterLU_LEL" , "BaseVertDist_mm" , m_dVerticalLineDist_mm ) ;
    sVertDist_mm.Format( "%.03f" , dVertDist_mm ) ;
    SetDlgItemText( IDC_VERT_DIST , sVertDist_mm ) ;
  }
}


void CUI_NViews::OnOK()
{
  return ;

  CDialogEx::OnOK();
}


BOOL CUI_NViews::PreTranslateMessage( MSG* pMsg )
{
  if ( ( pMsg->message == WM_KEYDOWN ) &&
    ( ( pMsg->wParam == VK_RETURN ) || ( pMsg->wParam == VK_ESCAPE ) ) )
  {
    return TRUE ;// do not dispatch message
  }

  return CDialogEx::PreTranslateMessage( pMsg );
}


void CUI_NViews::OnBnClickedTechnician()
{
  BOOL bEnabled = IsChecked( IDC_TECHNICIAN ) ;
  if ( bEnabled )
  {
    m_PasswordDlg.m_Password.Empty() ;
    if ( ( m_PasswordDlg.DoModal() == IDOK ) )
//     if ( ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
//       && ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) )
    {
      FXString Password( ( LPCTSTR ) m_PasswordDlg.m_Password ) ;
      FXRegistry Reg( "TheFileX\\UI_NViews" );
      FXString RealPW = Reg.GetRegiString( m_ConfigName , "TechnicianPW" , "Skew" ) ;
      if ( RealPW == Password )
      {
        ShowCalibrationControls( true ) ;
        return ;
      }
    }
    SetCheck( IDC_TECHNICIAN , false ) ;
  }
  else
    ShowCalibrationControls( false ) ;
}


void CUI_NViews::OnEnKillfocusSkewCalibUm()
{
  CString sSkewCalib_um ;
  if ( GetDlgItemText( IDC_SKEW_CALIB_UM , sSkewCalib_um ) )
  {
    m_dSkewCalib_um = atof( sSkewCalib_um ) ;
    FXRegistry Reg( "TheFileX\\UI_NViews" );
    Reg.WriteRegiDouble( m_ConfigName , "dSkewCalib_um" , m_dSkewCalib_um ) ;
    m_Graph.SetProperty( "SkewMeterLU_LEL" , "DistCorrectionB" , m_dSkewCalib_um ) ;
    sSkewCalib_um.Format( "%.03f" , m_dSkewCalib_um ) ;
    SetDlgItemText( IDC_SKEW_CALIB_UM , sSkewCalib_um ) ;
  }
}

void CUI_NViews::ShowCalibrationControls( int iMode )
{
  int iShow = ( iMode == 1 ) ? SW_SHOW : SW_HIDE ;
  GetDlgItem( IDC_STATIC_HOR_DIST )->ShowWindow( iShow ) ;
  GetDlgItem( IDC_STATIC_VERT_DIST )->ShowWindow( iShow ) ;
  GetDlgItem( IDC_STATIC_SKEW_CALIB )->ShowWindow( iShow ) ;
  GetDlgItem( IDC_HORIZ_DIST )->ShowWindow( iShow ) ;
  GetDlgItem( IDC_VERT_DIST )->ShowWindow( iShow ) ;
  GetDlgItem( IDC_SKEW_CALIB_UM )->ShowWindow( iShow ) ;
  GetDlgItem( IDC_VIEW_SETUP )->ShowWindow( iShow ) ;
  GetDlgItem( IDC_VIEW_GRAPH )->ShowWindow( iShow ) ;
  GetDlgItem( IDC_SAVE_GRAPH )->ShowWindow( iShow ) ;

}

void CUI_NViews::ProcessDataFromShMem()
{
  int iRes = 0 ;
  int iLen = m_pShMemControl->GetRequestLength() ;
  if ( iLen > 0 )
  {
    int iBufLen = iLen + 20 ;
    BYTE * pBuf = new BYTE[ iBufLen ] ;
    iRes = m_pShMemControl->ReceiveRequest( pBuf , iBufLen ) ;
    if ( iRes > 0 )
    {
      PostMessage( WM_REQUEST_FROM_SHARED_MEM , iRes , ( LPARAM ) pBuf ) ;
      return ;
    }
    else if ( iRes < 0 )
    {
      CString Msg ;
      Msg.Format( "ProcessDataFromShMem - Error %d on request data receive, Len=%d" , iRes , iLen ) ;
      m_LogView.InsertString( 0 , Msg ) ;
    }
  }
}

void CUI_NViews::ProcessErrorFromShMem( DWORD ErrCode )
{
  CString Msg ;
  Msg.Format( "Error %d on ShMem data waiting" , ErrCode ) ;
  m_LogView.InsertString( 0 , Msg ) ;
}

enum IP_CAM_POSITION
{
  FULL_IR ,			// complete tray - IR
  FULL_COLOR ,			// complete tray - color
  HEAD_CENTER_COLOR ,	// tray/flower head - centered
  HEAD_RIGHT_COLOR ,	// tray/flower head - right side (from bottom of image)
  HEAD_LEFT_COLOR ,	// tray/flower head - left side (from top of image)
};

struct SImageData
{
  int width;				// image width
  int height;				// image height
  DWORD format;	// format of image buffer
  BYTE data[1];				// image data buffer
};

DWORD GetImageBufferSize( SImageData * pImage )
{
  return ( ( pImage->width * pImage->height ) * ( ( pImage->format == IP_YUV422 ) ? 2 : 1 ) ) ;
}

DWORD GetSHImageBufferSize( SImageData * pImage )
{
  DWORD dwImageSize = ( pImage->width * pImage->height ) ;
  switch ( pImage->format )
  {
    case BI_YUV422:
    case BI_UYVY:
    case BI_I420:
    case BI_YUY2:
    case IP_YUV422: dwImageSize *= 2 ; break ;
    case BI_YUV12:
    case BI_NV12:
    case BI_YUV411: dwImageSize = ( dwImageSize * 3 ) / 2 ; break ;
    case BI_YUV9: dwImageSize = ( dwImageSize * 9 ) / 8 ; break ;
    case BI_Y8:
    case 0: pImage->format = BI_Y800; break ; // 0 is mono 8 bit format in Tecto, it's not BI_RGB of SH
  }
  return dwImageSize ;
}

CVideoFrame *  CUI_NViews::ConvertIPImageToSH( TectoMsg * pAnalyzeFlowerMsg , int iImageNum ) 
{
  AnalyzeFlowerContent * pContent = ( AnalyzeFlowerContent * ) pAnalyzeFlowerMsg->GetData() ;
  int imageCount = pContent->m_iImageCount ;
  if ( imageCount <= iImageNum )
    return NULL ;
  SImageData * pImage = ( SImageData * )pContent->m_Images ;
  int iIter = 0 ;

  DWORD dwImageSize = GetSHImageBufferSize( pImage ) ;
  while ( iIter != iImageNum )
  {
    pImage = ( SImageData * ) ( ( LPBYTE ) ( pImage + 1 ) + dwImageSize ) ;
    if ( ( LPBYTE ) pImage >= ( ( LPBYTE ) pContent + pAnalyzeFlowerMsg->GetMsgLen() ) )
      return NULL ;
    dwImageSize = GetImageBufferSize( pImage ) ;
  } ;

  pTVFrame pTV = NULL ;
  if ( (pImage->format == BI_Y800) || (pImage->format == 0) )
    pTV = makeNewY8Frame( pImage->width , pImage->height , pImage->data ) ;
  else //color image
  {

    LPBITMAPINFOHEADER lpBMI = (LPBITMAPINFOHEADER)malloc( sizeof(BITMAPINFOHEADER) + dwImageSize ) ;
    memset( lpBMI , 0 , sizeof( LPBITMAPINFOHEADER ) ) ;

    lpBMI->biSize = sizeof( BITMAPINFOHEADER ) ;
    lpBMI->biWidth = pImage->width ;
    lpBMI->biHeight = pImage->height ;
    lpBMI->biSizeImage = GetImageBufferSize( pImage ) ;
    lpBMI->biPlanes = 1 ;
    lpBMI->biBitCount = (WORD)((8 * dwImageSize) / (pImage->width * pImage->height));
    switch ( pImage->format )
    {
      case 1: 
        lpBMI->biCompression = BI_UYVY ; 
        TVFrame TV422 ;
        TV422.lpBMIH = lpBMI ;
        TV422.lpData = pImage->data ;
        pTV = ( pTVFrame ) malloc( sizeof( TVFrame ) );
        pTV->lpBMIH = _convertUYVY2YUV12( &TV422 ) ;
        pTV->lpData = NULL ;
        break ;
      default: 
        lpBMI->biCompression = pImage->format ;
        pTV = newTVFrame( lpBMI , pImage->data ) ;
        break ;
    }
  }
  if ( pTV )
  {
    CVideoFrame * pFrame = CVideoFrame::Create( pTV ) ;
    return pFrame ;
  }
  return NULL ;
}


int CUI_NViews::AnalyzeFlower( TectoMsg * pAnalyzeMsg )
{
  AnalyzeFlowerContent * pContent = ( AnalyzeFlowerContent * ) pAnalyzeMsg->GetData() ;
  int imageCount = pContent->m_iImageCount ;
  int iTicket = pContent->m_iTicket ;
  //strcpy_s( pContent->m_FlowerName , flowerName ) ;
  BYTE * pImageData = pContent->m_Images ;

  int iRes = TM_OK ;
  int iNOrdered = 0 ;
  for ( int i = 0 ; i < imageCount ; i++ )
  {
    CVideoFrame * pFrame = ConvertIPImageToSH( pAnalyzeMsg , i ) ;
    if ( pFrame )
    {
      CString TvObjPinName , TectoPinName , RenderPinName ;
      TvObjPinName.Format( "TVObjects%d<<1" , i + 1 ) ;
      TectoPinName.Format( "Tecto%d<<0" , i + 1 ) ;
      RenderPinName.Format( "FRender%d<<0" , i + 1 ) ;
      const char * TargetPins[] = { ( LPCTSTR ) TvObjPinName , ( LPCTSTR ) TectoPinName , NULL } ;
      int iSent = m_Graph.SendText( TargetPins , "Task(1);" , "mode" ) ;
      if ( !( iSent == ( ARRSZ( TargetPins ) - 1 ) ) )
      {
        CString Msg ;
        Msg.Format( "ERROR: Analyze Msgs sent to %d pins instead of %d " ,
          iSent , ARRSZ( TargetPins ) - 1 ) ;
        m_LogView.InsertString( 0 , Msg ) ;
        pAnalyzeMsg->m_Id = TM_AnalyzeTechnicalError ;
        strcpy_s( ( char* ) ( pAnalyzeMsg->m_MsgData ) , 256 , Msg ) ;
        pAnalyzeMsg->m_MsgDataLen = (DWORD)strlen( ( char* ) ( pAnalyzeMsg->m_MsgData ) ) + 1 ; // trailing zero
        iRes = TM_AnalyzeTechnicalError ;
        break ;
      }
      iNOrdered++ ;
      CString ImageDestPin , PinForSaveImage ;
      ImageDestPin.Format( "Buffer%d<<0" , i + 1 ) ;
      pFrame->ChangeId( iTicket ) ;
      CString LabelAsTicket ;
      LabelAsTicket.Format( "%d-%d" , iTicket , i ) ;
      pFrame->SetLabel( LabelAsTicket ) ;
      bool bSendForSave = IsChecked( IDC_SAVE_SRC ) ;
      if ( bSendForSave )
      {
        pFrame->AddRef() ;
        PinForSaveImage.Format( "BMPRender%d<<0" , i + 1 ) ;
      }
      
      if ( IsChecked( IDC_SAVE_RESULT ) )
      {
        CString SaveImageContent , Prefix , Suffix ;
        SaveImageContent.Format( "1,%s" , (LPCTSTR)m_CurrentResultImageSavingDir ) ;
        int iSent = m_Graph.SendText( RenderPinName , (LPCTSTR)SaveImageContent , "SaveImage" ) ;
        Prefix.Format( "%d-" , iTicket ) ;
        iSent += m_Graph.SendText( RenderPinName , ( LPCTSTR ) Prefix , "ImageSavePrefix" ) ;
        Suffix.Format( "-%d" , i ) ;
        iSent += m_Graph.SendText( RenderPinName , ( LPCTSTR ) Suffix , "ImageSaveSuffix" ) ;
      }

      if ( !m_Graph.SendFrame( ImageDestPin , pFrame ) )
      {
        CString Msg ;
        Msg.Format( "ERROR: Can't send image to %s " , (LPCTSTR) ImageDestPin ) ;
        m_LogView.InsertString( 0 , Msg ) ;
        pAnalyzeMsg->m_Id = TM_AnalyzeTechnicalError ;
        strcpy_s( ( char* ) ( pAnalyzeMsg->m_MsgData ) , 256 , Msg ) ;
        pAnalyzeMsg->m_MsgDataLen = ( DWORD ) strlen( ( char* ) ( pAnalyzeMsg->m_MsgData ) ) + 1 ; // trailing zero
        iRes = TM_AnalyzeTechnicalError ;
        if ( !PinForSaveImage.IsEmpty() )
          pFrame->Release() ;

        break ;
      }
      else
      {
        if ( !PinForSaveImage.IsEmpty() )
        {
          if ( !m_Graph.SendFrame( PinForSaveImage , pFrame ) )
          {
            CString Msg ;
            Msg.Format( "ERROR: Can't send image to %s for Save" , ( LPCTSTR ) PinForSaveImage ) ;
            m_LogView.InsertString( 0 , Msg ) ;
          }
        }
      }
    }
  }

  return iRes ;
}
int CUI_NViews::AnalyzeCalibration( TectoMsg * pCalibMsg )
{
  CalibContent * pContent = ( CalibContent * ) pCalibMsg->GetData() ;
  int imageCount = pContent->m_iNFiles ;
  int camsLayout[ 10 ] ;
  CStringArray CalibFileNames ;

  int iRes = ProcessCalibrationFiles( pCalibMsg , imageCount , camsLayout , CalibFileNames ) ;

  return iRes ;
}

LRESULT CUI_NViews::OnAnswerForTectoLib( WPARAM wParam , LPARAM lParam )
{
  CString Answer = ( LPCTSTR ) lParam ;
  delete[]( char* ) lParam ;
  m_dAnswerTime_ms = GetHRTickCount() ;
  double dProcessingTime_ms = ( m_dRequestTime_ms ) ?
    m_dAnswerTime_ms - m_dRequestTime_ms : 0. ;
  CString AnswerAsLower( Answer ) ;
  AnswerAsLower.MakeLower() ;
  bool bOK = ( AnswerAsLower.Find( "failed" ) == -1 ) ;
  bool bEmpty = !bOK && ( AnswerAsLower.Find( " empty" ) != -1 ) ;
  CString ProcTimeAsString ;
  ProcTimeAsString.Format( "Duration=%.2f ms\n" , dProcessingTime_ms ) ;
  Answer += ProcTimeAsString ;
  if ( Answer.Find( "Side") >= 0 )
  {
    m_Cam1StatusLED.SetBitmap( bOK ? m_GreenLed : m_RedLed ) ;
    m_iRestToViewCam1 = 2 ;
    m_bLastResult1 = bOK ;
    m_iSideImagesWithFlowerCount++ ;
  }
  else if ( Answer.Find( "Front" ) >= 0 )
  {
    m_Cam2StatusLED.SetBitmap( bOK ? m_GreenLed : m_RedLed ) ;
    m_iRestToViewCam2 = 2 ;
    m_bLastResult2 = bOK ;
    m_iFrontImagesWithFlowerCount++ ;
  }
  if ( AnswerAsLower.Find( " pass\n" ) != -1 )
    m_iGoodCount++ ;
  else if ( bEmpty )
    m_iEmptyTrayCount++ ;
  else if ( !bOK )
    m_iBadCount++ ;
  TectoMsg * pMsg = TectoMsg::CreateMsg( bOK ? TM_OK : TM_CalibError , Answer.GetLength() + 5 ) ;
  switch ( m_WaitAnswerForThisRequest )
  {
  case TM_Calibration:
  	break;
  case TM_AnalyzeFlower:
    if ( !bOK )
    {
      if ( bEmpty )
        pMsg->m_Id = TM_TrayEmpty ;
      else 
        pMsg->m_Id = TM_ERROR ;
    }
    else
    {
      m_iNRepeatCycles = 0 ;
    }
    break ;
  }
  
  if ( !m_iNRepeatCycles && (m_WaitAnswerForThisRequest != TM_NoWaiting) )
  {
    strcpy_s( ( char* ) pMsg->m_MsgData , Answer.GetLength() + 4 , ( LPCTSTR ) Answer ) ;
    m_pShMemControl->SendAnswer( pMsg , pMsg->GetMsgLen() ) ;
  }
  delete pMsg ;
  m_WaitAnswerForThisRequest = TM_NoWaiting ;

  // Data for statistics extraction
  FXPropertyKit pk( (LPCTSTR)AnswerAsLower ) ;
  FXString Numerical , FileName ;
  pk.GetString( "numerical" , Numerical , false ) ;
  if ( pk.GetString( "filename" , FileName , false ) )
  {
    int iBSlashPos = ( int ) FileName.ReverseFind( '\\' ) ;
    int iSlashPos = ( int ) FileName.ReverseFind( '/' ) ;
    if ( iSlashPos < iBSlashPos )
      iSlashPos = iBSlashPos ;
    if ( iSlashPos >= 0 )
      FileName = FileName.Mid( iSlashPos + 1 ) ;
    FXString ForStatistics ;
    ForStatistics.Format( "%5d, %s , %s , Time=%.2f , FileName=%s\n" , m_iSideImagesWithFlowerCount ,
      ( bOK ) ? "PASSED" : (bEmpty) ? "EMPTY" : "FAILED" ,
      ( LPCTSTR ) Numerical , dProcessingTime_ms , ( LPCTSTR ) FileName ) ;
    ofstream myfile( ( LPCTSTR ) m_StatisticsFileName , ios_base::app );
    if ( myfile.is_open() )
    {
      myfile << (LPCTSTR)ForStatistics ;
      myfile.close();
    }
    else
    {
      FXString Msg ;
      Msg.Format( "CUI_NViews::OnAnswerForTectoLib ERROR %s: Can't write Statistics data to file %s" ,
        strerror( GetLastError() ) , ( LPCTSTR ) m_StatisticsFileName );
      m_LogView.InsertString( 0 , Msg ) ;
    }
  };


  if ( IsChecked( IDC_BATCH ) )
    SetTimer( EvGetNextImage , 300 , NULL ) ;
  return NULL ;
}

// structure used for sending image data
struct Simu_ImageData
{
  int width;				// image width
  int height;				// image height
  UINT format;	// format of image buffer
  BYTE data[1];				// image data buffer
};

struct SimulationImages
{
  char FilePath[ 256 ] ;
  int  m_iNImages ; // return value
  BYTE m_Images[ 1 ] ; // series of images with pointers inside this structure
};

LRESULT CUI_NViews::OnSimuImageArrived( WPARAM wParam , LPARAM lParam )
{
  int iCamNum = ( int ) wParam ;

  CallBackDataA * pCallBackData = ( CallBackDataA* ) lParam ;
  if ( pCallBackData->m_ResultType == DATA_IMAGE )
  {
    LPBITMAPINFOHEADER pBMPI = pCallBackData->m_Par1.BmpInfo ;
    LPBYTE pData = ( pCallBackData->m_Par2 ) ?
      ( LPBYTE ) pCallBackData->m_Par2 : ( LPBYTE ) ( pBMPI + 1 ) ;

    UINT uiMsgSize = sizeof( Simu_ImageData ) + pBMPI->biSizeImage + 4096 ;
    TectoMsg * pMsg = TectoMsg::CreateMsg( TM_SimuImageArrived , uiMsgSize ) ;
    if ( pMsg )
    {
      SimulationImages * pDataWithImage = ( SimulationImages * ) ( pMsg->GetData() ) ;
      if ( pCallBackData->m_Label && pCallBackData->m_Label[ 0 ] )
        strcpy_s( pDataWithImage->FilePath , sizeof( SimulationImages::FilePath ) , pCallBackData->m_Label ) ;
      else
        pDataWithImage->FilePath[ 0 ] = 0 ;
      pDataWithImage->m_iNImages = 1 ;
      Simu_ImageData * pSimuImage = ( Simu_ImageData * )pDataWithImage->m_Images ;
      pSimuImage->width = pBMPI->biWidth ;
      pSimuImage->height = pBMPI->biHeight ;
      int iImageSizePix = pBMPI->biWidth * pBMPI->biHeight ;
      pSimuImage->format = (pBMPI->biSizeImage == iImageSizePix ) ? 0 : 1 ;
      memcpy_s( &pSimuImage->data , uiMsgSize - sizeof( Simu_ImageData ) , pData , pBMPI->biSizeImage ) ;
      m_pShMemControl->SendAnswer( pMsg , pMsg->GetMsgLen() ) ;
    }
    delete pMsg ;
  }
  m_WaitAnswerForThisRequest = TM_NoWaiting ;

  return 1 ;
}

LRESULT CUI_NViews::OnShMemMessage( WPARAM wParam , LPARAM lParam )
{
  TectoMsg * pMsg = ( ( TectoMsg* ) lParam )->Copy() ;
  delete[]( BYTE* )lParam ; // created in receive thread, necessary to 

  switch ( pMsg->m_Id )
  {
    case TM_Dummy: m_WaitAnswerForThisRequest = TM_NoWaiting ;  break ;
    case TM_Init:
//       if ( !m_Graph.SetTextCallBack( "Tecto1>>1" , ResultForTectoCallBack , this ) )
//       {
//         pMsg->m_Id = TM_ERROR ;
//         sprintf( ( char* ) ( pMsg->m_MsgData ) , "Can't assign callback from 'Tecto1>>1' pin" ) ;
//         m_LogView.InsertString( 0 , ( char* ) ( pMsg->m_MsgData ) ) ;
//       }
//       else
      {
        pMsg->m_Id = TM_OK ;
        strcpy( ( char* ) ( pMsg->m_MsgData ) , "OK" ) ;
        m_LogView.InsertString( 0 , "External Library Connected" ) ;
      }
      pMsg->m_MsgDataLen = ( DWORD ) strlen( ( char* ) ( pMsg->m_MsgData ) ) + 1 ; //  + trailing zero
      m_pShMemControl->SendAnswer( pMsg , pMsg->GetMsgLen() ) ;
      delete pMsg ;
      m_WaitAnswerForThisRequest = TM_NoWaiting ;
      SetCheck( IDC_BATCH , false ) ;
      EnableControl( IDC_BATCH , FALSE ) ;
      EnableControl( IDC_GET_NEXT , FALSE ) ;
      return 1 ;
    case TM_Terminate:
      pMsg->m_Id = TM_OK ;
      strcpy( ( char* ) ( pMsg->m_MsgData ) , "OK" ) ;
      pMsg->m_MsgDataLen = 3 ; // "OK" + trailing zero
      m_pShMemControl->SendAnswer( pMsg , pMsg->GetMsgLen() ) ;
      delete pMsg ;
      m_LogView.InsertString( 0 , "External Library Disconnected" ) ;
      m_WaitAnswerForThisRequest = TM_NoWaiting ;
      EnableControl( IDC_BATCH , TRUE ) ;
      EnableControl( IDC_GET_NEXT , TRUE ) ;
      return 1 ;
    case TM_Calibration:
    {
      int iRes = AnalyzeCalibration( pMsg ) ;
      if ( iRes != TM_OK ) // error before real calibration
      {
        m_LogView.InsertString( 0 , ( char* ) ( pMsg->GetData() ) ) ;
        m_pShMemControl->SendAnswer( pMsg , pMsg->GetMsgLen() ) ;
        delete pMsg ;
        m_WaitAnswerForThisRequest = TM_NoWaiting ;
        return 1 ; // operation finished
      }
      // Calibration started, wait for text answer through call back from Tecto1>>1 output
      m_WaitAnswerForThisRequest = TM_Calibration ;
      delete pMsg ;
      return 0 ; // operation is not finished, library waits for answer
    }
    case TM_SetFLower:
    {
      int iRes = SetFlower( pMsg ) ;
      if ( iRes == TM_OK )
        pMsg->m_Id = TM_OK ;
      m_pShMemControl->SendAnswer( pMsg , pMsg->GetMsgLen() ) ;
      m_WaitAnswerForThisRequest = TM_NoWaiting ;
      delete pMsg ;
      return ( iRes == TM_OK ) ;
    }
    case TM_GetParameterList:
    {
      int iRes = GetParameters( pMsg ) ;
      m_pShMemControl->SendAnswer( pMsg , pMsg->GetMsgLen() ) ;
      m_WaitAnswerForThisRequest = TM_NoWaiting ;
      delete pMsg ;
      return ( iRes == TM_OK ) ;
    }
    case TM_AnalyzeFlower:
    {
      m_dRequestTime_ms = GetHRTickCount() ;
      m_iNRepeatCycles = 0 ;
      int iRes = AnalyzeFlower( pMsg ) ;
      if ( iRes != TM_OK ) // error before real measurement
      {
        m_LogView.InsertString( 0 , ( char* ) ( pMsg->GetData() ) ) ;
        m_pShMemControl->SendAnswer( pMsg , pMsg->GetMsgLen() ) ;
        delete pMsg ;
        m_WaitAnswerForThisRequest = TM_NoWaiting ;
        return 1 ; // operation finished
      }
      delete pMsg ;

      m_WaitAnswerForThisRequest = TM_AnalyzeFlower ;
      m_ImageProcessingOrdered = IPO_OrderedByExternal ;
      return 0 ; // operation is not finished, library waits for answer
    }
    break ;
    case TM_GetSimulationImage:
    {
      m_WaitAnswerForThisRequest = TM_NoWaiting ;
      SimulationImageContent * pSimuContent = ( SimulationImageContent * ) pMsg->GetData() ;
      for ( int i = 0 ; i < pSimuContent->m_iNImages ; i++ )
      {
        CString BMPReadPinName ;
        BMPReadPinName.Format( "BMPCapture%d<<0" , i + 1 ) ;
        if ( m_Graph.SendQuantity( BMPReadPinName , 1 , "FromSimulator" ) )
          m_WaitAnswerForThisRequest = TM_SimuImageArrived ;
      }
      delete pMsg ;
      return ( m_WaitAnswerForThisRequest == TM_SimuImageArrived ) ;
    }
    default: 
      delete pMsg ;
      return 0 ;
  }
  return 1;
}


int CUI_NViews::SetFlower( TectoMsg * pSetFlowerMsg )
{
  FlowerSetContent * pContent = ( FlowerSetContent * ) pSetFlowerMsg->GetData() ;
  int imageCount ;
  int camsLayout[ 10 ] ;
  CStringArray CalibFileNames ;

    // ProcessCalibrationFiles will set all necessary directories
  int iRes = ProcessCalibrationFiles( pSetFlowerMsg , 
    imageCount , camsLayout , CalibFileNames ) ;

  if ( iRes == TM_OK )
    m_CurrentFlowerName = pContent->m_FlowerName ;

  return iRes ;
}

// ProcessCalibrationFiles works for TM_Calibration and TM_SetFLower messages
// This function initializes calibration for both messages
// And do initialize directories for simulation images reading


int CUI_NViews::ProcessCalibrationFiles( TectoMsg * pMsg ,
  int& imageCount , int camsLayout[ 10 ] , CStringArray& CalibFileNames )
{
  // initial message filtration by ID 
  switch ( pMsg->m_Id )
  {
    case TM_Calibration:
    case TM_SetFLower:
      break ;
    default: return TM_ERROR ;
  }

  imageCount = 0 ;
  FlowerSetContent * pFlowerSetContent = ( FlowerSetContent * ) ( pMsg->GetData() );
  CalibContent * pContent = ( ( pMsg->m_Id == TM_Calibration ) ?
    ( CalibContent * ) pFlowerSetContent : ( CalibContent * ) &( pFlowerSetContent->m_iCamCount ) );
  if ( pMsg->m_Id == TM_SetFLower )
  {
    imageCount = pFlowerSetContent->m_iCamCount ;
    pContent = ( CalibContent * ) ( ( ( BYTE* ) pContent ) + 128 ); // shift for flower name field size
  }
  else
    imageCount = pContent->m_iNFiles ;
  memcpy( camsLayout , pContent->m_CamPositions , imageCount * sizeof( int ) ) ;

  if ( pMsg->m_Id == TM_SetFLower )
  {
    FlowerSetContent * pSetContent = ( FlowerSetContent * ) pMsg->GetData() ;
    m_CurrentFlowerName = pSetContent->m_FlowerName ;
    SetDlgItemText( IDC_FLOWER_NAME , m_CurrentFlowerName ) ;
    if ( m_CurrentFlowerName.IsEmpty() )
    {
      CString Msg ;
      Msg.Format( "ERROR : Flower name is not assigned" ) ;
      m_LogView.InsertString( 0 , Msg ) ;
      pMsg->m_Id = TM_SetFlowerError ;
      strcpy_s( ( char* ) ( pMsg->m_MsgData ) , 256 , Msg ) ;
      pMsg->m_MsgDataLen = ( DWORD ) strlen( ( char* ) ( pMsg->m_MsgData ) ) + 1 ; // trailing zero
      return TM_SetFlowerError ;
    }
    return SetFlowerAndDirectories( imageCount , pMsg ) ;
  }

  int iCopiedLen = 0 ;
  int iFileIndex = 0 ;
  do
  {
    FILE * fr ;
    errno_t err = fopen_s( &fr , pContent->m_CalibFileNames[ iFileIndex ] , "r" ) ;
    if ( err == 0 )
    {
      fclose( fr ) ;
      CalibFileNames.Add( pContent->m_CalibFileNames[ iFileIndex ] ) ;
    }
    else
    {
      pMsg->m_Id = TM_FileOpenError ;
      sprintf_s( ( char* ) ( pMsg->m_MsgData ) , 256 ,
        "Can't open calibration file %s" , pContent->m_CalibFileNames[ iFileIndex ] ) ;
      pMsg->m_MsgDataLen = ( DWORD ) strlen( ( char* ) ( pMsg->m_MsgData ) ) + 1 ; // trailing zero

      return TM_FileOpenError ;
    }
  } while ( ++iFileIndex < imageCount );

  char Drive[ 10 ] , Dir[ 400 ] , FName[ 400 ] , Ext[ 100 ] ;
  bool bOrdered = false ;
  for ( INT_PTR i = 0 ; i < CalibFileNames.GetCount() ; i++ )
  {
    _splitpath_s( ( LPCTSTR ) CalibFileNames[ i ] , Drive , Dir , FName , Ext ) ;
    CString FullDir( Drive ) ;
    FullDir += Dir ;
    CString FullFileName( FName ) ;
    FullFileName += Ext ;
    switch ( camsLayout[ i ] )
    {
      case FULL_IR:
      {
        CString BMPReadCalibGadgetName ;
        BMPReadCalibGadgetName.Format( "BMPCapCalib%d" , i + 1 ) ;
        int iRes = m_Graph.SetProperty( BMPReadCalibGadgetName , "Directory" , FullDir ) ;
        if ( !iRes )
        {
          CString Msg ;
          Msg.Format( "ERROR: Can't set directory %s in %s gadget " ,
            ( LPCTSTR ) FullDir , ( LPCTSTR ) BMPReadCalibGadgetName ) ;
          m_LogView.InsertString( 0 , Msg ) ;
          pMsg->m_Id = TM_CalibSetupError ;
          strcpy_s( ( char* ) ( pMsg->m_MsgData ) , 256 , Msg ) ;
          pMsg->m_MsgDataLen = ( DWORD ) strlen( ( char* ) ( pMsg->m_MsgData ) ) + 1 ; // trailing zero
        }
        else
        {
          iRes = m_Graph.SetProperty( BMPReadCalibGadgetName , "FileName" , FullFileName ) ;
          if ( !iRes )
          {
            CString Msg ;
            Msg.Format( "ERROR: Can't set File Name %s in %s gadget " ,
              ( LPCTSTR ) FullFileName , ( LPCTSTR ) BMPReadCalibGadgetName ) ;
            m_LogView.InsertString( 0 , Msg ) ;
            pMsg->m_Id = TM_CalibSetupError ;
            strcpy_s( ( char* ) ( pMsg->m_MsgData ) , 256 , Msg ) ;
            pMsg->m_MsgDataLen = (DWORD)strlen( ( char* ) ( pMsg->m_MsgData ) ) + 1 ; // trailing zero
          }
          else
          {
            CString TvObjPinName , TectoPinName , BMPReadPinName ;
            TvObjPinName.Format( "TVObjects%d<<1" , i + 1 ) ;
            TectoPinName.Format( "Tecto%d<<0" , i + 1 ) ;
            BMPReadPinName = BMPReadCalibGadgetName + "<<0" ;
            const char * TargetPins[] = { ( LPCTSTR ) TvObjPinName , ( LPCTSTR ) TectoPinName , ( LPCTSTR ) BMPReadPinName , NULL } ;
            int iSent = m_Graph.SendText( TargetPins , "Task(2);" , "mode" ) ;
            bOrdered = ( iSent == ( ARRSZ( TargetPins ) - 1 ) ) ;
            if ( !bOrdered )
            {
              CString Msg ;
              Msg.Format( "ERROR: Calibration Msgs sent to %d pins instead of %d " ,
                iSent , ARRSZ( TargetPins ) - 1 ) ;
              m_LogView.InsertString( 0 , Msg ) ;
              pMsg->m_Id = TM_CalibSetupError ;
              strcpy_s( ( char* ) ( pMsg->m_MsgData ) , 256 , Msg ) ;
              pMsg->m_MsgDataLen = ( DWORD ) strlen( ( char* ) ( pMsg->m_MsgData ) ) + 1 ; // trailing zero
            }
          }
        }
      }
      default: break ;
    }
  }
  return bOrdered ? TM_OK : TM_ERROR ;
}

int CUI_NViews::GetParameters( TectoMsg * pGetParametersMsg )
{
  ParameterListContent * pContent = ( ParameterListContent * ) pGetParametersMsg->GetData() ;
  pContent->m_iParamCount = 0 ;
  pContent->m_Diagnostics[ 0 ] = 0 ;
  LPCTSTR pTecto1 = "Tecto1" ;
  BOOL bActive = FALSE ;
  double dMin , dMax ;

  if ( m_Graph.GetProperty( pTecto1 , "Meas_Length" , bActive ) )
  {
    if ( bActive )
    {
      int iAquired = 0 ;
      if ( !m_Graph.GetProperty( pTecto1 , "MinimalLength_mm" , dMin ) )
        ASSERT( 0 ) ;
      else
        iAquired++ ;
      if ( !m_Graph.GetProperty( pTecto1 , "MaximalLength_mm" , dMax ) )
        ASSERT( 0 ) ;
      else
        iAquired++ ;
      if ( iAquired == 2 )
      {
//         sprintf_s( pContent->m_Parameters[ pContent->m_iParamCount++ ] ,
//           "Length[%d,%d]mm" , ( int ) dMin , ( int ) dMax ) ;
        sprintf_s( pContent->m_Parameters[ pContent->m_iParamCount++ ] ,
          "Meas_Length" ) ;
      }
      else
        strcat_s( pContent->m_Diagnostics , "Can't get Length limits" ) ;
    }
  };

  if ( m_Graph.GetProperty( pTecto1 , "XRight_out" , bActive ) )
  {
//     if ( bActive )
//       sprintf_s( pContent->m_Parameters[ pContent->m_iParamCount++ ] , "Right Flower Edge" ) ;
    if ( bActive )
      sprintf_s( pContent->m_Parameters[ pContent->m_iParamCount++ ] , "XRight_out" ) ;
  };
  if ( m_Graph.GetProperty( pTecto1 , "Meas_Width" , bActive ) )
  {
    if ( bActive )
    {
      int iAquired = 0 ;
      if ( !m_Graph.GetProperty( pTecto1 , "MinimalWidth_mm" , dMin ) )
        ASSERT( 0 ) ;
      else
        iAquired++ ;
      if ( !m_Graph.GetProperty( pTecto1 , "MaximalWidth_mm" , dMax ) )
        ASSERT( 0 ) ;
      else
        iAquired++ ;
      if ( iAquired == 2 )
      {
//         sprintf_s( pContent->m_Parameters[ pContent->m_iParamCount++ ] ,
//           "Width[%d,%d]mm" , ( int ) dMin , ( int ) dMax ) ;
        sprintf_s( pContent->m_Parameters[ pContent->m_iParamCount++ ] ,
          "Meas_Width" ) ;
      }
      else
        strcat_s( pContent->m_Diagnostics , "Can't get Width limits" ) ;
    }
  };
  if ( m_Graph.GetProperty( pTecto1 , "Meas_Area" , bActive ) )
  {
    if ( bActive )
    {
      int iAquired = 0 ;
      if ( !m_Graph.GetProperty( pTecto1 , "MinimalFlowerArea_cm2" , dMin ) )
        ASSERT( 0 ) ;
      else
        iAquired++ ;
      if ( !m_Graph.GetProperty( pTecto1 , "MaximalFlowerArea_cm2" , dMax ) )
        ASSERT( 0 ) ;
      else
        iAquired++ ;
      if ( iAquired == 2 )
//         sprintf_s( pContent->m_Parameters[ pContent->m_iParamCount++ ] , "Area[%d,%d]cm2" , ( int ) dMin , ( int ) dMax ) ;
        sprintf_s( pContent->m_Parameters[ pContent->m_iParamCount++ ] , "Meas_Area" ) ;
      else
        strcat_s( pContent->m_Diagnostics , "Can't get Area limits" ) ;
    }
  };
  if ( m_Graph.GetProperty( pTecto1 , "Meas_Stalk" , bActive ) )
  {
    if ( bActive )
    {
      int iAquired = 0 ;
      if ( !m_Graph.GetProperty( pTecto1 , "StalkDiaMin_mm" , dMin ) )
        ASSERT( 0 ) ;
      else
        iAquired++ ;
      if ( !m_Graph.GetProperty( pTecto1 , "StalkDiaMax_mm" , dMax ) )
        ASSERT( 0 ) ;
      else
        iAquired++ ;
      if ( iAquired == 2 )
//        sprintf_s( pContent->m_Parameters[ pContent->m_iParamCount++ ] , "Stalk dia[%.1f,%.1f]mm" , dMin , dMax ) ;
        sprintf_s( pContent->m_Parameters[ pContent->m_iParamCount++ ] , "Meas_Stalk" ) ;
      else
        strcat_s( pContent->m_Diagnostics , "Can't get Stalk dia limits" ) ;
    }
  };
  if ( m_Graph.GetProperty( pTecto1 , "Meas_Stem" , bActive ) )
  {
    if ( bActive )
    {
      int iAquired = 0 ;
      if ( !m_Graph.GetProperty( pTecto1 , "StemDiaMin_mm" , dMin ) )
        ASSERT( 0 ) ;
      else
        iAquired++ ;
      if ( !m_Graph.GetProperty( pTecto1 , "StemDiaMax_mm" , dMax ) )
        ASSERT( 0 ) ;
      else
        iAquired++ ;
      if ( iAquired == 2 )
//        sprintf_s( pContent->m_Parameters[ pContent->m_iParamCount++ ] , "Stem dia[%.1f,%.1f]mm" , dMin , dMax ) ;
        sprintf_s( pContent->m_Parameters[ pContent->m_iParamCount++ ] , "Meas_Stem" ) ;
      else
        strcat_s( pContent->m_Diagnostics , "Can't get Stalk dia limits" ) ;
    }
  };
  if ( pContent->m_Diagnostics[ 0 ] == 0 )
  {
    pGetParametersMsg->m_Id = TM_OK ;
    return TM_OK ;
  }
  return TM_ERROR ;
}

void CUI_NViews::OnBnClickedSideProcSetup()
{
  CRect rcDlg ;
  GetWindowRect( &rcDlg ) ;
  CPoint ViewPt( rcDlg.right + 10 , rcDlg.top + 50 ) ;
  m_Graph.RunSetupDialog( "Tecto1" , &ViewPt ) ;
}

void CUI_NViews::OnBnClickedFrontProcSetup()
{
  CRect rcDlg ;
  GetWindowRect( &rcDlg ) ;
  CPoint ViewPt( rcDlg.right + 10 , rcDlg.top + 600 ) ;
  m_Graph.RunSetupDialog( "Tecto2" , &ViewPt ) ;
}

void CUI_NViews::OnBnClickedSaveToRegistry()
{
  m_Graph.SetProperty( "Tecto1" , "SaveToReg" , 1 ) ;
  m_iNSources = GetDlgItemInt( IDC_NSOURCES ) ;
  if ( m_iNSources > 2 )
    m_Graph.SetProperty( "Tecto2" , "SaveToReg" , 1 ) ;
  SaveGraphCorrections() ;
}

void CUI_NViews::OnEnChangeFlowerName()
{
}

void CUI_NViews::OnEnKillfocusFlowerName()
{
}

void CUI_NViews::OnEnUpdateFlowerName()
{
}

void CUI_NViews::OnBnClickedSetFlower()
{
  GetDlgItemText( IDC_FLOWER_NAME , m_CurrentFlowerName ) ;
  m_CurrentFlowerName.MakeLower() ;
  m_CurrentFlowerName.Trim() ;
  m_CurrentFlowerName.GetBuffer() ;
  if ( isalpha( m_CurrentFlowerName[0] ) )
  {
    m_CurrentFlowerName.Insert( 0 , m_CurrentFlowerName[ 0 ] - 0x20 ) ;
    m_CurrentFlowerName.Delete( 1 , 1 ) ;
  }
  SetDlgItemText( IDC_FLOWER_NAME , m_CurrentFlowerName ) ;
  SetFlowerAndDirectories( 1 ) ; // Now is for one source only
}


int CUI_NViews::SetFlowerAndDirectories( int iNSources , TectoMsg * pMsg )
{
  m_iEmptyTrayCount = m_iGoodCount = m_iBadCount = m_iSideImagesWithFlowerCount = 0 ;
  FXString TectoRegistryDir( "TheFileX\\UI_NViews\\Tecto\\" ) ;
  FXRegistry Reg( TectoRegistryDir );

  CString m_TectoRootDir = Reg.GetRegiString( "ImageFiles" , "ImagingRootDir" , "E:\\ForProjects\\Tecto\\Phase2\\" ) ;
  CString DirSuffix = Reg.GetRegiString( "ImageFiles" , "SimuImagesSuffix" , "Images" ) ;
  m_CurrentSimulationDir = m_TectoRootDir + m_CurrentFlowerName + '\\' + DirSuffix + '\\';
  CString ImagesSavingDir = m_TectoRootDir + m_CurrentFlowerName + ( LPCTSTR ) GetTimeAsString( "\\" ) ;
  m_CurrentSrcImageSavingDir = ImagesSavingDir + "\\Src\\" ;
  m_CurrentResultImageSavingDir = ImagesSavingDir + "\\ResultImages\\" ;
  m_StatisticsFileName = ImagesSavingDir + "\\Statistics.dat" ;

  int iRes = SHCreateDirectoryEx( m_hWnd , ( LPCTSTR ) m_CurrentSrcImageSavingDir , NULL ) ;
  CString Msg ;
  if ( iRes != ERROR_SUCCESS && iRes != ERROR_ALREADY_EXISTS )
  {
    Msg.Format( "ERROR %d: Can't create dir %s for saving images" , iRes ,
      ( LPCTSTR ) m_CurrentSrcImageSavingDir ) ;
    m_LogView.InsertString( 0 , Msg ) ;
    if ( pMsg )
    {
      pMsg->m_Id = TM_SetFlowerError ;
      strcpy_s( ( char* ) ( pMsg->m_MsgData ) , 256 , Msg ) ;
      pMsg->m_MsgDataLen = ( DWORD ) strlen( ( char* ) ( pMsg->m_MsgData ) ) + 1 ; // trailing zero
    }
    return TM_SetFlowerError ;
  }
  else
    iRes += SHCreateDirectoryEx( m_hWnd , ( LPCTSTR ) m_CurrentResultImageSavingDir , NULL ) ;

  ofstream myfile( ( LPCTSTR ) m_StatisticsFileName , ios_base::app );
  if ( myfile.is_open() )
  {
    myfile << "File: " << ( LPCTSTR ) m_StatisticsFileName << '\n' ;
    myfile << "   #  , Results           , Tproc ms , Src File Name\n" ;
    myfile.close();
  }
  else
  {
    Msg.Format( "CUI_NViews::SetFlowerAndDirectories ERROR %s: Can't create Statistics data file %s" ,
      strerror( GetLastError() ) , ( LPCTSTR ) m_StatisticsFileName );
    m_LogView.InsertString( 0 , Msg ) ;
  }


  iRes += SHCreateDirectoryEx( m_hWnd , ( LPCTSTR ) m_CurrentSimulationDir , NULL ) ;
  if ( iRes != ERROR_SUCCESS && iRes != ERROR_ALREADY_EXISTS )
  {
    Msg.Format( "ERROR %d: Can't see/create dir %s with images" , iRes ,
      ( LPCTSTR ) m_CurrentSimulationDir ) ;
    m_LogView.InsertString( 0 , Msg ) ;
    if ( pMsg )
    {
      pMsg->m_Id = TM_SetFlowerError ;
      strcpy_s( ( char* ) ( pMsg->m_MsgData ) , 256 , Msg ) ;
      pMsg->m_MsgDataLen = ( DWORD ) strlen( ( char* ) ( pMsg->m_MsgData ) ) + 1 ; // trailing zero
    }
    return TM_SetFlowerError ;
  }
  Reg.WriteRegiString( "Common" , "LastFlowerName" , m_CurrentFlowerName ) ;
  Reg.WriteRegiInt( "Common" , "NSources" , m_iNSources = iNSources) ;

  SetDlgItemInt( IDC_NSOURCES , m_iNSources ) ;
  RoiPoints RoiPts[ 4 ] ;// index is camera
  if ( pMsg )
  {
    FlowerSetContent * pContent = ( FlowerSetContent* ) pMsg->m_MsgData ;
    memcpy_s( RoiPts , sizeof( RoiPts ) , pContent->m_ROI ,
      m_iNSources * sizeof( RoiPoints ) ) ;
  }
  
  CString Extention = Reg.GetRegiString( "ImageFiles" , "FileExtention" , "bmp" ) ;
  for ( int i = 0 ; i < min( iNSources , 3 ); i++ )
  {
    CString TectoGadgetName ;
    TectoGadgetName.Format( "Tecto%d" , i + 1 ) ;
    iRes = m_Graph.SetProperty( TectoGadgetName , "FlowerName" , m_CurrentFlowerName ) ;

    CString ProtoPattern ;
    ProtoPattern.Format( "*-%d*.%s" , i , (LPCTSTR)Extention ) ;
    CString SimulFilePattern = Reg.GetRegiString( "ImageFiles" , "SimulationNamePattern" , ProtoPattern ) ;

    CString BMPReadGadgetName ;
    BMPReadGadgetName.Format( "BMPCapture%d" , i + 1 ) ;
    iRes = m_Graph.SetProperty( BMPReadGadgetName , "Directory" , m_CurrentSimulationDir ) ;
    if ( !iRes )
    {
      CString Msg ;
      Msg.Format( "ERROR: Can't set directory %s in BMPCapture%d gadget " ,
        ( LPCTSTR ) m_CurrentSimulationDir , i + 1 ) ;
      m_LogView.InsertString( 0 , Msg ) ;
      if ( pMsg )
      {
        pMsg->m_Id = TM_SetFlowerError ;
        strcpy_s( ( char* ) ( pMsg->m_MsgData ) , 256 , Msg ) ;
        pMsg->m_MsgDataLen = ( DWORD ) strlen( ( char* ) ( pMsg->m_MsgData ) ) + 1 ; // trailing zero
      }
      return TM_SetFlowerError ;
    }
    iRes = m_Graph.SetProperty( BMPReadGadgetName , "FileName" , ProtoPattern ) ;
    if ( !iRes )
    {
      CString Msg ;
      Msg.Format( "ERROR: Can't set directory %s in BMPCapture%d gadget " ,
        ( LPCTSTR ) m_CurrentSimulationDir , i + 1 ) ;
      m_LogView.InsertString( 0 , Msg ) ;
      if ( pMsg )
      {
        pMsg->m_Id = TM_SetFlowerError ;
        strcpy_s( ( char* ) ( pMsg->m_MsgData ) , 256 , Msg ) ;
        pMsg->m_MsgDataLen = ( DWORD ) strlen( ( char* ) ( pMsg->m_MsgData ) ) + 1 ; // trailing zero
      }
      return TM_SetFlowerError ;
    }
    CString BMPCallbackPin = BMPReadGadgetName + ">>0" ;
    if ( pMsg )
    {
      if ( !m_Graph.SetDataCallBack( BMPCallbackPin ,
        ( i == 0 ) ? ImageForTectoCallBack0 :
        ( i == 1 ) ? ImageForTectoCallBack1 :
        ( i == 2 ) ? ImageForTectoCallBack2 : ImageForTectoCallBack3 , this ) )
      {
        CString Msg ;
        Msg.Format( "Can't create callback for pin %s" , BMPCallbackPin ) ;
        m_LogView.InsertString( 0 , Msg ) ;
        if ( pMsg )
        {
          pMsg->m_Id = TM_SetFlowerError ;
          strcpy_s( ( char* ) ( pMsg->m_MsgData ) , 256 , Msg ) ;
          pMsg->m_MsgDataLen = ( DWORD ) strlen( ( char* ) ( pMsg->m_MsgData ) ) + 1 ; // trailing zero
        }
        return TM_SetFlowerError ;
      };
    }
    else    // disable call backs for working without external interface
      m_Graph.SetDataCallBack( BMPCallbackPin , NULL , this ) ; 
    CString SrcRenderName ;
    SrcRenderName.Format( "BMPRender%d" , i + 1 ) ;
    m_Graph.SetProperty( SrcRenderName , "FolderPath" , m_CurrentSrcImageSavingDir ) ;
    m_Graph.SetProperty( SrcRenderName , "FileName_NoExt" , m_CurrentFlowerName ) ;

    BOOL bSetROIOnFlowerSet = Reg.GetRegiInt( "Common" , "SetROIONFlowerSet" , 0 ) ;
    if ( bSetROIOnFlowerSet && pMsg 
      && ( RoiPts[ i ].topleft != RoiPts[ i ].topright)
      && ( RoiPts[ i ].topleft != RoiPts[ i ].bottomleft ) )
    {
      RoiPoints& ThisRoi = RoiPts[ i ] ;
      CRect ROI( min( ThisRoi.topleft.x , ThisRoi.bottomleft.x ) ,
        min( ThisRoi.topleft.y , ThisRoi.topright.y ) ,
        max( ThisRoi.bottomright.x , ThisRoi.topright.x ) ,
        max( ThisRoi.bottomright.y , ThisRoi.bottomleft.y ) ) ;
      FXString SHROIData ;
      SHROIData.Format( "name=flower;xoffset=%d;yoffset=%d;width=%d;height=%d;" ,
        ROI.left , ROI.top ,
        ROI.Width() , ROI.Height() );
      CString TObjectControlPinName ;
      TObjectControlPinName.Format( "TVObjects%d<<1" , i + 1 ) ;
      m_Graph.SendText( TObjectControlPinName , SHROIData , "SetObjectProp" ) ;

      TectoRegistryDir += (( i == 0 ) ? "SideView\\" : "FrontView\\") ;
//       TectoRegistryDir += m_CurrentFlowerName + '\\' ;

      FXRegistry Reg( TectoRegistryDir );
      Reg.WriteRegiInt( m_CurrentFlowerName , "xoffset" , ROI.left ) ;
      Reg.WriteRegiInt( m_CurrentFlowerName , "yoffset" , ROI.top ) ;
      Reg.WriteRegiInt( m_CurrentFlowerName , "width" , ROI.Width() ) ;
      Reg.WriteRegiInt( m_CurrentFlowerName , "height" , ROI.Height() ) ;
    }
//     else
//     {
//       TectoRegistryDir += ( ( i == 0 ) ? "SideView\\" : "FrontView\\" ) ;
// //       TectoRegistryDir += m_CurrentFlowerName + '\\' ;
// 
//       FXRegistry Reg( TectoRegistryDir );
//       Reg.WriteRegiInt( m_CurrentFlowerName , "xoffset" , ROI.left ) ;
//       Reg.WriteRegiInt( m_CurrentFlowerName , "yoffset" , ROI.top ) ;
//       Reg.WriteRegiInt( m_CurrentFlowerName , "width" , ROI.Width() ) ;
//       Reg.WriteRegiInt( m_CurrentFlowerName , "height" , ROI.Height() ) ;
//     }
  }
  CorrectGraph() ;
  CString Msg2 ;
  Msg2.Format( "Flower name set to %s with %d image sources;  " , 
    ( LPCTSTR ) m_CurrentFlowerName , m_iNSources ) ;
  m_LogView.InsertString( 0 , Msg2 ) ;
  Msg.Format( "Image Saving Dir is %s" , ( LPCTSTR ) ImagesSavingDir ) ;
  m_LogView.InsertString( 0 , Msg ) ;
  Msg2 += Msg + ";   " ;
  Msg.Format( "Image Simulation Dir is %s" , ( LPCTSTR ) m_CurrentSimulationDir ) ;
  m_LogView.InsertString( 0 , Msg ) ;
  Msg2 += Msg + ";" ;
  return TM_OK ;
}

void CUI_NViews::ReadOrRepeatImage( bool bMayBeRepeat ) 
{
  int imageCount = GetDlgItemInt( IDC_NSOURCES ) ;
  for ( int i = 0 ; i < imageCount ; i++ )
  {
    CString TvObjPinName , TectoPinName ;
    TvObjPinName.Format( "TVObjects%d<<1" , i + 1 ) ;
    TectoPinName.Format( "Tecto%d<<0" , i + 1 ) ;
    const char * TargetPins[] = { ( LPCTSTR ) TvObjPinName , ( LPCTSTR ) TectoPinName , NULL } ;
    int iSent = m_Graph.SendText( TargetPins , "Task(1);" , "mode" ) ;

    if ( IsChecked( IDC_SAVE_RESULT ) )
    {
      CString RenderPinName ;
      RenderPinName.Format( "FRender%d<<0" , i + 1 ) ;
      CString SaveImageContent , Prefix , Suffix ;
      SaveImageContent.Format( "1,%s" , ( LPCTSTR ) m_CurrentResultImageSavingDir ) ;
      int iSent = m_Graph.SendText( RenderPinName , ( LPCTSTR ) SaveImageContent , "SaveImage" ) ;
      iSent += m_Graph.SendText( RenderPinName , "<label>" , "ImageSavePrefix" ) ; // video frame label will be used as prefix
//       Suffix.Format( "-%d" , i ) ;
      iSent += m_Graph.SendText( RenderPinName , "" , "ImageSaveSuffix" ) ;
    }
  }
  if ( bMayBeRepeat && (GetAsyncKeyState( VK_CONTROL ) && 0x8000) )
  { // CNTRL is pressed - repeat
    m_Graph.SendQuantity( "Buffer1<>0" , 1 ) ;
    if ( m_iNSources > 1 )
      m_Graph.SendQuantity( "Buffer2<>0" , 1 ) ;
  }
  else
  { // CNTRL is not pressed - get next
    m_Graph.SendQuantity( "Trigger2<>0" , 1 ) ;
    m_Graph.SendQuantity( "BMPCapture1<<0" , 1 ) ;
  }
  if ( bMayBeRepeat )
  {
    m_ImageProcessingOrdered = ( bMayBeRepeat ) ? IPO_OrdredByGetNext : IPO_OrderedByBatch ;
  }
  m_dRequestTime_ms = GetHRTickCount() ;
}

void CUI_NViews::OnBnClickedGetNext()
{
  ReadOrRepeatImage( true ) ;
}

bool CUI_NViews::CorrectGraph()
{
  FXRegistry Reg( "TheFileX\\UI_NViews\\Tecto\\SideView" );
  
  int iNSuccessActions = 0 ;
    // Mass normalize control
  int iNormalizePercent = Reg.GetRegiInt(
    m_CurrentFlowerName , "Normalize_Percent" , 2 ) ;
  if ( iNormalizePercent <= 0 )  // switch off
    iNSuccessActions += m_Graph.SetWorkingMode( "VideoMassNormalize1" , GWM_Transmit ) ;
  else
  { // Switch on and set parameter (percent)
    iNSuccessActions += m_Graph.SetWorkingMode( "VideoMassNormalize1" , GWM_Process ) ;
    m_Graph.SetProperty( "VideoMassNormalize1" , "Percent" , iNormalizePercent ) ;
  }

      // Video HighPass control
  int iHighPassPar = Reg.GetRegiInt(
    m_CurrentFlowerName , "HighPass1DV1_par" , 700 ) ;
  if ( iHighPassPar <= 0 )  // switch off
    iNSuccessActions += m_Graph.SetWorkingMode( "VideoHighPass1DV1" , GWM_Transmit ) ;
  else
  { // Switch on and set parameter 
    iNSuccessActions += m_Graph.SetWorkingMode( "VideoHighPass1DV1" , GWM_Process ) ;
    m_Graph.SetProperty( "VideoHighPass1DV1" , "ConvParameter" , iHighPassPar ) ;
  }

      // Video Sharpen control
  int iSharpenPar = Reg.GetRegiInt(
    m_CurrentFlowerName , "Sharpen1_par" , 700 ) ;
  if ( iSharpenPar <= 0 )  // switch off
    iNSuccessActions += m_Graph.SetWorkingMode( "VideoSharpen1" , GWM_Transmit ) ;
  else
  { // Switch on and set parameter 
    iNSuccessActions += m_Graph.SetWorkingMode( "VideoSharpen1" , GWM_Process ) ;
    m_Graph.SetProperty( "VideoSharpen1" , "ConvParameter" , iSharpenPar ) ;
  }

        // Video Percent Binarize control
  int iBinarize_perc = Reg.GetRegiInt(
    m_CurrentFlowerName , "Binarize_percent" , 2 ) ;
  if ( iBinarize_perc <= 0 )  // switch off
    iNSuccessActions += m_Graph.SetWorkingMode( "VideoPercentBinarize1" , GWM_Transmit ) ;
  else
  { // Switch on and set parameter 
    iNSuccessActions += m_Graph.SetWorkingMode( "VideoPercentBinarize1" , GWM_Process ) ;
    m_Graph.SetProperty( "VideoPercentBinarize1" , "Percent" , iBinarize_perc ) ;
  }

    // Video Erosion control
  int iErosionPar = Reg.GetRegiInt(
    m_CurrentFlowerName , "ErosionN" , 2 ) ;
  if ( iErosionPar <= 0 )  // switch off
  {
    iNSuccessActions += m_Graph.SetWorkingMode( "VideoErode1" , GWM_Transmit ) ;
//     Reg.WriteRegiDouble( 
//       m_CurrentFlowerName , "ThicknessCorrection_pix" , 0. ) ;
//     m_Graph.SetProperty( "Tecto1" , "ThickCorrect_pix" , 0. ) ;
  }
  else
  { // Switch on and set parameter 
    iNSuccessActions += m_Graph.SetWorkingMode( "VideoErode1" , GWM_Process ) ;
    m_Graph.SetProperty( "VideoErode1" , "NInversed" , iErosionPar ) ;
//     Reg.WriteRegiDouble(
//       m_CurrentFlowerName , "ThicknessCorrection_pix" , ( double ) iErosionPar * 0.5 ) ;
//     m_Graph.SetProperty( "Tecto1" , "ThickCorrect_pix" , ( double ) iErosionPar * 0.5 ) ;
  }

  double dTVObjFlowerThreshold = Reg.GetRegiDouble( m_CurrentFlowerName , "FlowerThreshold_1" , 0.5 ) ;
  CString ControlMsgForTVObjects ;
  ControlMsgForTVObjects.Format( "name=flower;thres=%.6f;" , dTVObjFlowerThreshold ) ;
  m_Graph.SendText( "TVObjects1<<1" , ControlMsgForTVObjects , "SetObjectProp" ) ;
  return ( iNSuccessActions >= 5 ) ;
}

bool CUI_NViews::SaveGraphCorrections()
{
  FXRegistry Reg( "TheFileX\\UI_NViews\\Tecto\\SideView" );

  int iNSuccessActions = 0 ;
  GadgetWorkinMode WorkingMode = GWM_Reject ;
  int iValue ;
  
  // Mass normalize control
  if ( m_Graph.GetWorkingMode( "VideoMassNormalize1" , (int&)WorkingMode ) )
  {
    if ( WorkingMode == GWM_Process )
    {
      if ( m_Graph.GetProperty( "VideoMassNormalize1" , "Percent" , iValue ) )
      {
        Reg.WriteRegiInt( m_CurrentFlowerName , "Normalize_Percent" , iValue ) ;
        iNSuccessActions++ ;
      }
      else
        m_LogView.AddString( "Can't read property 'Percent' from gadget 'VideoMassNormalize1'" ) ;
    }
    else  // no normalization
    {
      Reg.WriteRegiInt( m_CurrentFlowerName , "Normalize_Percent" , 0 ) ;
      iNSuccessActions++ ;
    }

  }
  else
    m_LogView.AddString( "Can't read working mode from gadget 'VideoMassNormalize1'" ) ;



      // Video HighPass control
  if ( m_Graph.GetWorkingMode( "VideoHighPass1DV1" , ( int& ) WorkingMode ) )
  {
    if ( WorkingMode == GWM_Process )
    {
      if ( m_Graph.GetProperty( "VideoHighPass1DV1" , "ConvParameter" , iValue ) )
      {
        Reg.WriteRegiInt( m_CurrentFlowerName , "HighPass1DV1_par" , iValue ) ;
        iNSuccessActions++ ;
      }
      else
        m_LogView.AddString( "Can't read property 'ConvParameter' from gadget 'VideoHighPass1DV1'" ) ;
    }
    else  // no high pass filtration
    {
      Reg.WriteRegiInt( m_CurrentFlowerName , "HighPass1DV1_par" , 0 ) ;
      iNSuccessActions++ ;
    }

  }
  else
    m_LogView.AddString( "Can't read working mode from gadget 'VideoHighPass1DV1'" ) ;

      // Video Sharpen control
  if ( m_Graph.GetWorkingMode( "VideoSharpen1" , ( int& ) WorkingMode ) )
  {
    if ( WorkingMode == GWM_Process )
    {
      if ( m_Graph.GetProperty( "VideoSharpen1" , "ConvParameter" , iValue ) )
      {
        Reg.WriteRegiInt( m_CurrentFlowerName , "Sharpen1_par" , iValue ) ;
        iNSuccessActions++ ;
      }
      else
        m_LogView.AddString( "Can't read property 'ConvParameter' from gadget 'VideoSharpen1'" ) ;
    }
    else
    {
      Reg.WriteRegiInt( m_CurrentFlowerName , "Sharpen1_par" , 0 ) ; // no sharpen
      iNSuccessActions++ ;
    }

  }
  else
    m_LogView.AddString( "Can't read working mode from gadget 'VideoSharpen1'" ) ;

        // Video Percent Binarize control
  if ( m_Graph.GetWorkingMode( "VideoPercentBinarize1" , ( int& ) WorkingMode ) )
  {
    if ( WorkingMode == GWM_Process )
    {
      if ( m_Graph.GetProperty( "VideoPercentBinarize1" , "Percent" , iValue ) )
      {
        Reg.WriteRegiInt( m_CurrentFlowerName , "Binarize_percent" , iValue ) ;
        iNSuccessActions++ ;
      }
      else
        m_LogView.AddString( "Can't read property 'Percent' from gadget 'VideoPercentBinarize1'" ) ;
    }
    else
    {
      Reg.WriteRegiInt( m_CurrentFlowerName , "Binarize_percent" , 0 ) ; // no binarization
      iNSuccessActions++ ;
    }
  }
  else
    m_LogView.AddString( "Can't read working mode from gadget 'VideoPercentBinarize1'" ) ;

    // Video Erosion control
  if ( m_Graph.GetWorkingMode( "VideoErode1" , ( int& ) WorkingMode ) )
  {
    if ( WorkingMode == GWM_Process )
    {
      if ( m_Graph.GetProperty( "VideoErode1" , "NInversed" , iValue ) )
      {
        Reg.WriteRegiInt( m_CurrentFlowerName , "NInversed" , iValue ) ;
        iNSuccessActions++ ;
      }
      else
        m_LogView.AddString( "Can't read property 'Percent' from gadget 'VideoErode1'" ) ;
    }
    else
    {
      Reg.WriteRegiInt( m_CurrentFlowerName , "NInversed" , 0 ) ; // no erosion
      iNSuccessActions++ ;
    }

  }
  else
    m_LogView.AddString( "Can't read working mode from gadget 'VideoErode1'" ) ;

  double dTVObjFlowerThreshold = GetVideoObjectThreshold( "TVObjects1" , "flower" ) ;
  if ( dTVObjFlowerThreshold > 0. )
  {
    Reg.WriteRegiDouble( m_CurrentFlowerName , "FlowerThreshold_1" , dTVObjFlowerThreshold ) ;

  }
  return ( iNSuccessActions >= 5 ) ;
}

double CUI_NViews::GetVideoObjectThreshold( LPCTSTR pTVObjectGadget , LPCTSTR pObject )
{
  char Buf[ 8000 ] ;
  ULONG uiLen = sizeof( Buf ) ;
  CString TVObjectProp ;
  if ( !m_Graph.PrintProperties( pTVObjectGadget , Buf , uiLen ) )
  {
    if ( uiLen == 0 )
      return 0. ; // no gadget
    if ( uiLen >= sizeof( Buf ) )
    {
      char * pBuf = new char[ uiLen + 10 ] ;
      if ( !m_Graph.PrintProperties( pTVObjectGadget , Buf , uiLen ) )
        return 0. ;
      else
        TVObjectProp = pBuf ;
      delete[] pBuf ;
    }
  }
  else
    TVObjectProp = Buf ;
  CString ObjectByName("name=") ;
  ObjectByName += pObject ;
  ObjectByName += ';' ;
  int iPos = TVObjectProp.Find( ObjectByName ) ;
  if ( iPos < 0 )
    return 0. ; // Object is not found
  int iClosePos = TVObjectProp.Find( ')' , iPos ) ;
  int iThresPos = TVObjectProp.Find( "thres" , iPos ) ;
  if ( iThresPos > iClosePos )
    return 0. ; // Threshold property is not found
  int iEquPos = TVObjectProp.Find( '=' , iThresPos ) ;
  if ( iEquPos < 0 )
    return 0. ;
  const char * pValue = (( LPCTSTR ) TVObjectProp) + iEquPos + 1 ;
  double dThresVal = atof( pValue ) ;
  return dThresVal ;
}

void CUI_NViews::OnBnClickedSaveSrc()
{
}

void CUI_NViews::OnBnClickedSaveResult()
{
}

void CUI_NViews::OnBnClickedResult1()
{
//   BOOL bRes = m_Graph.ConnectRendererToWindow( 
//     NULL , NULL , m_RenderNames[ 0 ].c_str() ) ;
//   Sleep( 50 ) ;
  BOOL bRes = m_Graph.ConnectRendererToWindow( m_Renderers[0] ,
    m_OnScreenRenderNames[ 0 ].c_str() , "FRender1" ) ;
}

void CUI_NViews::OnBnClickedSrc1()
{
//   BOOL bRes = m_Graph.ConnectRendererToWindow(
//     NULL , NULL , m_RenderNames[ 0 ].c_str() ) ;
//   Sleep( 50 ) ;
  BOOL bRes = m_Graph.ConnectRendererToWindow( m_Renderers[ 0 ] ,
    "Camera 0 Source" , "FRender7" ) ;
}

void CUI_NViews::OnBnClickedResult2()
{
//   BOOL bRes = m_Graph.ConnectRendererToWindow(
//     NULL , NULL , m_RenderNames[ 1 ].c_str() ) ;
//   Sleep( 50 ) ;
  BOOL bRes = m_Graph.ConnectRendererToWindow( m_Renderers[ 1 ] ,
    m_OnScreenRenderNames[ 1 ].c_str() , "FRender2" ) ;
}

void CUI_NViews::OnBnClickedSrc2()
{
//   BOOL bRes = m_Graph.ConnectRendererToWindow(
//     NULL , NULL , m_RenderNames[ 1 ].c_str() ) ;
//   Sleep( 50 ) ;
  BOOL bRes = m_Graph.ConnectRendererToWindow( m_Renderers[ 1 ] ,
    "Camera 1 Source" , "FRender8" ) ;
}

void CUI_NViews::OnBnClickedResult1On2()
{
//   BOOL bRes = m_Graph.ConnectRendererToWindow(
//     NULL , NULL , m_RenderNames[ 1 ].c_str() ) ;
//   Sleep( 50 ) ;
  BOOL bRes = m_Graph.ConnectRendererToWindow( m_Renderers[ 1 ] ,
    "Camera 1 Source" , "FRender1" ) ;
}


void CUI_NViews::OnBnClickedBatch()
{
  if ( IsChecked( IDC_BATCH ) )
  {
    SetFlowerAndDirectories( m_iNSources , NULL ) ;
    CString BMPReadGadgetName ;
    BMPReadGadgetName.Format( "BMPCapture1" ) ;
    int iRes = m_Graph.SetProperty( BMPReadGadgetName , "Loop" , 0 ) ; // disable looping through images
    if ( !iRes )
    {
      CString Msg ;
      Msg.Format( "ERROR: Can't reset image looping in BMPCapture1 gadget " ) ;
      m_LogView.InsertString( 0 , Msg ) ;
      return ;
    }
    ReadOrRepeatImage( false ) ;
  }
  else
  {
    FXString ForStatistics ;
    ForStatistics.Format( "Batch is finished for %d plants: %d good, %d bad , %d empty.\n FileName=%s\n" , m_iSideImagesWithFlowerCount , m_iGoodCount , m_iBadCount , 
      m_iEmptyTrayCount , ( LPCTSTR ) m_StatisticsFileName ) ;
    ofstream myfile( ( LPCTSTR ) m_StatisticsFileName , ios_base::app );
    if ( myfile.is_open() )
    {
      myfile << ( LPCTSTR ) ForStatistics ;
      myfile.close();
    }
    else
    {
      FXString Msg ;
      Msg.Format( "CUI_NViews::OnAnswerForTectoLib ERROR %s: Can't write Statistics data to file %s" ,
        strerror( GetLastError() ) , ( LPCTSTR ) m_StatisticsFileName );
      m_LogView.InsertString( 0 , Msg ) ;
    }

  }
}

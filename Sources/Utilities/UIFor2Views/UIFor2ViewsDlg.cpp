
// UIFor2ViewsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "framework.h"
#include "UIFor2Views.h"
#include "UIFor2ViewsDlg.h"
#include "DlgProxy.h"
#include "afxdialogex.h"
#include <habitat.h>
#include <gadgets/stdsetup.h>
#include <helpers/propertykitEx.h>
#include <fxfc/FXRegistry.h>
#include <gadgets/quantityframe.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CUIFor2ViewsDlg * CUIFor2ViewsDlg::m_pCurrentDlg = NULL;
CUIFor2ViewsDlg * g_thisDlg = NULL ;
static const UINT VM_TVDB400_SHOWVOSETUPDLG = ::RegisterWindowMessage(
  _T( "Tvdb400_ShowVOSetupDialog" ) );
static const UINT WM_CAMERA_CALLBACK = ::RegisterWindowMessage(
  _T( "CallBackFromCamera" ) );


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
	EnableActiveAccessibility();
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
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
    return (HBRUSH) CreateSolidBrush( MBackgroundColor );  // color for the empty area of the control
}


void __stdcall PrintGraphMessage(
  int msgLevel , const char * src , int msgId , const char * msgText )
{
  char buf[ 2000 ] ;
  sprintf_s( buf , 2000 , "%d %s %d %s\n" , msgLevel , src , msgId , msgText );
  if ( g_thisDlg && (msgLevel >= 0) )
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
  CUIFor2ViewsDlg* pWnd = ( CUIFor2ViewsDlg* ) wParam ;
  if ( pWnd && IsWindow( pWnd->m_hWnd ) )
    return (((CUIFor2ViewsDlg*) wParam)->PostMessageA( WM_CAMERA_CALLBACK , 1 ) != FALSE) ;
  return false ;
}
bool __stdcall rcbCam2( CallBackDataA& Data , void* wParam )
{
  CUIFor2ViewsDlg* pWnd = ( CUIFor2ViewsDlg* ) wParam ;
  if ( pWnd && IsWindow( pWnd->m_hWnd ) )
    return ( ( ( CUIFor2ViewsDlg* ) wParam )->PostMessageA( WM_CAMERA_CALLBACK , 2 ) != FALSE ) ;
  return false ;
}


// CUIFor2ViewsDlg dialog


IMPLEMENT_DYNAMIC(CUIFor2ViewsDlg, CDialogEx);

CUIFor2ViewsDlg::CUIFor2ViewsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_UIFOR2VIEWS_DIALOG, pParent)
  , m_bCam1CallBackPassed( false )
  , m_bCam2CallBackPassed( false )
  , m_DataCam1( DATA_IMAGE) 
  , m_DataCam2( DATA_IMAGE )
  , m_iHLinePos( 0 )
  , m_iZOffset_um( 0 )
  , m_iYOffset_um( 0 )
  , m_iXOffset_um( 0 )
  , m_iTolerance_um(5)
  , m_iElectrodeWidth_Tenth(20)
{
	EnableActiveAccessibility();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pAutoProxy = nullptr;
  FXRegistry Reg( "TheFileX\\Micropoint" );

}

CUIFor2ViewsDlg::~CUIFor2ViewsDlg()
{
	// If there is an automation proxy for this dialog, set
	//  its back pointer to this dialog to null, so it knows
	//  the dialog has been deleted.
	if (m_pAutoProxy != nullptr)
		m_pAutoProxy->m_pDialog = nullptr;
}

void CUIFor2ViewsDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_CAM1_STATUS, m_Cam1StatusLED);
  DDX_Control(pDX, IDC_CAM2_STATUS, m_Cam2StatusLED);
  DDX_Control(pDX, IDC_LOG_LIST, m_LogView);
  DDX_Control(pDX, IDC_TOLERANCE_SPIN, m_SpinTolerance);
  DDX_Control( pDX , IDC_ELECTRODE_WIDTH_SPIN , m_SpinElectrodeWidth );
  DDX_Control( pDX , IDC_WEDGE_WIDTH_SPIN , m_SpinWedgeWidth );
  DDX_Control( pDX , IDC_HOR_LINE_SPIN , m_SpinHorLinePos );
  DDX_Control( pDX , IDC_VERT_LINE_SPIN , m_SpinVertLinePos );
  DDX_Control( pDX , IDC_WEDGE_X_MEAS_SPIN , m_SpinWedgeXMeasureYPos ) ;

  switch (m_AppMode)
  {
  case AM_Green:
    DDX_Control( pDX , IDC_HLINE_SPIN , m_SpinHLine );
    DDX_Control(pDX, IDC_OFFSET_Z_SPIN, m_SpinOffsetZ);
    DDX_Control(pDX, IDC_OFFSET_Y_SPIN, m_SpinOffsetY);
    DDX_Control(pDX, IDC_OFFSET_X_SPIN, m_SpinOffsetX);
    DDX_Text(pDX, IDC_HLINE_POS, m_iHLinePos);
    DDX_Text(pDX, IDC_OFFSET_Z, m_iZOffset_um);
    DDX_Text(pDX, IDC_OFFSET_Y, m_iYOffset_um);
    DDX_Text(pDX, IDC_OFFSET_X, m_iXOffset_um);
    break;
  case AM_Holes:
    DDX_Control(pDX, IDC_OFFSET_Y_SPIN, m_SpinOffsetY);
    DDX_Control(pDX, IDC_OFFSET_X_SPIN, m_SpinOffsetX);
    DDX_Text(pDX, IDC_OFFSET_Y, m_iLineDist_T);
    DDX_Text(pDX, IDC_OFFSET_X, m_iSideXOffset_Tx10);
    break;
  }
  DDX_Text(pDX, IDC_TOLERANCE_UM, m_iTolerance_um);
  DDX_Control(pDX, IDC_SIDE_VIEW, m_SideResultView);
  DDX_Control(pDX, IDC_FRONT_VIEW, m_FrontResultView);
  DDX_Control(pDX, IDC_MULTIWEDGE, m_MultiEdgeCheckBox);
  DDX_Text( pDX , IDC_ELECTRODE_WIDTH_T , m_iElectrodeWidth_Tenth );
  DDV_MinMaxInt( pDX , m_iElectrodeWidth_Tenth , 5 , 100 );
  DDX_Text( pDX , IDC_WEDGE_WIDTH_T , m_iWedgeWidth_Tenth );
  DDV_MinMaxInt( pDX , m_iWedgeWidth_Tenth , 10 , 400 );
}

BEGIN_MESSAGE_MAP(CUIFor2ViewsDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
  ON_MESSAGE( USER_WM_LOG_MSG , &CUIFor2ViewsDlg::OnLogMsg )
  ON_BN_CLICKED( IDC_WORK_MODE , &CUIFor2ViewsDlg::OnBnClickedWorkMode )
  ON_BN_CLICKED( IDC_GRAPH_VIEW , &CUIFor2ViewsDlg::OnBnClickedGraphView )
  ON_BN_CLICKED( IDC_SETUP_VIEW , &CUIFor2ViewsDlg::OnBnClickedSetupView )
  ON_REGISTERED_MESSAGE( VM_TVDB400_SHOWVOSETUPDLG , OnShowVOSetupDialog )
  ON_REGISTERED_MESSAGE( WM_CAMERA_CALLBACK , OnCameraMsg )
  ON_WM_TIMER()
  ON_BN_CLICKED( IDC_PROTECT , &CUIFor2ViewsDlg::OnBnClickedProtect )
  ON_NOTIFY( UDN_DELTAPOS , IDC_HLINE_SPIN , &CUIFor2ViewsDlg::OnDeltaposHlineSpin )
  ON_NOTIFY( UDN_DELTAPOS , IDC_OFFSET_Z_SPIN , &CUIFor2ViewsDlg::OnDeltaposOffsetZSpin )
  ON_NOTIFY( UDN_DELTAPOS , IDC_OFFSET_Y_SPIN , &CUIFor2ViewsDlg::OnDeltaposOffsetYSpin )
  ON_NOTIFY( UDN_DELTAPOS , IDC_OFFSET_X_SPIN , &CUIFor2ViewsDlg::OnDeltaposOffsetXSpin )
  ON_NOTIFY( UDN_DELTAPOS , IDC_TOLERANCE_SPIN , &CUIFor2ViewsDlg::OnDeltaposToleranceSpin )
  ON_EN_CHANGE( IDC_OFFSET_Z , &CUIFor2ViewsDlg::OnEnChangeOffsetZ )
  ON_EN_CHANGE( IDC_OFFSET_Y , &CUIFor2ViewsDlg::OnEnChangeOffsetY )
  ON_EN_CHANGE( IDC_OFFSET_X , &CUIFor2ViewsDlg::OnEnChangeOffsetX )
  ON_EN_CHANGE( IDC_TOLERANCE_UM , &CUIFor2ViewsDlg::OnEnChangeTolerance )
  ON_WM_CTLCOLOR()
  ON_BN_CLICKED( IDC_MULTIWEDGE , &CUIFor2ViewsDlg::OnBnClickedMultiwedge )
  ON_EN_CHANGE( IDC_HLINE_POS , &CUIFor2ViewsDlg::OnEnChangeHlinePos )
  ON_STN_CLICKED( IDC_EXPOSURE_FRONT_CAPTION , &CUIFor2ViewsDlg::OnStnClickedExposureFrontCaption )
  ON_EN_CHANGE( IDC_EXPOSURE_FRONT , &CUIFor2ViewsDlg::OnEnChangeExposureFront )
  ON_EN_CHANGE( IDC_EXPOSURE_SIDE , &CUIFor2ViewsDlg::OnEnChangeExposureSide )
  ON_BN_CLICKED( IDC_SAVE_SIDE_VIDEO , &CUIFor2ViewsDlg::OnBnClickedSaveSideVideo )
  ON_BN_CLICKED( IDC_SAVE_BOTH_VIDEO , &CUIFor2ViewsDlg::OnBnClickedSaveBothVideo )
  ON_BN_CLICKED( IDC_SAVE_FRONT_VIDEO , &CUIFor2ViewsDlg::OnBnClickedSaveFrontVideo )
  ON_BN_CLICKED( IDC_SAVE_ONE_SIDE_BMP , &CUIFor2ViewsDlg::OnBnClickedSaveOneSideBmp )
  ON_BN_CLICKED( IDC_SAVE_ONE_FRONT_BMP , &CUIFor2ViewsDlg::OnBnClickedSaveOneFrontBmp )
  ON_BN_CLICKED( IDC_DOWN_LIGHT , &CUIFor2ViewsDlg::OnBnClickedDownLight )
  ON_BN_CLICKED( IDC_FRONT_LIGHT , &CUIFor2ViewsDlg::OnBnClickedFrontLight )
  ON_BN_CLICKED( IDC_FRONT_EXP_PLUS , &CUIFor2ViewsDlg::OnBnClickedFrontExpPlus )
  ON_BN_CLICKED( IDC_FRONT_EXP_MINUS , &CUIFor2ViewsDlg::OnBnClickedFrontExpMinus )
  ON_BN_CLICKED( IDC_SIDE_EXP_PLUS , &CUIFor2ViewsDlg::OnBnClickedSideExpPlus )
  ON_BN_CLICKED( IDC_SIDE_EXP_MINUS , &CUIFor2ViewsDlg::OnBnClickedSideExpMinus )
  ON_EN_CHANGE( IDC_ELECTRODE_WIDTH_T , &CUIFor2ViewsDlg::OnEnChangeElectrodeWidthT )
  ON_STN_CLICKED( IDC_STATIC7 , &CUIFor2ViewsDlg::OnStnClickedStatic7 )
  ON_EN_CHANGE( IDC_WEDGE_WIDTH_T , &CUIFor2ViewsDlg::OnEnChangeWedgeWidthT )
  ON_EN_CHANGE( IDC_HOR_LINE_POS , &CUIFor2ViewsDlg::OnEnChangeHorLinePos )
  ON_EN_CHANGE( IDC_VERT_LINE_POS , &CUIFor2ViewsDlg::OnEnChangeVertLinePos )
  ON_EN_CHANGE( IDC_WEDGE_X_MEAS_POS , &CUIFor2ViewsDlg::OnEnChangeWedgeXMeasPos )
  ON_WM_PARENTNOTIFY()
  ON_BN_CLICKED( IDC_GRAPH_SAVE , &CUIFor2ViewsDlg::OnBnClickedGraphSave )
END_MESSAGE_MAP()

// CUIFor2ViewsDlg message handlers

BOOL CUIFor2ViewsDlg::OnInitDialog()
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
  HICON hIcon = LoadIcon( AfxGetInstanceHandle() , MAKEINTRESOURCE( IDI_ICON1 ) );
  SetIcon(hIcon, TRUE);			// Set big icon
	//SetIcon(hIcon, FALSE);		// Set small icon

  m_BlackLed.LoadBitmap( IDB_BLACK_LED ) ;
  m_BlueLed.LoadBitmap( IDB_BLUE_LED ) ;
  m_YellowLed.LoadBitmap( IDB_YELLOW_LED ) ;
  m_RedLed.LoadBitmap( IDB_RED_LED ) ;
  m_GreenLed.LoadBitmap( IDB_GREEN_LED ) ;

  m_Cam1StatusLED.SetBitmap( m_RedLed ) ;
  m_Cam2StatusLED.SetBitmap( m_RedLed ) ;


  LoadNamesFromRegistry() ;
  g_thisDlg = this ;
  bool bRes = m_Graph.Init( m_GraphName , PrintGraphMessage , false ) ;
  if ( !bRes )
  {
    
    FXString Content("Can't load graph: ") ;
    Content += ( m_Graph.m_EvaluationMsg.IsEmpty() ) ? m_LastErrorMsg : m_Graph.m_EvaluationMsg ;
    MessageBox( ( LPCTSTR ) Content , m_GraphName , MB_ICONERROR | MB_OK ) ;

    PostQuitMessage( -5 ) ;
    return FALSE ;
  }
  CWnd * pSideRender = GetDlgItem( IDC_CAM2 ) ;
  CWnd * pFrontRender = GetDlgItem( IDC_CAM1 ) ;
  if ( !m_Graph.ConnectRendererToWindow( pSideRender , _T( "Front" ) , "FRender1"  )
    || !m_Graph.ConnectRendererToWindow( pFrontRender , _T( "Side" ) , "FRender2" )
    //|| !AttachVideoHandler(GetDlgItem(IDC_VW_OSC),          GRAPHS_NAMES[MAIN],       RENDER_M_PLOT_GRAPH          )

    //|| !AttachVideoHandler(GetDlgItem(IDC_VW_COM_ECHO),     GRAPHS_NAMES[CONTROLLER], RENDER_C_TEXT_COM_PORT_ECHO)
    )
  {
    m_Graph.Disconnect() ;
    return FALSE;
  }

  FXRegistry Reg( "TheFileX\\Micropoint" );
  CString Caption = Reg.GetRegiString( "MPP_Green" , "ApplicationCaption" , "Holes Erosion Navigator" ) ;
  CString LeftCaption = Reg.GetRegiString( "MPP_Green" , "LeftCaption" , "Front View" ) ;
  CString RightCaption = Reg.GetRegiString( "MPP_Green" , "RightCaption" , "Side View" ) ;
  SetWindowText( Caption ) ;
  GetDlgItem( IDC_STATIC_LEFTCAPTION )->SetWindowText( LeftCaption ) ;
  GetDlgItem( IDC_STATIC_RIGHTCAPTION )->SetWindowText( RightCaption ) ;
  m_AppMode = (ApplicationMode)Reg.GetRegiInt( "MPP_Green" , "AppMode(0-Green,1-Holes)" , 1 ) ;
  

  switch ( m_AppMode )
  {
  case AM_Green:
    {
      SetControlsForGreenMachine() ;
    }
    break ;
  case AM_Holes:
    {
      SetControlsForHoles() ;
    }
    break ;
  }

  // Adjust Mode button for colors changing
  ((CMFCButton*) GetDlgItem( IDC_WORK_MODE ))->EnableWindowsTheming( FALSE ) ;
  ((CMFCButton*) GetDlgItem( IDC_WORK_MODE ))->m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
  ((CMFCButton*) GetDlgItem( IDC_WORK_MODE ))->m_bTransparent = false;

  m_Graph.SetDataCallBack( (LPCTSTR) (m_RotateName + "1>>0") , rcbCam1 , this ) ;
  m_Graph.SetDataCallBack( (LPCTSTR) (m_RotateName + "2>>0") , rcbCam2 , this ) ;

  m_Graph.StartGraph();

//  ((CMFCButton*) GetDlgItem( IDC_WORK_MODE ))->EnableMenuFont( FALSE ) ;


  SetTimer( EvDrawTimer , 200 , NULL  ) ;
  m_Graph.SetProperty( "FrontCam" , "Format" , 17301505 ) ;
  ( ( CButton* ) GetDlgItem( IDC_DOWN_LIGHT ) )->SetCheck( BST_CHECKED ) ;
  ( ( CButton* ) GetDlgItem( IDC_FRONT_LIGHT ) )->SetCheck( BST_CHECKED ) ;
  m_Graph.SendText( "UsbRelayGadget1<<0" , "Chan=1; Value=1;" , "SwitchDownLight" ) ;
  m_Graph.SendText( "UsbRelayGadget1<<0" , "Chan=2; Value=1;" , "SwitchFrontLight" ) ;
  Sleep( 500 ) ;
  m_Graph.SetProperty( "FrontCam" , "Format" , 34603067 ) ;
  return TRUE;  // return TRUE  unless you set the focus to a control
}

void CUIFor2ViewsDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
void CUIFor2ViewsDlg::SetControlsForGreenMachine()
{
  FXRegistry Reg( "TheFileX\\Micropoint" );
  m_CalculatorNames.Add( Reg.GetRegiString("MPP_Green", "CalculatorName", "MPPT_Green1") );
  m_CalculatorNames.Add( Reg.GetRegiString("MPP_Green", "CalculatorName", "MPPT_Green2") );

  static CFont WModeFont ;
  static CFont LogBoxFont ;
  WModeFont.CreatePointFont( 200 , _T( "Arial" ) ) ;
  GetDlgItem( IDC_WORK_MODE )->SetFont( &WModeFont ) ;
  LogBoxFont.CreatePointFont( 80 , _T( "Arial" ) ) ;
  GetDlgItem( IDC_LOG_LIST )->SetFont( &LogBoxFont ) ;
  SetWorkingMode( WM_Green_Measure ) ;

  m_SpinHLine.SetBuddy( GetDlgItem( IDC_HLINE_POS ) ) ;
  m_SpinOffsetZ.SetBuddy( GetDlgItem( IDC_OFFSET_Z ) ) ;
  m_SpinOffsetY.SetBuddy( GetDlgItem( IDC_OFFSET_Y ) ) ;
  m_SpinOffsetX.SetBuddy( GetDlgItem( IDC_OFFSET_X ) ) ;
  m_SpinTolerance.SetBuddy( GetDlgItem( IDC_TOLERANCE_UM ) ) ;
  m_SpinWedgeXMeasureYPos.SetBuddy( GetDlgItem( IDC_WEDGE_X_MEAS_POS ) ) ;
  m_iWedgeXMeasPos_pix = Reg.GetRegiInt( "MPP_Holes" , "WedgeXMeasureYPos" , 1200 );

  m_iHorLinePos_pix = m_iHLinePos = Reg.GetRegiInt(
    "MPP_Green" , "HLinePos_pix" , 1700 );
  m_iVertLinePos_pix = Reg.GetRegiInt(
    "MPP_Green" , "VLinePos_pix" , 600 );
  FXString OffZAsString = Reg.GetRegiString(
    "MPP_Green" , "ZOffset_um" , "100" ) ;
  FXString OffYAsString = Reg.GetRegiString(
    "MPP_Green" , "YOffset_um" , "-20" ) ;
  FXString OffXAsString = Reg.GetRegiString(
    "MPP_Green" , "XOffset_um" , "0" ) ;

  m_iZOffset_um = atoi( OffZAsString ) ;
  m_iYOffset_um = atoi( OffYAsString ) ;
  m_iXOffset_um = atoi( OffXAsString ) ;

  FXString MaxAsString = Reg.GetRegiString(
    "MPP_Green" , "HLinePosMax_pix" , "1900" ) ;
  int iLinePosMax = atoi( MaxAsString ) ;
  m_SpinHLine.SetRange32( 0 , iLinePosMax ) ;
  m_SpinHLine.SetPos32( m_iHLinePos ) ;
  int iMin , iMax ;
  m_SpinHLine.GetRange32( iMin , iMax ) ;

  FXString MinAsString = Reg.GetRegiString(
    "MPP_Green" , "OffsetMinZ_um" , "-200" ) ;
  int iOffsetMin = atoi( MinAsString ) ;
  MaxAsString = Reg.GetRegiString(
    "MPP_Green" , "OffsetMaxZ_um" , "200" ) ;
  int iOffsetMax = atoi( MaxAsString ) ;
  m_SpinOffsetZ.SetRange32( iOffsetMin , iOffsetMax ) ;
  m_SpinOffsetZ.GetRange32( iMin , iMax ) ;
  m_SpinOffsetZ.SetPos32( m_iZOffset_um ) ;

  MinAsString = Reg.GetRegiString(
    "MPP_Green" , "OffsetMinY_um" , "-200" ) ;
  iOffsetMin = atoi( MinAsString ) ;
  MaxAsString = Reg.GetRegiString(
    "MPP_Green" , "OffsetMaxY_um" , "200" ) ;
  iOffsetMax = atoi( MaxAsString ) ;
  m_SpinOffsetY.SetRange32( iOffsetMin , iOffsetMax ) ;
  m_SpinOffsetY.GetRange32( iMin , iMax ) ;
  m_SpinOffsetY.SetPos32( m_iYOffset_um ) ;

  MinAsString = Reg.GetRegiString(
    "MPP_Green" , "OffsetMinX_um" , "-100" ) ;
  iOffsetMin = atoi( MinAsString ) ;
  MaxAsString = Reg.GetRegiString(
    "MPP_Green" , "OffsetMaxX_um" , "100" ) ;
  iOffsetMax = atoi( MaxAsString ) ;
  m_SpinOffsetX.SetRange32( iOffsetMin , iOffsetMax ) ;
  m_SpinOffsetX.GetRange32( iMin , iMax ) ;
  m_SpinOffsetX.SetPos32( m_iXOffset_um ) ;

  m_Graph.SetProperty( m_CalculatorNames[ 1 ] , "HLinePos_pix" , ( double ) m_iHorLinePos_pix );
  m_Graph.SetProperty( m_CalculatorNames[ 1 ] , "VLinePos_pix" , ( double ) m_iVertLinePos_pix );
  m_Graph.SetProperty( m_CalculatorNames[ 1 ] , "OffsetZ_um" , ( double ) m_iZOffset_um ) ;
  m_Graph.SetProperty( m_CalculatorNames[ 1 ] , "OffsetY_um" , ( double ) m_iYOffset_um ) ;
  m_Graph.SetProperty( m_CalculatorNames[ 0 ] , "OffsetX_um" , ( double ) m_iXOffset_um ) ;

  SetDlgItemInt( IDC_OFFSET_Z , m_iZOffset_um ) ;
  SetDlgItemInt( IDC_OFFSET_Y , m_iYOffset_um ) ;
  SetDlgItemInt( IDC_OFFSET_X , m_iXOffset_um ) ;

  m_iTolerance_um = ( int ) round( Reg.GetRegiDouble( "MPP_Green" , "Tolerance_um" , 3. ) ) ;
  m_Graph.SetProperty( m_CalculatorNames[ 1 ] , "Tolerance_um" , ( double ) m_iTolerance_um ) ;
  m_Graph.SetProperty( m_CalculatorNames[ 0 ] , "Tolerance_um" , ( double ) m_iTolerance_um ) ;
  m_SpinTolerance.SetRange32( 0 , 20 ) ;
  m_SpinTolerance.SetPos32( m_iTolerance_um ) ;
  SetDlgItemInt( IDC_TOLERANCE_UM , m_iTolerance_um ) ;

  WM_Green WorkingMode = ( WM_Green ) Reg.GetRegiInt( "MPP_Green" , "WorkingMode" , WM_Green_Unknown ) ;
  SetWorkingMode( WorkingMode ) ; // restore last working mode

  // Restore locked coords for process continuation
  if ( WorkingMode == WM_Green_Lock )
  {
    FXString LockedAsString = Reg.GetRegiString( "MPP_Green" , "LockedSide" , "0.,0." ) ;
    m_Graph.SetProperty( m_CalculatorNames[ 1 ] , "LastLocked" , LockedAsString ) ;
    LockedAsString = Reg.GetRegiString( "MPP_Green" , "LockedFront" , "0.,0." ) ;
    m_Graph.SetProperty( m_CalculatorNames[ 0 ] , "LastLocked" , LockedAsString ) ;
    GetDlgItem( IDC_WORK_MODE )->EnableWindow( FALSE ) ;
    ( ( CButton* ) GetDlgItem( IDC_PROTECT ) )->SetState( BST_UNCHECKED ) ;
    SetTimer( EvViewErrors , 250 , NULL ) ;
  }

  BOOL bMulti = Reg.GetRegiInt( "MPP_Green" , "MultiWedgeState" , 0 ) ;
  m_MultiEdgeCheckBox.SetCheck( bMulti ) ;
  int iExposure_us = Reg.GetRegiInt(
    "MPP_Green" ,
    ( bMulti ) ? "SideExposureForMultiWedge_us" : "SideExposureForOneWedge_us" , ( bMulti ) ? 60000 : 28000 ) ;

  if ( !m_Graph.SetProperty( m_CameraName + '2' , "Shutter_us" , iExposure_us ) )
  {
    FXString ErrMsg ;
    ErrMsg.Format( "Can't set exposure %d in gadget %s" ,
      iExposure_us , ( LPCTSTR ) ( m_CameraName + '2' ) ) ;
    PrintGraphMessage( 7 , "UI" , 0 , ( LPCTSTR ) ErrMsg );
  }

}

void CUIFor2ViewsDlg::SetControlsForHoles( )
{
  GetDlgItem( IDC_WORK_MODE )->ShowWindow( SW_HIDE );
  GetDlgItem( IDC_PROTECT )->SetWindowTextA( "Move Enable" ); // "Switch Enable"
  GetDlgItem( IDC_MULTIWEDGE )->ShowWindow( SW_HIDE );
  GetDlgItem( IDC_STATIC1 )->SetWindowText( "Move From, pix" ); //HLine
  GetDlgItem( IDC_STATIC2 )->SetWindowText( "Move To, pix" );   // Offset Z
//  GetDlgItem( IDC_OFFSET_Z )->ShowWindow( SW_HIDE ) ;
//  GetDlgItem( IDC_STATIC3 )->ShowWindow( SW_HIDE ) ;
  GetDlgItem( IDC_STATIC3 )->SetWindowText( "Line Dist,T" );     // Offset Y
  GetDlgItem( IDC_STATIC4 )->SetWindowText( "F, Tx10" );     // Offset X
//   GetDlgItem( IDC_OFFSET_Y )->ShowWindow( SW_HIDE ) ;

//    GetDlgItem( IDC_HLINE_SPIN )->ShowWindow( SW_HIDE ) ;
//    GetDlgItem( IDC_OFFSET_Z_SPIN )->ShowWindow( SW_HIDE ) ;
//   GetDlgItem( IDC_OFFSET_Y_SPIN )->ShowWindow( SW_HIDE ) ;
//  GetDlgItem( IDC_STATIC4 )->ShowWindow( SW_HIDE ) ;
//   GetDlgItem( IDC_OFFSET_X )->ShowWindow( SW_HIDE ) ;
//   GetDlgItem( IDC_OFFSET_X_SPIN )->ShowWindow( SW_HIDE ) ;
//  GetDlgItem( IDC_STATIC5 )->ShowWindow( SW_HIDE ) ;

  m_SpinHLine.SetBuddy( GetDlgItem( IDC_HLINE_POS ) );
  m_SpinOffsetZ.SetBuddy( GetDlgItem( IDC_OFFSET_Z ) ); // Move from, pix
  m_SpinOffsetY.SetBuddy( GetDlgItem( IDC_OFFSET_Y ) ); // Move To, pix
  m_SpinOffsetX.SetBuddy( GetDlgItem( IDC_OFFSET_X ) ); // F, T
  m_SpinTolerance.SetBuddy( GetDlgItem( IDC_TOLERANCE_UM ) );
  m_SpinElectrodeWidth.SetBuddy( GetDlgItem( IDC_ELECTRODE_WIDTH_T ) );
  m_SpinWedgeWidth.SetBuddy( GetDlgItem( IDC_WEDGE_WIDTH_T ) );
  m_SpinVertLinePos.SetBuddy( GetDlgItem( IDC_VERT_LINE_POS ) );
  m_SpinHorLinePos.SetBuddy( GetDlgItem( IDC_HOR_LINE_POS ) );
  m_SpinWedgeXMeasureYPos.SetBuddy( GetDlgItem( IDC_WEDGE_X_MEAS_POS ) ) ;

  FXRegistry Reg( "TheFileX\\Micropoint" );
  m_CalculatorNames.Add( Reg.GetRegiString( "MPP_Holes" , "SideCalculator" , "SideProc" ) );
  m_CalculatorNames.Add( Reg.GetRegiString( "MPP_Holes" , "FrontCalculator" , "FrontProc" ) );

  m_Graph.GetProperty( m_CalculatorNames[ 0 ] , "Scale_um/pix" , m_dSideCameraScale_um_per_pix );
  m_Graph.GetProperty( m_CalculatorNames[ 1 ] , "Scale_um/pix" , m_dFrontCameraScale_um_per_pix );

  static CFont WModeFont;
  static CFont LogBoxFont;
  WModeFont.CreatePointFont( 200 , _T( "Arial" ) );
  GetDlgItem( IDC_WORK_MODE )->SetFont( &WModeFont );
  LogBoxFont.CreatePointFont( 80 , _T( "Arial" ) );
  GetDlgItem( IDC_LOG_LIST )->SetFont( &LogBoxFont );

  m_iPosFrom_pix = Reg.GetRegiInt( "MPP_Holes" , "MoveFrom_pix" , 100 );
  SetDlgItemInt( QIDC_POS_FROM , m_iPosFrom_pix );
  m_SpinHLine.SetRange32( 0 , 1700 );
  m_SpinHLine.SetPos32( m_iPosFrom_pix );
  BOOL bEnabled = Reg.GetRegiInt( "MPP_Holes" , "FrameMoveEnable" , 0 );
  m_Graph.SetProperty( m_FrontCalculator , "MoveFrom_pix" , ( bEnabled ) ? m_iPosFrom_pix : -1 );
  ( ( CButton* ) GetDlgItem( QIDC_MOVE_FRAGMENT ) )->SetState( bEnabled );

  m_iPosTo_pix = Reg.GetRegiInt( "MPP_Holes" , "MoveTo_pix" , 1300 );
  SetDlgItemInt( QIDC_POS_TO , m_iPosTo_pix );
  m_SpinOffsetZ.SetRange32( 400 , 1800 );
  m_SpinOffsetZ.SetPos32( m_iPosTo_pix );
  m_Graph.SetProperty( m_FrontCalculator , "MoveTo_pix" , m_iPosTo_pix );

  FXString RightEdgeSearchParams;
  RightEdgeSearchParams.Format( "name=wedge_x;yoffset=%d;" , m_iPosTo_pix );
  SetParametersToTVObject( ( m_ImagingName + "2" ) , "<<1" , RightEdgeSearchParams );

  int iFromToDist = m_iPosFrom_pix - m_iPosTo_pix - 50;

  m_iWedgeWidth_Tenth = Reg.GetRegiInt( "MPP_Holes" , "WedgeWidth_T" , 60 );
  double dWidth_um = m_iWedgeWidth_Tenth * 2.54;

  double dWidth_pix = dWidth_um / m_dFrontCameraScale_um_per_pix;
  FXString RightEdgeParams;
  RightEdgeParams.Format( "name=electrode_right;xoffset=%d;width=%d;yoffset=%d;" ,
    ROUND( -dWidth_pix * 0.45 ) , ROUND( dWidth_pix * 0.9 ) , iFromToDist );
  SetParametersToTVObject( ( m_ImagingName + "2" ) , "<<1" , RightEdgeParams );

  m_iLineDist_T = Reg.GetRegiInt( "MPP_Holes" , "VertLinesDist_tenth" , 21 );

  int iLineDistMax_T = Reg.GetRegiInt( "MPP_Holes" , "LineDistMax_T" , 150 );
  m_SpinOffsetY.SetRange32( 0 , iLineDistMax_T );
  m_SpinOffsetY.SetPos32( m_iLineDist_T );
  int iInVal = GetDlgItemInt( IDC_OFFSET_Y );
  SetDlgItemInt( IDC_OFFSET_Y , m_iLineDist_T );
  int iReadVal = m_SpinOffsetY.GetPos32( );


  m_iHorLinePos_pix = Reg.GetRegiInt( "MPP_Holes" , "HLinePos_pix" , 900 );
  m_SpinHorLinePos.SetRange32( 0 , 1900 );
  m_SpinHorLinePos.SetPos32( m_iHorLinePos_pix );
  SetDlgItemInt( IDC_HOR_LINE_POS , m_iHorLinePos_pix );
  m_Graph.SetProperty( m_CalculatorNames[ 1 ] , _T( "HLinePos_pix" ) , m_iHorLinePos_pix );
  
  m_iVertLinePos_pix = Reg.GetRegiInt( "MPP_Holes" , "VLinePos_pix" , 600 );
  m_SpinVertLinePos.SetRange32( 1 , 1199 );
  m_SpinVertLinePos.SetPos32( m_iVertLinePos_pix );
  SetDlgItemInt( IDC_VERT_LINE_POS , m_iVertLinePos_pix );
  m_Graph.SetProperty( m_CalculatorNames[ 1 ] , _T( "VLinePos_pix" ) , m_iVertLinePos_pix );

  m_iZOffset_um = m_iYOffset_um = m_iXOffset_um = 0;

  m_iSideXOffset_Tx10 = Reg.GetRegiInt( "MPP_Holes" , "XSideOffset_tenth" , 600 );
  int iSideXOffsetMin_Tx10 = Reg.GetRegiInt( "MPP_Holes" , "XSideOffsetMin_tenth" , 0 );
  int iSideXOffsetMax_Tx10 = Reg.GetRegiInt( "MPP_Holes" , "XSideOffsetMax_tenth" , 1500 );
  m_SpinOffsetX.SetRange32( iSideXOffsetMin_Tx10 , iSideXOffsetMax_Tx10 );
  m_SpinOffsetX.SetPos32( m_iSideXOffset_Tx10 );
  iReadVal = m_SpinOffsetX.GetPos32( );
  SetDlgItemInt( IDC_OFFSET_X , m_iSideXOffset_Tx10 );

  m_Graph.SetProperty( m_FrontCalculator , "VertLinesDist_tenth" , m_iLineDist_T );
  m_Graph.SetProperty( m_SideCalculator , "ShiftFromCorner_Tx10" , m_iSideXOffset_Tx10 );

  m_iTolerance_um = Reg.GetRegiInt( "MPP_Holes" , "Tolerance_um" , 3 );
  m_Graph.SetProperty( m_FrontCalculator , "Tolerance_um" , ( double ) m_iTolerance_um );
  m_Graph.SetProperty( m_SideCalculator , "Tolerance_um" , ( double ) m_iTolerance_um );
  m_SpinTolerance.SetRange32( 0 , 20 );
  m_SpinTolerance.SetPos32( m_iTolerance_um );
  SetDlgItemInt( IDC_TOLERANCE_UM , m_iTolerance_um );

  m_iElectrodeWidth_Tenth = Reg.GetRegiInt( "MPP_Holes" , "ElectrodeWidth_T" , 20 );
  m_SpinElectrodeWidth.SetRange32( 5 , 100 );
  m_SpinElectrodeWidth.SetPos32( m_iElectrodeWidth_Tenth );
  SetDlgItemInt( IDC_ELECTRODE_WIDTH_T , m_iElectrodeWidth_Tenth );

  m_iWedgeWidth_Tenth = Reg.GetRegiInt( "MPP_Holes" , "WedgeWidth_T" , 20 );
  m_SpinWedgeWidth.SetRange32( 10 , 300 );
  m_SpinWedgeWidth.SetPos32( m_iWedgeWidth_Tenth );
  SetDlgItemInt( IDC_WEDGE_WIDTH_T , m_iWedgeWidth_Tenth );

  m_iFrontExposure_us = Reg.GetRegiInt( "MPP_Holes" , "FrontExp_us" , 50000 );
  m_Graph.SetProperty( "FrontCam" , "Shutter_us" , m_iFrontExposure_us );
  SetDlgItemInt( IDC_EXPOSURE_FRONT , m_iFrontExposure_us );

  m_iSideExposure_us = Reg.GetRegiInt( "MPP_Green" , "SideExp_us" , 1000 );
  m_Graph.SetProperty( "SideCam" , "Shutter_us" , m_iSideExposure_us );
  SetDlgItemInt( IDC_EXPOSURE_SIDE , m_iSideExposure_us );

  m_iWedgeXMeasPos_pix = Reg.GetRegiInt( "MPP_Holes" , "WedgeXMeasureYPos" , 1200 );
  m_SpinWedgeXMeasureYPos.SetRange32( 200 , 1800 );
  m_SpinWedgeXMeasureYPos.SetPos32( m_iWedgeXMeasPos_pix );
  SetDlgItemInt( IDC_WEDGE_X_MEAS_POS , m_iWedgeXMeasPos_pix ) ;

  SetDlgItemInt( IDC_FRONT_VIDEO_REST_TIME , 0 );
  SetDlgItemInt( IDC_SIDE_VIDEO_REST_TIME , 0 );
  m_Graph.SendQuantity( "Switch2<>0" , true ); // Side process enable
  m_Graph.SendQuantity( "Switch3<>0" , true ); // Front process enable

  m_bUseBMPs = Reg.GetRegiInt( "MPP_Holes" , "UseBMPs" , 1 ) ;
  if ( m_bUseBMPs )
  {
    m_FrontBMPReadDirectory = Reg.GetRegiString( "MPP_Holes" , "FrontBMPReadDir" , "E:/ForProjects/MPP_HolesInWedges/BMP_20230324/Front/" );
    m_SideBMPReadDirectory = Reg.GetRegiString( "MPP_Holes" , "SideBMPReadDir" , "E:/ForProjects/MPP_HolesInWedges/BMP20221227" );
    m_FrontBMPReadFileName = Reg.GetRegiString( "MPP_Holes" , "FrontBMPReadFileName" , "Front*163*.bmp" );
    m_SideBMPReadFileName = Reg.GetRegiString( "MPP_Holes" , "SideBMPReadFileName" , "Side*.bmp" );

    m_Graph.SetProperty( "BMPFront" , "Directory" , ( LPCTSTR ) m_FrontBMPReadDirectory ) ;
    m_Graph.SetProperty( "BMPFront" , "FileName" , ( LPCTSTR ) m_FrontBMPReadFileName ) ;
    m_Graph.SetProperty( "BMPSide" , "Directory" , ( LPCTSTR ) m_SideBMPReadDirectory ) ;
    m_Graph.SetProperty( "BMPSide" , "FileName" , ( LPCTSTR ) m_SideBMPReadFileName ) ;
  }


}

void CUIFor2ViewsDlg::OnPaint()
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
HCURSOR CUIFor2ViewsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// Automation servers should not exit when a user closes the UI
//  if a controller still holds on to one of its objects.  These
//  message handlers make sure that if the proxy is still in use,
//  then the UI is hidden but the dialog remains around if it
//  is dismissed.

void CUIFor2ViewsDlg::OnClose()
{
	if (CanExit())
		CDialogEx::OnClose();
}

void CUIFor2ViewsDlg::OnOK()
{
	if (CanExit())
		CDialogEx::OnOK();
}

void CUIFor2ViewsDlg::OnCancel()
{
	if (CanExit())
		CDialogEx::OnCancel();
}

BOOL CUIFor2ViewsDlg::CanExit()
{
	// If the proxy object is still around, then the automation
	//  controller is still holding on to this application.  Leave
	//  the dialog around, but hide its UI.
	if (m_pAutoProxy != nullptr)
	{
		ShowWindow(SW_HIDE);
		return FALSE;
	}

	return TRUE;
}


void CUIFor2ViewsDlg::LoadNamesFromRegistry()
{
  FXRegistry Reg( "TheFileX\\Micropoint" );

  m_GraphName = Reg.GetRegiString( "MPP_Green" , "GraphName" ,
    "S:/Dev/NewTVObject_2020Oct06/Sources/FileXGadgets/FileX/res/MPPT_Green.tvg" ) ;
  m_CameraName = Reg.GetRegiString( "MPP_Green" , "CameraName" , "Cam" );
  m_RotateName = Reg.GetRegiString( "MPP_Green" , "RotateName" , "Rotate" );
  m_ImagingName = Reg.GetRegiString( "MPP_Green" , "ImagingName" , "TVObjects" );
  m_StraightSearchName = Reg.GetRegiString( "MPP_Green" , "FindLinesName" , "FindStraights" );
  m_RenderName = Reg.GetRegiString( "MPP_Green" , "RenderName" , "FRender" );
}

void	CUIFor2ViewsDlg::PrintMessage( LPCTSTR message )
{
  m_LastErrorMsg = message ;
  if ( m_LastErrorMsg.Find( "Expired" ) >= 0 )
    m_EvaluationMsg = m_LastErrorMsg ;
  FXAutolock Lock( m_Protect ) ;
  ASSERT( m_Protect.IsLockedByThisThread() ) ;
   m_Message.Add(message);
  m_bPrintMessage = true;
  PostMessage( USER_WM_LOG_MSG ) ;
}

LRESULT CUIFor2ViewsDlg::OnLogMsg( WPARAM wParam , LPARAM lParam )
{
  if ( m_bPrintMessage )
  {
    m_Protect.Lock() ;
    while (m_LogView.GetCount() > 50 )
      m_LogView.DeleteString( m_LogView.GetCount() - 1 ) ;
    while ( m_Message.GetCount() )
    {
      m_LogView.InsertString( 0 , m_Message[ m_Message.GetCount() - 1 ] + "\r\n" ) ;
      m_Message.RemoveAt(m_Message.GetCount() - 1) ;
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

HBRUSH CUIFor2ViewsDlg::OnCtlColor( CDC* pDC , CWnd *pWnd , UINT nCtlColor )
{
  switch ( nCtlColor )
  {
  case CTLCOLOR_STATIC:
    if ( pWnd == GetDlgItem(IDC_SIDE_VIEW) )
    {
      pDC->SetTextColor( m_SideResultView.MTextColor );
      pDC->SetBkColor( m_SideResultView.MBackgroundColor ) ;
      pDC->SetBkMode( OPAQUE );
      return (HBRUSH) GetStockObject( NULL_BRUSH );
    }
    else if ( pWnd == GetDlgItem( IDC_FRONT_VIEW ) )
    {
      pDC->SetTextColor( m_FrontResultView.MTextColor );
      pDC->SetBkColor( m_FrontResultView.MBackgroundColor ) ;
      pDC->SetBkMode( OPAQUE );
      return (HBRUSH) GetStockObject( NULL_BRUSH );
    }
  default:
    return CDialog::OnCtlColor( pDC , pWnd , nCtlColor );
  }
}

void CUIFor2ViewsDlg::OnBnClickedWorkMode()
{
  FXRegistry Reg( "TheFileX\\Micropoint" );
  BOOL bOnlyTwoModes = Reg.GetRegiInt(
    "MPP_Green" , "OnlyFreeAndLock" , 1 ) ;
  WM_Green WorkingMode = GetWorkingMode() ;

  if ( bOnlyTwoModes )
  {
    switch ( WorkingMode )
    {
    case WM_Green_Measure:
      SetWorkingMode( WM_Green_Lock ) ;
      SetTimer( EvLockModeOn , 1000 , NULL ) ;
      SetTimer( EvViewErrors , 250 , NULL ) ;
      break ;
    case WM_Green_Lock:
    default:
      SetWorkingMode( WM_Green_Measure ) ;
      KillTimer( EvLockModeOn ) ;
      break ;
    }
  }
  else
  {
    switch ( WorkingMode )
    {
    case WM_Green_Measure:
      SetWorkingMode( WM_Green_Lock ) ;
      SetTimer( EvLockModeOn , 1000 , NULL ) ;
      SetTimer( EvViewErrors , 250 , NULL ) ;
      break ;
    case WM_Green_Lock:
      SetWorkingMode( WM_Green_Manual ) ;
      KillTimer( EvLockModeOn ) ;
      break ;
    case WM_Green_Manual:
      SetWorkingMode( WM_Green_Measure ) ;
      KillTimer( EvLockModeOn ) ;
      break ;
    default:
      SetWorkingMode( WM_Green_Manual ) ;
      KillTimer( EvLockModeOn ) ;
      break ;
    }
  }

  WorkingMode = GetWorkingMode() ;
  Reg.WriteRegiInt( "MPP_Green" , "WorkingMode" , WorkingMode ) ;
  if ( WorkingMode == WM_Green_Lock )
  {
    FXPropertyKit Prop ;
    ULONG ulPropLen = 16000 ;
    FXString LockedAsString ;
    if ( m_Graph.PrintProperties( m_CalculatorNames[ 1 ] , Prop.GetBuffer( 16000 ) , ulPropLen ) )
    {
      Prop.ReleaseBuffer( ulPropLen ) ;
      if ( (*(FXPropKit2*)&Prop).GetString( "LastLocked" , LockedAsString ) )
        Reg.WriteRegiString( "MPP_Green" , "LockedSide" , LockedAsString ) ;
    }
    ulPropLen = 16000 ;
    if ( m_Graph.PrintProperties( m_CalculatorNames[ 0 ] , Prop.GetBuffer( 16000 ) , ulPropLen ) )
    {
      Prop.ReleaseBuffer( ulPropLen ) ;
      if ( (*(FXPropKit2*) &Prop).GetString( "LastLocked" , LockedAsString ) )
        Reg.WriteRegiString( "MPP_Green" , "LockedFront" , LockedAsString ) ;
    }
  }
  
}

void CUIFor2ViewsDlg::OnBnClickedGraphView()
{
//   CWnd * pSideRender = GetDlgItem( IDC_CAM2 ) ;
//   CWnd * pFrontRender = GetDlgItem( IDC_CAM1 ) ;
//   m_Graph.ConnectRendererToWindow( NULL , _T( "Front" ) , "FRender1" ) ;
//   m_Graph.ConnectRendererToWindow( NULL , _T( "Side" ) , "FRender2" ) ;
  m_Graph.ViewGraph( this ) ;
}


void CUIFor2ViewsDlg::OnBnClickedSetupView()
{
  m_Graph.ViewSetup( this ) ;
}

static int iFontSize = 25 ;

// iMode == 1 - free run, ==2 - lock
int CUIFor2ViewsDlg::SetWorkingMode( int iMode )
{
  m_Graph.SetProperty( m_CalculatorNames[ 1 ] , "WorkingMode" , iMode ) ;
  m_Graph.SetProperty( m_CalculatorNames[ 0 ] , "WorkingMode" , iMode ) ;

  switch ( (WM_Green)iMode)
  {
  case WM_Green_Unknown: 
    ((CMFCButton*) GetDlgItem( IDC_WORK_MODE ))->SetFaceColor( 0x808080 ) ; 
    SetDlgItemText( IDC_WORK_MODE , "Unknown\nFunction" ) ;
    break ;
  case WM_Green_Measure: 
    ((CMFCButton*) GetDlgItem( IDC_WORK_MODE ))->SetFaceColor( 0x00ff00 ) ;
    SetDlgItemText( IDC_WORK_MODE , "Free\n Move" ) ;
    break ;
  case WM_Green_Lock:
    ((CMFCButton*) GetDlgItem( IDC_WORK_MODE ))->SetFaceColor( 0x00ffff ) ;
    SetDlgItemText( IDC_WORK_MODE , "Lock\n Position" ) ;
    break ;
  case WM_Green_Manual:
    {
      ((CMFCButton*) GetDlgItem( IDC_WORK_MODE ))->SetFaceColor( 0xff8080 ) ;
      SetDlgItemText( IDC_WORK_MODE , "Manual\n Mode" ) ;
      m_iBaseLinePos = GetDlgItemInt( IDC_HLINE_POS ) ;
      m_dOffsetFromBaseLine_um = (double) GetDlgItemInt( IDC_OFFSET_Z ) ;
    }
    break ;
  }
  return 0;
}

LRESULT CUIFor2ViewsDlg::OnShowVOSetupDialog( WPARAM wParam , LPARAM lParam )
{
  CVideoObjectBase * pVO = (CVideoObjectBase*) lParam ;
  int x = (int) ((short int) (wParam & 0xffff)) ;
  int y = (int) ((short int) ((wParam >> 16) & 0xffff)) ;
  CPoint Pt( x , y ) ;
  Tvdb400_ShowObjectSetupDlg( pVO , Pt ) ;
  return 0;
}

LRESULT CUIFor2ViewsDlg::OnCameraMsg( WPARAM wParam , LPARAM lParam )
{
  switch ( wParam )
  {
  case 1: m_bCam1CallBackPassed = true ; break ;
  case 2: m_bCam2CallBackPassed = true ; break ;
  }
  return 0;
}


void CUIFor2ViewsDlg::OnTimer( UINT_PTR nIDEvent )
{
  switch ( nIDEvent )
  {
  case EvDrawTimer: 
    {
      if ( m_bCam1CallBackPassed != m_bLastDrawnCam1 )
      {
        m_bLastDrawnCam1 = m_bCam1CallBackPassed ;
        m_Cam1StatusLED.SetBitmap( m_bCam1CallBackPassed ? m_GreenLed : m_RedLed ) ;
      }
      if ( m_bCam2CallBackPassed != m_bLastDrawnCam2 )
      {
        m_bLastDrawnCam2 = m_bCam2CallBackPassed ;
        m_Cam2StatusLED.SetBitmap( m_bCam2CallBackPassed ? m_GreenLed : m_RedLed ) ;
      }
      m_bCam1CallBackPassed = m_bCam2CallBackPassed = false ;
    } ;
    break ;
  case EvLockModeOn:
    GetDlgItem( IDC_WORK_MODE )->EnableWindow( FALSE ) ;
    KillTimer( EvLockModeOn ) ;
    ((CButton*) GetDlgItem( IDC_PROTECT ))->SetState( BST_UNCHECKED) ;
    break ;
  case EvViewErrors:
    {
      FXPropertyKit Prop ;
      ULONG ulPropLen = 16000 ;
      if ( m_Graph.PrintProperties( m_CalculatorNames[ 1 ] , Prop.GetBuffer( 16000 ) , ulPropLen ) )
      {
        Prop.ReleaseBuffer( ulPropLen ) ;
        double dY_um = 0. , dZ_um = 0. ;
        if ( Prop.GetDouble( "dY_um" , dY_um ) 
          && Prop.GetDouble( "dZ_um" , dZ_um ) )
        {
          FXString ViewText ;
          COLORREF Color = (fabs( dY_um ) > m_iTolerance_um) ? RGB( 255 , 0 , 0 ) : RGB( 0 , 180 , 0 ) ;
          LPCTSTR pFormat = (dY_um > 0) ? "   <<< dY=%.2f ;  dZ=%.2f  " : "  dY=%.2f >>> ;  dZ=%.2f  " ;
          ViewText.Format( pFormat , dY_um , dZ_um ) ;
          m_SideResultView.SetTextColor( RGB(255,255,255) ) ;
          m_SideResultView.SetBackgroundColor( Color ) ;
          SetDlgItemText( IDC_SIDE_VIEW , (LPCTSTR) ViewText ) ;
        }
      }
      ulPropLen = 16000 ;
      if ( m_Graph.PrintProperties( m_CalculatorNames[ 0 ] , Prop.GetBuffer( 16000 ) , ulPropLen ) )
      {
        Prop.ReleaseBuffer( ulPropLen ) ;
        double dX_um = 0. , dZ_um = 0. ;
        if ( Prop.GetDouble( "dX_um" , dX_um )
          && Prop.GetDouble( "dZ_um" , dZ_um ) )
        {
          FXString ViewText ;
          COLORREF Color = (fabs( dX_um ) > m_iTolerance_um) ? RGB( 255 , 0 , 0 ) : RGB( 0 , 180 , 0 ) ;
          LPCTSTR pFormat = (dX_um > 0) ? "  <<< dX=%.2f ;  dZ=%.2f " : " dX=%.2f >>> ;  dZ=%.2f " ;
          ViewText.Format( pFormat , dX_um , dZ_um ) ;
          m_FrontResultView.SetTextColor( RGB( 255 , 255 , 255 ) ) ;
          m_FrontResultView.SetBackgroundColor( Color ) ;
          SetDlgItemText( IDC_FRONT_VIEW , (LPCTSTR) ViewText ) ;
        }
      }
    }
    break ;
  case EvWritingTimer:
    {
      int iRestFront = GetDlgItemInt( IDC_FRONT_VIDEO_REST_TIME );
      if ((iRestFront <= 1) && (iRestFront >= 0))
      {
        m_Graph.SendQuantity( "Switch1<>0" , false ); // Front write disable
        m_Graph.SendText( "FrontAvi<>0" , "close(1);" );
        SetDlgItemInt( IDC_FRONT_VIDEO_REST_TIME , iRestFront = -1 );
      }
      else
      {
        SetDlgItemInt( IDC_FRONT_VIDEO_REST_TIME , 
          (iRestFront < 0 ) ? iRestFront = -1 : --iRestFront );
      }
      int iRestSide = GetDlgItemInt( IDC_SIDE_VIDEO_REST_TIME );
      if ((iRestSide <= 1) && (iRestSide >= 0) )
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
      if (( iRestSide < 0 ) && ( iRestFront < 0 ))
      {
        KillTimer( EvWritingTimer );
        m_iWritingTimer = 0;
      }
    }
    break ;
   case EvRestoreRendering:
   {
      CWnd * pSideRender = GetDlgItem( IDC_CAM2 ) ;
      CWnd * pFrontRender = GetDlgItem( IDC_CAM1 ) ;
      m_Graph.ConnectRendererToWindow( pSideRender , _T( "Front" ) , "FRender1" ) ;
      m_Graph.ConnectRendererToWindow( pFrontRender , _T( "Side" ) , "FRender2" ) ;
      SetWindowPos( &wndTop , 0 , 0 , 0 , 0 , SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW ) ;
      KillTimer( EvRestoreRendering ) ;
   }
   break ;
  }
  CDialogEx::OnTimer( nIDEvent );
}


void CUIFor2ViewsDlg::OnBnClickedProtect()
{
  switch ( m_AppMode )
  {
  case AM_Green:
    {
      BOOL bEnabled = ( ( ( ( CButton* ) GetDlgItem( IDC_PROTECT ) )->GetState() ) & BST_CHECKED ) ;
      GetDlgItem( IDC_WORK_MODE )->EnableWindow( bEnabled ) ;
      return ;
    }
  case AM_Holes:
    {
      BOOL bEnabled = ( ( ( ( CButton* ) GetDlgItem( QIDC_MOVE_FRAGMENT ) )->GetState() ) & BST_CHECKED ) ;

      m_Graph.SetProperty( m_FrontCalculator , "MoveFrom_pix" , ( bEnabled ) ? m_iPosFrom_pix : -1 ) ;
      FXRegistry Reg( "TheFileX\\Micropoint" );
      Reg.WriteRegiInt( "MPP_Holes" , "FrameMoveEnable" , bEnabled ) ;

      return ;
    }
  }
}


WM_Green CUIFor2ViewsDlg::GetWorkingMode()
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


void CUIFor2ViewsDlg::OnDeltaposHlineSpin( NMHDR *pNMHDR , LRESULT *pResult )
{
  switch ( m_AppMode )
  {
  case AM_Green:
    OnCoordSpinDeltaPos( pNMHDR , m_iHLinePos , m_SpinHLine , IDC_HLINE_POS , "HLinePos_pix" , "1" ) ;
    *pResult = m_iHLinePos ;
    break ;
  case AM_Holes:
    OnCoordSpinDeltaPos( pNMHDR , m_iPosFrom_pix , m_SpinHLine , QIDC_POS_FROM , "MoveFrom_pix" , "" ) ;
    *pResult = m_iPosFrom_pix;

    FXString RightEdgeSearchParams;
    //RightEdgeSearchParams.Format( "name=electrode_right;yoffset=%d;" , m_iPosFrom_pix - m_iPosTo_pix - 50 );
    RightEdgeSearchParams.Format( "name=electrode_right;yoffset=%d;" , m_iPosFrom_pix - m_iWedgeXMeasPos_pix + 50 );
    SetParametersToTVObject( ( m_ImagingName + "2" ) , "<<1" , RightEdgeSearchParams );

    break ;
  }
}

void CUIFor2ViewsDlg::OnCoordSpinDeltaPos( NMHDR *pNMHDR , int& iResult ,
  CSpinButtonCtrl& Spin , size_t IDC , LPCTSTR pszRegValName , LPCTSTR pszCalcName )
{
  LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
  int iStep = pNMUpDown->iDelta ;
  iResult += iStep ;
  int iMin , iMax ;
  Spin.GetRange32( iMin , iMax ) ;
  if ( iResult < iMin )
    iResult = iMin ;
  else if ( iResult > iMax )
    iResult = iMax ;
  SetDlgItemInt( (int)IDC , iResult ) ;

  if ( pszRegValName )
  {
    FXRegistry Reg( "TheFileX\\Micropoint" );
    Reg.WriteRegiInt(
      (m_AppMode == AM_Green) ?  "MPP_Green" : "MPP_Holes" , pszRegValName , iResult ) ;
    if ( !m_Graph.SetProperty( pszCalcName , pszRegValName , (double) iResult ) )
    {
      FXString ErrMsg ;
      ErrMsg.Format( "Can't set %s to %d in gadget %s" , pszRegValName ,
        iResult , pszCalcName ) ;
      PrintGraphMessage( 7 , "UI" , 0 , (LPCTSTR) ErrMsg );
    }
  }

}


void CUIFor2ViewsDlg::OnDeltaposOffsetZSpin( NMHDR *pNMHDR , LRESULT *pResult )
{
  switch ( m_AppMode )
  {
  case AM_Green:
    OnCoordSpinDeltaPos( pNMHDR , m_iZOffset_um , m_SpinOffsetZ , IDC_OFFSET_Z , "ZOffset_um" , "2" ) ;
    *pResult = m_iZOffset_um;
    break ;
  case AM_Holes:
    OnCoordSpinDeltaPos( pNMHDR , m_iPosTo_pix , m_SpinOffsetZ , QIDC_POS_TO , "MoveTo_pix" , "" ) ;
    *pResult = m_iPosTo_pix;

//     FXString RightEdgeSearchParams;
//     RightEdgeSearchParams.Format( "name=wedge_x;yoffset=%d;" , m_iPosTo_pix );
//     SetParametersToTVObject( ( m_ImagingName + "2" ) , "<<1" , RightEdgeSearchParams );
    break ;
  }
}

void CUIFor2ViewsDlg::OnDeltaposOffsetYSpin( NMHDR *pNMHDR , LRESULT *pResult )
{
  switch ( m_AppMode )
  {
  case AM_Green:
    OnCoordSpinDeltaPos( pNMHDR , m_iYOffset_um , m_SpinOffsetY , IDC_OFFSET_Y , "YOffset_um" , "2" ) ;
    *pResult = m_iYOffset_um;
    break ;
  case AM_Holes:
    OnCoordSpinDeltaPos( pNMHDR , m_iLineDist_T , m_SpinOffsetY , IDC_OFFSET_Y , NULL , NULL ) ;
    *pResult = m_iLineDist_T;
    break ;
  }
}

void CUIFor2ViewsDlg::OnDeltaposOffsetXSpin( NMHDR *pNMHDR , LRESULT *pResult )
{
  switch ( m_AppMode )
  {
  case AM_Green:
    OnCoordSpinDeltaPos( pNMHDR , m_iXOffset_um , m_SpinOffsetX , IDC_OFFSET_X , "XOffset_um" , "1" ) ;
    *pResult = m_iXOffset_um;
    break ;
  case AM_Holes:
    OnCoordSpinDeltaPos( pNMHDR , m_iSideXOffset_Tx10 , m_SpinOffsetX , IDC_OFFSET_X , NULL , NULL ) ;
    *pResult = m_iSideXOffset_Tx10;
    break ;
  }
}
void CUIFor2ViewsDlg::OnDeltaposToleranceSpin( NMHDR *pNMHDR , LRESULT *pResult )
{
  OnCoordSpinDeltaPos( pNMHDR , m_iTolerance_um , m_SpinTolerance , 
    IDC_TOLERANCE_UM , "Tolerance_um" , m_CalculatorNames[ 0 ] ) ;
  if ( !m_Graph.SetProperty( m_CalculatorNames[ 0 ] , "Tolerance_um" ,
    (double) m_iTolerance_um ) )    // set value to front gadget
  {
    FXString ErrMsg ;
    ErrMsg.Format( "Can't set tolerance %d in gadget %s" ,
      m_iTolerance_um , (LPCTSTR) m_CalculatorNames[ 0 ] ) ;
    PrintGraphMessage( 7 , "UI" , 0 , (LPCTSTR) ErrMsg );
  }
  if ( !m_Graph.SetProperty( m_CalculatorNames[ 1 ] , "Tolerance_um" ,
    (double) m_iTolerance_um ) ) // set value to side gadget
  {
    FXString ErrMsg ;
    ErrMsg.Format( "Can't set tolerance %d in gadget %s" ,
      m_iTolerance_um , (LPCTSTR) m_CalculatorNames[ 1 ] ) ;
    PrintGraphMessage( 7 , "UI" , 0 , (LPCTSTR) ErrMsg );
  }
  *pResult = m_iTolerance_um;
}

void CUIFor2ViewsDlg::OnChangeCoordOffset( int& iVal , CSpinButtonCtrl& Spin , 
  size_t IDC , LPCTSTR pszRegValName , LPCTSTR pszCalcName )
{
  iVal = GetDlgItemInt( (int)IDC ) ;

  int iMin , iMax ;
  Spin.GetRange32( iMin , iMax ) ;
  if ( iVal < iMin )
    iVal = iMin ;
  else if ( iVal > iMax )
    iVal = iMax ;

  FXString OffAsString ;
  OffAsString.Format( "%d" , iVal ) ;

  FXRegistry Reg( "TheFileX\\Micropoint" );
  Reg.WriteRegiString(
    "MPP_Green" , pszRegValName , OffAsString ) ;

  m_Graph.SetProperty( pszCalcName , pszRegValName , (double) iVal ) ;
}

void CUIFor2ViewsDlg::OnEnChangeOffsetZ()
{
  switch ( m_AppMode )
  {
  case AM_Green:
    OnChangeCoordOffset( m_iYOffset_um , m_SpinOffsetY , IDC_OFFSET_Y , "YOffset_um" , "2" ) ;
    break ;
  case AM_Holes:
    {
      FXRegistry Reg( "TheFileX\\Micropoint" );
      BOOL bEnabled = ( ( ( ( CButton* ) GetDlgItem( QIDC_MOVE_FRAGMENT ) )->GetState() ) & BST_CHECKED ) ;

      if ( bEnabled )
      {
        m_Graph.SetProperty( m_FrontCalculator , "MoveTo_pix" , m_iPosTo_pix ) ;
        Reg.WriteRegiInt( "Holes" , "MoveTo_pix" , m_iPosTo_pix ) ;
      }
      Reg.WriteRegiInt( "MPP_Holes" , "FrameMoveEnable" , bEnabled ) ;

//       FXString RightEdgeSearchParams;
//       RightEdgeSearchParams.Format( "name=electrode_right;yoffset=%d;" , m_iPosFrom_pix - m_iPosTo_pix - 50 );
//       SetParametersToTVObject( ( m_ImagingName + "2" ) , "<<1" , RightEdgeSearchParams );
    }
    break ;
  }
}
void CUIFor2ViewsDlg::OnEnChangeOffsetY()
{
  switch ( m_AppMode )
  {
  case AM_Green:
    OnChangeCoordOffset( m_iYOffset_um , m_SpinOffsetY , IDC_OFFSET_Y , "YOffset_um" , "2" ) ;
    break ;
  case AM_Holes:
    {
      int iVal = m_SpinOffsetY.GetPos32() /*GetDlgItemInt( IDC_OFFSET_Y )*/ ;
      int iValIn = GetDlgItemInt( IDC_OFFSET_Y ) ;

      int iMin , iMax ;
      m_SpinOffsetY.GetRange32( iMin , iMax ) ;
      if ( iVal < iMin )
        iVal = iMin ;
      else if ( iVal > iMax )
        iVal = iMax ;

      FXRegistry Reg( "TheFileX\\Micropoint" );
      Reg.WriteRegiInt( "MPP_Holes" , "VertLinesDist_tenth" , iVal ) ;

      m_Graph.SetProperty( m_FrontCalculator , "VertLinesDist_tenth" , iVal ) ;
    }
    break ;

  }
}
void CUIFor2ViewsDlg::OnEnChangeOffsetX()
{
  switch ( m_AppMode )
  {
  case AM_Green:
    OnChangeCoordOffset( m_iXOffset_um , m_SpinOffsetX , IDC_OFFSET_X , "XOffset_um" , "1" ) ;
    break ;
  case AM_Holes:
    {
      int iVal = m_SpinOffsetX.GetPos32() /*GetDlgItemInt( IDC_OFFSET_Y )*/ ;

      int iMin , iMax ;
      m_SpinOffsetX.GetRange32( iMin , iMax ) ;
      if ( iVal < iMin )
        iVal = iMin ;
      else if ( iVal > iMax )
        iVal = iMax ;

      FXRegistry Reg( "TheFileX\\Micropoint" );
      Reg.WriteRegiInt( "MPP_Holes" , "XSideOffset_tenth" , iVal ) ;

      m_Graph.SetProperty( m_SideCalculator , "ShiftFromCorner_Tx10" , iVal ) ;
    }
    break ;
  }
}

void CUIFor2ViewsDlg::OnEnChangeTolerance()
{
  OnChangeCoordOffset( m_iTolerance_um , m_SpinTolerance , IDC_TOLERANCE_UM , "Tolerance_um" , "1" ) ;
  switch ( m_AppMode )
  {
  case AM_Green:
    if ( !m_Graph.SetProperty( m_CalculatorNames[0] , "Tolerance_um" ,
          (double) m_iTolerance_um ) )    // set value to front gadget
    {
      FXString ErrMsg ;
      ErrMsg.Format( "Can't set tolerance %d in gadget %s" ,
        m_iTolerance_um , (LPCTSTR) m_CalculatorNames[ 0 ] ) ;
      PrintGraphMessage( 7 , "UI" , 0 , (LPCTSTR) ErrMsg );
    }
    if ( !m_Graph.SetProperty( m_CalculatorNames[ 1 ] , "Tolerance_um" ,
      (double) m_iTolerance_um ) ) // set value to side gadget
    {
      FXString ErrMsg ;
      ErrMsg.Format( "Can't set tolerance %d in gadget %s" ,
        m_iTolerance_um , (LPCTSTR) m_CalculatorNames[ 1 ] ) ;
      PrintGraphMessage( 7 , "UI" , 0 , (LPCTSTR) ErrMsg );
    }
  	break;
  case AM_Holes:
    if ( !m_Graph.SetProperty( m_FrontCalculator , "Tolerance_um" ,
          (double) m_iTolerance_um ) )    // set value to front gadget
    {
      FXString ErrMsg ;
      ErrMsg.Format( "Can't set tolerance %d in gadget %s" ,
        m_iTolerance_um , (LPCTSTR) m_FrontCalculator ) ;
      PrintGraphMessage( 7 , "UI" , 0 , (LPCTSTR) ErrMsg );
    }
    else
    {
      FXRegistry Reg( "TheFileX\\Micropoint" );
      Reg.WriteRegiInt( "MPP_Holes" , "Tolerance_um" , m_iTolerance_um ) ;
    }
    if ( !m_Graph.SetProperty( m_SideCalculator , "Tolerance_um" ,
      (double) m_iTolerance_um ) ) // set value to side gadget
    {
      FXString ErrMsg ;
      ErrMsg.Format( "Can't set tolerance %d in gadget %s" ,
        m_iTolerance_um , (LPCTSTR) m_FrontCalculator ) ;
      PrintGraphMessage( 7 , "UI" , 0 , (LPCTSTR) ErrMsg );
    }
  }
}

void CUIFor2ViewsDlg::OnBnClickedMultiwedge()
{
  BOOL bMulti = ( m_MultiEdgeCheckBox.GetState() & BST_CHECKED ) != 0 ;

  FXRegistry Reg( "TheFileX\\Micropoint" );
  int iExposure_us = Reg.GetRegiInt(
    "MPP_Green" , 
    (bMulti) ? "SideExposureForMultiWedge_us" : "SideExposureForOneWedge_us" , (bMulti) ? 60000 : 28000 ) ;

  if ( !m_Graph.SetProperty( m_CameraName + '2' , "Shutter_us" , iExposure_us ) )
  {
    FXString ErrMsg ;
    ErrMsg.Format( "Can't set exposure %d in gadget %s" ,
      iExposure_us , (LPCTSTR) (m_CameraName + '2') ) ;
    PrintGraphMessage( 7 , "UI" , 0 , (LPCTSTR)ErrMsg );
  }

  Reg.WriteRegiInt(
    "MPP_Green" ,
    "MultiWedgeState" , bMulti ) ;
}


void CUIFor2ViewsDlg::OnEnChangeHlinePos()
{
  switch ( m_AppMode )
  {
  case AM_Green:

    break ;
  case AM_Holes:
    {
      BOOL bEnabled = ( ( ( ( CButton* ) GetDlgItem( QIDC_MOVE_FRAGMENT ) )->GetState() ) & BST_CHECKED ) ;

      if ( bEnabled )
      {
        m_Graph.SetProperty( m_FrontCalculator , "MoveFrom_pix" , m_iPosFrom_pix ) ;
        // Set Y coordinate right edge search position

      }
      FXRegistry Reg( "TheFileX\\Micropoint" );
      Reg.WriteRegiInt( "MPP_Holes" , "FrameMoveEnable" , bEnabled ) ;
    }
    break ;
  }
}

void CUIFor2ViewsDlg::OnStnClickedExposureFrontCaption()
{
  // TODO: Add your control notification handler code here
}

void CUIFor2ViewsDlg::OnBnClickedSaveSideVideo()
{
  m_Graph.SendQuantity( "Switch6<>0" , true ); // Side write enable
  if (!m_iWritingTimer)
    m_iWritingTimer = SetTimer( EvWritingTimer , 1000 , NULL );
  SetDlgItemInt( IDC_SIDE_VIDEO_REST_TIME , 10 );
}


void CUIFor2ViewsDlg::OnBnClickedSaveBothVideo()
{
  m_Graph.SendQuantity( "Switch1<>0" , true ) ; // Side write enable
  m_Graph.SendQuantity( "Switch6<>0" , true ) ; // Front write enable
  if (!m_iWritingTimer)
    m_iWritingTimer = SetTimer( EvWritingTimer , 1000 , NULL );
  SetDlgItemInt( IDC_FRONT_VIDEO_REST_TIME , 10 );
  SetDlgItemInt( IDC_SIDE_VIDEO_REST_TIME , 10 );
}


void CUIFor2ViewsDlg::OnBnClickedSaveFrontVideo()
{
  m_Graph.SendQuantity( "Switch1<>0" , true ); // Front write enable
  if (!m_iWritingTimer)
    m_iWritingTimer = SetTimer( EvWritingTimer , 1000 , NULL );
  SetDlgItemInt( IDC_FRONT_VIDEO_REST_TIME , 10 );
}


void CUIFor2ViewsDlg::OnBnClickedSaveOneSideBmp()
{
  FXRegistry Reg( "TheFileX\\Micropoint" );
  FXString ImageSaveDir = Reg.GetRegiString( "MPP_Holes" , 
    "SaveWithGraphicsDir" , "D:/HolesErrosion/Pictures/" ) ;
  m_Graph.SendText( "FRender1<<0" , "SideView_" , "ImageSavePrefix" ) ;
  m_Graph.SendText( "FRender1<<0" , (LPCTSTR)ImageSaveDir , "SaveImage" ) ;
}


void CUIFor2ViewsDlg::OnBnClickedSaveOneFrontBmp()
{
  FXRegistry Reg( "TheFileX\\Micropoint" );
  FXString ImageSaveDir = Reg.GetRegiString( "MPP_Holes" , "SaveWithGraphicsDir" , "D:/HolesErrosion/Pictures/" ) ;
  m_Graph.SendText( "FRender2<<0" , "FrontView_" , "ImageSavePrefix" ) ;
  m_Graph.SendText( "FRender2<<0" , ( LPCTSTR ) ImageSaveDir , "SaveImage" ) ;
}


void CUIFor2ViewsDlg::OnBnClickedDownLight()
{
  BOOL bEnabled = ( ( ( ( CButton* ) GetDlgItem( IDC_DOWN_LIGHT ) )->GetState() ) & BST_CHECKED ) ;
  FXString Command ;
  Command.Format( "Chan=%d; Value=%d;" , 2 , bEnabled ) ;
  m_Graph.SendText( "UsbRelayGadget1<<0" , ( LPCTSTR ) Command , "SwitchDownLight" ) ;
}


void CUIFor2ViewsDlg::OnBnClickedFrontLight()
{
  BOOL bEnabled = ( ( ( ( CButton* ) GetDlgItem( IDC_FRONT_LIGHT ) )->GetState() ) & BST_CHECKED ) ;
  FXString Command ;
  Command.Format( "Chan=%d; Value=%d;" , 1 , bEnabled ) ;
  m_Graph.SendText( "UsbRelayGadget1<<0" , ( LPCTSTR ) Command , "SwitchFrontLight" ) ;
}

void CUIFor2ViewsDlg::OnEnChangeExposureFront( )
{
  m_iFrontExposure_us = GetDlgItemInt( IDC_EXPOSURE_FRONT );
  FXRegistry Reg( "TheFileX\\Micropoint" );
  Reg.WriteRegiInt( "MPP_Holes" , "FrontExp_us" , m_iFrontExposure_us );
  m_Graph.SetProperty( "FrontCam" , "Shutter_us" , m_iFrontExposure_us );
}

void CUIFor2ViewsDlg::OnBnClickedFrontExpPlus( )
{
  int iNewExp = ROUND( ceil( m_iFrontExposure_us * 1.05 ) );
  if (iNewExp == m_iFrontExposure_us)
    ++m_iFrontExposure_us;
  else
    m_iFrontExposure_us = iNewExp;
  if (m_iFrontExposure_us > 150000)
    m_iFrontExposure_us = 150000;
  SetDlgItemInt( IDC_EXPOSURE_FRONT , m_iFrontExposure_us );
}

void CUIFor2ViewsDlg::OnBnClickedFrontExpMinus( )
{
  int iNewExp = ROUND( ceil( m_iFrontExposure_us * 0.95 ) );
  if (iNewExp == m_iFrontExposure_us)
    --m_iFrontExposure_us;
  else
    m_iFrontExposure_us = iNewExp;
  if (m_iFrontExposure_us < 30)
    m_iFrontExposure_us = 30;
  SetDlgItemInt( IDC_EXPOSURE_FRONT , m_iFrontExposure_us );
}

void CUIFor2ViewsDlg::OnEnChangeExposureSide( )
{
  m_iSideExposure_us = GetDlgItemInt( IDC_EXPOSURE_SIDE );
  FXRegistry Reg( "TheFileX\\Micropoint" );
  Reg.WriteRegiInt( "MPP_Holes" , "SideExp_us" , m_iSideExposure_us );
  m_Graph.SetProperty( "SideCam" , "Shutter_us" , m_iSideExposure_us );
}


void CUIFor2ViewsDlg::OnBnClickedSideExpPlus( )
{
  int iNewExp = ROUND( ceil( m_iSideExposure_us * 1.05 ) );
  if (iNewExp == m_iSideExposure_us)
    ++m_iSideExposure_us;
  else
    m_iSideExposure_us = iNewExp;
  if (m_iSideExposure_us > 150000)
    m_iSideExposure_us = 150000;
  SetDlgItemInt( IDC_EXPOSURE_SIDE , m_iSideExposure_us );
}


void CUIFor2ViewsDlg::OnBnClickedSideExpMinus( )
{
  int iNewExp = ROUND( ceil( m_iSideExposure_us * 0.95 ) );
  if (iNewExp == m_iSideExposure_us)
    --m_iSideExposure_us;
  else
    m_iSideExposure_us = iNewExp;
  if (m_iSideExposure_us < 30)
    m_iSideExposure_us = 30;
  SetDlgItemInt( IDC_EXPOSURE_SIDE , m_iSideExposure_us );
}


void CUIFor2ViewsDlg::OnEnChangeElectrodeWidthT( )
{
  m_iElectrodeWidth_Tenth = GetDlgItemInt( IDC_ELECTRODE_WIDTH_T );
  double dWidth_um = m_iElectrodeWidth_Tenth * 2.54;

  double dWidth_pix = dWidth_um / m_dFrontCameraScale_um_per_pix;
  FXString LeftEdgeParams;
  LeftEdgeParams.Format( "name=electrode_left;xoffset=%d;width=%d;yoffset=-53;" ,
    ROUND( -dWidth_pix * 1.2 ) , ROUND( dWidth_pix * 0.6 ) );
  SetParametersToTVObject( (m_ImagingName + "2") , "<<1" , LeftEdgeParams );

  FXRegistry Reg( "TheFileX\\Micropoint" );
  Reg.WriteRegiInt( "MPP_Holes" , "ElectrodeWidth_T" , m_iElectrodeWidth_Tenth );
}

bool CUIFor2ViewsDlg::SetParametersToTVObject( LPCTSTR pGadgetName , LPCTSTR pPin , LPCTSTR pParams )
{
  FXString PinName( pGadgetName );
  PinName += pPin;

  if (m_Graph.SendText( PinName , pParams , "SetObjectProps" ))
    return true;
  FXString ErrMessage;
  ErrMessage.Format( "Can't set parameters to pin %s (%s)\n" , ( LPCTSTR ) PinName , pParams );
  PrintMessage( ErrMessage );
  return false;
}


void CUIFor2ViewsDlg::OnStnClickedStatic7( )
{
  // TODO: Add your control notification handler code here
}


void CUIFor2ViewsDlg::OnEnChangeWedgeWidthT( )
{
  m_iWedgeWidth_Tenth = GetDlgItemInt( IDC_WEDGE_WIDTH_T );
  double dWidth_um = m_iWedgeWidth_Tenth * 2.54;

  double dWidth_pix = dWidth_um / m_dFrontCameraScale_um_per_pix;
  FXString RightEdgeParams;
  RightEdgeParams.Format( "name=electrode_right;xoffset=%d;width=%d;" ,
    ROUND( -dWidth_pix * 0.3 ) , ROUND( dWidth_pix * 0.8 ) );
  SetParametersToTVObject( ( m_ImagingName + "2" ) , "<<1" , RightEdgeParams );

  FXRegistry Reg( "TheFileX\\Micropoint" );
  Reg.WriteRegiInt( "MPP_Holes" , "WedgeWidth_T" , m_iWedgeWidth_Tenth );
}


void CUIFor2ViewsDlg::OnEnChangeHorLinePos( )
{
  CString Text;
  int iNChar = GetDlgItemText( IDC_HOR_LINE_POS , Text );
  Text.Remove( ',' );
  m_iHorLinePos_pix = atoi( ( LPCTSTR ) Text );
  m_Graph.SetProperty( m_CalculatorNames[ 1 ] , _T( "HLinePos_pix" ) , m_iHorLinePos_pix );

  FXRegistry Reg( "TheFileX\\Micropoint" );
  Reg.WriteRegiInt( "MPP_Holes" , "HLinePos_pix" , m_iHorLinePos_pix );
}


void CUIFor2ViewsDlg::OnEnChangeVertLinePos( )
{
  CString Text;
  int iNChar = GetDlgItemText( IDC_VERT_LINE_POS , Text );
  Text.Remove( ',' );
  m_iVertLinePos_pix = atoi( ( LPCTSTR ) Text );
  m_Graph.SetProperty( m_CalculatorNames[ 1 ] , _T( "VLinePos_pix" ) , m_iVertLinePos_pix );

  FXRegistry Reg( "TheFileX\\Micropoint" );
  Reg.WriteRegiInt( "MPP_Holes" , "VLinePos_pix" , m_iHorLinePos_pix );
}


void CUIFor2ViewsDlg::OnEnChangeWedgeXMeasPos()
{
  CString Text;
  int iNChar = GetDlgItemText( IDC_WEDGE_X_MEAS_POS , Text );
  Text.Remove( ',' );
  m_iWedgeXMeasPos_pix = atoi( ( LPCTSTR ) Text );

  FXString WedgeXMeasPars ;
  WedgeXMeasPars.Format( "name=wedge_x;yoffset=%d;" , m_iWedgeXMeasPos_pix );
  SetParametersToTVObject( ( m_ImagingName + "2" ) , "<<1" , WedgeXMeasPars );

  FXString ElectrodeRightPosPar ;
  ElectrodeRightPosPar.Format( "name=electrode_right;yoffset=%d;" , m_iPosFrom_pix - m_iWedgeXMeasPos_pix + 50 );
  SetParametersToTVObject( ( m_ImagingName + "2" ) , "<<1" , ElectrodeRightPosPar );

  FXRegistry Reg( "TheFileX\\Micropoint" );
  Reg.WriteRegiInt( "MPP_Holes" , "WedgeXMeasureYPos" , m_iWedgeXMeasPos_pix );
}


void CUIFor2ViewsDlg::OnParentNotify( UINT message , LPARAM lParam )
{
  // this must be a message from debug window
  if ( ( message == WM_DESTROY )/* && ( ( HWND ) lParam == GetSafeHwnd() )*/ )
  {
//     CWnd * pSideRender = GetDlgItem( IDC_CAM2 ) ;
//     CWnd * pFrontRender = GetDlgItem( IDC_CAM1 ) ;
    m_Graph.ConnectRendererToWindow( NULL , NULL , "FRender1" ) ;
    m_Graph.ConnectRendererToWindow( NULL , NULL , "FRender2" ) ;
// 
//     m_Graph.ConnectRendererToWindow( pSideRender , _T( "Front" ) , "FRender1" ) ;
//     m_Graph.ConnectRendererToWindow( pFrontRender , _T( "Side" ) , "FRender2" ) ;
    SetTimer( EvRestoreRendering , 500 , NULL ) ;
  }
  else
    CDialog::OnParentNotify( message , lParam );
}


void CUIFor2ViewsDlg::OnBnClickedGraphSave()
{
  m_Graph.SaveGraph() ;
}

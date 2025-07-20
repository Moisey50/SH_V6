
// UI_NViewsDlg.h : header file
//

#pragma once

#include <gadgets/gadbase.h>
#include <helpers\shwrapper.h>
#include <math/Intf_sup.h>
#include "PasswordDialog.h"
#include <helpers/ShMemControl.h>
#include <thread>
#include "gadgets/TectoMsgs.h"
#include "video/videoformats.h"

#define IP_YUV422 1
//#include <helpers\registry.h>

// Quasi IDCs
#define QIDC_MOVE_FRAGMENT IDC_PROTECT
#define QIDC_POS_FROM IDC_HLINE_POS
#define QIDC_POS_TO   IDC_OFFSET_Z

#define IS_CHECKED(x) ( ( ( ( CButton* ) GetDlgItem( x ) )->GetCheck() ) == BST_CHECKED )
#define SET_CHECK(x,bCheck) ( ( CButton* ) GetDlgItem( x ) )->SetCheck( bCheck ? BST_CHECKED : BST_UNCHECKED )
enum USER_WM
{
  USER_WM_CALLBACK_TEXT = WM_APP + 1 ,
  USER_WM_CALLBACK_DATA ,
  USER_WM_LOG_MSG ,
  USER_WM_NEW_MEASUREMENT_RESULT ,

  USER_WM_IMAGE_SIMULATION_0 ,
  USER_WM_IMAGE_SIMULATION_1 ,
  USER_WM_IMAGE_SIMULATION_2 ,
  USER_WM_IMAGE_SIMULATION_3
};

enum WM_Green
{
  WM_Green_Unknown = 0 ,
  WM_Green_Measure ,
  WM_Green_Lock ,
  WM_Green_Manual
};

enum ApplicationMode
{
  AM_Green = 0 ,
  AM_Holes ,
  AM_Tecto
};

enum GadgetWorkinMode
{
  GWM_Reject = 0 ,
  GWM_Transmit ,
  GWM_Process 
};

enum ImageProcessingOrdered
{
  IPO_NotOrdered = 0 ,
  IPO_OrderedByExternal ,
  IPO_OrdredByGetNext ,
  IPO_OrderedByBatch
};


class TColorText : public CStatic
{
protected:
  DECLARE_MESSAGE_MAP()

public:
  // make the background transparent (or if ATransparent == true, restore the previous background color)
  void setTransparent( bool ATransparent = true );
  // set background color and make the background opaque
  void SetBackgroundColor( COLORREF );
  void SetTextColor( COLORREF );
  bool MTransparent = true;
  COLORREF MBackgroundColor = RGB( 255 , 255 , 255 );  // default is white (in case someone sets opaque without setting a color)
  COLORREF MTextColor = RGB( 0 , 0 , 0 );  // default is black. it would be more clean 

protected:
  HBRUSH CtlColor( CDC* pDC , UINT nCtlColor );

private:
                                         // to not use the color before set with SetTextColor(..), but whatever...
};

class CUI_NViewsAutoProxy;

#define EvDrawTimer  10
#define EvLockModeOn 11
#define EvViewErrors 12
#define EvWritingTimer 13
#define EvRestoreRendering (EvWritingTimer+1)
#define EvGetNextImage (EvRestoreRendering+1)


#define m_FrontCalculator m_CalculatorNames[1] // left camera
#define m_SideCalculator  m_CalculatorNames[0] // Right camera

#define X_SPACE_BETWEEN_RENDERERS 10
#define Y_SPACE_BETWEEN_RENDERERS 30
#define X_REL_SPACE_BETWEEN_RENDERERS 0.01
#define Y_REL_SPACE_BETWEEN_RENDERERS 0.02

// CUI_NViews dialog
class CUI_NViews : public CDialogEx
{
	DECLARE_DYNAMIC(CUI_NViews);
	friend class CUI_NViewsAutoProxy;
  friend void ReceiveFromShMemFunc( CUI_NViews * pViews ) ;
  static CUI_NViews *m_pCurrentDlg;
  PasswordDialog m_PasswordDlg ;

// Construction
public:
	CUI_NViews(CWnd* pParent = nullptr);	// standard constructor
	virtual ~CUI_NViews();
  void	PrintMessage( LPCTSTR message ) ;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UI_NViews_DIALOG };
#endif
  // Sub windows parameters
  size_t        m_uNRows ;
  size_t        m_uNDefaultColumnsPerRow ;
  IntVector     m_NColumnsPerRow ;
  int           m_iMonitorNumber ;
  cmplx         m_csDialogSize ;
  double        m_dRelLogHeight ;
  double        m_dRelLogWidth ;
  StringVector  m_RenderNames ;
  StringVector  m_OnScreenRenderNames ;
  CmplxVector   m_RenderSizes ;
  CmplxVector   m_RenderPositions ;
  vector<CWnd*> m_Renderers ;
  vector<CWnd*> m_Statics ;
  FXSIZE        m_iViewTimeIndex = -1 ;
  double        m_dStartTime_ms ;
  IntVector     m_NotShowErrors ;
  int           m_iNRemoved = 0 ;
  int           m_iNRemovedReset = 400 ;
  BOOL          m_bEnableGraphSave = FALSE ;
  BOOL          m_bAppClosing = FALSE ;

  // For Skew Meter only
  double        m_dHorizontalLineDist_mm = 280.0;
  double        m_dVerticalLineDist_mm = 300.0 ;
  double        m_dHorizontalLineDistNominal_mm = 280.0;
  double        m_dVerticalLineDistNominal_mm = 300.0 ;
  double        m_dSkewCalib_um = 0. ;

  double        m_dDistAllowedVariation_mm = 3.0 ;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	CUI_NViewsAutoProxy* m_pAutoProxy;
	HICON m_hIcon;

  ConnectGraph m_Graph ;
  FXString     m_LastErrorMsg ;
  FXString     m_EvaluationMsg ;
  ApplicationMode m_AppMode ;

  CStringArray     m_Message ;
  bool             m_bPrintMessage ;
  FXLockObject     m_Protect ;
  CListBox         m_LogView;
  CFont            m_LogFont ;
  CFont            m_TechnicianFont ;
  CFont            m_ButtonsFont ;
  UINT_PTR         m_iWritingTimer;

  int              m_iEmptyTrayCount = 0 ;
  int              m_iSideImagesWithFlowerCount = 0 ;
  int              m_iFrontImagesWithFlowerCount = 0 ;
  int              m_iGoodCount = 0 ;
  int              m_iBadCount = 0 ;

  // Names for communications with graph
  FXString m_GraphName ;
  FXString m_ConfigName ;
  BOOL     m_bUseBMPs ;

  FXString m_FrontBMPReadDirectory ;
  FXString m_FrontBMPReadFileName ;
  FXString m_SideBMPReadDirectory ;
  FXString m_SideBMPReadFileName ;

  CStatic         m_Cam1StatusLED , m_Cam2StatusLED;
  CBitmap         m_BlackLed , m_GreenLed , m_YellowLed , m_BlueLed , m_RedLed ;
  bool     m_bCam1CallBackPassed , m_bLastDrawnCam1 , m_bLastResult1 ;
  bool     m_bCam2CallBackPassed , m_bLastDrawnCam2 , m_bLastResult2 ;
  int      m_iRestToViewCam1 = 0 , m_iRestToViewCam2 ;
  int      m_iNRepeatCycles = 0 ;
  double   m_dFrontCameraScale_um_per_pix;
  double   m_dSideCameraScale_um_per_pix;

  // variables for Tecto project
  CShMemControl * m_pShMemControl = NULL ;
  int             m_iShMemInSize_KB ;
  int             m_iShMemOutSize_KB ;
  CString         m_ShMemAreaName ;
  CString         m_InEventName ;
  CString         m_OutEventName ;
  bool            m_bFinishReceivengShMemMsgs = false ;
  ImageProcessingOrdered m_ImageProcessingOrdered = IPO_NotOrdered ;
  std::thread *   m_pSHMemReceiverThread = NULL ;
  TectoMsgId      m_WaitAnswerForThisRequest = TM_NoWaiting ;
  CString         m_CurrentFlowerName ;
  int             m_iNSources ;
  CString         m_CurrentSimulationDir ;
  CString         m_CurrentSrcImageSavingDir ;
  CString         m_CurrentResultImageSavingDir ;
  CString         m_CurrentConfigFile ;
  CStringArray    m_CurrentCalibrationFiles ;
  CString         m_StatisticsFileName ;

  double          m_dRequestTime_ms ;
  double          m_dAnswerTime_ms ;
  CallBackDataA   m_DataCam1 , m_DataCam2 ;
  BOOL CanExit();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
  HBRUSH OnCtlColor( CDC* pDC , CWnd *pWnd , UINT nCtlColor );

  void LoadPresentationParamsFromRegistry();
  LRESULT OnLogMsg( WPARAM wParam , LPARAM lParam ) ;
  DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnTimer( UINT_PTR nIDEvent );
  WM_Green GetWorkingMode();
  afx_msg void OnParentNotify( UINT message , LPARAM lParam );
  afx_msg void OnBnClickedClose();
  afx_msg void OnBnClickedViewSetup();
  afx_msg void OnBnClickedViewGraph();
  afx_msg void OnBnClickedSaveGraph();
  CRect SetNewRectPosition( CRect OriginalDlgClientRect , DWORD ID , 
    CRect * pEdges = NULL ) ;
  afx_msg void OnEnKillFocusHorizDist();
  afx_msg void OnEnKillfocusVertDist();
  virtual void OnOK();
  virtual BOOL PreTranslateMessage( MSG* pMsg );
  afx_msg void OnBnClickedTechnician();
  afx_msg void OnEnKillfocusSkewCalibUm();
  LRESULT OnShMemMessage( WPARAM wParam , LPARAM lParam ) ;
  LRESULT OnAnswerForTectoLib( WPARAM wParam , LPARAM lParam ) ;
  LRESULT OnSimuImageArrived( WPARAM wParam , LPARAM lParam ) ;
  void ShowCalibrationControls( int iMode ); // 0 - hide, 1 - show
  // For Tecto mode
  CShMemControl * GetShMemControl() { return m_pShMemControl ; } 
  void ProcessDataFromShMem() ;
  void ProcessErrorFromShMem( DWORD ErrCode ) ;
  int AnalyzeCalibration( TectoMsg * pCalibMsg ) ;
  int GetParameters( TectoMsg * pGetParametersMsg ) ;
  int AnalyzeFlower( TectoMsg * pAnalyzeFlowerMsg ) ;
  CVideoFrame *  ConvertIPImageToSH( TectoMsg * pAnalyzeFlowerMsg , int iImageNum ) ;
  int ProcessCalibrationFiles( TectoMsg * pCalibMsg , int& imageCount ,
    int camsLayout[ 10 ] , CStringArray& FileNames) ;
  int SetFlower( TectoMsg * pSetFlowerMsg  ) ;
  afx_msg void OnBnClickedSideProcSetup();
  afx_msg void OnBnClickedFrontProcSetup();
  afx_msg void OnBnClickedSaveToRegistry();
  afx_msg void OnEnChangeFlowerName();
  afx_msg void OnEnKillfocusFlowerName();
  afx_msg void OnEnUpdateFlowerName();
  afx_msg void OnBnClickedSetFlower();
  int SetFlowerAndDirectories( int iNSources , TectoMsg * pTectoMsg = NULL );
  bool CorrectGraph() ;
  bool SaveGraphCorrections() ;
  double GetVideoObjectThreshold( LPCTSTR pGadget , LPCTSTR pObject ) ;
  afx_msg void OnBnClickedGetNext();
  void ReadOrRepeatImage( bool bMayBeRepeat ) ;
  afx_msg void OnBnClickedSaveSrc();
  afx_msg void OnBnClickedSaveResult();
  afx_msg void OnBnClickedResult1();
  afx_msg void OnBnClickedSrc1();
  afx_msg void OnBnClickedResult2();
  afx_msg void OnBnClickedSrc2();
  afx_msg void OnBnClickedResult1On2();
  afx_msg void OnBnClickedBatch();

  bool IsChecked( DWORD Id )
  {
    CButton * pButton = ( CButton* ) GetDlgItem( Id ) ;
    if ( pButton )
      return ( pButton->GetCheck() == BST_CHECKED ) ;
    ASSERT( 0 ) ;
    return false ;
  }
  void SetCheck( DWORD Id , BOOL bCheck )
  {
    CButton * pButton = ( CButton* ) GetDlgItem( Id ) ;
    if ( pButton )
      pButton->SetCheck( bCheck ? BST_CHECKED : BST_UNCHECKED ) ;
    else
      ASSERT( 0 ) ;
  }
  void EnableControl( DWORD Id , BOOL bEnable )
  {
    CWnd * pControl = GetDlgItem( Id ) ;
    if ( pControl )
      pControl->EnableWindow( bEnable ) ;
    else
      ASSERT( 0 ) ;
  }
};

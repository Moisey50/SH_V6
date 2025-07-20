
// UIFor2ViewsDlg.h : header file
//

#pragma once

#include <gadgets/gadbase.h>
#include <helpers\shwrapper.h>
//#include <helpers\registry.h>

// Quasi IDCs
#define QIDC_MOVE_FRAGMENT IDC_PROTECT
#define QIDC_POS_FROM IDC_HLINE_POS
#define QIDC_POS_TO   IDC_OFFSET_Z

enum USER_WM
{
  USER_WM_CALLBACK_TEXT = WM_APP + 1 ,
  USER_WM_CALLBACK_DATA ,
  USER_WM_LOG_MSG ,
  USER_WM_NEW_MEASUREMENT_RESULT
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
  AM_Holes
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

class CUIFor2ViewsDlgAutoProxy;

#define EvDrawTimer  10
#define EvLockModeOn 11
#define EvViewErrors 12
#define EvWritingTimer 13
#define EvRestoreRendering (EvWritingTimer+1)
#define m_FrontCalculator m_CalculatorNames[1] // left camera
#define m_SideCalculator  m_CalculatorNames[0] // Right camera



// CUIFor2ViewsDlg dialog
class CUIFor2ViewsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CUIFor2ViewsDlg);
	friend class CUIFor2ViewsDlgAutoProxy;
  static CUIFor2ViewsDlg *m_pCurrentDlg;

// Construction
public:
	CUIFor2ViewsDlg(CWnd* pParent = nullptr);	// standard constructor
	virtual ~CUIFor2ViewsDlg();
  void	PrintMessage( LPCTSTR message ) ;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UIFOR2VIEWS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	CUIFor2ViewsDlgAutoProxy* m_pAutoProxy;
	HICON m_hIcon;

  ConnectGraph m_Graph ;
  FXString     m_LastErrorMsg ;
  FXString     m_EvaluationMsg ;
  ApplicationMode m_AppMode ;

  // Parameters for Green Machine
  int m_iHLinePos;
  int m_iZOffset_um;
  int m_iYOffset_um;
  int m_iXOffset_um;

  // Parameters for Holes Application
  int m_iVLinePos;
  int m_iPosFrom_pix ;
  int m_iElectrodeRight_x_abs ;
  int m_iPosTo_pix ;

  int m_iLineDist_T; // in tenth
  int m_iSideXOffset_Tx10 ;

  int m_iTolerance_um ;
  CStringArray     m_Message ;
  bool             m_bPrintMessage ;
  FXLockObject     m_Protect ;
  CListBox         m_LogView;
  int              m_iBaseLinePos ;
  double           m_dOffsetFromBaseLine_um ;
  int              m_iSideExposure_us ;
  int              m_iFrontExposure_us ;
  int              m_iElectrodeWidth_Tenth;
  int              m_iWedgeWidth_Tenth;
  int              m_iVertLinePos_pix;
  int              m_iHorLinePos_pix;
  int              m_iWedgeXMeasPos_pix ;

  int              m_iRestFrontWritingSeconds;
  int              m_iRestSideWritingSeconds;
  UINT_PTR         m_iWritingTimer;

  //CSpinButtonCtrl  m_SpinFrontExp ;
  //CSpinButtonCtrl  m_SpinSideExp ;

  CSpinButtonCtrl  m_SpinHLine;
  CSpinButtonCtrl  m_SpinOffsetZ;
  CSpinButtonCtrl  m_SpinOffsetY;
  CSpinButtonCtrl  m_SpinOffsetX;
  CSpinButtonCtrl  m_SpinTolerance;
  CSpinButtonCtrl  m_SpinElectrodeWidth;
  CSpinButtonCtrl  m_SpinWedgeWidth;
  CSpinButtonCtrl  m_SpinVertLinePos;
  CSpinButtonCtrl  m_SpinHorLinePos;
  CSpinButtonCtrl  m_SpinWedgeXMeasureYPos;

//   CSpinButtonCtrl  m_SpinK;
//   CSpinButtonCtrl  m_SpinSideOffsetX;


  // Names for communications with graph
  FXString m_GraphName ;
  FXString m_CameraName ;
  FXString m_RotateName ;
  FXString m_ImagingName ;
  FXString m_StraightSearchName ;
  FXString m_RenderName ;
      // For green machine
  FXStringArray m_CalculatorNames ;
    // For Holes Application

  BOOL     m_bUseBMPs ;
  FXString m_FrontBMPReadDirectory ;
  FXString m_FrontBMPReadFileName ;
  FXString m_SideBMPReadDirectory ;
  FXString m_SideBMPReadFileName ;

  CStatic         m_Cam1StatusLED , m_Cam2StatusLED;
  CBitmap         m_BlackLed , m_GreenLed , m_YellowLed , m_BlueLed , m_RedLed ;
  bool     m_bCam1CallBackPassed , m_bLastDrawnCam1 ;
  bool     m_bCam2CallBackPassed , m_bLastDrawnCam2 ;
  double   m_dFrontCameraScale_um_per_pix;
  double   m_dSideCameraScale_um_per_pix;

  CallBackDataA m_DataCam1 , m_DataCam2 ;
  BOOL CanExit();
  void DisableControls() ;
  void SetControlsForGreenMachine() ;
  void SetControlsForHoles() ;
  bool SetParametersToTVObject( LPCTSTR pGadgetName , LPCTSTR pPin , LPCTSTR pParams );

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
  HBRUSH OnCtlColor( CDC* pDC , CWnd *pWnd , UINT nCtlColor );

  void LoadNamesFromRegistry();
  LRESULT OnLogMsg( WPARAM wParam , LPARAM lParam ) ;
  DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnBnClickedWorkMode();
  afx_msg void OnBnClickedGraphView();
  afx_msg void OnBnClickedSetupView();
  afx_msg LRESULT OnShowVOSetupDialog( WPARAM wParam , LPARAM lParam );
  afx_msg LRESULT OnCameraMsg( WPARAM wParam , LPARAM lParam );
  // iMode == 1 - free run, ==2 - lock
  int SetWorkingMode( int iMode );
  afx_msg void OnTimer( UINT_PTR nIDEvent );
  afx_msg void OnBnClickedProtect();
  WM_Green GetWorkingMode();
  afx_msg void OnDeltaposHlineSpin( NMHDR *pNMHDR , LRESULT *pResult );
  void OnCoordSpinDeltaPos( NMHDR *pNMHDR , int& iResult ,
    CSpinButtonCtrl& Spin , size_t IDC , 
    LPCTSTR pszRegValName , LPCTSTR pszCalcAddition ) ;
  afx_msg void OnDeltaposOffsetZSpin( NMHDR *pNMHDR , LRESULT *pResult );
  afx_msg void OnDeltaposOffsetYSpin( NMHDR *pNMHDR , LRESULT *pResult );
  afx_msg void OnDeltaposOffsetXSpin( NMHDR *pNMHDR , LRESULT *pResult );
  afx_msg void OnDeltaposToleranceSpin( NMHDR *pNMHDR , LRESULT *pResult );
  void CUIFor2ViewsDlg::OnChangeCoordOffset( int& iVal , CSpinButtonCtrl& Spin ,
    size_t IDC , LPCTSTR pszRegValName , LPCTSTR pszCalcAddition ) ;
  afx_msg void OnEnChangeOffsetZ();
  afx_msg void OnEnChangeOffsetY();
  afx_msg void OnEnChangeOffsetX();
  afx_msg void OnEnChangeTolerance();
  TColorText m_SideResultView;
  TColorText m_FrontResultView;
  afx_msg void OnBnClickedMultiwedge();
  CButton m_MultiEdgeCheckBox;
  afx_msg void OnEnChangeHlinePos();
  afx_msg void OnStnClickedExposureFrontCaption();
  afx_msg void OnEnChangeExposureFront();
  afx_msg void OnEnChangeExposureSide();
  afx_msg void OnBnClickedSaveSideVideo();
  afx_msg void OnBnClickedSaveBothVideo();
  afx_msg void OnBnClickedSaveFrontVideo();
  afx_msg void OnBnClickedSaveOneSideBmp();
  afx_msg void OnBnClickedSaveOneFrontBmp();
  afx_msg void OnBnClickedDownLight();
  afx_msg void OnBnClickedFrontLight();
  afx_msg void OnBnClickedFrontExpPlus( );
  afx_msg void OnBnClickedFrontExpMinus( );
  afx_msg void OnBnClickedSideExpPlus( );
  afx_msg void OnBnClickedSideExpMinus( );
  afx_msg void OnEnChangeElectrodeWidthT( );
  afx_msg void OnStnClickedStatic7( );
  afx_msg void OnEnChangeWedgeWidthT( );
  afx_msg void OnEnChangeHorLinePos( );
  afx_msg void OnEnChangeVertLinePos( );
  afx_msg void OnEnChangeWedgeXMeasPos();
  afx_msg void OnParentNotify( UINT message , LPARAM lParam );
  afx_msg void OnBnClickedGraphSave();
};

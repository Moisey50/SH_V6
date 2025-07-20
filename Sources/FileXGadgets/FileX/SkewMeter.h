#pragma once
#include <fxfc\fxfc.h>
#include "helpers\UserBaseGadget.h"
#include "gadgets\videoframe.h"
#include "gadgets\QuantityFrame.h"
#include <gadgets\FigureFrame.h>
#include <imageproc/ImageProfile.h>
#include <Math\PlaneGeometry.h>
#include <helpers\FramesHelper.h>
#include <Math\FRegression.h>
#include <Math/FigureProcessing.h>

enum SkewMeterGadgetMode
{
  SMGM_Unknown = 0 ,
  SMGM_LeftTop ,
  SMGM_Right ,
  SMGM_Bottom
};

enum CaptureMode
{
  CM_Pause = 0 ,
  CM_LiveProcess = -1 ,
  CM_LiveView = -2 ,
  CM_OneProcess = 1 ,
  CM_Quit = 255
};

enum OutputConnectors
{
  OC_MainOutput = 0 ,
  OC_ResultView ,
  OC_CamControl
};

#define N_SAMPLES_FOR_STABILITY_CHECK 10

enum CalibrationCommand
{
  CC_NoCalibaration = 0 ,
  CC_SkewCalibrationB ,
  CC_SkewCalibrationA ,
  CC_HandVSizeCalibration
};

enum PowerStatus
{
  PS_Unknown = 0 ,
  PS_UPSExtPowerSupp = 1 ,
  PS_UPSBattMoreThanHalf = 2 ,
  PS_UPSBatteryIsCharging = 4 ,
  PS_ComputerBattIsCharging = 8 ,
  PS_ComputerExtPower = 16 ,
  PS_UPSExtPowerUpdated = 0x1000 ,
  PS_UPSBattMoreThanHalfUpdated = 0x2000 ,
  PS_UPSBatteryIsChargingUpdated = 0x4000 ,

  PS_UPSDataUpdated = PS_UPSExtPowerUpdated | PS_UPSBattMoreThanHalfUpdated
    | PS_UPSBatteryIsChargingUpdated
};

void CALLBACK MainLoopTimerRoutine(
  LPVOID lpParam , BOOLEAN TimerOrWaitFired ) ;


class SkewMeter :
  public UserBaseGadget
{
  friend void CALLBACK MainLoopTimerRoutine(
    LPVOID lpParam , BOOLEAN TimerOrWaitFired ) ;

public:
  SkewMeter();
  ~SkewMeter();

  static   int m_iViewMode ;
  double   m_dInitialDirection_deg = 180. ;
  double   m_dScale_um_per_pix = 3.5777;
  double   m_dSquareSize_um = 7000.0 ;
  double   m_dTolerance_um ;
  double   m_dSearchWidth_pix = 30. ;
  double   m_dSquareSearchArea_perc = 92. ;
  double   m_dLineSearchThres = 0.25 ;
  double   m_dMinLineWidth = 3. , m_dMaxLineWidth = 35. ;
  double   m_dMinimalContrast = 60. ; // in brightness units
  CalibrationCommand m_CalibCommand = CC_NoCalibaration ;
  int      m_iRecordPeriod_sec = 0 ;
  FXString     m_sThresholdForLog ;
  double       m_dSkewThresForLog_um = 0. ;
  double       m_dSizeThresForLog_um = 0. ;
  double       m_dHLineDistThresForLog_um = 0. ;


  SkewMeterGadgetMode m_GadgetMode ;

  const CVideoFrame * m_pLastVideoFrame = NULL ;
  double              m_dLastFrameTime = 0. ;
  double              m_dLastFrameOrderTime = 0. ;
  CRect    m_LastROI ;
  cmplx    m_cLastROICent_pix ;
  cmplx    m_cInitialSearchPt_pix ;
  cmplx    m_cInitOrthoSearchPt_pix ;
  double   m_dDirectionOnVideo_rad ;
  double   m_dOrthoDirectionOnVideo_rad ;
  static   double m_dVertDistBetweenSquares_mm ;
  static   double m_dHorDistBetweenSquares_mm ;
  static   double m_dBaseHorDist_mm ;
  static   double m_dBaseVertDist_mm ;

  CmplxVector m_MainFalls , m_MainRises , m_MainCenters;
  CmplxVector  m_OrthoFalls , m_OrthoRises , m_OrthoCenters;
  CmplxVectors m_MainLines , m_OrthoLines ;
  LinesVector  m_MainLinesRegression , m_OrthoLinesRegression ;

  CmplxVectors m_HorLines , m_VertLines ;
  LinesVector  m_HorLinesRegression , m_VertLinesRegression ;
  DoubleVector m_HWidths , m_VWidths ;
  CmplxVector  m_Intersections ;
  CDRect       m_IntersectionsArea ;
  Tetragon     m_Square ;
  cmplx        m_cHorCross , m_cVertCross ;
  cmplx        m_cRectCenter_pix ;
  cmplx        m_cVHLinesCross_pix ;
  cmplx        m_cRectCenterToFOVCenter_pix ;
  cmplx        m_cVHLinesCrossToFOVCenter_pix ;
  cmplx        m_cRectCenterToFOVCenter_um ;
  cmplx        m_cVHLinesCrossToFOVCenter_um ;
  cmplx        m_cAvgRectCenterToFOVCenter_um ;
  cmplx        m_cAvgVHLinesCrossToFOVCenter_um ;
  cmplx        m_cLineCrossOffsetToRectCenter_um ;

  cmplx        m_cUpperVect , m_cRigthVect , m_cBottomVect , m_cLeftVect ;
  cmplx        m_cLTtoRBVect , m_cRTtoLBVect ;

  // Exposure control
  int          m_iCurrentExposure_us ;

  cmplx        m_cCrossesAverage ;
 // Measured data from 3 gadgets
  static cmplx    m_cLeftTopRectCenter_pix ;
  static cmplx    m_cRightRectCenter_pix ;
  static cmplx    m_cBottomRectCenter_pix ;

  static cmplx    m_cLeftTopLinesCross_pix ;
  static cmplx    m_cRightLinesCross_pix ;
  static cmplx    m_cBottomLinesCross_pix ;

  static double   m_dHorLineVertShiftFromLUCorner_um ;
  static double   m_dVertLineHorShiftFromLUCorner_um ;

  static double   m_dHorLineVertShiftFromRUCorner_um ;
  static double   m_dVertLineHorShiftFromRUCorner_um ;

  static double   m_dHorLineVertShiftFromLDCorner_um ;
  static double   m_dVertLineHorShiftFromLDCorner_um ;

  static cmplx    m_cLeftTopLineCrossToRectCent_um ;
  static cmplx    m_cRightLineCrossToRectCent_um ;
  static cmplx    m_cBottomLineCrossToRectCenter_um ;
  static double   m_dParallaxScale ; // um per um of distance to rectangle center

  static double   m_dA ; // Y = ax + b, for lens distortion correction
  static double   m_dB ;
  static bool     m_bLEL_OK ;
  static bool     m_bLER_OK ;
  static bool     m_bTEL_OK ;
  static bool     m_bLEL_Stable ;
  static bool     m_bLER_Stable ;
  static bool     m_bTEL_Stable ;
  CmplxVector     m_CrossCenters ;
  CmplxVector     m_RectCenters ;

  double          m_dSkew_deg ;
  double          m_dSkew_um ;
  double          m_dCorrectedSkew_um ;
  double          m_dSkewByCent_deg ;
  double          m_dSkewByCent_um ;
  double          m_dCorrectedSkewByCent_um ;
  double          m_dHorErr_um ;
  double          m_dVertErr_um ;
  bool            m_bSkewOK = false ;
  bool            m_bHScaleOK = false ;
  bool            m_bVScaleOK = false ;
  double          m_dLastSquareStd = 0. ;
  double          m_dLastCrossStd = 0. ;

  static bool     m_bResultOK ;
  double          m_dCurrentMax ;
  double          m_dCurrentMin ;
      // Target rectangle view parameters
  DWORD   m_TargetRectColor = 0 ;
  int     m_iTargetRectThickness = 3 ;
  int     m_TargetRectStyle = PS_DASH ;


  // Result view parameters

  DWORD   m_BorderResultColor = 0xffc0c0 ;
  int     m_iPageBorderThickness = 3 ;
  int     m_PageBorderStyle = PS_DASH ;
  DWORD   m_PageColor = 0xc0c0c0 ;

  // Power status data

  FXLockObject        m_LockTimer ;
  HANDLE              m_hMainTimer = NULL ;
  int                 m_dwTimerPeriod = 100 ;
  int                 m_dwPowerStatusCheckTicks = 10 ; // 1 second for start up
  static bool         m_bBatteriesStatusUpdated ;
  static bool         m_bBatteriesAreOK ;
  int                 m_iTimeCount = 0 ;
  double              m_dTimerDeleteOrderTime = 0. ;
  static int          m_PowerStatus ;
  static int          m_PowerStatusSaved ;
  SYSTEM_POWER_STATUS m_SystemPowerStatus ;
  FXString            m_PowerStatusAsString ;
  CContainerFrame *   m_pLastResultViewWithoutPower = NULL ;
  double              m_dLastResultViewTime = 0. ;


  // Capture mode: 0 - pause, -1 - live 10 fps, n>0 - take n frames
  static int          m_iCaptureMode ;
  // Mode switching rectangles
  CRect               m_LiveProcessingRect ;
  CRect               m_LiveViewRect ;
  CRect               m_SingleFrameProcessingRect ;
  CRect               m_QuitRect ;
  int                 m_iLastSelectedByUI = 0 ;

  // Log variables
  FXString            m_LogFileName ;
  double              m_dLastLoggedTime_ms = 0. ;

  double              m_dPrevSkew_um = 0. ;
  double              m_dPrevHError_um = 0. ;
  double              m_dPrevVError_um = 0. ;
  double              m_dPrevHLineDist_um = 0. ;



  //   void ShutDown();
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );

  void PropertiesRegistration();
  void ConnectorsRegistration();
  static void ConfigParamChange( LPCTSTR pName , 
    void* pObject , bool& bInvalidate , bool& bInitRescan ) ;

  LPCTSTR AnalyzePower();
  int AnalyzeUPSStatus( const CTextFrame * pData ) ;
  int AddPowerStatusAndControls( CContainerFrame * pView );
  int CalcDistsAndViewRectData( CContainerFrame * pMarking );
  bool DeleteAsyncTimer() ;
  int FillTetragon( CDRect& IntersectionArea , CmplxVector& Points , Tetragon& Result );
  int FindLineCrosses( cmplx cInitialPt_pix ,
    double dDirectionOnVideo_rad , bool bOrtho , CContainerFrame * pOut ) ;
  double FindProfileCrossesPosition( Profile& Prof , // profile data
    int& iStartIndex ,   // the first point for line search
    double dNormThres ,   // normalized threshold (0.,1.) range
    double& dWidth ) ;
  int FormAndSendResultView();
  void ProcessBottomImage( const CDataFrame * pDataFrame , CContainerFrame * pMarking ) ;
  void ProcessLeftTopImage( const CDataFrame * pDataFrame , CContainerFrame * pMarking ) ;
  int ProcessProfile( Profile& Prof , DoubleVector& Results , Profile& OutProfile ) ;
  void ProcessRightImage( const CDataFrame * pDataFrame , CContainerFrame * pMarking ) ;
  virtual void ShutDown() ;
  int SimplifiedProcessing( CContainerFrame * pMarking ) ;

  void ProcessTimer();
  int IsThereClickedButton( int iXCursor , int iYCursor );
  int DoOrderGrab();
  int SetCameraExposure( int iExposure_us );
  BOOL CameraControl( LPCTSTR pCommand );

  DECLARE_RUNTIME_GADGET( SkewMeter );
} ;

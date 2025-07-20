#pragma once
#include "helpers/UserBaseGadget.h"

#include "gadgets\VideoFrame.h"
#include "gadgets\TextFrame.h"
#include "gadgets\QuantityFrame.h"
#include "gadgets\RectFrame.h"
#include "gadgets\FigureFrame.h"
#include "Gadgets\containerframe.h"
#include "helpers/FramesHelper.h"
#include "math/PlaneGeometry.h"
#include "math/FigureProcessing.h"
#include <fxfc/FXRegistry.h>
#include <imageproc/statistics.h>
#include <imageproc/VFrameEmbedInfo.h>
#include <..\..\gadgets\common\imageproc\clusters\Segmentation.h>
#include <imageproc\ExtractVObjectResult.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>      // std::stringstream
#include "Dispenser.h"

#define N_Samples 10 // n samples for eccentricity calculation
#define MotionStep 0.1 // Motion = MotionStep * <circle diameter>
#define EccThreshold 1.0 // Acceptable eccentricity
#define EccRelDiaThres 0.05 // If eccentricity more than this threshold

#define N_MEASUREMENTS_PER_TURN 4 // should be multiple of 4
#define N_SEGMENTS_FOR_CIRCLE_EXTRACTION 72

enum MPPDInputs
{
  MPPDI_MainInput = 0 
};

enum MPPDOutputs
{
  MPPDO_VideoOut = 0 ,
  MPPDO_DataOut ,
  MPPDO_CameraControl ,
  MPPDO_Measurement_Control 
};

enum MPPD_WorkingMode
{
  MPPD_NotSet = -1 ,
  MPPD_Side = 0,
  MPPD_Front ,
  Ako_Tek
};

enum MPPD_LastResult
{
  LR_Unknown = 0 ,
  LP_TooSmall ,
  LP_OK ,
  LP_TooBig ,
  LP_BigMaxMin
};
enum MPPD_Stage
{
  STG_Unknown = -1 ,
  STG_Idle    = 0 ,
  STG_Stopped = 100 ,
  STG_ShortCommand ,
  STG_SideMeasureAndView ,
  STG_SideCalibration ,
   // Front view and measurement
  STG_FrontBlackMeasAndView = 500 , // For focusing, "black" image
  STG_FrontWhiteMeasAndView , // "white" spot on front
  STG_FrontWhiteFinalMeasurement ,
  STG_FrontBlackFinalMeasurement ,
  STG_FrontWhiteFinalWithRotation ,
  STG_SideFinalMeasurement ,
  STG_FrontBlackCalibration , // calibration by front light
  STG_FrontWhiteCalibration , // calibration by back light
  STG_FrontFocusing ,

  
  // Common centering process
  STG_FrontSynchronization = 700 ,
  STG_MoveXPlusCalibration ,
  STG_MoveXMinusCalibration ,
  STG_MoveYPlusCalibration ,
  STG_MoveYMinusCalibration ,
  STG_Get0DegImage ,
  STG_Get90DegImage ,
  STG_Get180DegImage ,
  STG_Get270DegImage ,
  STG_CorrectX ,
  STG_CorrectY ,
  STG_Get0DegImageForAverage ,
  STG_WaitForMotorStop ,
  
    // Master Centering
  STG_MasterRawSync = 1000 , // Initial sync with rotation, one frame per sync
  STG_MasterXMeasure ,       // Measure X shift
  STG_MasterXAdjust ,        // X Command to motors
  STG_MasterYMeasure ,       // Measure Y shift
  STG_MasterYAdjust ,        // Y command to motors
   // Master measurement
  STG_MasterMeas = 2000 , // Switch on side measurement and view
  STG_MasterCalibration , // Take and save calibration data
   // Side Stones measurement
  STG_CoarseStoneMeas = 3000 , // Switch  on Coarse stone measurement and view
  STG_FineStoneMeas ,          // Switch  on Fine stone measurement and view
  STG_CoarseTipPosition ,
  STG_FineTipPosition ,
  // Blank Centering by white
  STG_BlankRawSync = 4000 , // Initial sync with rotation, one frame per sync
  STG_BlankXMeasure ,       // Measure X shift
  STG_BlankXAdjust ,        // X Command to motors
  STG_BlankYMeasure ,       // Measure Y shift
  STG_BlankYAdjust ,        // Y command to motors
    // Coarse side grinding
  STG_SideCoarseGrinding = 5000 ,
    // There will be centering by white
  STG_SideFineGrinding = 6000 ,
    // Front stone initial adjustment until grinding process first signs
  STG_FrontStoneInit = 7000 , 
  STG_FrontStoneTouchWait ,
    // Front grinding until last "5 microns"
  STG_FrontGrinding = 8000 ,
    // Front polishing
  STG_FrontPolishing = 9000 ,
  STG_SidePolishing = 10000
};

inline bool bIsSyncWork( MPPD_Stage Stage )
{
  return ((STG_FrontSynchronization <= Stage)
    && (Stage <= STG_Get0DegImageForAverage)) ;
}
enum SS_SyncAimMode
{
  SS_NoSync = 0 ,
  SS_SimpleSync ,
  SS_SyncForCentering ,
  SS_SyncForCorrection ,
  SS_SyncForCalibration ,
  SS_SyncForProduction ,
};

enum MPPD_ImagingMode
{
  MDI_SimpleConturs = 0 ,
  MDI_ExtByDiff
};

enum MPPD_TriggerMode
{
  TM_NoTrigger = 0 ,
  TM_OneFrame ,
  TM_Burst             // NFrames per trigger
};
enum MotionPhase
{
  MF_NoMotion = 0 ,
  MF_XPlus ,
  MF_XPlus2 ,
  MF_XMinus ,
  MF_YPlus ,
  MF_YPlus2 ,
  MF_YMinus
};

enum CenteringStatus
{
  CNTST_Unknown = 0 ,
  CNTST_Idle = 1 ,
  CNTST_Syncing ,
  CNTST_MeasureByWhite , // frontal measurement, back light is used
  CNTST_AdjustForWhite ,
  CNTST_SyncForGrinding ,
  CNTST_MeasureByBlack , // Frontal measurement, front light is used
  CNTST_AdjustForBlack ,
  CNTST_Finished ,
  CNTST_Fault
};

enum LightMask
{
  LightMask_Unknown = 0 ,
  LightMask_Front = 1 ,
  LightMask_Back = 2 ,
  LightMask_Side = 4 ,
  LightMaskFrontAndBack = (LightMask_Front | LightMask_Back)
};

enum DispensSaveMode
{
  DispensSaveMode_No = 0 ,
  DispensSave_Final ,
  DispensSave_Bad ,
  DispensSaveMode_All ,
  DispensSaveMode_OnePerSerie ,
  DispensSaveMode_Decimate
};

enum PartParametersMask
{
  PPM_PartId = 1 ,
  PPM_DI = 2 ,
  PPM_DITol = 4 ,
  PPM_DO = 8 ,
  PPM_DOTol = 0x10 ,
  PPM_Ecc = 0x20 ,
  PPM_EccTol = 0x40 ,
  PPM_DIDiffTol = 0x80 ,
  PPM_ConusAngle = 0x100 ,
  PPM_ConusAngleTol = 0x200 
};

enum CenteringDirections
{
  CD_XPlus = 0 ,
  CD_YPlus = N_MEASUREMENTS_PER_TURN / 4 ,
  CD_XMinus = N_MEASUREMENTS_PER_TURN * 2 / 4 ,
  CD_YMinus = N_MEASUREMENTS_PER_TURN * 3 / 4
};

enum CenterMoving
{
  CM_MoveX = 0 ,
  CM_MoveY
};

class CircleData
{
public:
  string m_Name ;
  int    m_iPositionInTurn ; // 0 is X+
  double m_dTime ;
  double m_dRadius ;
  cmplx m_cCenter ;
  bool m_bBackLight ;

  CircleData( LPCTSTR pName , double dRadius , cmplx cCenter , 
    double dTime = 0. , bool bBackLight = false )
  {
    m_Name = pName ;
    m_dRadius = dRadius ;
    m_cCenter = cCenter ;
    m_dTime = dTime ;
    m_bBackLight = bBackLight ;
    m_iPositionInTurn = -1 ;
  }
  CircleData& operator=( CircleData& Orig )
  {
    m_Name = Orig.m_Name ;
    m_dRadius = Orig.m_dRadius ;
    m_cCenter = Orig.m_cCenter ;
    m_dTime = Orig.m_dTime ;
    m_bBackLight = Orig.m_bBackLight ;
    m_iPositionInTurn = Orig.m_iPositionInTurn ;
    return *this ;
  }
};

class EccData
{
public:
  double m_dExccent ;
  cmplx m_dLastMotion ;
  cmplx m_dLastAccumulated ;
};

typedef vector<const CDataFrame*> FramesVector ;
typedef vector<CircleData> CirclesVector ;

enum WhatDispenserFigureFound
{
  WDFF_NotFound = 0 ,
  WDFF_WhiteContur = 1 ,

  WDFF_HoleEdge = WDFF_WhiteContur << 1 ,
  WDFF_ConeEdge = WDFF_HoleEdge << 1 ,

  WDFF_SideEdge = WDFF_ConeEdge << 1 ,
  WDFF_Segment = WDFF_SideEdge << 1 ,
  WDFF_SideStone = WDFF_Segment << 1 ,

  WDFF_AkoTek = WDFF_SideStone << 1 ,

  WDFF_Conturs = ( WDFF_HoleEdge | WDFF_ConeEdge 
  | WDFF_SideEdge | WDFF_WhiteContur | WDFF_SideStone | WDFF_AkoTek ) ,
  WDFF_Circles = ( WDFF_HoleEdge | WDFF_ConeEdge | WDFF_WhiteContur) 
};

typedef struct
{
  LPCTSTR pName ;
  WhatDispenserFigureFound ID ;
} NameIDPair;

class DispenserData
{
public:
  string m_Type ;
  double m_dInternalDia_um ;
  double m_dExternalDia_um ;
  double m_dMaxPtDeviation_pix ;

  double m_dConusAngle_deg ;
  double m_dSideDia_um ;

};

class MPPT_Dispens :
  public UserBaseGadget 
{
  static FXLockObject m_MPPD_Lock ;
  vector<MPPT_Dispens*> m_ExistentGadgets ;
  MPPT_Dispens * m_pMsgOrigin ;
public:
  FXString m_GadgetName ;
  // Properties
  int    m_iViewMode ;

  static FXString m_PartName ;      // It should be the same on all gadgets
  static double m_dCalibODia_um ;   // --------"------------
  static Dispenser        m_CurrentPart ;
  static Dispensers       m_KnownParts ;
  static DispenserProcessingResults m_LastResults ;
  static bool             m_bReadyToGrind;
  static double           m_dLastFrontDistaceToStone_um ;
  // Front properties
  double m_dCalibIDia_um = 41. ; // --------"------------

  double m_dMaxPtDeviation_pix ;
  double m_dInternalCorrection_pix ;
  double m_dExternalCorrection_pix ;
  double m_dImagingDiffThreshold ;
  int    m_iAveraging ;
  CSize  m_LastFOV;

  DispensSaveMode m_SaveMode ;

  FXString m_SelectedPartName ;

  LPTSTR           m_pKnownNames = NULL ;
  long             m_iSelectedPart ; // currently selected part
  MPPD_WorkingMode m_WorkingMode ;
  MPPD_WorkingMode m_OldWorkingMode ;
  bool             m_bRestoreScales ;
  MPPD_Stage       m_WorkingStage ;
  MPPD_Stage       m_PreviousStage ;
  SS_SyncAimMode   m_ForWhatSync ;
  bool             m_bMotorIsOn = false;
  bool             m_bMotorForFrontGrinding = false ;
  bool             m_bWaitForMotorOff = false ;
  MPPD_ImagingMode m_ImagingMode ;
  CircleExtractMode m_CircExtractMode ;
  MPPD_TriggerMode m_TriggerMode = TM_NoTrigger ;
  bool             m_bWasShiftedPhase = false ;
  bool             m_bUseFrontLight = false;
  bool             m_bUseBackLight = false;
  bool             m_bDoMeasurement = true ;
  int              m_iCurrentLightMask = 0 ;
  int              m_iExtractedForCalibration; //bit0 - internal, bit1 - external
  CRect            m_CameraROI;
  double           m_dMinBrightnessAmplitude = 40. ;
  int              m_iNMaxImagesWithLowContrast = 30 ;
  int              m_iLastConstrast = 0 ;
  int              m_iNLowContrastImages = 0 ;

  HANDLE           m_hWatchDogTimer = NULL ; //For lost images detection
  FXLockObject     m_LockWatchdog;
  FXString         m_CommandTimeStamp ;
  FXString         m_DoneTimeStamp ;
  FXString         m_CurrentCSVLogFilePath ;
  FXString         m_CurrentLogFilePath ;
  FXString         m_CurrentOperativeLogFilePath ;
  FXString         m_GrindingLogPath ;
  FXString         m_Info2 ;
  FXString         m_CurrentDataDir ;
  FXString         m_CurrentReportsDir ;
  FXString         m_CurrentImagesDir ;
  FXString         m_CurrentLogDir ;

  // Working variables
  cmplx m_cXVector ; // This is direction of object moving 
                     // in FOV when only X axis going to the (+)
                     // Absolute value of this vector is moving
                     // when motor activated for 100 ms
  cmplx m_cYVector ; // THe same for Y axis. Should be orthogonal to X vector

  FramesVector m_AccumulatedFrames ;
  FramesVector m_LastFigures ;
  LinesVector  m_LastSegments ;
  DoubleVector m_CaptureTimes ;
  SpotArray    m_LastSpots ;
  double       m_dLastCaptureTime_ms ;
  double       m_dPrevCaptureTime_ms = 0.;
  double       m_dRotationPeriod_ms ;
  double       m_dRotationPeriodTolerance ;
  bool         m_bIsSynchronized ;
  bool         m_bIsBlackMeasurement ;
  bool         m_bMeasureFocus ;
  bool         m_bDone ;
  bool         m_bWhiteIsOK ;
  bool         m_bWhiteWasUsed;
  bool         m_bCirclesPreprocessingIsOK ;
  bool         m_bCenteringWithBlack;
  bool         m_bCheckAsBlank = false ;
  bool         m_bMasterInserted = true;
  bool         m_bwasStoneAdjustment = false;
  int          m_iFrameCntAfterDone ;
  double       m_dCenteringTolerance_pix ;
  double       m_dXMotionDist_pix ;
  double       m_dYMotionDist_pix ;

  double       m_dScale_um_per_pix ; // Side scale and average scale for front
  double       m_dInternalBlackScale_um_per_pixel ; 
  double       m_dInternalWhiteScale_um_per_pixel ;
  double       m_dExternalScale_um_per_pixel ;
  double       m_dLastScalingTime = 0. ;
  int          m_iPolishSteps = 0 ;

  double       m_dLastDI_pix = 0.;
  double       m_dLastDI_um = 0.;
  double       m_dLastWhiteDI_pix = 0.;
  double       m_dLastWhiteDI_um = 0.;
  double       m_dLastBlackDI_pix = 0.;
  double       m_dLastBlackDI_um = 0.;
  double       m_dLastWhiteRotationDI_um = 0. ;
  double       m_dLastTirB_um = 0. ;
  double       m_dLastTirW_um = 0. ;
  double       m_dLastTirWRotation_um = 0. ;

  double       m_dLastMinDI_pix = 0.;
  double       m_dLastMinWhiteDI_pix = 0.;
  double       m_dLastMaxWhiteDI_pix = 0.;
  double       m_dLastMinWhiteDI_um = 0.;
  double       m_dLastMaxWhiteDI_um = 0.;
  double       m_dLastMinWhiteRotationDI_um = 0.;
  double       m_dLastMaxWhiteRotationDI_um = 0.;
  double       m_dLastDiaDiffToMin_um;
  double       m_dLastDiaDiffToNominal_um;
  double       m_dLastDiaDiffToMax_um;
  double       m_dLastDiaDiffMinToMin_um; // if >= 0 then OK
  double       m_dLastDiaDiffMaxToMax_um; // if <= 0 then OK
  double       m_dLastGrindingDist_um;
  double       m_dLastDiffToUse_um;
  double       m_dLastMinDiaAfterGrinding_um = 0. ;

  double       m_dLastMaxDI_pix = 0.;
  double       m_dLastMinBlackDI_pix = 0.;
  double       m_dLastMaxBlackDI_pix = 0.;
  double       m_dLastMinBlackDI_um = 0.;
  double       m_dLastMaxBlackDI_um = 0.;

  double       m_dLastExternalDia_pix = 0.;
  double       m_dLastExternalDia_um = 0.;
  double       m_dLastSideExternalDia_um = 0. ;
  double       m_dLastMinExternalDia_pix = 0.;
  double       m_dLastMaxExternalDia_pix = 0.;
  double       m_LastAveragedConeAngle_deg = 0. ;

  BOOL         m_bMinToMinForFrontGrinding = FALSE;
  BOOL         m_bWasGrindingOrPolishing = FALSE;

  BOOL         m_bUseWhiteForFinalDI = TRUE;
  BOOL         m_bUseWhiteForFinalMinMax = TRUE;
  int          m_iAverageForFinalResults = 1 ;
  int          m_iFinalAverageCntr = 0;
  double       m_dFinalDIAveraged_um;
  double       m_dFinalDIMinAveraged_um;
  double       m_dFinalDIMaxAveraged_um;
  double       m_dTirBAveraged_um;
  double       m_dTirWAveraged_um;
  double       m_dBlackDI_um[40];
  double       m_dBlackDImin_um[40];
  double       m_dBlackDImax_um[40];

  bool         m_bHighBrightnessMode = false ; 
  bool         m_bLastWhiteDIOK = false ; // measurement result from last image
  bool         m_bLastBlackDIOK = false ; // measurement result from last image
  bool         m_bLastBlackDOOK = false ; // measurement result from last image
  CmplxVector  m_WhiteIntCirclePts ;
  CmplxVector  m_BlackIntCirclePts ;
  CmplxVector  m_ExtCirclePts ;

  cmplx        m_cLastCenter_pix;
  cmplx        m_cLastIntCenter_pix;
  cmplx        m_cLastIntWhiteCenter_pix;
  cmplx        m_cLastIntBlackCenter_pix;
  cmplx        m_cLastExtCenter_pix;
  cmplx        m_cLastWhiteConturCenter_pix;
  CirclesVector   m_FoundCircles ;
  CirclesVector   m_FoundExtCircles ;
  CirclesVector   m_FoundIntCircles ;
  CenteringStatus m_Centering ;

  double       m_dLastSyncTime_ms ;
  double       m_dLastCommandTime ;
  int          m_iCurrentFrameNumberInTurn ; // 0 - turn begin, (N_MEASUREMENTS_PER_TURN - 1) - last frame in turn
  double       m_dLastCalculatedRotationPeriod_ms ;
  int          m_iTriggerDelay0 ;
  int          m_iTriggerDelay1 ;
  int          m_iCaptureCurrentPhase_us ;

  double       m_dAccumulatedX ;
  double       m_dAccumulatedY ;
  cmplx        m_cLastROICent_pix ;
  CPoint       m_LastROICent ;
  CRect        m_LastFrameRect = { 0 , 0 , 0 , 0 } ;

  cmplx        m_cLastInitialPosition ;
  cmplx        m_cLastXMovedPlusPosition ;
  cmplx        m_cLastXReturnedPosition ;
  cmplx        m_cLastYMovedPlusPosition ;
  cmplx        m_cLastYReturnedPosition ;
  cmplx        m_cXScalePlus_pix_per_sec ;
  cmplx        m_cXScaleMinus_pix_per_sec ;
  cmplx        m_cYScalePlus_pix_per_sec ;
  cmplx        m_cYScaleMinus_pix_per_sec ;
  // Part hole parameters for TVObjects
  int          m_iIDmin_pix = 35;
  int          m_iIDmax_pix = 47;
  int          m_iBlankIDmin_pix = 18;
  int          m_iBlankIDmax_pix = 55;

  double       m_dLastTriggerDelayTime_us ;
  cmplx        m_cLastSpotCent_0Deg ;
  cmplx        m_cLastSpotCent_90Deg ;
  cmplx        m_cLastSpotCent_180Deg ;
  cmplx        m_cLastSpotCent_270Deg ;
  double       m_dLastXEccentricitet_um;
  double       m_dLastYEccentricitet_um;
  cmplx        m_cAverageCenterFor0Deg;
  cmplx        m_cLastCenterMins;
  cmplx        m_cLastCenterMaxes;
  CmplxVector  m_cAccumulator;
  int          m_iAveragingC;
  double       m_dAllowedPtDeviation;
  int          m_iNFramesForThrowing;
  DoubleVector m_AllCaptureTimes;
  StringVector m_LastCameraCommands;
  int          m_iNOmittedFrames = 0 ;
  cmplx        m_cNormMainCrossCenter ; 
  cmplx        m_cMainCrossCenter ;
  cmplx        m_cSideUpperRightCorner ;
  cmplx        m_cSideLowerRightCorner ;
  cmplx        m_cMasterCenter_pix ;
  static double m_dDistFromLastVertEdgeToMasterVertEdge_um ;
  static double m_dFrontScaleCorrectionCoeff ;
  static double m_dFrontLensWorkingDistance_um ;
  double       m_dConesEdgePos_pix ;
  cmplx        m_cSideUpperLeftCorner ;
  cmplx        m_cSideLowerLeftCorner ;
  cmplx        m_cCalibSideUpperRightCorner ;
  cmplx        m_cCalibSideLowerRightCorner;
  cmplx        m_cCalibSideUpperLeftCorner ;
  cmplx        m_cCalibSideLowerLeftCorner ;

  // Parameters for stones adjustment
  double       m_dStoneToMasterDistance_um;
  cmplx        m_cLeftTopStoneTargetMark ;
  cmplx        m_cRightTopStoneTargetMark ;
  CLine2d      m_LastStoneTargetLine ;
  double       m_dLastStoneToTargetDist_um ;
  double       m_dLastStoneToPartSideDist_um ;
  double       m_dLastMinStoneToPartDist_um;
  int          m_iLastStoneMovingTime_ms;
  int          m_iGoodPositionCounter ;

  double       m_dMaxTipDeviationFromMaster_um = 50. ;
  double       m_dLastTipDeviationFromMaster_um = 0. ;

  double       m_dCoarseStoneTipDist_um ;
  double       m_dFineStoneToPartTargetPosition_um ;
  double       m_dLastFineSideGrindingStart ;  // Adjusted by vision
  double       m_dLastCoarseSideGrindingStop ; // result of tip position adjust (PLC defined)
  double       m_dLastCoarseSideAdjustedPos ;  // Adjusted by vision
  double       m_dLastFineSideAdjustedStop ;  // Result of stone position adjustment by vision
//   double       m_dLastFineSideGrindingStopByPLC ;  // Result of tip position (PLC) and stone position (vision)
//                                             // Should be the same for stone and tip adjustment

  double       m_dLastUpperConusAngle_deg ;
  double       m_dLastLowerConusAngle_deg ;

  cmplx        m_cStoneLeftPt ;
  cmplx        m_cStoneRightPt ;
  double       m_dLastStoneAngle_deg ;
  CLine2d      m_LastStoneEdgeLine ;
  CLine2d      m_LastPartUpperSideLine;

  bool         m_bStoneIsMeasured = false ;
  bool         m_bSideStoneInitialAdjustmentIsFinished = false;
  int          m_iCheckSideLight = 0 ; // counter for omitting several frames
                                       // when equal to 1, message to Engine will be sent
  int          m_iBlankLengthCheck = 0 ;
  FXString     m_AdditionalMsgForManual ;
  FXString     m_AdditionalInfo2;
  FXString     m_AdditionalInfo3;
  FXString     m_AdditionalInfo4;
  FXString     m_InfoAboutWhiteGrindingMeasurement;
  FXString     m_InfoAboutLastResults;

  int          m_iFrontForWhiteExposure_us ;
  int          m_iFrontForBlackExposure_us ;
  int          m_iSideExposure_us ;
  const CVideoFrame * m_pLastVideoFrame = NULL;

  int          m_iNProcessedParts ;
  int          m_iAfterCommandSaved ;
  int          m_iSaveDecimator ;
  int          m_iFrameCount ;
  int          m_iRightSideDist = 10 ;
  int          m_iLastIndexPlus = -1 ; // for right side 
  int          m_iLastIndexMinus = -1 ;
  double       m_dAngleThreshold = M_PI / 180. ;
  DWORD        m_dwLastFilledConturs ;
  double       m_dLastGrabOrderTime ;
  double       m_dLastMotionFinishedTime = 0. ;
  double       m_dLastFrontMeasurementTime = 0.;
  double       m_dLastFrontStoneMoveDist = 0.;
  bool         m_bFrontMotionOrdered = false ;
  double       m_dSideTipDiaBeforeFineGrinding_um ;
  double       m_dLastTipOnSideDia_um ;
  double       m_dLastTipOnSideMeasTime_ms ;
  double       m_dLastTipOnFrontDia_um ;

  bool         m_bWaitAnswerFromFRender = false;
  CRect        m_ContinueRect;
  CRect        m_FinishRect;

  CRect        m_SideROI ;
  CRect        m_ConeCircleROI ;
  CRect        m_HoleROI ;  // used for back light and for hole circle
  CRect        m_SideStoneROI ;

public:
  DECLARE_RUNTIME_GADGET( MPPT_Dispens );
  MPPT_Dispens() ;
  void ShutDown();

  void PropertiesRegistration();
  void ConnectorsRegistration();
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );
  
  bool AccumulateLastCmplxResult(cmplx cResult);
  bool CalculateAndSaveCenteringScales();
  BOOL CameraControl(LPCTSTR pCommand);
  bool CenteringProcedure() ;// command for centering is sent from control program
  CenteringStatus CenteringWithSync( const CDataFrame * pDataFrame , 
    CContainerFrame * pMarking ) ;
  int CheckAndSaveFinalImages( CContainerFrame * pOutInfo , 
    bool bBad , LPCTSTR pSuffix );
  int CheckAndSaveImage( const pTVFrame pImage , bool bFinal ) ;
  FXString CheckCreateCurrentLogs() ;
  FXString CheckCreateDataDir() ;
  FXString CheckCreatePartResultDir() ;
  int ClearInputQueue( bool bSingle = true , 
    bool bContainers = true , bool bVFrames = true );
  static void ConfigParamChange( LPCTSTR pName , 
    void* pObject , bool& bInvalidate , bool& bInitRescan ) ;
  void CopyDataToDialog( Dispenser& Src ) ;
  void CopyDataFromDialog( Dispenser& Dest ) ;
  int CreateQuestionsOnRender( cmplx& Pt,
    LPCTSTR pInfo, LPCTSTR * pQuestions , vector<CRect>& Zones ,
    CContainerFrame * pMarking);
  bool DeleteWatchDog() ;
  bool DecodeCalibrationPars( LPCTSTR AsString);
  void DoAdjustForCentering(CenterMoving Axe, double dMotionDist_pix);
  bool DoMove( MotionPhase Phase , double dDist , cmplx& cLastMotion );
  double DoMoveX( double dDeltaX ) ;
  double DoMoveY( double dDeltaY ) ;
  double DoMoveXY( cmplx& cVector ) ;
  int DrawStandardGraphics( CContainerFrame * pOutFrame ) ;
  bool ExtractCirclesForCalibration( const CVideoFrame * pVF , 
    CContainerFrame * pMarking );
  cmplx FilterAndGetMainPt();
  bool FilterCircle( CmplxVector& Pts , 
    cmplx& cCent , double& dDia , double& dMaxDia_pix , double& dMinDia_pix , 
    double dMaxDeviation_pix , CContainerFrame * pMarking = NULL ) ;
  bool FilterByCenterAndDia( CmplxVector& Pts ,
    cmplx cCent , double dDia , cmplx& cNewCent , double& dNewDia_pix ,
    double dMaxDeviation_pix , CContainerFrame * pMarking = NULL ) ;

  bool FindPointWithMinimalEccentricity(MotionPhase& Phase, 
    double& dEccentr, cmplx& cLastMotion);
  double MPPT_Dispens::GetCapturePeriod() ;
  void GetCenteringParameters() ;
  MPPT_Dispens * GetSCanOrigin() { return m_pMsgOrigin ; }
  int GetCircles( const CDataFrame* pDataFrame , CContainerFrame * pMarking ) ;
  int GetCircles( LPCTSTR pNamePrefix , CirclesVector& Results ,
    const CDataFrame* pDataFrame , CContainerFrame * pMarking ) ;
  double GetEccentricity( int iNSamples , double& dDia );
  double GetFrameTSFromEmbeddedInfo( const CVideoFrame * pVF );
  double GetLastWhiteAveragedResult( int iNAverage , double dHoleTargetSizeCorrection_um ) ;
    double GetLastWhiteMeasurementResult( FXString& FullResultAsText ,
    double dHoleTargetSizeCorrection_um);
  int GetLightMask() { return m_iCurrentLightMask ; }
  FXString GetMainDir();
  CColorSpot * GetSpotData( LPCTSTR pName ) ;
  LPCTSTR GetWorkingModeName();
  LPCTSTR GetWorkingStateName();
  void GrabImage() ;
  int GetVideoIntensityRange(const CVideoFrame * pVf,
    int& iMin, int& iMax, int iStep);
  int InitCentering();
  bool IsProcessingStage() ;
  bool IsSynchronized() { return m_bIsSynchronized;  }
  bool IsStoneSideMeasurement();
  int LightOff( int Mask ) ;
  int LightOn( int Mask ) ;
  void LoadAndUpdatePartParameters() ;
  void MoveFrontStone( double dDist , LPCTSTR pComments = NULL );
  static LRESULT MovementFinished() ;
  bool NotifyOtherGadgetsAboutPartChange() ;
  void MPPT_Dispens::OffsetCameraROI( int iXOffset , int iYOffset ) ;
  bool PreProcessVideoFrame( CContainerFrame * pMarking ,
    const CDataFrame * pDataFrame , const CVideoFrame * pVF ) ; // true - continue processing
  bool PreProcessCircles( CContainerFrame * pMarking , 
    const CDataFrame * pDataFrame , const CVideoFrame * pVF ) ; // true - there are some circles
  int ProcessAkoTekContour( const CVideoFrame * pVF , CContainerFrame * pOutFrame ) ;
  int ProcessSideContour( CContainerFrame * pOutFrame ) ;
  bool ProcessTextCommand( const CTextFrame * pCommand ) ;
  int ProcessImageForCentering( cmplx& TargetForCenter ,
    cmplx& cMeasuredCenter , LPCTSTR pOKMsg , LPCTSTR pCommandToEngine ,
    MPPD_Stage NextWorkingStateOnOK , CContainerFrame * pMarking ) ;
  int ProgramCamera( LPCTSTR pCameraParams , LPCTSTR pLabel = "CameraParamsSet" ) ;
  int ProgramImaging( LPCTSTR pCameraParams , LPCTSTR pLabel = "ImagingControl" ) ;
  int ResetIterations() ;
  int RestoreKnownParts( LPCTSTR pPartName = NULL , bool bTellToEngine = false );
  int SaveKnownParts() ;
  void SaveCSVLogMsg( LPCTSTR pFormat , ... ) ;
  void SaveGrindingLogMsg( LPCTSTR pFormat , ... ) ;
  int SaveImage( const pTVFrame pImage , LPCTSTR pFileName );
  void SaveLogMsg( LPCTSTR pFormat , ... ) ;
  void SaveOperativeLogMsg( LPCTSTR pFormat , ... ) ;
  int SavePartDataToRegistry( LPCTSTR pPartname , bool bTakeFromCurrent );
  void SaveScales();
  int ScanPartParameters( LPCTSTR pAsString , Dispenser& Part );
  bool ScanPropertiesBase( LPCTSTR text , bool& Invalidate ) ;
  int SelectCurrentPart( LPCTSTR pPartName );
  bool SendMessageToEngine(LPCTSTR pMessage, LPCTSTR pLabel);
  bool SendOKToEngine(LPCTSTR pNote = NULL);
  BOOL SetBackLight(bool bOn);
  BOOL SetBurstTriggerMode( double dFrameRate , int iNFrames = 4 ) ;
  void SetCameraTriggerParams( bool bTriggerOn = false , double dTriggerDelay_us = 0. ) ;
  WhatDispenserFigureFound SetDataAboutContour( const CFigureFrame * pFrame ) ;
  int SetDataAboutContours(CFramesIterator * pFiguresIterator);
  int SetDataAboutROIs( const CDataFrame * pDataFrame );
  int SetExposure( int iExp ) ;
  BOOL SetExposureAndGain(double dExposure_us, double dGain);
  BOOL SetFrontLight(bool bOn , bool bHighBrightness = false );
  void SetGrabForCentering( MPPD_Stage Stage ) ;
  MPPD_Stage SetIdleStage() { 
    MPPD_Stage RetVal = m_WorkingStage; 
    m_WorkingStage = STG_Idle;
    return RetVal;
  }
  void SetImagingParameters( bool bByPart , CRect * pROI = NULL );
  void SetNoTriggerMode();
  bool SetObjectPlacement(LPCTSTR pVideoObjectName , CRect& ROIPosition);
  bool SetObjectPlacementAndSize( LPCTSTR pVideoObjectName ,
    CSize& ObjSizeMin , CSize& ObjSizeMax ,
    CRect * pROIPosition = NULL );
  bool SetObjectSize( LPCTSTR pVideoObjectName ,
    CSize& ObjSizeMin , CSize& ObjSizeMax ,
    CSize& AreaMinMax_pix );
  bool SetParametersToTVObject( LPCTSTR pParams ) ;
  BOOL SetROIForCamera( CRect ROI );
  bool SetRotationPeriodMeasurement() ;
  BOOL SetSimpleTriggerMode() ;
  int SetWatchDog( int iTime_ms ) ;
  void StartMotor() ;
  void StopMotor() ;
  // Adjust object parameters in imaging
};


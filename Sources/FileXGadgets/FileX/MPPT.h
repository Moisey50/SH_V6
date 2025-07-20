#pragma once
#include "helpers\UserBaseGadget.h"
#include <imageproc\statistics.h>
#include "helpers/FramesHelper.h"
#include <math/PlaneGeometry.h>
#include <math/FRegression.h>
#include <math/FStraightLineRegression.h>
#include "math/FigureProcessing.h"
#include <math/FRegression.h>
#include <gadgets\quantityframe.h>
#include <imageproc\seekspots.h>
#include <imageproc/vframeembedinfo.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>      // std::stringstream


#define PIN_HORIZ_SIDE_ROI_WIDTH   60
#define PIN_HORIZ_SIDE_ROI_HEIGHT 200

#define SHIFT_LIMIT (1638.)
//#define SHIFT_LIMIT_Z (SHIFT_LIMIT/2.)
#define SHIFT_LIMIT_Z (SHIFT_LIMIT)

enum IndexesInCentersAndCornersArray
{
  ICAC_Center = 0 ,
  ICAC_LeftSideCent ,
  ICAC_TopSideCent ,
  ICAC_RightSideCent ,
  ICAC_BottomSideCent ,
  ICAC_LTCorner ,
  ICAC_RTCorner ,
  ICAC_RBCorner ,
  ICAC_LBCorner
};

enum MPPT_WorkingMode
{
  MPPTWM_NotSet = 0 ,
  MPPTWM_Down ,
  MPPTWM_UpFront ,
  MPPTWM_UpSide ,
  MPPTWM_FinalSide ,
  MPPTWM_FinalFront ,
  MPPTWM_Common ,
  MPPTWM_ShortCommand = 100
} ;

enum WhatMPPTFigureFound
{
  WFFMPPT_NotFound = 0 ,
  WFFMPPT_CavityBottom = 1 ,
  WFFMPPT_CavityPlaneCenter = WFFMPPT_CavityBottom << 1 ,
  WFFMPPT_LaserSpot = WFFMPPT_CavityPlaneCenter << 1 ,

  WFFMPPT_LaserSpotCenterCalib = WFFMPPT_LaserSpot << 1 ,

  WFFMPPT_CalibXYOnPin = WFFMPPT_LaserSpotCenterCalib << 1 ,
  WFFMPPT_CalibXPinUpperEdge = WFFMPPT_CalibXYOnPin << 1 ,
  WFFMPPT_CalibXPinRightEdge = WFFMPPT_CalibXPinUpperEdge << 1 ,
  WFFMPPT_CalibXPinDownEdge = WFFMPPT_CalibXPinRightEdge << 1 ,
  WFFMPPT_CalibXPinLeftEdge = WFFMPPT_CalibXPinDownEdge << 1 ,

  WFFMPPT_CalibXYOnLower = WFFMPPT_CalibXPinLeftEdge << 1 ,
  WFFMPPT_CalibZOnLower = WFFMPPT_CalibXYOnLower << 1 ,

  WFFMPPT_PartFrontOnLower = WFFMPPT_CalibZOnLower << 1 ,
  WFFMPPT_PartSideOnLower = WFFMPPT_PartFrontOnLower << 1 ,
  WFFMPPT_PartSideConturOnLower = WFFMPPT_PartSideOnLower << 1 ,

  WFFMPPT_PartSideOnFinal = WFFMPPT_PartSideConturOnLower << 1 ,
  WFFMPPT_PartFrontOnFinal = WFFMPPT_PartSideOnFinal << 1 ,

  WFFMPPT_PartFrontOnFinalCalb = WFFMPPT_PartSideOnFinal << 1 ,
  WFFMPPT_PartSideOnFinalCalib = WFFMPPT_PartFrontOnFinalCalb << 1 ,

  WFFMPPT_Conturs = WFFMPPT_CavityBottom | WFFMPPT_LaserSpot
  | WFFMPPT_PartFrontOnLower | WFFMPPT_PartSideOnFinal
  | WFFMPPT_CalibXYOnPin | WFFMPPT_CalibXYOnLower | WFFMPPT_PartSideConturOnLower
  | (WFFMPPT_PartSideOnFinalCalib << 1) ,

  WFFMPPT_Centers = WFFMPPT_CavityPlaneCenter 
  | WFFMPPT_CalibXYOnPin | WFFMPPT_CalibXPinUpperEdge
  | WFFMPPT_CalibXPinRightEdge | WFFMPPT_CalibXPinDownEdge | WFFMPPT_PartSideOnLower
  | WFFMPPT_CalibXPinLeftEdge | (WFFMPPT_PartSideOnFinalCalib << 2) 
};

enum MPPT_State
{
  State_Unknown            = 0 ,
  State_Idle               = State_Unknown,
  State_ShortCommand       = 1 ,
  State_Finished               ,
  State_Stopped                ,
  DL_LiveVideoPinFocusing = 50 ,
  DL_LiveVideoPinMeasure       ,
  DL_LiveVideoPinNorth ,
  DL_LiveVideoPinEast ,
  DL_LiveVideoPinSouth ,
  DL_LiveVideoPinWest ,
  DL_LiveVideoCavity       = 60 ,
  DL_LiveVideoCavityFocus       ,
  DL_LiveApexZView              ,
  DL_LiveVideoLaser        = 70 ,
  DL_ScaleCalib           = 100 ,
  DL_LaserCalib           = 150 ,
  DL_LaserCalibFinished   = 160 ,
  DL_PinToCenterForZ      = 170 ,
  DL_PinZMeasureByDefocus       ,
  DL_PinZDefocusToPlus          ,
  DL_PinZDefocusToMinus         ,
  DL_FinalImageOverPin          ,
  DL_CorrectAfterFinalVision    ,
  DL_PinNorthSide = 200         ,
  DL_PinEastSide                ,
  DL_PinSouthSide               ,
  DL_PinWestSide                ,
  DL_MeasCavity           = 300 ,
  DL_MeasCavityXY               ,
  DL_CavityJumpForZ             ,
  DL_MeasCavityZ                ,
  DL_MeasCavityZMultiple        ,
  DL_MoveToCorrectZ             ,
  DL_CaptureZWithCorrectHeigth  ,
  DL_MoveCavityForFinalImage    ,
  DL_SendCavityResult           ,
  DL_CaptureCavityFinalImage    ,
  DL_MeasZByDefocus       = 400 ,
  DL_MeasZAfterExpAdjust        ,
  DL_DefocusToPlus              , 
  DL_DefocusToMinus             ,
  DL_BadCavityOnDefocusing      ,
  DL_LongSweep                  ,
  DL_ShortSweep                 ,

  ULS_Unknown          = 1000 ,
  ULS_LiveVideo        = 1050 ,
  ULS_ScaleCalib       = 1100 ,
  ULS_MoveAfterCalib          ,
  ULS_FirstZForBlank   = 1150 ,     
  ULS_ZCorrection ,
  ULS_ZCorrection2 ,
  ULS_ZCorrNoZCalib ,
  ULS_ZCorrectionForMeas ,
  ULS_ZCorrectionForMeas2 ,
  ULS_ZMeasurement     = 1200 ,
  ULS_GrabFinal ,

  ULF_Unknown         = 2000 ,
  ULF_LiveVideo       = 2050 ,
  ULF_MoveForScaleCalib = 2100,
  ULF_ScaleCalib             ,
  ULF_MeasureAfterCalib      ,
  ULF_MoveAfterCalib         ,
  ULF_WaitForExternalReady   ,
  ULF_Measurement     = 2200 ,
  ULF_RightSideCorrection    ,
  ULF_AfterSideCorrection    ,
  ULF_MeasureAndCorrect      ,
  ULF_MoveAfterMeasurement   ,

  State_AddMotion = 10000

};

enum SaveMode
{
  SaveMode_No = 0 ,
  Save_Final ,
  Save_Bad ,
  SaveMode_All ,
  SaveMode_OnePerSerie ,
  SaveMode_Decimate
};

enum LaserExpMode
{
  LEM_Unknown = 0 ,
  LEM_ForPin ,
  LEM_ForCavity 
};
enum EDGE_DIR
{
  ED_UNKNOWN = 0 ,
  ED_UP ,
  ED_RIGHT ,
  ED_DOWN ,
  ED_LEFT ,
  ED_ALL ,
  ED_DEFOCUS
};

enum CAVITY_EDGE
{
  CavEdge_Unknown = 0 ,
  CavEdge_Lower_Xc ,
  CavEdge_Upper_Xc ,
  CavEdge_Lower ,
  CavEdge_Upper ,
  CavEdge_LowerAndUpper ,
  CavEdge_Left ,
  CavEdge_Right
};

enum DL_ZMeasurementMethod
{
  DLZ_Unknown = 0 ,
  DLZ_Laser ,
  DLZ_Defocusing ,
  DLZ_LongSweep ,
  DLZ_ShortSweep ,
  DLZ_NoZMeasurement
};

enum DL_WhatSideToUseForZ
{
  DLWSZ_Unknown = 0 ,
  DLWSZ_Left = 1 ,
  DLWSZ_Right = 2 ,
  DLWSZ_Both = 3
};

enum MEAS_OBJECT
{
  MO_Unknown = 0 ,
  MO_Pin ,
  MO_Cavity ,
  MO_Blank_Front ,
  MO_Blank_Side
};

class Named_ifstream : public std::ifstream
{
public:
  string m_Filename ;
};

class Named_ofstream : public std::ofstream
{
public:
  string m_Filename ;
};

class HeightMeasResult
{
public:
  double m_dHeight ;
  cmplx  m_cFOV ;

  HeightMeasResult( double dHeight = 0. , cmplx& cFOV = cmplx() )
  {
    m_dHeight = dHeight ;
    m_cFOV = cFOV ;
  }
};

typedef vector< CoordsCorresp> CoordPairs ;

class CavityParams
{
public:
  CmplxVector m_Extremes ;
  string      m_Name ;
  string      m_Description ;
  bool     m_bIsInitialized ;
  int      m_iIndex ;
  double   m_dPlaneHeight_um ;
  double   m_dPlaneWidth_um ;
  double   m_dPlaneArea_um2 ;
  double   m_dDistBetweenAreas_um ;
  int      m_iCentralZoneWidth_pix ;
  int      m_iCavityExp_us ;
  int      m_iLaserExp_us ;
  int      m_iDefocusExp_us ;
  int      m_iRingDefocusExp_us ;
  int      m_iStraightDefocusExp_us ;
  double   m_dTargetForFocusExpAdjust ; // norm brightness for defocusing
  BOOL     m_bCavFocusRingLightOn ;
  BOOL     m_bCavFocusStraightLightOn ;
  BOOL     m_bCavXYRingLightOn ;
  BOOL     m_bCavXYStraightLightOn ;
  BOOL     m_bFindBlackCorners ;
  double   m_dDefocusingThreshold;
  double   m_dDefocusingLongStep;
  double   m_dDefocusingShortStep;
  double   m_dNormBrightnessForCavity ;
  double   m_dCavitySizeTolerance_perc ;
  double   m_dYCorrectionWidth_um ;     // == 0 - we do measure extreme point
                                        // We are looking for level, where width will be more
                                        // than this parameter
  double   m_dRelativeHeightForXSampling ; // 0. - half real height, +0.5 - upper edge, -0.5 - lower edge
  cmplx    m_cCenterAsSpot ;
  cmplx    m_cSizeAsFigure ;
  cmplx    m_cCenterAsFigure ;
  CAVITY_EDGE m_CavityEdge;

  // Filters for good/bad cavity selection 
  BOOL        m_bAngleFiltrationOn ; // enables additional internal sides filtering 
                                     //for too large deviation from straight line
  BOOL        m_bCavCheckYdiffer ;   // Y extremes should be approximately the same
  double      m_dAllowedYdiffer_um ; // Max diff for Y extremes on left and right sides
  BOOL        m_bCavEnableProfileUsing ;
  BOOL        m_bCavCompareContAndProfResults ;
  double      m_dMaxDiffBetweenXsOnConturAndProfile_um ;
  double      m_dMaxAngleDiffBetweenInternalEdges_deg;



  CavityParams()
  {
    m_bIsInitialized = false ;
    m_iIndex = -1 ;
    m_dPlaneArea_um2 = m_dPlaneHeight_um = m_dPlaneWidth_um = 0 ;
    m_CavityEdge = CavEdge_Upper_Xc ;
    m_dCavitySizeTolerance_perc = 10. ;
    m_iCavityExp_us = m_iDefocusExp_us = m_iLaserExp_us =
      m_iRingDefocusExp_us = m_iStraightDefocusExp_us = 200 ;
    m_bCavFocusStraightLightOn = m_bCavXYStraightLightOn = TRUE ;
    m_bCavFocusRingLightOn = m_bCavXYRingLightOn = FALSE ;
    m_dDefocusingThreshold = 0.7 ;
    m_dDefocusingLongStep = 50. ;
    m_dDefocusingShortStep = 10. ;
    m_dTargetForFocusExpAdjust = 0.4 ;
    m_dNormBrightnessForCavity = 0.96 ;
    m_iCentralZoneWidth_pix = 300 ;
    m_dYCorrectionWidth_um = 0. ;
    m_dRelativeHeightForXSampling = 0. ;
    m_bFindBlackCorners = FALSE ;
    m_dDistBetweenAreas_um = 400. ;

    // Filters for good/bad cavity selection
    m_bAngleFiltrationOn = FALSE ; // exists in gadget properties
    m_bCavCheckYdiffer = FALSE  ;
    m_dAllowedYdiffer_um = 10. ;
    m_bCavEnableProfileUsing = FALSE ;
    m_bCavCompareContAndProfResults = FALSE ;
    m_dMaxDiffBetweenXsOnConturAndProfile_um = 1.5;
    m_dMaxAngleDiffBetweenInternalEdges_deg = 1.5;
  }
  CavityParams( LPCTSTR pSrcString )
  {
    FromString( pSrcString ) ;
  }
  FXString ToString();
  bool FromString( LPCTSTR AsString ) ;
  int RestoreCavityDataFromRegistry(
    LPCTSTR pPartFolder , bool bSetDefault = false );
  void SaveCavityDataToRegistry( LPCTSTR pPartsFolder , LPCTSTR PartName );

  bool IsInitialized()
  {
    return m_bIsInitialized ;
  }
  CavityParams& operator =( const CavityParams& Orig )
  {
    memcpy( &m_bIsInitialized , &Orig.m_bIsInitialized , 
      sizeof(*this) - sizeof(m_Extremes) - sizeof(m_Name) - sizeof(m_Description) ) ;
    m_Extremes = Orig.m_Extremes ;
    m_Name = Orig.m_Name ;
    m_Description = Orig.m_Description ;
    return *this ;
  }
};

class BlankParams
{
public:
  string      m_Description ;
  BOOL     m_bIsGauge ;
  int      m_iBlankExp_us ;
  double   m_dBlankWidth_um ;  // X dimension on screen
  double   m_dBlankHeight_um ; // Y dimension on screen
  double   m_dSizeTolerance_perc ;
  double   m_dParallelismTol_deg ;
  BOOL     m_bBlankXYRingLightOn;     // if any of these two bits on
  BOOL     m_bBlankXYStraightLightOn; // the constant light is in using for XY measurements
  SQ_EDGE_AND_CORNERS  m_UsedBlankEdge;
  double   m_dYShiftForBlank_um ;
  double   m_dBasePos_um;

  BlankParams(double dBlankWidth_um = 500. , double dBlankHeight_um = 400. ,
    int iBlankExp_us = 1400 )
  {
    m_iBlankExp_us = iBlankExp_us ;
    m_dBlankWidth_um = dBlankWidth_um ;
    m_dBlankHeight_um = dBlankHeight_um ;
    m_dSizeTolerance_perc = 3.0 ;
    m_UsedBlankEdge = SQREdge_Lower;
    m_dBasePos_um = m_dYShiftForBlank_um = 0.;
    m_bIsGauge = false ;
    m_bBlankXYRingLightOn = FALSE ;
    m_bBlankXYStraightLightOn = TRUE ;
  }
  BlankParams& operator =( const BlankParams& Orig )
  {
    memcpy( &m_bIsGauge , &Orig.m_bIsGauge ,
      sizeof( *this ) - sizeof( m_Description ) ) ;
    m_Description = Orig.m_Description ;
    return *this ;
  }
  FXString ToString();
  bool FromString( LPCTSTR AsString ) ;
  int RestoreBlankDataFromRegistry(
    LPCTSTR pPartFolder , bool bGauge , bool bSetDefault = false );
};

class PartParams
{
public:
  string m_Name ;
  string m_Description ;
  CavityParams m_Cavity ;
  BlankParams  m_Blank ;
  BlankParams  m_Gauge ;

  PartParams( LPCTSTR pPartName = "Unknown" , LPCTSTR pRegName = NULL , bool bSetDefault = false )
  {
    m_Gauge.m_bIsGauge = true ;
    if ( pPartName )
    {
      if ( pRegName == NULL )
        pRegName = "TheFileX\\Micropoint\\PartsData" ;
      RestorePartDataFromRegistry( pRegName , pPartName , bSetDefault ) ;
    }
  }
  PartParams& operator=( PartParams& Other )
  {
    m_Cavity = Other.m_Cavity ;
    m_Blank = Other.m_Blank ;
    m_Gauge = Other.m_Gauge ;
    m_Name = Other.m_Name ;
    m_Description = Other.m_Description ;
    return *this ;
  }
  FXString ToString();
  bool FromString( LPCTSTR AsString ) ;
  int RestorePartDataFromRegistry(
    LPCTSTR pFolderForParts , LPCTSTR pPartName , bool bSetDefault = false );
};


typedef vector<HeightMeasResult> HeightCalib ;
typedef vector<CavityParams> DataForConturs ;
typedef vector<PartParams> KnownParts ;

class AverageData
{
public:
  double m_dLeftFocus;
  double m_dRightFocus;
  double m_dLeftAverage;
  double m_dRightAverage;
  double m_dMinLeft;
  double m_dMaxLeft;
  double m_dMinRight;
  double m_dMaxRight;
  cmplx  m_cXYPos;
  int    m_iNSamples;
  AverageData() { Reset(); }
  void Reset() { memset(this, 0, sizeof(*this)); };

  FXString ToString()
  {
    FXString Output;
    if ( m_iNSamples )
    {
      double dMult = 1. / m_iNSamples;
      Output.Format("%.2f,%.2f,"
        "%.2f,%.2f,%.2f,"
        "%.1f,%.1f,%.1f,"
        "%.1f,%.1f,%.1f,%.1f,",
        m_cXYPos.real() * dMult , m_cXYPos.imag() * dMult ,
        dMult * (m_dLeftFocus + m_dRightFocus),
        dMult * m_dLeftFocus, dMult *  m_dRightFocus,
        dMult * 0.5 * (m_dLeftAverage + m_dRightAverage), 
        dMult * m_dLeftAverage, dMult * m_dRightAverage,
        dMult * m_dMinLeft, dMult * m_dMaxLeft, 
        dMult * m_dMinRight, dMult * m_dMaxRight);
    }
    return Output;
  }
};

class MPPT :
  public UserBaseGadget
{
public:
  MPPT_WorkingMode m_WorkingMode ;
  MPPT_WorkingMode m_OldWorkingMode ;
  MPPT_State       m_WorkingState ;

  FXString m_PartName ;
  FXString m_SelectedPartName ;
  TCHAR    m_PartListForStdDialog[2000] ;
  int      m_iSelectedPart ;
  long   m_ViewDetails ;
  int    m_SaveFinalImageWithGraphics ;
  int    m_iCentralZoneWidthForX ;
  double m_dScale_um_per_pix ;
  cmplx  m_cScale_um_pix ;
  double m_dMachineToCameraAngle ;

  int    m_iLaserExposure ;
  int    m_iLaserExposureOnPin ;
  int    m_iLaserExposureOnCavity ;
  int    m_iCavityExposure ;
  int    m_iNewCavityExposure ;
  int    m_iCavityFocusExposure ;     
  int    m_iNewFocusExposure ;  // For focus exposure changes (cavity, pin, blank)
  int    m_iPinExposure ;       // For pin measurement
  int    m_iPinDefocusExposure; // For pin defocus
  int    m_iFrontExposure_us ;
  int    m_iSideExposure_us ;
  double m_dTargetForFocusExpAdjust;
  int    m_iLastSettledExposure;
  bool   m_bWasGoodFrame;
  double m_dLastExposureSetTime;
  BOOL   m_bDisableLightSwitchOff;

  int    m_iFocusingRadius ;
  int    m_iFocusingY;
  double m_dNormBirghtnessForCavity;
  double m_dWhiteThreshold;
  BOOL   m_bMeasureZ ;
  BOOL   m_bNoXYMeasurement ;
  int    m_iDistToFocusingArea ;
  bool   m_bMeasureFocus;
  int    m_iPassCount;
  int    m_iPassNumber;
  double m_dLineFilterThres_pix ;
  double m_dMaxDistCut_pix ;
  double m_dPinDiam_um ;
  CSize  m_CalibMatrixSize ;
  CSize  m_NewCalibMatrixSize ;
  BOOL   m_bDoPinXYCalibration ;
  FXString m_MatrixParamAsString ;
  double   m_dCalibStep ;
  int    m_iAverager ;
  int    m_iAverageCnt ;
  int    m_bUpdatePartParameters;
  int    m_iMinSideSize_pix;


  CAVITY_EDGE m_CavityEdge ;
  CmplxVector m_LastCavityCenters ;
  cmplx       m_cLastLeftCavityCenter_pix ;
  cmplx       m_cLastRightCavityCenter_pix ;
  cmplx       m_cLastLeftCavitySize_pix ;
  cmplx       m_cLastRightCavitySize_pix ;
  cmplx       m_cLastLeftCavityCenter_um ;
  cmplx       m_cLastRightCavityCenter_um ;
  cmplx       m_cLastLeftCavitySize_um ;
  cmplx       m_cLastRightCavitySize_um ;
  double      m_dLastCavityArea_um2 ;
  double      m_dYCenterRelativelyToLowerEdge ;
  double      m_dBlankWidth_um ;
  double      m_dBlankHeight_um ;
  SQ_EDGE_AND_CORNERS  m_UsedBlankEdge ;
  CmplxVector m_BlankCentersAndCorners ;
  cmplx       m_LastBlankSize ;

  // Filters for good/bad cavity selection moved to CavityParams

  WhatMPPTFigureFound m_WhatToDo ;
  FXString m_GadgetName ;
  FXString m_CommandTimeStamp ;
  FXString m_DoneTimeStamp ;
  FXString m_CurrentDataDir ;
  FXString m_CurrentImagesDir ;
  FXString m_CurrentReportsDir ;
  FXString m_CurrentLogFilePath;
  FXString m_CurrentOperativeLogFilePath;
  FXString m_CurrentCSVLogFilePath ;
  FXString m_CurrentFocusLogFilePath;
  FXString m_FocusLogAccumulator;
  CCoor3t  m_ShiftsDL ; // shifts for down looking 
  static CCoor3t m_ShiftsUL ; // shifts for up looking
  int      m_iCalibCntr ;
  int      m_iFrameCount ;
  int      m_iCntForSave ;
  int      m_iNRestSamples ;
  int      m_iNAttempts ;
  int      m_iNMaxAttempts ;

  SaveMode m_SaveMode ;
  int      m_iSaveDecimator ;
  double   m_dLastImageSavingTime  ;
  int      m_iAfterCommandSaved ;
  int      m_iNProcessedCavities ;
  bool     m_bStabilizationDone ;
  static int m_iNProcessedBlanks ;


  cmplx    m_cPinCenter ;
  double   m_dAllowedAngleErrorBetweenVertLines ;
  int      m_iNAllowedErrors ;

  CRect    m_LastROI ;
  CPoint   m_LastROICenter ;
  cmplx    m_cLastROICenter ;
  cmplx    m_cLastMeasCenter ;

  cmplx       m_cLastExtractedResult_pix ;
  cmplx       m_cLastCavityXYResult_um ;
  cmplx       m_cLastCavityXYAveraged_um ;
  
  cmplx       m_cLastCent_um;
  cmplx       m_cLastUpper_um;
  cmplx       m_cLastLower_um;
  cmplx       m_cLastSelected;
  cmplx       m_cLastCentAver_um;
  cmplx       m_cLastUpperAver_um;
  cmplx       m_cLastLowerAver_um;
  cmplx       m_cLastSelectedAver_um;

  double      m_dWidth_Tolerance ;
  double      m_dHeight_Tolerance ;
  double      m_dArea_Tolerance ;

  bool        m_bLastCavityResult ;
  CmplxVector m_IterationResults ;
  CmplxVector m_IterResultLeft ;
  CmplxVector m_IterResultRight ;
  CmplxVector m_LastLaserSpotsPositions ;

  CmplxVector m_PartMeasResult ;
  CmplxVector m_PartMeasSizes ;
  const CFigureFrame * m_pSideBlankContur ;
  CmplxVector m_RightCornerOnSide ;
  CmplxVector m_LeftCornerOnSide ;

  
  CoordPairs  m_CalibData ;
  CoordPairs  m_NewCalibData ;
  CoordsCorresp m_NorthPinSide ;
  CoordsCorresp m_EastPinSide ;
  CoordsCorresp m_SouthPinSide ;
  CoordsCorresp m_WestPinSide ;
  cmplx       m_cFinalTargetOverPin ;
  cmplx       m_CalculatedPinPosition ;
  bool        m_bRestoreScales ;
  cmplx       m_cLastCalibTargetXYAveraged_pix ;

  DL_ZMeasurementMethod m_ZMethod ;
    // For Z measurement by defocusing
  int         m_iNZMeasured ;
  int         m_iNZShouldBeMeasuredWithHighResolution ;
  int         m_iZMeasurementPeriod; //==0 - only first
  CRect       m_LeftFocusMeasRect ;
  CRect       m_RightFocusMeasRect ;
  double      m_dZStepForHighResolutionDefocusing_um ;
  double      m_dZShiftMax_um ;
  double      m_dZShiftMin_um ; 
  int         m_iNZFullPlusValues;
  int         m_iNZLeftPlusValues;
  int         m_iNZRightPlusValues;
  double      m_dDLZLongRange_um ;
  int         m_iDLZSweepCount ;
  int         m_iDLZNSweepSteps ;

  double      m_dForFastZBigStep_um ;
  double      m_dForFastZSmallStep_um ;
  double      m_dExtdZ;
    // there is place for last Z position in microns (relatively to target point)
    // This position will be updated after every Z measurement
  static double m_dZAfterShift_um;    // static for passing from ULS to ULF
  static double m_dZStdAfterShift_um; // static for passing from ULS to ULF

  double      m_dLastFocusIndicator;
  double      m_dLastLeftFocusIndicator;
  double      m_dLastRightFocusIndicator;
  double      m_dAverageFocusIndicator;
  double      m_dLeftAverageFocusIndicator;
  double      m_dRightAverageFocusIndicator;
  double      m_dMaxFocusValue;
  double      m_dMaxLeftFocusValue;
  double      m_dMaxRightFocusValue;
  double      m_dDefocusThreshold ;// 0.5 < Thres < 1.
  int         m_iFocusLogPeriod_samples;
  int         m_iSamplesCnt;
  BOOL        m_bXYandFocusLog;

  // for log averaging


  HeightCalib m_ZCalibData ;
  HeightCalib m_ZNewCalibData ;
  double      m_dZCalibRange ;
  double      m_dZCalibStep ;
  double      m_dTargetZ_pix;
  double      m_dZSensitivity_pix_per_um ;
  double      m_dZScale_um_per_pix;
  cmplx       m_cNominalZZero ;
  int         m_iNCavityZMeasurements ; // Number of measurements by laser for averaging
  cmplx       m_cCavityZMeasurementStep_um ; // XY step For measurement by laser
  cmplx       m_cCavityZMeasurementShift_um ; // Current XY shift for returning to center
  double      m_dMaxDeltaZ_um ;
  int         m_iZeroIndex ;
  cmplx       m_cLastLaserXYAveraged_pix ;
  vector<CCoor3t> m_ZMeasurements ; // X and Y - shifts from center in FOV, 
                                    // Z - measured dZ, T - spot area
  double      m_dAverage_dZ;
  double      m_dAverageLeft_dZ;
  double      m_dAverageRight_dZ;
  double      m_dZUsedAsResult ;
  DL_WhatSideToUseForZ  m_WhatSideToUseForCavity;
  DL_WhatSideToUseForZ  m_WhatSideToUseForPin;
  DL_WhatSideToUseForZ  m_WhatSideToUse;
  cmplx       m_cNormZMeasArea;
  bool        m_bPinOnlyCalib ;
  bool        m_bAdditionalShift ;

  int         m_iSaveBlankPreliminaryData; // 1 - image in first position, 2 - data about first position to log
                                            // 3 - both
  int         m_iSaveCavityPreliminaryData; // 1 - image in first position, 2 - data about first position to log
                                            // 3 - both
  cmplx       m_cLastBlankAvPos_pix ;
  cmplx       m_cLastBlankStd_pix ;
  cmplx       m_cLastBlankAvPos_um ;
  cmplx       m_cLastBlankStd_um ;

  MPPT_State  m_PreviousState ;
  CCoord3     m_TheRestShift ;
  CCoord3     m_EffectiveShift ; // transfer to the machine accounted

  cmplx       m_cLastPartMeasuredPt ;
  cmplx       m_cPartPatternPt ;
  
  FramesCollection m_CavitiesConturs ;
  FramesCollection m_LaserConturs ;
  FramesCollection m_CavitiesCenters ;
  FramesCollection m_PartConturs ;
  CmplxArray       m_ConturExtremes ;
  cmplx            m_cLastConturSize ;
  cmplx            m_cLastConturCenter;

  KnownParts       m_KnownParts ;
  PartParams       m_CurrentPart;
  DataForConturs   m_DataForConturs ;

  DWORD            m_dwLastFilledConturs ;
  DWORD            m_dwFilledContours ;
  const CVideoFrame * m_pCurrentImage ;

  int           m_iDistToCenterForVertLines;
  int           m_iPinHorizBaseLine ;
  int           m_iDLCavityHorizBaseLine ;
  double        m_dDLCavityHorizBaseLine ;
  int           m_iULSPartHorizBaseLine ;
  double        m_dLastGrabOrderTime ;
  double        m_dLastPlotTime;
  double        m_dXY_PlotPeriod;
  double        m_dZ_PlotPeriod;
  double        m_dGoodStdThres_pix ;
  DWORD         m_PlotSampleCntr ;
  cmplx         m_cPrevMeasuredXY;
  double        m_dPrevMeasuredZ;
  BOOL          m_bWaitForContinueCommand; // stay in measurement loop while
                                           // "continue_process" command is not received
  int           m_iNAllowedBadMeasurementsOnStabilization;
  HANDLE              m_hWatchDogTimer; //For lost images detection
  FXLockObject        m_LockWatchdog;

  int           m_iWaitCounter ;
  FXString      m_Info1 , m_Info2 , m_Info3 ;
  FXString      m_PinAverageInfo;
  cmplx         m_AverageViewPt ;
  FXString      m_AverageViewText ;
  vector<int>   m_Exposures;
  vector<double> m_Averages;
  vector<int>    m_Threshs;
public:
  MPPT();

  void ShutDown();
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );
  int FindXCenter() ;

  void PropertiesRegistration();
  void ConnectorsRegistration();


  CDataFrame * ProcessCavity( const CDataFrame * pDataFrame );
  CDataFrame * ProcessUplook( const CDataFrame * pDataFrame );
  CDataFrame * ProcessUplookSide( const CDataFrame * pDataFrame );

  cmplx FindExtremePoint( const CDataFrame * pDataFrame ,
    LPCTSTR pFigNamePart , Edge WhatExtreme ) ;
  bool CalcAndAddFocusIndication(CContainerFrame * pOutputFrame,
    CRect * pROI = NULL, CRect * pROI2 = NULL);
  bool CalcPinCenter();
  cmplx CalculateScaling( FXString * pStatistics = NULL ); // Function returns average pixel-to-microns scale (with angle)
  cmplx ConvertCoordsRelativeToCenter( cmplx& cRelToCenter );

  // returns number of parts
  DECLARE_RUNTIME_GADGET( MPPT );
  // Select laser spot from measured spots
  int AddFocusResults( bool bToPlus = true ); // returns true when necessary to continue defocusing
  int AdjustExposureForCavity( const CVideoFrame * pVF , 
    CContainerFrame * pOutFrame , double dTarget = 0.9 );
  int AdjustExposureForFocus( const CVideoFrame * pVF , 
    CContainerFrame * pOutFrame, double dTarget = 0.7);
  int AccumulateDataAboutBlank( CContainerFrame * pOutFrame );
  DL_ZMeasurementMethod AnalyzeAndGetDLMeasurementZMethod();
  int CalcFinalZPosition( CContainerFrame * pOutFrame );
  cmplx CalcShiftForFinalPinPosition();
  double CalculateDeltaZByDefocus();
  double CalculateDeltaZByDefocus( CmplxVector& Data , bool bSendMessage = true ,
    FXString * pDiagnostics = NULL );
  CDataFrame *  CavityYCorrection( const CFigureFrame * pContur , FXIntArray& ExtremeIndexes ,
    double dAccumulatedAreaThres , double& dUpCorrection_pix , double& dDownCorrection_pix );
  int CheckAndAddCavity();
  int MPPT::CheckAndAddPart( LPCTSTR pPartName , PartParams& PartData ) ;
  // Check save  mode and save if necessary
  int CheckAndSaveFinalImages( CContainerFrame * pOutInfo , 
    bool bBad = false , LPCTSTR pSuffix = NULL );
  int CheckAndSaveImage( const pTVFrame pImage , bool bFinal = false );
  int CheckBlankSize( CmplxVector& CentersAndCorners ,
    CContainerFrame * pOutInfo , FXString& ErrorMsg );
  int CheckBlankSizesAndReport( CContainerFrame * pOutInfo );
  FXString CheckCreateCurrentLogs() ;
  FXString CheckCreateDataDir() ;
  FXString CheckCreatePartResultDir() ;
  void ClearConturData();
  int ClearInputQueue( bool bSingle = true , bool bContainers = true , bool bVFrames = true );
  //int ClearInputQueue(  );
  static void ConfigParamChange( LPCTSTR pName , void* pObject , bool& bInvalidate , bool& bInitRescan ) ;
  bool DeleteWatchDog();
  int DrawStandardGraphics( CContainerFrame * pOutFrame );
  bool ExtractResultAndCheckForIterationFinish( const CDataFrame * pData );
  void FillCurrentPartParameters( PartParams& Part ) ;
  bool FilterCavitiesContours( CContainerFrame * pOutFrame );
  CDataFrame * FormBigMsg( cmplx& cComplex , 
    cmplx cPtOnIMage = cmplx(0.,0.) , 
    int iSize = 20 , LPCTSTR pColor = "0x00ffff" ,
    LPCTSTR pFormat = "Dpix(%.2f,%.2f)\nDum(%.2f,%.2f)" );
  CDataFrame * FormTextFrameForView( cmplx& cViewPt ,
    int iFontSize , LPCTSTR pColor , LPCTSTR pFormat , ... ) ;
  bool GetAverageAndStd( CmplxVector& Data , cmplx& cAverage , cmplx& cCmplxStd );
  double GetBoundToTenth( double dVal )
  {
    return ROUND( dVal * 10. ) / 10. ;
  } ;
  double GetCenterForThresholdAndStep( CmplxVector& Data , double dAbsThres , int iStep );
  double GetdZ(cmplx& cLaserSpotPos);
  FXString GetMainDir();
  bool GetFocusRectanglesForCavity(CRect& Left, CRect& Right);
  CRect GetNormFocusRectangle( cmplx cCenter , cmplx cSize );
  CRect GetRectangle( cmplx cCenter , cmplx cSize );
  CLine2d GetRegressionNearPoint( const CFigure& Fig , const cmplx& cCentPt , 
    bool bXtoPlus , double dHalfLength , CContainerFrame * pOutInfo , StraightLineRegression& Regr );
  cmplx GetShiftForNextZMeasurement();
  int GetStatisticsAboutContursAsSpots( const CDataFrame * pDataFrame );
  LPCTSTR GetShortWorkingModeName();
  LPCTSTR GetWorkingModeName();
  LPCTSTR GetWorkingStateName();
  double GetZResult();
  void GrabImage();
  int InitZDefocusingMeasurement();
  void InitZGrabForULF( LPCTSTR pCommand = NULL );
  int IsExposureAndTimingOK(const CVideoFrame * pVF); // 1 - OK, 0 - NOK , (-1) - unknown
  bool IsShiftedCenter();
  void LoadAndUpdatePartParameters();
  bool ProcessContours(); // true when NCenters == NContours
  int ProcessNoContursNoPoints( const CDataFrame * pDataFrame , CContainerFrame * pOutFrame );
  bool ProcessMPPTCommand(const CTextFrame * pCommand);
  int ProcessSideContours( CContainerFrame * pOutFrame , cmplx& LeftPt , cmplx& RightPt );
  void GetPatternPoint();
  int ProgramCamera(LPCTSTR pCameraParams, LPCTSTR pLabel = NULL);
  bool ProgramExposure(int iExp_us);
  bool ProgramExposureAndLightParameters(int iExp_us, 
    int iFirstLightTime_us_24V, int iSecondLightTime_us_5V);
  int ResetIterations();
  int RestoreKnownParts( LPCTSTR pPartName = NULL , bool bTellToEngine = false );
  int RestorePartDataFromRegistry(
   PartParams& Part , PartParams * pPrototype , 
    string& PartFolder , LPCTSTR pPartName );
  int RestorePartDataFromRegistry(
    LPCTSTR pPartName , PartParams& Part , PartParams * pGauge = NULL);
  bool RestoreXYCalibData();
  bool RestoreZCalibData(Named_ifstream& myfile);
  bool RestoreZCalibData();
  void SaveCSVLogMsg( LPCTSTR pFormat , ... ) ;
  void SaveCavityResultLogMsg( LPCTSTR pFormat , ... ) ;
  int SaveImage(const pTVFrame pImage, LPCTSTR pFileName);
  int SaveKnownParts();
  void SaveLogMsg(LPCTSTR pFormat, ...);
  void SaveOperativeLogMsg(LPCTSTR pFormat, ...);
  void SaveFocusLog(LPCTSTR pFormat, ...);
  int SavePartDataToRegistry(LPCTSTR pPartname , bool bTakeFromCurrent = false );
  int SavePartDataToRegistry(PartParams& Part);
  int SavePartDataToRegistryEx( PartParams& Part );
  bool SaveXYCalibData(FXString * pStatistics = NULL);
  bool SaveZCalibData();
  bool SaveZCalibData(Named_ofstream& myfile);
  int SelectCurrentPart(LPCTSTR pPartName);
  double SelectLaserSpot( cmplx& cSelectedPos );
  bool SendDisplaceCommand(double dX, double dY, double dZ);
  bool SendDisplaceCommand(CCoor3t Shift);
  bool SendDisplaceCommand(CCoord3 Shift);
  bool SendDisplaceCommand(cmplx& cMoveVect);
  bool SendMessageToEngine(LPCTSTR pMessage, LPCTSTR pLabel);
  bool SendScalesToRender();
  int SetBlankMode(int iExposure);
  void SetCavityMode(bool bXYMeasure = true, bool bLive = false);
  WhatMPPTFigureFound SetDataAboutContour(const CFigureFrame * pFrame);
  int SetDataAboutContours(CFramesIterator * pFiguresIterator);
  int SetDataAboutContours(const CDataFrame * pDataFrame);
  void SetLaserMode(LaserExpMode ExpMode, bool bLive = false);
  bool SetParametersToTVObject( LPCTSTR pParams ) ;
  void SetPinMode( bool bLive = false ,
    MPPT_State PinPart = DL_LiveVideoPinFocusing);
  void SetPinEdgeMode(EDGE_DIR Edge, bool bLive = false);
  int SetWatchDog(int iTime_ms);
  void SwitchLaser(bool bOn);
  int SwitchOnConstantLight( bool bOnRing , bool bOnStraight );
  int SwitchOffConstantLight( bool bOnRing , bool bOnStraight );
  bool UpdateAverageFocuses();
  MEAS_OBJECT WhatWeSee();
  int XYCalibrationProcess( CContainerFrame * pOutFrame , CCoor3t& m_Shifts );
  int ZCalibrationProcess( CContainerFrame * pOutFrame , CCoor3t& m_Shifts );
  bool SetMeasureConturParameters();
  int SelectPartByName( FXString& SelectedPartName , bool bReportToEngine = false );
  int CheckAndSendCavityPlot();
  int PlotULandCheckJumps(cmplx& cDistFOV_um , double dZVal_um);
  int FindCavityInternalEdges( cmplx cLeftCenter , cmplx cRightCenter ,
    cmplx& cLeftEdge , cmplx& cRightEdge , CContainerFrame * pMarking );
  bool ProcessContinueCommand() ;
};




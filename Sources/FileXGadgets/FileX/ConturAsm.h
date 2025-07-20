// ConturAsmGadget.h : Declaration of the ConturAsm

#pragma once

//#ifdef _DEBUG
//#ifdef ATLTRACE 
//#undef ATLTRACE
//#undef ATLTRACE2
//
//#define ATLTRACE CustomTrace
//#define ATLTRACE2 ATLTRACE
//#endif // ATLTRACE
//#endif // _DEBUG
//
//inline void CustomTrace( const TCHAR* format , ... )
//{
//  const int TraceBufferSize = 1024;
//  TCHAR buffer[ TraceBufferSize ];
//
//  va_list argptr; 
//  va_start( argptr , format );
//  _vstprintf_s ( buffer , format , argptr );
//  va_end( argptr );
//
//  ::OutputDebugString( buffer );
//}
//
//inline void CustomTrace( int dwCategory , int line , const TCHAR* format , ... )
//{
//  va_list argptr; va_start( argptr , format );
//  CustomTrace( format , argptr );
//  va_end( argptr );
//}
//

#include <math\intf_sup.h>
#include <Gadgets\gadbase.h>
#include <gadgets\videoframe.h>
#include <helpers\FramesHelper.h>
#include <classes\drect.h>
#include "DecodeZaberMotion.h"
#include <Math/FigureProcessing.h>
#include <stack>
#include "ConturData.h"
#include "Math\FRegression.h"
#include "GeometryAnalyzer.h"
#include <helpers\GraphicsOnImage.h>
#include <fxfc\FXRegistry.h>
#include <fxfc/FXLoggerToFile.h>


#define Fig_Create 1
#define Fig_Release 2
#define Fig_AddRef 3
#define Fig_Touch 4

enum HomingStates
{
  HS_HomingUnknown = 0 ,
  HS_InHoming ,
  HS_HomingDone ,
  HS_AllCoordsReceived
};

enum PartInitStatus
{
  Part_Unknown = 0 ,
  Part_Known ,
  Part_New 
};
enum ProcessingStage
{
  Stage_Unchanged = -1 ,
  Stage_Inactive                = 0 ,
  Stage_Load_Part ,
  Stage_PrepareMeasurement , // move edge to center
  Stage_Focusing ,
  Stage_EdgeToCenter ,
  Stage_GetInternalBorder ,
  Stage_GetExternalBorder ,
  Stage_WaitForMotionEnd ,
  Stage_WaitForMotionEndX ,
  Stage_WaitForMotionEndY ,
  Stage_ScanFinished ,
  Stage_LightToCenter ,
  Stage_LightFromCenter ,
  Stage_FocusingPlus ,
  Stage_FocusingMinus ,
  Stage_GetCurrentPositions ,
  Stage_ManualMotion ,
  Stage_0_GetCurrPos , // get current position
  Stage_1_MoveToObserv , // Move XYZ to observation
  Stage_2_MeasOnObserv , // Measure conturs on observation
  Stage_3_AutoFocusOnObserv , // Do autofocusing for +- 5 mm and
                       // area on cutter body
  Stage_4_MeasFocusedOnObserv , // Measure observation conturs in focused state
  Stage_5_SendToEdgeMeasurement , // Calculate target coordinates and order XY motion
                       // to initial edge measurement point
  Stage_6_MoveEdgeToCenter , // Find edge and move edge to center
  Stage_7_AutoFocusEdge , // Do focusing +- 500(?) microns on edge inspection station
  Stage_8_GetInternalBorder , // Get Internal border
  Stage_9_MoveEdgeToCenter , // edge to center after focusing and internal edge find

  Stage_10_GetIntOrExtBorder , // After motion to next measurement point
                        // Or after edge to center.
                        // May be two variants:
                        //   a. Get internal border (measurement by internal 
                        //      light should be done
                        //   b. Get external border, if internal is not necessary to know
  
  Stage_11_GetExternalBorder ,// Internal => Get external border AFTER Stage_10...(a)
  Stage_12_MeasWalls ,// External (real measurement) 
                          // 1. Try to measure high contrast edge walls
                          // 2. If not measured, save external edge and order
                          //    stage 12a
                          // 3. Check for scan process finishing
                          // 4. If finished then save data and stop process
                          // 5.	if not end then Move to next point ( stage 13 ) 
                          //	and push (IsFocusingNecessary? 
                          //    Stage 13 ( focusing ): Stage 14 (get internal image))
  Stage_12a_MeasDarkEdge , // 1. Try to measure low contrast edge walls
                          // 2. Check for scan process finishing
                          // 3. If finished then save data and stop process
                          // 4.	if not end then Move to next point ( stage 13 ) 
                          //	and push (IsFocusingNecessary? 
                          //    Stage 13 ( focusing ): Stage 14 (get external image))
  Stage_13_ScanInlineFocusing , // Focusing on scan => Stage 10 ( Get internal image
  Stage_14_OrderIntContour , // Order internal image => Stage 11

  Stage_OneDarkMeasBeginToMax ,
  Stage_OneDarkMeasEndToMax ,
  //   Stage_TryFocusing ,
//   Stage_ContinueFocusing ,
//   Stage_TryFocusingBack ,
//   Stage_ContniueFocusingBack ,
//   Stage_FocusLocalCheck ,
  Stage_BeginToMax , // try to find Z for maximal focus 
                     //near begin of measurement segment
  Stage_EndToMax ,   // try to find Z for maximal focus 
                     // for last approved measured point
                     // till last point on measurement segment
                     // will be in good focus


  Stage_DistCalib1 , //Go to Observation position
  Stage_DistCalib2 , // Measure calib spot
  Stage_DistCalib3 , // Go to Edge inspection position
  Stage_DistCalib4 , // Measure Calib Spot
  Stage_Adjust_Z   ,
  Stage_Observation_Focus ,
  Stage_GetExtImage ,
  Stage_GetAllLightsImage ,
  Stage_OneIntMeasurement ,
  Stage_OneExtMeasurement ,
  Stage_OneExtMeasurement_a ,
  Stage_GoToObservation ,
  Stage_OneObservMeasurement ,
  Stage_HomeX ,
  Stage_HomeY ,
  Stage_HomeZ ,
  Stage_CalibLight0 ,
  Stage_CalibLight1 ,
  Stage_CalibLight2 ,
  Stage_CalibLight3 ,
  Stage_CalibLight4 ,
  Stage_CalibLight5 ,
  Stage_CalibLight6 ,
  Stage_CalibLight7 ,
  Stage_CalibLightLast
};

enum WhatFigureFound
{
  WFF_NotFound = 0 ,
  WFF_PartContur = 1 ,
  WFF_PartCenter = 2 ,
  WFF_HoleContur = 4 ,
  WFF_HoleCenter = 8 ,
  WFF_ExternalContur = 16 ,
  WFF_InternalContur = 32 ,
  WFF_CalibContur = 64 ,
  WFF_CalibCenter = 128 ,
  WFF_CalibObservContur = 256 ,
  WFF_CalibObservCenter = 512 ,

  WFF_Conturs = WFF_PartContur | WFF_HoleContur
  | WFF_ExternalContur | WFF_InternalContur
  | WFF_CalibContur | WFF_CalibObservContur ,

  WFF_Centers = WFF_PartCenter | WFF_HoleCenter
  | WFF_CalibCenter | WFF_CalibObservCenter 

};

enum SectorMarkingMode
{
  SMM_NoMarking = 0 ,
  SMM_Marking = 1 ,
  SMM_Delete = -1 ,
  SMM_DeleteAll = -2 
};

enum MapMouseClickMode
{
  MCM_NoReaction = 0 ,
  MCM_ViewSaved ,
  MCM_GetAverage ,
  MCM_DeleteAverage ,
  MCM_RangePlus ,
  MCM_RangeMinus ,
  MCM_MoveAndView ,
  MCM_MoveAndMeasureOnce ,
  MCM_MoveAndFullMeasurement ,
  MCM_DoMapOutput ,
  MCM_PrintMap ,
  MCM_MeasureManual ,
  MCM_DeleteManual 
};

enum WhenMC_Available
{
  WMCA_Unavailable = 0 ,
  WMCA_LiveOnly ,
  WMCA_LiveAndSaved
};

typedef struct  
{
  MapMouseClickMode m_Mode ;
  WhenMC_Available  m_When ;
  LPCTSTR           m_pName ;
} MCM_Name ;

#define TVOBJ_PARS_FOR_SIMULATION "name=ext_segment;xoffset=10;yoffset=10;width=280;height=280;dir_degrees=45;"
#define TVOBJ_PARS_FOR_WORK "name=ext_segment;xoffset=360;yoffset=100;width=1200;height=1000;dir_degrees=90;"

extern LPCTSTR StageNames[] ;
extern LPCTSTR StageDescriptions[] ;
int GetNStageNames() ;
int GetNStageDescriptions() ;

LPCTSTR GetStageName(ProcessingStage PrSt);
bool IsShiftKeyDown();
bool IsCTRLKeyDown();
LPCTSTR GetStageDescription(ProcessingStage PrSt);

class EmbeddedFrameInfo
{
public:
  DWORD m_dwPattern ;
  DWORD m_dwTimeStamp ;     //0
  DWORD m_dwFrameCntr ;     // 4
  DWORD m_dwStrobePattern ; // 8
  DWORD m_dwGPIOPinStae ;   // 12
  CRect m_ROIPos ;        // 16
  DWORD m_Reserved[ 2 ];    // 20

  EmbeddedFrameInfo() { Reset() ; }
  void Reset()
  { 
    memset( this , 0 , sizeof( *this ) ) ; 
    m_dwPattern = 0xaa5555aa ;
  }
  bool IsPattern()
  {
    return (m_dwPattern == 0xaa5555aa) ;
  }
} ;




enum WorkingMode 
{
  WM_Unknown = -1,
  WM_ScanOnly = 0 ,
  WM_Measure = 1 ,
  WM_DarkEdge = 2 ,
  WM_DarkEdge_NoInternal
};

enum ResultViewMode
{
  RVM_Unknown = -1 ,
  RVM_3D = 0 ,
  RVM_BurrWidth ,
  RVM_MaxBurrWidth ,
  RVM_BurrHeight ,
  RVM_MaxBurrHeight ,
  RVM_BurrVolume ,
  RVM_BurrEdgeLength
};
enum MotionSequence
{
  MS_NoMotion = 0 ,
  MS_Both ,
  MS_XSecond ,
  MS_YSecond
};
enum LightDir
{
  LD_Ortho = 0 ,
  LD_Centered
};

enum FocusState
{
  FS_OK = 0 ,
  FS_ContinueFocusing ,
  FS_TakeValuesFromPrevious ,
  FS_TakeValuesFrom = 1000 ,

  FS_ERROR = -1
};

enum AvailableDataFrom
{
  ADF_NoData = 0 ,
  ADF_MeasuredNow ,
  ADF_MeasuredInThePast
};


#define SEGMENT_TURN_INITIAL_THRES (DegToRad(7.5))

class SentFramesIds
{
public:
  TCHAR m_Label[40] ;
  int   m_iNUsers ;
  int   m_iNRUs ;
  const CDataFrame * m_pFrame ;
  DWORD m_Id ;
  DWORD m_dwBegin ;
  double m_dTime ;

  void Reset() { memset( this , 0 , sizeof( *this ) ) ; }
};

class AveragedData
{
public: 
  double m_dAverGrad_um  ;
  double m_dMaxGrad_um  ;
  double m_dLength  ;
  int m_iNAccum ;
  cmplx m_cSegmCenter ;
  cmplx m_cSegmCenterInFOV ;
  int m_iNSegments ;
  cmplx m_cSegmentBegin ;

  AveragedData() { Reset() ; }

  void Reset() { memset( this , 0 , sizeof( *this ) ) ; }

  double Add( const ConturSample * pCurr , const ConturSample * pNext )
  {
    if ( m_iNSegments == 0 )
      m_cSegmentBegin = cmplx( pCurr->m_PointOnMap.x , pCurr->m_PointOnMap.y ) ;

    double dWidth_um = pCurr->m_dAveBurrWidth_um ;
    if ( dWidth_um > 5. )
    {
      m_dAverGrad_um += dWidth_um ;
      m_iNAccum++ ;
      if ( dWidth_um > m_dMaxGrad_um )
        m_dMaxGrad_um = dWidth_um ;
    }
    m_cSegmCenter += pCurr->m_MiddlePt ;
    m_cSegmCenterInFOV += cmplx( pCurr->m_PointOnMap.x , pCurr->m_PointOnMap.y ) ;
    m_iNSegments++ ;
    m_dLength += abs( pCurr->m_MiddlePt - pNext->m_MiddlePt ) ;
    return m_dLength ;
  }
  void operator=( const AveragedData& Other ) { memcpy( this , &Other , sizeof( *this ) ) ; }

  double GetAverGrad_um() { return m_dAverGrad_um / (double)m_iNAccum ; }
  double GetMaxGrad_um() { return m_dMaxGrad_um ; }
  cmplx  GetCenterPt() { return m_cSegmCenter / (double)m_iNSegments ; }
  cmplx  GetCenterPtInFOV() { return m_cSegmCenterInFOV / (double) m_iNSegments ; }
};

typedef FXArray<AveragedData> AverData ;

#define CAM_OBSERVATION 0
#define CAM_MEASUREMENT 1
#define OBSERVATION_LEDS 0x0300
#define OBSERV_RIGHT_LED 0x0100
#define OBSERV_LEFT_LED  0x0200
#define MEASUREMENT_LEDS 0x00ff
#define GetRelIndex(iIndex,iArrLen) ( (iIndex + iArrLen) % iArrLen )
#define GetRelElem(pArr,iIndex,iShift,iLen) (pArr[(iIndex+iShift+iLen)%iLen]) 
#define LED_ANGLE_STEP M_PI_4
#define LEDS_SHIFT_ANGLE (M_PI_4/2)  // 22.5 degrees in mechanical construction
#define FULL_LEDS_SHIFT_ANGLE (7.*M_PI/8.) // relative to direction zero (~to the right on FOV)
#define N_NEIGHTBOURS 30
#define N_AVE_PTS     20



extern Directions gOpposite[8];
extern int gFront[8]          ;
extern int gBack[8]           ;
int GetLightDir(Directions Dir, bool bFront);
double GetCurrentInternalLightAngle(Directions Dir, bool bFront);
cmplx GetAverageAroundPt(cmplx * pcData, int iLen, int iPtIndex, int iRange);
FXString FormSpeedInUnitsFrom_um_persecond(double dSpeed_um_per_sec);


typedef struct 
{
public:
  double m_dPos ;
  double m_dPos2 ;
  cmplx m_cFirst ;
  cmplx m_cExt ;
  cmplx m_cOrtho ;
} PosOnInterval ;

void CALLBACK TimerRoutine( LPVOID lpParam , BOOLEAN TimerOrWaitFired ) ;

class ExtremSample
{
public:
  ExtremSample(double dValue = 0., double dCoord = 0.,
    double dResultTime = 0., double dCoordTime = 0.)
  {
    m_dValue = dValue;
    m_dCoord = dCoord;
    m_dResultTime = dResultTime;
    m_dCoordTime = dCoordTime;
  }

  double m_dValue, m_dCoord, m_dResultTime, m_dCoordTime;
};

typedef FXArray<ExtremSample> Samples;

class FocusSample
{
public:
  double m_dValue, m_dCoord ;
  FocusSample(double dCoord = 0., double dVal = 0.)
  {
    m_dValue = dVal; 
    m_dCoord = dCoord;
  }
  bool IsOtherZHigher( FocusSample& Other )
  {
    return (Other.m_dCoord > m_dCoord) ;
  }
  bool IsOtherValHigher( FocusSample& Other )
  {
    return (Other.m_dValue > m_dValue) ;
  }
};

typedef vector<FocusSample> FocusData;

class ConturAsm : public CFilterGadget, DecodeZaberMotion
{
  friend void CALLBACK TimerRoutine( LPVOID lpParam , BOOLEAN TimerOrWaitFired ) ;
public:

//

  FXString           m_GadgetInfo ;
  COutputConnector * m_pLightControl ; // light control , task control 
                                       // image pass to view control
                                       // Focus Control, Z control
  COutputConnector * m_pXMotion ;      // X motion control
  COutputConnector * m_pYMotion ;      // Y motion control
  COutputConnector * m_pMarking ;      // graphics on image output
  COutputConnector * m_pImageSave ;    //
  COutputConnector * m_pMainView ;

  CDuplexConnector * m_pDuplex ;

  int                m_iSimulationMode = 0 ;
  int                m_bShuffle = 0 ;
  bool               m_bUpdateSimulationData = false ;
  int                m_iLastSimulationPt = 0 ;
  int                m_iCurrPtForSimulation = 0 ;
  FXStringArray      m_SimulationDirectories ;
  FXUIntArray        m_NFilesInSimulationDirs ;
  int                m_iDirIterator ;

  ConturData         m_ConturData ;
  AverData           m_AverData ; // for average thickness showing
                                  // around contour on map
  AvailableDataFrom  m_DataFrom ;
  MapMouseClickMode  m_MapMouseClickMode ;
  CRect              m_SelectorsAreaOnMap ;
  int                m_iAveragingRange_um ;
  AveragedSelectedPoints m_AveragedSelectedPoints ;
  double             m_dLastPrintTime ;
  int                m_iLastSentToUI_Index;
  ConturSample       m_OneSample ;
  FXString           m_SystemConfigFileName ;
  FXString           m_PartFile ;  // Ordered part name and final file name
  FXString           m_NewPartName ;
  CRect              m_LastSavedFragment ;
  FXString           m_PartDirectory ; // there will be observation part image and log
  FXString           m_ResultDirectory ; // There will be result file, map file and observation image
  FXString           m_ImagesDirectory ; // There will be all images 300x300 (size is discussable)
  FXString           m_MeasurementTS ; // time stamp for current measurement
  FXString           m_PureFileName ;  // file name without extension and directory
  FXString           m_SimulationLogFileName ;
  FXSimpleLogToFile  m_SimulationLogger;
  UINT               m_uiSimuFileIndex ;
  FXString           m_WatchDogMessage ;
  int                m_iDelayBetweenSegmentsOnLoading ;
  int                m_iLastShownIndex ;  // Map to point data
  CCoordinate        m_SavedPositions[ 100 ] ;

  static int             g_iIndexInLCTRL ;
  static SentFramesIds   g_LCTRLSentContainers[ 1000 ] ;
  static double          g_dLastSendFrameTime ;

  HANDLE              m_hWatchDogTimer ;
  FXLockObject        m_LockWatchdog ;
    // Properties
  Directions  m_iDir ; 
  int         m_iMeasStep ;
  int         m_iMeasZoneWidth ;
  int         m_iMinimalContinuty ;
  int         m_iSegmentLength ;
  WorkingMode m_WorkingMode ;
  ResultViewMode m_ResultViewMode ;
  int         m_iSelectedPtIndex ;
  int         m_iMaxGradPtIndex = -1 ;
  bool        m_bFormMaxGradMarker = false;
  LightDir    m_LightDir ;
  int         m_iNLeds ;
  FXDblArray  m_dMaximumsForViewCsale ;

  FXString    m_sInitialDir ;
  Directions  m_InitialDir ;
  double      m_dInitialAngle ;
  double      m_dSegmentTurnThres ;
  double      m_dScale_pix_per_um ; // for measurement camera
  double      m_dObservScale_pix_per_um ;// for observation camera
  double      m_dMapScale_pix_per_um ;   // for map image
  double      m_dMinGradWidth_um ;
  double      m_dMaxGradWidth_um ;
  double      m_dMaxGradWidth_pix;
  int         m_iNMinGoodSamples ;
  double      m_dLastObservCoordTime ;
 
  double      m_dBaseHeight ;
  double      m_dPartHeight ;
  double      m_dAdapterHeight ;

  cmplx       m_cMeasCenter ; // Position of robot for 
                              // matching measurement FOV center
                              // and part platform center
  CCoordinate m_ccMeasCenter ; // the same 3d without adapter and part accouting
  cmplx       m_cObservCenter ; // Position of robot for 
                                // matching observation FOV center
                                // and part platform center
  CCoordinate m_ccObservCenter ; // the same 3d without adapter and part accouting
  cmplx       m_cPartCenterOnMap ;  // for % accounting
  CCoordinate m_DistFromObservationToMeasurement ;
  cmplx       m_cPartCenterOnMeas ; // This value tied to next one
  cmplx       m_cInitialRobotPosForMeas ; // correspondent
                                           // to m_cPartCenterOnMeas
  cmplx       m_cInitialMeasPos ;
  cmplx       m_cInitialVectToPartCent ;
  cmplx       m_cRobotPosToInitialPt ;

  cmplx       m_cMeasRange ;  // Restriction area from m_cMeasCenter 

  cmplx       m_cObservImageCent ;
  cmplx       m_cMeasImageCent ;
  cmplx       m_cPartPlacementPos ;
  cmplx       m_cNormToDirectionLight ; // to center
  CDRect      m_FreeZone ;
  CDRect      m_PlacementZone ;
  cmplx       m_cCurrentTarget ;
  cmplx       m_cFinalTarget ;
  double      m_dZTarget ;
  double      m_dLastMotionOrderTime ;
  CCoordinate m_CheckedPos ;
  MotionSequence m_MotionSequence ;
  bool        m_bReportAfterMotion ;
  FXString    m_StringAfterMotion ;

  Directions  m_iPrevDir ; 
  int         m_LastLightMask ;
  int         m_iLastPulseTime ;
  FXString    m_sLastLightCommand ;
  FXString    m_sLastCmndForIntLight ;
  FXString    m_sLastCmndForExtLight ;
  FXString    m_sLastCmndForFocusingLightWithExt ;
  int         m_iFocusingLight_us ;
  double      m_dLastLightAngle ;
  double      m_dNextLightAngle ;
  double      m_dLastAngleToCenter ; // Standard notation, 
                                     // i.e. Zero to the left, ccw plus
  double      m_dScanInitialAngle ; // angle when scan begins (~PI)
  bool        m_bPrepareToEnd ;     // when 
  bool        m_bScanFinished ;
  SectorMarkingMode m_SectorsMarkingMode ; // 0 - no marking, 1 - make new , -1 - delete , -2 - delete all
  double      m_dSectorRange_um ;
  double      m_dLightForExternal ; // calculated angle
  double      m_dLightForInternal ; // calculated angle
  FXDblArray  m_dDistToPt0 ;
  double      m_dMinimalDistToFirstPt ;
  double      m_dRealLightDirForExternal ;
  ProcessingStage m_ProcessingStage ;
  ProcessingStage m_StageBeforeMotion ;
  ProcessingStage m_StageAfterFocusing;
  bool        m_bFocusingAfterPosFinishing;
  std::stack<ProcessingStage> m_DelayedOperations ;
  cmplx       m_cLastMotion ;
  cmplx       m_cLastNotSmallMotion ;
  int         m_iNotMeasuredCntr ;
  int         m_iNotMeasuredLimit ;
  cmplx       m_cRobotPos ;
  cmplx       m_cMeasFOVCent ;
  cmplx       m_cObservFOVCenter ;
  cmplx       m_cFOVCenterForLightCalib ;
  cmplx       m_cPartCentPos ;
  cmplx       m_cTraveling ;
  double      m_dTraveling  ;
  cmplx       m_cLastNormalMotion ;
  cmplx       m_VectorsViewCent ;
  cmplx       m_cTextViewCent ;
  cmplx       m_cFinishingMsgPt;
  cmplx       m_cEdgeVect ;
  int         m_iNMotionSteps ;
  int         m_iLastFocusingMotionSteps ;
  bool        m_bPointsSwapped ;
  int         m_iStopOnFault ;
  double      m_dStopReceivedTime;
  int         m_iLastTask ;
  int         m_iLightCalibLEDCntr ;
  bool        m_bShiftKeyIsDown;
  bool        m_bCTRLKeyIsDown;

  CRect          m_LastROI ;
  ArrayOfSegments m_NewSegments ;
  //ActiveSegments m_pSavedActiveSegments ;

  const CFigureFrame * m_pLastInternalContur ;
  const CFigureFrame * m_pLastExternalContur ;

  const CFigureFrame * m_pPartConturOnObservation ;
  const CFigureFrame * m_pHoleConturOnObservation ;
  const CFigureFrame * m_pHoleConturOnMeasPlace ;
  FramesCollection     m_LastExtConturs ;
  FramesCollection     m_ObservationPartConturs ;
  CFigureFrame *       m_pFilteredObservationContur ;
  FramesCollection     m_LastScanFrames ;
  CVideoFrame *        m_pObservationImage ;
  CContainerFrame *    m_VectorsToVertices ;
  cmplx                m_cPartCenterOnObservation ;
  cmplx                m_cHoleCenterOnObservation ;
  cmplx                m_cHoleCenterOnMeasurement ;
  cmplx                m_cLastROICenter;
  DWORD                m_dwLastFilledConturs ;
  DWORD                m_dwFilledContours ;
  double               m_dPartConturLength ;
  GeometryAnalyzer     m_ObservGeomAnalyzer;
  FXString             m_FormDescrFullPath ;
  FormDescriptorEx     m_LastObservationForm;
  FormDescriptor       m_IdentifiedObservForm;
  FormDescriptor       m_UnknownObservationForm;
  FormDescriptorEx     m_CurrentDetailedForm ;
  int                  m_iIndexOfFoundObservForm;
  cmplx                m_cMapImageCenter ;
  cmplx                m_cPartOnMapCenterByExtrems ;
  
  FXIntArray           m_LastMaxesOnObservation ;
  FXIntArray           m_LastMinsOnObservation ;
  CCoordinate m_LastInternalConturRobotPos ;
  CCoordinate m_LastExternalConturRobotPos ;
  int         m_iMinDistIndexForInternal ;
  int         m_iMinDistIndexForExternal ;
  int         m_iFirstPtIndexForInternal ;
  int         m_iLastPtIndexForInternal ;
  CDPoint     m_NearestInternal ;
  CDPoint     m_NearestExternal ;
  CSegmentInsideROI * m_pLastActiveInternal ;
  CSegmentInsideROI * m_pLastActiveExternal ;
  CFigureFrame * m_pPartInternalEdge ;
  CFigureFrame * m_pPartExternalEdge ;
  CContainerFrame * m_pDebugging ;
  CDblArray   m_Widths ;

  FXIntArray  m_LastAverageDefocusingValues ;
  FXIntArray  m_LastMaxDefocusingValues ;
  int         m_iLastMinAverageDefocusing ;
  int         m_iLastMaxAverageDefocusing ;
  int         m_iLastMinMaxDefocusing ;
  int         m_iLastMaxMaxDefocusing ;
  double      m_dLastFocusRegressionA ; // For focus estimation F=a*x + b
  double      m_dLastFocusRegressionB ;

  cmplx       m_PartEdgeViewCenter;
  CmplxArray  m_ObservationExtremes ;
  CmplxArray  m_MeasurementExtremes ;
  cmplx       m_cPartCenterOnObserv ;
  cmplx       m_cObservationSizes ;

  // Temporary arrays
#define TEMP_ARRAY_SIZE 3000
  double      m_Signal[ TEMP_ARRAY_SIZE ] ;
  double      m_Signal2[ TEMP_ARRAY_SIZE ] ;
  int         m_Bads[ TEMP_ARRAY_SIZE ] ;
  double      m_dEdgeViewScale ;
  double      m_dWidthViewScale ;
  int         m_iViewLength ;
  int         m_iCameraMask ;
  int         m_iLightLen ;
  double      m_dUpperThreshold ;
  double      m_dThicknessThreshold ;
  double      m_dNeightboursThicknessRatioThres;
  double      m_dMinAmplAfterEdge;
  double      m_dShadowThres ;
  double      m_dStdThreshold ;
  double      m_dDarkEdgeThreshold ;
  int         m_iMinShadowAmpl ;

  int         m_iFocusingPeriod ; // if (m_iNMotionSteps % m_iFocusingPeriod) == 0,
                                  // system does auto focusing; if 0 - no focusing
  int         m_iNStepsAfterFocusing ;
  int         m_iFocusAreaSize_pix ;
  int         m_iFocusDistFromEdge_pix ;
  int         m_iObservFocusRange_um ;
  int         m_iMeasInitialFocusRange_um ;
  int         m_MeasFocusRange_um ;
  int         m_iFocusWorkingMode ;
  double      m_dFocusingCorrection_um ;
  double      m_dFocusDecreaseNorm ;
  double      m_dFocusRangeK ;
  double      m_dMinimalContrast ;

  double      m_dCenterFocusValue ;
  double      m_dOverFocusValue ;
  double      m_dUnderFocusValue ;
  double      m_dLastFocusValue ;  // value from DiffSum gadget
  double      m_dLastFocusValueAfterFocusing ;
  double      m_dLastFocusTime ;
  double      m_dLastFocusCoordinate ;
  double      m_dLastContrastAfterFocusing ;
  double      m_dLastContrast ;
  double      m_dLastNextPtContrast ;
  int         m_bIsFocusingNecessary ;
  double      m_dFocusingRange_um ;
  double      m_dStartFocusingTime ;
  double      m_dMaxCrossDist ;
  double      m_dLengthRatio ;
  double      m_dCrossDistStd ;
  double      m_dLastMaxTurn ;
  double      m_dLastMinSegmentLength ;
  double      m_dLastMaxSegmentLength ;
  double      m_dLastEdgePredictedAngle_deg ;

  FocusData   m_FocusData;
  double      m_dFoundFocus_um;
  double      m_dCurrentFocusStep_um ;
  int         m_iNSmallSteps ;
  bool        m_bFocusingFinished ;
  double      m_dCurrentZCoord_um ;
  CRect       m_FocusingRect;

  int         m_iMeasurementExposure_us ;
  int         m_iObservationMeasExp_us ;
  int         m_iExtLight_us ;
  int         m_iIntLight_us ;
  int         m_iLightForDark_us ;
  int         m_iLightForLED_us[ 8 ] ;
 
  bool        m_bInFocusing ;
  int         m_iObservationFocusExp_us ;
  int         m_iFocusStep_um ;
  bool        m_bInLineFocusing ;
  bool        m_bFocusFound ;
  bool        m_bInMeasurementFocusFound ;
  bool        m_bInitialFocusing ;
  int         m_iOut_pix ;
  int         m_iIn_pix ;

  double      m_dLastAverageWidth_um ;
  double      m_dLastMaxWidth_um ;     // for one XY position
  double      m_dLastScanMaxWidth_um ; // For full SCan
  double      m_dLastAverageStdWidth_um ;
  double      m_dLastMaxStdWidth_um ;

  double      m_dLastAverageHeight_um ;
  double      m_dLastMaxHeight_um ;
  double      m_dLastBurrLength_um ;
  double      m_dLastBurrVolume_qum ;
  int         m_iIndexWithMaxWidth ;
  int         m_iIndexWithMaxOfMaxWidth ;
  CRect       m_ScaleBarPos ;
  int         m_iPercentOfMeasured ;
  double      m_dLastResultSaveTime ;
  double      m_dMeasurementStartTime;
  double      m_dLastScanTime_ms;

  CCoordinate m_InitPos ;
  double      m_dLastEdgeAngle ;
  int         m_iStage2FaultCounter ;
  int         m_iIndexForMapView ;
  HomingStates m_HomingState ;

  int         m_iDefocusToPlus ; // 1: lens to cutter, -1: lens from cutter
  bool        m_bFocusDirectionChanged ;
  int         m_iFocusFall_perc ;
  double      m_dK_DefocusTodZ ; //Percents to microns
                               // i.e. H_um=Def_perc * m_dK_DefocusTodZ 
  double      m_dLastdZ ;
  double      m_dLastOptimalZ ;

  cmplx       m_cLastBeginPt ;
  cmplx       m_cLastEndPt ;
  cmplx       m_cFirstPtRobotCoord ;
  int         m_iFocusPtIndex ;
  double      m_dMaxAvFocusForPosition ;
  double      m_dFocusStep ;
  double      m_dLastGradValue_um ;
  cmplx       m_cGradViewPt ;
  FocusState  m_LastFocusState ;
  DataForPosition m_DataForPosition ;
  FramesCollection m_ViewsForPosition ;
  cmplx       m_cResMiddlePoint ;
  // Non standard using of following var
  // m_Results will hold maximal average value for each segment
  // m_FV will hold maximal average and maximal values
  //      on edges and in the center
  MeasuredValues  m_CurrentViewMeasStatus ;
  double          m_dMinZ , m_dMinZWithGood ;
  double          m_dMaxZ , m_dMaxZWithGood ;
  int             m_iLastGoodMeasured ;
  double          m_dMinimalLocalFocus ;
  double          m_dNextGoodZ ;
  int             m_iXYMotionScanSpeed_um_sec ;
  int             m_iZMotionSpeed_um_sec ;
  int             m_iAccel_units ;
  cmplx           m_SaveFragmentSize ;

  // For map message processing
  CPoint          m_LastMapPt ;
  bool            m_bLastShiftPressedForMap ;
  CPoint          m_LastScaleCaptionPt ;
  FXString        m_LastScaleCaptionText ;
  CPoint          m_LastInfoAboutGradPt ;
  FXString        m_LastInfoAboutGrad ;
  CPoint          m_LastTextViewPtOnSavedImage ;
  FXString        m_LastPtDescription ;
  cmplx           m_LastViewPtr1st , m_LastViewPtr2nd ;
  cmplx           m_LastPointer[2] ;
  cmplx           m_MaxGradPtr[2] ;
  // Current part parameters
  
  FXString           m_PO; // order Id for Iscar DB interaction
  FXString           m_InternalCatalogID;  // pure ID without catalog name
  FXString           m_PartCatalogName;
  FXString           m_FullPartName; // 
  FXString           m_PartDetailsFileName;
  int                m_iKnownPartsIndex;
  PartInitStatus     m_CurrentPartStatus;

  FXSimpleLogToFile m_MotionLogger;
  BOOL              m_bDoMotionLog = FALSE;


// Main gadget declaration
  DECLARE_RUNTIME_GADGET(ConturAsm);
  
  
  // class methods
  public:
  // SH related constructor and virtual functions
  ConturAsm(void);
  void ShutDown();
  virtual int GetOutputsCount() { return 7; }
  virtual COutputConnector * GetOutputConnector(int n)
  {
    switch (n)
    {
    case 0: return m_pOutput;
    case 1: return m_pLightControl;
    case 2: return m_pXMotion;
    case 3: return m_pYMotion;
    case 4: return m_pMarking;
    case 5: return m_pImageSave;
    case 6: return m_pMainView;
    default: return NULL;
    }
  }
  virtual int GetDuplexCount() { return 1; }
  virtual CDuplexConnector* GetDuplexConnector(int n)
  {
    return (n == 0) ? m_pDuplex : NULL;
  }

  LPCTSTR GetGadgetInfo() { return (LPCTSTR)m_GadgetInfo; };
  void AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame ) ;
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );


  bool ScanProperties( LPCTSTR text , bool& Invalidate );
  bool PrintProperties( FXString& text );
  bool ScanSettings( FXString& text );

  // Part descriptors decoding
  // m_PartFile and m_NewPartName analysis on scan operation start
  PartInitStatus DecodePartDescriptors();
  bool LoadPartParametersFromRegistry( bool bDefault = false ); // registry item is in m_FullPartName
  bool SavePartParametersToRegistry( bool bSuccess ); // registry item is in m_FullPartName
  bool SaveLastScanParametersToRegistry( bool bSuccess );
  CContainerFrame * GetNewOutputContainer( const CDataFrame * pInput ) ;
  Directions GetNewDirection( cmplx& EdgeToM , cmplx& EdgeToP , cmplx& cCent );
  bool GetScalingFragment( const CmplxArray& source , int scalingFactor, __out CRect& ScalingFragment ) const;

  LPCTSTR GetStageName()
  {
    return ::GetStageName( m_ProcessingStage ) ;
  }

  int SetWorkingLed( Directions Dir , bool bFront , int iLedNum = -1 ) ;
  int SetWorkingLed( DWORD dwMask , int iLightTime = 500 ) ;
  int SetWorkingLeds( DWORD dwCamera , DWORD dwLEDMask , int iLightTime = 500 ) ;
  int SetWatchDog( int iTime_ms , LPCTSTR pMsg = NULL ) ;
  bool DeleteWatchDog() ;

  double GetAngleFromDir( int iDir )
  {
    return iDir * LED_ANGLE_STEP ;
  }
  Directions GetDir( double dAngle )
  {
    int iDir = ROUND( ( -( dAngle - M_PI_2 ) + M_2PI ) / LED_ANGLE_STEP ) ;
    iDir &= 0x07 ;
    return (Directions)iDir ;
  }

  double GetDiscreteDir(double dAngle);
  DWORD GetLightMask(double dAngle,
    double& dRealLightAngle, int iNLeds = 0);
  int SetLightFromAngle(double dAngle,
    int iLightningTime_us, double& dRealLightAngle, int iNLeds = 0);
  int SetLightForExternal(double dAngle,
    int iLightningTime_us, double& dRealLightAngle);
  int SetLightForInternal(double dAngle,
    int iLightningTime_us, double& dRealLightAngle, int iNLeds = 0);
  cmplx GetMeasAbsPos(cmplx cPosInFOV);
  cmplx GetPartCenterPositionOnMeas();
  cmplx GetVectFromFOVCentToPartCent();
  cmplx GetVectToPartCent(const cmplx& cPos);
  cmplx GetVectToPartCent();
  cmplx GetVectToPartCent(cmplx& cPtInFOV);

  double GetAngleToCenter();
  double GetAngleFromCenter();
  void OrderBMPForSimulation( bool bMeasurement ); // false - observation, true - measurement
  void OrderNextDirForSimulation( LPCTSTR pDirName , bool bMeasurement ) ;
  cmplx DecodeCoordsForSimulation( FXString Label ) ;
  void SetTask( int iTask );
  void SetTVObjPars( LPCTSTR pObjNameAndPars );
  int SetLightFromCenter(int iNLeds = 0);
  int SetLightToCenter(int iNLeds = 0);
  int SetLightForDarkEdge(int iTaskNumber = -1);
  bool IsWaitForCoordinates()
  {
    return (m_ProcessingStage == Stage_WaitForMotionEnd
      || m_ProcessingStage == Stage_GetCurrentPositions
      || m_ProcessingStage == Stage_ManualMotion 
      || m_ProcessingStage == Stage_0_GetCurrPos 
      || IsLightCalibration() ) ;
  }
  bool IsDarkMode()
  {
    return (m_WorkingMode == WM_DarkEdge
      ||    m_WorkingMode == WM_DarkEdge_NoInternal) ;
  }

  bool IsDark()
  {
    return (m_ProcessingStage == Stage_12a_MeasDarkEdge
         || m_ProcessingStage == Stage_OneExtMeasurement_a
         || m_ProcessingStage == Stage_OneIntMeasurement ) 
      && IsDarkMode() ;
  }
  bool IsBeginToMax()
  {
    return (m_ProcessingStage == Stage_OneDarkMeasBeginToMax
         || m_ProcessingStage == Stage_BeginToMax) ;

  }
  bool IsOneDarkMeasurement()
  {
    return ( m_ProcessingStage == Stage_OneDarkMeasBeginToMax 
          || m_ProcessingStage == Stage_OneDarkMeasEndToMax ) ;
  }

  bool IsGoToFocusPosition()
  {
    return ((m_ProcessingStage == Stage_4_MeasFocusedOnObserv)   // ZStage goes to focus position 
                                                                 //after observation
      || (m_ProcessingStage == Stage_9_MoveEdgeToCenter)      // ZStage goes to focus position after 
                                                        // initial measurement focusing
      || (m_ProcessingStage == Stage_10_GetIntOrExtBorder)) ;  // ZStage goes to focus position after
                                                        // inline focusing
  }
  bool IsEmbeddedFocus()
  {
    return ( m_ProcessingStage == Stage_BeginToMax
          || m_ProcessingStage == Stage_EndToMax
          || m_StageBeforeMotion == Stage_BeginToMax
          || m_StageBeforeMotion == Stage_EndToMax
          || m_ProcessingStage == Stage_ScanFinished
          || IsOneDarkMeasurement() 
      ) ;
  }
  bool IsDarkWithEmbFocus()
  {
    return ((IsEmbeddedFocus() || IsOneDarkMeasurement()) && !m_iSimulationMode ) ;
  }
  bool IsFinalMeasurement()
  {
    return ( (m_ProcessingStage == Stage_12_MeasWalls
      || m_ProcessingStage == Stage_12a_MeasDarkEdge
      || m_ProcessingStage == Stage_OneExtMeasurement
      || m_ProcessingStage == Stage_OneExtMeasurement_a) && !m_iSimulationMode ) ;
  }
  bool IsBeforeDark()
  {
    return (m_ProcessingStage == Stage_12_MeasWalls
         || m_ProcessingStage == Stage_OneExtMeasurement ) ;
  }
  bool IsOneMeasurement()
  {
    return (m_ProcessingStage == Stage_OneIntMeasurement
      || m_ProcessingStage == Stage_OneExtMeasurement
      || m_ProcessingStage == Stage_OneExtMeasurement_a
      || IsOneDarkMeasurement()
      /*|| m_ProcessingStage == Stage_OneExtMeasurement_a*/) ;
  }

  bool IsLightCalibration()
  {
    return ( (int)m_ProcessingStage >= (int)Stage_CalibLight0
      && (int)m_ProcessingStage < (int)Stage_CalibLightLast ) ;
  }

  bool IsInScanProcess()
  {
    return (m_ProcessingStage == Stage_1_MoveToObserv)
      || (m_ProcessingStage == Stage_2_MeasOnObserv )
      || (m_ProcessingStage == Stage_4_MeasFocusedOnObserv)
      || (m_ProcessingStage == Stage_5_SendToEdgeMeasurement)
      || (m_ProcessingStage == Stage_9_MoveEdgeToCenter)
      || (m_ProcessingStage == Stage_BeginToMax)
      || (m_ProcessingStage == Stage_EndToMax)
      ;
  }

  bool IsInScanMotion()
  {
    ProcessingStage Delayed = m_DelayedOperations.size() ?
      m_DelayedOperations.top() : Stage_Inactive ;
    if ( m_ProcessingStage == Stage_WaitForMotionEnd
      && m_DelayedOperations.size() )
      return true ;

    return false ;
  }

  bool IsInScanLighting()
  {
    switch ( m_ProcessingStage )
    {
      case Stage_12a_MeasDarkEdge:
      case Stage_12_MeasWalls:
      case Stage_11_GetExternalBorder:
      case Stage_10_GetIntOrExtBorder:
      case Stage_7_AutoFocusEdge:
      case Stage_5_SendToEdgeMeasurement:
      case Stage_2_MeasOnObserv: 
      case Stage_BeginToMax:
      case Stage_EndToMax:
        return true ;

    }
    return false ;
  }

  bool ConturAsm::PutFrameAndCheck(COutputConnector * pPin, CDataFrame * pData,
    LPCTSTR ErrorMsg, ProcessingStage OnError, ProcessingStage OnOK = Stage_Unchanged);

  bool ConturAsm::SetCamerasExposure(int iExp_us, LPCTSTR pErrorMsg, ProcessingStage OnError);

  bool ConturAsm::StartFocusing(int iRangeMinus, int iRangePlus,
    LPCTSTR pLightCommand, int iScanSpeed, int iExposure_us, int iWorkingMode = 2);

  bool StartFocusing( LPCTSTR pLightCommand , int iScanSpeed , int iExp_us , int iWorkingMode ) ;
  bool SetFocusingArea( cmplx& ViewPtOnEdge ) ;
  bool IsFocusing()
  {
    return (m_bInFocusing) ;
  }

  bool IsInactive()
  {
    if (m_dStopReceivedTime != 0.)
    {
      if (GetHRTickCount() - m_dStopReceivedTime > 2000.)
      {
        m_dStopReceivedTime = 0.;
        m_ProcessingStage = Stage_Inactive;
        return true;
      }
      else
        return false ;
    }

    return (m_ProcessingStage == Stage_Inactive
      || m_ProcessingStage == Stage_ScanFinished) ;
  }

  bool IsMotionStep()
  {
    return (m_ProcessingStage == Stage_WaitForMotionEnd
      || m_ProcessingStage == Stage_WaitForMotionEndX
      || m_ProcessingStage == Stage_WaitForMotionEndY ) ;

  }
  void SendReport( LPCTSTR pReportText )
  {
    CTextFrame * pReport = CreateTextFrame(
      pReportText , _T( "Status" ) ) ;
    PutFrame( m_pOutput , pReport ) ;
  }
  ProcessingStage SetProcessingStage(
    ProcessingStage NewStage, bool bReport = false,
    LPCTSTR pAddition = NULL, bool bDontAllert = false);
  bool CheckAndAddSegment(BorderSegment& Seg,
    const CDPoint * pPt, Edge CurrEdge);
  ProcessingStage CheckOperationStack(bool bStopIfEmpty = true);
  void SetMotionSpeed(double dSpeed_um_per_sec);
  void SetMotionAcceleration(int iAccInUnits, int iAxesMask = -1);
  void StopMotion(int iAxesMask = -1);
  void SetZMotionSpeed(double dSpeed_um_per_sec);
  bool MoveToTarget(cmplx& cTargetXY_um, double& dTargetZ_um,
    double dXYSpeed_um_sec = 0.) ;
  bool MoveToTarget(CCoordinate& Target);
  void OrderImagingTaskAndLight( int iTaskNum , DWORD dwCamera ,
    DWORD dwLedMask , int iFlashTime_us )
  {
    SetTask( iTaskNum ) ;
    SetWorkingLeds( dwCamera , dwLedMask , iFlashTime_us ) ;
  }
  bool IsValidGradValue( double dWidth_um )
  {
    return ((m_dMinGradWidth_um <= dWidth_um)
      && (dWidth_um <= m_dMaxGradWidth_um)) ;
  }
  DWORD GetMarkColor( double dValue , double dMaxValue , double dMinValue ) ;
//   ActiveSegments * GetInformationalSegments(
//     const CFigureFrame& Contur, CRect& ROI , 
//     int& iMinIndex , double& dMinDist );
  double CorrectAngle( double dAngle ) ;
  
  void ConturAsm::RequestCurrentPosition();
  int OrderRelMotion(CCoordinate& Directions);
  int OrderAbsMotion( CCoordinate& Target ) ;
  int OrderAbsMotion( cmplx& Target ) ;
  int OrderRelZMotion( double dDist , 
    bool dReport = true , double dCheckDeviation = 0.0 ) ;
  int OrderAbsZMotion( double dDist , bool dReport = true ) ;
  int OrderRelMotion( double& dDX , double& dDY , double& dDZ ) ;
  bool IsOnTargetPosition( double dTolerance = 1.5 ) ;
  int FindBurrByAvgAndStd( const CVideoFrame * pVF ,
    cmplx * pEdgeData, int iEdgeDataLen, int iPt1Index,
    int iNPtsOnEdge, int iStep , int& iNGood ,
    CFigureFrame * pExternal , CFigureFrame * pInternal , 
    CFigureFrame * pAdditional , CFigureFrame * pDirMarking = NULL );

  int FindFocusAndBurrByAvg( const CVideoFrame * pVF ,
    cmplx * pEdgeData , int iEdgeDataLen , int iPt1Index ,
    int iNPtsOnEdge , int iStep , int& iNGood ,
    CFigureFrame * pExternal , CFigureFrame * pInternal ,
    CFigureFrame * pAdditional , CContainerFrame * pMainOut );
    
    // Return is true, if not necessary to continue focusing (i.e. ~all 
    // segments were in focus
  bool AnalyzeLastMeasByAvg( int& iNGood , // N good segments in this measurement
    int& iFirstGoodIndex , // index of first good segment in this measurement
    int& iLastGoodIndex ) ;// index of last good segment in this measurement

  int FindWallEdges( const CVideoFrame * pVF ,
    cmplx * pEdgeData, int iEdgeDataLen, int iPt1Index,
    int iNPtsOnEdge, double dRealLightDirForExternal ,
    CFigureFrame * pExternal , CFigureFrame * pInternal , 
    CFigureFrame * pShadow , CFigureFrame * pDirMarking = NULL );

  int Find3Edges(double * pSignal, int iSignalLength, double d3edges[3]);
  // Calculates lights directions for current edge position
  Directions CalcLightDirections( cmplx& cPt1 , cmplx& cPt2 ,
    double& dInternalLightDir , double& dExternalLightDir  , 
    double& dRealLightDirForExternal ,
    CContainerFrame* pGraphics = NULL );
  CRect GetFocusingArea( const CFigureFrame * pFigure , cmplx cCenter ) ;
  CRect GetFocusingArea( const CFigureFrame * pFigure ) ;
  CRect GetFocusingArea( const CmplxArray& Extrems , Edge WhatExtreme ) ;
  WhatFigureFound SetDataAboutContour( const CFigureFrame * pFrame ) ;
  int SetDataAboutContours( CFramesIterator * pFiguresIterator ) ;
  cmplx FindExtremePoint( const CDataFrame * pDataFrame ,
    LPCTSTR pFigNamePart , Edge WhatExtreme ) ;

  bool InitMeasurement();
  bool SetFocusingArea( int iXOffset , int iYOffset , 
    int iWidth , int iHeight , int iWorkingMode );
  CCoordinate GetAbsPosOnObservation( cmplx ObservPt );
  // This function returns SHIFT from point on observation
  // to FOV center on measurement 
  // Z coordinate will be taken from current 
  // motion position (do correct if necessary)
  CCoordinate GetTargetForObservationPoint( cmplx ObservPt );
  cmplx GetDistanceOnObservation( cmplx Pt1 , cmplx Pt2 ); // vect from pt1 to pt2
  bool GetTargetForMapPoint( CPoint MapPt , CCoordinate& Target ,
    int& iNearestIndex ) ;
  size_t SaveScanResults( LPCTSTR pFileName = NULL );
  // Form image for 3d presentation
  CVideoFrame *FormImageFor3dView( const CVideoFrame * pImage );
  // Form image for Volume presentation
  CDataFrame *FormMapImage( const CVideoFrame * pImage , 
    ResultViewMode Mode = RVM_BurrVolume , int iViewIndex = -1 ,
    int iPercent = -1 ) ;

  cmplx FormMarker(int iIndexInConturData, DWORD dwColor, 
    bool bViewValue , CContainerFrame * pMarking , int iSaveEnds = 1 );
  bool FormScaleView( 
    CVideoFrame * pTarget , CRect Placement , double dMax , double dMin = 10. ) ;
  bool ProcessMsgFromMap( const CTextFrame * pMsgFromMap );
  bool ProcessMsgFromUI( const CTextFrame * pMsgFromMap );
  int ProcessLightCalibration( const CDataFrame * pDataFrame = NULL ) ;
  bool GetDataForIndex( ResultViewMode Mode , int iIndex ,
    double& dValue ) ;
  LPCTSTR GetScaleCaption( ResultViewMode Mode ) ;
  int GetFocusValueForPt( 
    const CVideoFrame * pVF , cmplx& Pt , int& iMaxFocusValue ) ;
  int GetMaxDiffAndMaxValForPt(
    const CVideoFrame * pVF , cmplx& Pt , int& iMaxDiffVal , int& iMaxVal) ;

  // Function does test for focus in the beginning of working segment
  FocusState ConturAsm::CheckPtForFocusAndGetDirection(
    double& dNextStep , int iCheckPointIndex ,
    int& iLastGoodIndex ) ;
  // Function looking for points in Data with bad focus and returns first with minimum 3 following points with bad focus
  int GetLastIndexWithGoodFocus( MeasuredValues& Data , int iFirstIndex );
  // Function compare measured focuses for different Z for assigned point and provide Z step value for focusing
  double GetZStepForLocalPoint( int iPtNumber );
  bool FocusLogAndMoveZ( LPCTSTR pMsg , double dZ = 0. , 
    CDataFrame ** pFrameForMarkingOutput = NULL );
  double GetZStepFromHistory() ;
  double GetOptimalZForIndex( int iIndexForPosition , int iAvFactor = 3 );
  int GetIndexWithOptimalFocus( int iIndexForPosition , int iAvFactor = 3 ) ;
  double GetFocusingStepForFarFromFocus();
  bool PutAndRegisterFrame( COutputConnector * pConnector , const CDataFrame * pFrame );
  int FormReportStringForUI(FXString& Result);
  FXString CheckCreateDataDir();
  FXString CheckCreateDataSubDir( LPCTSTR pszSubdir );
  FXString CheckCreatePartResultDir();
  void SaveFormsFileName(LPCTSTR pFileName);
  bool SaveImageFragment( const CVideoFrame * pVf , CRect& Fragment , 
    FXString& FilePath );
  CVideoFrame * GetImageFromFile( FXString& FilePath , CRect& Fragment ) ;
  int GetFirstSampleInSegment( int iIndexInSegment ) ;
  CDataFrame * FormFrameForSavedImageView( int iPtNumInContur );
  bool LoadResults( LPCTSTR pFileName , LPCTSTR pDataDir = NULL );
  int AddNewSampleToContur( OneMeasPt& Pt ,
    int iIndex , int iSegmentNumber , FXString * pForReporting = NULL );
  // Check Last results (m_CurrentViewMeasStatus) for spikes and remove them
  int FilterLastResults();
  CContainerFrame * CreateCombinedResultContainer( 
    const CVideoFrame * pOptimalImage , 
    const CDataFrame * pInputDataFrame );
  int ViewSavedObservationImage();
  CVideoFrame *  GetSavedImage( LPCTSTR pFileNameFragmentWithExt , LPCTSTR pExt = NULL );
  double GetAverageOfSegmentForPt( int iPtIndex );
  int MarkSector( int iPtIndex , cmplx cPartCEnter , double SectorLength_um );
  FXString CheckSendFoundPartInfoToYuri( LPCTSTR psPartId = NULL , 
    int iHeight_um = 0 , LPCTSTR psPartDir = NULL , 
    LPCTSTR psErr = NULL , bool bIsNewPart = false );
  CTextFrame * FormScanFinishMessage( LPCTSTR pLabel , bool bFromFile = false );
  int CheckObservationContur( CContainerFrame * pOutData , 
    const CDataFrame * pOrigin , bool bScanOperation );

 bool GetCurrentDetailedForm();
 cmplx GetPtPosInRobotCoords( OneMeasPt& Pt ) ;

 int CombineImagesAndPutOverlay( 
   pTVFrame pFrame1 , pTVFrame pFrame2 , Directions iOrient , 
   GraphicsData& Overlay , bool bInvertBW = false , bool bPrint = false ,
   const pTVFrame pOverviewFrame = NULL );
 int CalcAveragesForRegions( AverData& Result );
 int SetAveragedValuesToOverlay( GraphicsData& Overlay );
 int PrepareDataForCombinedMap( CDataFrame * pMapView , bool bPrint = false );
 bool ResetViews();
 int DrawMouseModeSelectMenu( CContainerFrame * pOutData , CPoint LeftTopPt );
 MapMouseClickMode IsNewModeSelected( CPoint MouseCoords );
 int ChangeAverageRange( bool bPlus );
 double GetAverageAroundPt( int iPtNum , double dRange_um ,
   CPoint& PtOnMapBackward , CPoint& PtOnMapForward );
 bool SendEdgeAngleToTVObject(double dAngle_rad);
 // Calculate laplacian, check previous images, do Z step, if necessary and return true, stop process, if image is in focus
 bool ContinueFocusing(const CVideoFrame * pImage);
 int InitIterativeFocusing( ProcessingStage StageAfterFocusing = Stage_Inactive , 
   bool bGrabAndSetStage = true );
 double GetBestZForEndAndSlope(double& dAvOnEnd, double& dOtherZ, double& dAvForOtherZ);
 double SetPredictedEdgeAngle();
 bool InitDirectoriesForSimulation(); 
 int OrderNextPartForSimulation( int iResultForLog ); // 1 - success, 0 - failed , -1 - no log (first call)
};

enum ProcessState
{
  PS_Inactive = 0 ,
  PS_SendToBegin ,
  PS_InScan ,
  PS_ScanFinished ,
  PS_Finished 
};

class FindExtrem : public CFilterGadget , DecodeZaberMotion
{
  CInputConnector * m_pFromMotionInput ;
  COutputConnector * m_pToMotion ;
  COutputConnector * m_pToLightControl ;
  ProcessState      m_State ;
  int               m_iControlAxis ;
  int	              m_Count;
  double            m_dBegin ;
  double            m_dStep ;
  double            m_dEnd ;
  double            m_dScanSpeed_umPerSec ;
  double            m_dCorrection_um ;
  double            m_dTargetForScan ;
  bool              m_bRelative ;
  double            m_dLastValue ;
  double            m_dScanBeginTime ;
  double            m_dLastValueTime ;
  double            m_dLastCoordTime ;
  double            m_dLastStartTime ;
  double            m_dFoundFocusCoord ;
  double            m_dOptimalValue ;
  int               m_iIntMask ;
  BOOL              m_bIntFormat ;
  BOOL              m_bLoop ;
  bool              m_bWasReset ;
  bool              m_bWasInRange ;
//  CCoordinate       m_CurrentPos ;
  Samples           m_Samples ;
#ifdef _DEBUG
  double            m_DebugSamples[ 500 ] ;
#endif
  FXString          m_OutFormat ;
  FXString          m_IDPrefix ;
  FXString          m_CoordPrefix ;
  FXString          m_LightCommand ;
  FXLockObject      m_InputLock;
  FXString          m_GadgetInfo ;
protected:
  FindExtrem();
public:
  virtual void ShutDown();

  virtual bool PrintProperties( FXString& text );
  virtual bool ScanProperties( LPCTSTR text , bool& Invalidate );
  virtual bool ScanSettings( FXString& text );
  virtual int GetInputsCount() { return 2 ; } ;
  virtual CInputConnector * GetInputConnector( int n )
  {
    if ( n == 0 )
      return m_pInput ;
    if ( n == 1 )
      return m_pFromMotionInput ;
    return NULL ;
  }
  virtual int GetOutputsCount() { return 3 ; } ;
  virtual COutputConnector * GetOutputConnector( int n )
  {
    if ( n == 0 )
      return m_pOutput ;
    if ( n == 1 )
      return m_pToMotion ;
    if ( n == 2 )
      return m_pToLightControl;
    
    return NULL ;
  }
  LPCTSTR GetGadgetInfo() { return ( LPCTSTR ) m_GadgetInfo ; } ;
private:
  static friend void CALLBACK _fn_MotionStatus( CDataFrame* pDataFrame ,
    void* pGadget , CConnector* lpInput )
  {
    FindExtrem * pFindExtrem = ( ( FindExtrem* ) pGadget ) ;
    CTextFrame * pText = pDataFrame->GetTextFrame() ;
    if ( pText && pFindExtrem )
    {
      FXString Msg = pText->GetString() ;
      if ( pFindExtrem->DecodeMotionStatus( Msg ) )
        pFindExtrem->CheckAndProcessLastMotion() ;
    }
    pDataFrame->Release() ;
  }
  virtual CDataFrame* DoProcessing( const CDataFrame* pDataFrame ) ;

  void CheckAndProcessLastMotion()
  {
    if ( m_State == PS_Finished )
    {
      CQuantityFrame * pFocusData = CreateQuantityFrame(
        m_dOptimalValue , _T( "DiffSum" ) ) ;
      PutFrame( m_pOutput , pFocusData ) ;
      //CTextFrame * pLightCommand = CreateTextFrame( m_LightCommand , (LPCTSTR) NULL ) ;
      //PutFrame( m_pToLightControl , pLightCommand ) ;
      Sleep( 200 ) ;
      CQuantityFrame * pResult = CreateQuantityFrame(
        m_dFoundFocusCoord , _T( "FocusFound" ) ) ;
      PutFrame( m_pOutput , pResult ) ;
      m_State = PS_Inactive ;
    }
  }
public:
  int DecodeMotionStatus( FXString& Msg );

  DECLARE_RUNTIME_GADGET( FindExtrem );
};

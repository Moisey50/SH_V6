#pragma once
#include <fxfc\fxfc.h>
#include "helpers\UserBaseGadget.h"
#include "gadgets\videoframe.h"
#include "gadgets\QuantityFrame.h"
#include <gadgets\FigureFrame.h>
#include <Math\PlaneGeometry.h>
#include <helpers\FramesHelper.h>
#include <Math\FRegression.h>
#include <Math/FigureProcessing.h>
#include <imageproc/ExtractVObjectResult.h>
#include "Tecto_Polyline.h"
#include "Tecto_DetectFlowerComponents.h"
#include "Tecto_ProcessPlantImage.h"
#include "Tecto_R2.h"


enum TectoGadgetMode
{
  TGM_SIDE = 0 ,
  TGM_FRONT
};

enum TectoProcessingMode
{
  TPM_Unknown = 0 ,
  TPM_SIDE_Processing = 1 ,
  TPM_SIDE_Calibration 
};

enum TectoImageViewMode
{
  TIVM_Original = 0 ,
  TIVM_Processed ,
  TIVM_Binaryzed 
};

#define SliceStep (10)

class Tecto :
  public UserBaseGadget
{
public:
  Tecto();
  ~Tecto();

  //   void ShutDown();
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );

  void PropertiesRegistration();
  void ConnectorsRegistration();
  bool ScanProperties( LPCTSTR text , bool& Invalidate ) ;
  bool FormResult( CContainerFrame * pMarking ) ;
  bool DoCalibration( CContainerFrame * pMarking ) ;
  void ProcessHistogram( CRect * pRC = NULL ) ;
  void Logger( LPCTSTR pString ) ;
  static void GlobalLogger( LPCTSTR pString ) ;
  static void ConfigParamChange( LPCTSTR pName , void* pObject ,
    bool& bInvalidate , bool& bInitRescan ) ;
  LPCTSTR GetTectoGadgetModeName() ;
  int RestoreFlower( LPCTSTR pFlowerName = NULL );
  int SaveToRegistry( LPCTSTR pFlowerName = NULL );
  double GetStalkDia( CmplxRect& Boundary , CFigureFrame * pMainContur , 
    CVideoFrame * pImageForResult , CContainerFrame * pOut , int& iMinX );
  double GetStemDia( CmplxRect& Boundary , CFigureFrame * pMainContur , 
    CVideoFrame * pImageForResult , CContainerFrame * pOut );
  size_t TrackHContrastLine(
    cmplx cInitCent , int iStripRange ,
    CmplxVector& ScanCenters , double dNormThres , int iMinAmpl ,
    const CVideoFrame * pVF , double& dAvLineWidth_pix ,
    CContainerFrame* pMarking = NULL , int iScanWidth_pix = 0 ) ;// 0 - no restriction

  // function returns point on stem for measurements, 
  // if abs of this point is zero - bad measurement
  cmplx CheckAndScanStem(
    CVideoFrame * pImageForResult , CFigureFrame * pMainContur ,
    int& iLeftPtIndex , double dXBegin , double dXLim ,
    bool bScanToTheRight , CContainerFrame * pOut );
  cmplx Tecto::FindFirstPtForMeasurement(
    CVideoFrame * pImageForResult , CFigureFrame * pMainContur ,
    int iLeftPtIndex , double dXBegin , double dXLim , 
    double& dInitialDia_pix , CContainerFrame * pOut ) ;
  cmplx TrackLeft(
    CVideoFrame * pImageForResult , cmplx cInitPoint ,
    double dInitialWidth_pix , CContainerFrame * pOut ) ;
  bool ViewHistogram( CContainerFrame * pMarking ) ;
  TectoGadgetMode m_GadgetMode ;
  TectoProcessingMode m_ProcessingMode ;
  FXString  m_FlowerName ;
  double   m_dScale_mm_per_pix ;
  TectoImageViewMode      m_iViewMode ;
  int      m_iDebugView = 5 ;

  CRect    m_LastROI ;
  cmplx    m_cLastFOV_pix ;
  cmplx    m_cLastFOVCenter_pix ;
  SpotVector m_LastSpots ;
  NamedCDRects m_LastROIs ;
  double   m_dCalibLength_mm ;

// Measurements and out data enable
  BOOL     m_bLengthMeasurement ;
  BOOL     m_bXRightOut ;
  BOOL     m_bWidthMeasurement ;
  BOOL     m_bAreaMeasurement ;
  BOOL     m_bMeasureStalk ;
  BOOL     m_bMeasureCentralStem ;

  double   m_dLastLength_mm ;
  double   m_dMinimalLength_mm  ;
  double   m_dMinimalLength_pix ;
  double   m_dMaximalLength_mm  ;
  double   m_dLastXRight_mm ;

  double   m_dLastWidth_mm ;
  double   m_dMinimalWidth_mm ;
  double   m_dMaximalWidth_mm ;

  double   m_dLastFlowerArea_cm2 ;
  double   m_dAltFlowerArea_cm2 ;
  double   m_dMinimalFlowerArea_cm2  ;
  double   m_dMaximalFlowerArea_cm2  ;

  double   m_dStalkAndStemCorrection_pix ;
  double   m_dLastStalkThickness_mm ;
  double   m_dLastSTalkDiaStd_mm ;
  double   m_dStalkDiaMin_mm ;
  double   m_dStalkDiaMax_mm ;
  int      m_iStalkMeasDist_pix = 16 ;
  double   m_dStalkThreshold = 0.6 ;
  double   m_dStalkShiftRight_mm = 0. ;

  double  m_dStemWidthStart_perc ;
  double  m_dStemWidthEnd_perc ;
  double  m_dLastStemDia_mm ;
  double  m_dStemDiaMin_mm ;
  double  m_dStemDiaMax_mm ;
  CmplxVector m_UpperByContur ;
  CmplxVector m_LowerByContur ;
  DoubleVector m_Diameters ;

  double m_dMachingSize = 10. ;
  double m_dXAxeCompressCoeff = 0.2;
  double m_dRelationDistance = 5.0;
  double m_dSlicedPartForStalk = 0.1 ; 

  int    m_iNBackCheckedOnRightScan = 5 ;
  DWORD  m_uiNOriginalFrames = 0 ;


  double   m_dLastStartTime_ms ;
  double   m_dLastProcessingTime_ms ;
  double   m_dLastGadgetProcessingTime_ms ;
  double   m_dHistoCutLevelForEmptyTray = 0.005 ;
  int      m_iFoundCutLevel ;
  int      m_LastHistoGram[ 256 ] ;
  DWORD    m_HistoId = 0 ;
  double   m_dHistoTime ;
  DWORD    m_dwMemOnEntry ;
  DWORD    m_dwMemOnExit ;
  int      m_iHistoMinIndex ;
  int      m_iHistoMaxIndex ;
  int      m_iMaxThreshold ;
  double   m_dHistoPerc = 0.1 ;
  double   m_dOutBinaryThres = 0.3 ;
  int      m_iResultOnScreenFontSize ;
  cmplx    m_cRectOnLineSize_mm ;
  cmplx    m_cRectOnLineTol_mm ;

  bool     m_bTrayIsEmpty ;

  BOOL     m_bSaveToRegistry ;

  bool     m_bProcessDisable = false ;
  const CVideoFrame * m_pLastOriginalVideoFrame ;

  FXString m_AdditionalInfo ;

  // Names for communications with graph
  FXString m_GraphName ;
  FXString m_CameraName ;
  FXString m_ImagingName ;
  FXString m_StraightSearchName ;
  FXString m_CalculatorName ;
  FXString m_RenderName ;
  FXString m_SaveResultDirectory ;

  DECLARE_RUNTIME_GADGET( Tecto );
};


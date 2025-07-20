// UserExampleGadget.h : Declaration of the UserExampleGadget class



#ifndef __INCLUDE__RandomNumber_H__
#define __INCLUDE__RandomNumber_H__


#pragma once
#include "helpers\UserBaseGadget.h"
#include "gadgets/ContainerFrame.h"
#include <helpers\FramesHelper.h>
#include <iostream>
#include <iomanip>
#include <map>
#include <random>
#include <cmath>
#include <math\intf_sup.h>
#include <helpers\PropertyKitEx.h>
// #include "math\random.h"

enum ISIM_WorkingMode
{
  ISIMWM_NotSet = 0 ,
  ISIMWM_DispensSide ,
  ISIMWM_DispensFront ,
  ISIMWM_Pin
} ;

enum DispenserProcessingStage
{
  DPS_Inactive = 0 ,
  DPS_ProcessStart ,
  DPS_PartLoaded  ,
  DPS_FirstCentering ,
  DPS_FirstCenteringFinished ,
  DPS_SideGrinding ,
  DPS_SidePrepareToFinish ,
  DPS_SideExtSizeOK ,
  DPS_SecondCentering ,
  DPS_FrontGrinding ,
  DPS_FrontGrindingFinished ,
  DPS_FrontWhiteMeasurement ,
  DPS_FrontBlackMeasurement 
};

enum AutoProcessing
{
  AP_Off = 0 ,
  AP_On
};


#define BLACK_CENTER_INTENS 20
#define FRONT_INTENS 70
#define FRONT_INTENS_STD_DEV 10
#define EDGE_MEAN_INTENS 180.
#define EDGE_INTENS_STDDEV 60.
#define AVERAGE_CONUS     150
#define ANGLE_NON_UNIFORMITY 20
#define RADIAL_NON_UNIFORMITY 50


class ImageSimulator : public UserBaseGadget
{
protected:
  ISIM_WorkingMode m_WorkingMode ;
  FXString         m_sImageSize_pix ;
  CSize            m_ImageSize_pix ;
  static FXString    m_sPartName ;


	ImageSimulator();
  virtual ~ImageSimulator() ;

public:

	//	Dispensers simulation
  static __int64 m_GenerationStage ;
  static AutoProcessing m_AutoProc ;
  static double m_dStateChangeTime_ms ;
  
  static double		m_dExtTargetRadius_um ;
  static double   m_dExtCurrentRadius_um ;
  static double		m_dIntRadius_um ;
  static double		m_dIntCurrentRadius_um ;
  static double    m_dSideAngle_deg ;
  static double    m_dInternalAngle_deg ;
  double		m_dExtBaseRadius_um ;
  double    m_dDepth_um ;
  double    m_dSideStoneSwipeUp_um ;
  double    m_dSideStoneSwipeDown_um ;
  double    m_dSideStonePos_um ;
  double    m_dSideSwipeTime_turns ;
  double    m_dSideSwipePhase_rad ;
  double    m_dSideSwipePhaseStep_rad ;
  double    m_dSideStoneSpeed_um_per_turn = 20. ;
  double    m_dFrontStonePos_um ;
  double    m_dFrontSwipeTime_turns ;
  double    m_dFrontSwipePhase_rad ;
  double    m_dFrontSwipePhaseStep_rad ;
  double    m_dFrontStoneSpeed_um_per_cycle = 1. ;
  double    m_dSideScale_um_per_pix ;
  double    m_dFrontScale_um_per_pix ;
  static cmplx   m_cExtEccentricity_um ;
  static cmplx   m_cIntEccentricity_um ;
  static cmplx   m_cInitialExtEccentricity_um ;
  static cmplx   m_cInitialIntEccentricity_um ;
  FXString  m_sExtEccentricity_um ;
  FXString  m_sIntEccentricity_um ;

  double    m_dPinDia_um = 300. ;
  double    m_dHeadRadius_um = 1000. ;
  double    m_dPinLength_um = 1000. ;
  double    m_dRelLeftSide = 0.2 ;
  int       m_iBasementAngle = 0 ;
  

  int       m_iNframesPerRevol = 4 ;

  cmplx     m_cConstantShift_um = 0. ;
  double    m_dTurnAngle_rad ;
  double    m_dTurnAngleStep_rad ;
  cmplx     m_cCenter_pix ;
  cmplx     m_cROI_pix ;
  double    m_dSideAngle_rad ;
  double    m_dInternalAngle_rad ;
  double    m_dLastSwithToSideFinishedTime_ms = 0. ;
  double    m_dLastFrontGrindingBeginTime_ms = 0. ;
  int       m_iNFrontGrindingCycles = 0 ;

  int         iHRCounter;
  bool        m_bReset ;
  int         m_iSavePart = 0 ;
  int         m_iLoadPart = 0 ;
  LPBYTE      m_pFrontBackground = NULL ;
  BYTE        m_EdgePixels[ 3000 ] ;
  LPBYTE      m_pCurrentPixel = m_EdgePixels ;
  LPBYTE      m_pEdgePixelsEnd = m_EdgePixels + sizeof( m_EdgePixels ) ;
  BYTE        m_FrontPixels[ 12981 ] ;
  LPBYTE      m_pCurrentFrontPix = m_FrontPixels ;
  LPBYTE      m_pFrontPixelsEnd = m_FrontPixels + sizeof( m_FrontPixels ) ;

  cmplx       m_cProcessViewPt ;
  cmplx       m_cViewPt ;
  int         m_iNFrames = 0 ;

  FXLockObject     m_Protect ;
	//	Mandatory functions

	void PropertiesRegistration();
	void ConnectorsRegistration();

  static void ConfigParamChange( LPCTSTR pName , void* pObject , bool& bInvalidate , bool& bInitRescan ) ;
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
//   virtual bool ScanProperties(LPCTSTR text, bool& Invalidate) ;
//   virtual bool PrintProperties(FXString& text) ;
  int SavePart() ;
  int SelectPartByName( FXString& SelectedPartName , bool bReportToEngine ) ;

  int SimulateFront( CVideoFrame * pFrame , CContainerFrame * pResult );
  int SimulateSide( CVideoFrame * pFrame , CContainerFrame * pResult );

  int SimulateDispenserFront( CVideoFrame * pFrame , CContainerFrame * pResult );
  int SimulateDispenserSide( CVideoFrame * pFrame , CContainerFrame * pResult );
  int SimulatePinSide( CVideoFrame * pFrame , CContainerFrame * pResult ) ;

  DECLARE_RUNTIME_GADGET(ImageSimulator);
  int DrawSideConus( CVideoFrame * pFrame , cmplx& cOrigin_pix );
  int DrawFullBlank( CVideoFrame * pFrame );
  int DrawCircle( CVideoFrame * pVF , cmplx& cCenter_pix , double dRadius_pix , 
    BYTE bIntens , BYTE * pEdgePixels = NULL , BYTE * pFrontPixels = NULL );
  int CreateFrontImageBackground( LPBYTE pBuffer , CSize ROI );
  int FillConstFromCenterToEdge( LPBYTE pBuffer , CSize ROI , cmplx cEdgePt , BYTE Intens ) ;

  BYTE GetNextEdgePixel() 
  {
    if ( ++m_pCurrentPixel >= m_pEdgePixelsEnd )
      m_pCurrentPixel = m_EdgePixels ;
    return *m_pCurrentPixel ;
  }
  BYTE GetNextFrontPixel()
  {
    if ( ++m_pCurrentFrontPix >= m_pFrontPixelsEnd )
      m_pCurrentFrontPix = m_FrontPixels ;
    return *m_pCurrentFrontPix ;
  }

};

#endif	// __INCLUDE__UserExampleGadget_H__


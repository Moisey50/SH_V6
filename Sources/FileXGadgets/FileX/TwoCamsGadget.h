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

enum WM_Green
{
  WM_Green_Unknonw = 0 ,
  WM_Green_Measure ,
  WM_Green_Lock ,
  WM_Green_Manual
};

enum AppMode
{
  AppMode_Green_Machine = 0 ,
  AppMode_Holes_Erosion
};

enum GreenGadgetMode
{
  GM_SIDE = 0 ,
  GM_FRONT ,
};

enum HolesSideALgorithm
{
  HSA_ByLowerPoint = 0 ,
  HSA_OrthoToBase
};

enum BaseGreenMethod
{
  BGM_Sraights = 0 , // find straights approximation
  BGM_Corners        // find corners near center
};
class TwoChans :
  public UserBaseGadget
{
public:
  TwoChans();
  ~TwoChans();

  //   void ShutDown();
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );

  void PropertiesRegistration();
  void ConnectorsRegistration();
  virtual void PrintAdditionalData( FXString& Text ) ;

  AppMode         m_ApplicationMode ;
  GreenGadgetMode m_GadgetMode ;
  WM_Green m_WorkingMode ;
  BaseGreenMethod m_BGMforBasePoint ;
  HolesSideALgorithm m_SideAlgorithm ;

  CRect    m_LastROI ;
  cmplx    m_cLastROICent_pix ;
  cmplx    m_cLastLocked ;
  double   m_dScale_um_per_pix ;
  double   m_dTolerance_um ;

  // Green machine parameters
  cmplx    m_cLastRightPtLocked ;
  cmplx    m_cLastTipCenter ;
  cmplx    m_cLastRightPt ;
  int      m_iVertLinePosition_pix = 600 ;
  int      m_iHorLinePosition_pix = 900 ;
  int      m_iZOffset_um ;
  int      m_iYOffset_um ;
  int      m_iXOffset_um ;
  double   m_dX_um ;
  double   m_dY_um ;
  double   m_dZ_um ;
  double   m_dLineAngle ;

  // Holes in wedges parameters
  double   m_dWedgeAngleToElectrode_deg ;
//   double   m_dElectrodeToCornerHorizDist_um ;
  int      m_iShiftFromCorner_Tx10 ;
  double   m_dElectrodeToCornerHorizDist_tenth ;
  int   m_iVertLinesDist_tenth ;
  int   m_iMoveFragmentFrom_pix = -1 ;
  int   m_iMoveFragmentTo_pix = 400 ;
  int   m_iFragmentHeight_pix = 100 ;

  int      m_iWindowWidth = 10 ;
  cmplx    m_cAvLeftCorner ;
  int      m_iLeftCornerCntr = 0 ;
  cmplx    m_cAvTipCorner ;
  int      m_iTipCornerCntr = 0 ;
  cmplx    m_cAvBottomCorner ;
  int      m_iBottomCornerCntr = 0 ;
  cmplx    m_cAvBottomRightCorner ;
  int      m_iBottomRightCornerCntr = 0 ;
  cmplx    m_cAvElectrodeTarget ;
  int      m_iElectrodeTargetCntr = 0 ;
  double   m_dShutDownProcessingTime = 0 ;
  CmplxVector m_LastLeftCorners ;

  StraightLines m_LastLines ;

  // Names for communications with graph
  FXString m_GraphName ;
  FXString m_CameraName ;
  FXString m_ImagingName ;
  FXString m_StraightSearchName ;
  FXString m_CalculatorName ;
  FXString m_RenderName ;
  bool GetBasePoint( const CFigureFrame * pInFig , 
    CmplxVector& Crosses , CRect& WorkingZone , FXSIZE iMaxYIndex , 
    CContainerFrame * pMarking );
  void ProcessForGreenMachine( const CDataFrame * pDataFrame , CContainerFrame * pMarking ) ;
  void ProcessForHolesInWedges( const CDataFrame * pDataFrame , CContainerFrame * pMarking ) ;
  void ProcessWedgeSide( const CDataFrame * pDataFrame , CContainerFrame * pMarking ) ;
  void ProcessWedgeFront( const CDataFrame * pDataFrame , CContainerFrame * pMarking ) ;

  DECLARE_RUNTIME_GADGET( TwoChans );
};


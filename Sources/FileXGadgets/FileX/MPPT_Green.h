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

enum BaseGreenMethod
{
  BGM_Sraights = 0 , // find straights approximation
  BGM_Corners        // find corners near center
};
class MPPT_Green :
  public UserBaseGadget
{
public:
  MPPT_Green();
  ~MPPT_Green();

  //   void ShutDown();
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );

  void PropertiesRegistration();
  void ConnectorsRegistration();
  virtual void PrintAdditionalData( FXString& Text ) ;

  AppMode         m_ApplicationMode ;
  GreenGadgetMode m_GadgetMode ;
  WM_Green m_WorkingMode ;
  BaseGreenMethod m_BGMforBasePoint ;

  CRect    m_LastROI ;
  cmplx    m_cLastLocked ;
  cmplx    m_cLastRightPtLocked ;
  cmplx    m_cLastTipCenter ;
  cmplx    m_cLastRightPt ;
  double   m_dScale ;
  double   m_dTolerance_um ;
  int      m_iHorLinePosition_pix ;
  int      m_iZOffset_um ;
  int      m_iYOffset_um ;
  int      m_iXOffset_um ;
  double   m_dX_um ;
  double   m_dY_um ;
  double   m_dZ_um ;
  double   m_dLineAngle ;

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

  DECLARE_RUNTIME_GADGET( MPPT_Green );
};


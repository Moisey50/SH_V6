// SpotStatistics.h : Declaration of the SpotStatistics class

#ifndef __INCLUDE__SpotStatistics_H__
#define __INCLUDE__SpotStatistics_H__

#pragma once
#include "helpers\UserBaseGadget.h"
#include "gadgets\QuantityFrame.h"
#include <imageproc\clusters\segmentation.h>
#include "helpers\FramesHelper.h"
#include "imageproc\ExtractVObjectResult.h"
#include "SpotsDataProcessing.h"
#include "Math\FRegression.h"
#include "LinesAsSpotsProcessing.h"

#ifndef UINT8
#define UINT8 unsigned char
#endif

#ifndef UINT16_MAX
#define UINT16_MAX 0xffff
#define UINT8_MAX  0xff
#endif

enum SSAlgorithm
{
  NoControlAndProcessing ,
  NoProcessing ,
  HLinesAsSpots ,
  VLinesAsSpots ,
  SmallDots
};

enum SS_OutputNum
{
  SSON_WithImages = 0 ,
  SSON_Statistics  ,
  SSON_TaskSelector ,
  SSON_ScalingView ,
  SSON_ResultView
};

class SpotStatistics : public UserBaseGadget
{
private:
  FXString&			StatisticsForLinesAsSpots(
    const CDataFrame * pSrc, CContainerFrame * pMarking);
  
  FXString      m_LineAreaIDPrefix ;
  FXString      m_IDtoTaskCorrespondence ;
  FXString      m_Result;
  SpotVectors   m_SeparatedResults;
  NamedCmplxVectors m_Profiles;
  FXString      m_LastTypeString ;
  double        m_dDistBetweenSquares_um ;
  cmplx         m_cLastScaleViewPoint ;

  bool          m_bUseIndexes = true ;
  IntVector     m_HLineIndexes ;
  IntVector     m_VLineIndexes ;
  IntVector     m_DotIndexes ;
  IntVector     m_OtherIndexes ;
  int           m_iNMeasurements = 0 ;

protected:

	SpotStatistics();

  SSAlgorithm m_iAlgorithm = NoProcessing ;
  SpotsDataProcessing m_SpotDataProcessing ;
  LinesAsSpotsProcessing m_LinesAsSpotsProcessing;
  NamedCDRects m_ROIs ;


public:
  void ShutDown() 
  {
    UserGadgetBase::ShutDown() ;
  }

	void PropertiesRegistration();
  static void ConfigParamChange( LPCTSTR pName , void* pObject , bool& bInvalidate , bool& bInitRescan ) ;
  void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
  bool CalculateScales( const CDataFrame * pDataFrame , CContainerFrame * pMarking );
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
  bool RestoreIndexes() ; // true - indexes used

	DECLARE_RUNTIME_GADGET(SpotStatistics);

};

#endif	// __INCLUDE__SpotStatistics_H__


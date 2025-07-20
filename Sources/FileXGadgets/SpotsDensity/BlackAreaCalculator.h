// BlackArea.h : Declaration of the BlackArea class

#ifndef __INCLUDE__BlackAreaCalculator_H__
#define __INCLUDE__BlackAreaCalculator_H__

#pragma once
#include "helpers\UserBaseGadget.h"
#include "gadgets\videoframe.h"
#include "gadgets\QuantityFrame.h"

#ifndef UINT8
#define UINT8 unsigned char
#endif

#ifndef UINT16_MAX
#define UINT16_MAX 0xffff
#define UINT8_MAX  0xff
#endif


enum Algorithm
{
  HistAnalyze ,
  FixedThresholdRelativelyToHistDensityOnEdges ,
  FixedThresholdRelativePresettledBlackWhite
};
class BlackArea : public UserBaseGadget
{
private:

	unsigned int	defineTresholdByValue		(
    LPBYTE pProcessingPicture, unsigned int uiProcessingPictureSize ,
    int iPixelSize = 1 );
  void			calculateHistogramArray		(
    LPBYTE pProcessingPicture, unsigned int uiProcessingPictureSize, 
    double* histogramArray_Percents ,
    int iPixelSize = 1 );
  void			calculateHistogramArray		(
    LPBYTE pProcessingPicture, unsigned int uiProcessingPictureSize, 
    DWORD* histogramArray_Pixels ,
    int iPixelSize = 1 );
	unsigned int	defineTresholdByHistogram	(
    LPBYTE pProcessingPicture, unsigned int uiProcessingPictureSize ,
    int iPixelSize = 1 );
	double			countSpotsDensity			(
    LPBYTE pProcessingPicture, unsigned int uiProcessingPictureSize ,
    int iPixelSize = 1 );
  unsigned int BuildBinarizedPicture( CVideoFrame * pFrame , int iThres ) ;
  int GetLowerHistMax( int& iMaxPos , int& iIntegral ) ;
  int GetHighHistMax( int& iMaxPos , int& iIntegral ) ;
protected:

	BlackArea();

  int m_iMin ;
  int m_iMax ;
  int m_iRealMinPos ;
  int m_iRealMaxPos ;
  int m_iBlackThres ;
  int m_iWhiteThres ;
  int m_iMainThres ;
  int m_bOutBinarized ;
  Algorithm m_iAlgorithm ;
  int m_iFrameSize ;
  int m_iHistSize ;
  int m_iAllocatedHistSize ;
  DWORD * m_pHist ;
  int m_iDistForward ;

  int m_iHistLowMaxPos ;
  int m_iLowBegin ;
  int m_iLowEnd ;
  int m_iHistLowMaxValue ;
  int m_iHistLowIntegral ;

  int m_iHistHighMaxPos ;
  int m_iHighBegin ;
  int m_iHighEnd ;
  int m_iHistHighMaxValue ;
  int m_iHistHighIntegral ;
  
  DWORD m_dwHistMax ;
  DWORD m_dwHistMaxPos ;
  DWORD m_dwSecondHistMax ;
  DWORD m_dwSecondHistMaxPos ;


public:
  void ShutDown() 
  {
    UserGadgetBase::ShutDown() ;
    if ( m_pHist )
    {
      delete m_pHist ;
      m_pHist = NULL ;
    }
  }
	double	m_dProportionThres_Perc;
	int		iHistogramDensityPercents_Property; // not used
  double m_dHistDensityPercent ;
	bool	bUseTresholdByValueOnly_Property;
  int   m_iSavedWhiteLevel ;
  int   m_iSavedBlackLevel ;


	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

	DECLARE_RUNTIME_GADGET(BlackArea);
};

#endif	// __INCLUDE__BlackAreaCalculator_H__


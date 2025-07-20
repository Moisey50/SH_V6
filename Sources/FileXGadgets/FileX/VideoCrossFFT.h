// UserExampleGadget.h : Declaration of the UserExampleGadget class



#ifndef __INCLUDE__VideoCrossFFT_H__
#define __INCLUDE__VideoCrossFFT_H__


#pragma once
#include "helpers\UserBaseGadget.h"
#include "gadgets/ContainerFrame.h"
#include <helpers\FramesHelper.h>
#include <imageproc\conv.h>

#include <iostream>
#include <iomanip>
#include <map>
#include <random>
#include <cmath>
#include <math\FigureProcessing.h>
#include <helpers\PropertyKitEx.h>

enum UseSpectrums
{
  US_Both = 0 ,
  US_Vertical ,
  US_Horizontal 
};

class CRangeI
{
public:
  int m_iBegin = 0 ;
  int m_iEnd = 0 ;
  CRangeI( int iBegin , int iEnd ) 
  {
    m_iBegin = iBegin ;
    m_iEnd = iEnd ;
  }
  CRangeI( LPCTSTR pAsString )
  {
    FXString AsString( pAsString ) ;
    AsString.Trim() ;
    LPCTSTR pBegin = ( LPCTSTR ) AsString ;
    if ( ( AsString[ 0 ] == '(' )
      || (AsString[0] == '[') || (AsString[0] == '{') ) 
    {
      pBegin++ ;
    }
    m_iBegin = m_iEnd = 0 ;
    sscanf( pBegin , "%d,%d" , &m_iBegin , &m_iEnd ) ;
  }
  bool IsValid() { return ( m_iBegin != m_iEnd ) ; } ;
  bool FromString( LPCTSTR pAsString )
  {
    FXString AsString( pAsString ) ;
    AsString.Trim() ;
    LPCTSTR pBegin = ( LPCTSTR ) AsString ;
    if (( AsString[ 0 ] == '(' )
      || ( AsString[ 0 ] == '[' ) || ( AsString[ 0 ] == '{' ))
    {
      pBegin++ ;
    }
    m_iBegin = m_iEnd = 0 ;
    sscanf( pBegin , "%d,%d" , &m_iBegin , &m_iEnd ) ;
    return (m_iBegin != m_iEnd) ;
  }
  FXString ToString()
  {
    FXString AsString ;
    AsString.Format( "(%d,%d)" , m_iBegin , m_iEnd ) ;
    return AsString ;
  }
};

typedef std::vector<CRangeI> IRanges ;

class CrossFFT : public UserBaseGadget
{
protected:
	CrossFFT();
  virtual ~CrossFFT() ;
public:

  FXLockObject m_LockSettings ;
  
  FXString      m_sCrossPt ;
  FXString      m_sActiveRectHalfSize ;

  CPoint        m_CrossPt ;
  CPoint        m_CrossShift;
  UseSpectrums  m_UseSpectrums = US_Both ;
  BOOL          m_bRemoveDC ;
  double        m_dLastHorDC ;
  double        m_dLastVertDC ;

  CSize         m_ActiveRectHalfSize ;
  int           m_iFFT_Order ;
  CSize         m_Multiplier ; // two dimensional
  int           m_iSpectrumViewShift ;
  FXString      m_AccumulationRanges ;


  double *      m_pdHorIm ;
  double *      m_pdHorRe ;
  double *      m_pdVertIm ;
  double *      m_pdVertRe ;
  double *      m_pdVertAmpl ;
  double *      m_pdHorAmpl ;
  double *      m_pdCoeffs ;
  double *      m_pdWindow ;   // window envelope
  int           m_iAllocatedFFTSize ;
  int           m_iHalfUsedData ; // Half window size for FFT
  int           m_iAllocatedWindowSize ;
  double        m_dCameraResolution_um_per_pix;
  
  CRect         m_LastWorkingRect ;
  IRanges       m_Ranges ;
  DoubleVector  m_LastPowers ;
  CVideoFrame * m_pLastVideoFrame ;

	//	Mandatory functions
	void PropertiesRegistration();
	void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
  void bCheckAndRealloc( double ** pArray , int iNewSize ) ;
  void InitEnvelope( double * pEnv , int iHalfSize ) ;
  void GetSpectrums( void* pData );
  void CrossFFT::CalcSpectrums( const CVideoFrame * pData ) ;
  CContainerFrame * CreateMarkers(CFigureFrame * pData, cmplx& Origin, cmplx& Step , 
    int iAmpl , bool bVert = false);
  int ScanRanges( FXString& AsString ) ;
	DECLARE_RUNTIME_GADGET(CrossFFT);
};

#endif	// __INCLUDE__VideoCrossFFT_H__


#ifndef _SVM_DETECTOR_H_
#define _SVM_DETECTOR_H_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <fxfc\fxfc.h>
#include <math\Intf_sup.h>
#include <gadgets\gadbase.h>
#include <gadgets\stdsetup.h>
#include <gadgets\videoframe.h>
#include <helpers\propertykitEx.h>


typedef double * pDblImageData ; // full data for one image Width*Height
typedef unsigned short * pshImageData ;

typedef FXArray<pDblImageData> Cube ; // Set of images
typedef FXArray<pshImageData> shCube ;

class CSVM_Detector
{

public:
  CSVM_Detector::CSVM_Detector() ;
  ~CSVM_Detector() ;


  CRect m_RefRect ;
  CSize m_ImageSize ;
  int   m_iNChannels ;
  int   m_iAveraging ;
  double m_dBias ;
  double m_dImageUpdatingTime ;
  double m_dNormCubeUpdateTime ;
  double m_dDetectionTime ;
  BYTE* m_pResult ; // Width * Height, byte per pixel

  Cube  m_Cube ;
  Cube  m_IntegralCube ;
  FXIntArray m_ImagePresence ; // ?
  FXDblArray m_dWaveLengths ;
  FXDblArray m_dRefVect ;
  FXDblArray m_dRefNormVect ;
  FXDblArray m_dNormVect ; //?
  FXDblArray m_dLogMeanVect ;
  FXDblArray m_dLogStdVect ;
  FXDblArray m_dBetaVect ;

  void Init( int iHeight , int iWidth , int iNChannels , int iAver ,
    double * pRefVect , CRect Rect , double * pLogNormMean , double * pLogStdVect ,
    double * pBetaVect , double dBias ) ;
  void Init() ;

  bool bAddImage( const CVideoFrame * pNewFrame ) ;
  bool CalcIntegralImage( const CVideoFrame * pNewFrame , int iChan ) ;
  void ExtractMeanFromCube( int iChan ,
    FXDblArray& Target , int x1 , int y1 , int x2 , int y2 , int s ) ;
  void ExtractMeanFromCube(
    FXDblArray& Target , int x1 , int y1 , int x2 , int y2 , int s ) ;
  void UpdateNormalizedCube( int iChan ) ;
  void CalcNormalizedCube() ;
  void UpdateNormalizedVector( int iChan ) ;
  void CalcNormalizedVector() ;
  int RunPrediction() ;
  int  Detect() ;
  bool IsReady() 
  {
    for ( int i = 0 ; i < m_ImagePresence.GetCount() ; i++ )
    {
      if ( !m_ImagePresence[ i ] )
        return false ;
    }
    return true ;
  }
};

#endif
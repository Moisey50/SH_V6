#include "stdafx.h"
#include "csvm_detector.h"

#define my_min( a , b )		(a) < (b) ? (a) : (b)
#define my_max( a , b )		(a) > (b) ? (a) : (b)

CSVM_Detector::CSVM_Detector()
{
  memset( this , 0 , ( LPBYTE ) &m_Cube - ( LPBYTE )this ) ;
}


CSVM_Detector::~CSVM_Detector()
{
  for ( int i = 0 ; i < m_iNChannels ; i++ )
  {
    if ( m_Cube.GetCount() > i )
      delete[] m_Cube[ i ] ;
    if ( m_IntegralCube.GetCount() > i )
      delete[] m_IntegralCube[ i ] ;
  }
  if ( m_pResult )
    delete m_pResult ;
}


void CSVM_Detector::Init( int iHeight , int iWidth , int iNChannels , int iAver ,
  double * pRefVect , CRect Rect , double * pLogNormMean , double * pLogStdVect ,
  double * pBetaVect , double dBias )
{
  m_RefRect = Rect ;
  m_ImageSize = CSize( iWidth , iHeight ) ;
  int iImageSize = iWidth * iHeight ;
  m_iNChannels = iNChannels ;
  m_iAveraging = iAver ;
  m_dBias = dBias ;
  for ( int i = 0 ; i < iNChannels ; i++ )
  {
    m_dRefVect.Add( pRefVect[ i ] ) ;
    m_dLogMeanVect.Add( pLogNormMean[ i ] ) ;
    m_dLogStdVect.Add( pLogStdVect[ i ] ) ;
    m_dBetaVect.Add( pBetaVect[ i ] ) ;
    m_Cube.Add( new double[ iImageSize ] ) ;
    m_IntegralCube.Add( new double[ iImageSize ] ) ;
    m_ImagePresence.Add( 0 ) ;
    m_dNormVect.Add( 0. ) ;
    m_dRefNormVect.Add( 0. ) ;
  }
  m_pResult = new BYTE[ iImageSize ] ;
}

void CSVM_Detector::Init()
{
  int iImageSize = m_ImageSize.cx * m_ImageSize .cy ;
  ASSERT( iImageSize ) ;
  for ( int i = 0 ; i < m_iNChannels ; i++ )
  {
    m_Cube.Add( new double[ iImageSize ] ) ;
    m_IntegralCube.Add( new double[ iImageSize ] ) ;
    m_ImagePresence.Add( 0 ) ;
    m_dNormVect.Add( 0. ) ;
    m_dRefNormVect.Add( 0. ) ;
  }
  m_pResult = new BYTE[ iImageSize ] ;
}


void CSVM_Detector::ExtractMeanFromCube( int iChan ,
  FXDblArray& Target , int x1 , int y1 , int x2 , int y2 , int s )
{
  double * pIm = m_IntegralCube[ iChan ] ;
  int iy2Shift = y2 * m_ImageSize.cx  ;
  int iy1Shift = y1 * m_ImageSize.cx ;
  Target[ iChan ] = pIm[ iy2Shift + x2 ] + pIm[ iy1Shift + x1 ] ;
  if ( y1 > 0 )
    Target[ iChan ] -= pIm[ iy1Shift - m_ImageSize.cx + x2 ] ;
  if ( x1 > 0 )
    Target[ iChan ] -= pIm[ iy2Shift + x1 - 1 ] ;
  Target[ iChan ] /= s ;
}

void CSVM_Detector::ExtractMeanFromCube( 
  FXDblArray& Target , int x1 , int y1 , int x2 , int y2 , int s )
{
  for ( int iChan = 0 ; iChan < m_iNChannels ; iChan++ )
  {
    double * pIm = m_IntegralCube[ iChan ] ;
    int iy2Shift = y2 * m_ImageSize.cx  ;
    int iy1Shift = y1 * m_ImageSize.cx ;
    Target[ iChan ] = pIm[ iy2Shift + x2 ] + pIm[ iy1Shift + x1 ] ;
    if ( y1 > 0 )
      Target[ iChan ] -= pIm[ iy1Shift - m_ImageSize.cx + x2 ] ;
    if ( x1 > 0 )
      Target[ iChan ] -= pIm[ iy2Shift + x1 - 1 ] ;
    Target[ iChan ] /= s ;
  }
}

void CSVM_Detector::UpdateNormalizedCube( int iChan )
{
  double * pIm = m_Cube[ iChan ] ;
  int iLx = 1 + 2 * m_iAveraging , iLy = 1 + 2 * m_iAveraging ;
  int iS = iLx * iLy ;
  for ( int iY = 0 ; iY < m_ImageSize.cy - iLy - 1 ; iY++ )
  {
    for ( int iX = 0 ; iX < m_ImageSize.cy - iLx - 1 ; iX++ )
    {
      ExtractMeanFromCube( iChan , m_dNormVect , iX , iY , iX + iLx , iY + iLy , iS ) ;
      pIm[ ( iY + m_iAveraging ) * m_ImageSize.cx + iX + m_iAveraging ] =
        ( log10( m_dNormVect[ iChan ] ) - m_dRefNormVect[ iChan ] + m_dLogMeanVect[ iChan ] ) / ( m_dLogStdVect[ iChan ] + 1e-5 ) ;
    }
  }
}

void CSVM_Detector::CalcNormalizedCube()
{
  int iLx = 1 + 2 * m_iAveraging , iLy = 1 + 2 * m_iAveraging ;
  int iS = iLx * iLy ;
  for ( int iY = 0 ; iY < m_ImageSize.cy - m_iAveraging - 1 ; iY++ )
  {
    for ( int iX = 0 ; iX < m_ImageSize.cy - m_iAveraging - 1 ; iX++ )
    {
      ExtractMeanFromCube( m_dNormVect , iX , iY , iX + iLx , iY + iLy , iS ) ;
      for ( int iChan = 0 ; iChan < m_iNChannels ; iChan++ )
      {
        double * pIm = m_Cube[ iChan ] ;
        pIm[ ( iY + m_iAveraging ) * m_ImageSize.cx + iX + m_iAveraging ] =
          ( log10( m_dNormVect[ iChan ] ) - m_dRefNormVect[ iChan ] + m_dLogMeanVect[ iChan ] ) / (m_dLogStdVect[ iChan ] + 1e-5) ;
      }
    }
  }
}

void CSVM_Detector::UpdateNormalizedVector( int iChan )
{
  int iS = m_RefRect.Width() * m_RefRect.Height() ;
  ExtractMeanFromCube( m_dNormVect , m_RefRect.left , 
    m_RefRect.top , m_RefRect.right , m_RefRect.bottom , iS ) ;
  m_dRefNormVect[ iChan ] = log10( m_dNormVect[ iChan ] ) / m_dRefVect[ iChan ] ;
}

void CSVM_Detector::CalcNormalizedVector()
{
  int iS = m_RefRect.Width() * m_RefRect.Height() ;
  ExtractMeanFromCube( m_dNormVect , m_RefRect.left , m_RefRect.top , m_RefRect.right , m_RefRect.bottom , iS ) ;
  for ( int iChan = 0 ; iChan < m_iNChannels ; iChan++ )
    m_dRefNormVect[ iChan ] = log10( m_dNormVect[ iChan ] ) / m_dRefVect[ iChan ] ;
}

bool CSVM_Detector::CalcIntegralImage( 
  const CVideoFrame * pFrame , int iChan )
{
  double dStart = GetHRTickCount() ;
  LPBYTE pNewIm = GetData( pFrame ) ;
  if ( !pNewIm  || iChan < 0  ||  iChan >= m_ImagePresence.GetCount() )
    return false ;

  pDblImageData pdIm = m_IntegralCube[ iChan ] ;
  double * pdIter = pdIm ;
  DWORD dwComp = pFrame->lpBMIH->biCompression ;
  if ( dwComp == BI_Y16 ) // 16 bits per pixel
  {
    LPWORD pwIm = ( LPWORD ) pNewIm ;
    LPWORD pwIter = pwIm ;
    for ( int iY = 0 ; iY < m_ImageSize.cy ; iY++ )
    {
      pdIter = pdIm + m_ImageSize.cx * iY ;
      double * pLimit = pdIter + m_ImageSize.cx ;
      pwIter = pwIm + m_ImageSize.cx * iY ;
      *( pdIter ) = ( double ) *( pwIter++ ) ; // fill first pixel for following operations
      while ( ++pdIter < pLimit )
        *pdIter = *( pdIter - 1 ) + ( double ) *( pwIter++ ) ; // add source pixel
      // to the previous sum
    }
  }
  else if ( dwComp == BI_Y8 || dwComp == BI_Y800  // 8 bits per pixel
    || dwComp == BI_YUV9 || dwComp == BI_YUV12 )
  {
    LPBYTE pbyIter = pNewIm ;
    for ( int iY = 0 ; iY < m_ImageSize.cy ; iY++ )
    {
      pdIter = pdIm + m_ImageSize.cx * iY ;
      double * pLimit = pdIter + m_ImageSize.cx ;
      pbyIter = pNewIm + m_ImageSize.cx * iY ;
      *( pdIter ) = ( double ) *( pbyIter++ ) ; // fill first pixel for following operations
      while ( ++pdIter < pLimit )
        *pdIter = *( pdIter - 1 ) + ( double ) *( pbyIter++ ) ; // add source pixel
      // to the previous sum
    }

  }
  // step 1: load row by row with integral calculation in row
  // i.e. each pixel will be equal to sum of all original 
  // pixels on the left plus this pixel
  double * pEnd = pdIm + m_ImageSize.cx * m_ImageSize.cy ;
  // now all pixels are consisting of row integrals
  // lets do columns integral
  for ( int iX = 0 ; iX < m_ImageSize.cx ; iX++ )
  {
    // begin from second row because first row is already integral
    pdIter = pdIm + iX ;
    double * pdPrevIter = pdIter ; // pointer to pixel above
    while ( ( pdIter += m_ImageSize.cx ) < pEnd ) // we are going with steps of full row
    {
      *pdIter += *( pdPrevIter ) ;
      pdPrevIter += m_ImageSize.cx ;
    }
  }
  m_ImagePresence[ iChan ] = 1 ;
  m_dImageUpdatingTime = GetHRTickCount() - dStart ;
  UpdateNormalizedCube( iChan ) ;
  m_dNormCubeUpdateTime = GetHRTickCount() - dStart - m_dImageUpdatingTime ;
  UpdateNormalizedVector( iChan ) ;
  return true ;
}


int CSVM_Detector::RunPrediction()
{
  double dStart = GetHRTickCount() ;
  int iNPoints = 0 ;
  int iClearance = m_iAveraging + 1 ;
  memset( m_pResult , 0 , m_ImageSize.cx * m_ImageSize.cy ) ;
  for ( int iY = iClearance ; iY < m_ImageSize.cy - iClearance ; iY++ )
  {
    int iShift = iY * m_ImageSize.cx + iClearance ;
    for ( int iX = iClearance ; iX < m_ImageSize.cx - iClearance ; iX++ , iShift++ )
    {
      double dR = m_dBias ;
      for ( int iChan = 0 ; iChan < m_iNChannels ; iChan++ )
        dR += m_dBetaVect[ iChan ] * ( *( m_Cube[ iChan ] + iShift ) ) ;
      m_pResult[ iShift ] = ( dR > 0. ) ? 255 : 0 ;
      iNPoints += ( dR > 0. ) ;
    }
  }
  m_dDetectionTime = GetHRTickCount() - dStart ;
  return iNPoints ;
}

int CSVM_Detector::Detect()
{
  return RunPrediction() ;
}
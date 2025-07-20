// FRegression.cpp: implementation of the CFRegression class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <math.h>
#include <gadgets/figureframe.h>
#include "FRegression.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CFStatistics operations 
//////////////////////////////////////////////////////////////////////

bool CFStatistics::GetValueForPercentOnHistogram( 
  double dPercent , double& dMin , double& dMax )
{
  if ( ( dPercent < 0. ) || ( 100. < dPercent ) || ( m_dMin >= m_dMax ) )
    return false ;

  size_t uNTarget = ROUND( ( ( 100. - dPercent ) * m_uNValuesInHisto ) / 100. ) ;
  size_t uAccumulated = 0 ;
  int i = 0 ;
  for ( ; i < HISTO_LEN / 2 ; i++ )
  {
    uAccumulated += m_Histogram[ i ] + m_Histogram[ HISTO_LEN - i - 1 ] ;
    if ( uAccumulated >= uNTarget )
      break ;
  }
  dMin = m_dMin + i * ( m_dMax - m_dMin ) / HISTO_LEN ;
  dMax = m_dMin + ( ( HISTO_LEN - i - 1 ) * ( m_dMax - m_dMin ) ) / HISTO_LEN ;

  return true ;
}





//////////////////////////////////////////////////////////////////////
// CFRegression Construction/Destruction 
//////////////////////////////////////////////////////////////////////

CFRegression::CFRegression()
{
  Reset() ;
}

CFRegression::~CFRegression()
{

}

void 
CFRegression::Reset()
{
  m_dSumX = m_dSumY = m_dSumXX = m_dSumYY = m_dSumXY = 0. ;
  m_da = m_db = 0. ;
  m_iNSamples = 0 ;
}

int 
CFRegression::Add(double dX, double dY)
{
  m_dSumX += dX ;
  m_dSumY += dY ;
  m_dSumXX += dX * dX ;
  m_dSumXY += dX * dY ;
  m_dSumYY += dY * dY ;
  m_iNSamples++ ;

  return m_iNSamples ;
}

double 
CFRegression::GetY(double dX)
{
  if ( m_dXConst == DBL_MAX )
    return m_da * dX + m_db ;
  return 0.0 ;
}
double
CFRegression::GetX( double dY )
{
  if ( m_dYConst == DBL_MAX )
    return m_daY * dY + m_dbY ;
  return 0.0 ;
}

double 
CFRegression::Calculate()
{
  if ( !m_iNSamples )
    return 0. ;
  double dDividerX = m_iNSamples * m_dSumXX - m_dSumX * m_dSumX ;
  double dDividerY = m_iNSamples * m_dSumYY - m_dSumY * m_dSumY ;
  double dXYAll = m_iNSamples * m_dSumXY ;
  double dXY = m_dSumX * m_dSumY ;
  double dCov = (m_dSumXY - (dXY/m_iNSamples)) / m_iNSamples ;
  if ( fabs( dDividerX ) > 1.e-6 ) 
  {
    // Beta https://en.wikipedia.org/wiki/Simple_linear_regression
    m_da = ( dXYAll - dXY ) / dDividerX ; 
    m_db =  ( (m_dSumY * m_dSumXX) - (m_dSumX * m_dSumXY)) / dDividerX ;
    m_dDividerX = sqrt( m_da * m_da + 1. ) ;
    m_dXConst = DBL_MAX ;
    // CLine2d coefficients alignment, i.e. 
    // adjustment y=m_da*x+m_db to m_a * x + m_b * y + m_c = 0
    m_dEstimBeta = m_da ;
    m_dEstimAlpha = (m_dSumY - m_dEstimBeta * m_dSumX) / m_iNSamples ;
    if ( m_iNSamples > 2 )
    {
      m_dS2Epsilon = (dDividerY - (m_dEstimBeta * m_dEstimBeta) * (dDividerX)) ;
      m_dS2Beta = (m_iNSamples * m_dS2Epsilon) / dDividerX ;
      m_dS2Alpha = m_dS2Beta * m_dSumXX / m_iNSamples ;
    }
  }
  else
  {
    m_da = m_db = 0.0 ;
    m_dXConst = m_dSumX / m_iNSamples ;
    m_dYConst = DBL_MAX ;
  }
  if ( fabs( dDividerY ) > 1.e-6 )
  {
    m_daY = (dXYAll - dXY ) / dDividerY ;
    m_dbY = ((m_dSumX * m_dSumYY) - (m_dSumY * m_dSumXY) ) / dDividerY ;
    m_dDividerY = sqrt( m_daY * m_daY + 1. ) ;
    m_dYConst = DBL_MAX ;
    return dDividerY ;
  }
  else
  {
    m_daY = m_dbY = 0.0 ;
    m_dYConst = m_dSumY / m_iNSamples ;
    m_dXConst = DBL_MAX ;
  }
  return 0. ;
}

int
CFRegression::Remove(double X, double Y)
{
  if ( !m_iNSamples )
    return -1 ;
  m_dSumX -= X ;
  m_dSumY -= Y ;
  m_dSumXX -= X * X ; 
  m_dSumXY -= X * Y ;
  m_dSumYY -= Y * Y ;
  m_iNSamples --;

  return m_iNSamples;
}

double CFRegression::GetRSquared()
{
  double dXAv = m_dSumX / m_iNSamples;
  double dYAv = m_dSumY / m_iNSamples;
  double dXYAv = m_dSumXY / m_iNSamples;
  double dNumerator = dXYAv - (dXAv * dYAv);
  double dX2Av = (m_dSumXX * m_dSumXX) / m_iNSamples;
  double dY2Av = (m_dSumYY * m_dSumYY) / m_iNSamples;

  double dDivider = (dX2Av - (dXAv * dXAv)) * (dY2Av -( dYAv * dYAv));
  double r = (fabs(dDivider) > 1e-10) ? dNumerator / dDivider : 0.;
 
  return r;
}

void CFRegression::AddPtsToRegression( const cmplx * pArray , int iLen ,
  int iBeginIndex , int iEndIndex )
{
  if ( iBeginIndex == -1 )
    iBeginIndex = 0 ;
  if ( iEndIndex == -1 )
    iEndIndex = iLen - 1 ;
  ASSERT( (iBeginIndex >= 0) && (iBeginIndex < iLen) && (iEndIndex >= 0) && ( iEndIndex < iLen ) ) ;
  if ( ( iBeginIndex == 0 ) && ( iEndIndex == 0 ) )
    iEndIndex = iLen - 1 ;
  int iNSamples = ( iBeginIndex < iEndIndex ) ?
    iEndIndex - iBeginIndex : ( iLen - iBeginIndex ) + iEndIndex;
  int iIterator = iBeginIndex ;
  do
  {
    Add( pArray[ iIterator++ ] ) ;
    iIterator %= iLen ;
  } while ( --iNSamples > 0 );
}

void CFRegression::AddPtsToRegression( const CmplxVector& Data ,
  int iBeginIndex , int iEndIndex )
{
  int iLen = (int) Data.size() ;
  if ( iBeginIndex == -1 )
    iBeginIndex = 0 ;
  if ( iEndIndex == -1 )
    iEndIndex = iLen - 1 ;
  ASSERT( ( iBeginIndex >= 0 ) && ( iBeginIndex < iLen ) && ( iEndIndex >= 0 ) && ( iEndIndex < iLen ) ) ;
  int iNSamples = ( iBeginIndex < iEndIndex ) ?
    iEndIndex - iBeginIndex : ( iLen - iBeginIndex ) + iEndIndex;
  int iIterator = iBeginIndex ;
  do
  {
    Add( Data[ iIterator++ ] ) ;
    iIterator %= iLen ;
  } while ( --iNSamples > 0 );
}

void CFRegression::AddPtsToRegressionSwapXY( const cmplx * pArray , int iLen ,
  int iBeginIndex , int iEndIndex )
{
//   if ( iEndIndex < iBeginIndex )
//     swap( iBeginIndex , iEndIndex ) ;
  if ( iBeginIndex < 0 )
    iBeginIndex = 0 ;
  if ( iEndIndex < 0 )
    iEndIndex = iLen - 1 ;
  ASSERT( ( iBeginIndex >= 0 ) && ( iBeginIndex < iLen ) && ( iEndIndex >= 0 ) && ( iEndIndex < iLen ) ) ;
  if ( ( iBeginIndex == 0 ) && ( iEndIndex == 0 ) )
    iEndIndex = iLen - 1 ;
  if ( (iLen - iEndIndex + iBeginIndex) < (iEndIndex - iBeginIndex) )
  { // we are crossing array origin
    int iIterator = iEndIndex ;
    do
    {
      AddSwapped( pArray[ iIterator ] ) ;
      iIterator++ ;
      iIterator %= iLen ;
    } while ( iIterator != iBeginIndex + 1 );
  }
  else // we are not crossing array origin
  {
    int iIterator = iBeginIndex ;
    do
    {
      AddSwapped( pArray[ iIterator++ ] ) ;
    } while ( iIterator != iEndIndex + 1 );
  }
}

CLine2d  CFRegression::GetCLine2d()
{
  if (m_dXConst < 1.e308)
  {
    CLine2d AsLine(cmplx(m_dXConst , 500.) , cmplx(m_dXConst , 1000.));
    return AsLine;
  }
  if (m_dYConst < 1.e308)
  {
      CLine2d AsLine(cmplx(500. , m_dYConst ) , cmplx(1000. , m_dYConst));
      return AsLine;
  }
  if ( fabs(m_da) < fabs(m_daY) )
  {
    CLine2d AsLineAX( m_da , -1. , m_db ) ;
    return AsLineAX ;
  }
  else
  {
    CLine2d AsLineAY( -1. , m_daY , m_dbY ) ;
    return AsLineAY ;
  }
}

int CFilterRegression::Add( cmplx& cPt )
{
  m_Pts.push_back( cPt ) ;
  return CFRegression::Add( cPt ) ;
}

cmplx CalcCmplxAverageAndStd( const cmplx * pData , FXSIZE iLen , double& dStd ) 
{
  cmplx cSum , cStd ;
  const cmplx * pC = pData ;
  const cmplx * pEnd = pData + iLen ;
  do
  {
    cSum += *pC ;
  } while ( ++pC < pEnd ) ;
  cmplx cAvg = cSum / ( double ) iLen ;
  pC = pData ;
  double dSqSum = 0. ;
  do 
  {
    double dDist = abs( *pC - cAvg ) ;
    dSqSum += dDist * dDist ;
  } while ( ++pC < pEnd );
  dStd = sqrt( dSqSum / iLen ) ;
  return cAvg ;
}

cmplx CalcCmplxAverageAndStd( const CmplxVector& Data , double& dStd )
{
  return CalcCmplxAverageAndStd( Data.data() , Data.size() , dStd ) ;
}

cmplx AddCmplxValAndCalcAverageAndStd( cmplx cNewVal , 
  CmplxVector& Data , double& dStd ) 
{
  memcpy( Data.data() + 1 , Data.data() ,
    ( Data.size() - 1 ) * sizeof( cmplx ) ) ;
  Data[ 0 ] = cNewVal ;
  return CalcCmplxAverageAndStd( Data.data() , Data.size() , dStd ) ;
}


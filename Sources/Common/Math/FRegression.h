// FRegression.h: interface for the CFRegression class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FREGRESSION_H__DA52B4B3_CAC0_11D3_8D73_000000000000__INCLUDED_)
#define AFX_FREGRESSION_H__DA52B4B3_CAC0_11D3_8D73_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//#include <math/PlaneGeometry.h>
#include <math/Intf_sup.h>
#include <math/Line2d.h>

#define HISTO_LEN 1001

class CFStatistics
{
public:
  double   m_dStdDev ;
  double   m_dSumOfSquares ;
  cmplx    m_cmplxStdDev ;
  double   m_dAveragedValue ;
  double   m_dAccumulated ;
  double   m_dMin ;
  double   m_dMax ;
  size_t   m_uNValuesInHisto ;
  size_t   m_Histogram[ HISTO_LEN ] ;
  DoubleVector m_Values ;

  CFStatistics() { Reset();  }
  void Reset() 
  { 
    memset( &m_dStdDev , 0 , (LPBYTE)&m_Histogram - (LPBYTE)&m_dStdDev ) ; 
    m_dMin = DBL_MAX ; m_dMax = -DBL_MAX ; 
    m_Values.clear() ;
  }
  void ResetHisto() { memset( m_Histogram , 0 , sizeof( m_Histogram ) ) ; m_uNValuesInHisto = 0 ; }
  virtual size_t Add( double dVal )
  {
    m_dAccumulated += dVal ;
    SetMinMax( dVal , m_dMin , m_dMax ) ;
    m_dSumOfSquares += dVal * dVal ;
    m_Values.push_back( dVal ) ;
    return m_Values.size() ;
  }
  virtual double Calculate()
  {
    m_dAveragedValue = m_dAccumulated / m_Values.size() ;
    m_dStdDev = sqrt( ( m_dSumOfSquares - ( m_dAccumulated * m_dAccumulated / m_Values.size() ) ) / m_Values.size() ) ;
    for ( auto it = m_Values.begin() ; it < m_Values.end() ; )
      AddToHistogram( *(it++) ) ;
    return m_dAveragedValue ;
  }
  virtual int AddToHistogram( double dVal )
  {
    if ( m_dMin < m_dMax ) 
    {
      int iSlot = ROUND(( dVal - m_dMin ) * HISTO_LEN / ( m_dMax - m_dMin )) ;
      BOUND( iSlot , 0 , HISTO_LEN - 1 ) ;

      ++m_Histogram[ iSlot ] ;
      ++m_uNValuesInHisto ;
      return iSlot ;
    }
    return -1 ;
  }
  virtual bool GetValueForPercentOnHistogram( double dPercent , double& dMin , double& dMax );
  double GetAverage() { return m_dAveragedValue;  }
  double GetStd() { return m_dStdDev ;  }
  size_t GetNAccumulated() {  return m_Values.size() ; }
  double GetMinMax( double& dMin , double& dMax )
  {
    dMin = m_dMin ; dMax = m_dMax ; return m_dAveragedValue ;
  }
  size_t GetMinIndex()
  {
    double * p = m_Values.data() ;
    while ( fabs( *p - m_dMin ) > 1e-10 ) p++ ;
    return (p - m_Values.data()) ;
  }
};

typedef vector<CFStatistics> StatVector ;
typedef vector<StatVector> StatVectors ;

class CFRegression 
{
public:
	double Calculate();

	double GetY( double dX ) ;
  double GetX( double dY ) ;
  CLine2d GetCLine2d() ;
  virtual int Add( double X , double Y );
  virtual int Add( const cmplx& Pt ) { return Add( Pt.real() , Pt.imag() ) ; };
  virtual int AddSwapped( const cmplx& Pt ) { return Add( Pt.imag() , Pt.real() ) ; };
  void Reset();
  int Remove( double X , double Y ) ;
  int Remove( const cmplx& Pt ) { return Remove( Pt.real() , Pt.imag() ) ; };
  size_t Size() { return (size_t)m_iNSamples;  }
  double GetRSquared();
  inline double GetDistFromPt( const cmplx& Pt )
  {
    double dDist = ( m_dXConst > 1.e308 ) ?
      fabs( -m_da*Pt.real() + Pt.imag() - m_db ) 
      : fabs( Pt.real() - m_dXConst ) ;
    return dDist ;
  }
  cmplx GetPtOnLine( const cmplx& Pt )
  {
    if ( m_dXConst < 1.e308 )
      return cmplx( m_dXConst , Pt.imag() ) ;
    else if ( m_dYConst < 1.e308 )
      return cmplx( Pt.real() , m_dYConst ) ;

    return GetOrthCrossOnLine( cmplx( 0. , m_db ) , cmplx( -m_db / m_da ) , Pt ) ;
  }
  CFRegression& operator=( const CFRegression& Orig )
  {
    memcpy( &m_dSumX , &Orig.m_dSumX , ((( LPBYTE ) ( &m_dDividerY )) + sizeof(double) ) - ( LPBYTE )&m_dSumX ) ;
  }
  // next function takes points pArray from iBeginIndex to iEndIndex
  // It's supposed, that array is closed contur with pints enumeration 
  // in any direction
  // Begin point could be bigger than end point
  void AddPtsToRegression( const cmplx * pArray , int iArrayLen ,
    int iBeginIndex = -1 , int iEndIndex = -1 );
  void AddPtsToRegression( const CmplxVector& Data ,
    int iBeginIndex = -1 , int iEndIndex = -1 ) ;
  void AddPtsToRegressionSwapXY( const cmplx * pArray , int iArrayLen ,
    int iBeginIndex = -1 , int iEndIndex = -1 );

	CFRegression();
	virtual ~CFRegression();
  double GetStdFromLine( const cmplx * pData , int iDataLen , 
    int iBeginIndex = -1 , int iEndIndex = -1 , FXDblArray * pDists = NULL );

  double GetAngle() 
  { 
    CLine2d Line = GetCLine2d() ;
    return Line.get_angle() ;
  }

  double m_dSumX ;
  double m_dSumY ;
  double m_dSumXX ;
  double m_dSumXY ;
  double m_dSumYY ;

  int    m_iNSamples ;

  double m_da ;  // for Y = a * X + b
  double m_db ;  // ----"-----
  double m_dXConst ;
  double m_daY ; // for X = a * Y + b
  double m_dbY ; // ----"-----
  double m_dYConst ;
  double m_dDividerX ;
  double m_dDividerY ;
  // For https://en.wikipedia.org/wiki/Simple_linear_regression
  // Numerical example
  double m_dEstimBeta ;
  double m_dEstimAlpha ;
  double m_dS2Epsilon ;
  double m_dS2Beta ;
  double m_dS2Alpha ;
};

typedef vector<CFRegression> RegrVector ;
typedef vector<RegrVector> RegrVectors ;

class CFigure ;

#define USED_SIMPLE   1
#define USED_FILTERED 2

class CFilterRegression : public CFRegression
{
public:
  CmplxVector m_Pts ;
  FXIntArray m_Used ;
  FXDblArray  m_Dists ;

  virtual int Add( cmplx& cPt ) ;
};

cmplx CalcCmplxAverageAndStd( const cmplx * pData , FXSIZE iLen , double& cStd ) ;
cmplx CalcCmplxAverageAndStd( const CmplxVector& Data , double& cStd ) ;
cmplx AddCmplxValAndCalcAverageAndStd( cmplx cNewVal , CmplxVector& Data , double& dStd ) ;
#endif // !defined(AFX_FREGRESSION_H__DA52B4B3_CAC0_11D3_8D73_000000000000__INCLUDED_)

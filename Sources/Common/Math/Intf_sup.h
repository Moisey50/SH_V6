/*
*
* Copyright 2018-2023 by File X Ltd., Ness Ziona, Israel
*
* All Rights Reserved
*
* Permission to use, copy, modify and distribute this 
* software and its documentation for any purpose is 
* restricted according to the software end user license 
* attached to this software.
*
* Any use of this software is subject to the limitations
* of warranty and liability contained in the end user
* license.  Without derogating from the abovesaid, 
* File X Ltd. disclaims all warranty with regard to
* this software, including all implied warranties of 
* merchantability and fitness.  In no event shall
* File X Ltd. be held liable for any special, indirect
* or consequential damages or any damages whatsoever
* resulting from loss of use, data or profits, whether in
* an action of contract, negligence or other tortious
* action, arising out of or in connection with the use or 
* performance of this software.
*
*/
#ifndef ____INTF_SUP_H
#define ____INTF_SUP_H
#pragma once

#include <afxtempl.h>
#include <complex>
#include <algorithm>
#include <vector>
#include <math.h>
#include <fxfc\fxfc.h>

#define _USE_MATH_DEFINES
#include <math\hbmath.h>


//typedef unsigned char Boolean;

#define EPSILON  (1E-9)


#ifndef M_PI
#define M_PI 3.141592653589793238462643
#endif

#define M_2PI ( M_PI * 2.0 )
#define M_PI2 ( M_PI / 2.0 )
#define M_PI_8 ( M_PI / 8.0 )
#define RadToDeg(x)   ((x) * ( 180.0 / M_PI ))
#define DegToRad(x)   ((x) * ( M_PI / 180.0))

#define X_CMP_MASK      1
#define Y_CMP_MASK      2
#define Z_CMP_MASK      4
#define THETA_CMP_MASK  8

#define ARRSZ(x) (sizeof(x)/sizeof(x[0]))
#define OBJECT_ZERO(x) (memset(&x,0,sizeof(x)))
#define PTR_CHECK_DEL(x) \
  if(x)\
  {\
    delete x;\
    x=NULL ;\
  }
#define PTRARR_CHECK_DEL(x) \
  if(x)\
  {\
    delete[] x;\
    x=NULL ;\
  }

#define det(a,b,c,d)  (a*d-b*c)

#define Sign sign

#ifndef ROUND
#define ROUND(a) ((int)round(a))
#endif

#define ROUNDPM(a) ( (a>=0.) ? ROUND(a) : (-ROUND(-a)) )


#ifndef _RE
#define _RE 0
#define _IM 1
#endif

typedef WORD Word;
typedef DWORD Dword;


typedef struct
{
  double x ;
  double y ;
} MeasSample ;

using namespace std;

typedef complex<double> cmplx;

typedef FXArray<cmplx , cmplx&> CmplxArray ;
typedef FXArray<MeasSample , MeasSample&> SampleArray ;
typedef FXArray<CPoint , CPoint&> CPointArray ;

typedef vector<string> StringVector ;
typedef vector<int>    IntVector ;
typedef vector<__int64> Int64Vector ;
typedef vector<size_t> Size_tVector;
typedef vector<double> DoubleVector ;
typedef vector<cmplx>  CmplxVector ;

typedef vector<StringVector> StringVectors ;
typedef vector<IntVector> IntVectors ;
typedef vector<Int64Vector> Int64Vectors ;
typedef vector<DoubleVector> DoubleVectors ;
typedef vector<CmplxVector> CmplxVectors ;

class NamedCmplxVector
{
public:
  string m_ObjectName ;
  DWORD  m_ObjectIndex = 0 ;
  CmplxVector m_Data ;

  NamedCmplxVector( LPCTSTR pName = "" , DWORD dwIndex = 0xffff , CmplxVector * pData = NULL )
  {
    m_ObjectName = pName ;
    m_ObjectIndex = dwIndex ;
    if (pData)
      m_Data = *pData ;
  }
  NamedCmplxVector& operator = ( const NamedCmplxVector& Orig )
  {
    m_ObjectName = Orig.m_ObjectName ;
    m_ObjectIndex = Orig.m_ObjectIndex ;
    m_Data = Orig.m_Data ;
    return *this ;
  }
};
typedef vector<NamedCmplxVector> NamedCmplxVectors ;

// squared distance between 2 points.
inline double dist2( cmplx x , cmplx y )
{
  cmplx cDiff = x - y ;
  return ( cDiff.real() * cDiff.real() + cDiff.imag() * cDiff.imag() ) ;
}

    // class for rectangle. The access to s and e are public.
class CmplxRect
{
public:

  CmplxRect() {}
  CmplxRect( const cmplx& cLT , const cmplx& cRD ) : m_cLT( cLT ) , m_cRD( cRD )
  {}

  void Normalize() 
  {
    if ( m_cLT.real() > m_cRD.real() )
      swap( m_cLT._Val[ _RE ] , m_cRD._Val[ _RE ] ) ;
    if ( m_cLT.imag() > m_cRD.imag() )
      swap( m_cLT._Val[ _IM ] , m_cRD._Val[ _IM ] ) ;
  }
  void Reset() { memset( this , 0 , sizeof(*this)) ; }

  double GetWidth() { return m_cRD.real() - m_cLT.real() ; }
  double GetHeight() { return m_cRD.imag() - m_cLT.imag() ; }
  // compute the area of a rectangle.
  double Area() const
  {
    return ( m_cRD.real() - m_cLT.real() ) * ( m_cRD.imag() - m_cLT.imag() );
  }

  // check if two rectangles are overlapped.
  bool IsOverlapped( const CmplxRect& rect ) const
  {
    if ( ( m_cLT.real() > rect.m_cRD.real()) // this on the right side of other
      || ( rect.m_cLT.real() > m_cRD.real() ) // this on the left side of other
      || ( m_cRD.imag() < rect.m_cLT.imag())  // this below of other (Y is going down)
      || ( rect.m_cRD.imag() < m_cLT.imag()) ) // this above the other
      return false;

    return true;
  }

  // Compute the distance between two non-overlapped rectangles.
  // Return squared distance between rectangles.
  double DistanceTo( const CmplxRect& rect ) const
  {
//     auto dist = []( double x , double y ) { return x * x + y * y; };
//     auto square = []( double x ) { return x * x; };
// 
    // position of rect related to this
//     bool left = rect.m_cRD.real()< m_cLT.real();
//     bool right = m_cRD.real()< rect.m_cLT.real();
//     bool below = rect.m_cRD.imag() < m_cLT.imag() ;
//     bool above = m_cRD.imag() < rect.m_cLT.imag() ;
// 
//     if ( left && below )
//       return dist2( rect.m_cRD , m_cLT );
//     else if ( left && above )
//       return dist2( m_cLT.real()- rect.m_cRD.real(), m_cRD.imag() - rect.m_cLT.imag() );
//     else if ( right && below )
//       return dist( m_cRD.real()- rect.m_cLT.real(), m_cLT.imag() - rect.m_cRD.imag() );
//     else if ( right && above )
//       return R2::distance2( rect.s , e );
//     else if ( left )
//       return square( m_cLT.real()- rect.m_cRD.real());
//     else if ( right )
//       return square( rect.m_cLT.real()- m_cRD.real());
//     else if ( above )
//       return square( rect.m_cLT.imag() - m_cRD.imag() );
//     else if ( below )
//       return square( m_cLT.imag() - rect.m_cRD.imag() );

    return 0.0;

  };

  // Compute the rectangle containing both given rectangles. 
  static CmplxRect BoundRectangle(
    const CmplxRect& cRectA , const CmplxRect& cRectB )
  {
    CmplxRect ret(
      cmplx( std::min<double>( cRectA.m_cLT.real() , cRectB.m_cLT.real() ) ,
      std::min<double>( cRectA.m_cLT.imag() , cRectB.m_cLT.imag() ) ) ,
      cmplx( std::max<double>( cRectA.m_cRD.real() , cRectB.m_cRD.real() ) ,
      std::max<double>( cRectA.m_cRD.imag() , cRectB.m_cRD.imag() ) ) ) ;
    return ret;
  };
  // Compute the rectangle containing both given rectangle and given point. 
  static CmplxRect BoundRectangle( const CmplxRect& cRect , const cmplx& cPt )
  {
    CmplxRect ret( 
      cmplx ( std::min<double>( cRect.m_cLT.real() , cPt.real() ) ,
              std::min<double>( cRect.m_cLT.imag() , cPt.imag() ) ),
      cmplx ( std::max<double>( cRect.m_cRD.real() , cPt.real() ) ,
              std::max<double>( cRect.m_cRD.imag() , cPt.imag() ) ) ) ;
    return ret;
  };
public:
  cmplx m_cLT , m_cRD ; // left-upper and right-bottom corner points.
};

inline void SwapReIm(cmplx& Pt)
{
  swap(Pt._Val[_RE], Pt._Val[_IM]);
}

inline cmplx GetOrthoRight( cmplx Vect ) // 90 degrees to the right (cw)
{
  return Vect * cmplx( 0. , -1. ) ;
}
inline cmplx GetOrthoLeft( cmplx Vect )  // 90 degrees to the left (ccw)
{
  return Vect * cmplx( 0. , 1. ) ;
}
inline cmplx GetOrthoRightOnVF( cmplx Vect ) // 90 degrees to the right (cw)
{                                            // on video frame (Y is reversed)
  return Vect * cmplx( 0. , 1. ) ;
}
inline cmplx GetOrthoLeftOnVF( cmplx Vect )  // 90 degrees to the left (ccw)
{                                            // on video frame (Y is reversed)
  return Vect * cmplx( 0. , -1. ) ;
}
inline cmplx GetNormalized( cmplx Vect )
{
  return Vect / abs( Vect ) ;
}

inline cmplx StringToCmplx( FXString& Orig )
{
  cmplx Result ;
  FXString ForParse( Orig ) ;
  ForParse = ForParse.Trim( "()[]{}<> \t,;\r\n" ) ;
  Result._Val[ _RE ] = atof( (LPCTSTR) ForParse ) ;

  FXSIZE iPos = ForParse.FindOneOf( " ,\t" ) ;
  if ( iPos > 0 && (iPos < ForParse.GetLength()-1) )
    Result._Val[ _IM ] = atof( (LPCTSTR) ForParse + iPos + 1 ) ;
  return Result ;
}

inline FXString CmplxToString( cmplx& Value )
{
  FXString Result ;
  Result.Format( _T( "%g,%g" ) , Value.real() , Value.imag() ) ;
  return Result ;
}

inline CPoint GetCPoint( cmplx pt ) { return CPoint( ROUND( pt.real() ) , ROUND( pt.imag() ) ) ; }
inline cmplx GetCmplx( CPoint pt ) { return cmplx( ( double )pt.x , (double)pt.y ) ; }

inline double SquareSum( double dVal ) //https://en.wikipedia.org/wiki/Faulhaber%27s_formula
{
  return (dVal * (dVal+1.) * ((2. * dVal) + 1.)/6.) ;
}

inline double sign( double dVal )
{
  if ( dVal < 0. )
    return -1. ;
  if ( dVal > 0.0 )
    return 1.0 ;
  return 0.0 ;
}

inline void SetMinMax( int iVal , int& iMin , int& iMax )
{
  if ( iVal > iMax )
    iMax = iVal ;
  else if ( iVal < iMin )
    iMin = iVal ;
}

inline void SetMinMax(double dVal , double& dMin , double& dMax)
{
  if (dVal < dMin)
    dMin = dVal;
  if (dVal > dMax)
    dMax = dVal;
}

inline void SetToMax(double& dVal , double& dMax)
{
  if (dVal < dMax)
    dVal = dMax;
}

inline void SetToMin(double& dVal , double& dMin)
{
  if (dVal > dMin)
    dVal = dMin;
}

inline void SetMinMax(double dVal , double& dMin , double& dMax ,
  int iIndex , int& iMinIndex , int& iMaxIndex)
{
  if (dVal < dMin)
  {
    dMin = dVal;
    iMinIndex = iIndex;
  }
  if (dVal > dMax)
  {
    dMax = dVal;
    iMaxIndex = iIndex;
  }
}

inline void CmplxSetMinMax(cmplx cVal , cmplx& cMins , cmplx& cMaxes)
{
  SetMinMax(cVal.real() , cMins._Val[ _RE ] , cMaxes._Val[ _RE ]);
  SetMinMax(cVal.imag() , cMins._Val[ _IM ] , cMaxes._Val[ _IM ]);
}

inline void BOUND( int& x , int min , int max)
{
    if( x < min ) 
     x = (min) ;
    else if ( x > max ) 
      x = (max);
}

inline void Bound( double& dVal , double dMin , double dMax )
{
  if ( dVal < dMin )
    dVal = dMin ;
  if ( dVal > dMax )
    dVal = dMax ;
}

inline double SlidingWindow( double dNewVal , double& dAccum , int& iCnt , int iWidth )
{
  if ( iCnt == 0 )
  {
    iCnt++ ;
    return ( dAccum = dNewVal ) ;
  }
  if ( iCnt < iWidth )
  {
    dAccum += dNewVal ;
    return ( dAccum / ++iCnt ) ;
  }
  double dWidth = iWidth ;
  dAccum += dNewVal - ( dAccum / dWidth )   ;
  return dAccum / dWidth ;
}

inline cmplx SlidingWindow( cmplx cNewVal , cmplx& cAccum , int& iCnt , int iWidth )
{
  if ( iCnt == 0 )
  {
    iCnt++ ;
    return ( cAccum = cNewVal ) ;
  }
  if ( iCnt < iWidth )
  {
    cAccum += cNewVal ;
    return ( cAccum / (double)(++iCnt) ) ;
  }
  double dWidth = iWidth ;
  cAccum += cNewVal - (cAccum/dWidth)   ;
  return cAccum / dWidth ;
}

inline void GetMinMaxDbl( const double * pData , int iLen , double& dMin , double& dMax )
{
  const double * pEnd = pData + iLen ;
  while ( pData < pEnd )
    SetMinMax( *(pData++) , dMin , dMax ) ;
}

inline void GetMinMax( LPBYTE pData , int iLen , BYTE& byMin , BYTE& byMax )
{
  LPBYTE pEnd = pData + iLen ;
  while ( pData < pEnd )
  {
    if ( *pData < byMin )
      byMin = *pData ;
    if ( *pData > byMax )
      byMax = *pData ;
    pData++ ;
  }
}

// Find min and max on image vertical line : step is image width
inline void GetMinMax( LPBYTE pData , int iLen , 
  BYTE& byMin , BYTE& byMax , int iStep )
{
  LPBYTE pEnd = pData + iLen * iStep ;
  while ( pData < pEnd )
  {
    if ( *pData < byMin )
      byMin = *pData ;
    if ( *pData > byMax )
      byMax = *pData ;
    pData += iStep ;
  }
}

inline void GetMinMax( LPWORD pData , int iLen , WORD& Min , WORD& Max )
{
  LPWORD pEnd = pData + iLen ;
  while ( pData < pEnd )
  {
    if ( *pData < Min )
      Min = *pData ;
    if ( *pData > Max )
      Max = *pData ;
    pData++ ;
  }
}

// Find max position on image vertical slice : step is image width
inline LPBYTE GetMaxPos(  LPBYTE pData , int iLen ,
  BYTE& byMin , BYTE& byMax , int iStep )
{
  LPBYTE pEnd = pData + iLen * iStep ;
  LPBYTE pMax = pData ;
  while ( pData < pEnd )
  {
    if ( *pData < byMin )
      byMin = *pData ;
    if ( *pData > byMax )
    {
      byMax = *pData ;
      pMax = pData ;
    }
    pData += iStep ;
  }
  return pMax ;
}

inline LPWORD GetMaxPos( LPWORD pData , int iLen , WORD& Min , WORD& Max , int iStep )
{
  LPWORD pEnd = pData + iLen * iStep ;
  LPWORD pMax = pData ;
  while ( pData < pEnd )
  {
    if ( *pData < Min )
      Min = *pData ;
    if ( *pData > Max )
    {
      Max = *pData ;
      pMax = pData ;
    }
    pData += iStep ;
  }
  return pMax ;
}

// Find min and max on image vertical line (for 16 bits images): step is image width
inline void GetMinMax( LPWORD pData , int iLen , WORD& byMin , WORD& byMax , int iStep )
{
  LPWORD pEnd = pData + iLen * iStep ;
  while ( pData < pEnd )
  {
    if ( *pData < byMin )
      byMin = *pData ;
    if ( *pData > byMax )
      byMax = *pData ;
    pData += iStep ;
  }
}

inline void GetMinMax( int * pData , int iLen , int& Min , int& Max )
{
  int * pEnd = pData + iLen ;
  while ( pData < pEnd )
  {
    if ( *pData < Min )
      Min = *pData ;
    if ( *pData > Max )
      Max = *pData ;
    pData++ ;
  }
}

inline int GetMaxPos( double * pData , FXSIZE iLen , double& dMax )
{
  double * pEnd = pData + iLen ;
  double * p = pData ;
  FXSIZE iPos = 0 ;
  dMax = *p ;
  while (++p < pEnd)
  {
    if ( *p > dMax)
    {
      dMax = *p ;
      iPos = (p - pData) ;
    }
  }
  return ( int ) iPos ;
}

inline double GetThresByMinMax( double * pData , int iLen , double dRelThres )
{
  double dMin = DBL_MAX , dMax = -DBL_MAX ;
  GetMinMaxDbl( pData , iLen  , dMin , dMax ) ;
  double dAbsThres = dMin + dRelThres * ( dMax - dMin ) ;
  return dAbsThres ;
}

inline void GetDiffMinMaxDbl( double * pData , 
  int iLen , double& dDiffMin , double& dDiffMax ,
  double * pBufferForDiff = NULL )
{
  double * pEnd = pData + iLen - 2 ;
  while (pData < pEnd)
  {
    double dDiff = *( pData + 1 ) - *pData ;
    if ( pBufferForDiff )
      *( pBufferForDiff++ ) = dDiff ;

    if ( dDiff < dDiffMin)
      dDiffMin = dDiff ;
    if (dDiff > dDiffMax)
      dDiffMax = dDiff ;
    pData++ ;
  }
}

enum DiffThresMode
{
  DTM_FullAmpl = 0 ,
  DTM_PositiveOnly ,
  DTM_NegativeOnly
};

inline double GetDiffThresByMinMax( double * pData , 
  int iLen , double dRelThres , 
  DiffThresMode Mode = DTM_PositiveOnly ,
  double * pBufferForDiff = NULL )
{
  double dMin = DBL_MAX , dMax = -DBL_MAX ;
  GetDiffMinMaxDbl( pData , iLen , dMin , dMax , pBufferForDiff ) ;
  switch ( Mode )
  {
    case DTM_FullAmpl: return dMin + dRelThres * ( dMax - dMin ) ;
    case DTM_PositiveOnly: return dRelThres * dMax ; // always positive 
    case DTM_NegativeOnly: return dRelThres * dMin ; // always negative
  }
  return 0. ;
}

class Segment1d
{
public:
  double m_dBegin , m_dEnd ; 

  Segment1d( double dBegin = 0. , double dEnd = 0. )
  {
    m_dBegin = dBegin , m_dEnd = dEnd ;
  }

  Segment1d( const Segment1d& Other )
  {
    *this = Other ;
  }

//Segment1d operator=( const Segment1d& Other )
//{
//  *this = Other;
//  return *this;
//}
  bool IsInSegment( const double dVal )
  {
    return ((m_dBegin <= dVal) && (dVal <= m_dEnd)) ;
  }

  bool IsOverlapped( const Segment1d& Other ) const
  {
    return (Other.m_dBegin <= m_dEnd) && (m_dBegin <= Other.m_dEnd) ;
  }
  
  bool bIsInside( const Segment1d& Other ) const
  {
    return ((m_dBegin <= Other.m_dBegin) && (Other.m_dEnd <= m_dEnd)) ;
  }
  bool Union( const Segment1d& Other )
  {
    if ( IsOverlapped( Other ) )
    {
      m_dBegin = min( m_dBegin , Other.m_dBegin ) ;
      m_dEnd = max( m_dEnd , Other.m_dEnd ) ;
      return true ;
    }
    return false ;
  }

  bool Intersection(const  Segment1d& Other )
  {
    if ( IsOverlapped( Other ) )
    {
      m_dBegin = max( m_dBegin , Other.m_dBegin ) ;
      m_dEnd = min( m_dEnd , Other.m_dEnd ) ;
      return true ;
    }
    return false ;
  }
  double GetLength() const { return (m_dEnd - m_dBegin); }

  void Offset( double dOffset )
  {
    m_dBegin += dOffset;
    m_dEnd += dOffset;
  }
} ;

typedef vector<Segment1d> Segments1d;

inline double RetBound( double dVal , double dMin , double dMax )
{
  if ( dVal < dMin )
    return dMin ;
  if ( dVal > dMax )
    return dMax ;
  return dVal ;
}

inline void swapdouble( double& a , double& b )
{
  double tmp = a ;
  a = b ;
  b = tmp ;
}

inline void SwapRects( RECT& a , RECT& b )
{
  RECT tmp = a ;
  a = b ;
  b = tmp ;
}

inline void swapcmplx(cmplx& a, cmplx& b)
{
	cmplx tmp = a;
	a = b;
	b = tmp;
}

// this works in condition, that iBegin < iArrayLen && iEnd < iArrayLength
inline int GetLengthOnRing( int iBegin , int iEnd , int iArrayLength )
{
  if ( iEnd >= iBegin )
    return iEnd - iBegin ;
  else
    return iEnd + ( iArrayLength - iBegin ) ;
}
// this works in condition, that iBegin < iArrayLen && iEnd < iArrayLength
// ends are included
inline bool InRangeOnRing( const int iIndex , const int iBegin , 
  const int iEnd , const int iArrayLength )
{
  if ( iEnd >= iBegin )
    return ( iBegin <= iIndex ) && ( iIndex <= iEnd ) ;
  return ( !(( iIndex <= iBegin ) && ( iIndex >= iEnd ))  ) ;
}

inline FXSIZE GetIndexInRing( const int iIndex , const FXSIZE iArrayLength )
{
  return ( iIndex + iArrayLength ) % iArrayLength ;
}

inline bool IsInRange( double dVal , 
  double dMin , double dMax )
{
  return ( (dMin <= dVal) && (dVal < dMax) ) ;
}

inline bool IsInRange( cmplx cVal ,
  cmplx cMin , cmplx cMax )
{
  return ( IsInRange( cVal.real() , cMin.real() , cMax.real() )
        && IsInRange( cVal.imag() , cMin.imag() , cMax.imag() ) );
}

// tolerance is relative to max
// Tolerance is positive number
inline bool IsInTolerance( double dVal1 , double dVal2 , double dTolerance ) 
{
  if ( dVal1 > EPSILON )
    return fabs( (dVal1 - dVal2) / dVal1 ) <= dTolerance ;
  if ( dVal2 > EPSILON )
    return fabs( (dVal1 - dVal2) / dVal2 ) <= dTolerance ;
  return dTolerance > EPSILON ;
}

inline double GetStd( double * pData , int iLen ,
  double * pMean = NULL )
{
  if ( iLen < 2 )
    return 0. ;

  double dMean ;
  double *pEnd = pData + iLen;
  double *p = pData ;
  if ( !pMean )
  {
    dMean = 0. ;
    do
    {
      dMean += *(p++) ;
    } while ( p < pEnd );
    dMean /= iLen ;
    p = pData ;
  }
  else
    dMean = *pMean ;
  double dAcc2 = 0. ;
  do
  {
    double dDiff = fabs( *p - dMean ) ;
    dAcc2 += dDiff * dDiff ;
  } while ( ++p < pEnd );
  dAcc2 /= iLen - 1 ;
  return sqrt( dAcc2 ) ;
}

inline double GetStd( double * pData , int iLen , 
  int iStep = sizeof(double) , double * pMean = NULL )
{
  if ( iLen < 2 )
    return 0. ;

  double dMean ;
  LPBYTE pEnd = ((LPBYTE) pData) + iLen * iStep;
  LPBYTE p = (LPBYTE)pData ;
  if ( !pMean )
  {
    dMean = 0. ;
    do
    {
      dMean += *((double*)p) ;
    } while ( (p += iStep) < pEnd );
    dMean /= iLen ;
    p = (LPBYTE) pData ;
  }
  double dAcc2 = 0. ;
  do
  {
    double dDiff = fabs(*((double*)p)) ;
    dAcc2 += dDiff * dDiff ;
  } while ( (p += iStep) < pEnd );
  dAcc2 /= iLen - 1 ;
  return sqrt( dAcc2 ) ;
}

inline double GetThresPosition( double dVal1 , double dVal2 , double dThres )
{
  if ( dVal1 == dVal2 )
    return (dThres == dVal1) ? dThres : 0 ;
  if ( dVal1 <= dThres )
  {
    if ( dThres <= dVal2 )
    {
      double dPos = (dThres - dVal1) / (dVal2 - dVal1) ;
      return dPos ;
    }
    return (dVal1 < dVal2) ? 1. : 0. ;
  }
  if ( dVal2 <= dThres )
  {
    double dPos = (dVal1 - dThres) / (dVal1 - dVal2) ;
    return dPos ;
  }
  return (dVal1 > dVal2) ? 1. : 0. ;
}

inline bool isdecimal( LPCTSTR p )
{
  if ( !p || (*p == 0) )
    return false ;
  LPCTSTR pStr = p ;
  do
  {
    if ( (*pStr < _T('0')) || (*pStr > _T('9')) )
    {
      if ( ((*pStr == _T( '-' )) || (*pStr == _T( '+' )))
        && p != pStr        )
        return false ;
    }
  } while ( *(++pStr) != 0 ) ;
  
  return true;
}

inline bool isfloat( LPCTSTR pStr )
{
  if ( !pStr || (*pStr == 0) )
    return false ;

  int iNPoints = 0 ;
  int iNPowerLetters = 0 ;
  do
  {
    if ( (*pStr < _T( '0' )) || (*pStr > _T( '9' )) )
    {
      if ( *pStr == _T('.')  )
      {
        if ( iNPowerLetters || ++iNPoints > 1 )
          return false ;
        else
          continue ;
      }
      if ( *pStr == _T( 'e' ) 
        || *pStr == _T( 'E' )
        || *pStr == _T( 'g' )
        || *pStr == _T( 'G' )
        )
      {
        if ( ++iNPowerLetters > 1 )
          return false ;
        else
          continue ;
      }
      return false ;
    }
  } while ( *(++pStr) != 0 ) ;

  return true;
}

__forceinline bool ConvToBinary( LPCTSTR strIn , FXSIZE& iResult )
{
  FXString s( strIn ); s.Trim() ;
  if ( s[ 0 ] == _T( '(' )
    || s[ 0 ] == _T( '[' )
    || s[ 0 ] == _T( '{' ) )
    s.Delete( 0 ) ;
  if ( s[ 0 ] == _T( '0' ) )
  {
    if ( _totlower( s[ 1 ] ) == _T( 'x' ) )
    {
      FXSIZE dwRes ;
      s = s.Mid( 2 ) ;
    #ifdef _WIN64
      if ( _stscanf_s( s , _T( "%llx" ) , &dwRes ) )
      #else
      if ( _stscanf_s( s , _T( "%x" ) , &dwRes ) )
      #endif
      {
        iResult = dwRes ;
        return true ;
      }
      return false ;
    }
    else if ( _totlower( s[ 1 ] ) == _T( 'b' ) )
    {
      FXSIZE dwRes = 0 ;
      s = s.Mid( 2 ) ;
      while ( !s.IsEmpty() )
      {
        switch ( s[ 0 ] )
        {
          case _T( '1' ): dwRes++ ;
          case _T( '0' ): dwRes <<= 1 ;
            s.Delete( 0 ) ;
            break ;
          default: s.Empty() ; break ;
        }
      }
      iResult = dwRes ;
      return true ;
    }
    else if ( isdigit( s[ 1 ] ) )
    {
      FXSIZE dwRes ;
      s = s.Mid( 1 ) ;
      if ( _stscanf_s( s ,
      #ifdef _WIN64
        _T( "%llo" ) ,
      #else
        _T( "%o" ) ,
      #endif
        &dwRes ) )
      {
        iResult = dwRes ;
        return true ;
      }
      return false ;
    }
  }
  if ( isdigit( s[ 0 ] ) )
  {
    iResult = _tstoi( s ) ;
    return true ;
  }
  return false ;
}

__forceinline FXSIZE ConvToBinary( LPCTSTR strIn )
{
  FXSIZE iResult = 0 ;
  ConvToBinary( strIn , iResult ) ;
  return iResult ;
}

class ImgMoments
{
public:
  double m_dAreaPixels ;
  double m_dM00; // weighted sum
  double m_dM01; // Weight * X
  double m_dM10; // Weight * Y
  double m_dM11; // Weight * X * Y
  double m_dM02; // Weight * X^2
  double m_dM20; // Weight * Y^2
  double m_dValMin ;
  double m_dValMax ;

  double m_dXMin ;
  double m_dXMax ;
  double m_dYMin ;
  double m_dYMax ;

  ImgMoments() { Reset() ; }
  inline void Reset() 
  { 
    memset( &m_dAreaPixels , 0 , (LPBYTE)&m_dValMin - (LPBYTE)&m_dAreaPixels ) ; 
    m_dXMin = m_dYMin = m_dValMin = DBL_MAX ;
    m_dXMax = m_dYMax = m_dValMax = -DBL_MAX ;
  }
  cmplx GetCenter()
  {
    if ( m_dM00 > 0. )
    {
      cmplx Res( m_dM01/m_dM00 , m_dM10 / m_dM00 ) ;
      return Res ;
    }
    return cmplx(0.,0.) ;
  }
  double GetWidth() { return m_dXMax - m_dXMin ; }
  double GetHeight() { return m_dYMax - m_dYMin ; }
  double GetRange() { return m_dValMax - m_dValMin ; }
  inline double GetAngle()
  {
    double dAngle = 0. ;
    if ( m_dM00 )
    {
      double dXCent = m_dM01/m_dM00 ;
      double dYCent = m_dM10/m_dM00 ;

      double dMu11 = m_dM11 - dXCent * m_dM10 ;
      double dMu20 = m_dM02 - dXCent * m_dM01 ;
      double dMu02 = m_dM20 - dYCent * m_dM10 ;

      double dMuCov20 = dMu20/m_dM00 ;
      double dMuCov02 = dMu02/m_dM00 ;
      double dMuCov11 = dMu11/m_dM00 ;
      dAngle = 0.5 * atan2( (2 * dMuCov11) , (dMuCov20 - dMuCov02))	;
    }
    return dAngle ;
  }
  inline void Add( ImgMoments& Other )
  {
    double * pThis = (double*)this ;
    double * pOther = (double*)&Other ;
    for ( int i = 0 ; i < 7 ; i++ )
      *(pThis++) += *(pOther++) ;
    for ( int i = 0 ; i < 3 ; i++ )
    {
      if ( *pThis > *pOther )  // check min and update
        *pThis = *pOther ;
      ++pThis ; ++pOther ;
      if ( *pThis < *pOther )  // Check max and update
        *pThis = *pOther ;
      ++pThis ; ++pOther ;
    }
  }
  inline void Add( double dX , double dY , double dVal )
  {
    m_dAreaPixels++ ;
    m_dM00 += dVal ;
    m_dM01 += dVal * dX ;
    m_dM10 += dVal * dY ;
    m_dM11 += dVal * dX * dY ;
    m_dM02 += dVal * dX * dX ;
    m_dM20 += dVal * dY * dY ;

    if ( dX < m_dXMin )
      m_dXMin = dX ;
    if ( dX > m_dXMax)
      m_dXMax = dX ;
    if ( dY < m_dYMin )
      m_dYMin = dY ;
    if ( dY > m_dYMax )
      m_dYMax = dY ;
    if ( dVal < m_dValMin )
      m_dValMin = dVal ;
    if ( dVal > m_dValMax )
      m_dValMax = dVal ;
  }

  inline void Add( cmplx& Pt , double dVal )
  {
    Add( Pt.real() , Pt.imag() , dVal ) ;
  }

  inline void Add( int iX , int iY , double dVal )
  {
    Add( (double)iX , (double)iY , dVal ) ;
  }

  inline void Add( int iX , int iY )
  {
    m_dAreaPixels++ ;
    m_dM00 += 1.0 ;
    m_dM01 += iX ;
    m_dM10 += iY ;
    m_dM11 += iX * iY ;
    m_dM02 += iX * iX ;
    m_dM20 += iY * iY ;
    if ( iX < m_dXMin )
      m_dXMin = iX ;
    if ( iX > m_dXMax)
      m_dXMax = iX ;
    if ( iY < m_dYMin )
      m_dYMin = iY ;
    if ( iY > m_dYMax )
      m_dYMax = iY ;
  }

  inline void Add( int iXBeg , int iXEnd , int iY ) // no weight
  {
    double dLen = iXEnd - iXBeg + 1 ;
    double dSumX = 0.5 * dLen * (iXEnd + iXBeg) ;
    m_dAreaPixels += dLen ;
    m_dM00 += dLen ;
    m_dM01 += dSumX ;
    m_dM10 += iY * dLen ;
    m_dM11 += dSumX * iY ;
    m_dM02 += SquareSum( iXEnd ) - SquareSum( iXBeg - 1. ) ;
    m_dM20 += iY * iY * dLen ;
    if ( iXBeg < m_dXMin )
      m_dXMin = iXBeg ;
    if ( iXEnd > m_dXMax)
      m_dXMax = iXEnd ;
    if ( iY < m_dYMin )
      m_dYMin = iY ;
    if ( iY > m_dYMax )
      m_dYMax = iY ;
  }
  FXString& ToString( bool bNamed = false )
  {
    FXString Out ;
    if ( bNamed )
    {
      Out.Format( _T( "M00=%g;M01=%g;M10=%g;M11=%g;M02=%g;M20=%g;" ) ,
        m_dM00 , m_dM01 , m_dM10 , m_dM11 , m_dM02 , m_dM20 ) ;
    }
    else
    {
      Out.Format( _T( "%g,%g,%g,%g,%g,%g" ) ,
        m_dM00 , m_dM01 , m_dM10 , m_dM11 , m_dM02 , m_dM20 ) ;
    }
    return Out ;
  }
  bool FromString( LPCTSTR pValue )
  {
    int iNValues = 0 ;
    if ( strchr( pValue , _T(';') ) ) // formatted as named for PropertyKit
    {
      FXPropertyKit pk( pValue ) ;
      
      iNValues += pk.GetDouble( _T( "M00" ) , m_dM00 ) ;
      iNValues += pk.GetDouble( _T( "M01" ) , m_dM01 ) ;
      iNValues += pk.GetDouble( _T( "M10" ) , m_dM10 ) ;
      iNValues += pk.GetDouble( _T( "M11" ) , m_dM11 ) ;
      iNValues += pk.GetDouble( _T( "M02" ) , m_dM02 ) ;
      iNValues += pk.GetDouble( _T( "M20" ) , m_dM20 ) ;
    }
    else // CSV
    {
      iNValues = sscanf_s( pValue , _T( "%lg,%lg,%lg,%lg,%lg,%lg" ) ,
        &m_dM00 , &m_dM01 , &m_dM10 , &m_dM11 , &m_dM02 , &m_dM20 ) ;
    }
    return (iNValues == 6) ;
  }
};

// Normalization to +-PI range
inline double NormRad( double dAngle_rad )
{
  return ( fmod(dAngle_rad + M_PI , M_2PI) - M_PI ) ;
}
// Normalization to 0-2*PI range
inline double NormTo2PI( double dAngle_rad )
{
  return fmod( dAngle_rad + M_2PI , M_2PI ) ;
}
  // get angle from dAng1 to dAng2; ccw is positive
  // i.e. Ang2 = Result + dAng1
inline double GetDeltaAngle( double dAng1 , double dAng2 )
{
  double dResult = (dAng2 - dAng1) ;
  if ( dAng1 <= M_PI )
  {
    if ( dAng2 < dAng1 + M_PI )
      return dResult ; // will be positive from zero to M_PI (ccw)
    else
      return ( dResult - M_2PI ) ;// will be negative from -M_PI to zero (cw)
  }
  else
  {
    if ( dAng2 + M_PI < dAng1 )
      return -( M_2PI - dResult ) ; // will be positive from zero to M_PI (ccw)
    else
      return dResult ; // will be negative from -M_PI to zero (cw)
  }
}
inline double AbsDeltaAngle( double dAng1 , double dAng2 )
{
  double dDelta = NormTo2PI(dAng2) - NormTo2PI(dAng1) ;
  if ( dDelta < 0. )
    dDelta = -dDelta ;
  if ( dDelta > M_PI )
    dDelta = M_2PI - dDelta ;
  return fabs( dDelta ) ;
}
inline double SnapToNet( double x , double step )
{
  if ( x >= 0. )
    return ( step * ( int )( ( x+step*0.5 )/step )  );
  else
    return ( -step * ( int )( ( -x+step*0.5 )/step )  );
}

inline int clamp( int iIndex , int iMin ,int iMax )
{
  iIndex = max( iIndex , iMin ) ;
  iIndex = min( iIndex , iMax ) ;
  return iIndex ;
}


inline int GetBitsCount( DWORD dwNumb )
{
  int iCnt = 0 ;
  while ( dwNumb )
  {
    iCnt++ ;
    dwNumb &= dwNumb - 1 ;
  }
  return iCnt ;
}
inline double GetDist( double x1 , double y1 , double x2 , double y2 )
{
  return sqrt( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) ) ; 
}

inline CSize GetStringSizeOnRenderInPix( LPCTSTR pString , int iFontSize )
{
  int iStringLenChar = (int)strlen( pString );
  CSize QuestionSize_pix( ROUND( iFontSize * 1.2 * iStringLenChar ) , ROUND( iFontSize * 1.5 ) ) ;

  return QuestionSize_pix ;
}

class CCoordinate 
{
public:

  CCoordinate() { Reset() ; };

  CCoordinate (double x, double y, double z = 0, double theta = 0)
  {
    m_x = x;
    m_y = y;
    m_z = z;
    m_theta = theta;
  };

  CCoordinate( cmplx& Orig )
  {
    m_x = Orig.real() ;
    m_y = Orig.imag() ;
    m_z = m_theta = 0. ;
  }

  void Reset() { memset( this , 0 , sizeof( *this ) ) ; }
  ~CCoordinate ()  {};

  // this method does a two dimensional shift, in the XY plane of
  // point B relative to point A (this).  Z and Theta of the
  // A point remain unchanged.

  CCoordinate ShiftXY( CCoordinate b )
  {
    CCoordinate c = *this;

    c.m_x += b.m_x;
    c.m_y += b.m_y;

    return (c);
  };

  CCoordinate ShiftXYZ( CCoordinate b )
  {
    CCoordinate c = *this;

    c.m_x += b.m_x;
    c.m_y += b.m_y;
    c.m_z += b.m_z ;
    return (c);
  };
  CCoordinate MinusXY( CCoordinate b )
  {
    CCoordinate c = *this;

    c.m_x -= b.m_x;
    c.m_y -= b.m_y;

    return (c);
  };

  CCoordinate MinusXYZ( CCoordinate b )
  {
    CCoordinate c = *this;

    c.m_x -= b.m_x;
    c.m_y -= b.m_y;
    c.m_z -= b.m_z ;
    return (c);
  };

  // this method takes point B and places it in the
  // coordinate system of A (this).  To do this we rotate B about B's
  // origin by the amount A.theta.   Then change B.theta
  // to reflect the rotation.  The rotation is in the XY plane
  // so it has no effect on Z.  Then translate by A's
  // location in three space.

  CCoordinate ShiftXYZT (CCoordinate b)
  {
    CCoordinate c = b.Rotate (m_theta);

    c.m_x += m_x;
    c.m_y += m_y;
    c.m_z += m_z;

    return (c);
  };



  //BOGUS fix the geometry...
  CCoordinate& operator/=(const double s)
  {
    m_x     = m_x / s;
    m_y     = m_y / s;
    //m_z     = m_z / s;
    //m_theta = m_theta / s;

    return (*this);
  };

  CCoordinate& operator-=( CCoordinate b )
  {
    m_x -= b.m_x ;
    m_y -= b.m_y ;
    m_z -= b.m_z ;
    //m_theta -= b.m_theta ;
    return *this ;
  }
  CCoordinate& operator-=( cmplx b )
  {
    m_x -= b.real() ;
    m_y -= b.imag() ;
    return *this ;
  }

  static double cabs( const CCoordinate& a )
  {
    return sqrt( a.m_x * a.m_x + a.m_y * a.m_y + a.m_z * a.m_z ) ;
  }

  double cabs() const
  {
    return cabs( *this ) ;
  }

  bool operator==(const CCoordinate& s)
  {
    return ((m_x == s.m_x) && 
      (m_y == s.m_y) &&
      (m_z == s.m_z) &&
      (m_theta == s.m_theta));
  };

  bool operator!=(const CCoordinate& s)
  {
    return ((m_x != s.m_x) ||
      (m_y != s.m_y) ||
      (m_z != s.m_z) ||
      (m_theta != s.m_theta));
  };


  DWORD Compare(const CCoordinate& s , double dToleranceCoord , double dTolTheta = 0 )
  {
    DWORD result = (fabs(m_x - s.m_x) > dToleranceCoord) 
      | ((fabs(m_y - s.m_y) > dToleranceCoord) << 1)
      | ((fabs(m_z - s.m_z) > dToleranceCoord) << 2) ;
    if( dTolTheta > 0.)
      result |= (fabs(m_theta - s.m_theta) > dTolTheta) << 3;
    return result ;
  };
  CCoordinate Rotate (double theta)
  {
    CCoordinate c;

    double cos_theta = cos ( - theta);
    double sin_theta = sin ( - theta);

    c.m_x = (   m_x * cos_theta) + (m_y * sin_theta);
    c.m_y = ( - m_x * sin_theta) + (m_y * cos_theta);

    c.m_z = m_z;

    c.m_theta = m_theta + theta;

    return (c);
  };
  operator cmplx()  { return cmplx( m_x , m_y ) ; }
  CCoordinate& operator =( cmplx Orig )
  {
    Reset() ;
    m_x = Orig.real() ;
    m_y = Orig.imag() ;
    return *this ;
  }
  bool ToString( LPTSTR Buf , int iLen , LPCTSTR pFormat = NULL )
  {
    int iResult = _stprintf_s( Buf , iLen , pFormat ? pFormat : _T( "%g,%g,%g,%g" ) ,
      m_x , m_y , m_z , m_theta ) ;
    return (iResult < iLen) ;
  }
  FXString& ToString( 
    FXString& AsText , LPCTSTR pFormat = NULL )
  {
    AsText.Format( 
      pFormat ? pFormat : _T( "%g,%g,%g,%g" ) ,
      m_x , m_y , m_z , m_theta ) ;
    return AsText ;
  }

  int FromString( LPCTSTR Buf )
  {
    Reset() ;
    int iResult = _stscanf_s( Buf , _T("%lg,%lg,%lg,%lg") ,
      &m_x , &m_y , &m_z , &m_theta ) ;
    return iResult ;
  }
  bool IsZero()
  {
    return ((m_x == 0.) && (m_y == 0.) && (m_z == 0.) && (m_theta==0.)) ;
  }

  double      m_x;
  double      m_y;
  double      m_z;
  double      m_theta;
};
class CCoord3
{
public:

  CCoord3 ()
  {
    m_x = m_y = m_z = 0.0;
  };

  CCoord3 (double x, double y, double z = 0)
  {
    m_x = x;
    m_y = y;
    m_z = z;
  };

  ~CCoord3 ()  {};

  // this method does a two dimensional shift, in the XY plane of
  // point B relative to point A (this).  Z and Theta of the
  // A point remain unchanged.

  CCoord3 ShiftXY (CCoord3 b)
  {
    CCoord3 c = *this;

    c.m_x += b.m_x;
    c.m_y += b.m_y;

    return (c);
  };

  // this method takes point B and places it in the
  // coordinate system of A (this).  To do this we rotate B about B's
  // origin by the amount A.theta.   Then change B.theta
  // to reflect the rotation.  The rotation is in the XY plane
  // so it has no effect on Z.  Then translate by A's
  // location in three space.

  CCoord3 ShiftXYZ (CCoord3 b)
  {
    CCoord3 c = *this;

    c.m_x += b.m_x;
    c.m_y += b.m_y;
    c.m_z += b.m_z;

    return (c);
  };

  CCoord3 operator +(CCoord3 b)
  {
    CCoord3 c = *this;

    c.m_x += b.m_x;
    c.m_y += b.m_y;
    c.m_z += b.m_z;

    return (c);
  };


  //BOGUS fix the geometry...
  CCoord3& operator*=(const double s)
  {
    m_x     = m_x * s;
    m_y     = m_y * s;
    m_z     = m_z * s;
    return (*this);
  };
  CCoord3 operator*(const double s)
  {
    CCoord3 c = *this;
    c.m_x     = m_x * s;
    c.m_y     = m_y * s;
    c.m_z     = m_z * s;
    return (c);
  };

  CCoord3 operator-(const CCoord3& b)
  {
    CCoord3 c = *this;

    c.m_x -= b.m_x;
    c.m_y -= b.m_y;
    c.m_z -= b.m_z;

    return (c);
  };
  CCoord3 operator-=(const CCoord3& b)
  {
    m_x -= b.m_x;
    m_y -= b.m_y;
    m_z -= b.m_z;

    return (*this);
  };


  CCoord3& operator/=(const double s)
  {
    m_x     = m_x / s;
    m_y     = m_y / s;
    m_z     = m_z / s;
    return (*this);
  };


  bool operator==(const CCoord3& s)
  {
    return ((m_x == s.m_x) && 
      (m_y == s.m_y) &&
      (m_z == s.m_z) );
  };

  bool operator!=(const CCoord3& s)
  {
    return ((m_x != s.m_x) ||
      (m_y != s.m_y) ||
      (m_z != s.m_z) );
  };


  DWORD Compare(const CCoord3& s , double dToleranceCoord )
  {
    DWORD result = (fabs(m_x - s.m_x) > dToleranceCoord) 
      | ((fabs(m_y - s.m_y) > dToleranceCoord) << 1)
      | ((fabs(m_z - s.m_z) > dToleranceCoord) << 2) ;
    return result ;
  };
  CCoord3 Rotate (double theta)
  {
    CCoord3 c;

    double cos_theta = cos ( - theta);
    double sin_theta = sin ( - theta);

    c.m_x = (   m_x * cos_theta) + (m_y * sin_theta);
    c.m_y = ( - m_x * sin_theta) + (m_y * cos_theta);

    c.m_z = m_z;

    return (c);
  };

  bool ToString( LPTSTR Buf , int iLen , LPCTSTR pFormat = NULL )
  {
    int iResult = _stprintf_s( Buf , iLen , pFormat ? pFormat : _T( "%g,%g,%g" ) ,
      m_x , m_y , m_z  ) ;
    return (iResult < iLen) ;
  }

  int FromString( LPCTSTR Buf )
  {
    memset( this , 0 , sizeof( *this ) ) ;
    int iResult = _stscanf_s( Buf , _T( "%lg,%lg,%lg" ) ,
      &m_x , &m_y , &m_z ) ;
    return iResult ;
  }
  CCoordinate& operator =( cmplx Orig )
  {
    m_x = Orig.real() ;
    m_y = Orig.imag() ;
    m_z = 0. ;
  }

  bool IsZero()
  {
    return ((m_x == 0.) && (m_y == 0.) && (m_z == 0.)) ;
  }
  double      m_x;
  double      m_y;
  double      m_z;
} ;

class CCoor2
{
public:

  CCoor2 () {  m_x = m_y = 0.0; };
  CCoor2 (double x, double y)  {
    m_x = x;
    m_y = y;
  };

  ~CCoor2 () {};

  CCoor2 ( CCoordinate& s) 
  {
    m_x=s.m_x;
    m_y=s.m_y;
  }

  CCoor2 ( CCoord3& s) 
  {
    m_x=s.m_x;
    m_y=s.m_y;
  }

  //BOGUS fix the geometry...
  CCoor2 operator+(const CCoor2& s) 
  {
    CCoor2 c;

    c.m_x     = m_x + s.m_x;
    c.m_y     = m_y + s.m_y;

    return (c);
  };

  //BOGUS fix the geometry...
  CCoor2 operator-(const CCoor2& s) 
  {
    CCoor2 c;

    c.m_x     = m_x - s.m_x;
    c.m_y     = m_y - s.m_y;

    return (c);
  };

  //BOGUS fix the geometry...
  CCoor2 operator+=(const CCoor2& s) 
  {
    m_x += s.m_x;
    m_y += s.m_y;

    return (*this);
  };

  //BOGUS fix the geometry...
  CCoor2 operator-=(const CCoor2& s) 
  {
    m_x -= s.m_x;
    m_y -= s.m_y;

    return (*this);
  };

  CCoor2& operator=(const CCoor2& s) 
  {
    m_x     = s.m_x;
    m_y     = s.m_y;
    return (*this);
  };
  double      Dist( CCoor2& p )           
  {
    return ( sqrt( sq( p ) ) );
  }
  double      sq( CCoor2& p )
  {
    double dx = p.m_x - m_x;
    double dy = p.m_y - m_y;
    return ( dx*dx + dy*dy );
  }
  bool IsZero()
  {
    return ((m_x == 0.) && (m_y == 0.) ) ;
  }

  double      m_x;
  double      m_y;
};

class CCoor3t 
{
public:
  CCoor3t() { Reset(); };
  CCoor3t (double x, double y, double z = 0, double time = 0)
  {
    m_x = x;
    m_y = y;
    m_z = z;
    m_time = time;
  };

  CCoor3t( CCoordinate Coord )
  {
    m_x = Coord.m_x;
    m_y = Coord.m_y;
    m_z = Coord.m_z;
    m_time = 0. ;
  }
 
  ~CCoor3t ()  {};

  void Reset() { m_x = m_y = m_z = m_time = 0.0; };
  // this method does a two dimensional shift, in the XY plane of
  // point B relative to point A (this).  Z and Theta of the
  // A point remain unchanged.
  CCoor3t ShiftXY (CCoor3t b)
  {
    CCoor3t c = *this;
    c.m_x += b.m_x;
    c.m_y += b.m_y;
    return (c);
  };

  CCoor3t& operator=( CCoordinate Coord )
  {
    m_x = Coord.m_x;
    m_y = Coord.m_y;
    m_z = Coord.m_z;
    m_time = 0. ;
    return *this ;
  }

  //BOGUS fix the geometry...
  CCoor3t& operator/=(const double s)
  {
    m_x     = m_x / s;
    m_y     = m_y / s;
    //m_z     = m_z / s;
    //m_theta = m_theta / s;

    return (*this);
  };
  CCoor3t& operator += (const CCoor3t& Add)
  {
    m_x     += Add.m_x ;
    m_y     += Add.m_y ;
    m_z     += Add.m_z ;

    return (*this);
  };

  bool operator==(const CCoor3t& s)
  {
    return ( (m_x == s.m_x) && 
      (m_y == s.m_y) &&
      (m_z == s.m_z) 
      );
  };

  DWORD Compare(const CCoor3t& s , double dToleranceCoord )
  {
    DWORD result = (fabs(m_x - s.m_x) > dToleranceCoord) 
      | ((fabs(m_y - s.m_y) > dToleranceCoord) << 1)
      | ((fabs(m_z - s.m_z) > dToleranceCoord) << 2) ;
    return result ;
  };

  bool operator!=(const CCoor3t& s)
  {
    return ((m_x != s.m_x) ||
      (m_y != s.m_y) ||
      (m_z != s.m_z) 
      );
  };

  CCoor3t GetSpeed( const CCoor3t& s )
  {
    CCoor3t Speed ;
    double dt = m_time - s.m_time ;
    if ( dt != 0.0 )
    {
      Speed.m_x = ( m_x - s.m_x ) / dt ;
      Speed.m_x = ( m_y - s.m_y ) / dt ;
      Speed.m_x = ( m_z - s.m_z ) / dt ;
      Speed.m_time = dt ;
    }
    return Speed ;
  }
  double abs( CCoor3t& Targ )
  {
    double dD1 = m_x - Targ.m_x ;
    double dD2 = m_y - Targ.m_y ;
    double dD3 = m_z - Targ.m_z ;
    return sqrt( dD1*dD1 + dD2*dD2 + dD3*dD3 ) ;
  }

  bool IsZero()
  {
    return ((m_x == 0.) && (m_y == 0.) && (m_z == 0.)) ;
  }


  double      m_x;
  double      m_y;
  double      m_z;
  double      m_time;
};

typedef FXArray<CCoor3t,CCoor3t&> CoorTimeArray ;
typedef FXArray<CPoint,CPoint&> PointArray ;

class MultiCoord
{
public:
  MultiCoord() { m_dRobotTime = m_dReceiveTime = 0. ; } ;
  ~MultiCoord() {} ;

  void AddCoord( double value ) { m_Coords.Add(value) ; } ;
  double GetCoord( int iIndex ) 
  {
    if ( iIndex >= 0  &&  iIndex < (int) m_Coords.GetCount() )
      return m_Coords[iIndex] ; 
    else
    {
      ASSERT(0) ;
      return 1e30 ;
    }
  } ;
  void SetCoord( int iIndex , double dVal) 
  {
    if ( iIndex >= 0  &&  iIndex < (int) m_Coords.GetCount() )
      m_Coords[iIndex] = dVal ; 
    else
    {
      do 
      {
        AddCoord( 1e30 ) ;
      } while ( (int) m_Coords.GetUpperBound() < iIndex ) ;
      m_Coords[iIndex] = dVal ;
    }
  } ;
  void SetRobotTime( double dTime ) { m_dRobotTime = dTime ; } ;
  double GetRobotTime() { return m_dRobotTime ; } ;
  void SetReceiveTime( double dTime ) { m_dReceiveTime = dTime ; } ;
  double GetReceiveTime() { return m_dReceiveTime ; } ;
  void   Clear() { m_Coords.RemoveAll() ; } ;
  int GetCount() { return (int)m_Coords.GetCount(); }
  int RemoveAt( int iPos , int iNForDelete = 1 ) 
  { 
    if ( iPos < (int) m_Coords.GetCount()  && iPos >= 0 )
    {
      if ( iNForDelete + iPos > ( int )m_Coords.GetCount() )
        iNForDelete = (int)m_Coords.GetCount() - iPos ;
      m_Coords.RemoveAt( iPos , iNForDelete ) ;
      return (int)m_Coords.GetCount() ;
    }
    return -1 ;
  }
  double abs( MultiCoord& Patt , 
    int iAxis1 , int iAxis2 , int iAxis3 )
  {
    int iMaxAxis = max( iAxis2 , iAxis1 ) ;
    iMaxAxis = max( iMaxAxis , iAxis3 ) ;
    if ( iMaxAxis >= (int) m_Coords.GetCount()
      || iMaxAxis >= (int) m_Coords.GetCount() )
      return FALSE ;
    double dD1 = m_Coords[iAxis1] - Patt.m_Coords[iAxis1] ;
    double dD2 = m_Coords[iAxis2] - Patt.m_Coords[iAxis2] ;
    double dD3 = m_Coords[iAxis3] - Patt.m_Coords[iAxis3] ;
    return sqrt( dD1*dD1 + dD2*dD2 + dD3*dD3 ) ;
  } ;
  double abs( MultiCoord& Patt )
  {
    double dDist = 0.0 ;
    for ( int i = 0 ; i < (int) m_Coords.GetCount() ; i++ )
    {
      double dD1 = m_Coords[i] - Patt.m_Coords[i] ;
      dDist += dD1 * dD1 ;
    }
    return sqrt( dDist ) ;
  } ;

  MultiCoord& operator =( MultiCoord& Orig)
  {
    m_Coords.RemoveAll() ;  
    for ( int i = 0 ; i < (int) Orig.m_Coords.GetCount() ; i++ )
      m_Coords.Add( Orig.m_Coords[i] ) ;
    m_dRobotTime = Orig.m_dRobotTime ;
    m_dReceiveTime = Orig.m_dReceiveTime ;
    return *this ;
  }
  int StringToMC( CString& Orig)
  {
    m_Coords.RemoveAll() ;
    CString ForParse( Orig ) ;
    ForParse = ForParse.Trim( " \t,;\r\n") ;
    int iNConverted = 0 ;
    do 
    {
      int iPos = ForParse.FindOneOf( " \t,;" ) ;
      if ( iPos > 0 )
      {
        double dVal = atof( (LPCTSTR)ForParse ) ;
        ForParse = ForParse.Right( ForParse.GetLength() - iPos - 1 ) ;
        ForParse = ForParse.TrimLeft( " \b\t,;") ;
        m_Coords.Add( dVal ) ;
      }
      else
      {
        double dVal = atof( (LPCTSTR)ForParse ) ;
        ForParse.Empty() ;
        m_Coords.Add( dVal ) ;
      }
    } while ( !ForParse.IsEmpty() );
    return (int)m_Coords.GetCount() ;
  }
  bool IsZero()
  {
    for ( int i = 0 ; i < (int) m_Coords.Count() ; i++ )
    {
      if ( m_Coords[ i ] != 0. )
        return false;
    }
    return true ;
  }

private:
  CDblArray m_Coords ;
  double    m_dRobotTime ;
  double    m_dReceiveTime ;
};

class Tetragon
{
public:
  cmplx m_c1st ;
  cmplx m_c2nd ;
  cmplx m_c3rd ;
  cmplx m_c4th ;

  Tetragon() {} ;
  Tetragon( cmplx c1st , cmplx c2nd , cmplx c3rd , cmplx c4th )
  {
    m_c1st = c1st ; m_c2nd = c2nd ; m_c3rd = c3rd ; m_c4th = c4th ;
  } ;

  cmplx TopSide() { return m_c2nd - m_c1st ; };
  cmplx RightSide() { return m_c3rd - m_c2nd ; };
  cmplx BottomSide() { return m_c4th - m_c3rd ; };
  cmplx LeftSide() { return m_c4th - m_c1st ; };

  double Perimeter() { return abs(TopSide()) + abs( RightSide() ) 
    + abs( BottomSide()) + abs( LeftSide() ) ; } ;

  bool IsFilled()
    { return (m_c1st.real() && m_c2nd.real() && m_c3rd.real() && m_c4th.real()) ; };
  cmplx GetCenter() 
  {
    return (m_c1st + m_c2nd + m_c3rd + m_c4th) / 4. ;
  }
  bool IsRomb( double dTolerance_perc = 1. )
  {
    double dL1 = abs( TopSide() ) ;
    double dL2 = abs( RightSide() ) ;
    double dL3 = abs( BottomSide() ) ;
    double dL4 = abs( LeftSide() ) ;
    double dMinDiff = DBL_MAX , dMaxDiff = 0. ;
    double dDiff = fabs( dL1 - dL2 ) ; SetMinMax( dDiff , dMinDiff , dMaxDiff ) ;
    dDiff = fabs( dL3 - dL2 ) ; SetMinMax( dDiff , dMinDiff , dMaxDiff ) ;
    dDiff = fabs( dL3 - dL4 ) ; SetMinMax( dDiff , dMinDiff , dMaxDiff ) ;
    dDiff = fabs( dL4 - dL1 ) ; SetMinMax( dDiff , dMinDiff , dMaxDiff ) ;
    double dDistortion = dMaxDiff - dMinDiff ;
    if ( dDistortion )
    {
      dDistortion /= Perimeter() * 0.25 ;
      if ( dDistortion > dTolerance_perc * 0.01 )
       return false ;
    }
    return true ;
  }
};

inline BOOL PtInRect( CRect rc , cmplx Pt )
{
  CPoint Point( ROUND( Pt.real() ) , ROUND( Pt.imag() ) ) ;
  return PtInRect( &rc , Point ) ;
}

inline void RotateArray(double * pArr, int iLen, int iNRot)
{
  ASSERT(iNRot < iLen);
  double * pdTmp = new double[iNRot];
  memcpy(pdTmp, pArr, iNRot * sizeof(double));
  memcpy(pArr, pArr + iNRot, (iLen - iNRot) * sizeof(double));
  memcpy(pArr + (iLen - iNRot), pdTmp, iNRot * sizeof(double));
  delete[] pdTmp;
}
inline int FindLocalMinimum(double * pdArr, int iArrLen, int iLookDepth)
{
  int iIndex = 0;
  int iLimit = iArrLen - iLookDepth;
  while ( iIndex < iLimit  )
  {
    int iNCatched = iLookDepth;
    while (pdArr[iIndex] >= pdArr[iIndex + 1])
    {
      if (++iIndex >= iLimit)
        break;
    }
    while ( iNCatched > 1 )
    {
      if (pdArr[iIndex] >= pdArr[iIndex + iNCatched])
      {
        iIndex++;
        break;
      }
      iNCatched--;
    }
    if (iNCatched <= 1)
      return iIndex ;
  }
  return -1;
}

inline int FindLocalMaximum(double * pdArr, int iArrLen, int iLookDepth)
{
  int iIndex = 0;
  int iLimit = iArrLen - iLookDepth;
  while (iIndex < iLimit)
  {
    int iNCatched = iLookDepth;
    while (pdArr[iIndex] <= pdArr[iIndex + 1])
    {
      if (++iIndex >= iLimit)
        return -1;
    }
    while (iNCatched > 1)
    {
      if (pdArr[iIndex] <= pdArr[iIndex + iNCatched])
      {
        iIndex++;
        break;
      }
      iNCatched--;
    }
    if (iNCatched <= 1)
      return iIndex ;
  }
  return -1 ;
}

class FXAverager
{
public:
  double m_dSum ;
  int    m_iCounter ;
  double m_dSqareSum ;

  void Reset() { memset( this , 0 , sizeof( this ) ) ; }
  FXAverager() { Reset(); }
  int Add( double dVal )
  {
    m_dSum += dVal ;
    m_dSqareSum += dVal * dVal ;
    return ++m_iCounter ;
  }
  double GetAver()
  {
    if ( m_iCounter )
      return m_dSum / m_iCounter ;

    return 0. ;
  }
  double GetStd()
  {
    if ( m_iCounter > 1 )
    {
      double dAver = GetAver() ;
      return sqrt( (m_dSqareSum / m_iCounter) - dAver * dAver ) ;
    }
    return 0. ;
  }
};

#endif  // ____INTF_SUP_H
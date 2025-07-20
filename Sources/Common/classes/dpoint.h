// DPoint.h: interface for the CDPoint class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DPOINT_H__AE7D4E69_6B3F_4E84_ACE3_E3D7D6578BF3__INCLUDED_)
#define AFX_DPOINT_H__AE7D4E69_6B3F_4E84_ACE3_E3D7D6578BF3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <fxfc/fxstruct.h>
#include <fxfc/fxarrays.h>
//#include <math/Intf_sup.h>

#ifndef ROUND
#define ROUND(a) ((int)round(a))
#endif

class CDPoint: public DPOINT
{
public:
    CDPoint() { x = y = 0. ;};
    CDPoint(const double X, const double Y) { x=X; y=Y;};
    CDPoint(const DPOINT& dp) { x=dp.x; y=dp.y;};
    CDPoint( const CPoint pt ) { x=pt.x ; y=pt.y ;}
    //virtual ~CDPoint() {};
    CDPoint& operator = (const CDPoint& dptCopy)
    {
        if (this != &dptCopy)
        {
          x = dptCopy.x;
          y = dptCopy.y;
        }
        return *this;
    }
    CDPoint& operator = (const CPoint dptCopy)
    {
      x = dptCopy.x;
      y = dptCopy.y;
      return *this;
    }

    CDPoint operator -(const CDPoint dptPoint)
    {
        CDPoint dptTmp = *this;
        dptTmp.x -= dptPoint.x;
        dptTmp.y -= dptPoint.y;
        return dptTmp;
    }
    CDPoint operator +(const CDPoint dptPoint)
    {
        CDPoint dptTmp = *this;
        dptTmp.x += dptPoint.x;
        dptTmp.y += dptPoint.y;
        return dptTmp;
    }
    CDPoint operator +=(const CDPoint dptPoint)
    {
      x += dptPoint.x;
      y += dptPoint.y;
      return *this;
    }
    CDPoint operator +=(const CPoint Point)
    {
      x += Point.x;
      y += Point.y;
      return *this;
    }
    CDPoint operator -=(const CDPoint dptPoint)
    {
      x -= dptPoint.x;
      y -= dptPoint.y;
      return *this;
    }
    CDPoint operator -=(const CPoint Point)
    {
      x -= Point.x;
      y -= Point.y;
      return *this;
    }

    CDPoint operator *(const CDPoint dptPoint)
    {
        CDPoint dptTmp = *this;
        dptTmp.x *= dptPoint.x;
        dptTmp.y *= dptPoint.y;
        return dptTmp;
    }
    CDPoint operator /(const CDPoint dptPoint)
    {
        CDPoint dptTmp = *this;
        dptTmp.x /= dptPoint.x;
        dptTmp.y /= dptPoint.y;
        return dptTmp;
    }

    CDPoint operator *(double dLamda)
    {
        CDPoint dptTmp = *this;
        dptTmp.x *= dLamda;
        dptTmp.y *= dLamda;
        return dptTmp;
    }
    CDPoint operator /(double dLamda)
    {
        CDPoint dptTmp = *this;
        dptTmp.x /= dLamda;
        dptTmp.y /= dLamda;
        return dptTmp;
    }
    CPoint GetCPoint() const
    {
      return CPoint( ROUND(x) , ROUND(y) ) ;
    }
    double GetDist( const CDPoint& Other ) const
    {
      double dX = x - Other.x ;
      double dY = y - Other.y ;
      return sqrt( dX * dX + dY * dY ) ;
    }
};

class CDPointArray: public FXArray<CDPoint,CDPoint> {};

#ifndef SHBASE_CLI
using namespace std;

#include <complex>
typedef complex<double> cmplx ;

inline cmplx GetCmplxFromCDPoint( const CDPoint& Pt )
{
  return *( ( cmplx* ) &Pt ) ;
}
inline cmplx CDP2Cmplx( const CDPoint& Pt )
{
  return *( ( cmplx* ) &Pt );
}
inline cmplx CDP2Cmplx( const DPOINT& Pt )
{
  return *( ( cmplx* ) &Pt );
}

inline CDPoint GetCDPointFromCmplx( const cmplx Pt )
{
  return *( ( CDPoint* ) &Pt ) ;
}
inline CDPoint CmplxToCDPoint( const cmplx Pt )
{
  return CDPoint( Pt.real() , Pt.imag() ) ;
}

inline cmplx CDPointToCmplx( const CDPoint Pt )
{
  return cmplx( Pt.x , Pt.y ) ;
}
#endif

#endif // !defined(AFX_DPOINT_H__AE7D4E69_6B3F_4E84_ACE3_E3D7D6578BF3__INCLUDED_)

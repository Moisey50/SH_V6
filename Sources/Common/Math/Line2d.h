#pragma once
#include <math/Intf_sup.h>


class CLine2d // Line in form m_a * x + m_b * y + m_c = 0 
{
public:
  double m_a , m_b , m_c , m_div ;

  CLine2d() { m_a = m_b = m_c = 0. ; }
  CLine2d( const cmplx& p , const cmplx& q )
  {
    m_a = q.imag() - p.imag() ;
    m_b = p.real() - q.real() ;
    m_c = -m_a * p.real() - m_b * p.imag() ;
    norm();
  }
  CLine2d( double a , double b , double c )
  {
    m_a = a ;
    m_b = b ;
    m_c = c ;
    norm() ;
  }
  CLine2d( const CLine2d& Orig )
  {
    m_a = Orig.m_a ;
    m_b = Orig.m_b ;
    m_c = Orig.m_c ;
    m_div = Orig.m_div ;
  }

  void Update( const cmplx& p , const cmplx& q )
  {
    m_a = q.imag() - p.imag() ;
    m_b = p.real() - q.real() ;
    m_c = -m_a * p.real() - m_b * p.imag() ;
    norm();
  }

  void ByPointAndDir( const cmplx& Pt , const cmplx& Dir )
  {
    m_a = -Dir.imag() ;
    m_b = Dir.real() ;
    m_c = -m_a * Pt.real() - m_b * Pt.imag() ;
    norm() ;
  }
  void norm()
  {
    m_div = sqrt( m_a*m_a + m_b * m_b );
    if ( abs( m_div ) > EPSILON )
    {
      m_a /= m_div ; m_b /= m_div ; m_c /= m_div ;
    }
    m_div = 1. ;
  }
  double get_angle() const
  {
    return atan2( m_a , m_b ) ;
  }
  double GetAngleDiff( const CLine2d& Other )
  {
    double dOtherAngle = Other.get_angle() ;
    double dAngle = get_angle() ;
    double dDiff = dAngle - dOtherAngle ;
    dDiff = NormRad( dDiff ) ;
    return dDiff ;
  }
  double dist( const cmplx& p ) const
  {
    return m_a * p.real() + m_b * p.imag() + m_c;
  }
  bool intersect( const CLine2d& m , const CLine2d& n , cmplx& res ) const
  {
    double zn = det( m.m_a , m.m_b , n.m_a , n.m_b );
    if ( fabs( zn ) < EPSILON )
      return false;
    res = cmplx( -det( m.m_c , m.m_b , n.m_c , n.m_b ) , -det( m.m_a , m.m_c , n.m_a , n.m_c ) ) / zn ;
    return true;
  }
  bool intersect( const CLine2d& Other , cmplx& res ) const
  {
    double zn = det( Other.m_a , Other.m_b , m_a , m_b );
    if ( fabs( zn ) < EPSILON )
      return false;
    res = cmplx( -det( Other.m_c , Other.m_b , m_c , m_b ) ,
      -det( Other.m_a , Other.m_c , m_a , m_c ) ) / zn ;
    return true;
  }

  bool parallel( const CLine2d& m , const CLine2d& n ) const
  {
    return abs( det( m.m_a , m.m_b , n.m_a , n.m_b ) ) < EPSILON ;
  }

  bool equivalent( const CLine2d& m , const CLine2d& n )
  {
    return fabs( det( m.m_a , m.m_b , n.m_a , n.m_b ) ) < EPSILON
      && fabs( det( m.m_a , m.m_c , n.m_a , n.m_c ) ) < EPSILON
      && fabs( det( m.m_b , m.m_c , n.m_b , n.m_c ) ) < EPSILON;
  }

  CLine2d GetBisector( CLine2d& Other )
  {
    double a = m_a * Other.m_div - Other.m_a * m_div ;
    double b = m_b * Other.m_div - Other.m_b * m_div ;
    double c = m_c * Other.m_div - Other.m_c * m_div ;

    return CLine2d( a , b , c ) ;
  }
  double GetDistFromPoint( const cmplx& Pt )
  {
    double dDist = (m_div >= EPSILON) ?
      m_a * Pt.real() + m_b * Pt.imag() + m_c : abs( Pt ) ; // distance to zero in last case
    return dDist ;
  }
  double GetAbsDistFromPoint( const cmplx& Pt )
  {
    return abs( GetDistFromPoint( Pt ) ) ;
  }
  cmplx GetNearestPt( const cmplx& Pt )
  {
    if ( m_div < EPSILON )
      return cmplx() ; // line is one pt
    cmplx Result( (m_b * (m_b * Pt.real() - m_a * Pt.imag())) - (m_a * m_c)  ,
      (m_a * (-m_b * Pt.real() + m_a * Pt.imag())) - (m_b * m_c) ) ;
    return Result ;
  }
  double GetY( double dX )
  {
    if ( fabs( m_b ) > EPSILON )
      return (-(m_a * dX + m_c) / m_b) ;
    return DBL_MAX ;
  }
  double GetX( double dY )
  {
    if ( fabs( m_a ) > EPSILON )
      return (-(m_b * dY + m_c) / m_a) ;
    return DBL_MAX ;
  }
  cmplx GetPtForX( double dX )
  {
    return cmplx( dX , GetY( dX ) ) ;
  }
  cmplx GetPtForY(  double dY )
  {
    return cmplx( GetX( dY ) , dY ) ;
  }
  cmplx GetOrthoVect() const
  {
    double dAngle_rad = get_angle() + M_PI2 ;
    dAngle_rad = NormRad( dAngle_rad ) ;
    return polar( 1. , dAngle_rad ) ;
  }
  CLine2d GetOrthoLine( const cmplx& Pt ) const 
  {
    CLine2d Result ;
    Result.ByPointAndDir( Pt , GetOrthoVect() ) ;
    return Result ;
  }
  CLine2d GetOrthoLineVF( const cmplx& Pt ) const
  {
    CLine2d Result ;
    Result.ByPointAndDir( Pt , conj(GetOrthoVect()) ) ;
    return Result ;
  }
  bool GetOrthoCross( const cmplx& Pt , cmplx& Result )
  {
    return intersect( GetOrthoLineVF( Pt ) , Result )  ;
  }
  cmplx GetFarestPtOfSegment( int iIndexBegin , int iIndexEnd ,
    const cmplx * pFigure , int& iFarestIndex , double& dFarestDist )
  {
    const cmplx * pIter = pFigure + iIndexBegin ;
    const cmplx * pEnd = pFigure + iIndexEnd ;
    dFarestDist = 0.  ;
    int iIndexMax = -1 ;
    do
    {
      double dNewAbsDist = GetAbsDistFromPoint( *pIter ) ;
      if ( dNewAbsDist > dFarestDist )
      {
        iIndexMax = ( int ) ( pIter - pFigure ) ;
        dFarestDist = dNewAbsDist ;
      }
    } while ( ++pIter < pEnd ) ;
    iFarestIndex = iIndexMax ;
    return pFigure[ iIndexMax ] ;
  };
};

// there is line with Pt1 and Pt2 and point on the side
// function calculates cross point of line and orthogonal from side point
inline cmplx GetOrthCrossOnLine( const cmplx& LinePt1 , 
  const cmplx& LinePt2 , const cmplx& SidePt )
{
  cmplx LineVect = LinePt2 - LinePt1 ;
  cmplx SideVect = SidePt - LinePt1 ;
  double dLen = abs( LineVect ) ;
  double dK = SideVect.real() * LineVect.real() + SideVect.imag() * LineVect.imag() ;
  dK /= dLen * dLen ;
  cmplx Cross = LinePt1 + LineVect * dK ;
  return Cross ;
}

typedef vector<CLine2d> LinesVector ;


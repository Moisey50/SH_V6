// DRect.h: interface for the CDRect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRECT_H__AE7D4E69_6B3F_4E84_ACE3_E3D7D6578BF3__INCLUDED_)
#define AFX_DRECT_H__AE7D4E69_6B3F_4E84_ACE3_E3D7D6578BF3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <classes/dpoint.h>


class CDRect : public DRECT
{
public:
  CDRect() { left = right = top = bottom = 0; }
  CDRect( double left1 , double right1 , double top1 , double bottom1 )
  {
    left = left1; right = right1; top = top1; bottom = bottom1;
  };

  CDRect( DRECT& dr ) { left = dr.left; right = dr.right; top = dr.top; bottom = dr.bottom; };
  CDRect( DPOINT& LeftTop , DPOINT& RightDown ) { left = LeftTop.x ; 
    right = RightDown.x ; top = LeftTop.y ; bottom = RightDown.y ; };
  CDRect( CRect IRect ) { left = IRect.left ; right = IRect.right; top = IRect.top; bottom = IRect.bottom; }
 
 
 ~CDRect() {};
  CDRect& operator = ( const CDRect& drCopy )
  {
    if ( this != &drCopy )
    {
      left = drCopy.left;
      right = drCopy.right;
      top = drCopy.top;
      bottom = drCopy.bottom;
    }
    return *this ;
  }
  CDRect& Union( const CDRect& Other )
  {
    if ( this != &Other )
    {
      left = min( Other.left , left ) ;
      right = max( Other.right , right ) ;
      top = min( Other.top , top );
      bottom = max( Other.bottom , bottom ) ;
    }
    return *this ;
  }
  CDRect& Union( const DPOINT& Other )
  {
    left = min( Other.x , left ) ;
    right = max( Other.x , right ) ;
    top = min( Other.y , top );
    bottom = max( Other.y , bottom ) ;
    return *this ;
  }
  CDRect& Union( const CPoint& Other )
  {
    left = min( ( double ) Other.x , left ) ;
    right = max( ( double ) Other.x , right ) ;
    top = min( ( double ) Other.y , top );
    bottom = max( ( double ) Other.y , bottom ) ;
    return *this ;
  }
  bool IsPtInside( const DPOINT& Pt )
  {
    return ( ( left <= Pt.x ) && ( Pt.x <= right )
      && ( top <= Pt.y ) && ( Pt.y <= bottom ) ) ;
  }
  bool IsPtInside( const POINT& Pt )
  {
    return ( ( left <= Pt.x ) && ( Pt.x <= right )
      && ( top <= Pt.y ) && ( Pt.y <= bottom ) ) ;
  }

  CDRect& Offset( double dOffX , double dOffY )
  {
    left += dOffX ;
    right += dOffX ;
    top += dOffY ;
    bottom += dOffY ;
    return *this ;
  }
  void SetRectEmpty()
  {
    left = right = top = bottom = 0.0;
  }
  BOOL IsRectEmpty()
  {
    return (
      ( left == right ) && ( top == bottom )
      );
  }
  double Width() const { return ( right - left ); }
  double Height() const { return ( bottom - top ); }
  void NormalizeRect()
  {
    if ( left > right )
      swap( left , right ) ;
    if ( bottom > top )
      swap( top , bottom ) ;
  }

};

typedef FXArray<CDRect , CDRect> CDRectArray ;


#endif // !defined(AFX_DRECT_H__AE7D4E69_6B3F_4E84_ACE3_E3D7D6578BF3__INCLUDED_)

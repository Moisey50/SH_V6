// FigureFrame.h: interface for the CFigureFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FIGUREFRAME_H__0A6B3DC8_184F_4F5D_A030_7B93E93AD738__INCLUDED_)
#define AFX_FIGUREFRAME_H__0A6B3DC8_184F_4F5D_A030_7B93E93AD738__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include <classes\dpoint.h>
// #include <complex.h>
// 
/*using namespace std;*/

//#include <math/Intf_sup.h>

class FX_EXT_GADGET CFigure : public CDPointArray
{
public:
  CFigure();
  CFigure( CFigure& fig )
  {
    Copy( fig );
  }
  ~CFigure();
  void Clean();
  FXSIZE  AddPoint( DPOINT& p )
  {
    Add( CDPoint( p ) );
    return GetUpperBound() + 1;
  }
  FXSIZE  AddPoint( CDPoint& p )
  {
    Add( p );
    return GetUpperBound() + 1;
  }
  FXSIZE  AddPoint( double dX , double dY )
  {
    CDPoint p( dX , dY );
    Add( p );
    return GetUpperBound() + 1;
  }

  FXSIZE  GetNumberVertex() const
  {
    return GetUpperBound() + 1;
  }
  const CDPoint& GetAt( FXSIZE i ) const
  {
    return this->operator []( i );
  }
  //CDPoint& GetAt(FXSIZE i) { return this->operator [](i); }
  FXString ToString() const
  {
    FXString retV ;
    if (GetUpperBound() + 1 == 1)
    {
      retV.Format(_T("Point (%f,%f)"), GetAt(0).x, GetAt(0).y);
    }
    else if (GetUpperBound() + 1 == 2)
    {
      retV = _T("Segment");
    }
    else if (GetUpperBound() + 1 == 4)
    {
      retV = _T("Rectangle");
    }
    else
      retV.Format(_T("Polyline %u points"), GetCount());
    return retV;
  }
  CFigure& operator =( const CFigure& figSrc )
  {
    Copy( figSrc );
    return *this;
  }
  double GetFigureLength() const ; // length as sum of all distances between points
  double GetConturLength() const ; // as previous plus distance between first and last points
  double GetNearestPt( CDPoint& Other , int& iNearestIndex  , 
    int &iStart , int * piLength = NULL )
  {
    int iLimit = (!piLength || *piLength > Count() || *piLength < 0 ) ? (int)Count() : *piLength ;
    double dMinDist = GetAt( iStart ).GetDist( Other ) ;
    iNearestIndex = iStart ;
    for ( int i = 1 ; i < iLimit ; i++ )
    {
      double dDist = GetAt( (iStart + i) % Count() ).GetDist( Other ) ;
      if ( dDist < dMinDist )
      {
        dMinDist = dDist ;
        iNearestIndex = iStart + i ;
      }
    }
    return dMinDist ;
  }
  int GetNormalizedIndex( int iIndex ) const
  {
    return (iIndex % GetCount()) ;
  }
  int IncrementIndex( int& iIndex ) const
  {
    return (iIndex = (iIndex + 1) % GetCount()) ;
  }
  int DecrementIndex( int& iIndex ) const
  {
    return (iIndex = (int)(( (iIndex + GetCount()) - 1 ) % GetCount())) ;
  }
};

class FX_EXT_GADGET CFigureFrame : public CDataFrame , public CFigure
{
protected:
  CFigureFrame();
  CFigureFrame( const CFigureFrame* Frame );
  virtual ~CFigureFrame();
public:
  CFigureFrame* GetFigureFrame( LPCTSTR label = DEFAULT_LABEL )
  {
    if ( !label || m_Label == label ) return this; return NULL;
  }
  const CFigureFrame* GetFigureFrame( LPCTSTR label = DEFAULT_LABEL ) const
  {
    if ( !label || m_Label == label ) return this; return NULL;
  }
  static  CFigureFrame* Create( CFigureFrame* Frame = NULL );
  static  CFigureFrame* Create( CFigure* Figure );
  BOOL Serialize( LPBYTE* ppData , FXSIZE* cbData ) const;
  BOOL Serialize( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const ;
  BOOL Restore( LPBYTE lpData , FXSIZE cbData );
  virtual FXSIZE GetSerializeLength( FXSIZE& uiLabelLen ,
    FXSIZE * pAttribLen = NULL ) const ;
  CDataFrame* Copy() const
  {
    return new CFigureFrame( this );
  }
  virtual void ToLogString(FXString& Output);
};

typedef FXArray<CFigureFrame* , CFigureFrame*> FigFrames ;

#ifndef SHBASE_CLI
//using namespace std;

// #include <complex>
// typedef complex<double> cmplx ;
#include <math/Intf_sup.h>

inline cmplx GetFigurePt( const CFigure& Fig , int iIndex )
{
  return CDPointToCmplx( Fig[ iIndex % Fig.GetCount() ] ) ;
}

FX_EXT_GADGET CFigureFrame * CreatePtFrameEx( const cmplx& Pt , DWORD dwColor = 0xc0ffc0 , int iSize = 2 , int iThickness = 1 ) ;

FX_EXT_GADGET CFigureFrame * CreatePtFrameEx( const cmplx& Pt , LPCTSTR Attributes ) ;

FX_EXT_GADGET CFigureFrame * CreateLineFrameEx( const cmplx& Pt1 , const cmplx& Pt2 ,
  DWORD dwColor = 0xc0ffc0 , int iThickness = 1 ) ;

FX_EXT_GADGET CFigureFrame * CreateLineFrameEx( const cmplx& Pt1 , const cmplx& Pt2 ,
  LPCTSTR Attributes ) ;
 
  // Rectangle drawing From CRect
FX_EXT_GADGET CFigureFrame * CreateFigureFrameEx( const CRect& Rect ,
  DWORD dwColor = 0xc0ffc0 , int iThickness = 1 ) ;

FX_EXT_GADGET CFigureFrame * CreateFigureFrameEx( const cmplx * Pts , int iLen ,
  DWORD dwColor = 0xc0ffc0 , int iThickness = 1 ) ;
FX_EXT_GADGET CFigureFrame * CreateFigureFrameEx( CmplxVector& Data , 
  DWORD dwColor = 0xc0ffc0 , int iThickness = 1 ) ;

FX_EXT_GADGET CFigureFrame * CreateFigureFrameEx( const cmplx * Pts , int iLen ,
  LPCTSTR pAttributes ) ;
FX_EXT_GADGET CFigureFrame * CreateFigureFrameEx( CmplxVector& Data ,
  LPCTSTR pAttributes ) ;

FX_EXT_GADGET CFigureFrame * CreateFigureFrameEx( const double * pData ,
  int iLen , const cmplx& cOrigin , const cmplx& cGraphStep , double dYSHift ,
  double dYScale , DWORD dwColor = 0xc0ffc0 , int iThickness = 1 ) ;
FX_EXT_GADGET CFigureFrame * CreateFigureFrameEx( const DoubleVector& Data ,
  const cmplx& cOrigin , const cmplx& cGraphStep , double dYSHift ,
  double dYScale , DWORD dwColor = 0xc0ffc0 , int iThickness = 1 ) ;

FX_EXT_GADGET CFigureFrame * CreateFigureFrameEx( const double * pData ,
  int iLen , const cmplx& cOrigin , const cmplx& cStepScaling , double dYSHift , 
  double dYScale , LPCTSTR pAttributes ) ;
FX_EXT_GADGET CFigureFrame * CreateFigureFrameEx( DoubleVector& Data ,
  const cmplx& cOrigin , const cmplx& cStepScaling , double dYSHift ,
  double dYScale , LPCTSTR pAttributes ) ;

FX_EXT_GADGET CFigureFrame * CreateCircleViewEx( const cmplx& cCenter ,
  double dRadius , DWORD dwCircColor , int iNPointsOnCircle = 72 ) ;

#endif // SHBASE_CLI

#endif // !defined(AFX_FIGUREFRAME_H__0A6B3DC8_184F_4F5D_A030_7B93E93AD738__INCLUDED_)

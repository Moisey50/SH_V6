#pragma once

#include <classes\drect.h>
#include <math/FRegression.h>

#include <math/PlaneGeometry.h>

enum Directions
{
  H_UNKNOWN = -1 ,
  H00_00 = 0 ,
  H12_00 = 0 ,
  H01_30 ,
  H03_00 ,
  H04_30 ,
  H06_00 ,
  H07_30 ,
  H09_00 ,
  H10_30
};

enum Edge
{
  E_UNDEF = -1 ,
  E12 = 0 ,
  E03 ,
  E06 ,
  E09
};

enum SQ_EDGE_AND_CORNERS
{
  SQREdge_Center = 0 ,
  SQREdge_Left ,
  SQREdge_Upper ,
  SQREdge_Right ,
  SQREdge_Lower ,
  
  SQRCorner_LT ,
  SQRCorner_TR ,
  SQRCorner_RB ,
  SQRCorner_BL 
};


inline cmplx CDPtToCmplx( const CDPoint& Pt )
{
  return cmplx( Pt.x , Pt.y ) ;
}

inline void SwapCmplx( cmplx& First , cmplx& Second )
{
  cmplx Tmp = First ;
  First = Second ;
  Second = Tmp ;
}

inline int IsPtOnEdge( const CDPoint& Pt , const CRect& Rect )
{
  return ( (fabs( Pt.x - Rect.left ) < 1.) 
    | ((fabs( Pt.x - Rect.right ) < 1.) << 1)
    | ((fabs( Pt.y - Rect.top ) < 1.) << 2)
    | ((fabs( Pt.y - Rect.bottom ) < 1.) << 3)
    ) ;
}

inline int IsPtNearEdge( const CDPoint& Pt , const CRect& Rect , double dThres )
{
  return ((fabs( Pt.x - Rect.left ) < dThres)
    | ((fabs( Pt.x - Rect.right ) < dThres) << 1)
    | ((fabs( Pt.y - Rect.top ) < dThres) << 2)
    | ((fabs( Pt.y - Rect.bottom ) < dThres) << 3)
    ) ;
}


inline int IsPtOnEdge( const cmplx& Pt , const CRect& Rect )
{
  return IsPtOnEdge( (const CDPoint&)Pt , Rect ) ;
}

inline int IsPtNearEdge( const cmplx& Pt , const CRect& Rect , double dThres )
{
  return IsPtNearEdge( (const CDPoint&) Pt , Rect , dThres ) ;
}

inline bool IsPointInRect( RECT& rect , cmplx& Pt )
{
  return ((rect.left <= Pt.real()) && (Pt.real() <= rect.right)
    && (rect.top <= Pt.imag()) && (Pt.imag() <= rect.bottom) );
}
class BorderSegment
{
public:
  BorderSegment( const CDPoint * pBegin = NULL ,
    const CDPoint * pEnd = NULL )
  {
    m_pBegin = pBegin ; m_pEnd = pEnd ;
    m_EdgeBegin = m_EdgeEnd = E_UNDEF ;
  }
  void Reset()
  {
    m_pBegin = m_pEnd = NULL ;
    m_EdgeBegin = m_EdgeEnd = E_UNDEF ;
  }
  const CDPoint * m_pBegin ;
  const CDPoint * m_pEnd ;
  Edge            m_EdgeBegin ;
  Edge            m_EdgeEnd ;
};

#define Fig_Create 1
#define Fig_Release 2
#define Fig_AddRef 3
#define Fig_Touch 4


void LogFigure( LPCTSTR pFigName , FXSIZE iOper , FXSIZE iString , FXSIZE iNRef = -1 ) ;


typedef FXArray <BorderSegment> ArrayOfSegments ;
class CSegmentInsideROI : public CFigure
{
public:
  CSegmentInsideROI( int iIndexBegin = 0 , const CFigureFrame * pFigure = NULL )
  {
    Reset();
    m_iIndexBegin = iIndexBegin ;
    m_dMinDistToCenter = DBL_MAX ;
    m_pFigure = pFigure ;
  }
  ~CSegmentInsideROI()
  {
    Reset(); 
  }
  void Reset()
  {
    //SetSize( 0 , 100); // remove all from figure and set allocation step
    memset( &m_iIndexBegin , 0, sizeof(*this) - sizeof( CFigure) );
  }
  CSegmentInsideROI& operator = ( CSegmentInsideROI& Orig )
  {
    Reset();
    Copy((CFigure)Orig);
    memcpy( &m_iIndexBegin , &Orig.m_iIndexBegin ,
      ((size_t) &m_Rect - (size_t) &m_iIndexBegin) + sizeof( m_Rect ) ) ;
    m_pFigure = Orig.m_pFigure ;
    //((CFigure*)this)->Copy( (CFigure)Orig ) ;
    return *this ;
  }
  void AccountPt( const cmplx& Pt , cmplx * cCent = NULL )
  {
    if ( !m_iNElements )
    {
      m_Rect.left = m_Rect.right = Pt.real() ;
      m_Rect.top = m_Rect.top = Pt.imag() ;
      m_dSegmentLength = 0. ; 
    }
    else
    {
      if ( Pt.real() < m_Rect.left )
        m_Rect.left = Pt.real() ;
      if ( Pt.real() > m_Rect.right )
        m_Rect.right = Pt.real() ;
      if ( Pt.imag() < m_Rect.top )
        m_Rect.top = Pt.imag() ;
      if ( Pt.imag() > m_Rect.bottom )
        m_Rect.bottom = Pt.imag() ;
      cmplx cPrevPt = CDPtToCmplx( GetAt( m_iNElements - 1 ) ) ;
      m_dSegmentLength += abs( Pt - cPrevPt ) ;
    }
    if ( cCent )
    {
      double dDistToCent = abs( *cCent - Pt ) ;
      if ( dDistToCent < m_dMinDistToCenter )
      {
        m_dMinDistToCenter = dDistToCent ;
        m_iMinDistToCenterIndex = m_iIndexBegin + m_iNElements ;
      }
    }
    Add( CmplxToCDPoint( Pt ) ) ;
    m_iNElements++ ;
  }
  void AccountFigure( const CFigure * pFigure , cmplx * pCent = NULL )
  {
    const cmplx * pPt = ( const cmplx * )(pFigure->GetData()) ;
    const cmplx * pEnd = pPt + pFigure->GetCount() ;
    while ( pPt < pEnd )
    {
      AccountPt( *( pPt++ ) , pCent ) ;
    }
  }

  int GetIndexInRing( int iIndex ) const
  {
    return (int)::GetIndexInRing( iIndex + m_iIndexBegin , 
      m_pFigure->GetCount() ) ;
  }
  cmplx GetPointOnSegment( int iIndex ) const
  {
    if ( iIndex >= 0 && iIndex < m_iNElements )
    {
      return CDPtToCmplx( m_pFigure->GetAt( GetIndexInRing(iIndex) ) ) ;
      //return CDPtToCmplx( m_pFigure->GetAt( GetIndexInRing( iIndex ) ) ) ;
    }
    return cmplx( 0. , 0. ) ;
  }

  bool IsSegmentComplicated() ;
  int GetNPoints() { return m_iNElements; } ;

  int m_iIndexBegin ; // first and last Indexes
  int m_iIndexEnd ;   // in m_pFigure array
  int m_iSecondBegin ;// If segment is covering the figure begin and end
  int m_iSecondEnd ;  // m_iSecond... indexes are showing part of segment on end side
  int m_iNElements ;  // values for faster working
  int m_iMinDistToCenterIndex ; 
    // both following values should be zero (fully internal segment case) or not zero
    // both values are bit mask (1-left edge,2-right edge,4-top edge, 8-bottom edge)
  int m_iFirstEdge ;  // ROI edge of segment beginning (bit mask)
  int m_iLastEdge ;   // ROI edge  on segment end
  double m_dMinDistToCenter ;
  double m_dSegmentLength ;
  CDRect m_Rect ;     // rectangle containing segment
  const CFigureFrame * m_pFigure ; // original figure frame from which segment is taken
};
typedef FXArray <CSegmentInsideROI* , CSegmentInsideROI*> ActiveSegments ;

class StraightLine : public CFRegression
{
public:
  cmplx     m_Begin ;
  cmplx     m_End ;
  double    m_dStd ;
  double    m_dMaxDiff ;
  int       m_iGroupIndex ;
  int       m_iBeginIndex ;
  int       m_iEndIndex ;
  int       m_iLength ;
  COLORREF  m_Color ;


  StraightLine( cmplx Begin = cmplx() , cmplx End = cmplx() ,
    int iGroupIndex = 0 , int iBeginIndex = -1 , int iEndIndex = -1 )
  {
    m_Begin = Begin ;
    m_End = End ;
    m_dStd = m_dMaxDiff = 0. ;
    m_iGroupIndex = iGroupIndex ;
    m_iBeginIndex = iBeginIndex ;
    m_iEndIndex = iEndIndex ;
    m_iLength = 0 ;
    m_Color = 0 ;
  }
  StraightLine( const StraightLine& Orig )
  {
    memcpy( &m_Begin , &Orig.m_Begin ,
      (LPBYTE) ((&m_Color) + 1) - (LPBYTE) &m_Begin ) ;
    memcpy( &m_dSumX , &Orig.m_dSumX ,
      (LPBYTE) ((&m_dDividerY) + 1) - (LPBYTE) &m_dSumX ) ;
  }
  StraightLine& operator=( const StraightLine& Orig )
  {
    memcpy( &m_Begin , &Orig.m_Begin ,
      (LPBYTE) ((&m_Color) + 1) - (LPBYTE) &m_Begin ) ;
    memcpy( &m_dSumX , &Orig.m_dSumX ,
      (LPBYTE) ((&m_dDividerY) + 1) - (LPBYTE) &m_dSumX ) ;
    return *this ;
  }
} ;

class StraightLines : public FXArray <StraightLine> {} ;


// Function returns contour segments which are not touching ROI edge.
ActiveSegments * GetInternalSegments(
  const CFigureFrame * pContur , const CRect * ROI ) ;

ActiveSegments * GetCrossROISegments(
  const CFigureFrame& Contur, CRect& ROI,
  int * piMinIndex = NULL, double * pdMinDist = NULL);

int RemoveSmallOutGrowths( CFigureFrame& pResult ,
  int iStep , double dMaxAngle , int iLongJump );

// function marks points  with big distance from line between neighbors
// Another future enhancement - with big difference in distance to neighbors
// 
int MarkPointsWithBigDeviation( const CFigure& Orig ,
  double dMaxDeviation , int * Bads ); 


int GetNVerticesOnFigure( const CFigure& Fig ,
  cmplx& Center , CmplxArray * pResults ,
  double dMinMaxThres = 0.5 , double * pdPerimeter = NULL , CIntArray * pIndexes = 0 ) ;

// GetNFilteredVertices function looks for vertices with distance to center
// around max distance.
// After that, if there is only 2 such vertices, it looks for 
// additional two vertices with maximal distance from line
// between found 2 vertices
int GetNFilteredVertices( const CFigure& Fig ,
  cmplx& Center , CmplxArray * pResults , double dMinMaxThres ,
  double *pdPerimeter , FXIntArray * pIndexes = NULL ) ;

cmplx GetWeightCenter( const CFigure& Fig , ImgMoments * pMoments = NULL ) ;

bool MeasureHorizAndVertEdges( const CFigureFrame * pFigure ,
  cmplx& cCent , cmplx& cSize , CContainerFrame * pView = NULL ,
  CmplxVector * pCentsAndCorners = NULL ) ;

bool MeasureAndFilterHorizAndVertEdges( const CFigureFrame * pFigure ,
  cmplx& cCent , cmplx& cSize , CContainerFrame * pView , 
  CmplxVector * pCentsAndCorners , double dGoodStdThres_pix = 0.5 );

int FindStraightSegments( const CFigureFrame * pFF , int iGroupIndex ,
  const CRectFrame * pROI , StraightLines& Lines ,
  int iMinStraightLen , double dDiffFromLine_pix , int iNMaxDeviated ,
  FigFrames * pSavedIntSegments = NULL , int * piNLastSegments = NULL ) ;

double GetMostOutlyingPoint( const cmplx& cPtOnLineBegin , const cmplx& cPtOnLineEnd , 
  const CFigure * pFigure , cmplx& cResult , FXSIZE& MaxIndex ,
  FXSIZE iIBeginIndex = 0 , FXSIZE iEndIndex = 0 ) ;

int ExtractStraightSegment( const cmplx * pFF , int iLen ,
  int iInitIndex , int iInitialStep ,
  int& iMinIndex , int& iMaxIndex , double dMaxDistortion ) ;

/********************************************************************
Modified:	01 Apr 2015
created:	10:11:2005   11:41
filename: 	segmentation.h
file path:	C:\Dev\SH\Integration24Feb15\Sources\gadgets\common\imageproc\clusters\Segmentation.h
file base:	segmentation
file ext:	h
author: Moisey Bernstein

purpose: definitions and operators for algorithm of finding blobs
(FindSpots)
*********************************************************************/

#if !defined(AFX_CSPOT_H__132A0B73_DC7B_482F_B004_226F86AB2F2A__INCLUDED_)
#define AFX_CSPOT_H__132A0B73_DC7B_482F_B004_226F86AB2F2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include <math\Intf_sup.h>
#include <classes\dpoint.h>
#include <helpers\FramesHelper.h>
#include <video\tvframe.h>
#ifndef TVDB300_APPL
#include <Gadgets\gadbase.h>
#include "imageproc\SeekSpots.h"
#endif

#define SPOT_TYPE_ID 100



// Bits for parameter "m_WhatToMeasure" parsing
#define DET_MEASURE_ANGLE_CNW        0x01000000
#define DET_MEASURE_ANGLE_CW         0x02000000
#define DET_MEASURE_FULL_FRAME_ANGLE 0x04000000
#define DONT_TOUCH_BORDER 0x08000000 // for spots only



#define MEASURE_POSITION       0x00000001   // weigth center for spots
#define MEASURE_AREA           0x00000002
#define MEASURE_THICKNESS      0x00000004
#define MEASURE_PERIMETER      0x00000008
#define MEASURE_ANGLE          0x00000010  // for lines, borders; longest diameter of contour
#define MEASURE_RATIO          0x00000020  // Power ratio in rectangles
#define MEASURE_ARC            0x00000040
#define MEASURE_TEXT           0x00000080
#define MEASURE_PROFILE        0x00003000
#define MEASURE_XPROFILE       0x00001000
#define MEASURE_YPROFILE       0x00002000
#define MEASURE_DIFFRACT       0x00000200
#define MEASURE_CONTUR         0x00000400
#define MEASURE_SUB_SIDE_BACKS 0x00000800
#define MEASURE_ANGLE_CW       0x00000100
#define MEASURE_DIAMETERS      0x00004000  // longest and orthogonal diameter of contour
//#define MEASURE_CENT_BY_PROF   0x00008000
#define MEASURE_IMG_MOMENTS_NW 0x00010000  // for avoiding influence to multi measurement with MEASURE_ANGLE_CNW
#define MEASURE_IMG_MOMENTS_W  0x00020000  // for avoiding influence to multi measurement with MEASURE_ANGLE_CW
#define MEASURE_RUNS           0x00040000
#define MEASURE_SUBPIXEL       0x00080000
#define MEASURE_TXT_FAST       0x00100000
#define MEASURE_EDGE_AS_CONTUR 0x00200000


typedef struct _ObjNameToMask
{
  DWORD   bitmask ;
  LPCTSTR name ;
} ObjNameToMask ;

#ifndef EMBEDDED_OBJ_NAME_LEN
#define EMBEDDED_OBJ_NAME_LEN  30
#define MAX_OBJ_NAME_LEN       (EMBEDDED_OBJ_NAME_LEN - 1)
#endif

enum CalcImageMoments
{
  NoImageMoments = 0 ,
  CalcNotWeighted ,
  CalcWeighted
} ;
class FXUintArray : public FXArray < DWORD , DWORD > {};

inline void CorrectRect( cmplx Pt , CDRect& Rect )
{
  SetMinMax( Pt.real() , Rect.left , Rect.right ) ;
  SetMinMax( Pt.imag() , Rect.top , Rect.bottom ) ;
}


class HorSegm
{
public:
  int m_iB ;              //   B   E
  int m_iE ;              //  B  E

  HorSegm() { m_iB = m_iE = 0; } ;
  HorSegm( int iB , int iE ) { m_iB = iB ; m_iE = iE ; } ;
  bool Overlap( HorSegm& Other )
  {
    if ( m_iB <= Other.m_iB )
      return ( m_iE >= Other.m_iB ) ;
    else
      return ( m_iB <= Other.m_iE ) ;
  } ;
  int GetLen() { return ( m_iE - m_iB + 1 ) ; } ;
  int GetSumCoord() { return ( ( GetLen()*( m_iB + m_iE ) ) / 2 ) ; };
};



//typedef struct tagdPoint
//{
//  double x ; 
//  double y ;
//}	dPoint;
class CSegment /*: public HorSegm*/
{
public:
  int m_iY ;
  int m_iColor ;
  int m_iCont ;
  int m_iMaxPixel ;
  int m_iMinPixel ;
  int m_iMatched ;
  ImgMoments m_ImgMoments ;
  HorSegm m_Segm ;
  CSegment()
  {
    m_iY = -1 ;
    m_iColor = -1 ;
    m_iCont = -1 ;
    m_Segm.m_iB = m_Segm.m_iE = -1 ;
    m_iMinPixel = INT_MAX ;
    m_iMaxPixel = INT_MIN ;
    m_iMatched = 0 ;
  }
  CSegment( CSegment& Orig )
  {
    memcpy( this , &Orig , sizeof( *this ) ) ;
  }
  CSegment( int iY , int iColor , HorSegm Segm )
  {
    m_iY = iY ; m_iColor = iColor ; m_iCont = -1 ;
    m_Segm = Segm ;
    m_iMinPixel = INT_MAX ;
    m_iMaxPixel = INT_MIN ;
    m_iMatched = 0 ;
  } ;
  CSegment( int iY , int iColor , int iB , int iE ,
    int iMaxPixel , int iMinPixel )
  {
    m_iY = iY ; m_iColor = iColor ; m_Segm.m_iB = iB ; m_Segm.m_iE = iE ;
    m_iCont = -1 ;
    m_iMatched = 0 ;
    m_iMaxPixel = iMaxPixel;
    m_iMinPixel = iMinPixel ;
    m_iMatched = 0 ;
  } ;
  CSegment( int iY , int iColor , int iB , int iE )
  {
    m_iY = iY ; m_iColor = iColor ; m_Segm.m_iB = iB ; m_Segm.m_iE = iE ;
    m_iCont = -1 ;
    m_iMatched = 0 ;
    m_iMinPixel = INT_MAX ;
    m_iMaxPixel = INT_MIN ;
    m_iMatched = 0 ;
  } ;
  bool Match( CSegment& Other )
  {
    if ( m_iColor == Other.m_iColor )
      return m_Segm.Overlap( Other.m_Segm ) ;
    return false ;
  }
  bool Match( CDPoint& Pt )
  {
    return ( ( m_Segm.m_iB <= Pt.x ) && ( Pt.x <= m_Segm.m_iE ) ) ;
  }
  int GetLen() { return m_Segm.GetLen() ; }
  CSize GetSumCoord()
  {
    return CSize( m_Segm.GetSumCoord() , m_iY*m_Segm.GetLen() ) ;
  } ;
  CRect GetRect()
  {
    return CRect( m_Segm.m_iB , m_iY , m_Segm.m_iE , m_iY ) ;
  }
  void FormImgMoments( ImgMoments * pImgMoments = NULL )
  {
    if ( !pImgMoments )
      pImgMoments = &m_ImgMoments ;
    pImgMoments->m_dM00 = ( pImgMoments->m_dAreaPixels += m_Segm.GetLen() ) ;
    double dAccumX = ( double )( ( m_Segm.m_iB + m_Segm.m_iE ) * GetLen() ) * 0.5 ;
    pImgMoments->m_dM01 += dAccumX ;
    pImgMoments->m_dM02 += SquareSum( m_Segm.m_iE ) - SquareSum( m_Segm.m_iB - 1. ) ;
    pImgMoments->m_dM11 += dAccumX * m_iY ;
    double dSumY = ( double )( GetLen() * m_iY ) ;
    pImgMoments->m_dM10 += dSumY ;
    pImgMoments->m_dM20 += dSumY * m_iY ;
  }
  void FormImgMoments( ImgMoments * pImgMoments , BYTE * pImageData , double dCorrection = 0 )
  {
    pImgMoments->m_dAreaPixels = m_Segm.GetLen() ;
    double dPixSum = 0. ;
    double dM01 = 0. ;
    for ( int i = m_Segm.m_iB ; i <= m_Segm.m_iE ; i++ )
    {
      double dVal = *pImageData++ ;
      if ( dCorrection > 0 )
        dVal = dCorrection - dVal ;
      else
        dVal += dCorrection ;
      dPixSum += dVal ;
      dM01 += dVal * i ;
      pImgMoments->m_dM02 += dVal * ( i * i ) ;
      if ( dVal < pImgMoments->m_dValMin )
        pImgMoments->m_dValMin = dVal ;
      if ( dVal > pImgMoments->m_dValMax )
        pImgMoments->m_dValMax = dVal ;
    }
    pImgMoments->m_dM00 += dPixSum ;
    pImgMoments->m_dM01 += dM01 ;
    pImgMoments->m_dM11 += dM01 * m_iY ;
    pImgMoments->m_dM10 += dPixSum * m_iY ;
    pImgMoments->m_dM20 += dPixSum * m_iY * m_iY ;
    if ( pImgMoments->m_dXMin > m_Segm.m_iB )
      pImgMoments->m_dXMin = m_Segm.m_iB ;
    if ( pImgMoments->m_dXMax < m_Segm.m_iE )
      pImgMoments->m_dXMax = m_Segm.m_iE ;
    if ( pImgMoments->m_dYMin > m_iY )
      pImgMoments->m_dYMin = m_iY ;
    if ( pImgMoments->m_dYMax < m_iY )
      pImgMoments->m_dYMax = m_iY ;
  }
  void FormImgMoments( ImgMoments * pImgMoments , WORD * pImageData , double dCorrection = 0 )
  {
    pImgMoments->m_dAreaPixels = m_Segm.GetLen() ;
    double dPixSum = 0. ;
    double dM01 = 0. ;
    for ( int i = m_Segm.m_iB ; i <= m_Segm.m_iE ; i++ )
    {
      double dVal = *pImageData++ ;
      if ( dCorrection > 0 )
        dVal = dCorrection - dVal ;
      else
        dVal += dCorrection ;
      dPixSum += dVal ;
      dM01 += dVal * i ;
      pImgMoments->m_dM02 += dVal * ( i * i ) ;
      if ( dVal < pImgMoments->m_dValMin )
        pImgMoments->m_dValMin = dVal ;
      if ( dVal > pImgMoments->m_dValMax )
        pImgMoments->m_dValMax = dVal ;
    }
    pImgMoments->m_dM00 += dPixSum ;
    pImgMoments->m_dM01 += dM01 ;
    pImgMoments->m_dM11 += dM01 * m_iY ;
    pImgMoments->m_dM10 += dPixSum * m_iY ;
    pImgMoments->m_dM20 += dPixSum * m_iY * m_iY ;
    if ( pImgMoments->m_dXMin > m_Segm.m_iB )
      pImgMoments->m_dXMin = m_Segm.m_iB ;
    if ( pImgMoments->m_dXMax < m_Segm.m_iE )
      pImgMoments->m_dXMax = m_Segm.m_iE ;
    if ( pImgMoments->m_dYMin > m_iY )
      pImgMoments->m_dYMin = m_iY ;
    if ( pImgMoments->m_dYMax < m_iY )
      pImgMoments->m_dYMax = m_iY ;
  }

};
typedef FXArray<CSegment , CSegment&> SegmentArray;
typedef vector<CSegment> SegmentVector;

class Runs : public HorSegm
{
public:
  int iY ;
  Runs() { memset( this , 0 , sizeof( *this ) ) ; } ;
  Runs( int iXbo , int iXeo , int iYo )
  {
    m_iB = iXbo ;
    m_iE = iXeo ;
    iY = iYo ;
  };
  Runs( CSegment& AddSegm )
  {
    m_iB = AddSegm.m_Segm.m_iB ;
    m_iE = AddSegm.m_Segm.m_iE ;
    iY = AddSegm.m_iY ;
  }
  void Swap( Runs& Other )
  {
    Runs Tmp = Other ;
    Other = *this ;
    *this = Tmp ;
  }
}	;
class RunAndSpace : public Runs
{
public:
  int m_iSpaceAfter ;
  RunAndSpace() { memset( this , 0 , sizeof( *this ) ) ; } ;
  RunAndSpace( int iXbo , int iXeo , int iYo )
  {
    m_iB = iXbo ;
    m_iE = iXeo ;
    iY = iYo ;
    m_iSpaceAfter = 0 ;
  }
  RunAndSpace( Runs r )
  {
    m_iB = r.m_iB ;
    m_iE = r.m_iE ;
    iY = r.iY ;
    m_iSpaceAfter = 0 ;
  }

}	;


class AdditionalResult
{
public:
  AdditionalResult() { OBJECT_ZERO( *this ) ; }
  AdditionalResult( DWORD dwConditions , cmplx& Cent , double dThres , double dAngle = 360. )
  {
    m_dwConditions = dwConditions ;
    m_Cent = Cent ;
    m_dThreshold = dThres ;
    m_dAngle = dAngle ;
  }

  cmplx m_Cent ;
  cmplx m_Size ;
  double m_dThreshold ;
  double m_dAngle ;
  DWORD  m_dwConditions ;
};

typedef FXArray<AdditionalResult> AddResults ;
typedef FXArray<Runs , Runs&>       RunsArray ;
typedef FXArray<RunsArray , RunsArray&> RunsArrays ;
typedef vector<Runs>        RunsVector ;
typedef vector<RunsVector>  RunsVectors ;

inline Runs& GetLastRun( RunsArrays& Arrays , int iArrayIndex )
{
  return Arrays[ iArrayIndex ].GetAt( Arrays[ iArrayIndex ].GetUpperBound() ) ;
}

class CColorSpot
{
public:
  int      m_iTypeId ;
  char     m_szName[ 30 ] ;
  int      m_iIndex ; // in array of spots, in multispot measurement
  bool     m_bDetailed ; // for compatibility with old ImCNTL_MV 
  int      m_WhatToMeasure ; // this is what to necessary to measure
  int      m_WhatIsMeasured ;

  int      m_iPixelSize ; // 0 - BYTE , 1 - WORD
  int      m_iColor ; // 0 - black, 1 - white
  CRect    m_OuterFrame ;
  CDRect   m_EdgesAsDbl ;
  CDPoint  m_SimpleCenter ;
  CDPoint  m_CenterByIntense ;
  CDPoint  m_CenterByOutFrame ;
  CDPoint  m_CenterByContur ;
  int      m_Area ;
  int      m_iPerimeter ;
  double   m_dPerimeter ;
  int      m_iMaxPixel ;
  int      m_iMinPixel ;
  double   m_dBlobWidth ;
  double   m_dBlobHeigth ;
  double   m_dSumPower;
  double   m_dCentral5x5Sum ;
  double   m_dSumOverThreshold ;
  double   m_iGain;
  double   m_iExposure;


  double   m_dAngle ;
  double   m_dAngleWeighted ;
  double   m_dAngleAlt ;       // Angle by alternate threshold
  double   m_dAngles[ 4 ] ;  // 0 - by rectangle (Eilat)
  // 1 - by runs weighted
  // 2 - by runs unweighted
  cmplx    m_Centers[ 4 ] ;  // indexes are the same with angles
  double   m_dLongDiametr;
  double   m_dShortDiametr;
  cmplx    m_UpLong ;
  cmplx    m_DownLong ;
  cmplx    m_UpShort ;
  cmplx    m_DownShort ;

  double     m_dAccurateArea ;


  //   CSize    m_CoordSum ;
  CSize    m_DiffrRadius ;
  double   m_dCentralIntegral;
  double   m_dRDiffraction ;
  double   m_dLDiffraction ;
  double   m_dUDiffraction ;
  double   m_dDDiffraction ;
  double   m_dIntegrals[ 9 ] ;

  CPoint   m_FirstPoint ;
  int      m_iBefore ;
  int      m_iAfter ;
  int      m_iMatched ;
  int      m_iAddedTo ;
  int      m_iContNum ;
  ImgMoments m_ImgMoments;
  ImgMoments m_ImgMomentsWeighted;

  RunsArray	m_Runs;
  CmplxArray m_Contur ;
  //AddResults m_AddResults ;

  Profile  m_HProfile ;
  Profile  m_VProfile ;

  inline void InitialReset()
  {
    m_iTypeId = SPOT ; //SPOT_TYPE_ID ;
    LPBYTE pBegin = ( LPBYTE )m_szName ;
    LPBYTE pEnd = ( LPBYTE )&m_Runs ;
    memset( pBegin , 0 , pEnd - pBegin ) ; 
    m_Contur.SetSize( 0 , 20 ) ;
  }
  CColorSpot( int iDetailed = 0 )
  {
    InitialReset() ;
    m_WhatToMeasure = iDetailed ;
    if ( m_WhatToMeasure & ( MEASURE_DIAMETERS | MEASURE_ANGLE ) )
    {
      if ( !( m_WhatToMeasure & ( MEASURE_IMG_MOMENTS_W | MEASURE_IMG_MOMENTS_NW ) ) )
        m_WhatToMeasure |= MEASURE_IMG_MOMENTS_NW ;
    }

    m_SimpleCenter.x = m_SimpleCenter.y = -1 ;  // Sign, that spot is not finished
    m_CenterByIntense.x = m_CenterByIntense.y = -1 ;  // Sign, that spot is not finished
    m_iContNum = -1 ;
    m_iAddedTo = -1 ;
  }
  void AllocProfiles( int iHorLen , int iVertLen )
  {
    m_HProfile.Realloc( iHorLen ) ;
    m_VProfile.Realloc( iVertLen ) ;
  }

  CColorSpot( const CColorSpot& Orig )
  {
    LPBYTE pDest = ( LPBYTE )&m_iTypeId ;
    LPBYTE pEnd = ( LPBYTE )&m_Runs ;
    FXSIZE iLen = pEnd - pDest ;
    LPBYTE pSrc = ( LPBYTE )&Orig.m_iTypeId ;
    memcpy( pDest , pSrc , iLen ) ;
    if ( Orig.m_Runs.GetCount() )
      m_Runs.Copy( Orig.m_Runs ) ;

    if ( Orig.m_Contur.GetCount() )
      m_Contur.Copy( Orig.m_Contur ) ;
    else
      m_Contur.SetSize( 0 , 20 ) ;

    if ( Orig.m_HProfile.m_iProfLen )
      m_HProfile.Realloc( Orig.m_HProfile ) ;
    if ( Orig.m_VProfile.m_iProfLen )
      m_VProfile.Realloc( Orig.m_VProfile ) ;
  } ;

  CColorSpot( CSegment& Seg , int iContNum = -1 , int iDetailed = 0 )
  {
    InitialReset() ;
    m_WhatToMeasure = iDetailed  ;
    m_iColor = Seg.m_iColor ;
    m_iMaxPixel = Seg.m_iMaxPixel;
    m_iMinPixel = Seg.m_iMinPixel ;
    m_OuterFrame = Seg.GetRect() ;
    //m_iPerimeter += Seg.GetLen() + 2 ;
//     if ( !( m_WhatToMeasure & ( MEASURE_IMG_MOMENTS_NW | MEASURE_IMG_MOMENTS_W ) ) )
      m_Area += Seg.GetLen() ;
//     else
//    {
      if (( iDetailed & MEASURE_IMG_MOMENTS_W ))
        m_ImgMomentsWeighted.Add( Seg.m_ImgMoments ) ;
      else if (iDetailed & MEASURE_IMG_MOMENTS_NW)
        m_ImgMoments.Add( Seg.m_ImgMoments ) ;
//    }

    m_CenterByIntense.x = m_CenterByIntense.y =
      m_SimpleCenter.x = m_SimpleCenter.y = -1. ;  // Sign, that spot is not finished

    if ( m_WhatToMeasure & MEASURE_RUNS )
      m_Runs.Add( Runs( Seg ) ) ;

    m_iContNum = ( iContNum >= 0 ) ? iContNum : Seg.m_iCont ;
    m_iAddedTo = -1 ;
    m_FirstPoint = CPoint( Seg.m_Segm.m_iB , Seg.m_iY ) ;
  } ;
  const CColorSpot& operator = ( const CColorSpot& Orig )
  {
    m_iTypeId = Orig.m_iTypeId ; // 100
    LPBYTE pDest = ( LPBYTE )this ;
    LPBYTE pEnd = ( LPBYTE )&m_Runs ;
    FXSIZE iLen = pEnd - pDest ;
    LPBYTE pSrc = ( LPBYTE )&Orig ;
    memcpy( pDest , pSrc , iLen ) ;
    if ( m_WhatToMeasure & MEASURE_RUNS )
      m_Runs.Copy( Orig.m_Runs ) ;
    if ( m_WhatToMeasure & MEASURE_CONTUR )
      m_Contur.Copy( Orig.m_Contur ) ;

    if ( Orig.m_HProfile.m_iProfLen )
      m_HProfile.Realloc( Orig.m_HProfile ) ;
    if ( Orig.m_VProfile.m_iProfLen )
      m_VProfile.Realloc( Orig.m_VProfile ) ;

    return *this;
  }
  CColorSpot& Add( CSegment& Segm )
  {
    if ( m_OuterFrame.left > Segm.m_Segm.m_iB )
      m_OuterFrame.left = Segm.m_Segm.m_iB ;
    if ( m_OuterFrame.top > Segm.m_iY )
      m_OuterFrame.top = Segm.m_iY ;
    if ( m_OuterFrame.right < Segm.m_Segm.m_iE )
      m_OuterFrame.right = Segm.m_Segm.m_iE ;
    if ( m_OuterFrame.bottom < Segm.m_iY )
      m_OuterFrame.bottom = Segm.m_iY ;

//    m_iPerimeter += Segm.GetLen() + 2 ;  // not correct !!!!!!
    m_Area += Segm.GetLen() ;
    if ( ( m_WhatToMeasure & MEASURE_IMG_MOMENTS_W ) )
      m_ImgMomentsWeighted.Add( Segm.m_ImgMoments ) ;
    if ( m_WhatToMeasure & MEASURE_IMG_MOMENTS_NW )
      m_ImgMoments.Add( Segm.m_ImgMoments ) ;

    if ( Segm.m_iMaxPixel > m_iMaxPixel )
      m_iMaxPixel = Segm.m_iMaxPixel ;
    if ( Segm.m_iMinPixel < m_iMinPixel )
      m_iMinPixel = Segm.m_iMinPixel ;
    if ( m_WhatToMeasure & MEASURE_RUNS )
      m_Runs.Add( Runs( Segm ) ) ;

    ASSERT( Segm.m_iColor == m_iColor ) ;
    Segm.m_iMatched++ ;
    if ( Segm.m_iCont < 0 )
      Segm.m_iCont = m_iContNum ;
    return *this ;
  } ;
  CColorSpot& operator += ( CSegment& Segm )
  {
    return Add( Segm ) ;
  }
  CColorSpot& Add( CColorSpot& Other )
  {
    if ( m_iContNum != Other.m_iContNum )
    {
      if ( m_OuterFrame.left > Other.m_OuterFrame.left )
        m_OuterFrame.left = Other.m_OuterFrame.left ;
      if ( m_OuterFrame.top > Other.m_OuterFrame.top )
        m_OuterFrame.top = Other.m_OuterFrame.top ;
      if ( m_OuterFrame.right < Other.m_OuterFrame.right )
        m_OuterFrame.right = Other.m_OuterFrame.right ;
      if ( m_OuterFrame.bottom < Other.m_OuterFrame.bottom )
        m_OuterFrame.bottom = Other.m_OuterFrame.bottom ;

      if ( Other.m_FirstPoint.y < m_FirstPoint.y )
        m_FirstPoint = Other.m_FirstPoint ;
      m_iMaxPixel = max( Other.m_iMaxPixel , m_iMaxPixel );
      if ( Other.m_iMinPixel < m_iMinPixel )
        m_iMinPixel = Other.m_iMinPixel ;

      //m_iPerimeter += Other.m_iPerimeter ;  // not correct !!!!!!
      m_Area += Other.m_Area ;
      if ( m_WhatToMeasure & MEASURE_IMG_MOMENTS_W )
        m_ImgMomentsWeighted.Add( Other.m_ImgMomentsWeighted ) ;
      else if ( m_WhatToMeasure & MEASURE_IMG_MOMENTS_NW )
        m_ImgMoments.Add( Other.m_ImgMoments ) ;
      Other.m_Area = -Other.m_Area ;

      if ( m_WhatToMeasure & MEASURE_RUNS )
        m_Runs.Append( Other.m_Runs ) ;

      Other.m_iAddedTo = m_iContNum ;
    }
    return *this ;
  } ;
  CColorSpot& operator += ( CColorSpot& Other )
  {
    return Add( Other ) ;
  } ;
  void EndSpot( pTVFrame pImage )
  {
    if ( m_Area < 0 || m_iAddedTo >= 0 )
      return ;

    m_CenterByOutFrame = CDPoint(
      ( double )m_OuterFrame.left + ( double )m_OuterFrame.right ,
      ( double )m_OuterFrame.top + ( double )m_OuterFrame.bottom ) * 0.5 ;
    m_WhatIsMeasured |= MEASURE_POSITION | MEASURE_AREA ;
    if ( m_WhatToMeasure & MEASURE_IMG_MOMENTS_W )
    {
      if ( m_ImgMomentsWeighted.m_dAreaPixels > 0 )
      {
        m_Area = ROUND( m_ImgMomentsWeighted.m_dAreaPixels ) ;
        cmplx Cent = m_ImgMomentsWeighted.GetCenter() ;
        m_SimpleCenter = m_CenterByIntense = CmplxToCDPoint( Cent ) ;
        m_dAngle = m_dAngleWeighted = -m_ImgMomentsWeighted.GetAngle() ; // Y is directed to down
        m_dAngleWeighted = RadToDeg( m_dAngleWeighted ) ;
        m_WhatIsMeasured |= MEASURE_IMG_MOMENTS_W | MEASURE_ANGLE ;
      }
      else
      {
        m_Area = 1 ;
        m_dAngle = 0. ;
        m_SimpleCenter = m_CenterByOutFrame ;
      }
    }
    else if ( m_WhatToMeasure & MEASURE_IMG_MOMENTS_NW )
    {
      if ( m_ImgMoments.m_dAreaPixels > 0 )
      {
        m_Area = ROUND( m_ImgMoments.m_dAreaPixels ) ;
        cmplx SCent = m_ImgMoments.GetCenter() ;
        m_SimpleCenter = CmplxToCDPoint( SCent ) ;
        m_dAngle = -m_ImgMoments.GetAngle() ; // Y is directed to down
        m_dAngle = RadToDeg( m_dAngle ) ;
        m_WhatIsMeasured |= MEASURE_IMG_MOMENTS_NW | MEASURE_ANGLE ;
      }
      else
      {
        m_Area = 1 ;
        m_dAngle = 0. ;
        m_SimpleCenter = m_CenterByOutFrame ;
      }
    }
    else
    {
      m_Centers[ 0 ] = CDPointToCmplx( m_CenterByOutFrame );

      m_SimpleCenter = m_CenterByOutFrame ;
    }
    ASSERT( m_Area > 0 ) ;
    //     if ( m_iColor == 0 ) // black contur
    //       m_dAngle = -m_dAngle ;
  }
  void ToString( FXString& Result , LPCTSTR pName = NULL )
  {
    Result.Format(
      "%d %7.2f %7.2f %6.2f %6.2f %d %d %8.1f %8.1f %7.2f %7.2f %7.2f \
            %6.2f %6.2f %6.2f %6.2f %8.1f %d %d %d %d Name=%s; \n" ,
            m_iIndex ,
            m_SimpleCenter.x , m_SimpleCenter.y ,
            m_dBlobWidth ,
            m_dBlobHeigth ,
            m_Area ,
            m_iMaxPixel ,
            m_dCentral5x5Sum ,
            m_dSumOverThreshold ,
            m_dAngles[ 0 ] ,
            m_dLongDiametr ,
            m_dShortDiametr ,
            100. *m_dRDiffraction ,
            100. *m_dLDiffraction ,
            100. *m_dDDiffraction ,
            100. *m_dUDiffraction ,
            100. *m_dCentralIntegral ,
            m_OuterFrame.left ,
            m_OuterFrame.top ,
            m_OuterFrame.right ,
            m_OuterFrame.bottom ,
            pName ? pName : "\0"
            ) ;
  }
  bool FromString( FXString& Src , FXString * pName = NULL )
  {
    if ( 21 == sscanf( ( LPCTSTR )Src ,
      "%d %lf %lf %lf %lf %d %d %lf %lf %lf %lf %lf \
              %lf %lf %lf %lf %lf %d %d %d %d" ,
              &m_iIndex ,
              &m_SimpleCenter.x , &m_SimpleCenter.y ,
              &m_dBlobWidth ,
              &m_dBlobHeigth ,
              &m_Area ,
              &m_iMaxPixel ,
              &m_dCentral5x5Sum ,
              &m_dSumOverThreshold ,
              &m_dAngles[ 0 ] ,
              &m_dLongDiametr ,
              &m_dShortDiametr ,
              &m_dRDiffraction ,
              &m_dLDiffraction ,
              &m_dDDiffraction ,
              &m_dUDiffraction ,
              &m_dCentralIntegral ,
              &m_OuterFrame.left ,
              &m_OuterFrame.top ,
              &m_OuterFrame.right ,
              &m_OuterFrame.bottom ) )
    {
      if ( pName )
      {
        int iNamePos = (int) Src.Find( "Name=" ) ;
        if ( iNamePos > 0 )
        {
          pName->Empty() ;
          iNamePos += 5 ;
          LPCTSTR pChar = ( ( LPCTSTR )( Src ) ) + iNamePos ;
          while ( *pChar  &&  *pChar != _T( ';' ) )
            *pName += *(pChar++) ;
          _tcscpy_s(m_szName, (LPCTSTR)(*pName));
        }
      }
      m_dAngle = m_dAngles[ 0 ] ;
      m_dRDiffraction /= 100. ;
      m_dLDiffraction /= 100 ;
      m_dDDiffraction /= 100. ;
      m_dUDiffraction /= 100. ;
      m_dCentralIntegral /= 100. ;
      return true ;
    }
    return false ;
  }
  BOOL Serialize( LPBYTE* ppData , UINT* pDataLen ) const
  {
    if ( !ppData )
      return FALSE ;
    LPBYTE pBegin = ( LPBYTE )&m_iTypeId ;
    LPBYTE pEnd = ( LPBYTE )&m_dAngle ;

    if ( m_WhatIsMeasured & MEASURE_DIFFRACT )
      pEnd = ( LPBYTE )&m_FirstPoint ;
    else if ( m_WhatIsMeasured & ( MEASURE_DIAMETERS | MEASURE_ANGLE | MEASURE_RATIO ) )
      pEnd = ( LPBYTE )&m_DiffrRadius ;

    UINT cb = (UINT)(pEnd - pBegin) ;
    UINT uiAllocLen = cb + sizeof( UINT ); // block len will be written before real data

    UINT uiOldDataLen = *pDataLen ;
    *pDataLen += uiAllocLen ;
    *ppData = ( LPBYTE )realloc( *ppData , *pDataLen );
    LPBYTE ptr = *ppData + uiOldDataLen ;
    *( ( UINT* )ptr ) = uiAllocLen ;
    ptr += sizeof( uiAllocLen ) ;
    memcpy( ptr , this , cb );
    return TRUE;
  }
  BOOL Restore( LPBYTE pData , UINT uiDataLen )
  {
    if ( !pData || !uiDataLen )
    {
      ASSERT( 0 ) ;
      return FALSE ;
    }

    m_Runs.RemoveAll() ;
    m_Contur.RemoveAll() ;
    m_HProfile.Reset() ;
    m_VProfile.Reset() ;

    UINT uiBlockLen = *( ( UINT* )pData ) ;
    UINT uiSpotDataLen = uiBlockLen - sizeof( uiBlockLen ) ;
    pData += sizeof( uiBlockLen ) ;
    if ( uiBlockLen < uiDataLen )
    {
      ASSERT( 0 ) ; // Not enough data for necessary block len
      return FALSE ;
    }
    LPBYTE pBegin = ( LPBYTE )&m_iTypeId ;
    LPBYTE pSimpleDataEnd = ( LPBYTE )&m_Runs ;
    if ( uiSpotDataLen > ( UINT )( pSimpleDataEnd - pBegin ) )
    {
      ASSERT( 0 ) ;  // too much block len for object
      return FALSE ;
    }

    memcpy( ( LPVOID )this , pData , uiSpotDataLen );
    return uiBlockLen ;
  }
};

class SpotArray : public FXArray < CColorSpot , CColorSpot& >
{
#if (_MSC_VER<1300)
public:
  int GetCount() { return GetUpperBound( )+1; }
#endif
};

inline CColorSpot * GetSpotData( SpotArray& SpotsData , LPCTSTR pName )
{
  for ( int i = 0 ; i < SpotsData.Size() ; i++ )
  {
    if ( _tcsstr( pName , SpotsData[ i ].m_szName ) == pName )
      return &SpotsData[ i ] ;
  }
  return NULL ;
}

typedef SpotArray *pSpotArray;
typedef FXArray< pSpotArray > SpotArrays ;

typedef vector<CColorSpot> SpotVector ;
typedef vector<SpotVector> SpotVectors ;

class CSegmentation
{
public:
  CSegmentation( int iNObjectsMax = 5000 , double dTimeout = 0. )
  {
    m_pCurr = m_pPrev = NULL ;
    m_MinColor = 0 ;
    m_MaxColor = 127 ;
    m_iWidth = m_iHeight = 0 ;
    m_pFrame = NULL ;
    m_pBitBuf = NULL ;
    m_WhatToMeasure = 0 ;
    m_iNMaxContours = iNObjectsMax ;
    m_dTimeout = dTimeout  ;
    m_Status[ 0 ] = 0 ;
  }
  ~CSegmentation() {} ;

  SpotArray   m_ColSpots ;
  FXUintArray m_ActiveSpots ;
  SegmentArray    m_Segm1 , m_Segm2 ;
  SegmentArray *  m_pCurr , *m_pPrev ;
  BYTE m_MinColor;
  BYTE m_MaxColor;
  WORD m_wMinColor ;
  WORD m_wMaxColor ;
  int  m_iNMaxContours ;
  double m_dTimeout ; // in ms
  double m_dStartTime ;
  TCHAR  m_Status[ 200 ] ;



  int         m_iWidth ;
  int         m_iHeight ;

  LPBYTE      m_pBitBuf ;
  LPWORD      m_pWordBuf ;
  pTVFrame    m_pFrame ;
  int         m_WhatToMeasure ;

  void		SetClusterColors( int min , int max )
  {
    m_MinColor = ( BYTE )( m_wMinColor = ( WORD )min );
    m_MaxColor = ( BYTE )( m_wMaxColor = ( WORD )max );
  };
  void		GetClusterColors( WORD* min , WORD* max )
  {
    if ( min ) *min = m_wMinColor; if ( max ) *max = m_wMaxColor;
  };
  int         FindSpots( pTVFrame pFrame , BOOL iDetailed = FALSE ) ;
  int         SimpleThres( int iY , CalcImageMoments CalcMoments = NoImageMoments ) ;
  void        MeasureSpotSize( CColorSpot& Spot ) ;
  pSpotArray  GetSpotsInfo() { return &m_ColSpots ; } ;
  void        SetNMax( int iNMaxCont ) { m_iNMaxContours = iNMaxCont ; } ;
  int         GetNMax( void ) { return m_iNMaxContours ; } ;
  void        SetTimeout( double dTimeout ) { m_dTimeout = dTimeout ; } ;
  double      GetTimeout() { return m_dTimeout ; } ;
  double      GetWorkingTime()
  {
    return (GetHRTickCount() - m_dStartTime) ;
  }
  bool        IsTimeout()
  {
    return ((m_dTimeout > 0.) && (GetWorkingTime() > m_dTimeout)) ;
  }

};

__forceinline void Spots_Exchange( pSpotArray Spots , int pos1 , int pos2 )
{
  CColorSpot tmpP( Spots->ElementAt( pos1 ) ) ;
  Spots->SetAt( pos1 , Spots->ElementAt( pos2 ) ) ;
  Spots->SetAt( pos2 , tmpP ) ;
}

__forceinline void Spots_SortX( pSpotArray Spots )
{
  bool sorted = true;
  CColorSpot * clusters = Spots->GetData() ;
  do
  {
    sorted = true;
    for ( int i = 0; i < Spots->GetCount() - 1; i++ )
    {
      if ( ( clusters[ i ].m_SimpleCenter.x ) > ( clusters[ i + 1 ].m_SimpleCenter.x ) )
      {
        Spots_Exchange( Spots , i , i + 1 );
        sorted = false;
      }
    }
  }
  while ( !sorted );
}

__forceinline void Spots_SortArea( pSpotArray Spots )
{
  bool sorted = true;
  CColorSpot * clusters = Spots->GetData() ;
  do
  {
    sorted = true;
    for ( int i = 0 ; i < Spots->GetCount() - 1 ; i++ )
    {
      if ( ( clusters[ i ].m_Area ) > ( clusters[ i + 1 ].m_Area ) )
      {
        Spots_Exchange( Spots , i , i + 1 );
        sorted = false;
      }
    }
  }
  while ( !sorted );
}

__forceinline int Spots_GetOverlappedX( CColorSpot * a , CColorSpot * b )
{
  int la = a->m_OuterFrame.left , ra = a->m_OuterFrame.right;
  int lb = b->m_OuterFrame.left , rb = b->m_OuterFrame.right;
  if ( la < lb ) la = lb;
  if ( ra > rb ) ra = rb;
  if ( ra <= la ) return 0;
  return ( ra - la );
}

__forceinline int Spots_GetOverlappedY( CColorSpot * a , CColorSpot * b )
{
  int la = a->m_OuterFrame.top , ra = a->m_OuterFrame.bottom;
  int lb = b->m_OuterFrame.top , rb = b->m_OuterFrame.bottom;
  if ( la < lb ) la = lb;
  if ( ra > rb ) ra = rb;
  if ( ra <= la ) return 0;
  return ( ra - la );
}

__forceinline int Spots_GetXYPos( CColorSpot& a , int width )
{
  int y = ( 2 * a.m_OuterFrame.top ) / ( 3 * a.m_OuterFrame.Height() );
  return ( a.m_OuterFrame.left + y*width );
}

__forceinline void Spots_SortLinepos( pSpotArray Spots , int fwidth )
{
  bool sorted = true;
  do
  {
    sorted = true;
    for ( int i = 0; i < Spots->GetCount() - 1; i++ )
    {
      if ( Spots_GetXYPos( ( *Spots )[ i ] , fwidth ) > Spots_GetXYPos( ( *Spots )[ i + 1 ] , fwidth ) )
      {
        Spots_Exchange( Spots , i , i + 1 );
        sorted = false;
      }
    }
  }
  while ( !sorted );

  /*    int clstrnmb=pCI->m_ClustersNmb;
  pSpotArray Spots=pCI->m_Clusters;
  Spots_SortX(clusters, clstrnmb);
  for (int i=0; i<clstrnmb-1; i++)
  {
  Spots_GetOverlappedX(&clusters[i],&clusters[i+1]);
  } */
}


__forceinline void Spots_RemoveAt( pSpotArray Spots , int pos )
{
  Spots->RemoveAt( pos ) ;
}

__forceinline void Spots_LeaveLargest( pSpotArray Spots , int& length )
{
  if ( Spots->GetCount() == 0 )
    return ;
  CColorSpot Biggest = ( *Spots )[ 0 ] ;
  for ( int i = 1 ; i < Spots->GetCount() ; i++ )
  {
    if ( Biggest.m_Area < ( *Spots )[ i ].m_Area )
      Biggest = ( *Spots )[ i ] ;
  }
  Spots->RemoveAll() ;
  Spots->Add( Biggest ) ;
}

__forceinline void Spots_RemoveDeleted( pSpotArray Spots )
{
  for ( int i = 0; i < Spots->GetCount(); i++ )
  {
    if ( ( *Spots )[ i ].m_iAddedTo >= 0 )
    {
      Spots_RemoveAt( Spots , i );
      i--;
    }
  }
}

__forceinline void Spots_RemoveAll( pSpotArray Spots )
{
  Spots->RemoveAll() ;
}

__forceinline void Spots_MinMaxAreaFilter(
  pSpotArray Spots , int min , int max , bool remove = true )
{
  CColorSpot * pData = Spots->GetData() ;
  for ( int i = 0; i < Spots->GetCount(); i++ )
  {
    if ( max > 0 )
    {
      if ( pData[ i ].m_Area > max ) pData[ i ].m_iAddedTo = 100000000 ;
    }
    if ( min > 0 )
    {
      if ( pData[ i ].m_Area < min ) pData[ i ].m_iAddedTo = 100000000 ;
    }
  }
  if ( remove ) { Spots_RemoveDeleted( Spots ); }
}

__forceinline void Spots_XYRatioFilter(
  pSpotArray Spots , double min , double max , bool remove = true )
{
  CColorSpot * clusters = Spots->GetData() ;
  for ( int i = 0; i < Spots->GetCount(); i++ )
  {
    ASSERT( clusters[ i ].m_OuterFrame.Height() != 0 );
    double ratio = ( double )clusters[ i ].m_OuterFrame.Width() / ( double )clusters[ i ].m_OuterFrame.Height();
    if ( max > 0 )
    {
      if ( ratio > max ) { clusters[ i ].m_iAddedTo = 100000000 ; continue; }
    }
    if ( min > 0 )
    {
      if ( ratio < min ) { clusters[ i ].m_iAddedTo = 100000000 ; }
    }
  }
  if ( remove ) { Spots_RemoveDeleted( Spots ); }
}

__forceinline void Spots_HeightFilter( pSpotArray Spots , int min , int max , bool remove = true )
{
  CColorSpot * clusters = Spots->GetData() ;
  for ( int i = 0; i < Spots->GetCount(); i++ )
  {
    int iHeight = clusters[ i ].m_OuterFrame.Height() ;
    if ( max > 0 )
    {
      if ( iHeight > max )
      {
        clusters[ i ].m_iAddedTo = 100000000 ; continue;
      }
    }
    if ( min > 0 )
    {
      if ( iHeight < min ) { clusters[ i ].m_iAddedTo = 100000000 ; }
    }
  }
  if ( remove ) { Spots_RemoveDeleted( Spots ); }
}

__forceinline void Spots_WidthFilter( pSpotArray Spots , int min , int max , bool remove = true )
{
  CColorSpot * clusters = Spots->GetData() ;
  for ( int i = 0; i < Spots->GetCount(); i++ )
  {
    int iWidth = clusters[ i ].m_OuterFrame.Width() ;
    if ( max > 0 )
    {
      if ( iWidth > max )
      {
        clusters[ i ].m_iAddedTo = 100000000 ; continue;
      }
    }
    if ( min > 0 )
    {
      if ( iWidth < min ) { clusters[ i ].m_iAddedTo = 100000000 ; }
    }
  }
  if ( remove ) { Spots_RemoveDeleted( Spots ); }
}

#endif // !defined(AFX_CSPOT_H__132A0B73_DC7B_482F_B004_226F86AB2F2A__INCLUDED_)



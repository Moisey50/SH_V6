/********************************************************************
created:	2005/11/10
created:	10:11:2005   11:41
filename: 	segmentation.h
file path:	c:\msc_proj\newcomponents\imcntlmv_2
file base:	segmentation
file ext:	h
author:Moisey Bernstein 		

purpose: definitions and operators for algorithm of finding blobs
(FindSpots)
*********************************************************************/

#if !defined(AFX_CSPOT_H__132A0B73_DC7B_482F_B004_226F86AB2F2A__INCLUDED_)
#define AFX_CSPOT_H__132A0B73_DC7B_482F_B004_226F86AB2F2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include <classes\dpoint.h>
#include <math\Intf_sup.h>
#include <helpers\FramesHelper.h>
#include <video\tvframe.h>
#ifndef TVDB300_APPL
#include <Gadgets\gadbase.h>
#include "imageproc\SeekSpots.h"
#endif

class HorSegm
{
public:
  int m_iB ;              //   B   E
  int m_iE ;              //  B  E

  HorSegm() { m_iB = m_iE = 0; } ;
  HorSegm( int iB , int iE) { m_iB = iB ; m_iE = iE ; } ;
  bool Overlap( HorSegm& Other ) 
  { 
    if (m_iB<=Other.m_iB)
      return (m_iE>=Other.m_iB) ;
    else 
      return (m_iB<=Other.m_iE) ;
  } ;
  int GetLen() { return (m_iE - m_iB + 1) ; } ;
  int GetSumCoord() { return ((GetLen()*(m_iB+m_iE))/2) ; };
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
  int m_iMatched ;
  int m_iBrSum ;              //  Brightness
  int m_iSumBrMulX;         // Brightness multiple X coordinates
  int m_iSumBrMulY;         // Brightness multiple Y coordinates
  HorSegm m_Segm ;
  CSegment()
  {
    m_iY = -1 ;
    m_iColor = -1 ;
    m_iCont = -1 ;
    m_Segm.m_iB = m_Segm.m_iE = -1 ;
    m_iMatched = 0 ; m_iMaxPixel = -1; m_iBrSum = 0; m_iSumBrMulX = 0;
    m_iSumBrMulY = 0;
  }
  CSegment( CSegment& Orig )
  {
    m_iY = Orig.m_iY ;
    m_iColor = Orig.m_iColor ;
    m_iCont = Orig.m_iCont ;
    m_Segm = Orig.m_Segm ;
    m_iBrSum = Orig.m_iBrSum;
    m_iSumBrMulX = Orig.m_iSumBrMulX;
    m_iSumBrMulY = Orig.m_iSumBrMulY;
    m_iMatched = 0 ;
  }
  CSegment( int iY , int iColor , HorSegm Segm) 
  {
    m_iY=iY ; m_iColor= iColor ; m_Segm=Segm ; m_iCont = -1 ;  
    m_iMatched = 0 ;
  } ;
  CSegment( int iY , int iColor , int iB , int iE , 
    int iMaxPixel,int iBrSum,int iBrMulX) 
  {
    m_iY=iY ; m_iColor=iColor ; m_Segm.m_iB=iB ; m_Segm.m_iE=iE ; 
    m_iCont = -1 ; 
    m_iMatched = 0 ;	
    m_iMaxPixel = iMaxPixel; 
    m_iBrSum = iBrSum; 
    m_iSumBrMulX = iBrMulX;
    m_iSumBrMulY = iY * iBrSum;

  } ;
  CSegment( int iY , int iColor , int iB , int iE ) 
  {
    m_iY=iY ; m_iColor=iColor ; m_Segm.m_iB=iB ; m_Segm.m_iE=iE ; 
    m_iCont = -1 ; 
    m_iMatched = 0 ;	
    m_iMaxPixel = -1; 
    m_iBrSum = 0; 
    m_iSumBrMulX = 0;
    m_iSumBrMulY = 0;

  } ;
  bool Match( CSegment& Other ) 
  {
    if (m_iColor == Other.m_iColor)
      return m_Segm.Overlap(Other.m_Segm) ;
    return false ;
  }
  bool Match( CDPoint& Pt )
  {
    return (( m_Segm.m_iB <= Pt.x)  &&  (Pt.x <= m_Segm.m_iE) ) ;
  }
  CSize GetSumCoord() 
  { 
    return CSize(m_Segm.GetSumCoord(),m_iY*m_Segm.GetLen()) ; 
  } ;
  CRect GetRect() 
  { 
    return CRect(m_Segm.m_iB , m_iY , m_Segm.m_iE , m_iY) ;
  }
};

typedef CArray<CSegment,CSegment&> SegmentArray;

class Runs : public HorSegm
{
public:
  int iY ;
  Runs() { iY = 0 ; } ;
  Runs( int iXbo , int iXeo , int iYo )
  {
    m_iB = iXbo ;
    m_iE = iXeo ;
    iY = iYo ;
  };
  Runs( CSegment& AddSegm ) 
  {
    m_iB = AddSegm.m_Segm.m_iB ;
    m_iE = AddSegm.m_Segm.m_iB ;
    iY = AddSegm.m_iY ;
  }
}	;


class CColorSpot
{
public:
  bool     m_bDetailed ;
  int      m_WhatIsMeasured ;
  int      m_iColor ; // 0 - black, 1 - white
  CRect    m_OuterFrame ;
  CSize    m_CoordSum ;
  CSize    m_DiffrRadius ;
  CPoint   m_FirstPoint ;
  int      m_iBefore ;
  int      m_iAfter ;
  CDPoint  m_SimpleCenter ;
  CDPoint  m_CenterByIntense ;
  int      m_Area ;
  int      m_iMaxPixel ;
  int      m_iMatched ;
  int      m_iAddedTo ;
  int      m_iContNum ;
  int      m_iBrSum;
  int      m_iSumBrMulX;         // Brightness multiple X coordinates
  int      m_iSumBrMulY;          // Brightness multiple Y coordinates 

  double   m_dRDiffraction ;
  double   m_dLDiffraction ;
  double   m_dUDiffraction ;
  double   m_dDDiffraction ;
  double   m_dBlobWidth ;
  double   m_dBlobHeigth ;
  double   m_dCentralIntegral;
  double   m_dSumPower;
  double   m_dCentral5x5Sum ;
  double   m_dSumOverThreshold ;
  double   m_dAngle;
  double   m_dLongDiametr;
  double   m_dShortDiametr;
  Profile  m_HProfile ;
  Profile  m_VProfile ;
  double   m_dIntegrals[9] ;
  cmplx    m_UpLong ;
  cmplx    m_DownLong ;
  cmplx    m_UpShort ;
  cmplx    m_DownShort ;

  double  m_iGain;
  double  m_iExposure;

  ImgMoments m_ImgMoments;

  CArray<Runs,Runs&>	m_Runs;
  CmplxArray m_Contur ;

  CColorSpot( bool bDetailed = false )
  {
    LPBYTE pBegin = (LPBYTE)&m_WhatIsMeasured ;
    LPBYTE pEnd = (LPBYTE)&m_Runs ;
    memset( pBegin , 0 , pEnd - pBegin ) ;
    m_bDetailed = bDetailed ;
    m_SimpleCenter.x = m_SimpleCenter.y = -1 ;  // Sign, that spot is not finished
    m_CenterByIntense.x = m_CenterByIntense.y = -1 ;  // Sign, that spot is not finished
    m_iContNum = -1 ;
    m_iAddedTo = -1 ;
  }
  void AllocProfiles( int iHorLen , int iVertLen )
  {
    m_HProfile.Realloc( iHorLen ) ;
    m_VProfile.Realloc( iVertLen) ;
  }

  CColorSpot( CColorSpot& Orig )
  {
    m_bDetailed = Orig.m_bDetailed ;
    m_WhatIsMeasured = Orig.m_WhatIsMeasured ;
    m_iColor = Orig.m_iColor ;
    m_iMaxPixel = Orig.m_iMaxPixel;
    m_OuterFrame = Orig.m_OuterFrame ;
    m_CoordSum = Orig.m_CoordSum ;
    m_SimpleCenter = Orig.m_SimpleCenter ;
    m_iBrSum = Orig.m_iBrSum;
    m_iSumBrMulX = Orig.m_iSumBrMulX;
    m_iSumBrMulY = Orig.m_iSumBrMulY;
    m_Area = Orig.m_Area ;
    m_iContNum = Orig.m_iContNum ;
    m_iAddedTo = Orig.m_iAddedTo ;
    m_dBlobWidth = Orig.m_dBlobWidth;
    m_dBlobHeigth = Orig.m_dBlobHeigth;
    m_iMaxPixel = Orig.m_iMaxPixel;
    m_FirstPoint = Orig.m_FirstPoint ;
    m_iBefore = Orig.m_iBefore ;
    m_iAfter = Orig.m_iAfter ;
    if ( m_bDetailed )
    {
      m_CenterByIntense = Orig.m_CenterByIntense;
      m_dRDiffraction = Orig.m_dRDiffraction;
      m_dLDiffraction = Orig.m_dLDiffraction;
      m_dUDiffraction = Orig.m_dUDiffraction;
      m_dDDiffraction = Orig.m_dDDiffraction;
      m_dSumPower = Orig.m_dSumPower;
      m_dCentralIntegral= Orig.m_dCentralIntegral;
      m_dSumPower = Orig.m_dSumPower;
      m_dCentral5x5Sum = Orig.m_dCentral5x5Sum ;
      m_dSumOverThreshold = Orig.m_dSumOverThreshold ;
      m_dLongDiametr = Orig.m_dLongDiametr;
      m_dShortDiametr = Orig.m_dShortDiametr;
      m_dAngle = Orig.m_dAngle;
      m_DiffrRadius = Orig.m_DiffrRadius ;
      memcpy( m_dIntegrals , Orig.m_dIntegrals , sizeof(m_dIntegrals) ) ;
      m_Runs.Copy( Orig.m_Runs ) ;
      m_Contur.Copy( Orig.m_Contur ) ;
    }
    if ( Orig.m_HProfile.m_iProfLen )
      m_HProfile.Realloc( Orig.m_HProfile ) ;
    if ( Orig.m_VProfile.m_iProfLen )
      m_VProfile.Realloc( Orig.m_VProfile ) ;
  } ;

  CColorSpot( CSegment& Seg , int iContNum = -1 , bool bDetailed = false ) 
  {
    LPBYTE pBegin = (LPBYTE)&m_WhatIsMeasured ;
    LPBYTE pEnd = (LPBYTE)&m_Runs ;
    memset( pBegin , 0 , pEnd - pBegin ) ;
    m_bDetailed = bDetailed ;
    m_iColor = Seg.m_iColor ;
    m_iMaxPixel = Seg.m_iMaxPixel;
    m_OuterFrame = Seg.GetRect() ;
    m_CoordSum = Seg.GetSumCoord() ;
    m_iBrSum = Seg.m_iBrSum;
    m_iSumBrMulX = Seg.m_iSumBrMulX;
    m_iSumBrMulY = Seg.m_iSumBrMulY;

    m_Area = Seg.m_Segm.GetLen() ;
    m_SimpleCenter.x = m_SimpleCenter.y = -1 ;  // Sign, that spot is not finished
    if ( m_bDetailed )
    {
      m_CenterByIntense.x = m_CenterByIntense.y = -1 ;  // Sign, that spot is not finished
      m_Runs.Add( Runs( Seg.m_Segm.m_iB , Seg.m_Segm.m_iE , Seg.m_iY) ) ;
    }

    m_iContNum = (iContNum > 0) ? iContNum : Seg.m_iCont ;
    m_iAddedTo = -1 ;
    m_FirstPoint = CPoint( Seg.m_Segm.m_iB , Seg.m_iY ) ;

  } ;
  const CColorSpot& operator = ( const CColorSpot& Orig)
  {
    m_bDetailed = Orig.m_bDetailed ;
    m_WhatIsMeasured = Orig.m_WhatIsMeasured ;
    m_iColor = Orig.m_iColor ;
    m_iMaxPixel = Orig.m_iMaxPixel;
    m_OuterFrame = Orig.m_OuterFrame ;
    m_CoordSum = Orig.m_CoordSum ;
    m_SimpleCenter = Orig.m_SimpleCenter ;
    m_CenterByIntense = Orig.m_CenterByIntense ;
    m_Area = Orig.m_Area ;
    m_iBrSum = Orig.m_iBrSum;
    m_iSumBrMulX = Orig.m_iSumBrMulX;
    m_iSumBrMulY = Orig.m_iSumBrMulY;
    m_iContNum = Orig.m_iContNum ;
    m_iAddedTo = Orig.m_iAddedTo ;
    m_dBlobWidth = Orig.m_dBlobWidth;
    m_dBlobHeigth = Orig.m_dBlobHeigth;
    m_iMaxPixel = Orig.m_iMaxPixel;
    m_FirstPoint = Orig.m_FirstPoint ;
    m_iBefore = Orig.m_iBefore ;
    m_iAfter = Orig.m_iAfter ;
    if ( m_bDetailed )
    {
      m_dRDiffraction = Orig.m_dRDiffraction;
      m_dLDiffraction = Orig.m_dLDiffraction;
      m_dUDiffraction = Orig.m_dUDiffraction;
      m_dDDiffraction = Orig.m_dDDiffraction;
      m_dSumPower = Orig.m_dSumPower;
      m_dCentralIntegral= Orig.m_dCentralIntegral;
      m_dSumPower = Orig.m_dSumPower;
      m_dCentral5x5Sum = Orig.m_dCentral5x5Sum ;
      m_dSumOverThreshold = Orig.m_dSumOverThreshold ;
      m_dLongDiametr = Orig.m_dLongDiametr;
      m_dShortDiametr = Orig.m_dShortDiametr;
      m_dAngle = Orig.m_dAngle;
      m_DiffrRadius = Orig.m_DiffrRadius ;
      m_iGain = Orig.m_iGain;
      m_iExposure = Orig.m_iExposure;
      memcpy( &m_ImgMoments , &Orig.m_ImgMoments , sizeof(m_ImgMoments) ) ;
      memcpy( m_dIntegrals , Orig.m_dIntegrals , sizeof(m_dIntegrals) ) ;
      m_Runs.Append( Orig.m_Runs ) ;
      m_Contur.Copy( Orig.m_Contur ) ;
    }
    if ( Orig.m_HProfile.m_iProfLen )
      m_HProfile.Realloc( Orig.m_HProfile ) ;
    if ( Orig.m_VProfile.m_iProfLen )
      m_VProfile.Realloc( Orig.m_VProfile ) ;

    return *this;
  }
  void Add(CSegment& Segm)
  { 
    m_CoordSum += Segm.GetSumCoord() ;
    m_OuterFrame.left = min(Segm.m_Segm.m_iB,m_OuterFrame.left) ;
    m_OuterFrame.top = min(Segm.m_iY,m_OuterFrame.top) ;
    m_OuterFrame.right = max(Segm.m_Segm.m_iE,m_OuterFrame.right) ;
    m_OuterFrame.bottom = max(Segm.m_iY,m_OuterFrame.bottom) ;
    m_Area += Segm.m_Segm.GetLen() ;
    m_iBrSum += Segm.m_iBrSum;
    m_iSumBrMulX += Segm.m_iSumBrMulX;
    m_iSumBrMulY += Segm.m_iSumBrMulY;
    if (Segm.m_iMaxPixel > m_iMaxPixel)
      m_iMaxPixel = Segm.m_iMaxPixel ;
    if ( m_bDetailed )
      m_Runs.Add( Runs( Segm ) ) ; 

    m_ImgMoments.dM00 = 0;
    m_ImgMoments.dM01 = 0;
    m_ImgMoments.dM10 = 0;
    m_ImgMoments.dM11 = 0;
    m_ImgMoments.dM02 = 0;
    m_ImgMoments.dM20 = 0;

    ASSERT(Segm.m_iColor == m_iColor) ;
    Segm.m_iMatched++ ;
    if (Segm.m_iCont < 0)
      Segm.m_iCont = m_iContNum ;
  } ;
  void operator += (CSegment& Segm)
  {
    m_CoordSum += Segm.GetSumCoord() ;
    m_OuterFrame.left = min(Segm.m_Segm.m_iB,m_OuterFrame.left) ;
    m_OuterFrame.top = min(Segm.m_iY,m_OuterFrame.top) ;
    m_OuterFrame.right = max(Segm.m_Segm.m_iE,m_OuterFrame.right) ;
    m_OuterFrame.bottom = max(Segm.m_iY,m_OuterFrame.bottom) ;
    if (Segm.m_iMaxPixel > m_iMaxPixel)
      m_iMaxPixel = Segm.m_iMaxPixel ;
    m_Area += Segm.m_Segm.GetLen() ;
    m_iBrSum += Segm.m_iBrSum;
    m_iSumBrMulX += Segm.m_iSumBrMulX;
    m_iSumBrMulY += Segm.m_iSumBrMulY;
    if ( m_bDetailed )
      m_Runs.Add( Runs( Segm ) ) ; 

    m_ImgMoments.dM00 = 0;
    m_ImgMoments.dM01 = 0;
    m_ImgMoments.dM10 = 0;
    m_ImgMoments.dM11 = 0;
    m_ImgMoments.dM02 = 0;
    m_ImgMoments.dM20 = 0;

    ASSERT(Segm.m_iColor == m_iColor) ;
    Segm.m_iMatched++ ;
    if (Segm.m_iCont < 0)
      Segm.m_iCont = m_iContNum ;
  }
  void Add(CColorSpot& Other)
  {
    if (m_iContNum != Other.m_iContNum)
    {
      m_CoordSum += Other.m_CoordSum ;
      m_OuterFrame.left = min(Other.m_OuterFrame.left,m_OuterFrame.left) ;
      m_OuterFrame.top = min(Other.m_OuterFrame.top,m_OuterFrame.top) ;
      m_OuterFrame.right = max(Other.m_OuterFrame.right,m_OuterFrame.right) ;
      m_OuterFrame.bottom = max(Other.m_OuterFrame.bottom,m_OuterFrame.bottom) ;
      m_Area += Other.m_Area ;
      m_iMaxPixel = max(Other.m_iMaxPixel,m_iMaxPixel);
      m_iBrSum += Other.m_iBrSum;
      m_iSumBrMulX += Other.m_iSumBrMulX;
      m_iSumBrMulX = Other.m_iSumBrMulX;

      m_ImgMoments.dM00 = 0;
      m_ImgMoments.dM01 = 0;
      m_ImgMoments.dM10 = 0;
      m_ImgMoments.dM11 = 0;
      m_ImgMoments.dM02 = 0;
      m_ImgMoments.dM20 = 0;

      if ( m_bDetailed )
        m_Runs.Append( Other.m_Runs ) ;

      ASSERT(Other.m_Area > 0) ;
      Other.m_Area *= -1 ;
      Other.m_iAddedTo = m_iContNum ;
    }
  } ;
  void operator += (CColorSpot& Other)
  {
    if (m_iContNum != Other.m_iContNum)
    {
      m_CoordSum += Other.m_CoordSum ;
      m_OuterFrame.left = min(Other.m_OuterFrame.left,m_OuterFrame.left) ;
      m_OuterFrame.top = min(Other.m_OuterFrame.top,m_OuterFrame.top) ;
      m_OuterFrame.right = max(Other.m_OuterFrame.right,m_OuterFrame.right) ;
      m_OuterFrame.bottom = max(Other.m_OuterFrame.bottom,m_OuterFrame.bottom) ;
      m_iMaxPixel = max(Other.m_iMaxPixel,m_iMaxPixel);
      m_Area += Other.m_Area ;
      m_iBrSum += Other.m_iBrSum;
      m_iSumBrMulX += Other.m_iSumBrMulX;
      m_iSumBrMulY += Other.m_iSumBrMulY;
     
      m_ImgMoments.dM00 = 0;
      m_ImgMoments.dM01 = 0;
      m_ImgMoments.dM10 = 0;
      m_ImgMoments.dM11 = 0;
      m_ImgMoments.dM02 = 0;
      m_ImgMoments.dM20 = 0;
      
      if ( m_bDetailed )
        m_Runs.Append( Other.m_Runs ) ;

      ASSERT(Other.m_Area > 0) ;
      Other.m_Area *= -1 ;
      Other.m_iAddedTo = m_iContNum ;
    }
  } ;
  void EndSpot()
  {
    m_CenterByIntense.x = (double)m_iSumBrMulX  / (double)m_iBrSum ;
    m_CenterByIntense.y = (double)m_iSumBrMulY  / (double)m_iBrSum ;
    m_SimpleCenter.x = (double)m_CoordSum.cx / abs(m_Area) ;
    m_SimpleCenter.y = (double)m_CoordSum.cy / abs(m_Area) ;
  }

};

class SpotArray: public CArray<CColorSpot,CColorSpot&> 
{
#if (_MSC_VER<1300)
public:
  int GetCount() { return GetUpperBound( )+1; }
#endif
};

typedef SpotArray *pSpotArray;

class CSegmentation
{
public:
  CSegmentation() 
  {
    m_pCurr = m_pPrev = NULL ;
    m_MinColor = 0 ;
    m_MaxColor = 127 ;
    m_iWidth = m_iHeight = 0 ;
    m_pFrame = NULL ;
    m_pBitBuf = NULL ;
    m_bDetailed = false ;
    m_iNMaxContours = 5000 ;
  }
  ~CSegmentation() {} ;

  SpotArray   m_ColSpots ;
  SegmentArray    m_Segm1 , m_Segm2 ;
  SegmentArray *  m_pCurr , * m_pPrev ;
  BYTE m_MinColor;
  BYTE m_MaxColor;
  WORD m_wMinColor ;
  WORD m_wMaxColor ;
  int  m_iNMaxContours ;

  int         m_iWidth ;
  int         m_iHeight ;

  LPBYTE      m_pBitBuf ;
  LPWORD      m_pWordBuf ;
  pTVFrame    m_pFrame ;
  bool        m_bDetailed ;

  void		SetClusterColors(int min, int max) 
  { 
    m_MinColor = (BYTE) (m_wMinColor = (WORD) min); 
    m_MaxColor = (BYTE) (m_wMaxColor = (WORD) max); 
  };
  void		GetClusterColors(WORD* min, WORD* max) 
  { if (min) *min = m_wMinColor; if (max) *max = m_wMaxColor; };
  int         FindSpots( pTVFrame pFrame , BOOL bDetailed = FALSE ) ;
  int         SimpleThres( int iY ) ;
  pSpotArray  GetSpotsInfo() { return &m_ColSpots ; } ;
  void        SetNMax( int iNMaxCont ) { m_iNMaxContours = iNMaxCont ; } ;
  int         GetNMax( void ) { return m_iNMaxContours ; } ;
};

__forceinline void Spots_Exchange(pSpotArray Spots, int pos1, int pos2)
{
  CColorSpot tmpP( Spots->ElementAt(pos1) ) ;
  Spots->SetAt( pos1 , Spots->ElementAt(pos2) ) ;
  Spots->SetAt( pos2 , tmpP ) ;
}

__forceinline void Spots_SortX(pSpotArray Spots)
{
  bool sorted=true;
  CColorSpot * clusters = Spots->GetData() ;
  do
  {
    sorted=true;
    for (int i=0; i<Spots->GetCount() - 1; i++)
    {
      if ((clusters[i].m_SimpleCenter.x)>(clusters[i+1].m_SimpleCenter.x))
      {
        Spots_Exchange(Spots, i, i+1);
        sorted=false;
      }
    }
  }while (!sorted);
}

__forceinline void Spots_SortArea(pSpotArray Spots)
{
  bool sorted=true;
  CColorSpot * clusters = Spots->GetData() ;
  do
  {
    sorted=true;
    for (int i=0 ; i<Spots->GetCount() - 1 ; i++)
    {
      if ((clusters[i].m_Area)>(clusters[i+1].m_Area))
      {
        Spots_Exchange(Spots, i, i+1);
        sorted=false;
      }
    }
  }while (!sorted);
}

__forceinline int Spots_GetOverlappedX(CColorSpot * a, CColorSpot * b)
{
  int la=a->m_OuterFrame.left , ra=a->m_OuterFrame.right;
  int lb=b->m_OuterFrame.left , rb=b->m_OuterFrame.right;
  if (la < lb) la=lb;
  if (ra > rb) ra=rb;
  if (ra<=la) return 0;
  return (ra-la);
}

__forceinline int Spots_GetOverlappedY(CColorSpot * a, CColorSpot * b)
{
  int la=a->m_OuterFrame.top , ra=a->m_OuterFrame.bottom;
  int lb=b->m_OuterFrame.top , rb=b->m_OuterFrame.bottom;
  if (la < lb) la=lb;
  if (ra > rb) ra=rb;
  if (ra<=la) return 0;
  return (ra-la);
}

__forceinline int Spots_GetXYPos(CColorSpot& a, int width)
{
  int y=(2*a.m_OuterFrame.top)/(3*a.m_OuterFrame.Height());
  return (a.m_OuterFrame.left+y*width);
}

__forceinline void Spots_SortLinepos(pSpotArray Spots, int fwidth)
{
  bool sorted=true;
  do
  {
    sorted=true;
    for (int i=0; i<Spots->GetCount()-1; i++)
    {
      if (Spots_GetXYPos((*Spots)[i],fwidth)>Spots_GetXYPos((*Spots)[i+1],fwidth))
      {
        Spots_Exchange(Spots, i, i+1);
        sorted=false;
      }
    }
  }while (!sorted);

  /*    int clstrnmb=pCI->m_ClustersNmb;
  pSpotArray Spots=pCI->m_Clusters;
  Spots_SortX(clusters, clstrnmb);
  for (int i=0; i<clstrnmb-1; i++)
  {
  Spots_GetOverlappedX(&clusters[i],&clusters[i+1]);
  } */
}


__forceinline void Spots_RemoveAt(pSpotArray Spots, int pos)
{
  Spots->RemoveAt( pos ) ;
}

__forceinline void Spots_LeaveLargest(pSpotArray Spots, int& length)
{
  if ( Spots->GetCount() == 0 )
    return ;
  CColorSpot Biggest = (*Spots)[0] ;
  for ( int i = 1 ; i < Spots->GetCount() ; i++ )
  {
    if ( Biggest.m_Area < (*Spots)[i].m_Area )
      Biggest = (*Spots)[i] ;
  }
  Spots->RemoveAll() ;
  Spots->Add( Biggest ) ;
}

__forceinline void Spots_RemoveDeleted(pSpotArray Spots)
{
  for(int i=0; i<Spots->GetCount(); i++)
  {
    if ((*Spots)[i].m_iAddedTo >= 0)
    {
      Spots_RemoveAt(Spots,i);
      i--;
    }
  }
}

__forceinline void Spots_RemoveAll(pSpotArray Spots)
{
  Spots->RemoveAll() ;
}

__forceinline void Spots_MinMaxAreaFilter(
  pSpotArray Spots, int min, int max, bool remove=true)
{
  CColorSpot * pData = Spots->GetData() ;
  for (int i=0; i<Spots->GetCount(); i++)
  {
    if (max>0) 
    {
      if (pData[i].m_Area > max) pData[i].m_iAddedTo = 100000000 ;
    }
    if (min>0) 
    {
      if (pData[i].m_Area < min) pData[i].m_iAddedTo = 100000000 ;
    }
  }
  if (remove) { Spots_RemoveDeleted(Spots); }
}

__forceinline void Spots_XYRatioFilter(
                                       pSpotArray Spots, double min, double max, bool remove=true)
{
  CColorSpot * clusters = Spots->GetData() ;
  for (int i=0; i<Spots->GetCount(); i++)
  {
    ASSERT(clusters[i].m_OuterFrame.Height()!=0);
    double ratio=(double)clusters[i].m_OuterFrame.Width()/(double) clusters[i].m_OuterFrame.Height();
    if (max>0) 
    {
      if (ratio>max) { clusters[i].m_iAddedTo = 100000000 ; continue; }
    }
    if (min>0) 
    {
      if (ratio<min) { clusters[i].m_iAddedTo = 100000000 ;  }
    }
  }
  if (remove) { Spots_RemoveDeleted(Spots); }
}

__forceinline void Spots_HeightFilter(pSpotArray Spots, int min, int max, bool remove=true)
{
  CColorSpot * clusters = Spots->GetData() ;
  for (int i=0; i<Spots->GetCount(); i++)
  {
    int iHeight = clusters[i].m_OuterFrame.Height() ;
    if (max>0) 
    {
      if ( iHeight > max) 
      { clusters[i].m_iAddedTo = 100000000 ; continue; }
    }
    if (min>0) 
    {
      if (iHeight<min) { clusters[i].m_iAddedTo = 100000000 ;  }
    }
  }
  if (remove) { Spots_RemoveDeleted(Spots); }
}

__forceinline void Spots_WidthFilter(pSpotArray Spots, int min, int max, bool remove=true)
{
  CColorSpot * clusters = Spots->GetData() ;
  for (int i=0; i<Spots->GetCount(); i++)
  {
    int iWidth = clusters[i].m_OuterFrame.Width() ;
    if (max>0) 
    {
      if ( iWidth > max) 
      { clusters[i].m_iAddedTo = 100000000 ; continue; }
    }
    if (min>0) 
    {
      if (iWidth<min) { clusters[i].m_iAddedTo = 100000000 ;  }
    }
  }
  if (remove) { Spots_RemoveDeleted(Spots); }
}





#endif // !defined(AFX_CSPOT_H__132A0B73_DC7B_482F_B004_226F86AB2F2A__INCLUDED_)



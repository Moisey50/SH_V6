// TColorMatch.h : Declaration of the TColorMatch

#pragma once
#include <gadgets\gadbase.h>
#include <classes\drect.h>
#include <gadgets\videoframe.h>

#define SQUARE_SIZE   512
#define YCOLUMN_WIDTH  32




enum TWorkingMode { Teaching , AutoTeaching , RecognitionText , RecognitionVideo , Pass } ;

inline bool PtInside( CRect& rect , CPoint& Pt )
{
  return ( Pt.x >= rect.left 
    && Pt.x <= rect.right 
    && Pt.y >= rect.top 
    && Pt.y <= rect.bottom ) ;
}

inline void UnionRectAndPt( CRect& rect , CPoint& Pt )
{
  if ( Pt.x < rect.left )
    rect.left = Pt.x ;
  else if ( Pt.x > rect.right )
    rect.right = Pt.x ;
  if ( Pt.y < rect.top )
    rect.top = Pt.y ;
  else if ( Pt.y > rect.bottom ) 
    rect.bottom = Pt.y ;
}


class TColorMatch : public CFilterGadget
{
public:
  TColorMatch(void);
  void ShutDown();
  //
  CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
  bool ScanProperties(LPCTSTR text, bool& Invalidate);
  bool PrintProperties(FXString& text);
  bool ScanSettings(FXString& text);
  void FormUVPresentation( CContainerFrame * pOut , CDPoint * Pt = NULL , int * iY = NULL ) ;

  FXString        m_FileName ;
  TWorkingMode    m_WorkingMode ;
  BOOL            m_bAutoTeach ;
  BOOL            m_bGetNextForTeaching ;
  CDRect          m_AllowedColorRange ;
  CRect           m_UVRange ;
  CPoint          m_GoodMark ;
  CPoint          m_BadMark ;
  double          m_dYMin ;
  double          m_dYMax ;
  int             m_iYForPresentation ;
  int             m_iMinRecognitionIntens ;
  int             m_iMinInHistValue ;
  bool            m_bYChanged ;
  CVideoFrame   * m_pOutputFrame ;
  CVideoFrame   * m_pMaskFrame ;

  FXString        m_LastData ;
  int             m_UVHist[65536] ;

  inline void AddUV_Value( CDPoint& UV )
  {
    if ( m_WorkingMode == Teaching )
    {
      if ( m_AllowedColorRange.left == 1e30 )
      {
        m_AllowedColorRange.left = m_AllowedColorRange.right = UV.x ;
        m_AllowedColorRange.top = m_AllowedColorRange.bottom = UV.y ;
      }
      else
      {
        m_AllowedColorRange.Union( UV ) ;
      }
    }
  }
  inline bool IsGoodColor( int UPlus128 , int VPlus128 )
  {
    int iIndex =  UPlus128 + VPlus128 * 256 ;
    ASSERT( iIndex >= 0 && iIndex < sizeof(m_UVHist)/sizeof(m_UVHist[0])) ;
    int iVal = m_UVHist[ iIndex ] ;
    return ( iVal >= m_iMinInHistValue ) ;
  }
  inline void PutToHist( int UPlus128 , int VPlus128 )
  {
    int iIndex =  UPlus128 + VPlus128 * 256 ;
    ASSERT( iIndex >= 0 && iIndex < sizeof(m_UVHist)/sizeof(m_UVHist[0])) ;
    m_UVHist[ iIndex ]++ ;
  }

  DECLARE_RUNTIME_GADGET(TColorMatch);
};


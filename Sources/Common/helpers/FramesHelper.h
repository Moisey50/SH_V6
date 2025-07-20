#ifndef FRAMES_HELPER_H__
#define FRAMES_HELPER_H__

#include <gadgets\gadbase.h>
#include <gadgets\videoframe.h>
#include <gadgets\quantityframe.h>
#include <gadgets\containerframe.h>
#include <gadgets\FigureFrame.h>
#include "helpers/propertykitEx.h"
#include <imageproc/ImageProfile.h>


#define FR_RELEASE_DEL(x) \
  if(x)\
  {\
    ((CDataFrame*)(x))->Release();\
    x=NULL ;\
  }

typedef enum
{
  WP_Full = 0 ,
  WP_Begin ,
  WP_Any ,
  WP_CaseSensitive = 0x80000000 ,
} WordPart ;

enum CircleExtractMode
{
  CEM_FirstEdge = 0x00000001 ,
  CEM_LastEdge = 0x00000002 ,
  CEM_Extrem = 0x00000004 ,
  CEM_ExtremWithNeightbours = 0x00000008 ,
  CEM_MiddleBetweenEdges = 0x00000010 ,
  CEM_FirstDiff = 0x00000020
};

CFigureFrame * CreateFigureFrame( CRect& Rect ,
  DWORD dwColor = 0xc0ffc0 , const char * pLabel = NULL ,
  DWORD dwId = 0 , int iThickness = 0 );// 0 means "do default"

CFigureFrame * CreateGraphic( double * Pts , int iLen ,
  cmplx Origin , cmplx Step , cmplx Range , double dMin , double dMax ,
  const char * pColor = _T( "c0ffc0" ) , double dK = 1. , double dMaxAmpl = 0. ) ;

CFigureFrame * CreateCircleView( cmplx& cCenter ,
  double dRadius , DWORD dwCircColor ) ;

void CreateFullFrameCross( cmplx& Pt , CRect& ROI ,
  CContainerFrame * POut , COLORREF Color = 0x010101 ) ;

const CDataFrame * GetFrameWithLabel( const CDataFrame * pSrc ,
  datatype type , LPCTSTR pLabelPart , DWORD dwMask ) ;

bool PutTextsSeparated(
  COutputConnector * pPin , const FXString& TextData ,
  LPCTSTR pSeparator , LPCTSTR pLabel = NULL ,
  DWORD dwDelay_ms = 0 , DWORD * piFirstID = NULL ,
  const FXString& cmdWithDelayAfter = "" , DWORD dwDelay4CmdWithDelayAfter_ms = 0 ) ;

int FormFrameTextView(
  const CDataFrame * pDataFrame , FXString& outp ,
  FXString& Prefix , int& iNFrames , FXPtrArray& FramePtrs );

bool CheckContainer( const CContainerFrame * pContainer ,
  int& iFirst , int& iSecond ) ;

bool IsInContainer( const CContainerFrame * pContainer ,
  const CDataFrame * pFrame ) ;

int FormQuestionWithRectOnRender( cmplx& ViewPt ,
  LPCTSTR pQuestion , DWORD iFontSize , DWORD dwColor ,
  vector<CRect>& Zones ,
  CContainerFrame * pMarking ) ;

int FormQuestionsOnRender( cmplx& ViewPt ,
  LPCTSTR pInfo , LPCTSTR * pQuestions ,
  DWORD iFontSize , DWORD dwColor , vector<CRect>& Zones ,
  CContainerFrame * pMarking ) ;


double GetCrossPosOnStripWithView(
  cmplx& cBegin , // strip corner
  cmplx& cDirAlongStep , // strip direction
  cmplx& cDirCrossStep ,  // for viewing only
  int    iNAlongSteps ,
  int    iNCrossSteps , // Every cross step will be 90 degrees to the left
                        // with the same length
  double dThres ,
  double * pSignal , // average value of cross direction will be saved
                      // in this array for every step in along direction
  int iMaxSignalLen ,
  const pTVFrame pTV ,
  CContainerFrame * pDiagOut ) ;

size_t GetContrastLine( cmplx InitialPt , // some point on line center
  cmplx cInitalLineDirection , // line direction in initialPt (brightness in initial point is contrast
                               // relatively to points on range ends
  double dStripRange , // How many pixels look to the sides
  CmplxVector& PtsOnLine , // Result points on line center ordered from one end to enother
  const CVideoFrame * pVF ) ; // video frame for processing

bool ExtractCirclesByCenterAndRadius(
  const CVideoFrame * pVF , cmplx& cOrigCenter_FOV ,
  double dInitialRadius_pix , double dFinalRadius_pix , // if Initial - Final is negative - scan to center
  double dThres_rel ,
  CircleExtractMode ExtractionMode ,
  DiffThresMode DiffDirection ,
  CmplxVector& ResultPts ,
  CContainerFrame * pMarking = NULL ,
  int iNSegments = 72 ,          // every 5 degrees
  double dDeviation_pix = 0.5 ) ; // deviation of circle from straight line

size_t GetVerticalContrastLine( cmplx cInitialPt , int iStripRange ,
  CmplxVector& PtsOnLine , double dNormThres , int iMinAmpl ,
  const CVideoFrame * pVF , double& dLineWidth ,
  DoubleVector * pdAvoidPointsY = NULL ) ;

size_t GetPixelsOnVerticalLine8( CPoint ptBegin , int iStep , 
  LPBYTE pResult , size_t ResultLen , const CVideoFrame * pVF , int& iMin , int& iMax ) ;

size_t GetHorizontalContrastLine( cmplx cInitialPt , int iStripRange ,
  CmplxVector& PtsOnLine , double dNormThres , int iMinAmpl ,
  const CVideoFrame * pVF , double& dLineWidth ,
  DoubleVector * pdAvoidPointsX = NULL ) ;

CFigureFrame * CreateHGraphForDraw( Profile& HorProfile , CRect HorProfileROI ) ;
CFigureFrame * CreateVGraphForDraw( Profile& VertProfile , CRect VertProfileROI ) ;

// Inline functions

inline bool ReleaseAndZeroPtr( const CDataFrame *& pDataFrame )
{
  if ( pDataFrame )
  {
    bool bRes = ((CDataFrame*)pDataFrame)->Release() ;
    pDataFrame = NULL ;
    return bRes ;
  }
  return false ;
}

inline bool GetThresCrossOnDirection(
  pTVFrame ptv , CPoint Pt , CSize Step , 
  int iThres , CPoint& CrossPt )
{
  if ( !ptv )
    return false;
  int width = ptv->lpBMIH->biWidth , height = ptv->lpBMIH->biHeight;

  CRect ROI( 0 , 0 , width - 1 , height - 1 ) ;

  int iVal = GetPixel( ptv , Pt ) ;
  BOOL bInvert = (iVal < iThres) ;
  Pt += Step ;
  while ( ROI.PtInRect( Pt) )
  {
    if ( !WhitePixel(ptv , Pt , iThres , bInvert ) )
    {
      CrossPt = Pt ;
      return true ;
    }
    Pt += Step ;
  }
  return false ;
}

inline double GetDeltaArea( cmplx NewPt , cmplx PrevPt )
{
  return 0.5 * 
    (NewPt.imag() - PrevPt.imag()) * (NewPt.real() + PrevPt.real()) ;
}

inline double GetFigureArea( const CFigure * pFigure ) // correctly works for closed contour only
{
  double dArea = 0. ;
  cmplx * pData = (cmplx*) pFigure->GetData() ;
  cmplx * pIter = pData + 1 ;
  cmplx * pEnd = pData + pFigure->GetCount() ;
  do 
  {
    dArea += GetDeltaArea( *(pIter - 1) , *pIter ) ;
  } while ( ++pIter < pEnd );
  dArea += GetDeltaArea( *(pIter - 1) , *pData ) ;
  return fabs( dArea ) ;
}


inline int GetAngleIndex( double dAngle_rad , double dAngStep )
{
  double dAngPlus = fabs(dAngle_rad) ;
  int iIndex = ROUND( RadToDeg(dAngStep) )
    * ROUND((dAngPlus + (dAngStep * 0.49999999999))/dAngStep ) ;
  return ((dAngle_rad >= 0) ? iIndex : 360 - iIndex) % 360 ;
}

inline CTextFrame * CreateTextFrame( cmplx& Pt , LPCTSTR Text ,
  const char * pColor = "c0ffc0" , int iSize = 16 ,
  const char * pLabel = NULL , DWORD dwId = 0 )
{
  CTextFrame * pText = CTextFrame::Create() ;
  if ( pText )
  {
    pText->Attributes()->WriteInt( "x" , ROUND(Pt.real()) ) ;
    pText->Attributes()->WriteInt( "y" , ROUND(Pt.imag()) ) ;
    pText->Attributes()->WriteInt("Sz" , iSize ) ;
    pText->Attributes()->WriteString( "color" , pColor ) ;
    pText->GetString() = Text ;
    if ( pLabel )
      pText->SetLabel( pLabel ) ;

    pText->ChangeId( dwId ) ;
    pText->SetTime( GetHRTickCount() ) ;
  }
  return pText ;
}

inline CTextFrame * CreateTextFrame( cmplx& Pt , LPCTSTR Text ,
  DWORD Color = 0xc0ffc0 , int iSize = 16 ,
  const char * pLabel = NULL , DWORD dwId = 0 )
{
  CTextFrame * pText = CTextFrame::Create() ;
  if ( pText )
  {
    pText->Attributes()->WriteInt( "x" , ROUND( Pt.real() ) ) ;
    pText->Attributes()->WriteInt( "y" , ROUND( Pt.imag() ) ) ;
    pText->Attributes()->WriteInt( "Sz" , iSize ) ;
    TCHAR Buf[ 20 ] ;
    sprintf_s( Buf , "0x%06x" , Color ) ;
    pText->Attributes()->WriteString( "color" , Buf ) ;
    pText->GetString() = Text ;
    if ( pLabel )
      pText->SetLabel( pLabel ) ;

    pText->ChangeId( dwId ) ;
    pText->SetTime( GetHRTickCount() ) ;
  }
  return pText ;
}

inline CTextFrame * CreateTextFrame( cmplx& Pt , LPCTSTR pColor , int iSize ,
  const char * pLabel , DWORD dwId , LPCTSTR pContentFormat , ... )
{
  FXString Content ;
  va_list argList;
  va_start( argList , pContentFormat );
  Content.FormatV( pContentFormat , argList );

  CTextFrame * pText = CTextFrame::Create( Content ) ;
  if ( pText )
  {
    pText->Attributes()->WriteInt( "x" , ROUND( Pt.real() ) ) ;
    pText->Attributes()->WriteInt( "y" , ROUND( Pt.imag() ) ) ;
    pText->Attributes()->WriteInt( "Sz" , iSize ) ;
    pText->Attributes()->WriteString( "color" , pColor ) ;
    if ( pLabel )
      pText->SetLabel( pLabel );
    pText->ChangeId( dwId ) ;
    pText->SetTime( GetHRTickCount() ) ;
  }
  return pText ;
}

inline CTextFrame * CreateTextFrame( 
  const char * pLabel = NULL , DWORD dwId = 0 )
{
  CTextFrame * pText = CTextFrame::Create() ;
  if ( pText )
  {
    if ( pLabel )
      pText->SetLabel( pLabel );
    pText->ChangeId( dwId ) ;
    pText->SetTime( GetHRTickCount() ) ;
  }
  return pText ;
}

inline CTextFrame * CreateTextFrame(
  const char * pContentText ,
  const char * pLabel = NULL , DWORD dwId = 0 )
{
  CTextFrame * pText = CTextFrame::Create( pContentText ) ;
  if ( pText )
  {
    if ( pLabel )
      pText->SetLabel( pLabel );
    pText->ChangeId( dwId ) ;
    pText->SetTime( GetHRTickCount() ) ;
  }
  return pText ;
}

inline CQuantityFrame * CreateQuantityFrame( int iNumber ,
  const char * pLabel = NULL , DWORD dwId = 0 )
{
  CQuantityFrame * pQ = CQuantityFrame::Create( iNumber ) ;
  if ( pQ )
  {
    if ( pLabel )
      pQ->SetLabel( pLabel );
    pQ->ChangeId( dwId ) ;
    pQ->SetTime( GetHRTickCount() ) ;
  }
  return pQ ;
}

inline CQuantityFrame * CreateQuantityFrame( double dNumber ,
  const char * pLabel = NULL , DWORD dwId = 0 )
{
  CQuantityFrame * pQ = CQuantityFrame::Create( dNumber ) ;
  if ( pQ )
  {
    if ( pLabel )
      pQ->SetLabel( pLabel );
    pQ->ChangeId( dwId ) ;
    pQ->SetTime( GetHRTickCount() ) ;
  }
  return pQ ;
}

inline CFigureFrame * CreatePtFrame( const cmplx& Pt ,
  double dTime = GetHRTickCount() , const char * pColor = "c0ffc0" ,
  const char * pLabel = NULL , DWORD dwId = 0 )
{
  CFigureFrame * pPt = CFigureFrame::Create() ;
  if ( pPt )
  {
    pPt->Add( CDPoint( Pt.real() , Pt.imag() ) ) ;
    pPt->SetTime( dTime ) ;
    if ( pColor )
      pPt->Attributes()->WriteString( "color" , pColor ) ;
    if ( pLabel )
      pPt->SetLabel( pLabel ) ;
    pPt->ChangeId( dwId ) ;
  }
  return pPt ;
}

inline CFigureFrame * CreatePtFrame( const cmplx& Pt ,LPCTSTR Attributes ,
  const char * pLabel = NULL , DWORD dwId = 0 )
{
  CFigureFrame * pPt = CFigureFrame::Create() ;
  if ( pPt )
  {
    pPt->Add( CDPoint( Pt.real() , Pt.imag() ) ) ;
    pPt->SetTime( GetHRTickCount() ) ;
    *(pPt->Attributes()) = Attributes ;
    if ( pLabel )
      pPt->SetLabel( pLabel ) ;
    pPt->ChangeId( dwId ) ;
  }
  return pPt ;
}

inline CFigureFrame * CreatePtFrame( const cmplx& Pt ,
  double dTime = GetHRTickCount() , COLORREF Color = 0x010101 ,
  const char * pLabel = NULL , DWORD dwId = 0 )
{
  CFigureFrame * pPt = CFigureFrame::Create() ;
  if ( pPt )
  {
    pPt->Add( CDPoint( Pt.real() , Pt.imag() ) ) ;
    pPt->SetTime( dTime ) ;
    if ( Color != 0x010101 )
      ( ( FXPropKit2* )( pPt->Attributes() ) )
      ->WriteUIntNotDecimal( "color" , ( UINT )Color ) ;
    if ( pLabel )
      pPt->SetLabel( pLabel ) ;
    pPt->ChangeId( dwId ) ;
  }
  return pPt ;
}

inline CFigureFrame * CreateLineFrame( const cmplx& Pt1 , const cmplx& Pt2 ,
  const char * pColor = "c0ffc0" , const char * pLabel = NULL , 
  DWORD dwId = 0 , double dTime = GetHRTickCount() )
{
  CFigureFrame * pLine = CFigureFrame::Create() ;
  if ( pLine )
  {
    pLine->Add( CDPoint( Pt1.real() , Pt1.imag() ) ) ;
    pLine->Add( CDPoint( Pt2.real() , Pt2.imag() ) ) ;
    if ( pColor )
      pLine->Attributes()->WriteString( "color" , pColor ) ;
    if ( pLabel )
      pLine->SetLabel( pLabel ) ;
    pLine->SetTime( dTime ) ;
    pLine->ChangeId( dwId ) ;
  }
  return pLine ;
}

inline CFigureFrame * CreateLineFrame( const cmplx& Pt1 , const cmplx& Pt2 ,
  COLORREF Color = 0x010101, const char * pLabel = NULL,
  DWORD dwId = 0, double dTime = GetHRTickCount())
{
  CFigureFrame * pLine = CFigureFrame::Create() ;
  if ( pLine )
  {
    pLine->Add( CDPoint( Pt1.real() , Pt1.imag() ) ) ;
    pLine->Add( CDPoint( Pt2.real() , Pt2.imag() ) ) ;
    if ( Color != 0x010101 )
    {
      ( ( FXPropKit2* )( pLine->Attributes() ) )
        ->WriteUIntNotDecimal( "color" , ( UINT )Color ) ;
    }
    if ( pLabel )
      pLine->SetLabel( pLabel ) ;
    pLine->SetTime( dTime ) ;
    pLine->ChangeId( dwId ) ;
  }
  return pLine ;
}

CFigureFrame * CreateFigureFrame( cmplx * Pts , int iLen ,
  double dTime = GetHRTickCount() , const char * pColor = "c0ffc0" ,
  const char * pLabel = NULL , DWORD dwId = 0 ) ;

CFigureFrame * CreateFigureFrame( cmplx * Pts , int iLen ,
  DWORD dwColor = 0x00c0ffc0 ,
  const char * pLabel = NULL , DWORD dwId = 0 ) ;


inline CFigureFrame * CreateFigureFrameCR( cmplx& cCent , int iRadius ,
  const char * pColor = "c0ffc0" , const char * pLabel = NULL ,
  DWORD dwId = 0 , double dTime = GetHRTickCount() )
{
  CFigureFrame * pRect = CFigureFrame::Create() ;
  if ( pRect )
  {
    CDPoint CDPt1( cCent.real() - iRadius , cCent.imag() - iRadius ) ;
    pRect->AddPoint(CDPt1) ;
    CDPoint CDPt2( cCent.real() + iRadius , cCent.imag() - iRadius ) ;
    pRect->AddPoint( CDPt2 ) ;
    CDPoint CDPt3( cCent.real() + iRadius , cCent.imag() + iRadius ) ;
    pRect->AddPoint( CDPt3 ) ;
    CDPoint CDPt4( cCent.real() - iRadius , cCent.imag() + iRadius ) ;
    pRect->AddPoint( CDPt4 ) ;
    pRect->AddPoint( CDPt1 ) ;
    if ( pColor )
      pRect->Attributes()->WriteString( "color" , pColor ) ;
    if ( pLabel )
      pRect->SetLabel( pLabel ) ;
    pRect->SetTime( dTime ) ;
    pRect->ChangeId( dwId ) ;
  }
  return pRect ;
}

inline CFigureFrame * CreateFigureFrameCR( CPoint& Cent , int iRadius ,
  const char * pColor = "c0ffc0" , const char * pLabel = NULL ,
  DWORD dwId = 0 , double dTime = GetHRTickCount() )
{
  cmplx cCent( Cent.x , Cent.y ) ;

  return CreateFigureFrameCR( cCent , iRadius , pColor , pLabel ,
    dwId , dTime ) ;
}

inline CRectFrame * CreateRectFrame( CRect& Rect ,
  const char * pColor = "c0ffc0" , const char * pLabel = NULL ,
  DWORD dwId = 0 , double dTime = GetHRTickCount() )
{
  CRectFrame * pRect = CRectFrame::Create( &Rect ) ;
  if ( pRect )
  {
    if ( pColor )
      pRect->Attributes()->WriteString( "color" , pColor ) ;
    if ( pLabel )
      pRect->SetLabel( pLabel ) ;
    pRect->SetTime( dTime ) ;
    pRect->ChangeId( dwId ) ;
  }
  return pRect ;
}

inline CRectFrame * CreateRectFrame( CRect& Rect ,
  DWORD dwColor = 0xc0ffc0 , const char * pLabel = NULL ,
  DWORD dwId = 0 , int iThickness = 0 ) // 0 means "do default"
{
  CRectFrame * pRect = CRectFrame::Create( &Rect ) ;
  if ( pRect )
  {
    pRect->Attributes()->WriteLong( "color" , (long)dwColor ) ;
    if ( iThickness )
      pRect->Attributes()->WriteLong( "thickness" , (long) iThickness ) ;
    if ( pLabel )
      pRect->SetLabel( pLabel ) ;
    if ( dwId )
      pRect->ChangeId( dwId ) ;
  }
  return pRect ;
}

inline CContainerFrame * CheckAndCreateContainer(
  CContainerFrame * pContainer , DWORD dwId = 0 , LPCTSTR pLabel = NULL )
{
  if ( !pContainer )
  {
    pContainer = CContainerFrame::Create() ;
    pContainer->ChangeId( dwId ) ;
    pContainer->SetTime( GetHRTickCount() ) ;
    if ( pLabel )
      pContainer->SetLabel( pLabel ) ;
  }
  return pContainer ;
}

inline bool CopyIdAndTime( CDataFrame * pDest , const CDataFrame * pSrc )
{
  if ( pSrc && pDest )
  {
    pDest->ChangeId( pSrc->GetId() ) ;
    pDest->SetTime( pSrc->GetTime() ) ;
    return true ;
  }
  return false ;
}

inline bool Is8BitsImage( const CVideoFrame * pFrame )
{
  int iCompression = GetCompression( pFrame ) ;
  if ( ( iCompression == BI_Y8 )   || ( iCompression == BI_Y800 ) 
    || ( iCompression == BI_YUV9 ) || ( iCompression == BI_YUV12 ) )
  {
    return true ;
  }
  return false ;
}

inline bool Is16BitsImage( const CVideoFrame * pFrame )
{
  int iCompression = GetCompression( pFrame ) ;
  if ( iCompression == BI_Y16 )
  {
    return true ;
  }
  return false ;
}

inline char * GetU( const CVideoFrame * vf )
{
  if ( vf->lpBMIH )
  {
    LPBYTE lpData = (vf->lpData) ? vf->lpData : (LPBYTE)(vf->lpBMIH + 1) ;
    switch ( GetCompression( vf ) )
    {
    case BI_YUV9:
    case BI_YUV12:
      {
        char * pU = (char *)lpData + (vf->lpBMIH->biWidth * vf->lpBMIH->biHeight) ;
        return pU ;
      }
    }
  }
  return NULL ;
}

inline char * GetV( const CVideoFrame * vf )
{
  if ( vf->lpBMIH )
  {
    LPBYTE lpData = (vf->lpData) ? vf->lpData : (LPBYTE)(vf->lpBMIH + 1) ;
    UINT uiImageSize = GetImageSizeWH( vf ) ;
    switch ( GetCompression( vf ) )
    {
    case BI_YUV9:
      {
        char * pV = (char *)lpData + uiImageSize + uiImageSize/16 ;
        return pV ;
      }
    case BI_YUV12:
      {
        char * pV = (char *)lpData + uiImageSize + uiImageSize/4 ;
        return pV ;
      }
    }
  }
  return NULL ;
}

inline bool ClearColor( CVideoFrame * vf )
{
  char * pU = GetU( vf ) ;
  if ( pU )
  {
    UINT uiImageSize = GetImageSizeWH( vf ) ;
    switch( GetCompression( vf) )
    {
    case BI_YUV9:
      memset( pU , 128 , uiImageSize/8 ) ;
      return true ;
    case BI_YUV12:
      memset( pU , 128 , uiImageSize/2 ) ;
      return true ;
    }
  }
  return false ;
}

inline bool PutFrame( COutputConnector * pPin , const CDataFrame * pData , int iTimeout_ms = -1 )
{
  if ( iTimeout_ms >= 0 )
  {
    FXLockObject& Locker = pPin->GetLocker() ;
    if ( Locker.Lock( iTimeout_ms , "PutFrame" ) )
      Locker.Unlock() ;
    else
    {
      TRACE( "\nPutFrame: Can't lock pin ""%s"" " , pPin->GetName() ) ;
      ((CDataFrame*)(pData))->Release() ;
      return false ;
    }
  }
  if ( !pPin || !pPin->Put((CDataFrame*)pData) )
  {
    TRACE( "\nCan't send frame ""%s"" to pin ""%s"" " ,
           pData->GetLabel() , (pPin)? pPin->GetName() : "NoPinName" ) ;
    ( ( CDataFrame* ) ( pData ) )->Release() ;
    return false ;
  }
  return true ;
}


inline bool IsFrameLabelEqual( const CDataFrame * pFrame , LPCTSTR Value )
{
  LPCTSTR pLabel ;
  if ( !Value || !pFrame || !(pLabel = pFrame->GetLabel()) )
    return false ;
  return (_tcscmp( pLabel , Value ) == 0) ;
}

inline void ReplaceFrame( const CDataFrame ** pHolder ,
  const CDataFrame * pNewFrame )
{
  if ( *pHolder == pNewFrame )
    return ;
  if ( *pHolder )
    ((CDataFrame*) (*pHolder))->Release() ;
  if ( pNewFrame )
    ((CDataFrame*) pNewFrame)->AddRef() ;
  *pHolder = pNewFrame ;
}

inline void ReplaceFrame( CDataFrame ** pHolder ,
  CDataFrame * pNewFrame )
{
  if ( *pHolder == pNewFrame )
    return ;
  if ( *pHolder )
    (*pHolder)->Release() ;
  if ( pNewFrame )
    pNewFrame->AddRef() ;
  *pHolder = pNewFrame ;
}

typedef FXArray<const CDataFrame*> FramesArray ;

class FramesCollection
{
public:
  FramesArray m_Frames ;

  FramesCollection() {} ;
  ~FramesCollection()
  {
    RemoveAll() ;
  }
  void RemoveAll()
  {
    for ( int i = 0 ; i < m_Frames.GetCount() ; i++ )
      ((CDataFrame*) m_Frames[ i ])->Release() ;

    m_Frames.RemoveAll() ;
  }
  FXSIZE AddFrame( const CDataFrame * pFrame )
  {
    ((CDataFrame*) pFrame)->AddRef() ;

    m_Frames.Add( pFrame ) ;
    return m_Frames.GetCount() ;
  }
  FXSIZE AddFrame( CDataFrame * pFrame )
  {
    m_Frames.Add( pFrame ) ;
    return m_Frames.GetCount() ;
  }

  CDataFrame * GetFrame( FXSIZE iIndex )
  {
    if ( iIndex >= 0 && iIndex < m_Frames.GetCount() )
      return ((CDataFrame*)m_Frames[ iIndex ]) ;
    return NULL ;
  }

  bool SetFrame( FXSIZE iIndex , const CDataFrame * pFr )
  {
    if ( iIndex >= 0 && iIndex < m_Frames.GetCount() )
    {
      ((CDataFrame*) m_Frames[ iIndex ])->Release() ;
      ((CDataFrame*) pFr)->AddRef() ;
      m_Frames.SetAt( iIndex , pFr ) ;
      return true ;
    }
    return false ;
  }

  CDataFrame * SetGetFrame( int iIndex , const CDataFrame * pFr )
  {
    if ( iIndex >= 0 && iIndex < m_Frames.GetCount() )
    {
      CDataFrame * RetVal = (CDataFrame*) m_Frames[ iIndex ] ;
      m_Frames.SetAt( iIndex , pFr ) ;
      return RetVal ;
    }
    return NULL ;
  }

  FXSIZE RemoveFrame( FXSIZE iIndex , FXSIZE iCnt = 1 )
  {
    if ( iIndex >= 0 && iIndex < m_Frames.GetCount() )
    {
      while ( iCnt-- > 0 )
      {
        ((CDataFrame*) m_Frames[ iIndex ])->Release() ;
        m_Frames.RemoveAt( iIndex++ ) ;
      }
      return m_Frames.GetCount() ;
    }
    return (-1)  ;
  }
  FXSIZE RemoveLast()
  {
    if ( m_Frames.GetCount() )
      RemoveFrame( m_Frames.GetUpperBound() ) ;
    return m_Frames.GetCount() ;
  }
  FXSIZE GetCount() { return m_Frames.Count() ; }
  FXSIZE Count() { return m_Frames.Count() ; }
  const CDataFrame * GetLast()
  {
    if ( Count() )
      return m_Frames[ m_Frames.GetUpperBound() ] ;
    return NULL ;
  }
  const CDataFrame * Last()
  {
    return GetLast() ;
  }
};


class NamedCDRect
{
public:
  string m_ObjectName ;
  DWORD  m_ObjectIndex = 0 ;
  CDRect m_Rect ;

  NamedCDRect( LPCTSTR pName = "" , DWORD dwIndex = 0xffff ,
    double left = 0. , double top = 0. , double right = 0. , double bottom = 0. )
  {
    m_ObjectName = pName ;
    m_ObjectIndex = dwIndex ;
    m_Rect.left = left ;
    m_Rect.top = top ;
    m_Rect.right = right ;
    m_Rect.bottom = bottom ;
  }
  NamedCDRect( LPCTSTR pName = "" , DWORD dwIndex = 0xffff ,
    CDRect rect = CDRect() )
  {
    m_ObjectName = pName ;
    m_ObjectIndex = dwIndex ;
    m_Rect = rect ;
  }
  NamedCDRect( LPCTSTR pName = "" , DWORD dwIndex = 0xffff ,
    CRect * pRect = NULL )
  {
    m_ObjectName = pName ;
    m_ObjectIndex = dwIndex ;
    if ( pRect )
    {
      m_Rect.left = pRect->left ;
      m_Rect.top = pRect->top ;
      m_Rect.right = pRect->right ;
      m_Rect.bottom = pRect->bottom ;
    }
  }
  NamedCDRect( const NamedCDRect& Orig )
  {
    m_ObjectName = Orig.m_ObjectName ;
    m_ObjectIndex = Orig.m_ObjectIndex ;
    m_Rect = Orig.m_Rect ;
  }

  NamedCDRect& operator = ( const NamedCDRect& Orig )
  {
    m_ObjectName = Orig.m_ObjectName ;
    m_ObjectIndex = Orig.m_ObjectIndex ;
    m_Rect = Orig.m_Rect ;
    return *this ;
  }
};
typedef vector<NamedCDRect> NamedCDRects ;



#endif  // FRAMES_HELPER_H__
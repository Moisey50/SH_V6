#ifndef FRAMES_HELPER_H__
#define FRAMES_HELPER_H__

#include <math\intf_sup.h>
#include <gadgets\gadbase.h>
#include <gadgets\videoframe.h>
#include <gadgets\quantityframe.h>
#include <gadgets\containerframe.h>
#include <gadgets\FigureFrame.h>

// Utilities

inline DWORD GetCompression( const pTVFrame fr )
{
  if ( fr->lpBMIH )
    return fr->lpBMIH->biCompression ;
  else
    return 0xffffffff ;
}
inline bool IsPtInFrame( cmplx& Pt , const pTVFrame pFrame )
{
  if ( (Pt.real() <= 0.) || (Pt.real() >= GetWidth(pFrame)) )
    return false ;
  if ( (Pt.imag() <= 0.) || (Pt.imag() >= GetHeight(pFrame)) )
    return false ;
  return true ;
}

inline CPoint RoundPt( CDPoint& Pt ) { return CPoint(ROUND(Pt.x) , ROUND(Pt.y)); };

inline LPBYTE GetLine8( LPBYTE pData , int iY , int iWidth )
{
  return pData + iY * iWidth ;
}
inline LPWORD GetLine16( LPWORD pData , int iY , int iWidth )
{
  return pData + iY * iWidth ;
}

inline LPVOID GetLine( const pTVFrame fr , int iY )
{
  LPBYTE pData = GetData( fr ) ;
  if ( is16bit( fr ) )
    return GetLine16( (LPWORD) pData , iY , GetWidth( fr ) ) ;
  else
    return GetLine8( pData , iY , GetWidth( fr ) ) ;
}

inline int GetPixel8( LPBYTE pData , int iX , int iY , int iWidth )
{
  return pData[ iY * iWidth + iX ] ;
}
inline int GetPixel8( LPBYTE pData , cmplx Pt , int iWidth )
{
  return GetPixel8( pData , ROUND(Pt.real()) , ROUND(Pt.imag()) , iWidth ) ;
}
inline int GetPixel16( LPWORD pData , int iX , int iY , int iWidth )
{
  return pData[ iY * iWidth + iX ] ;
}
inline int GetPixel16( LPBYTE pData , cmplx Pt , int iWidth )
{
  return GetPixel16( (LPWORD)pData , ROUND(Pt.real()) , ROUND(Pt.imag()) , iWidth ) ;
}
inline int GetPixel( const pTVFrame fr , CPoint& Pt )
{
  LPBYTE pData = GetData( fr ) ;
  if (!pData)
    return 0 ;
  if ( is16bit( fr ) )
    return GetPixel16( (LPWORD)pData , Pt.x , Pt.y , GetWidth(fr) ) ;
  else
    return GetPixel8( pData , Pt.x , Pt.y , GetWidth(fr) ) ;
}
inline int GetPixel( const pTVFrame fr , int iX , int iY )
{
  LPBYTE pData = GetData( fr ) ;
  if (!pData)
    return 0 ;
  if ( is16bit( fr ) )
    return GetPixel16( (LPWORD)pData , iX , iY , GetWidth(fr) ) ;
  else
    return GetPixel8( pData , iX , iY , GetWidth(fr) ) ;
}

inline int GetPixel( const pTVFrame fr , cmplx& Pt )
{
  return GetPixel( fr , ROUND(Pt.real()) , ROUND(Pt.imag()) ) ;
}
inline int GetPixel( const pTVFrame fr , CDPoint& Pt )
{
  return GetPixel( fr , RoundPt(Pt) ) ;
}

inline void SetPixel8( LPBYTE pData , int iX , int iY , int iWidth , int iVal)
{
  pData[ iY * iWidth + iX ] = (BYTE)iVal ;
}
inline void SetPixel16( LPWORD pData , int iX , int iY , int iWidth , int iVal)
{
  pData[ iY * iWidth + iX ] = (WORD)iVal ;
}

inline BOOL SetPixel( pTVFrame fr , int iX , int iY , int iVal )
{
  LPBYTE pData = GetData( fr ) ;
  if (!pData)
    return FALSE ;
  if ( is16bit( fr ) )
    SetPixel16( (LPWORD)pData , iX , iY , GetWidth(fr) , iVal) ;
  else
    SetPixel8( pData , iX , iY , GetWidth(fr) , iVal) ;
  return TRUE ;
}

inline BOOL WhitePixel( const pTVFrame fr , CPoint Pt , int iThres , BOOL bInvert )
{
  return ((GetPixel( fr , Pt) >= iThres) ^ bInvert ) ;
}

inline BOOL TheSame( const pTVFrame fr , CPoint Pt , int iThres , BOOL bInvert )
{
  return !WhitePixel( fr , Pt , iThres , bInvert ) ;
}
inline BOOL Other( const pTVFrame fr , CPoint Pt , int iThres , BOOL bInvert )
{
  return WhitePixel( fr , Pt , iThres , bInvert ) ;
}

inline double GetDeltaArea( cmplx NewPt , cmplx PrevPt )
{
  return 0.5 * 
    (NewPt.imag() - PrevPt.imag()) * (NewPt.real() + PrevPt.real()) ;
}
inline int GetPixelsOnLine( cmplx& Begin , cmplx& End ,
  double * pSignal , int iMaxSignalLen , const pTVFrame pFrame )
{
  cmplx Vector = End - Begin ;
  cmplx Step = polar( 1.0 , arg(Vector) ) ;
  cmplx Pos = Begin ;
  int iNPoints = ROUND(abs(Vector)) ;

  int i = 0 ;
  for (  ; i <= iNPoints  && i < iMaxSignalLen ; i++ )
  {
    *(pSignal++) = GetPixel( pFrame , Pos ) ;
    Pos += Step ;
  }
  return i - 1 ;
}

inline int GetPixelsOnLineSafe( cmplx& Begin , cmplx& End ,
  double * pSignal , int iMaxSignalLen , const pTVFrame pFrame )
{
  cmplx Vector = End - Begin ;
  cmplx Step = polar( 1.0 , arg(Vector) ) ;
  cmplx Pos = Begin ;
  int iNPoints = ROUND(abs(Vector)) ;

  int i = 0 ;
  for (  ; i <= iNPoints  && i < iMaxSignalLen ; i++ )
  {
    if ( IsPtInFrame( Pos , pFrame ) )
    {
      *(pSignal++) = GetPixel( pFrame , Pos ) ;
    }
    Pos += Step ;
  }
  return i - 1 ;
}

inline int GetRadiusPixels( cmplx& Cent , double dAngle_rad ,
  int iMinRad , int iMaxRad , double * pSignal , const pTVFrame pFrame )
{

  cmplx Step = polar( 1.0 , dAngle_rad ) ;
  cmplx Pos = (Step *(double) iMinRad) ;

  Pos += Cent ;
  for ( int i = iMinRad ; i <= iMaxRad ; i++ )
  {
    *(pSignal++) = GetPixel( pFrame , Pos ) ;
    Pos += Step ;
  }
  return (iMaxRad - iMinRad + 1) ;
}

__forceinline pTVFrame makeNewYUV9Frame( DWORD dwWidth , DWORD dwHeight )
{
  if ( dwWidth & 1)   // should be even
    dwWidth++ ;
  if ( dwHeight & 1 ) // should be even
    dwHeight++ ;
  DWORD dwImageSize = ( 9 * dwHeight * dwWidth ) / 8 ;

  pTVFrame retVal = (pTVFrame)malloc(sizeof(TVFrame));
  retVal->lpBMIH = (LPBITMAPINFOHEADER)malloc( sizeof(BITMAPINFOHEADER) + dwImageSize );
  retVal->lpData = NULL ;
  retVal->lpBMIH->biSize = sizeof(BITMAPINFOHEADER) ;
  retVal->lpBMIH->biWidth = dwWidth ;
  retVal->lpBMIH->biHeight = dwHeight ;
  retVal->lpBMIH->biSizeImage = dwImageSize ;
  retVal->lpBMIH->biPlanes = 1 ;
  retVal->lpBMIH->biBitCount = 0 ;
  retVal->lpBMIH->biCompression = BI_YUV9 ;
  retVal->lpBMIH->biXPelsPerMeter = 0 ;
  retVal->lpBMIH->biYPelsPerMeter = 0 ;
  retVal->lpBMIH->biClrUsed = 0 ;
  retVal->lpBMIH->biClrImportant = 0 ;

  return retVal ;
}

__forceinline pTVFrame makeNewY8Frame( DWORD dwWidth , DWORD dwHeight )
{
  DWORD dwImageSize = dwHeight * dwWidth ;

  pTVFrame retVal = (pTVFrame)malloc(sizeof(TVFrame));
  retVal->lpBMIH = (LPBITMAPINFOHEADER)malloc( sizeof(BITMAPINFOHEADER) + dwImageSize );
  retVal->lpData = NULL ;
  retVal->lpBMIH->biSize = sizeof(BITMAPINFOHEADER) ;
  retVal->lpBMIH->biWidth = dwWidth ;
  retVal->lpBMIH->biHeight = dwHeight ;
  retVal->lpBMIH->biSizeImage = dwImageSize ;
  retVal->lpBMIH->biPlanes = 1 ;
  retVal->lpBMIH->biBitCount = 0 ;
  retVal->lpBMIH->biCompression = BI_Y8 ;
  retVal->lpBMIH->biXPelsPerMeter = 0 ;
  retVal->lpBMIH->biYPelsPerMeter = 0 ;
  retVal->lpBMIH->biClrUsed = 0 ;
  retVal->lpBMIH->biClrImportant = 0 ;

  return retVal ;
}


inline int GetAngleIndex( double dAngle_rad , double dAngStep )
{
  double dAngPlus = fabs(dAngle_rad) ;
  int iIndex = ROUND( RadToDeg(dAngStep) )
    * ROUND((dAngPlus + (dAngStep * 0.49999999999))/dAngStep ) ;
  return ((dAngle_rad >= 0) ? iIndex : 360 - iIndex) % 360 ;
}

inline CTextFrame * CreateTextFrame( cmplx& Pt , FXString& Text ,
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


inline CFigureFrame * CreatePtFrame( cmplx& Pt , 
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

inline CFigureFrame * CreateLineFrame( cmplx& Pt1 , cmplx& Pt2 , 
  double dTime = GetHRTickCount() ,const char * pColor = "c0ffc0" ,  
  const char * pLabel = NULL , DWORD dwId = 0 )
{
  CFigureFrame * pLine = CFigureFrame::Create() ;
  if (pLine)
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

inline CFigureFrame * CreateFigureFrame( cmplx * Pts , int iLen , 
  double dTime = GetHRTickCount() ,const char * pColor = "c0ffc0" ,  
  const char * pLabel = NULL , DWORD dwId = 0 )
{
  CFigureFrame * pFigure = CFigureFrame::Create() ;
  if (pFigure)
  {
    for ( int i = 0 ; i < iLen ; i++ )
      pFigure->Add( CDPoint( Pts[i].real() , Pts[i].imag() ) ) ;
    if ( pColor )
      pFigure->Attributes()->WriteString( "color" , pColor ) ;
    if ( pLabel )
      pFigure->SetLabel( pLabel ) ;
    pFigure->SetTime( dTime ) ;
    pFigure->ChangeId( dwId ) ;
  }
  return pFigure ;
}

inline CFigureFrame * CreateGraphic( double * Pts , int iLen ,
  cmplx Origin , cmplx Step , cmplx Range , double dMin , double dMax ,
  const char * pColor = _T("c0ffc0") )
{
  CFigureFrame * pFigure = CFigureFrame::Create() ;
  if (pFigure)
  {
    // Draw axis
    cmplx XEnd = Origin + Step * (double)iLen ;
    cmplx YEnd = Origin + Range ;
    pFigure->Add( CDPoint( XEnd.real() , XEnd.imag() ) ) ;
    pFigure->Add( CDPoint( Origin.real() , Origin.imag() ) ) ;
    pFigure->Add( CDPoint( YEnd.real() , YEnd.imag() ) ) ;

    cmplx CurPos = Origin ;
    for ( int i = 0 ; i < iLen ; i++ )
    {
      cmplx NextPt = CurPos + (Range * (Pts[i] - dMin)/(dMax - dMin)) ;
      pFigure->Add( CDPoint( NextPt.real() , NextPt.imag() ) ) ;
      CurPos += Step ;
    }
    if ( pColor )
      pFigure->Attributes()->WriteString( "color" , pColor ) ;
    pFigure->SetLabel( _T("Graphic") ) ;
    pFigure->SetTime( GetHRTickCount() ) ;
    pFigure->ChangeId( 0 ) ;
  }
  return pFigure ;
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
#endif  // FRAMES_HELPER_H__
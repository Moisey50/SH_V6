//  $File : TVFrame.h - main function for handling with base structure TVFrame
//  (C) Copyright The FileX Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 First release version, all followed changes must be listed below  (Andrey Chernushich)

#ifndef _TVFRAME_INC
#define _TVFRAME_INC



//#include <vfw.h>
#include <video\videoformats.h>
#include "classes/dpoint.h"
#include <math/Intf_sup.h>
#include <ImageProc/LinearSignals.h>
// #include <complex>
// using namespace std;
// 
// typedef complex<double>  cmplx;

// Structure for image data transfer between modules
// There are agreement about data allocation
//          if lpData == NULL - all data is stored in one block - lpBMIH and you can free it after using
//          otherwise - the BITMAPINFOHEADER is stored separately, and will be free automatically
// Use just corresponding functions for initialization and deleting data in a structure


typedef struct tagTVFrame
{
  LPBITMAPINFOHEADER lpBMIH;
  LPBYTE             lpData;
}TVFrame;

#define pTVFrame TVFrame*

inline DWORD GetCompression( const pTVFrame fr )
{
  if ( fr->lpBMIH )
    return fr->lpBMIH->biCompression ;
  else
    return 0xffffffff ;
}

__forceinline DWORD  GetWidth( const pTVFrame frame )
{
  return frame->lpBMIH->biWidth;
}

__forceinline DWORD  GetHeight( const pTVFrame frame )
{
  return frame->lpBMIH->biHeight;
}

__forceinline DWORD  GetPaletteSize( LPBITMAPINFOHEADER lpBMIH )
{
  ASSERT( lpBMIH != NULL );
  if ( (lpBMIH->biCompression == BI_RGB) && (lpBMIH->biBitCount == 8) )
    return sizeof( RGBQUAD )*lpBMIH->biClrUsed;
  return 0;
}

__forceinline DWORD  GetImageSize( LPBITMAPINFOHEADER lpBMIH )
{
  ASSERT( lpBMIH != NULL );
  if ( lpBMIH->biSizeImage != 0 )
    return lpBMIH->biSizeImage;
  DWORD dwBWSize = lpBMIH->biWidth*lpBMIH->biHeight;
  switch ( lpBMIH->biCompression )
  {
  case BI_RGB:
    {
      switch ( lpBMIH->biBitCount )
      {
      case 8:
        return dwBWSize + sizeof( RGBQUAD )*lpBMIH->biClrUsed;
      case 16:
        return dwBWSize * 2;
      case 24:
        return dwBWSize * 3;
      case 32:
        return dwBWSize * 4;
      default:
        return -1;
      }
    }

  case BI_Y8:
  case BI_Y800: return dwBWSize;

  case BI_YUV9: return (dwBWSize * 9) / 8 ;

  case BI_NV12:
  case BI_YUV12: return (dwBWSize * 12) / 8;

  case BI_Y16:
    {
      return dwBWSize * 2;
    }
  default:
    return -1;
  }
}

__forceinline DWORD  GetImageSizeWH( const TVFrame * pTV )
{
  if ( !pTV )
    return 0 ;
  LPBITMAPINFOHEADER lpBMIH = pTV->lpBMIH ;
  if ( lpBMIH == NULL )
    return 0 ;

  int pix = lpBMIH->biWidth*lpBMIH->biHeight;
  switch ( lpBMIH->biCompression )
  {
    case BI_RGB:
    {
      switch ( lpBMIH->biBitCount )
      {
        case 8:
          return pix + sizeof( RGBQUAD )*lpBMIH->biClrUsed;
        case 16:
          return pix * 2;
        case 24:
          return pix * 3;
        case 32:
          return pix * 4;
        default:
          return -1;
      }
    }

    case BI_Y8:
    case BI_Y800:
    case BI_YUV9:
    case BI_NV12:
    case BI_YUV12:
    {
      return pix ;
    }
    case BI_Y16:
    {
      return pix * 2;
    }
    default:
      return -1;
  }
}

__forceinline DWORD  GetImageSize( const pTVFrame frame )
{
  ASSERT( frame->lpBMIH != NULL );
  return GetImageSize( frame->lpBMIH );
}

__forceinline bool FramesCompare( pTVFrame dst , pTVFrame src ) // can copy src to dst or data resize required
{
  if ( dst->lpBMIH == NULL ) return false;
  if ( src->lpBMIH == NULL ) return false;
  if ( dst->lpData == NULL )
  {
    if ( src->lpData != NULL ) return false;
    return (memcmp( dst->lpBMIH , src->lpBMIH , dst->lpBMIH->biSize ) == 0);
  }
  if ( dst->lpBMIH != src->lpBMIH ) return false;
  if ( GetImageSize( dst ) != GetImageSize( src ) ) return false;
  return true;
}

__forceinline DWORD  GetI8Size( pTVFrame frame )
{
  ASSERT( frame->lpBMIH->biCompression != BI_RGB );
  ASSERT( frame->lpBMIH->biSizeImage != 0 );
  return frame->lpBMIH->biHeight*frame->lpBMIH->biWidth;
}

__forceinline LPBYTE GetData( LPBITMAPINFOHEADER DIB )
{
  if ( DIB )
  {
    if ( DIB->biCompression != BI_RGB )
      return ( ( ( LPBYTE ) DIB ) + DIB->biSize );
    else if ( DIB->biBitCount == 8 )
      return ( ( ( LPBYTE ) DIB ) + DIB->biSize + sizeof( RGBQUAD )*DIB->biClrUsed );
    else
      return ( ( ( LPBYTE ) DIB ) + DIB->biSize );
  }
  return NULL;
}

__forceinline  const LPBYTE GetData( const pTVFrame frame )
{
  if ( frame )
  {
    if ( ( frame->lpData ) && ( frame->lpBMIH ) )
      return frame->lpData;
    if ( frame->lpBMIH )
      return ( ( ( LPBYTE ) frame->lpBMIH ) + frame->lpBMIH->biSize
        + sizeof( RGBQUAD )*frame->lpBMIH->biClrUsed );
  }
  return NULL;
}

__forceinline  LPBYTE GetData( pTVFrame frame )
{
  if ( frame )
  {
    if ( (frame->lpData) && (frame->lpBMIH) ) 
      return frame->lpData;
    if ( frame->lpBMIH ) 
      return (((LPBYTE) frame->lpBMIH) + frame->lpBMIH->biSize
        + sizeof( RGBQUAD )*frame->lpBMIH->biClrUsed);
  }
  return NULL;
}

__forceinline  LPWORD GetData16( const pTVFrame frame )
{
  return (LPWORD) GetData( frame );
}

__forceinline  bool is16bit( const pTVFrame frame )
{
  return frame ? (frame->lpBMIH->biCompression == BI_Y16) : false ;
}
enum PlanarSize
{
  PSize_Unknown = -1 ,
  PSize_NotPlanar = 0 ,
  PSize_8Bits = 1 ,
  PSize_16Bits = 2
};
// returns 1 for 8 bits planar,
__forceinline PlanarSize GetPlanarPixelSize( const pTVFrame frame )
{
  switch ( frame->lpBMIH->biCompression )
  {
  case BI_Y8:
  case BI_Y800:
  case BI_YUV9:
  case BI_YUV12:
  case BI_NV12:
  case BI_I420: return PSize_8Bits ;
  case BI_Y16:  return PSize_16Bits ;
  case BI_RGB:
  case BI_YUV411:
  case BI_YUV422:
  case BI_YUY2:
  case BI_UYVY:
  case BI_MJPG:
  case BI_H264:
  case BI_RGB48: return PSize_NotPlanar ;
  }
  return PSize_Unknown ;
}

__forceinline  int Width( const pTVFrame frame )
{
  if ( frame->lpBMIH ) return frame->lpBMIH->biWidth;
  return 0;
}

__forceinline  int Height( const pTVFrame frame )
{
  if ( frame->lpBMIH ) return frame->lpBMIH->biHeight;
  return 0;
}

__forceinline bool GetRect( const pTVFrame frame , CRect& ImRect )
{
  if ( frame && frame->lpBMIH )
  {
    ImRect = CRect( 0 , 0 , frame->lpBMIH->biWidth , frame->lpBMIH->biHeight ) ;
    return true ;
  }
  return false ;
}
__forceinline  DWORD getBMIHsize( LPBITMAPINFOHEADER lpBMIH )
{
  DWORD retVal = 0;
  if ( (lpBMIH) )
  {
    retVal = lpBMIH->biSize + GetImageSize( lpBMIH );
  }
  return retVal;
}

__forceinline  DWORD getsize4BMIH( const pTVFrame frame )
{
  DWORD retVal = 0;
  if ( (frame) && (frame->lpBMIH) )
  {
    retVal = frame->lpBMIH->biSize + GetImageSize( frame );
  }
  return retVal;
}

__forceinline  bool copy2BMIH( LPBITMAPINFOHEADER pDst , const pTVFrame pSrc )
{
  LPBYTE pntr = (LPBYTE) pDst;
  if ( (!pSrc) || (!pSrc->lpBMIH) ) return false;
  if ( pSrc->lpData )
  {
    memcpy( pntr , pSrc->lpBMIH , pSrc->lpBMIH->biSize );
    pntr += pSrc->lpBMIH->biSize;
    memcpy( pntr , pSrc->lpData , GetImageSize( pSrc ) );
  }
  else
  {
    memcpy( pntr , pSrc->lpBMIH , pSrc->lpBMIH->biSize + GetImageSize( pSrc ) );
  }
  return true;
}

__forceinline pTVFrame makeNewYUV12Frame( DWORD dwWidth , DWORD dwHeight )
{
  if ( dwWidth & 1 )   // should be even
    dwWidth++ ;
  if ( dwHeight & 1 ) // should be even
    dwHeight++ ;
  DWORD dwImageSize = (3 * dwHeight * dwWidth) / 2 ;

  pTVFrame retVal = (pTVFrame) malloc( sizeof( TVFrame ) );
  retVal->lpBMIH = (LPBITMAPINFOHEADER) malloc( sizeof( BITMAPINFOHEADER ) + dwImageSize );
  retVal->lpData = NULL ;
  retVal->lpBMIH->biSize = sizeof( BITMAPINFOHEADER ) ;
  retVal->lpBMIH->biWidth = dwWidth ;
  retVal->lpBMIH->biHeight = dwHeight ;
  retVal->lpBMIH->biSizeImage = dwImageSize ;
  retVal->lpBMIH->biPlanes = 1 ;
  retVal->lpBMIH->biBitCount = 12 ;
  retVal->lpBMIH->biCompression = BI_YUV12 ;
  retVal->lpBMIH->biXPelsPerMeter = 0 ;
  retVal->lpBMIH->biYPelsPerMeter = 0 ;
  retVal->lpBMIH->biClrUsed = 0 ;
  retVal->lpBMIH->biClrImportant = 0 ;

  return retVal ;
}

__forceinline pTVFrame makeNewY8Frame( DWORD dwWidth , DWORD dwHeight , LPBYTE pData = NULL )
{
  DWORD dwImageSize = dwHeight * dwWidth ;

  pTVFrame retVal = ( pTVFrame ) malloc( sizeof( TVFrame ) );
  retVal->lpBMIH = ( LPBITMAPINFOHEADER ) malloc( sizeof( BITMAPINFOHEADER ) + dwImageSize );
  retVal->lpData = NULL ;
  retVal->lpBMIH->biSize = sizeof( BITMAPINFOHEADER ) ;
  retVal->lpBMIH->biWidth = dwWidth ;
  retVal->lpBMIH->biHeight = dwHeight ;
  retVal->lpBMIH->biSizeImage = dwImageSize ;
  retVal->lpBMIH->biPlanes = 1 ;
  retVal->lpBMIH->biBitCount = 8 ;
  retVal->lpBMIH->biCompression = BI_Y8 ;
  retVal->lpBMIH->biXPelsPerMeter = 0 ;
  retVal->lpBMIH->biYPelsPerMeter = 0 ;
  retVal->lpBMIH->biClrUsed = 0 ;
  retVal->lpBMIH->biClrImportant = 0 ;

  if ( pData )
    memcpy( retVal->lpBMIH + 1 , pData , dwImageSize ) ;
  else
    memset( retVal->lpBMIH + 1 , 0 , dwImageSize ) ;

  return retVal ;
}

__forceinline  pTVFrame newTVFrame( LPBITMAPINFOHEADER lpBMIH , LPBYTE lpData = NULL )
{
  if ( !lpBMIH ) return NULL;
  pTVFrame retVal = (pTVFrame) malloc( sizeof( TVFrame ) );
  retVal->lpBMIH = lpBMIH;
  retVal->lpData = lpData;
  return retVal;
}

__forceinline  pTVFrame makeTVFrame( LPBITMAPINFOHEADER lpBMIH )
{
  if ( !lpBMIH ) return NULL;
  pTVFrame retVal = (pTVFrame) malloc( sizeof( TVFrame ) );
  retVal->lpBMIH = (LPBITMAPINFOHEADER) malloc( lpBMIH->biSize + GetImageSize( lpBMIH ) );
  memcpy( retVal->lpBMIH , lpBMIH , lpBMIH->biSize + GetImageSize( lpBMIH ) );
  retVal->lpData = NULL;
  return retVal;
}


__forceinline  void freeTVFrame( pTVFrame frame )
{
  if ( !frame ) 
    return;
  if ( frame->lpData ) 
  { 
    free( frame->lpData ); 
    free( frame ); 
  }
  else 
  { 
    free( frame->lpBMIH ); 
    free( frame ); 
  }
}

/* Too many errors while using it
__forceinline  pTVFrame makecopyTVFrame(pTVFrame frame)
{
    if ((!frame) || (!frame->lpBMIH)) return NULL;
    pTVFrame retVal=(pTVFrame)malloc(sizeof(TVFrame));
    if (!frame->lpData)
    {
        int dSize=frame->lpBMIH->biSize+GetImageSize(frame);
        retVal->lpData=NULL;
        retVal->lpBMIH=(LPBITMAPINFOHEADER)malloc(dSize);
        memcpy(retVal->lpBMIH,frame->lpBMIH,dSize);
#if ((defined _DEBUG) && (defined _DETAIL_LOG))
        TRACE("+++ TVFrame.h: Make a second copy of the data: src->lbBMIH=0x%X, dst->lbBMIH=0x%X, lpData=NULL\n",frame->lpBMIH,retVal->lpBMIH);
#endif
    }
    else
    {
        retVal->lpBMIH=frame->lpBMIH;
        retVal->lpData=(LPBYTE)malloc(GetImageSize(frame));
        memcpy(retVal->lpData,frame->lpData,GetImageSize(frame));
#if ((defined _DEBUG) && (defined _DETAIL_LOG))
        TRACE("+++ TVFrame.h: Make a second copy of the data: src->lbBMIH=0x%X, dst->lbBMIH=0x%X, src->lpData=0x%X, dst->lpData=0x%X\n",frame->lpBMIH,retVal->lpBMIH,frame->lpData,retVal->lpData);
#endif
    }
    return retVal;
}
*/

// Previous makecopyTVFrame2
__forceinline  pTVFrame makecopyTVFrame( const pTVFrame frame )
{
  if ( (!frame) || (!frame->lpBMIH) ) return NULL;
  pTVFrame retVal = (pTVFrame) malloc( sizeof( TVFrame ) );
  if ( !frame->lpData )
  {
    int dSize = frame->lpBMIH->biSize + GetImageSize( frame );
    retVal->lpData = NULL;
    retVal->lpBMIH = (LPBITMAPINFOHEADER) malloc( dSize );
    memcpy( retVal->lpBMIH , frame->lpBMIH , dSize );
  }
  else
  {
    int iSize = GetImageSize( frame );
    int dSize = frame->lpBMIH->biSize + iSize;
    retVal->lpData = NULL;
    retVal->lpBMIH = (LPBITMAPINFOHEADER) malloc( dSize );
    if ( retVal->lpBMIH )
    {
      memcpy( retVal->lpBMIH , frame->lpBMIH , frame->lpBMIH->biSize );
      memcpy( (LPBYTE) (retVal->lpBMIH) + frame->lpBMIH->biSize , frame->lpData , iSize );
    }
  }
  return retVal;
}


__forceinline  pTVFrame dublicateTVFrame( const pTVFrame frame )
{
  if ( (!frame) || (!frame->lpBMIH) ) return NULL;
  pTVFrame retVal = (pTVFrame) malloc( sizeof( TVFrame ) );

  int dSize = frame->lpBMIH->biSize + GetImageSize( frame );
  retVal->lpData = NULL;
  retVal->lpBMIH = (LPBITMAPINFOHEADER) malloc( dSize );
  memcpy( retVal->lpBMIH , frame->lpBMIH , frame->lpBMIH->biSize );
  memcpy( ((LPBYTE) retVal->lpBMIH) + frame->lpBMIH->biSize , GetData( frame ) , GetImageSize( frame ) );
  return retVal;
}


__forceinline  bool copyTVFrame( pTVFrame dst , pTVFrame src ) // Valid just for yuv9 and Y9
{
  if ( (dst->lpData == NULL) && (src->lpData == NULL) )
  {
    memcpy( dst->lpBMIH , src->lpBMIH , src->lpBMIH->biSize + GetImageSize( src ) );
    return true;
  }
  if ( dst->lpBMIH == src->lpBMIH )
  {
    memcpy( dst->lpData , src->lpData , GetImageSize( src ) );
    return true;
  }
  else if ( (dst->lpData == NULL) && (src->lpData != NULL) )
  {
    memcpy( (LPBYTE) (dst->lpBMIH) + dst->lpBMIH->biSize , src->lpData , GetImageSize( src ) );
    return true;
  }
  return false;
}

__forceinline  void frame_CopyScales( pTVFrame dst , pTVFrame src )
{
  ASSERT( (dst != 0) && (dst->lpBMIH != NULL) );
  ASSERT( (src != 0) && (src->lpBMIH != NULL) );
  dst->lpBMIH->biXPelsPerMeter = src->lpBMIH->biXPelsPerMeter;
  dst->lpBMIH->biYPelsPerMeter = src->lpBMIH->biYPelsPerMeter;
}

__forceinline  bool _scale_anisotropic( pTVFrame frame )
{
  return (frame->lpBMIH->biXPelsPerMeter != frame->lpBMIH->biYPelsPerMeter);
}

__forceinline  double _get_anisotropy( pTVFrame frame )
{
  if ( !frame->lpBMIH->biXPelsPerMeter ) return 1;
  return ((double) frame->lpBMIH->biYPelsPerMeter / frame->lpBMIH->biXPelsPerMeter);
}

// Processing two field

__forceinline  LPBITMAPINFOHEADER _dualBMICreate( LPBITMAPINFOHEADER Src )
{
  if ( !Src ) return(NULL);
  LPBITMAPINFOHEADER lpbmi = (LPBITMAPINFOHEADER) malloc( Src->biSize + GetImageSize( Src ) );
  memcpy( lpbmi , Src , Src->biSize );
  return(lpbmi);
}

__forceinline  bool _dualBMIDecode( pTVFrame src , pTVFrame dst1 , pTVFrame dst2 )
{
  dst1->lpBMIH = _dualBMICreate( src->lpBMIH ); dst1->lpData = NULL;
  dst2->lpBMIH = _dualBMICreate( src->lpBMIH ); dst2->lpData = NULL;

  LPBYTE offD1 = GetData( dst1 );
  LPBYTE offD2 = GetData( dst2 );

  memset( offD1 , 128 , dst1->lpBMIH->biSizeImage );
  memset( offD2 , 128 , dst2->lpBMIH->biSizeImage );

  LPBYTE  offS1 = GetData( src ) , offS2 = offS1 + src->lpBMIH->biWidth;

  for ( int y = 0; y < dst1->lpBMIH->biHeight; y += 2 )
  {
    memcpy( offD1 , offS1 , src->lpBMIH->biWidth );
    memcpy( offD2 , offS2 , src->lpBMIH->biWidth );
    offD1 += src->lpBMIH->biWidth;
    offD2 += src->lpBMIH->biWidth;
    memcpy( offD1 , offS1 , src->lpBMIH->biWidth );
    memcpy( offD2 , offS2 , src->lpBMIH->biWidth );
    offD1 += src->lpBMIH->biWidth;
    offD2 += src->lpBMIH->biWidth;
    offS1 += 2 * src->lpBMIH->biWidth;
    offS2 += 2 * src->lpBMIH->biWidth;
  }
  return(TRUE);
}

// TVFrame utilities
inline bool IsPtInFrame( cmplx& Pt , const pTVFrame pFrame )
{
  if ( ( Pt.real() <= 0. ) || ( Pt.real() >= GetWidth( pFrame ) ) )
    return false ;
  if ( ( Pt.imag() <= 0. ) || ( Pt.imag() >= GetHeight( pFrame ) ) )
    return false ;
  return true ;
}

inline bool IsPtInFrameSafe( cmplx& Pt , const pTVFrame pFrame , double dSafetyDist = 10. )
{
  if ( ( Pt.real() <= dSafetyDist ) || ( Pt.real() >= GetWidth( pFrame ) - dSafetyDist ) )
    return false ;
  if ( ( Pt.imag() <= dSafetyDist ) || ( Pt.imag() >= GetHeight( pFrame ) - dSafetyDist ) )
    return false ;
  return true ;
}

inline CPoint RoundPt( CDPoint& Pt ) { return CPoint( ROUND( Pt.x ) , ROUND( Pt.y ) ); };

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
    return GetLine16( ( LPWORD ) pData , iY , GetWidth( fr ) ) ;
  else
    return GetLine8( pData , iY , GetWidth( fr ) ) ;
}
inline int GetPixel8( LPBYTE pData , int iX , int iY , int iWidth )
{
  return pData[ iY * iWidth + iX ] ;
}
inline int GetPixel8( LPBYTE pData , cmplx Pt , int iWidth )
{
  return GetPixel8( pData , ROUND( Pt.real() ) , ROUND( Pt.imag() ) , iWidth ) ;
}
inline int GetPixel16( LPWORD pData , int iX , int iY , int iWidth )
{
  return pData[ iY * iWidth + iX ] ;
}
inline int GetPixel16( LPBYTE pData , cmplx Pt , int iWidth )
{
  return GetPixel16( ( LPWORD ) pData , ROUND( Pt.real() ) , ROUND( Pt.imag() ) , iWidth ) ;
}
inline LPBYTE GetPixel24Ptr(
  LPBYTE pData , int iX , int iY , int iImWidth , int iImHeight )
{
  return pData + ( ( iImHeight - iY - 1 ) * iImWidth + iX ) * 3 ;
}
inline LPBYTE GetPixel24Ptr( LPBYTE pData , cmplx Pt , int iImWidth , int iImHeight )
{
  return GetPixel24Ptr(
    pData , ROUND( Pt.real() ) , ROUND( Pt.imag() ) , iImWidth , iImHeight ) ;
}
inline LPBYTE GetPixel24Ptr( pTVFrame pTV , cmplx Pt )
{
  return GetPixel24Ptr( GetData( pTV ) , Pt , GetWidth( pTV ) , GetHeight( pTV ) ) ;
}
inline int GetPixel( const pTVFrame fr , CPoint& Pt )
{
  LPBYTE pData = GetData( fr ) ;
  if ( !pData )
    return 0 ;
  if ( is16bit( fr ) )
    return GetPixel16( ( LPWORD ) pData , Pt.x , Pt.y , GetWidth( fr ) ) ;
  else
    return GetPixel8( pData , Pt.x , Pt.y , GetWidth( fr ) ) ;
}
inline int GetPixel( const pTVFrame fr , int iX , int iY )
{
  LPBYTE pData = GetData( fr ) ;
  if ( !pData )
    return 0 ;
  if ( is16bit( fr ) )
    return GetPixel16( ( LPWORD ) pData , iX , iY , GetWidth( fr ) ) ;
  else
    return GetPixel8( pData , iX , iY , GetWidth( fr ) ) ;
}

inline int GetPixel( const pTVFrame fr , cmplx& Pt )
{
  return GetPixel( fr , ROUND( Pt.real() ) , ROUND( Pt.imag() ) ) ;
}
inline int GetPixel( const pTVFrame fr , CDPoint& Pt )
{
  return GetPixel( fr , ROUND( Pt.x ) , ROUND( Pt.y ) ) ;
}

inline void SetPixel8( LPBYTE pData , int iX , int iY , int iWidth , int iVal )
{
  pData[ iY * iWidth + iX ] = ( BYTE ) iVal ;
}
inline void SetPixel16( LPWORD pData , int iX , int iY , int iWidth , int iVal )
{
  pData[ iY * iWidth + iX ] = ( WORD ) iVal ;
}

inline BOOL SetPixel( pTVFrame fr , int iX , int iY , int iVal )
{
  LPBYTE pData = GetData( fr ) ;
  if ( !pData )
    return FALSE ;
  if ( is16bit( fr ) )
    SetPixel16( ( LPWORD ) pData , iX , iY , GetWidth( fr ) , iVal ) ;
  else
    SetPixel8( pData , iX , iY , GetWidth( fr ) , iVal ) ;
  return TRUE ;
}

inline BOOL SetPixel( pTVFrame fr , cmplx& Pt , int iVal )
{
  LPBYTE pData = GetData( fr ) ;
  if ( !pData )
    return FALSE ;
  if ( is16bit( fr ) )
    SetPixel16( ( LPWORD ) pData , ( int ) Pt.real() , ( int ) Pt.imag() , GetWidth( fr ) , iVal ) ;
  else
    SetPixel8( pData , ( int ) Pt.real() , ( int ) Pt.imag() , GetWidth( fr ) , iVal ) ;
  return TRUE ;
}

inline BOOL WhitePixel( const pTVFrame fr , CPoint Pt , int iThres )
{
  return ( GetPixel( fr , Pt ) >= iThres ) ;
}


inline BOOL WhitePixel( const pTVFrame fr , CPoint Pt , int iThres , BOOL bInvert )
{
  return ( WhitePixel( fr , Pt , iThres ) ^ bInvert ) ;
}

inline BOOL TheSame( const pTVFrame fr , CPoint Pt , int iThres , BOOL bInvert )
{
  return !WhitePixel( fr , Pt , iThres , bInvert ) ;
}
inline BOOL Other( const pTVFrame fr , CPoint Pt , int iThres , BOOL bInvert )
{
  return WhitePixel( fr , Pt , iThres , bInvert ) ;
}

inline CPoint GetContrastFor4Neightbour( const pTVFrame fr ,
  CPoint Pt , int iThres )
{
  BOOL bInverse = WhitePixel( fr , Pt , iThres ) ;
  Pt.x++ ;
  if ( !WhitePixel( fr , Pt , iThres ) )
    return Pt ;
  Pt.x -= 2 ;
  if ( !WhitePixel( fr , Pt , iThres ) )
    return Pt ;
  Pt.x++ ;
  Pt.y++ ;
  if ( !WhitePixel( fr , Pt , iThres ) )
    return Pt ;
  Pt.y -= 2 ;
  if ( !WhitePixel( fr , Pt , iThres ) )
    return Pt ;
  return CPoint( 0 , 0 ) ;
}

inline int SetPixelsOnLine( cmplx& Begin , cmplx& End ,
  int * pSignal , int iMaxSignalLen , pTVFrame pFrame )
{
  cmplx Vector = End - Begin ;
  cmplx Step = polar( 1.0 , arg( Vector ) ) ;
  cmplx Pos = Begin ;
  int iNPoints = ROUND( abs( Vector ) ) ;

  int i = 0 ;
  for ( ; i <= iNPoints ; i++ )
  {
    SetPixel( pFrame , Pos , *pSignal ) ;
    Pos += Step ;
    if ( i < iMaxSignalLen )
      pSignal++ ;
  }
  return i - 1 ;
}

inline bool GetNegativeNeightbour(
  const pTVFrame ptv , CPoint Pt , int iThres , CPoint& FoundPt )
{
  BOOL bInvert = GetPixel( ptv , Pt ) >= iThres ;
  Pt.x += 1 ;  // Check right
  if ( WhitePixel( ptv , Pt , iThres , bInvert ) )
  {
    FoundPt = Pt ;
    return true ;
  }
  Pt.x -= 2 ; // Check left
  if ( WhitePixel( ptv , Pt , iThres , bInvert ) )
  {
    FoundPt = Pt ;
    return true ;
  }
  Pt += CSize( 1 , 1 ) ; // Check down
  if ( WhitePixel( ptv , Pt , iThres , bInvert ) )
  {
    FoundPt = Pt ;
    return true ;
  }
  Pt.y -= 2 ;  // Check up
  if ( WhitePixel( ptv , Pt , iThres , bInvert ) )
  {
    FoundPt = Pt ;
    return true ;
  }
  Pt.x += 1  ;  // Check up right
  if ( WhitePixel( ptv , Pt , iThres , bInvert ) )
  {
    FoundPt = Pt ;
    return true ;
  }
  Pt.x -= 2 ; // Check up left
  if ( WhitePixel( ptv , Pt , iThres , bInvert ) )
  {
    FoundPt = Pt ;
    return true ;
  }
  Pt.y += 2 ; // Check down left
  if ( WhitePixel( ptv , Pt , iThres , bInvert ) )
  {
    FoundPt = Pt ;
    return true ;
  }
  Pt.x += 2 ; // Check down right
  if ( WhitePixel( ptv , Pt , iThres , bInvert ) )
  {
    FoundPt = Pt ;
    return true ;
  }
  return false ;
}

inline int GetRadiusPixels( cmplx& Cent , double dAngle_rad ,
  int iMinRad , int iMaxRad , double * pSignal , const pTVFrame pFrame )
{

  cmplx Step = polar( 1.0 , dAngle_rad ) ;
  cmplx Pos = ( Step *( double ) iMinRad ) ;

  Pos += Cent ;
  for ( int i = iMinRad ; i <= iMaxRad ; i++ )
  {
    *( pSignal++ ) = GetPixel( pFrame , Pos ) ;
    Pos += Step ;
  }
  return ( iMaxRad - iMinRad + 1 ) ;
}

__forceinline pTVFrame makeNewYUV9Frame( DWORD dwWidth , DWORD dwHeight )
{
  if ( dwWidth & 1 )   // should be even
    dwWidth++ ;
  if ( dwHeight & 1 ) // should be even
    dwHeight++ ;
  DWORD dwImageSize = ( 9 * dwHeight * dwWidth ) / 8 ;

  pTVFrame retVal = ( pTVFrame ) malloc( sizeof( TVFrame ) );
  retVal->lpBMIH = ( LPBITMAPINFOHEADER ) malloc( sizeof( BITMAPINFOHEADER ) + dwImageSize );
  retVal->lpData = NULL ;
  retVal->lpBMIH->biSize = sizeof( BITMAPINFOHEADER ) ;
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
__forceinline pTVFrame makeNewRGBFrame( DWORD dwWidth , DWORD dwHeight )
{
  DWORD dwImageSize = dwHeight * dwWidth * 3 ;

  pTVFrame retVal = ( pTVFrame ) malloc( sizeof( TVFrame ) );
  retVal->lpBMIH = ( LPBITMAPINFOHEADER ) malloc( sizeof( BITMAPINFOHEADER ) + dwImageSize );
  retVal->lpData = NULL ;
  retVal->lpBMIH->biSize = sizeof( BITMAPINFOHEADER ) ;
  retVal->lpBMIH->biWidth = dwWidth ;
  retVal->lpBMIH->biHeight = dwHeight ;
  retVal->lpBMIH->biSizeImage = dwImageSize ;
  retVal->lpBMIH->biPlanes = 1 ;
  retVal->lpBMIH->biBitCount = 24 ;
  retVal->lpBMIH->biCompression = BI_RGB ;
  retVal->lpBMIH->biXPelsPerMeter = 0 ;
  retVal->lpBMIH->biYPelsPerMeter = 0 ;
  retVal->lpBMIH->biClrUsed = 0 ;
  retVal->lpBMIH->biClrImportant = 0 ;

  return retVal ;
}

__forceinline pTVFrame makeNewY16Frame( DWORD dwWidth , DWORD dwHeight )
{
  DWORD dwImageSize = dwHeight * dwWidth * 2 ;

  pTVFrame retVal = ( pTVFrame ) malloc( sizeof( TVFrame ) );
  retVal->lpBMIH = ( LPBITMAPINFOHEADER ) malloc( sizeof( BITMAPINFOHEADER ) + dwImageSize );
  retVal->lpData = NULL ;
  retVal->lpBMIH->biSize = sizeof( BITMAPINFOHEADER ) ;
  retVal->lpBMIH->biWidth = dwWidth ;
  retVal->lpBMIH->biHeight = dwHeight ;
  retVal->lpBMIH->biSizeImage = dwImageSize ;
  retVal->lpBMIH->biPlanes = 1 ;
  retVal->lpBMIH->biBitCount = 16 ;
  retVal->lpBMIH->biCompression = BI_Y16 ;
  retVal->lpBMIH->biXPelsPerMeter = 0 ;
  retVal->lpBMIH->biYPelsPerMeter = 0 ;
  retVal->lpBMIH->biClrUsed = 0 ;
  retVal->lpBMIH->biClrImportant = 0 ;

  return retVal ;
}

inline int FormBMPIH( CSize ImageSize , DWORD Compression , BITMAPINFOHEADER * pResult )
{
  memset( pResult , 0 , sizeof( *pResult ) ) ;
  pResult->biSize = sizeof( *pResult ) ;
  pResult->biWidth = ImageSize.cx ;
  pResult->biHeight = ImageSize.cy ;
  pResult->biCompression = Compression ;
  DWORD dwImageSize = ImageSize.cx * ImageSize.cy ;
  switch (Compression)
  {
    case BI_Y8:
    case BI_Y800:
      pResult->biSizeImage = dwImageSize ;
      pResult->biPlanes = 1 ;
      pResult->biBitCount = 8 ;
      pResult->biXPelsPerMeter = 0 ;
      pResult->biYPelsPerMeter = 0 ;
      pResult->biClrUsed = 0 ;
      pResult->biClrImportant = 0 ;
      return dwImageSize ;
    case BI_Y16:
      pResult->biSizeImage = dwImageSize * 2 ;
      pResult->biPlanes = 1 ;
      pResult->biBitCount = 16 ;
      pResult->biXPelsPerMeter = 0 ;
      pResult->biYPelsPerMeter = 0 ;
      pResult->biClrUsed = 0 ;
      pResult->biClrImportant = 0 ;
      return dwImageSize ;
    case BI_YUV9:
      pResult->biSizeImage = ( dwImageSize * 9 ) / 8 ;
      pResult->biPlanes = 1 ;
      pResult->biBitCount = 9 ;
      pResult->biXPelsPerMeter = 0 ;
      pResult->biYPelsPerMeter = 0 ;
      pResult->biClrUsed = 0 ;
      pResult->biClrImportant = 0 ;
      return dwImageSize ;
    case BI_YUV12:
    case BI_NV12:
      pResult->biSizeImage = ( dwImageSize * 3 ) / 2 ;
      pResult->biPlanes = 1 ;
      pResult->biBitCount = 12 ;
      pResult->biXPelsPerMeter = 0 ;
      pResult->biYPelsPerMeter = 0 ;
      pResult->biClrUsed = 0 ;
      pResult->biClrImportant = 0 ;
      return dwImageSize ;
    case BI_RGB:
      pResult->biSizeImage = ( dwImageSize * 3 ) ;
      pResult->biPlanes = 1 ;
      pResult->biBitCount = 24 ;
      pResult->biXPelsPerMeter = 0 ;
      pResult->biYPelsPerMeter = 0 ;
      pResult->biClrUsed = 0 ;
      pResult->biClrImportant = 0 ;
      return dwImageSize ;
    case BI_YUY2:
    case BI_UYVY:
      pResult->biSizeImage = ( dwImageSize * 2 ) ;
      pResult->biPlanes = 1 ;
      pResult->biBitCount = 16 ;
      pResult->biXPelsPerMeter = 0 ;
      pResult->biYPelsPerMeter = 0 ;
      pResult->biClrUsed = 0 ;
      pResult->biClrImportant = 0 ;
      return dwImageSize ;
  }
  return 0 ;
}
inline int GetPixelsOnLine( cmplx& Begin , cmplx& End ,
  double * pSignal , int iMaxSignalLen , const pTVFrame pFrame ,
  double * pAvg = NULL)
{
  cmplx Vector = End - Begin ;
  cmplx Step = polar( 1.0 , arg( Vector ) ) ;
  cmplx Pos = Begin ;
  int iNPoints = ROUND( abs( Vector ) ) ;

  int i = 0 ;
  if ( !pAvg )
  {
    for ( ; i <= iNPoints && i < iMaxSignalLen ; i++ )
    {
      *( pSignal++ ) = GetPixel( pFrame , Pos ) ;
      Pos += Step ;
    }
  }
  else
  {
    for ( ; i < iNPoints && i < iMaxSignalLen ; i++ )
    {
      *pAvg += ( *( pSignal++ ) = GetPixel( pFrame , Pos ) ) ;
      Pos += Step ;
    }
    *pAvg /= i ;
  }
  return i ;
}

inline int GetPixelsOnLine(cmplx& Begin, cmplx Step, double dDist,
  double * pSignal, int iMaxSignalLen, const pTVFrame pFrame,
  double * pAvg = NULL)
{
  cmplx Pos = Begin;

  int i = 0;
  if (!pAvg)
  {
    for (; (abs(Pos - Begin) < dDist) && (i < iMaxSignalLen); i++)
    {
      *(pSignal++) = GetPixel(pFrame, Pos);
      Pos += Step;
    }
  }
  else
  {
    for (; (abs(Pos - Begin) < dDist) && (i < iMaxSignalLen); i++)
    {
      *pAvg += (*(pSignal++) = GetPixel(pFrame, Pos));
      Pos += Step;
    }
    *pAvg /= i;
  }
  return i;
}

inline int GetPixelsOnLineSafe(cmplx& Begin, cmplx Step, double dDist,
  double * pSignal, int iMaxSignalLen, const pTVFrame pFrame,
  double * pAvg = NULL)
{
  cmplx Pos = Begin;
  int iTaken = 0;
  int i = 0;

  if (!pAvg)
  {
    for (; (abs(Pos - Begin) < dDist) && (i < iMaxSignalLen); i++)
    {
      if (IsPtInFrame(Pos, pFrame))
      {
        *(pSignal++) = GetPixel(pFrame, Pos);
        iTaken++;
      }
      else
        *(pSignal++) = -1. ;
      Pos += Step;
    }
  }
  else
  {
    for (; (abs(Pos - Begin) < dDist) && (i < iMaxSignalLen); i++)
    {
      if (IsPtInFrame(Pos, pFrame))
      {
        *pAvg += (*(pSignal++) = GetPixel(pFrame, Pos));
        iTaken++;
      }
      else
        *(pSignal++) = -1.;
      Pos += Step;
    }
    *pAvg /= iTaken ;
  }
  return iTaken ;
}

inline int GetPixelsOnLineSafe(cmplx& Begin, cmplx& End,
  double * pSignal, int iMaxSignalLen, const pTVFrame pFrame,
  double * pAvg = NULL)
{
  cmplx Vector = End - Begin;
  cmplx Step = polar(1.0, arg(Vector));
  cmplx Pos = Begin;

  return GetPixelsOnLineSafe(Begin, Step, abs(Vector),
    pSignal, iMaxSignalLen, pFrame, pAvg);
}

inline cmplx GetThresCrossOnLine( cmplx& Begin , cmplx Step , double dDist ,
  double * pSignal , int iMaxSignalLen , const pTVFrame pFrame ,
  double dThres )
{
  int iNSamples = GetPixelsOnLine( Begin , Step , dDist , pSignal , iMaxSignalLen , pFrame ) ;

  double dPos = find_border_forw( pSignal , iNSamples , dThres ) ;
  cmplx cRes ;
  if ( dPos )
    cRes = Begin + Step * dPos ;
  return cRes ;
}


inline double GetAverageOnLine( cmplx& Begin , cmplx Step , double dDist ,
  const pTVFrame pFrame )
{
  cmplx Pos = Begin ;
  int i = 0 ;
  double dAvg = 0. ;
  for ( ; ( abs( Pos - Begin ) < dDist ) ; i++ )
  {
    dAvg += GetPixel( pFrame , Pos ) ;
    Pos += Step ;
  }
  dAvg /= i ;
  return dAvg ;
}

inline double GetAverageOnLine( cmplx& Begin , cmplx Step , int iNSteps ,
  const pTVFrame pFrame )
{
  cmplx Pos = Begin ;
  int i = 0 ;
  double dAvg = 0. ;
  for ( ; i < iNSteps ; i++ )
  {
    dAvg += GetPixel( pFrame , Pos ) ;
    Pos += Step ;
  }
  dAvg /= i ;
  return dAvg ;
}


inline double GetAverageOnLineSafe( cmplx& Begin ,
  cmplx Step , double dDist , const pTVFrame pFrame )
{
  cmplx Pos = Begin ;
  int iNAccum = 0;;
  double dAvg = 0. ;
  for ( ; ( abs( Pos - Begin ) < dDist ) ;  Pos += Step )
  {
    if (IsPtInFrame(Pos, pFrame))
    {
      dAvg += GetPixel(pFrame, Pos);
      iNAccum++;
    }
  }
  if ( iNAccum )
    return ( dAvg /= iNAccum) ;
  return 0. ;
}

inline double GetAverageOnLineSafe( cmplx& Begin ,
  cmplx Step , int iNSteps , const pTVFrame pFrame )
{
  cmplx Pos = Begin ;
  int iNAccum = 0;;
  double dAvg = 0.;
  for (; 0 < iNSteps-- ; Pos += Step)
  {
    if (IsPtInFrame(Pos, pFrame))
    {
      dAvg += GetPixel(pFrame, Pos);
      iNAccum++;
    }
  }
  if (iNAccum)
    return (dAvg /= iNAccum);
  return 0. ;
}

inline double GetAvrgAndStdOnLine( cmplx& cBegin , cmplx& cDir , double dDist ,
  double * pSignal , int iMaxLen , double& dStd2 , const pTVFrame pFrame )
{
  double dAv = 0. ;

  int iLen = GetPixelsOnLine( cBegin , cDir , dDist ,
    pSignal , iMaxLen , pFrame , &dAv ) ;
  dStd2 = 0. ;
  if ( iLen >= 2 )
  {
    for ( int i = 0; i < iLen; i++ )
    {
      double dDiff = pSignal[ i ] - dAv ;
      dStd2 += dDiff * dDiff ;
    }
    dStd2 /= iLen - 1 ;
    dStd2 = sqrt( dStd2 );
  }
  return dAv ;
}

inline double GetAverageSignalOnStrip(
  cmplx& cBegin , // strip corner
  cmplx& cDirAlongStep , // strip direction
  int    iNAlongSteps ,
  int    iNCrossSteps , // Every cross step will be 90 degrees to the left
                           // with the same length
  double * pSignal , // average value of cross direction will be saved
                      // in this array for every step in along direction
  int iMaxSignalLen ,
  const pTVFrame pFrame )
{
  if ( iNAlongSteps <= 0. )
    return 0. ;
  double dAver = 0. ;
  cmplx cAlongIter = cBegin ;
  cmplx cCrossStep = GetOrthoRight( cDirAlongStep ) ;
  for ( int iAlong = 0 ; iAlong < iNAlongSteps ;
    cAlongIter += cDirAlongStep , iAlong++ )
  {
    dAver += ( pSignal[ iAlong ] = GetAverageOnLine(
      cAlongIter , cCrossStep , iNCrossSteps , pFrame ) );
  }
  return ( dAver / iNAlongSteps ) ;
}

#undef cmplx 

#endif
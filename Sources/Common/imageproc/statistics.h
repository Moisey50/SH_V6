//  $File : statistics.h - calculate some statistic data for the image
//  (C) Copyright The FileX Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#ifndef STATISTICS_INC
#define STATISTICS_INC

#include <video\tvframe.h>
#include <imageproc\simpleip.h>
#include <imageproc\imagebits.h>
#include <afxtempl.h>
#include <afxmt.h>

class CSData : public CArray<double , double>
{
public:
  FXLockObject m_Busy;
};

inline void GetSection( pTVFrame frame , CPoint a , CPoint b , CSData& DataBuffer )
{
  DataBuffer.m_Busy.Lock();
  int dx = b.x - a.x; int dy = b.y - a.y;
  int x = a.x , y = a.y; LPBYTE data;
  DataBuffer.RemoveAll();
  if ( dx == 0 )
  {
    for ( y = a.y; y != b.y; y += ( ( dy < 0 ) ? -1 : 1 ) )
    {
      data = __getdata_I_XY( frame , x , y );
      if ( data ) DataBuffer.Add( *data );
    }
  }
  else if ( dy == 0 )
  {
    for ( x = a.x; x != b.x; x += ( ( dx < 0 ) ? -1 : 1 ) )
    {
      data = __getdata_I_XY( frame , x , y );
      if ( data ) DataBuffer.Add( *data );
    }
  }
  else if ( abs( dx ) > abs( dy ) )
  {

    double r = ( ( double ) dy ) / dx;
    for ( x = a.x; x != b.x; x += ( ( dx < 0 ) ? -1 : 1 ) )
    {
      y = a.y + ( int ) ( ( x - a.x )*r + 0.5 );
      data = __getdata_I_XY( frame , x , y );
      if ( data ) DataBuffer.Add( *data );
    }
  }
  else
  {
    double r = ( ( double ) dx ) / dy;
    for ( y = a.y; y != b.y; y += ( ( dy < 0 ) ? -1 : 1 ) )
    {
      x = a.x + ( int ) ( ( y - a.y )*r + 0.5 );
      data = __getdata_I_XY( frame , x , y );
      if ( data ) DataBuffer.Add( *data );
    }
  }
  DataBuffer.m_Busy.Unlock();
}

inline void GetHistogram( pTVFrame frame , CSData& DataBuffer , CSData& U , CSData& V )
{
  DataBuffer.m_Busy.Lock();
  DataBuffer.RemoveAll();
  DataBuffer.SetSize( 256 , -1 );
  U.RemoveAll();
  U.SetSize( 256 , -1 );
  V.RemoveAll();
  V.SetSize( 256 , -1 );

  LPBYTE s = GetData( frame );
  LPBYTE e = s + frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
  while ( s < e )
  {
    DataBuffer[ *s ]++; s++;
  }
  s = GetData( frame ) + frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
  e = s + frame->lpBMIH->biWidth*frame->lpBMIH->biHeight / 16;
  while ( s < e )
  {
    U[ *s ]++; s++;
  }
  e = s + frame->lpBMIH->biWidth*frame->lpBMIH->biHeight / 16;
  while ( s < e )
  {
    V[ *s ]++; s++;
  }
  DataBuffer.m_Busy.Unlock();
}

inline void GetHistogram( const pTVFrame frame , RECT rc , CSData& DataBuffer , CSData* U = NULL , CSData* V = NULL )
{
  DataBuffer.m_Busy.Lock();
  DataBuffer.RemoveAll();
  int size = ( rc.right - rc.left )*( rc.bottom - rc.top );
  if ( size )
  {
    DataBuffer.SetSize( 256 , -1 );
    for ( int i = 0; i < 256; i++ ) DataBuffer[ i ] = 0;
  }
  if ( U )
  {
    U->RemoveAll();
    if ( size )
    {
      U->SetSize( 256 , -1 );
      for ( int i = 0; i < 256; i++ ) ( *U )[ i ] = 0;
    }
  }
  if ( V )
  {
    V->RemoveAll();
    if ( size )
    {
      V->SetSize( 256 , -1 );
      for ( int i = 0; i < 256; i++ ) ( *V )[ i ] = 0;
    }
  }

  for ( int y = rc.top; y < rc.bottom; y++ )
  {
    for ( int x = rc.left; x < rc.right; x++ )
    {
      if ( ( !U ) && ( !V ) )
      {
        DataBuffer[ *__getdata_I_XY( frame , x , y ) ]++;
      }
      else
      {
        int _i , _u , _v;
        __getdata_IUV( frame , x , y , _i , _u , _v );
        DataBuffer[ _i ]++;
        if ( U ) ( *U )[ _u ]++;
        if ( V ) ( *V )[ _v ]++;
      }
    }
  }
  DataBuffer.m_Busy.Unlock();
}

// For 8 bits planar images only
inline void GetHistogram( const pTVFrame frame , int DataBuffer[ 256 ] )
{
  memset( DataBuffer , 0 , sizeof( int ) * 256 );
  LPBYTE s = GetData( frame );
  LPBYTE e = s + frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
  while ( s < e )
  {
    DataBuffer[ *s ]++; s++;
  }
}

// For 8 bits planar images only
inline void GetHistogram( const pTVFrame frame , CRect ROI , int DataBuffer[ 256 ] )
{
  memset( DataBuffer , 0 , sizeof( int ) * 256 );
  for ( int iY = ROI.top ; iY < ROI.bottom ; iY++ )
  {
    LPBYTE s = GetData( frame ) + iY * ROI.Width() ;
    LPBYTE e = (s--) + frame->lpBMIH->biWidth ;
    while ( s++ < e )
      DataBuffer[ *s ]++; 
  }
}

// For 8 bits planar images only
inline void GetHistogram( const pTVFrame frame , int DataBuffer[ 256 ] , int &maxV , int &maxPos )
{
  memset( DataBuffer , 0 , sizeof( int ) * 256 );
  maxV = 0;
  LPBYTE s = GetData( frame );
  LPBYTE e = s + frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
  while ( s < e )
  {
    DataBuffer[ *s ]++;
    if ( DataBuffer[ *s ] > maxV )
    {
      maxV = DataBuffer[ *s ];
      maxPos = *s;
    }
    s++;
  }
}

// For 8 bits planar images only
inline void GetHistogram( const pTVFrame frame , CRect ROI , 
  int DataBuffer[ 256 ] , int &maxH , int &maxPos  )
{
  memset( DataBuffer , 0 , sizeof( int ) * 256 );
  maxH = 0;
  LPBYTE pOrig = GetData( frame ) ;
  LPBYTE s = pOrig + ROI.left + ROI.top * GetWidth( frame );
  for ( int iY = ROI.top ; iY < ROI.bottom ; iY++ )
  {
    LPBYTE p = s ;
    LPBYTE e = p + ROI.Width() ;
    while ( p < e )
      DataBuffer[ *( p++) ]++;
    s += GetWidth( frame );
  }
  for ( int i = 0 ; i < 256 ; i++ )
  {
    if ( maxH < DataBuffer[i] )
    {
      maxH = DataBuffer[ i ] ;
      maxPos = i ;
    }
  }
}
// function returns such maximal brightness level that number of image pixels with higher
// brightness is higher than dNormaLevel * <number of pixels in image>
// dNormalLevel is number from 0. to 1.0
// works for 8 bits images only
inline int GetCutLevelByHisto8( 
  const pTVFrame pImage, double dNormLevel , 
  int * piHisto = NULL , CRect * pROI = NULL )
{
  if ( !pImage )
    return 0 ;
  int Histo[ 256 ] ;
  if ( piHisto == NULL )
    piHisto = Histo ;
  if ( pROI )
    GetHistogram( pImage , *pROI , piHisto ) ;
  else
    GetHistogram( pImage , piHisto ) ;
  int iNPixels = GetWidth( pImage ) * GetHeight( pImage ) ;
  __int64 iThreshold = (__int64)( iNPixels * dNormLevel ) ;
  __int64 iSum = 0 ;
  for ( int i = 255 ; i >= 0 ; i-- )
  {
    iSum += piHisto[ i ] ;
    if ( iSum >= iThreshold )
      return i ;
  }
  return 0 ;
}
__forceinline double _calc_diff_sum( const pTVFrame frame , CRect& Area )
{
  LPBYTE lpData = GetData( frame );
  if ( !lpData )
    return false;

  int iLeft = Area.left ;
  if ( iLeft < 0 )
    iLeft = 0 ;
  int iTop = Area.top ;
  if ( iTop < 0 )
    iTop = 0 ;
  int iWidth = frame->lpBMIH->biWidth ;
  int iLineStep = iWidth ;
  if ( iLeft + Area.Width() >= iWidth )
    iWidth -= iLeft ;
  else
    iWidth = Area.Width() ;
  int iHeight = frame->lpBMIH->biHeight ;
  if ( iTop + Area.Height() >= iHeight )
    iHeight -= iTop ;
  else
    iHeight = Area.Height() ;

  double dResult = 0. ;
  int iYBegin = iTop + 1 ;
  int iYEnd = iTop + iHeight - 1 ;
  switch ( frame->lpBMIH->biCompression )
  {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
    case BI_Y800:
    {
      for ( int iY = iYBegin ; iY < iYEnd ; iY++ )
      {
        LPBYTE pRow = lpData + iLineStep * iY + iLeft ;
        LPBYTE eod = pRow + iWidth - 1 ;

        while ( pRow < eod )
        {
          dResult += abs( *pRow - *( pRow + 1 ) ) ;
          dResult += abs( *pRow - *( pRow + iLineStep ) ) ;
          pRow++ ;
        }
      }
      break ;
    }
    case BI_Y16:
    {
      for ( int iY = iYBegin ; iY < iYEnd ; iY++ )
      {
        LPWORD pRow = ( LPWORD ) lpData + iLineStep * iY + iLeft ;
        LPWORD eod = pRow + iWidth - 1 ;

        while ( pRow < eod )
        {
          dResult += abs( *pRow - *( pRow + 1 ) ) ;
          dResult += abs( *pRow - *( pRow + iLineStep ) ) ;
          pRow++ ;
        }
      }
      break ;
    }
    default: return dResult ;
  }
  int iNActivePixels = ( iWidth - 1 ) * ( iHeight - 1 ) ;
  dResult /= ( double ) iNActivePixels ;
  return dResult ;
}

__forceinline double _find_max_diff( const pTVFrame frame , CRect& Area )
{
  LPBYTE lpData = GetData( frame );
  if ( !lpData )
    return false;

  int iLeft = Area.left ;
  if ( iLeft < 0 )
    iLeft = 0 ;
  int iTop = Area.top ;
  if ( iTop < 0 )
    iTop = 0 ;
  int iWidth = frame->lpBMIH->biWidth ;
  int iLineStep = iWidth ;
  if ( iLeft + Area.Width() >= iWidth )
    iWidth -= iLeft ;
  else
    iWidth = Area.Width() ;
  int iHeight = frame->lpBMIH->biHeight ;
  if ( iTop + Area.Height() >= iHeight )
    iHeight -= iTop ;
  else
    iHeight = Area.Height() ;

  int iMax = 0 ;
  int iYBegin = iTop + 1 ;
  int iYEnd = iTop + iHeight - 1 ;
  switch ( frame->lpBMIH->biCompression )
  {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
    case BI_Y800:
    {
      for ( int iY = iYBegin ; iY < iYEnd ; iY++ )
      {
        LPBYTE pRow = lpData + iLineStep * iY + iLeft ;
        LPBYTE eod = pRow + iWidth - 1 ;

        while ( pRow < eod )
        {
          int iDiffRight = ( int ) *pRow - ( int )*( pRow + 1 ) ;
          iDiffRight = abs( iDiffRight ) ;
          if ( iMax < iDiffRight )
            iMax = iDiffRight ;
          int iDiffDown = ( int ) *pRow - ( int )*( pRow + iLineStep ) ;
          iDiffDown = abs( iDiffDown ) ;
          if ( iMax < iDiffDown )
            iMax = iDiffDown ;
          pRow++ ;
        }
      }
      break ;
    }
    case BI_Y16:
    {
      for ( int iY = iYBegin ; iY < iYEnd ; iY++ )
      {
        LPWORD pRow = ( LPWORD ) lpData + iLineStep * iY + iLeft ;
        LPWORD eod = pRow + iWidth - 1 ;

        while ( pRow < eod )
        {
          int iDiffRight = ( int ) *pRow - ( int )*( pRow + 1 ) ;
          iDiffRight = abs( iDiffRight ) ;
          if ( iMax < iDiffRight )
            iMax = iDiffRight ;
          int iDiffDown = ( int ) *pRow - ( int )*( pRow + iLineStep ) ;
          iDiffDown = abs( iDiffDown ) ;
          if ( iMax < iDiffDown )
            iMax = iDiffDown ;
          pRow++ ;
        }
      }
      break ;
    }
  }
  return ( double ) iMax ;
}

__forceinline double _calc_laplace( const pTVFrame frame , CRect& Area )
{
  LPBYTE lpData = GetData( frame );
  if ( !lpData )
    return false;

  int iLeft = Area.left ;
  if ( iLeft < 0 )
    iLeft = 0 ;
  int iTop = Area.top ;
  if ( iTop < 0 )
    iTop = 0 ;
  int iWidth = frame->lpBMIH->biWidth ;
  int iLineStep = iWidth ;
  if ( iLeft + Area.Width() >= iWidth )
    iWidth -= iLeft ;
  else
    iWidth = Area.Width() ;
  int iHeight = frame->lpBMIH->biHeight ;
  if ( iTop + Area.Height() >= iHeight )
    iHeight -= iTop ;
  else
    iHeight = Area.Height() ;

  int iResult = 0 ;
  int iYBegin = iTop + 1 ;
  int iYEnd = iTop + iHeight - 1 ;
  switch ( frame->lpBMIH->biCompression )
  {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
    case BI_Y800:
    {
      for ( int iY = iYBegin ; iY < iYEnd ; iY++ )
      {
        LPBYTE pRow = lpData + iLineStep * iY + iLeft ;
        LPBYTE eod = pRow + iWidth - 1 ;

        while ( pRow < eod )
        {
          int iLapl = 4 * ( *pRow ) - *( pRow + 1 ) - *( pRow - 1 ) ;
          iLapl -= *( pRow + iLineStep ) + *( pRow - iLineStep ) ;
          iResult += abs( iLapl ) ;
          pRow++ ;
        }
      }
      break ;
    }
    case BI_Y16:
    {
      for ( int iY = iYBegin ; iY < iYEnd ; iY++ )
      {
        LPWORD pRow = ( LPWORD ) lpData + iLineStep * iY + iLeft ;
        LPWORD eod = pRow + iWidth - 1 ;

        while ( pRow < eod )
        {
          int iLapl = 4 * ( *pRow ) - *( pRow + 1 ) - *( pRow - 1 ) ;
          iLapl -= *( pRow + iLineStep ) + *( pRow - iLineStep ) ;
          iResult += abs( iLapl ) ;
          pRow++ ;
        }
      }
      break ;
    }
    default: return ( double ) iResult ;
  }
  int iNActivePixels = ( iWidth - 1 ) * ( iHeight - 1 ) ;
  double dResult = iResult / ( double ) iNActivePixels ;
  return dResult ;
}

__forceinline double _calc_average(
  const pTVFrame frame , CRect& Area )
{
  LPBYTE lpData = GetData( frame );
  if ( !lpData )
    return false;

  int iLeft = Area.left ;
  if ( iLeft < 0 )
    iLeft = 0 ;
  int iTop = Area.top ;
  if ( iTop < 0 )
    iTop = 0 ;
  int iWidth = frame->lpBMIH->biWidth ;
  int iLineStep = iWidth ;
  if ( iLeft + Area.Width() >= iWidth )
    iWidth -= iLeft ;
  else
    iWidth = Area.Width() ;
  int iHeight = frame->lpBMIH->biHeight ;
  if ( iTop + Area.Height() >= iHeight )
    iHeight -= iTop ;
  else
    iHeight = Area.Height() ;

  double dResult = 0. ;
  int iYBegin = iTop ;
  int iYEnd = iTop + iHeight  ;
  switch ( frame->lpBMIH->biCompression )
  {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
    case BI_Y800:
    {
      for ( int iY = iYBegin ; iY < iYEnd ; iY++ )
      {
        int iResult = 0 ;
        LPBYTE pPix = lpData + iLineStep * iY + iLeft ;
        LPBYTE eod = pPix + iWidth ;

        while ( pPix < eod )
          iResult += *( pPix++ ) ;

        dResult += iResult ;
      }
      break ;
    }
    case BI_Y16:
    {
      for ( int iY = iYBegin ; iY < iYEnd ; iY++ )
      {
        int iResult = 0 ;
        LPWORD pPix = ( LPWORD ) lpData + iLineStep * iY + iLeft ;
        LPWORD eod = pPix + iWidth ;

        while ( pPix < eod )
          iResult += *( pPix++ ) ;
        dResult += iResult ;
      }
      break ;
    }
    default: return ( double ) dResult ;
  }
  int iNActivePixels = ( iWidth ) * ( iHeight ) ;
  dResult /= ( double ) iNActivePixels ;
  return dResult ;
}
__forceinline double _calc_average(
  const pTVFrame frame , CRect& Area , int& iMin , int& iMax )
{
  LPBYTE lpData = GetData( frame );
  if ( !lpData )
    return false;

  int iLeft = Area.left ;
  if ( iLeft < 0 )
    iLeft = 0 ;
  int iTop = Area.top ;
  if ( iTop < 0 )
    iTop = 0 ;
  int iWidth = frame->lpBMIH->biWidth ;
  int iLineStep = iWidth ;
  if ( iLeft + Area.Width() >= iWidth )
    iWidth -= iLeft ;
  else
    iWidth = Area.Width() ;
  int iHeight = frame->lpBMIH->biHeight ;
  if ( iTop + Area.Height() >= iHeight )
    iHeight -= iTop ;
  else
    iHeight = Area.Height() ;

  double dResult = 0. ;
  int iYBegin = iTop + 1 ;
  int iYEnd = iTop + iHeight - 1 ;
  switch ( frame->lpBMIH->biCompression )
  {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
    case BI_Y800:
    {
      for ( int iY = iYBegin ; iY < iYEnd ; iY++ )
      {
        int iResult = 0 ;
        LPBYTE pPix = lpData + iLineStep * iY + iLeft ;
        LPBYTE eod = pPix + iWidth - 1 ;

        while ( pPix < eod )
        {
          SetMinMax( *pPix , iMin , iMax ) ;
          iResult += *( pPix++ ) ;
        }

        dResult += iResult ;
      }
      break ;
    }
    case BI_Y16:
    {
      for ( int iY = iYBegin ; iY < iYEnd ; iY++ )
      {
        int iResult = 0 ;
        LPWORD pPix = ( LPWORD ) lpData + iLineStep * iY + iLeft ;
        LPWORD eod = pPix + iWidth - 1 ;

        while ( pPix < eod )
        {
          SetMinMax( *pPix , iMin , iMax ) ;
          iResult += *( pPix++ ) ;
        }
        dResult += iResult ;
      }
      break ;
    }
    default: return ( double ) dResult ;
  }
  int iNActivePixels = ( iWidth - 1 ) * ( iHeight - 1 ) ;
  dResult /= ( double ) iNActivePixels ;
  return dResult ;
}

__forceinline double _calc_average(
  const pTVFrame frame , CPoint Center , int iRadius = 5 )
{
  LPBYTE lpData = GetData( frame );
  if ( !lpData )
    return false;

  CRect Area( Center.x - iRadius , Center.y - iRadius ,
    Center.x + iRadius , Center.y + iRadius );
  int iLeft = Area.left;
  if ( iLeft < 0 )
    iLeft = 0;
  int iTop = Area.top;
  if ( iTop < 0 )
    iTop = 0;
  int iWidth = frame->lpBMIH->biWidth;
  int iLineStep = iWidth;
  if ( iLeft + Area.Width() >= iWidth )
    iWidth -= iLeft;
  else
    iWidth = Area.Width();
  int iHeight = frame->lpBMIH->biHeight;
  if ( iTop + Area.Height() >= iHeight )
    iHeight -= iTop;
  else
    iHeight = Area.Height();

  double dResult = 0.;
  int iYBegin = iTop ;
  int iYEnd = iTop + iHeight - 1;
  switch ( frame->lpBMIH->biCompression )
  {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
    case BI_Y800:
    {
      for ( int iY = iYBegin; iY < iYEnd; iY++ )
      {
        int iResult = 0;
        LPBYTE pPix = lpData + iLineStep * iY + iLeft;
        LPBYTE eod = pPix + iWidth - 1;

        while ( pPix < eod )
          iResult += *( pPix++ );

        dResult += iResult;
      }
      break;
    }
    case BI_Y16:
    {
      for ( int iY = iYBegin; iY < iYEnd; iY++ )
      {
        int iResult = 0;
        LPWORD pPix = ( LPWORD ) lpData + iLineStep * iY + iLeft;
        LPWORD eod = pPix + iWidth - 1;

        while ( pPix < eod )
          iResult += *( pPix++ );

        dResult += iResult;
      }
      break;
    }
    default: return ( double ) dResult;
  }
  int iNActivePixels = iWidth * iHeight ;
  dResult /= ( double ) iNActivePixels;
  return dResult;
}

__forceinline double _calc_average(
  const pTVFrame frame , cmplx Center , int iRadius = 5 )
{
  CPoint CentPt( ROUND( Center.real() ) , ROUND( Center.imag() ) );
  return _calc_average( frame , CentPt , iRadius );
}


#endif
//  $File : resample.h - change scale of the image
//  (C) Copyright The File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)
//   26 Jun 02 Y8 format support for some of procedures is implemented

#ifndef _RESAMPLE_INC
#define _RESAMPLE_INC

#include <video\shvideo.h>
#include <math.h>

__forceinline bool _enlarge8( pTVFrame frame )
{
  if ( !frame ) return false;
  if ( !frame->lpBMIH ) return false;

  ASSERT( ( frame->lpBMIH->biCompression == BI_Y8 ) || ( frame->lpBMIH->biCompression == BI_Y800 ) ); // Check format is supported

  long oldwidth = ( frame->lpBMIH )->biWidth , oldheight = ( frame->lpBMIH )->biHeight;
  LPBYTE src = ( LPBYTE ) GetData( frame );

  int width = ( oldwidth << 1 ) , height = ( oldheight << 1 ) , new_size = ( ( ( frame->lpBMIH )->biSizeImage ) << 2 );

  LPBITMAPINFOHEADER lpNewBM = ( LPBITMAPINFOHEADER ) malloc( new_size + ( frame->lpBMIH )->biSize );
  memset( lpNewBM , 0 , new_size + ( frame->lpBMIH )->biSize );

  LPBYTE dst = ( ( LPBYTE ) ( lpNewBM ) ) + ( frame->lpBMIH )->biSize;
  memcpy( lpNewBM , frame->lpBMIH , ( frame->lpBMIH )->biSize );

  lpNewBM->biWidth = width;
  lpNewBM->biHeight = height;
  lpNewBM->biSizeImage = new_size;

  int y;

  LPBYTE noff , ooff , ooff_e , ooff1;

  for ( y = 0; y < oldheight; y++ )
  {
    noff = dst + 2 * width*y;
    ooff = src + oldwidth * y;
    ooff1 = src + ( ( y < oldheight - 1 ) ? oldwidth * ( y + 1 ) : oldwidth * ( y ) );
    ooff_e = ooff + oldwidth - 1; // Last point will be processed separately
    while ( ooff < ooff_e )
    {
      *( noff ) = *( ooff );
      *( noff + 1 ) = ( *( ooff + 1 ) + *( ooff ) ) >> 1;
      *( noff + width ) = ( *( ooff1 ) +*( ooff ) ) >> 1;
      *( noff + width + 1 ) = ( *( ooff + 1 ) + *( ooff ) +*( ooff1 ) +*( ooff1 + 1 ) ) >> 2;
      ooff++;
      ooff1++;
      noff += 2;
    }
    *( noff ) = *( ooff );
    *( noff + 1 ) = *( ooff );
    *( noff + width ) = ( *( ooff1 ) +*( ooff ) ) >> 1;
    *( noff + width + 1 ) = ( *( ooff1 ) +*( ooff ) ) >> 1;
  }
  if ( frame->lpData )
    free( frame->lpData );
  else
    free( frame->lpBMIH );

  frame->lpBMIH = lpNewBM;
  frame->lpData = NULL;

  return true;
}

__forceinline bool _enlarge_yuv9( pTVFrame frame )
{
  if ( !frame ) return false;
  if ( !frame->lpBMIH ) return false;

  ASSERT( frame->lpBMIH->biCompression == BI_YUV9 ); // Check format is supported

  long oldwidth = ( frame->lpBMIH )->biWidth , oldheight = ( frame->lpBMIH )->biHeight;
  LPBYTE src = ( LPBYTE ) GetData( frame );

  int width = ( oldwidth << 1 ) , height = ( oldheight << 1 ) , new_size = ( ( ( frame->lpBMIH )->biSizeImage ) << 2 );

  LPBITMAPINFOHEADER lpNewBM = ( LPBITMAPINFOHEADER ) malloc( new_size + ( frame->lpBMIH )->biSize );
  memset( lpNewBM , 0 , new_size + ( frame->lpBMIH )->biSize );

  LPBYTE dst = ( ( LPBYTE ) ( lpNewBM ) ) + ( frame->lpBMIH )->biSize;
  memcpy( lpNewBM , frame->lpBMIH , ( frame->lpBMIH )->biSize );

  lpNewBM->biWidth = width;
  lpNewBM->biHeight = height;
  lpNewBM->biSizeImage = new_size;

  int y , x;

  LPBYTE noff , ooff , ooff_e , ooff1;

  for ( y = 0; y < oldheight; y++ )
  {
    noff = dst + 2 * width*y;
    ooff = src + oldwidth * y;
    ooff1 = src + ( ( y < oldheight - 1 ) ? oldwidth * ( y + 1 ) : oldwidth * ( y ) );
    ooff_e = ooff + oldwidth - 1; // Last point will be processed separatly
    while ( ooff < ooff_e )
    {
      *( noff ) = *( ooff );
      *( noff + 1 ) = ( *( ooff + 1 ) + *( ooff ) ) >> 1;
      *( noff + width ) = ( *( ooff1 ) +*( ooff ) ) >> 1;
      *( noff + width + 1 ) = ( *( ooff + 1 ) + *( ooff ) +*( ooff1 ) +*( ooff1 + 1 ) ) >> 2;
      ooff++;
      ooff1++;
      noff += 2;
    }
    *( noff ) = *( ooff );
    *( noff + 1 ) = *( ooff );
    *( noff + width ) = ( *( ooff1 ) +*( ooff ) ) >> 1;
    *( noff + width + 1 ) = ( *( ooff1 ) +*( ooff ) ) >> 1;
  }
  {
    LPBYTE _dst = dst + width * height;
    src += oldwidth * oldheight;
    int width2 = width >> 2;
    for ( y = 0; y < oldheight >> 1; y++ )
    {
      for ( x = 0; x < oldwidth >> 2; x++ )
      {
        *( _dst + width2 + 1 ) = *( _dst + width2 ) = *( _dst + 1 ) = *_dst = *src;
        _dst++; _dst++;
        src++;
      }
      _dst += width >> 2;
    }
  }
  if ( frame->lpData )
    free( frame->lpData );
  else
    free( frame->lpBMIH );

  frame->lpBMIH = lpNewBM;
  frame->lpData = NULL;

  return true;
}

__forceinline bool _enlarge_yuv12( pTVFrame frame )
{
  if ( !frame ) return false;
  if ( !frame->lpBMIH ) return false;

  ASSERT( frame->lpBMIH->biCompression == BI_YUV12 ); // Check format is supported

  long oldwidth = ( frame->lpBMIH )->biWidth , oldheight = ( frame->lpBMIH )->biHeight;
  LPBYTE src = ( LPBYTE ) GetData( frame );

  int width = ( oldwidth << 1 ) , height = ( oldheight << 1 ) , new_size = ( ( ( frame->lpBMIH )->biSizeImage ) << 2 );

  LPBITMAPINFOHEADER lpNewBM = ( LPBITMAPINFOHEADER ) malloc( new_size + ( frame->lpBMIH )->biSize );
  memset( lpNewBM , 0 , new_size + ( frame->lpBMIH )->biSize );

  LPBYTE dst = ( ( LPBYTE ) ( lpNewBM ) ) + ( frame->lpBMIH )->biSize;
  memcpy( lpNewBM , frame->lpBMIH , ( frame->lpBMIH )->biSize );

  lpNewBM->biWidth = width;
  lpNewBM->biHeight = height;
  lpNewBM->biSizeImage = new_size;

  int y , x;

  LPBYTE noff , ooff , ooff_e , ooff1;

  for ( y = 0; y < oldheight; y++ )
  {
    noff = dst + 2 * width*y;
    ooff = src + oldwidth * y;
    ooff1 = src + ( ( y < oldheight - 1 ) ? oldwidth * ( y + 1 ) : oldwidth * ( y ) );
    ooff_e = ooff + oldwidth - 1; // Last point will be processed separatly
    while ( ooff < ooff_e )
    {
      *( noff ) = *( ooff );
      *( noff + 1 ) = ( *( ooff + 1 ) + *( ooff ) ) >> 1;
      *( noff + width ) = ( *( ooff1 ) +*( ooff ) ) >> 1;
      *( noff + width + 1 ) = ( *( ooff + 1 ) + *( ooff ) +*( ooff1 ) +*( ooff1 + 1 ) ) >> 2;
      ooff++;
      ooff1++;
      noff += 2;
    }
    *( noff ) = *( ooff );
    *( noff + 1 ) = *( ooff );
    *( noff + width ) = ( *( ooff1 ) +*( ooff ) ) >> 1;
    *( noff + width + 1 ) = ( *( ooff1 ) +*( ooff ) ) >> 1;
  }
  {
    LPBYTE _dst = dst + width * height;
    src += oldwidth * oldheight;
    int width2 = width >> 1;
    for ( y = 0; y < oldheight; y++ )
    {
      for ( x = 0; x < oldwidth >> 1; x++ )
      {
        *( _dst + width2 + 1 ) = *( _dst + width2 ) = *( _dst + 1 ) = *_dst = *src;
        _dst++; _dst++;
        src++;
      }
      _dst += width >> 1;
    }
  }
  if ( frame->lpData )
    free( frame->lpData );
  else
    free( frame->lpBMIH );

  frame->lpBMIH = lpNewBM;
  frame->lpData = NULL;

  return true;
}


__forceinline bool _enlarge16( pTVFrame frame )
{
  if ( !frame ) return false;
  if ( !frame->lpBMIH ) return false;

  ASSERT( ( frame->lpBMIH->biCompression ) == BI_Y16 ); // Check format is supported

  long oldwidth = ( frame->lpBMIH )->biWidth , oldheight = ( frame->lpBMIH )->biHeight;
  LPWORD src = ( LPWORD ) GetData( frame );

  int width = ( oldwidth << 1 ) , height = ( oldheight << 1 ) , new_size = width * height;

  LPBITMAPINFOHEADER lpNewBM = ( LPBITMAPINFOHEADER ) malloc( new_size * sizeof( WORD ) + ( frame->lpBMIH )->biSize );

  memset( lpNewBM , 0 , new_size * sizeof( WORD ) + ( frame->lpBMIH )->biSize );
  LPWORD dst = ( LPWORD ) ( ( ( LPBYTE ) ( lpNewBM ) ) + ( frame->lpBMIH )->biSize );
  memcpy( lpNewBM , frame->lpBMIH , ( frame->lpBMIH )->biSize );
  lpNewBM->biWidth = width;
  lpNewBM->biHeight = height;
  lpNewBM->biSizeImage = new_size * sizeof( WORD );
  int y;

  LPWORD noff , ooff , ooff_e , ooff1;

  for ( y = 0; y < oldheight; y++ )
  {
    noff = dst + 2 * width*y;
    ooff = src + oldwidth * y;
    ooff1 = src + ( ( y < oldheight - 1 ) ? oldwidth * ( y + 1 ) : oldwidth * ( y ) );
    ooff_e = ooff + oldwidth - 1; // Last point will be processed separatly
    while ( ooff < ooff_e )
    {
      *( noff ) = *( ooff );
      *( noff + 1 ) = ( *( ooff + 1 ) + *( ooff ) ) >> 1;
      *( noff + width ) = ( *( ooff1 ) +*( ooff ) ) >> 1;
      *( noff + width + 1 ) = ( *( ooff + 1 ) + *( ooff ) +*( ooff1 ) +*( ooff1 + 1 ) ) >> 2;
      ooff++;
      ooff1++;
      noff += 2;
    }
    *( noff ) = *( ooff );
    *( noff + 1 ) = *( ooff );
    *( noff + width ) = ( *( ooff1 ) +*( ooff ) ) >> 1;
    *( noff + width + 1 ) = ( *( ooff1 ) +*( ooff ) ) >> 1;
  }
  if ( frame->lpData )
    free( frame->lpData );
  else
    free( frame->lpBMIH );

  frame->lpBMIH = lpNewBM;
  frame->lpData = NULL;

  return true;
}

__forceinline bool _enlarge( pTVFrame frame )
{
  switch ( frame->lpBMIH->biCompression )
  {
    case BI_YUV12:
      return _enlarge_yuv12( frame );
    case BI_YUV9:
      return _enlarge_yuv9( frame );
    case BI_Y8:
    case BI_Y800:
      return _enlarge8( frame );
    case BI_Y16:
      return _enlarge16( frame );
  }
  return false;
}

__forceinline LPBITMAPINFOHEADER _cdiminish8( const pTVFrame frame )
{
  if ( !frame ) return NULL;
  if ( !frame->lpBMIH ) return NULL;

  ASSERT( (frame->lpBMIH->biCompression == BI_Y8) || (frame->lpBMIH->biCompression == BI_Y800) ); // Check format is supported

  long oldwidth = ( frame->lpBMIH )->biWidth , oldheight = ( frame->lpBMIH )->biHeight;

//    ASSERT(9*oldwidth*oldheight/8==frame->lpBMIH->biSizeImage);
  LPBYTE src = GetData( frame );

  int width = ( oldwidth >> 1 ) , height = ( oldheight >> 1 );
  int new_size = width * height;

  LPBITMAPINFOHEADER lpNewBM = ( LPBITMAPINFOHEADER ) malloc( new_size + ( frame->lpBMIH )->biSize );
  ASSERT( lpNewBM != NULL );
  LPBYTE dst = ( ( LPBYTE ) ( lpNewBM ) ) + ( frame->lpBMIH )->biSize;
  memcpy( lpNewBM , frame->lpBMIH , ( frame->lpBMIH )->biSize );
  lpNewBM->biWidth = width;
  lpNewBM->biHeight = height;
  lpNewBM->biSizeImage = new_size;

  int y , x , nx , noff = 0 , ooff = 0;
  for ( y = 0; y < height; y++ )
  {
    for ( x = 0; x < width; x++ )
    {
      nx = x << 1;
      dst[ x + noff ] = ( src[ ooff + nx ] + src[ ooff + nx + 1 ] + src[ ooff + nx + oldwidth ] + src[ ooff + nx + oldwidth + 1 ] + 2 ) >> 2;
    }
    noff += width;
    ooff += 2 * oldwidth;
  }
  lpNewBM->biXPelsPerMeter *= 2;
  lpNewBM->biYPelsPerMeter *= 2;
  return lpNewBM;
}

__forceinline LPBITMAPINFOHEADER _cdiminish_yuv9( const pTVFrame frame )
{
  if ( !frame ) return NULL;
  if ( !frame->lpBMIH ) return NULL;

  ASSERT( frame->lpBMIH->biCompression == BI_YUV9 ); // Check format is supported

  long oldwidth = ( frame->lpBMIH )->biWidth , oldheight = ( frame->lpBMIH )->biHeight;

  LPBYTE src = GetData( frame );

  int width = ( oldwidth >> 1 ) , height = ( oldheight >> 1 );
  int new_size;

  width = ( ( width >> 2 ) << 2 );
  height = ( ( height >> 2 ) << 2 );
  new_size = width * height * 9 / 8;

  LPBITMAPINFOHEADER lpNewBM = ( LPBITMAPINFOHEADER ) malloc( new_size + ( frame->lpBMIH )->biSize );
  ASSERT( lpNewBM != NULL );
  LPBYTE dst = ( ( LPBYTE ) ( lpNewBM ) ) + ( frame->lpBMIH )->biSize;
  memcpy( lpNewBM , frame->lpBMIH , ( frame->lpBMIH )->biSize );
  lpNewBM->biWidth = width;
  lpNewBM->biHeight = height;
  lpNewBM->biSizeImage = new_size;

  int y , x , nx , noff = 0 , ooff = 0;
  for ( y = 0; y < height; y++ )
  {
    for ( x = 0; x < width; x++ )
    {
      nx = x << 1;
      dst[ x + noff ] = ( src[ ooff + nx ] + src[ ooff + nx + 1 ] + src[ ooff + nx + oldwidth ] + src[ ooff + nx + oldwidth + 1 ] + 2 ) >> 2;
    }
    noff += width;
    ooff += 2 * oldwidth;
  }
  {
    LPBYTE _dst = dst + width * height;
    src += oldwidth * oldheight;
    int owidth2 = oldwidth >> 2; if ( owidth2 % 2 ) owidth2++;
    for ( y = 0; y < height >> 1; y++ )
    {
      for ( x = 0; x < width >> 2; x++ )
      {
        *_dst = ( *src + *( src + 1 ) + *( src + owidth2 ) + *( src + 1 + owidth2 ) ) >> 2;
        _dst++; src++; src++;
      }
      src += owidth2;
    }
  }
  lpNewBM->biXPelsPerMeter *= 2;
  lpNewBM->biYPelsPerMeter *= 2;
  return lpNewBM;
}

__forceinline LPBITMAPINFOHEADER _cdiminish_yuv12( const pTVFrame frame )
{
  if ( !frame ) return NULL;
  if ( !frame->lpBMIH ) return NULL;

  ASSERT( frame->lpBMIH->biCompression == BI_YUV12 ); // Check format is supported

  long oldwidth = ( frame->lpBMIH )->biWidth , oldheight = ( frame->lpBMIH )->biHeight;

  LPBYTE src = GetData( frame );

  int width = ( oldwidth >> 1 ) , height = ( oldheight >> 1 );
  int new_size;

  if ( ( frame->lpBMIH->biCompression ) == BI_YUV12 )
  {
    width = ( ( width >> 1 ) << 1 );
    height = ( ( height >> 1 ) << 1 );
    new_size = width * height * 12 / 8;
  }

  LPBITMAPINFOHEADER lpNewBM = ( LPBITMAPINFOHEADER ) malloc( new_size + ( frame->lpBMIH )->biSize );
  ASSERT( lpNewBM != NULL );
  LPBYTE dst = ( ( LPBYTE ) ( lpNewBM ) ) + ( frame->lpBMIH )->biSize;
  memcpy( lpNewBM , frame->lpBMIH , ( frame->lpBMIH )->biSize );
  lpNewBM->biWidth = width;
  lpNewBM->biHeight = height;
  lpNewBM->biSizeImage = new_size;

  int y , x , nx , noff = 0 , ooff = 0;
  for ( y = 0; y < height; y++ )
  {
    for ( x = 0; x < width; x++ )
    {
      nx = x << 1;
      dst[ x + noff ] = ( src[ ooff + nx ] + src[ ooff + nx + 1 ] + src[ ooff + nx + oldwidth ] + src[ ooff + nx + oldwidth + 1 ] + 2 ) >> 2;
    }
    noff += width;
    ooff += 2 * oldwidth;
  }
  {
    LPBYTE _dst = dst + width * height;
    src += oldwidth * oldheight;
    int owidth2 = oldwidth >> 1; if ( owidth2 % 2 ) owidth2++;
    for ( y = 0; y < height; y++ )
    {
      for ( x = 0; x < width >> 1; x++ )
      {
        *_dst = ( *src + *( src + 1 ) + *( src + owidth2 ) + *( src + 1 + owidth2 ) ) >> 2;
        _dst++; src++; src++;
      }
      src += owidth2;
    }
  }
  lpNewBM->biXPelsPerMeter *= 2;
  lpNewBM->biYPelsPerMeter *= 2;
  return lpNewBM;
}

__forceinline LPBITMAPINFOHEADER _cdiminish16( pTVFrame frame )
{
  if ( !frame ) return NULL;
  if ( !frame->lpBMIH ) return NULL;

  ASSERT( ( frame->lpBMIH->biCompression ) == BI_Y16 ); // Check format is supported

  long oldwidth = ( frame->lpBMIH )->biWidth , oldheight = ( frame->lpBMIH )->biHeight;

  LPWORD src = ( LPWORD ) GetData( frame );

  int width = ( oldwidth >> 1 ) , height = ( oldheight >> 1 );

    // fit to 4x4
  width = ( ( width >> 2 ) << 2 );
  height = ( ( height >> 2 ) << 2 );

  int new_size = width * height;


  LPBITMAPINFOHEADER lpNewBM = ( LPBITMAPINFOHEADER ) malloc( new_size * sizeof( WORD ) + ( frame->lpBMIH )->biSize );
  ASSERT( lpNewBM != NULL );
  LPWORD dst = ( LPWORD ) ( ( ( LPBYTE ) ( lpNewBM ) ) + ( frame->lpBMIH )->biSize );
  memcpy( lpNewBM , frame->lpBMIH , ( frame->lpBMIH )->biSize );
  lpNewBM->biWidth = width;
  lpNewBM->biHeight = height;
  lpNewBM->biSizeImage = new_size * sizeof( WORD );

  int y , x , nx , noff = 0 , ooff = 0;
  for ( y = 0; y < height; y++ )
  {
    for ( x = 0; x < width; x++ )
    {
      nx = x << 1;
      dst[ x + noff ] = ( src[ ooff + nx ] + src[ ooff + nx + 1 ] + src[ ooff + nx + oldwidth ] + src[ ooff + nx + oldwidth + 1 ] + 2 ) >> 2;
    }
    noff += width;
    ooff += 2 * oldwidth;
  }
  return lpNewBM;
}

__forceinline bool _diminish( pTVFrame frame )
{
  switch ( frame->lpBMIH->biCompression )
  {
    case BI_YUV12:
    {
      LPBITMAPINFOHEADER lpNewBM = _cdiminish_yuv12( frame );
      if ( lpNewBM )
      {
        if ( frame->lpData )
          free( frame->lpData );
        else
          free( frame->lpBMIH );
        frame->lpBMIH = lpNewBM;
        frame->lpData = NULL;
        return true;
      }
      break;
    }
    case BI_YUV9:
    {
      LPBITMAPINFOHEADER lpNewBM = _cdiminish_yuv9( frame );
      if ( lpNewBM )
      {
        if ( frame->lpData )
          free( frame->lpData );
        else
          free( frame->lpBMIH );
        frame->lpBMIH = lpNewBM;
        frame->lpData = NULL;
        return true;
      }
      break;
    }
    case BI_Y8:
    case BI_Y800:
    {
      LPBITMAPINFOHEADER lpNewBM = _cdiminish8( frame );
      if ( lpNewBM )
      {
        if ( frame->lpData )
          free( frame->lpData );
        else
          free( frame->lpBMIH );
        frame->lpBMIH = lpNewBM;
        frame->lpData = NULL;
        return true;
      }
      break;
    }
    case BI_Y16:
    {
      LPBITMAPINFOHEADER lpNewBM = _cdiminish16( frame );
      if ( lpNewBM )
      {
        if ( frame->lpData )
          free( frame->lpData );
        else
          free( frame->lpBMIH );
        frame->lpBMIH = lpNewBM;
        frame->lpData = NULL;
        return true;
      }
      break;
    }
  }
  return false;
}


__forceinline bool _resampleY2( BITMAPINFOHEADER** lppBMIH )
{
  if ( !*lppBMIH ) return false;

  ASSERT( ( ( *lppBMIH )->biCompression ) == 0x39555659 ); // Check is it YUV9

  BITMAPINFOHEADER* oldBMIH = *lppBMIH;
  DWORD sizeImage = oldBMIH->biSizeImage * 2;
  BITMAPINFOHEADER* newBMIH = ( LPBITMAPINFOHEADER ) malloc( sizeImage + oldBMIH->biSize );
  if ( !newBMIH ) return false;
  memcpy( newBMIH , oldBMIH , oldBMIH->biSize );
  newBMIH->biHeight *= 2;
  newBMIH->biSizeImage *= 2;
  LPBYTE src , dst;
  src = ( LPBYTE ) oldBMIH + oldBMIH->biSize;
  dst = ( LPBYTE ) newBMIH + newBMIH->biSize;
  memset( dst , 128 , newBMIH->biSizeImage );
  memcpy( dst , src , oldBMIH->biWidth );
  dst += newBMIH->biWidth;
  memcpy( dst , src , oldBMIH->biWidth );
  src += oldBMIH->biWidth;
  dst += newBMIH->biWidth;

  for ( int y = 1; y < oldBMIH->biHeight; y++ )
  {
    for ( int x = 0; x < oldBMIH->biWidth; x++ )
    {
      *( dst + x ) = ( *( src + x ) + *( src + x - oldBMIH->biWidth ) ) / 2;
    }
    dst += newBMIH->biWidth;
    memcpy( dst , src , oldBMIH->biWidth );
    src += oldBMIH->biWidth;
    dst += newBMIH->biWidth;
  }
  *lppBMIH = newBMIH; free( oldBMIH );

  newBMIH->biYPelsPerMeter /= 2;

  return true;
}

__forceinline bool _diminishX8( pTVFrame frame )
{
  if ( !frame ) return false;
  if ( !frame->lpBMIH ) return false;

  long oldwidth = ( frame->lpBMIH )->biWidth , oldheight = ( frame->lpBMIH )->biHeight;

  LPBYTE src = GetData( frame );

  int width = ( oldwidth >> 1 ) , height = oldheight;
  width = ( ( width >> 2 ) << 2 );

  int new_size;

  if ( frame->lpBMIH->biCompression == BI_YUV9 )
    new_size = width * height * 9 / 8;
  else     if ( frame->lpBMIH->biCompression == BI_YUV12 )
    new_size = width * height * 12 / 8;
  else
    new_size = width * height;

  LPBITMAPINFOHEADER lpNewBM = ( LPBITMAPINFOHEADER ) malloc( new_size + ( frame->lpBMIH )->biSize );
  LPBYTE dst = ( ( LPBYTE ) ( lpNewBM ) ) + ( frame->lpBMIH )->biSize;
  memcpy( lpNewBM , frame->lpBMIH , ( frame->lpBMIH )->biSize );
  lpNewBM->biWidth = width;
  lpNewBM->biHeight = height;
  lpNewBM->biSizeImage = new_size;

  int y , x , nx , noff = 0 , ooff = 0;
  for ( y = 0; y < height; y++ )
  {
    for ( x = 0; x < width; x++ )
    {
      nx = x << 1;
      dst[ x + noff ] = ( src[ ooff + nx ] + src[ ooff + nx + 1 ] ) >> 1;
    }
    noff += width;
    ooff += oldwidth;
  }
  if ( frame->lpBMIH->biCompression == BI_YUV9 )
  {
    LPBYTE _dst = dst + width * height;
    src += oldwidth * oldheight;
    LPBYTE scanner = _dst;
    int YUVlength = width * height >> 3;
    while ( scanner < ( _dst + YUVlength ) )
    {
      *scanner = ( ( *src + *( src + 1 ) ) >> 1 );
      scanner++;
      src += 2;
    }
  }
  else if ( frame->lpBMIH->biCompression == BI_YUV12 )
  {
    LPBYTE _dst = dst + width * height;
    src += oldwidth * oldheight;
    LPBYTE scanner = _dst;
    int YUVlength = width * height >> 1;
    while ( scanner < ( _dst + YUVlength ) )
    {
      *scanner = ( ( *src + *( src + 1 ) ) >> 1 );
      scanner++;
      src += 2;
    }
  }

  if ( frame->lpData )
    free( frame->lpData );
  else
    free( frame->lpBMIH );
  lpNewBM->biXPelsPerMeter *= 2;
  frame->lpBMIH = lpNewBM;
  frame->lpData = NULL;
  return true;
}

__forceinline bool _diminishX16( pTVFrame frame )
{
  if ( !frame ) return false;
  if ( !frame->lpBMIH ) return false;

  long oldwidth = ( frame->lpBMIH )->biWidth , oldheight = ( frame->lpBMIH )->biHeight;

  LPWORD src = ( LPWORD ) GetData( frame );

  int width = ( oldwidth >> 1 ) , height = oldheight;
  width = ( ( width >> 2 ) << 2 );

  int new_size = width * height;

  LPBITMAPINFOHEADER lpNewBM = ( LPBITMAPINFOHEADER ) malloc( new_size * sizeof( WORD ) + ( frame->lpBMIH )->biSize );
  LPWORD dst = ( LPWORD ) ( ( ( LPBYTE ) ( lpNewBM ) ) + ( frame->lpBMIH )->biSize );
  memcpy( lpNewBM , frame->lpBMIH , ( frame->lpBMIH )->biSize );
  lpNewBM->biWidth = width;
  lpNewBM->biHeight = height;
  lpNewBM->biSizeImage = new_size * sizeof( WORD );

  int y , x , nx , noff = 0 , ooff = 0;
  for ( y = 0; y < height; y++ )
  {
    for ( x = 0; x < width; x++ )
    {
      nx = x << 1;
      dst[ x + noff ] = ( src[ ooff + nx ] + src[ ooff + nx + 1 ] ) >> 1;
    }
    noff += width;
    ooff += oldwidth;
  }

  if ( frame->lpData )
    free( frame->lpData );
  else
    free( frame->lpBMIH );
  lpNewBM->biXPelsPerMeter *= 2;
  frame->lpBMIH = lpNewBM;
  frame->lpData = NULL;
  return true;
}

__forceinline bool _diminishX( pTVFrame frame )
{
  switch ( frame->lpBMIH->biCompression )
  {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
    case BI_Y800:
    {
      return _diminishX8( frame );
    }
    case BI_Y16:
    {
      return _diminishX16( frame );
    }
  }
  return false;

}

__forceinline bool _enlargeY_YUV12( pTVFrame frame )
{
  if ( !frame ) return false;
  if ( !frame->lpBMIH ) return false;

  long oldwidth = ( frame->lpBMIH )->biWidth , oldheight = ( frame->lpBMIH )->biHeight;
  LPBYTE src = GetData( frame );

  int width = oldwidth , height = ( oldheight << 1 );
  int new_size;

  new_size = width * height * 12 / 8;

  LPBITMAPINFOHEADER lpNewBM = ( LPBITMAPINFOHEADER ) malloc( new_size + ( frame->lpBMIH )->biSize );
  LPBYTE dst = ( ( LPBYTE ) ( lpNewBM ) ) + ( frame->lpBMIH )->biSize;
  memcpy( lpNewBM , frame->lpBMIH , ( frame->lpBMIH )->biSize );
  lpNewBM->biWidth = width;
  lpNewBM->biHeight = height;
  lpNewBM->biSizeImage = new_size;
  int y , x;

  LPBYTE _src , _dst;
  _dst = dst;
  _src = src;


  for ( y = 0; y < oldheight; y++ )
  {
    memcpy( _dst , _src , oldwidth );
    _dst += oldwidth;
    for ( x = 0; x < oldwidth; x++ )
    {
      *_dst = ( *_src + *( _src + oldwidth ) ) / 2;
      _dst++; _src++;
    }
  }
  {
    _dst = dst + width * height;
    src += oldwidth * oldheight;
    LPBYTE eod = src + ( oldwidth*oldheight >> 1 );
    while ( src < eod )
    {
      memcpy( _dst , src , width );
      _dst += width;
      memcpy( _dst , src , width );
      _dst += width; src += width;
    }
  }

  if ( frame->lpData )
    free( frame->lpData );
  else
    free( frame->lpBMIH );

  frame->lpBMIH = lpNewBM;
  frame->lpData = NULL;

  lpNewBM->biYPelsPerMeter /= 2;

  return true;
}

__forceinline bool _enlargeY_YUV9( pTVFrame frame )
{
  if ( !frame ) return false;
  if ( !frame->lpBMIH ) return false;

  long oldwidth = ( frame->lpBMIH )->biWidth , oldheight = ( frame->lpBMIH )->biHeight;
  LPBYTE src = GetData( frame );

  int width = oldwidth , height = ( oldheight << 1 );
  int new_size;

  new_size = width * height * 9 / 8;

  LPBITMAPINFOHEADER lpNewBM = ( LPBITMAPINFOHEADER ) malloc( new_size + ( frame->lpBMIH )->biSize );
  LPBYTE dst = ( ( LPBYTE ) ( lpNewBM ) ) + ( frame->lpBMIH )->biSize;
  memcpy( lpNewBM , frame->lpBMIH , ( frame->lpBMIH )->biSize );
  lpNewBM->biWidth = width;
  lpNewBM->biHeight = height;
  lpNewBM->biSizeImage = new_size;
  int y , x;

  LPBYTE _src , _dst;
  _dst = dst;
  _src = src;


  for ( y = 0; y < oldheight; y++ )
  {
    memcpy( _dst , _src , oldwidth );
    _dst += oldwidth;
    for ( x = 0; x < oldwidth; x++ )
    {
      *_dst = ( *_src + *( _src + oldwidth ) ) / 2;
      _dst++; _src++;
    }
  }
  {
    _dst = dst + width * height;
    src += oldwidth * oldheight;
    LPBYTE eod = src + ( oldwidth*oldheight >> 3 );
    while ( src < eod )
    {
      memcpy( _dst , src , width );
      _dst += width;
      memcpy( _dst , src , width );
      _dst += width; src += width;
    }
  }

  if ( frame->lpData )
    free( frame->lpData );
  else
    free( frame->lpBMIH );

  frame->lpBMIH = lpNewBM;
  frame->lpData = NULL;

  lpNewBM->biYPelsPerMeter /= 2;

  return true;
}


__forceinline bool _enlargeY_Y8( pTVFrame frame )
{
  if ( !frame ) return false;
  if ( !frame->lpBMIH ) return false;

  long oldwidth = ( frame->lpBMIH )->biWidth , oldheight = ( frame->lpBMIH )->biHeight;
  LPBYTE src = GetData( frame );

  int width = oldwidth , height = ( oldheight << 1 );
  int new_size;

  new_size = width * height;

  LPBITMAPINFOHEADER lpNewBM = ( LPBITMAPINFOHEADER ) malloc( new_size + ( frame->lpBMIH )->biSize );
  LPBYTE dst = ( ( LPBYTE ) ( lpNewBM ) ) + ( frame->lpBMIH )->biSize;
  memcpy( lpNewBM , frame->lpBMIH , ( frame->lpBMIH )->biSize );
  lpNewBM->biWidth = width;
  lpNewBM->biHeight = height;
  lpNewBM->biSizeImage = new_size;
  int y , x;

  LPBYTE _src , _dst;
  _dst = dst;
  _src = src;


  for ( y = 0; y < oldheight; y++ )
  {
    memcpy( _dst , _src , oldwidth );
    _dst += oldwidth;
    for ( x = 0; x < oldwidth; x++ )
    {
      *_dst = ( *_src + *( _src + oldwidth ) ) / 2;
      _dst++; _src++;
    }
  }

  if ( frame->lpData )
    free( frame->lpData );
  else
    free( frame->lpBMIH );

  frame->lpBMIH = lpNewBM;
  frame->lpData = NULL;

  lpNewBM->biYPelsPerMeter /= 2;

  return true;
}

__forceinline bool _enlargeY_Y16( pTVFrame frame )
{
  if ( !frame ) return false;
  if ( !frame->lpBMIH ) return false;

  long oldwidth = ( frame->lpBMIH )->biWidth , oldheight = ( frame->lpBMIH )->biHeight;
  LPWORD src = ( LPWORD ) GetData( frame );

  int width = oldwidth , height = ( oldheight << 1 );
  int new_size = width * height;

  LPBITMAPINFOHEADER lpNewBM = ( LPBITMAPINFOHEADER ) malloc( new_size * sizeof( WORD ) + ( frame->lpBMIH )->biSize );
  LPWORD dst = ( LPWORD ) ( ( ( LPBYTE ) ( lpNewBM ) ) + ( frame->lpBMIH )->biSize );
  memcpy( lpNewBM , frame->lpBMIH , ( frame->lpBMIH )->biSize );
  lpNewBM->biWidth = width;
  lpNewBM->biHeight = height;
  lpNewBM->biSizeImage = new_size * sizeof( WORD );
  int y , x;

  LPWORD _src , _dst;
  _dst = dst;
  _src = src;


  for ( y = 0; y < oldheight; y++ )
  {
    memcpy( _dst , _src , oldwidth * sizeof( WORD ) );
    _dst += oldwidth;
    for ( x = 0; x < oldwidth; x++ )
    {
      *_dst = ( *_src + *( _src + oldwidth ) ) / 2;
      _dst++; _src++;
    }
  }
  if ( frame->lpData )
    free( frame->lpData );
  else
    free( frame->lpBMIH );

  frame->lpBMIH = lpNewBM;
  frame->lpData = NULL;

  lpNewBM->biYPelsPerMeter /= 2;

  return true;
}

__forceinline bool _enlargeY( pTVFrame frame )
{
  switch ( frame->lpBMIH->biCompression )
  {
    case BI_YUV12:
      return _enlargeY_YUV12( frame );
    case BI_YUV9:
      return _enlargeY_YUV9( frame );
    case BI_Y8:
    case BI_Y800:
      return _enlargeY_Y8( frame );
    case BI_Y16:
      return _enlargeY_Y16( frame );
  }
  return false;
}

__forceinline bool _restoreOrgSize( pTVFrame frame )
{
  if ( _get_anisotropy( frame ) == 2.0 )    return _enlargeY( frame );
  return true;
}

__forceinline bool _resampleRGB32( pTVFrame frame , int newWidth , int newHeight )
{
  newWidth = ( ( newWidth >> 2 ) << 2 );
  newHeight = ( ( newHeight >> 2 ) << 2 );

  long oldWidth = ( frame->lpBMIH )->biWidth , oldHeight = ( frame->lpBMIH )->biHeight;

  double xr = ( ( double ) oldWidth ) / newWidth;
  double yr = ( ( double ) oldHeight ) / newHeight;

  LPBYTE src = GetData( frame );

  int newSize = newWidth * newHeight * 4;

  LPBITMAPINFOHEADER dstBM = ( LPBITMAPINFOHEADER ) malloc( ( frame->lpBMIH )->biSize + newSize );
  LPBYTE dst = ( ( LPBYTE ) dstBM ) + ( frame->lpBMIH )->biSize;
  memset( dst , 0 , newSize );
  memcpy( dstBM , frame->lpBMIH , ( frame->lpBMIH )->biSize );

  dstBM->biWidth = newWidth;
  dstBM->biHeight = newHeight;
  for ( int y = 0; y < newHeight; y++ )
  {
    int x1 , y1;
    for ( int x = 0; x < newWidth; x++ )
    {

      int x0 = ( int ) floor( xr*x );
      int y0 = ( int ) floor( yr*y );
      int soff = ( x0 + y0 * oldWidth ) * 4;
      double errX = xr * x - floor( xr*x );
      double errY = yr * y - floor( yr*y );
      if ( ( errX == 0 ) && ( errY == 0 ) )
      {
        dst[ ( x + y * newWidth ) * 4 ] = src[ soff ];
        dst[ ( x + y * newWidth ) * 4 + 1 ] = src[ soff + 1 ];
        dst[ ( x + y * newWidth ) * 4 + 2 ] = src[ soff + 2 ];
        dst[ ( x + y * newWidth ) * 4 + 3 ] = src[ soff + 3 ];
      }
      else
      {
        x1 = x0 + 1;
        y1 = y0 + 1;
        double r0 = sqrt( 2.0 ) - sqrt( errX*errX + errY * errY );
        double r1 = sqrt( 2.0 ) - sqrt( ( 1.0 - errX )*( 1.0 - errX ) + errY * errY );
        double r2 = sqrt( 2.0 ) - sqrt( ( 1.0 - errX )*( 1.0 - errX ) + ( 1.0 - errY )*( 1.0 - errY ) );
        double r3 = sqrt( 2.0 ) - sqrt( errX*errX + ( 1.0 - errY )*( 1.0 - errY ) );
        dst[ ( x + y * newWidth ) * 4 ] = ( int ) ( ( r0*src[ ( x0 + y0 * oldWidth ) * 4 ] +
          r1 * src[ ( x1 + y0 * oldWidth ) * 4 ] +
          r2 * src[ ( x1 + y1 * oldWidth ) * 4 ] +
          r3 * src[ ( x0 + y1 * oldWidth ) * 4 ] ) / ( r0 + r1 + r2 + r3 ) );
        dst[ ( x + y * newWidth ) * 4 + 1 ] = ( int ) ( ( r0*src[ ( x0 + y0 * oldWidth ) * 4 + 1 ] +
          r1 * src[ ( x1 + y0 * oldWidth ) * 4 + 1 ] +
          r2 * src[ ( x1 + y1 * oldWidth ) * 4 + 1 ] +
          r3 * src[ ( x0 + y1 * oldWidth ) * 4 + 1 ] ) / ( r0 + r1 + r2 + r3 ) );
        dst[ ( x + y * newWidth ) * 4 + 2 ] = ( int ) ( ( r0*src[ ( x0 + y0 * oldWidth ) * 4 + 2 ] +
          r1 * src[ ( x1 + y0 * oldWidth ) * 4 + 2 ] +
          r2 * src[ ( x1 + y1 * oldWidth ) * 4 + 2 ] +
          r3 * src[ ( x0 + y1 * oldWidth ) * 4 + 2 ] ) / ( r0 + r1 + r2 + r3 ) );
        dst[ ( x + y * newWidth ) * 4 + 3 ] = ( int ) ( ( r0*src[ ( x0 + y0 * oldWidth ) * 4 + 3 ] +
          r1 * src[ ( x1 + y0 * oldWidth ) * 4 + 3 ] +
          r2 * src[ ( x1 + y1 * oldWidth ) * 4 + 3 ] +
          r3 * src[ ( x0 + y1 * oldWidth ) * 4 + 3 ] ) / ( r0 + r1 + r2 + r3 ) );

      }
    }
  }
  if ( frame->lpData )
    free( frame->lpData );
  else
    free( frame->lpBMIH );
  frame->lpBMIH = dstBM;
  frame->lpData = NULL;
  return true;
}

__forceinline bool _resampleRGB24( pTVFrame frame , int newWidth , int newHeight )
{
  ASSERT( FALSE ); // Not implemeneted yet
  return NULL;
}

__forceinline bool _resampleRGB8( pTVFrame frame , int newWidth , int newHeight )
{
  ASSERT( FALSE ); // Not implemeneted yet
  return NULL;
}

__forceinline bool _resampleRGB( pTVFrame frame , int newWidth , int newHeight )
{
  if ( !frame ) return false;
  if ( !frame->lpBMIH ) return false;

  ASSERT( ( frame->lpBMIH->biCompression ) == BI_RGB ); // Check format is supported
  switch ( frame->lpBMIH->biBitCount )
  {
    case 32:
      return _resampleRGB32( frame , newWidth , newHeight );
    case 24:
      return _resampleRGB24( frame , newWidth , newHeight );
    case 8:
      return _resampleRGB8( frame , newWidth , newHeight );
  }
  ASSERT( FALSE ); // Unexpected format;
  return NULL;
}

__forceinline bool _resampleYUV9( pTVFrame frame , int newWidth , int newHeight )
{
  if ( !frame ) return false;
  if ( !frame->lpBMIH ) return false;

  ASSERT( ( ( frame->lpBMIH->biCompression ) == BI_YUV9 ) || ( ( frame->lpBMIH->biCompression ) == BI_Y8 ) ); // Check format is supported

  newWidth = ( ( newWidth >> 2 ) << 2 );
  newHeight = ( ( newHeight >> 2 ) << 2 );

  long oldWidth = ( frame->lpBMIH )->biWidth , oldHeight = ( frame->lpBMIH )->biHeight;

  double xr = ( ( double ) oldWidth ) / newWidth;
  double yr = ( ( double ) oldHeight ) / newHeight;

  LPBYTE src = GetData( frame );

  int newSize = 9 * ( newWidth*newHeight / 8 );

  LPBITMAPINFOHEADER dstBM = ( LPBITMAPINFOHEADER ) malloc( ( frame->lpBMIH )->biSize + newSize );
  LPBYTE dst = ( ( LPBYTE ) dstBM ) + ( frame->lpBMIH )->biSize;
  memcpy( dstBM , frame->lpBMIH , ( frame->lpBMIH )->biSize );
  memset( dst , 128 , newSize );

  dstBM->biWidth = newWidth;
  dstBM->biHeight = newHeight;
  dstBM->biSizeImage = newSize;
  for ( int y = 0; y < newHeight; y++ )
  {
    int x1 , y1;
    for ( int x = 0; x < newWidth; x++ )
    {

      int x0 = ( int ) floor( xr*x );
      int y0 = ( int ) floor( yr*y );
      int soff = x0 + y0 * oldWidth;
      double errX = xr * x - floor( xr*x );
      double errY = yr * y - floor( yr*y );
      if ( ( errX == 0 ) && ( errY == 0 ) )
        dst[ x + y * newWidth ] = src[ soff ];
      else
      {
        x1 = x0 + 1;
        y1 = y0 + 1;
        double r0 = sqrt( 2.0 ) - sqrt( errX*errX + errY * errY );
        double r1 = sqrt( 2.0 ) - sqrt( ( 1.0 - errX )*( 1.0 - errX ) + errY * errY );
        double r2 = sqrt( 2.0 ) - sqrt( ( 1.0 - errX )*( 1.0 - errX ) + ( 1.0 - errY )*( 1.0 - errY ) );
        double r3 = sqrt( 2.0 ) - sqrt( errX*errX + ( 1.0 - errY )*( 1.0 - errY ) );
        dst[ x + y * newWidth ] = ( int ) ( ( r0*src[ x0 + y0 * oldWidth ] + r1 * src[ x1 + y0 * oldWidth ] +
          r2 * src[ x1 + y1 * oldWidth ] + r3 * src[ x0 + y1 * oldWidth ] ) / ( r0 + r1 + r2 + r3 ) );
      }
    }
  }
  int newCWidth , newCHeight , oldCWidth , oldCHeight;
  newCWidth = newWidth / 4;
  newCHeight = newHeight / 2;
  oldCWidth = oldWidth / 4;
  oldCHeight = oldHeight / 2;
  LPBYTE cdst = dst + newWidth * newHeight;
  LPBYTE csrc = src + oldWidth * oldHeight;
  for ( int y = 0; y < newCHeight; y++ )
  {
    int x1 , y1;
    for ( int x = 0; x < newCWidth; x++ )
    {

      int x0 = ( int ) floor( xr*x );
      int y0 = ( int ) floor( yr*y );
      int soff = x0 + y0 * oldCWidth;
      double errX = xr * x - floor( xr*x );
      double errY = yr * y - floor( yr*y );
      if ( ( errX == 0 ) && ( errY == 0 ) )
        cdst[ x + y * newCWidth ] = csrc[ soff ];
      else
      {
        x1 = x0 + 1;
        y1 = y0 + 1;
        double r0 = sqrt( 2.0 ) - sqrt( errX*errX + errY * errY );
        double r1 = sqrt( 2.0 ) - sqrt( ( 1.0 - errX )*( 1.0 - errX ) + errY * errY );
        double r2 = sqrt( 2.0 ) - sqrt( ( 1.0 - errX )*( 1.0 - errX ) + ( 1.0 - errY )*( 1.0 - errY ) );
        double r3 = sqrt( 2.0 ) - sqrt( errX*errX + ( 1.0 - errY )*( 1.0 - errY ) );
        cdst[ x + y * newCWidth ] = ( int ) ( ( r0*csrc[ x0 + y0 * oldCWidth ] + r1 * csrc[ x1 + y0 * oldCWidth ] +
          r2 * csrc[ x1 + y1 * oldCWidth ] + r3 * csrc[ x0 + y1 * oldCWidth ] ) / ( r0 + r1 + r2 + r3 ) );
      }
    }
  }
  if ( frame->lpData )
    free( frame->lpData );
  else
    free( frame->lpBMIH );
  frame->lpBMIH = dstBM;
  frame->lpData = NULL;
  return true;
}

__forceinline bool _resampleYUV12( pTVFrame frame , int newWidth , int newHeight )
{
  if ( !frame ) return false;
  if ( !frame->lpBMIH ) return false;

  ASSERT( ( frame->lpBMIH->biCompression ) == BI_YUV12 ); // Check format is supported

  newWidth = ( ( newWidth >> 2 ) << 2 );
  newHeight = ( ( newHeight >> 2 ) << 2 );

  long oldWidth = ( frame->lpBMIH )->biWidth , oldHeight = ( frame->lpBMIH )->biHeight;

  double xr = ( ( double ) oldWidth ) / newWidth;
  double yr = ( ( double ) oldHeight ) / newHeight;

  LPBYTE src = GetData( frame );

  int newSize = 12 * ( newWidth*newHeight / 8 );

  LPBITMAPINFOHEADER dstBM = ( LPBITMAPINFOHEADER ) malloc( ( frame->lpBMIH )->biSize + newSize );
  LPBYTE dst = ( ( LPBYTE ) dstBM ) + ( frame->lpBMIH )->biSize;
  memcpy( dstBM , frame->lpBMIH , ( frame->lpBMIH )->biSize );
  memset( dst , 128 , newSize );

  dstBM->biWidth = newWidth;
  dstBM->biHeight = newHeight;
  dstBM->biSizeImage = newSize;
  for ( int y = 0; y < newHeight; y++ )
  {
    int x1 , y1;
    for ( int x = 0; x < newWidth; x++ )
    {

      int x0 = ( int ) floor( xr*x );
      int y0 = ( int ) floor( yr*y );
      int soff = x0 + y0 * oldWidth;
      double errX = xr * x - floor( xr*x );
      double errY = yr * y - floor( yr*y );
      if ( ( errX == 0 ) && ( errY == 0 ) )
        dst[ x + y * newWidth ] = src[ soff ];
      else
      {
        x1 = x0 + 1;
        y1 = y0 + 1;
        double r0 = sqrt( 2.0 ) - sqrt( errX*errX + errY * errY );
        double r1 = sqrt( 2.0 ) - sqrt( ( 1.0 - errX )*( 1.0 - errX ) + errY * errY );
        double r2 = sqrt( 2.0 ) - sqrt( ( 1.0 - errX )*( 1.0 - errX ) + ( 1.0 - errY )*( 1.0 - errY ) );
        double r3 = sqrt( 2.0 ) - sqrt( errX*errX + ( 1.0 - errY )*( 1.0 - errY ) );
        dst[ x + y * newWidth ] = ( int ) ( ( r0*src[ x0 + y0 * oldWidth ] + r1 * src[ x1 + y0 * oldWidth ] +
          r2 * src[ x1 + y1 * oldWidth ] + r3 * src[ x0 + y1 * oldWidth ] ) / ( r0 + r1 + r2 + r3 ) );
      }
    }
  }
  int newCWidth , newCHeight , oldCWidth , oldCHeight;
  newCWidth = newWidth / 2;
  newCHeight = newHeight;
  oldCWidth = oldWidth / 2;
  oldCHeight = oldHeight;
  LPBYTE cdst = dst + newWidth * newHeight;
  LPBYTE csrc = src + oldWidth * oldHeight;
  for ( int y = 0; y < newCHeight; y++ )
  {
    int x1 , y1;
    for ( int x = 0; x < newCWidth; x++ )
    {

      int x0 = ( int ) floor( xr*x );
      int y0 = ( int ) floor( yr*y );
      int soff = x0 + y0 * oldCWidth;
      double errX = xr * x - floor( xr*x );
      double errY = yr * y - floor( yr*y );
      if ( ( errX == 0 ) && ( errY == 0 ) )
        cdst[ x + y * newCWidth ] = csrc[ soff ];
      else
      {
        x1 = x0 + 1;
        y1 = y0 + 1;
        double r0 = sqrt( 2.0 ) - sqrt( errX*errX + errY * errY );
        double r1 = sqrt( 2.0 ) - sqrt( ( 1.0 - errX )*( 1.0 - errX ) + errY * errY );
        double r2 = sqrt( 2.0 ) - sqrt( ( 1.0 - errX )*( 1.0 - errX ) + ( 1.0 - errY )*( 1.0 - errY ) );
        double r3 = sqrt( 2.0 ) - sqrt( errX*errX + ( 1.0 - errY )*( 1.0 - errY ) );
        cdst[ x + y * newCWidth ] = ( int ) ( ( r0*csrc[ x0 + y0 * oldCWidth ] + r1 * csrc[ x1 + y0 * oldCWidth ] +
          r2 * csrc[ x1 + y1 * oldCWidth ] + r3 * csrc[ x0 + y1 * oldCWidth ] ) / ( r0 + r1 + r2 + r3 ) );
      }
    }
  }
  //memset(cdst,0,newCWidth*newCHeight);
  if ( frame->lpData )
    free( frame->lpData );
  else
    free( frame->lpBMIH );
  frame->lpBMIH = dstBM;
  frame->lpData = NULL;
  return true;
}

__forceinline bool _resample8( pTVFrame frame , int newWidth , int newHeight )
{
  if ( !frame ) return false;
  if ( !frame->lpBMIH ) return false;

  ASSERT( ( ( frame->lpBMIH->biCompression ) == BI_YUV9 ) || ( ( frame->lpBMIH->biCompression ) == BI_Y8 )
    || ( ( frame->lpBMIH->biCompression ) == BI_Y800 ) ); // Check format is supported

  newWidth = ( ( newWidth >> 2 ) << 2 );
  newHeight = ( ( newHeight >> 2 ) << 2 );

  long oldWidth = ( frame->lpBMIH )->biWidth , oldHeight = ( frame->lpBMIH )->biHeight;

  double xr = ( ( double ) oldWidth ) / newWidth;
  double yr = ( ( double ) oldHeight ) / newHeight;

  LPBYTE src = GetData( frame );

  int newSize = newWidth * newHeight;

  LPBITMAPINFOHEADER dstBM = ( LPBITMAPINFOHEADER ) malloc( ( frame->lpBMIH )->biSize + newSize );
  LPBYTE dst = ( ( LPBYTE ) dstBM ) + ( frame->lpBMIH )->biSize;
  memcpy( dstBM , frame->lpBMIH , ( frame->lpBMIH )->biSize );

  dstBM->biWidth = newWidth;
  dstBM->biHeight = newHeight;
  dstBM->biSizeImage = newSize;
  for ( int y = 0; y < newHeight; y++ )
  {
    int x1 , y1;
    for ( int x = 0; x < newWidth; x++ )
    {

      int x0 = ( int ) floor( xr*x );
      int y0 = ( int ) floor( yr*y );
      int soff = x0 + y0 * oldWidth;
      double errX = xr * x - floor( xr*x );
      double errY = yr * y - floor( yr*y );
      if ( ( errX == 0 ) && ( errY == 0 ) )
        dst[ x + y * newWidth ] = src[ soff ];
      else
      {
        x1 = x0 + 1;
        y1 = y0 + 1;
        double r0 = sqrt( 2.0 ) - sqrt( errX*errX + errY * errY );
        double r1 = sqrt( 2.0 ) - sqrt( ( 1.0 - errX )*( 1.0 - errX ) + errY * errY );
        double r2 = sqrt( 2.0 ) - sqrt( ( 1.0 - errX )*( 1.0 - errX ) + ( 1.0 - errY )*( 1.0 - errY ) );
        double r3 = sqrt( 2.0 ) - sqrt( errX*errX + ( 1.0 - errY )*( 1.0 - errY ) );
        dst[ x + y * newWidth ] = ( int ) ( ( r0*src[ x0 + y0 * oldWidth ] + r1 * src[ x1 + y0 * oldWidth ] +
          r2 * src[ x1 + y1 * oldWidth ] + r3 * src[ x0 + y1 * oldWidth ] ) / ( r0 + r1 + r2 + r3 ) );
      }
    }
  }
  if ( frame->lpData )
    free( frame->lpData );
  else
    free( frame->lpBMIH );
  frame->lpBMIH = dstBM;
  frame->lpData = NULL;
  return true;
}


__forceinline bool _resample16( pTVFrame frame , int newWidth , int newHeight )
{
  if ( !frame ) return false;
  if ( !frame->lpBMIH ) return false;

  newWidth = ( ( newWidth >> 2 ) << 2 );
  newHeight = ( ( newHeight >> 2 ) << 2 );

  long oldWidth = ( frame->lpBMIH )->biWidth , oldHeight = ( frame->lpBMIH )->biHeight;

  double xr = ( ( double ) oldWidth ) / newWidth;
  double yr = ( ( double ) oldHeight ) / newHeight;

  LPWORD src = ( LPWORD ) GetData( frame );

  int newSize = newWidth * newHeight;

  LPBITMAPINFOHEADER dstBM = ( LPBITMAPINFOHEADER ) malloc( ( frame->lpBMIH )->biSize + newSize * sizeof( WORD ) );
  LPWORD dst = ( LPWORD ) ( ( ( LPBYTE ) dstBM ) + ( frame->lpBMIH )->biSize );
  memcpy( dstBM , frame->lpBMIH , ( frame->lpBMIH )->biSize );

  dstBM->biWidth = newWidth;
  dstBM->biHeight = newHeight;
  dstBM->biSizeImage = newSize * sizeof( WORD );
  for ( int y = 0; y < newHeight; y++ )
  {
    int x1 , y1;
    for ( int x = 0; x < newWidth; x++ )
    {

      int x0 = ( int ) floor( xr*x );
      int y0 = ( int ) floor( yr*y );
      int soff = x0 + y0 * oldWidth;
      double errX = xr * x - floor( xr*x );
      double errY = yr * y - floor( yr*y );
      if ( ( errX == 0 ) && ( errY == 0 ) )
        dst[ x + y * newWidth ] = src[ soff ];
      else
      {
        x1 = x0 + 1;
        y1 = y0 + 1;
        double r0 = sqrt( 2.0 ) - sqrt( errX*errX + errY * errY );
        double r1 = sqrt( 2.0 ) - sqrt( ( 1.0 - errX )*( 1.0 - errX ) + errY * errY );
        double r2 = sqrt( 2.0 ) - sqrt( ( 1.0 - errX )*( 1.0 - errX ) + ( 1.0 - errY )*( 1.0 - errY ) );
        double r3 = sqrt( 2.0 ) - sqrt( errX*errX + ( 1.0 - errY )*( 1.0 - errY ) );
        dst[ x + y * newWidth ] = ( int ) ( ( r0*src[ x0 + y0 * oldWidth ] + r1 * src[ x1 + y0 * oldWidth ] +
          r2 * src[ x1 + y1 * oldWidth ] + r3 * src[ x0 + y1 * oldWidth ] ) / ( r0 + r1 + r2 + r3 ) );
      }
    }
  }
  if ( frame->lpData )
    free( frame->lpData );
  else
    free( frame->lpBMIH );
  frame->lpBMIH = dstBM;
  frame->lpData = NULL;
  return true;
}

__forceinline bool _resample( pTVFrame frame , int newWidth , int newHeight )
{
  switch ( frame->lpBMIH->biCompression )
  {
    case BI_RGB:
      return _resampleRGB( frame , newWidth , newHeight );
    case BI_YUV9:
      return _resampleYUV9( frame , newWidth , newHeight );
    case BI_YUV12:
      return _resampleYUV12( frame , newWidth , newHeight );
    case BI_Y8:
    case BI_Y800:
      return _resample8( frame , newWidth , newHeight );
    case BI_Y16:
      return _resample16( frame , newWidth , newHeight );
      break;
  }
  return false;
}

__forceinline void redouble( LPBYTE data , int w , int h )
{
  LPBYTE tmp = ( LPBYTE ) malloc( w*h );
  memcpy( tmp , data , w*h );
  int x , y;
  for ( y = 0; y < h - 1; y++ )
  {
    for ( x = 0; x < w - 1; x++ )
    {
      *data = tmp[ y*w + x ];
      *( data + 1 ) = ( tmp[ y*w + x ] + tmp[ y*w + x + 1 ] ) / 2;
      *( data + 2 * w ) = ( tmp[ y*w + x ] + tmp[ ( y + 1 )*w + x ] ) / 2;
      *( data + 2 * w + 1 ) = ( tmp[ y*w + x ] + tmp[ y*w + x + 1 ] + tmp[ ( y + 1 )*w + x ] + tmp[ ( y + 1 )*w + x + 1 ] ) / 4;
      data += 2;
    }
    *data = tmp[ y*w + x ];
    *( data + 1 ) = tmp[ y*w + x ];
    *( data + 2 * w ) = ( tmp[ y*w + x ] + tmp[ ( y + 1 )*w + x ] ) / 2;
    *( data + 2 * w + 1 ) = ( tmp[ y*w + x ] + tmp[ ( y + 1 )*w + x ] ) / 2;
    data += 2;

    data += 2 * w;
  }
  for ( x = 0; x < w - 1; x++ )
  {
    *data = tmp[ y*w + x ];
    *( data + 1 ) = ( tmp[ y*w + x ] + tmp[ y*w + x + 1 ] ) / 2;
    *( data + 2 * w ) = tmp[ y*w + x ];
    *( data + 2 * w + 1 ) = ( tmp[ y*w + x ] + tmp[ y*w + x + 1 ] ) / 2;
    data += 2;
  }
  *data = tmp[ y*w + x ];
  *( data + 1 ) = tmp[ y*w + x ];
  *( data + 2 * w ) = tmp[ y*w + x ];
  *( data + 2 * w + 1 ) = tmp[ y*w + x ];

  free( tmp );
}

#endif _RESAMPLE_INC
//  $File : videologic.h - simple video logic functions
//  (C) Copyright The File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)

#ifndef VIDEOLOGIC_INC
#define VIDEOLOGIC_INC

#include <math.h>
#include <imageproc\basedef.h>
#include <imageproc\simpleip.h>

#define max5(a,b,c,d,e)		max(max(max(a,b),max(c,d)),e)

__forceinline void _nearest_neighbours( LPBYTE outD , LPBYTE inD , int width , int depth )
{
  int iX , j;
  for ( iX = 1; iX < width - 1; iX++ )
  {
    for ( j = 1; j < depth - 1; j++ )
    {
      outD[ iX + width * j ] = ( inD[ iX - 1 + width * j ] + inD[ iX + 1 + width * j ] + inD[ iX + width * j - width ] + inD[ iX + width * j + width ] ) >> 2;
    }
  }
}

__forceinline void _videoXORB( LPBYTE outD , LPBYTE inD , int size )
{
  LPBYTE eod = outD + size;
  while ( outD < eod )
  {
    *outD = ( ( *outD < 128 ) ^ ( *inD < 128 ) ) ? 255 : 0;
    outD++; inD++;
  }

}

__forceinline void _videoXORB16( LPWORD outD , LPWORD inD , int size )
{
  LPWORD eod = outD + size;
  while ( outD < eod )
  {
    *outD = ( ( *outD < 32768 ) ^ ( *inD < 32768 ) ) ? 65535 : 0;
    outD++; inD++;
  }

}

__forceinline void _videoXOR( LPBYTE outD , LPBYTE inD , int size )
{
  LPBYTE eod = outD + size;
  while ( outD < eod )
  {
    *outD = abs( *outD - *inD );
    outD++; inD++;
  }
}

__forceinline void _videoXOR16( LPWORD outD , LPWORD inD , int size )
{
  LPWORD eod = outD + size;
  while ( outD < eod )
  {
    *outD = abs( *outD - *inD );
    outD++; inD++;
  }
}

__forceinline void _videoAND( LPBYTE outD , LPBYTE inD , int size )
{
  LPBYTE eod = outD + size;
  while ( outD < eod )
  {
    *outD = ( *outD < *inD ) ? *outD : *inD;
    outD++; inD++;
  }
}

__forceinline void _videoAND16( LPWORD outD , LPWORD inD , int size )
{
  LPWORD eod = outD + size;
  while ( outD < eod )
  {
    *outD = ( *outD < *inD ) ? *outD : *inD;
    outD++; inD++;
  }
}
__forceinline void _videoMask( LPBYTE outD , LPBYTE inD , int size )
{
  LPBYTE eod = outD + size;
  while ( outD < eod )
  {
    if ( *( inD++ ) == 0 )
      *outD = 0 ;
    outD++;
  }
}

__forceinline void _videoMask16( LPWORD outD , LPWORD inD , int size )
{
  LPWORD eod = outD + size;
  while ( outD < eod )
  {
    if ( *( inD++ ) == 0 )
      *outD = 0 ;
    outD++;
  }
}

__forceinline void _videoANDB( LPBYTE outD , LPBYTE inD , int size )
{
  LPBYTE eod = outD + size;
  while ( outD < eod )
  {
    *outD = ( ( *outD > 128 ) && ( *inD > 128 ) ) ? 255 : 0;
    outD++; inD++;
  }
}

__forceinline void _videoANDB16( LPWORD outD , LPWORD inD , int size )
{
  LPWORD eod = outD + size;
  while ( outD < eod )
  {
    *outD = ( ( *outD > 32768 ) && ( *inD > 32768 ) ) ? 65535 : 0;
    outD++; inD++;
  }
}

__forceinline void _videoOR( LPBYTE outD , LPBYTE inD , int size )
{
  LPBYTE eod = outD + size;
  while ( outD < eod )
  {
    *outD = ( *outD > *inD ) ? *outD : *inD;
    outD++; inD++;
  }

}

__forceinline void _videoOR16( LPWORD outD , LPWORD inD , int size )
{
  LPWORD eod = outD + size;
  while ( outD < eod )
  {
    *outD = ( *outD > *inD ) ? *outD : *inD;
    outD++; inD++;
  }

}

__forceinline void _videoORB( LPBYTE outD , LPBYTE inD , int size )
{
  LPBYTE eod = outD + size;
  while ( outD < eod )
  {
    *outD = ( ( *outD > 128 ) || ( *inD > 128 ) ) ? 255 : 0;
    outD++; inD++;
  }
}

__forceinline void _videoORB16( LPWORD outD , LPWORD inD , int size )
{
  LPWORD eod = outD + size;
  while ( outD < eod )
  {
    *outD = ( ( *outD > 32768 ) || ( *inD > 32768 ) ) ? 65535 : 0;
    outD++; inD++;
  }
}

__forceinline void _videoNot( LPBYTE outD , LPBYTE inD , int size )
{
  LPBYTE eod = outD + size;
  while ( outD < eod )
  {
    *outD = 255 - *inD;
    outD++; inD++;
  }
}


__forceinline void _videoEQ( LPBYTE U , LPBYTE V , int size )
{
  LPBYTE eod = U + size;
  while ( U < eod )
  {
    *U = ( *U == *V ) ? 255 : 0;
    U++; V++;
  }

}

__forceinline void _videoEQ16( LPWORD U , LPWORD V , int size )
{
  LPWORD eod = U + size;
  while ( U < eod )
  {
    *U = ( *U == *V ) ? 65535 : 0;
    U++; V++;
  }
}


__forceinline void _videoADD( LPBYTE U , LPBYTE V , int size )
{
  LPBYTE eod = U + size;
  while ( U < eod )
  {
    *U = ( *U + *V ) / 2;
    U++; V++;
  }

}

__forceinline void _videoSUB( LPBYTE U , LPBYTE V , int size )
{
  LPBYTE eod = U + size;
  while ( U < eod )
  {
    *U = ( *U - *V ) / 2 + 128;
    U++; V++;
  }

}

__forceinline void _videoSUB16( LPWORD U , LPWORD V , int size )
{
  LPWORD eod = U + size;
  while ( U < eod )
  {
    *U = ( *U - *V ) / 2 + 32768;
    U++; V++;
  }

}

__forceinline void _videoMULT( LPBYTE U , LPBYTE V , int size )
{
  LPBYTE eod = U + size;
  while ( U < eod )
  {
    *U = ( unsigned char ) sqrt( ( double ) ( *U ) * ( *V ) );
    U++; V++;
  }

}

__forceinline void _videoDIV( LPBYTE U , LPBYTE V , int size )
{
  LPBYTE eod = U + size;
  while ( U < eod )
  {
    double res = sqrt( ( double ) ( *U ) / ( ( *V ) + 1 ) );
    res = ( res > 1.0 ) ? 1.0 : res;
    *U = ( unsigned char ) ( 255 * res );
    U++; V++;
  }

}

__forceinline void _videoBIN( LPBYTE Data , int size )
{
  int sum = 0;
  LPBYTE pData = Data;
  LPBYTE eod = Data + size;
  while ( pData < eod )
  {
    sum += *pData;
    pData++;
  }
  sum /= ( size / 2 );
  pData = Data;
  while ( pData < eod )
  {
    *pData = ( *pData > sum ) ? 255 : 0;
    pData++;
  }
}

__forceinline void _videoAND_MSK( LPBYTE outD , LPBYTE inD , int width , int depth )
{
  int i , j;
  for ( i = 1; i < width - 1; i++ )
    for ( j = 1; j < depth - 1; j++ )
    {
      outD[ i + width * j ] = ( inD[ i / 2 + ( width / 2 )*( j / 2 ) ] && outD[ i + width * j ] ) ? 255 : 0;
    }
}

__forceinline void _remove_points( long width , long depth , LPBYTE data )
{

  LPBYTE tmp1 = ( LPBYTE ) malloc( width*depth );
  LPBYTE tmp2 = ( LPBYTE ) malloc( width*depth );

  _nearest_neighbours( tmp1 , data , width , depth );
  _simplebinarize8( tmp1 , width*depth , 250 );
  _videoXOR( data , tmp1 , width*depth );
  _videoNot( tmp1 , data , width*depth );
  _nearest_neighbours( tmp2 , tmp1 , width , depth );
  _simplebinarize8( tmp2 , width*depth , 250 );
  _videoXOR( tmp2 , tmp1 , width*depth );
  _videoNot( tmp2 , tmp2 , width*depth );
  _videoAND( data , tmp2 , width*depth );
  free( tmp1 ); free( tmp2 );
}

__forceinline void _s_remove_points( long width , long depth , LPBYTE data )
{
  LPBYTE tmp1 = ( LPBYTE ) malloc( width*depth );
  _nearest_neighbours( tmp1 , data , width , depth );
  LPBYTE dst = data + 1;
  LPBYTE eod = data + width * depth - 2;
  LPBYTE src = tmp1;
  while ( dst < eod )
  {
    if ( *src )
    {
      *dst = ( *src == 255 ) ? 0 : 255;
    }
    dst++; src++;
  }
  free( tmp1 );
}

__forceinline void _erode( int width , int depth , LPBYTE dst , LPBYTE src )
{
#define AREA 1
  if ( ( !src ) || ( !dst ) ) 
    return;
  int offset = 0 ;
  int minX , maxX , minY , maxY , Val;
  for ( int iY = 0; iY < depth; iY ++ )
  {
    minY = iY  - AREA; 
    if ( minY < 0 )  
      minY = 0;
    maxY = iY  + AREA; 
    if ( maxY >= depth ) 
      maxY = depth - 1;
    for ( int iX = 0; iX < width; iX++ )
    {
      minX = iX - AREA; 
      if ( minX < 0 )  
        minX = 0;
      maxX = iX + AREA; 
      if ( maxX >= width ) 
        maxX = width - 1;
      int r = 0;
      Val = 0;
      for ( int y = minY; y <= maxY; y++ )
      {
        for ( int x = minX; x <= maxX; x++ )
        {
          if ( Val < src[ x + y * width ] ) 
            Val = src[ x + y * width ];
        }
      }
      dst[ offset + iX ] = Val;
    }
    offset += width;
  }
#undef AREA
}

__forceinline void _erode16( int width , int depth , LPWORD dst , LPWORD src )
{
#define AREA 1
  if ( ( !src ) || ( !dst ) ) return;
  int offset = 0 , iY ;
  int minX , maxX , minY , maxY , Val;
  for ( iY  = 0; iY  < depth; iY ++ )
  {
    minY = iY  - AREA; if ( minY < 0 )  minY = 0;
    maxY = iY  + AREA; if ( maxY >= depth ) maxY = depth - 1;
    for ( int iX = 0; iX < width; iX++ )
    {
      minX = iX - AREA; if ( minX < 0 )  minX = 0;
      maxX = iX + AREA; if ( maxX >= width ) maxX = width - 1;
      int r = 0;
      Val = 0;
      for ( int y = minY; y <= maxY; y++ )
      {
        for ( int x = minX; x <= maxX; x++ )
        {
          if ( Val < src[ x + y * width ] ) Val = src[ x + y * width ];
        }
      }
      dst[ offset + iX ] = Val;
    }
    offset += width;
  }
#undef AREA
}

__forceinline bool _erode( pTVFrame frame )
{
  switch ( frame->lpBMIH->biCompression )
  {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
    case BI_Y800:
    {
      int imgsize = frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
      LPBYTE dst = ( LPBYTE ) malloc( imgsize );
      _erode( frame->lpBMIH->biWidth , frame->lpBMIH->biHeight , dst , GetData( frame ) );
      memcpy( GetData( frame ) , dst , imgsize );
      free( dst );
      return true;
    }
    case BI_Y16:
    {
      int imgsize = frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
      LPWORD dst = ( LPWORD ) malloc( imgsize * sizeof( WORD ) );
      _erode16( frame->lpBMIH->biWidth , frame->lpBMIH->biHeight , dst , ( LPWORD ) GetData( frame ) );
      memcpy( GetData( frame ) , dst , imgsize * sizeof( WORD ) );
      free( dst );
      return true;
    }
  }
  return false;
}

__forceinline void _dilate( int width , int depth , LPBYTE dst , LPBYTE src , BYTE iNInversed = 1 )
{
#define AREA 1
  if ( ( !src ) || ( !dst ) )
    return;
  int offset = 0 , iY ;
  int minX , maxX , minY , maxY/*,Val*/;
  for ( iY  = 0; iY  < depth; iY ++ )
  {
    minY = iY  - AREA;

    if ( minY < 0 )
      minY = 0;
    maxY = iY  + AREA;
    if ( maxY >= depth )
      maxY = depth - 1;
    for ( int iX = 0; iX < width; iX++ )
    {
      minX = iX - AREA;
      if ( minX < 0 )
        minX = 0;
      maxX = iX + AREA;
      if ( maxX >= width )
        maxX = width - 1;
      int r = 0;
//       Val=255;
      for ( int y = minY; y <= maxY && r < iNInversed ; y++ )
      {
        for ( int x = minX; x <= maxX && r < iNInversed ; x++ )
          r += ( src[ x + y * width ] < 255 ) ;
      }
      dst[ offset + iX ] = ( r < iNInversed ) ? 255 : 0 ;
    }
    offset += width;
  }
#undef AREA
}

__forceinline void _dilate16( int width , int depth , LPWORD dst , LPWORD src , WORD iNInversed = 1 )
{
#define AREA 1
  if ( ( !src ) || ( !dst ) ) return;
  int offset = 0 , iY ;
  int minX , maxX , minY , maxY/*,Val*/;
  for ( iY  = 0; iY  < depth; iY ++ )
  {
    minY = iY  - AREA; if ( minY < 0 )  minY = 0;
    maxY = iY  + AREA; if ( maxY >= depth ) maxY = depth - 1;
    for ( int iX = 0; iX < width; iX++ )
    {
      minX = iX - AREA; if ( minX < 0 )  minX = 0;
      maxX = iX + AREA; if ( maxX >= width ) maxX = width - 1;
      int r = 0;
//       Val=65535;
//       for (int y=minY; y<=maxY; y++)
//       {
//         for (int x=minX; x<=maxX; x++)
//         {
//           if (Val>src[x+y*width]) Val=src[x+y*width];
//         }
//       }
//       dst[offset+iX]=Val;
      for ( int y = minY; y <= maxY && r < iNInversed ; y++ )
      {
        for ( int x = minX; x <= maxX && r < iNInversed ; x++ )
          r += ( src[ x + y * width ] < 65535 ) ;
      }
      dst[ offset + iX ] = ( r < iNInversed ) ? 255 : 0 ;

    }
    offset += width;
  }
#undef AREA
}

__forceinline bool _dilate( pTVFrame frame , int iNInversed = 1 )
{
  switch ( frame->lpBMIH->biCompression )
  {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
    case BI_Y800:
    {
      int imgsize = frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
      LPBYTE dst = ( LPBYTE ) malloc( imgsize );
      _dilate( frame->lpBMIH->biWidth , frame->lpBMIH->biHeight , dst , GetData( frame ) , iNInversed );
      memcpy( GetData( frame ) , dst , imgsize );
      free( dst );
      return true;
    }
    case BI_Y16:
    {
      int imgsize = frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
      LPWORD dst = ( LPWORD ) malloc( imgsize * sizeof( WORD ) );
      _dilate16( frame->lpBMIH->biWidth , frame->lpBMIH->biHeight , dst , ( LPWORD ) GetData( frame ) , iNInversed );
      memcpy( GetData( frame ) , dst , imgsize * sizeof( WORD ) );
      free( dst );
      return true;
    }
  }
  return false;
}

__forceinline void _erodeh( int width , int depth , LPBYTE dst , LPBYTE src )
{
#define AREA 1
  if ( ( !src ) || ( !dst ) ) return;
  int offset = 0 , iY ;
  int minX , maxX , minY , maxY , Val = 0;
  for ( iY  = 0; iY  < depth; iY ++ )
  {
    minY = iY  - AREA; if ( minY < 0 )  minY = 0;
    maxY = iY  + AREA; if ( maxY >= depth ) maxY = depth - 1;
    for ( int iX = 0; iX < width; iX++ )
    {
      minX = iX - AREA; if ( minX < 0 )  minX = 0;
      maxX = iX + AREA; if ( maxX >= width ) maxX = width - 1;
      int r = 0; Val = 0; int x;
      int yoffset = iY  * width;
      for ( x = minX; x <= maxX; x++ )
      {
        if ( Val < src[ x + yoffset ] ) Val = src[ x + yoffset ];
      }
      dst[ offset + iX ] = Val; //(ValL+ValR)/2;
    }
    offset += width;
  }
#undef AREA
}

__forceinline void _erodeh16( int width , int depth , LPWORD dst , LPWORD src )
{
#define AREA 1
  if ( ( !src ) || ( !dst ) ) return;
  int offset = 0 , iY ;
  int minX , maxX , minY , maxY , Val = 0;
  for ( iY  = 0; iY  < depth; iY ++ )
  {
    minY = iY  - AREA; if ( minY < 0 )  minY = 0;
    maxY = iY  + AREA; if ( maxY >= depth ) maxY = depth - 1;
    for ( int iX = 0; iX < width; iX++ )
    {
      minX = iX - AREA; if ( minX < 0 )  minX = 0;
      maxX = iX + AREA; if ( maxX >= width ) maxX = width - 1;
      int r = 0; Val = 0; int x;
      int yoffset = iY  * width;
      for ( x = minX; x <= maxX; x++ )
      {
        if ( Val < src[ x + yoffset ] ) Val = src[ x + yoffset ];
      }
      dst[ offset + iX ] = Val; //(ValL+ValR)/2;
    }
    offset += width;
  }
#undef AREA
}

__forceinline bool _erodeh( pTVFrame frame )
{
  switch ( frame->lpBMIH->biCompression )
  {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
    case BI_Y800:
    {
      int imgsize = frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
      LPBYTE dst = ( LPBYTE ) malloc( imgsize );
      _erodeh( frame->lpBMIH->biWidth , frame->lpBMIH->biHeight , dst , GetData( frame ) );
      memcpy( GetData( frame ) , dst , imgsize );
      free( dst );
      return true;
    }
    case BI_Y16:
    {
      int imgsize = frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
      LPWORD dst = ( LPWORD ) malloc( imgsize * sizeof( WORD ) );
      _erodeh16( frame->lpBMIH->biWidth , frame->lpBMIH->biHeight , dst , ( LPWORD ) GetData( frame ) );
      memcpy( GetData( frame ) , dst , imgsize * sizeof( WORD ) );
      free( dst );
      return true;
    }
  }
  return false;
}

__forceinline void _dilateh( int width , int depth , LPBYTE dst , LPBYTE src )
{
#define AREA 1
  if ( ( !src ) || ( !dst ) ) return;
  int offset = 0 , iY ;
  int minX , maxX , minY , maxY , Val;
  for ( iY  = 0; iY  < depth; iY ++ )
  {
    minY = iY  - AREA; if ( minY < 0 )  minY = 0;
    maxY = iY  + AREA; if ( maxY >= depth ) maxY = depth - 1;
    for ( int iX = 0; iX < width; iX++ )
    {
      minX = iX - AREA; if ( minX < 0 )  minX = 0;
      maxX = iX + AREA; if ( maxX >= width ) maxX = width - 1;
      int r = 0; Val = 255; int x;
      int yoffset = iY  * width;
      for ( x = minX; x <= maxX; x++ )
      {
        if ( Val > src[ x + yoffset ] ) Val = src[ x + yoffset ];
      }
      dst[ offset + iX ] = Val;
    }
    offset += width;
  }
#undef AREA
}

__forceinline void _dilateh16( int width , int depth , LPWORD dst , LPWORD src )
{
#define AREA 1
  if ( ( !src ) || ( !dst ) ) return;
  int offset = 0 , iY ;
  int minX , maxX , minY , maxY , Val;
  for ( iY  = 0; iY  < depth; iY ++ )
  {
    minY = iY  - AREA; if ( minY < 0 )  minY = 0;
    maxY = iY  + AREA; if ( maxY >= depth ) maxY = depth - 1;
    for ( int iX = 0; iX < width; iX++ )
    {
      minX = iX - AREA; if ( minX < 0 )  minX = 0;
      maxX = iX + AREA; if ( maxX >= width ) maxX = width - 1;
      int r = 0; Val = 65535; int x;
      int yoffset = iY  * width;
      for ( x = minX; x <= maxX; x++ )
      {
        if ( Val > src[ x + yoffset ] ) Val = src[ x + yoffset ];
      }
      dst[ offset + iX ] = Val;
    }
    offset += width;
  }
#undef AREA
}

__forceinline bool _dilateh( pTVFrame frame )
{
  switch ( frame->lpBMIH->biCompression )
  {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
    case BI_Y800:
    {
      int imgsize = frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
      LPBYTE dst = ( LPBYTE ) malloc( imgsize );
      _dilateh( frame->lpBMIH->biWidth , frame->lpBMIH->biHeight , dst , GetData( frame ) );
      memcpy( GetData( frame ) , dst , imgsize );
      free( dst );
      return true;
    }
    case BI_Y16:
    {
      int imgsize = frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
      LPWORD dst = ( LPWORD ) malloc( imgsize * sizeof( WORD ) );
      _dilateh16( frame->lpBMIH->biWidth , frame->lpBMIH->biHeight , dst , ( LPWORD ) GetData( frame ) );
      memcpy( GetData( frame ) , dst , imgsize * sizeof( WORD ) );
      free( dst );
      return true;
    }
  }
  return false;
}

__forceinline void _erodev( int width , int depth , LPBYTE dst , LPBYTE src )
{
#define AREA 1
  if ( ( !src ) || ( !dst ) ) return;
  int offset = 0 , iY ;
  int minX , maxX , minY , maxY , Val = 0;
  for ( iY  = 0; iY  < depth; iY ++ )
  {
    minY = iY  - AREA; if ( minY < 0 )  minY = 0;
    maxY = iY  + AREA; if ( maxY >= depth ) maxY = depth - 1;
    for ( int iX = 0; iX < width; iX++ )
    {
      minX = iX - AREA; if ( minX < 0 )  minX = 0;
      maxX = iX + AREA; if ( maxX >= width ) maxX = width - 1;
      int r = 0; Val = 0;
      for ( int y = minY; y <= maxY; y++ )
      {
        if ( Val < src[ iX + y * width ] ) Val = src[ iX + y * width ];
      }
      dst[ offset + iX ] = Val; //(ValL+ValR)/2;
    }
    offset += width;
  }
#undef AREA
}

__forceinline void _erodev16( int width , int depth , LPWORD dst , LPWORD src )
{
#define AREA 1
  if ( ( !src ) || ( !dst ) ) return;
  int offset = 0 , iY ;
  int minX , maxX , minY , maxY , Val = 0;
  for ( iY  = 0; iY  < depth; iY ++ )
  {
    minY = iY  - AREA; if ( minY < 0 )  minY = 0;
    maxY = iY  + AREA; if ( maxY >= depth ) maxY = depth - 1;
    for ( int iX = 0; iX < width; iX++ )
    {
      minX = iX - AREA; if ( minX < 0 )  minX = 0;
      maxX = iX + AREA; if ( maxX >= width ) maxX = width - 1;
      int r = 0; Val = 0;
      for ( int y = minY; y <= maxY; y++ )
      {
        if ( Val < src[ iX + y * width ] ) Val = src[ iX + y * width ];
      }
      dst[ offset + iX ] = Val; //(ValL+ValR)/2;
    }
    offset += width;
  }
#undef AREA
}

__forceinline bool _erodev( pTVFrame frame )
{
  switch ( frame->lpBMIH->biCompression )
  {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
    case BI_Y800:
    {
      int imgsize = frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
      LPBYTE dst = ( LPBYTE ) malloc( imgsize );
      _erodev( frame->lpBMIH->biWidth , frame->lpBMIH->biHeight , dst , GetData( frame ) );
      memcpy( GetData( frame ) , dst , imgsize );
      free( dst );
      return true;
    }
    case BI_Y16:
    {
      int imgsize = frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
      LPWORD dst = ( LPWORD ) malloc( imgsize * sizeof( WORD ) );
      _erodev16( frame->lpBMIH->biWidth , frame->lpBMIH->biHeight , dst , ( LPWORD ) GetData( frame ) );
      memcpy( GetData( frame ) , dst , imgsize * sizeof( WORD ) );
      free( dst );
      return true;
    }
  }
  return false;
}

__forceinline void _dilatev( int width , int depth , LPBYTE dst , LPBYTE src )
{
#define AREA 1
  if ( ( !src ) || ( !dst ) ) return;
  int offset = 0 , iY ;
  int minX , maxX , minY , maxY , Val;
  for ( iY  = 0; iY  < depth; iY ++ )
  {
    minY = iY  - AREA; if ( minY < 0 )  minY = 0;
    maxY = iY  + AREA; if ( maxY >= depth ) maxY = depth - 1;
    for ( int iX = 0; iX < width; iX++ )
    {
      minX = iX - AREA; if ( minX < 0 )  minX = 0;
      maxX = iX + AREA; if ( maxX >= width ) maxX = width - 1;
      int r = 0; Val = 255;
      for ( int y = minY; y <= maxY; y++ )
      {
        if ( Val > src[ iX + y * width ] ) Val = src[ iX + y * width ];
      }
      dst[ offset + iX ] = Val;
    }
    offset += width;
  }
#undef AREA
}

__forceinline void _dilatev16( int width , int depth , LPWORD dst , LPWORD src )
{
#define AREA 1
  if ( ( !src ) || ( !dst ) ) return;
  int offset = 0 , iY ;
  int minX , maxX , minY , maxY , Val;
  for ( iY  = 0; iY  < depth; iY ++ )
  {
    minY = iY  - AREA; if ( minY < 0 )  minY = 0;
    maxY = iY  + AREA; if ( maxY >= depth ) maxY = depth - 1;
    for ( int iX = 0; iX < width; iX++ )
    {
      minX = iX - AREA; if ( minX < 0 )  minX = 0;
      maxX = iX + AREA; if ( maxX >= width ) maxX = width - 1;
      int r = 0; Val = 65535;
      for ( int y = minY; y <= maxY; y++ )
      {
        if ( Val > src[ iX + y * width ] ) Val = src[ iX + y * width ];
      }
      dst[ offset + iX ] = Val;
    }
    offset += width;
  }
#undef AREA
}

__forceinline bool _dilatev( pTVFrame frame )
{
  switch ( frame->lpBMIH->biCompression )
  {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
    case BI_Y800:
    {
      int imgsize = frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
      LPBYTE dst = ( LPBYTE ) malloc( imgsize );
      _dilatev( frame->lpBMIH->biWidth , frame->lpBMIH->biHeight , dst , GetData( frame ) );
      memcpy( GetData( frame ) , dst , imgsize );
      free( dst );
      return true;
    }
    case BI_Y16:
    {
      int imgsize = frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
      LPWORD dst = ( LPWORD ) malloc( imgsize * sizeof( WORD ) );
      _dilatev16( frame->lpBMIH->biWidth , frame->lpBMIH->biHeight , dst , ( LPWORD ) GetData( frame ) );
      memcpy( GetData( frame ) , dst , imgsize * sizeof( WORD ) );
      free( dst );
      return true;
    }
  }
  return false;
}

__forceinline void _and_rh( int width , int depth , LPBYTE dst , LPBYTE src )
{
  if ( ( !src ) || ( !dst ) ) return;
  LPBYTE pntr;
  int iY  , iX;
  for ( iY  = 0; iY  < depth - 1; iY ++ )
  {
    for ( iX = 0; iX < width; iX++ )
    {

      pntr = &( src[ iX + iY  * width ] );
      dst[ iX + iY  * width ] = ( *pntr > *( pntr + width ) ) ? *( pntr + width ) : *pntr;
    }
  }
  memcpy( dst + iY  * width , dst + ( iY  - 1 )*width , width );
}

__forceinline void _and_rh( pTVFrame frame )
{
  int imgsize = frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
  LPBYTE dst = ( LPBYTE ) malloc( imgsize );
  _and_rh( frame->lpBMIH->biWidth , frame->lpBMIH->biHeight , dst , GetData( frame ) );
  memcpy( GetData( frame ) , dst , imgsize );
  free( dst );
}


__forceinline void _minhlinelength( int width , int depth , LPBYTE src , int minlength = 4 , int color = 255 )
{
  LPBYTE  scanner = src;
  LPBYTE  end = src + width * depth;
  int     length;
  while ( scanner < end )
  {
    if ( *scanner == color )
    {
      length = 1;
      while ( ( *( scanner + length ) == color ) && ( scanner + length < end ) )
      {
        length++;
      }
      if ( length <= minlength )
      {
        for ( LPBYTE dst = scanner; dst < scanner + length; dst++ )
        {
          *dst = !color;
        }
      }
      scanner += length;
    }
    else scanner++;
  }
}

__forceinline void _minvlinelength( int width , int depth , LPBYTE src , int minlength , int color )
{
  int x;
  int     length;
  LPBYTE  offset;
  LPBYTE  end;
  for ( x = 0; x < width; x++ )
  {
    offset = src + x;
    end = offset + width * depth;
    while ( offset < end )
    {
      if ( *offset == color )
      {
        length = 1;
        while ( ( ( offset + length * width ) < end ) && ( *( offset + length * width ) == color ) )
          length++;
        if ( length <= minlength )
        {
          for ( int iY  = 0; iY  < length; iY ++ )
          {
            *( offset + iY  * width ) = !color;
          }
        }
        offset += length * width;
      }
      else offset += width;
    }
  }
}

__forceinline void _erode_kernel5( LPBYTE Data , DWORD width , DWORD height )
{
  DWORD length = width * height;
  LPBYTE Copy = ( LPBYTE ) malloc( length );
  if ( !Copy )
    return;
  memcpy( Copy , Data , length );
  DWORD offset = width + 2 , offset2 = 2 * width + 2 , iMax = length - offset2 , i = offset2;
  while ( i < iMax )
  {
    Data[ i ] = max5( max5( Copy[ i - offset2 ] , Copy[ i - offset2 + 1 ] , Copy[ i - offset2 + 2 ] , Copy[ i - offset2 + 3 ] , Copy[ i - offset2 + 4 ] ) ,
      max5( Copy[ i - offset ] , Copy[ i - offset + 1 ] , Copy[ i - offset + 2 ] , Copy[ i - offset + 3 ] , Copy[ i - offset + 4 ] ) ,
      max5( Copy[ i - 2 ] , Copy[ i - 1 ] , Copy[ i ] , Copy[ i + 1 ] , Copy[ i + 2 ] ) ,
      max5( Copy[ i + offset ] , Copy[ i + offset - 1 ] , Copy[ i + offset - 2 ] , Copy[ i + offset - 3 ] , Copy[ i + offset - 4 ] ) ,
      max5( Copy[ i + offset2 ] , Copy[ i + offset2 - 1 ] , Copy[ i + offset2 - 2 ] , Copy[ i + offset2 - 3 ] , Copy[ i + offset2 - 4 ] ) );
    i++;
    if ( ( i + 2 ) % width == 0 )
      i += 4;
  }
  free( Copy );
}

__forceinline void _remove_smallobjects( LPBYTE Data , DWORD width , DWORD height )
{
  DWORD length = width * height;
  LPBYTE Copy = ( LPBYTE ) malloc( length );
  if ( !Copy )
    return;
  memcpy( Copy , Data , length );
  memset( Data , 0 , length );
  LPBYTE src = Copy + width , dst = Data + width;

  while ( src < Copy + length - width )
  {
    int neighbours = ( *( src - 1 ) + *( src + 1 ) + *( src - width ) + *( src + width ) ) / 4;
    if ( ( *src == 0 ) && ( neighbours > 127 ) )
      *dst = 255;
    else if ( ( *src == 255 ) && ( neighbours < 128 ) )
      *dst = 0;
    else
      *dst = *src;
    src++;
    dst++;
  }
  free( Copy );
}

__forceinline void _remove_biggerobjects( LPBYTE Data , DWORD width , DWORD height )
{
  DWORD length = width * height;
  LPBYTE Copy = ( LPBYTE ) malloc( length );

  if ( !Copy )  return;

  memcpy( Copy , Data , length );
  memset( Data , 0 , length );
  LPBYTE src = Copy + 2 * width , dst = Data + 2 * width;

  while ( src < Copy + length - 2 * width )
  {
    int neighbours = ( *( src - 2 ) + *( src + 2 ) + *( src - 1 ) + *( src + 1 ) + *( src - width ) + *( src + width )
      + *( src - 2 * width ) + *( src + 2 * width ) + *( src - width - 1 ) + *( src + width - 1 )
      + *( src - width + 1 ) + *( src + width + 1 ) ) / 12;
    if ( ( *src == 0 ) && ( neighbours > 127 ) )
      *dst = 255;
    else if ( ( *src == 255 ) && ( neighbours < 128 ) )
      *dst = 0;
    else
      *dst = *src;
    src++;
    dst++;
  }
  free( Copy );
}


__forceinline void _sew_together( LPBYTE Data , DWORD width , DWORD height )
{
  DWORD length = width * height;
  LPBYTE Copy = ( LPBYTE ) malloc( length );

  if ( !Copy )  return;

  memcpy( Copy , Data , length );
  memset( Data , 0 , length );
  LPBYTE src = Copy + width , dst = Data + width;

  while ( src < Copy + length - width )
  {
    *dst = *src;
    if ( *( src - 1 ) && *( src + 1 ) || *( src - width ) && *( src + width ) )
      *dst = 255;
    if ( !*( src - 1 ) && !*( src + 1 ) || !*( src - width ) && !*( src + width ) )
      *dst = 0;
    src++;
    dst++;
  }
  free( Copy );
}

__forceinline void skelet_remove( LPBYTE data , int x , int y , int width )
{
  if ( data[ x + y * width ] == BLACK_COLOR ) return;

  if ( ( data[ ( y - 1 )*width + x ] != BLACK_COLOR ) &&
    ( data[ y    *width + x + 1 ] != BLACK_COLOR ) &&
    ( data[ ( y + 1 )*width + x ] != BLACK_COLOR ) &&
    ( data[ y    *width + x - 1 ] != BLACK_COLOR ) )  return;

  int count = 0;
  if ( data[ ( y - 1 )*width + x - 1 ] == ACTIVE_COLOR ) count = count + 1;
  if ( data[ ( y - 1 )*width + x ] == ACTIVE_COLOR ) count = count + 1;
  if ( data[ ( y - 1 )*width + x + 1 ] == ACTIVE_COLOR ) count = count + 1;
  if ( data[ y   *width + x + 1 ] == ACTIVE_COLOR ) count = count + 1;
  if ( data[ ( y + 1 )*width + x + 1 ] == ACTIVE_COLOR ) count = count + 1;
  if ( data[ ( y + 1 )*width + x ] == ACTIVE_COLOR ) count = count + 1;
  if ( data[ ( y + 1 )*width + x - 1 ] == ACTIVE_COLOR ) count = count + 1;
  if ( data[ y   *width + x - 1 ] == ACTIVE_COLOR ) count = count + 1;
  if ( count == 1 ) return;

  count = 0;
  if ( ( data[ ( y - 1 )*width + x - 1 ] == ACTIVE_COLOR ) && ( data[ ( y - 1 )*width + x ] < ACTIVE_COLOR ) )   count++;
  if ( ( data[ ( y - 1 )*width + x ] == ACTIVE_COLOR ) && ( data[ ( y - 1 )*width + x + 1 ] < ACTIVE_COLOR ) && ( data[ y*width + x + 1 ] < ACTIVE_COLOR ) ) count++;
  if ( ( data[ ( y - 1 )*width + x + 1 ] == ACTIVE_COLOR ) && ( data[ y*width + x + 1 ] < ACTIVE_COLOR ) )   count++;
  if ( ( data[ y*width + x + 1 ] == ACTIVE_COLOR ) && ( data[ ( y + 1 )*width + x + 1 ] < ACTIVE_COLOR ) && ( data[ ( y + 1 )*width + x ] < ACTIVE_COLOR ) ) count++;
  if ( ( data[ ( y + 1 )*width + x + 1 ] == ACTIVE_COLOR ) && ( data[ ( y + 1 )*width + x ] < ACTIVE_COLOR ) )   count++;
  if ( ( data[ ( y + 1 )*width + x ] == ACTIVE_COLOR ) && ( data[ ( y + 1 )*width + x - 1 ] < ACTIVE_COLOR ) && ( data[ y*width + x - 1 ] < ACTIVE_COLOR ) ) count++;
  if ( ( data[ ( y + 1 )*width + x - 1 ] == ACTIVE_COLOR ) && ( data[ y*width + x - 1 ] < ACTIVE_COLOR ) )   count++;
  if ( ( data[ y*width + x - 1 ] == ACTIVE_COLOR ) && ( data[ ( y - 1 )*width + x - 1 ] < ACTIVE_COLOR ) && ( data[ ( y - 1 )*width + x ] < ACTIVE_COLOR ) ) count++;

  if ( count > 1 ) return;
  data[ y*width + x ] = 1;
}

__forceinline void _skeletize( LPBYTE data , int width , int height )
{
  int x , y;
  for ( int i = 0; i < 5; i++ )
  {
    for ( y = 1; y < height - 1; y++ )
    {
      for ( x = 1; x < width - 1; x++ )
      {
        skelet_remove( data , x , y , width );
      }
    }
    for ( y = 0; y < height; y++ )
    {
      for ( x = 0; x < width; x++ )
      {
        if ( data[ y*width + x ] == 1 ) data[ y*width + x ] = BLACK_COLOR;
      }
    }

  }
}

__forceinline void _delholes( LPBYTE data , int width , int height )
{
  LPBYTE dst = data;
  for ( int y = 0; y < height; y++ )
  {

    LPBYTE left = dst;
    LPBYTE right = left + width - 1;
    while ( ( left <= right ) && ( *left == 0 ) ) left++;
    while ( ( left <= right ) && ( *right == 0 ) ) right--;
    if ( left != right ) left++;
    if ( left != right ) right--;
    for ( int x = 0; x < width; x++ )
    {
      LPBYTE p = dst + x;
      *p = ( ( p >= left ) && ( p <= right ) ) ? 255 : 0;
    }
    dst += width;
  }
}

__forceinline void _skeletize( pTVFrame frame )
{
  _skeletize( GetData( frame ) , frame->lpBMIH->biWidth , frame->lpBMIH->biHeight );
}

__forceinline void _skelet_rebuild( LPBYTE data , int x , int y , int width , int height )
{
  const BYTE JUST_REMOVED = 1;
  const BYTE JUST_PLACED = 2;

  if ( data[ x + y * width ] != ACTIVE_COLOR ) return;

  BYTE Color[ 8 ];															  //
  Color[ 0 ] = ( y == 0 ) ? 0 : ( ( x == 0 ) ? 0 : data[ ( y - 1 )*width + x - 1 ] ); // +---+---+---+
  Color[ 1 ] = ( y == 0 ) ? 0 : ( data[ ( y - 1 )*width + x ] ); // | 0 | 1 | 2 |
  Color[ 2 ] = ( y == 0 ) ? 0 : ( ( x == width - 1 ) ? 0 : data[ ( y - 1 )*width + x + 1 ] ); // +---+---+---+
  Color[ 3 ] = ( ( x == width - 1 ) ? 0 : data[ y    *width + x + 1 ] ); // | 7 |   | 3 |
  Color[ 4 ] = ( y == height - 1 ) ? 0 : ( ( x == width - 1 ) ? 0 : data[ ( y + 1 )*width + x + 1 ] ); // +---+---+---+
  Color[ 5 ] = ( y == height - 1 ) ? 0 : ( data[ ( y + 1 )*width + x ] ); // | 6 | 5 | 4 |
  Color[ 6 ] = ( y == height - 1 ) ? 0 : ( ( x == 0 ) ? 0 : data[ ( y + 1 )*width + x - 1 ] ); // +---+---+---+
  Color[ 7 ] = ( ( x == 0 ) ? 0 : data[ y    *width + x - 1 ] ); //

  if ( ( Color[ 1 ] != BLACK_COLOR ) &&
    ( Color[ 3 ] != BLACK_COLOR ) &&
    ( Color[ 5 ] != BLACK_COLOR ) &&
    ( Color[ 7 ] != BLACK_COLOR ) ) return;

  int count = 0;
  if ( Color[ 0 ] == ACTIVE_COLOR ) count++;
  if ( Color[ 1 ] == ACTIVE_COLOR ) count++;
  if ( Color[ 2 ] == ACTIVE_COLOR ) count++;
  if ( Color[ 3 ] == ACTIVE_COLOR ) count++;
  if ( Color[ 4 ] == ACTIVE_COLOR ) count++;
  if ( Color[ 5 ] == ACTIVE_COLOR ) count++;
  if ( Color[ 6 ] == ACTIVE_COLOR ) count++;
  if ( Color[ 7 ] == ACTIVE_COLOR ) count++;
  if ( count == 1 ) return;

  int dx = 0 , dy = 0;
  if ( ( Color[ 0 ] == ACTIVE_COLOR ) || ( Color[ 0 ] == JUST_REMOVED ) ) { dx--; dy--; }
  else { dx++; dy++; }
  if ( ( Color[ 1 ] == ACTIVE_COLOR ) || ( Color[ 1 ] == JUST_REMOVED ) ) { dy--; }
  else { dy++; }
  if ( ( Color[ 2 ] == ACTIVE_COLOR ) || ( Color[ 2 ] == JUST_REMOVED ) ) { dx++; dy--; }
  else { dx--; dy++; }
  if ( ( Color[ 3 ] == ACTIVE_COLOR ) || ( Color[ 3 ] == JUST_REMOVED ) ) { dx++; }
  else { dx--; }
  if ( ( Color[ 4 ] == ACTIVE_COLOR ) || ( Color[ 4 ] == JUST_REMOVED ) ) { dx++; dy++; }
  else { dx--; dy--; }
  if ( ( Color[ 5 ] == ACTIVE_COLOR ) || ( Color[ 5 ] == JUST_REMOVED ) ) { dy++; }
  else { dy--; }
  if ( ( Color[ 6 ] == ACTIVE_COLOR ) || ( Color[ 6 ] == JUST_REMOVED ) ) { dx--; dy++; }
  else { dx++; dy--; }
  if ( ( Color[ 7 ] == ACTIVE_COLOR ) || ( Color[ 7 ] == JUST_REMOVED ) ) { dx--; }
  else { dx++; }
  if ( ( dx == 0 ) && ( dy == 0 ) ) return;

  int dmax = ( abs( dx ) < abs( dy ) ) ? abs( dy ) : abs( dx );
  dx /= dmax; dy /= dmax;

  int index = -1;
  if ( dy == -1 )
  {
    if ( dx == -1 ) index = 0;
    else if ( dx == 0 ) index = 1;
    else if ( dx == 1 ) index = 2;
  }
  else if ( dy == 1 )
  {
    if ( dx == -1 ) index = 6;
    else if ( dx == 0 ) index = 5;
    else if ( dx == 1 ) index = 4;
  }
  else if ( dy == 0 )
  {
    if ( dx == -1 ) index = 7;
    else if ( dx == 1 ) index = 3;
  }
  ASSERT( index != -1 );

  Color[ index ] = JUST_PLACED;
  int cafter = 0;
  if ( ( Color[ 0 ] >= JUST_PLACED ) && ( Color[ 1 ] < JUST_PLACED ) )							 cafter++;
  if ( ( Color[ 1 ] >= JUST_PLACED ) && ( Color[ 2 ] < JUST_PLACED ) && ( Color[ 3 ] < JUST_PLACED ) ) cafter++;
  if ( ( Color[ 2 ] >= JUST_PLACED ) && ( Color[ 3 ] < JUST_PLACED ) )							 cafter++;
  if ( ( Color[ 3 ] >= JUST_PLACED ) && ( Color[ 4 ] < JUST_PLACED ) && ( Color[ 5 ] < JUST_PLACED ) ) cafter++;
  if ( ( Color[ 4 ] >= JUST_PLACED ) && ( Color[ 5 ] < JUST_PLACED ) )							 cafter++;
  if ( ( Color[ 5 ] >= JUST_PLACED ) && ( Color[ 6 ] < JUST_PLACED ) && ( Color[ 7 ] < JUST_PLACED ) ) cafter++;
  if ( ( Color[ 6 ] >= JUST_PLACED ) && ( Color[ 7 ] < JUST_PLACED ) )							 cafter++;
  if ( ( Color[ 7 ] >= JUST_PLACED ) && ( Color[ 0 ] < JUST_PLACED ) && ( Color[ 1 ] < JUST_PLACED ) ) cafter++;

  if ( cafter > 1 ) return;

  data[ y*width + x ] = JUST_REMOVED;
  data[ ( y + dy )*width + x + dx ] = JUST_PLACED;
}

__forceinline void _skelet_dry( LPBYTE data , int width , int height , int citer = 1 )
{
  int x , y , i;
  for ( i = 0; i < citer; i++ )
  {
    for ( y = 0; y < height; y++ )
    {
      for ( x = 0; x < width; x++ )
      {
        _skelet_rebuild( data , x , y , width , height );
      }
    }
    for ( y = 0; y < height; y++ )
    {
      for ( x = 0; x < width; x++ )
      {
        if ( data[ y*width + x ] == 1 ) data[ y*width + x ] = BLACK_COLOR;
        else if ( data[ y*width + x ] == 2 ) data[ y*width + x ] = ACTIVE_COLOR;
      }
    }
  }
}

__forceinline void _skeletize_v2( pTVFrame frame )
{
  _skelet_dry( GetData( frame ) , frame->lpBMIH->biWidth , frame->lpBMIH->biHeight );
}

#endif
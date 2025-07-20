#ifndef YUV411_INC
#define YUV411_INC

LPBITMAPINFOHEADER yuv9rgb24( LPBITMAPINFOHEADER src , LPBYTE lpData = NULL );
LPBITMAPINFOHEADER yuv12rgb24( LPBITMAPINFOHEADER src , LPBYTE lpData = NULL );
LPBITMAPINFOHEADER rgb24yuv9( LPBITMAPINFOHEADER src , LPBYTE lpData = NULL );
LPBITMAPINFOHEADER yuv411yuv9( LPBITMAPINFOHEADER src , LPBYTE lpData = NULL );
LPBITMAPINFOHEADER yuv12yuv9( LPBITMAPINFOHEADER src , LPBYTE lpData = NULL );
LPBITMAPINFOHEADER yuv9rgb8( LPBITMAPINFOHEADER src , LPBYTE lpData );
LPBITMAPINFOHEADER y8rgb8( LPBITMAPINFOHEADER src , LPBYTE lpData );
LPBITMAPINFOHEADER y16rgb8( LPBITMAPINFOHEADER src , LPBYTE lpD );
LPBITMAPINFOHEADER y16yuv8( LPBITMAPINFOHEADER src , LPBYTE lpD );
LPBITMAPINFOHEADER y8yuv9( LPBITMAPINFOHEADER src , LPBYTE lpD );
LPBITMAPINFOHEADER y8rgb24( LPBITMAPINFOHEADER src , LPBYTE lpD );
LPBITMAPINFOHEADER rgb24rdb32( LPBITMAPINFOHEADER src , LPBYTE lpData = NULL );
LPBITMAPINFOHEADER y16rgb24( LPBITMAPINFOHEADER src , LPBYTE lpD );

__forceinline LPBITMAPINFOHEADER _decompress2rgb( pTVFrame frame )
{
  LPBITMAPINFOHEADER lpRet = NULL;
  if ( ( frame ) && ( frame->lpBMIH ) )
  {
    switch ( frame->lpBMIH->biCompression )
    {
      case BI_YUV9:
        lpRet = yuv9rgb24( frame->lpBMIH , frame->lpData );
        break;
      case BI_YUV12:
        lpRet = yuv12rgb24( frame->lpBMIH , frame->lpData );
        break;
      case BI_Y8:
      case BI_Y800:
        lpRet = y8rgb8( frame->lpBMIH , frame->lpData );
        break;
      case BI_Y16:
        lpRet = y16rgb8( frame->lpBMIH , frame->lpData );
        break;
    }
  }
  return lpRet;
}

#endif

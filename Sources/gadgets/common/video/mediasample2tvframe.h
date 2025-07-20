#pragma once

#define THIS_MODULENAME "mediasample2tvframe"
__forceinline bool _clearcolor( pTVFrame frame )
{
  switch ( frame->lpBMIH->biCompression )
  {
  case BI_YUV12:
  case BI_YUV9: //the only color format
    {
      // This old implementation just set U and V values to 128...
      /* LPBYTE lpData=GetData(frame);
      if (!lpData) return false;
      LPBYTE eod=lpData+frame->lpBMIH->biSizeImage;
      lpData+=frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
      while (lpData<eod) { *lpData=128; lpData++;}
      return true;*/

      // The second implementation converts it to Y8 format (just change BM header
      frame->lpBMIH->biCompression = BI_Y8;
      frame->lpBMIH->biSizeImage = frame->lpBMIH->biWidth*frame->lpBMIH->biHeight;
      frame->lpBMIH->biBitCount = 0;
      return true;

    }
  }
  return false;
}

__forceinline pTVFrame setframeformat( TVFrame& tvFrame , DWORD OutputFormat )
{
  if ( tvFrame.lpBMIH->biCompression == BI_YUV9 )
  {
    switch ( OutputFormat )
    {
      case BI_YUV9:
      {
        pTVFrame frame = makecopyTVFrame( &tvFrame );
        return frame;
      }
      case BI_YUV12:
      {
        //TODO
        ASSERT( FALSE );
        return NULL;
      }
      case BI_Y8:
      {
        pTVFrame frame = makecopyTVFrame( &tvFrame );
        _clearcolor( frame );
        return frame;
      }
    }
  }
  else if ( tvFrame.lpBMIH->biCompression == BI_UYVY ) // convert to more compact and efficient BI_YUV9
  {
    switch ( OutputFormat )
    {
      case BI_YUV9:
      {
        pTVFrame frame = newTVFrame( _convertUYVY2YUV9( &tvFrame ) );
        return frame;
      }
      case BI_YUV12:
      {
        pTVFrame frame = newTVFrame( _convertUYVY2YUV12( &tvFrame ) );
        return frame;
      }
      case BI_Y8:
      {
        pTVFrame frame = newTVFrame( _convertUYVY2YUV9( &tvFrame ) );
        _clearcolor( frame );
        return frame;
      }
    }
  }
  else if ( tvFrame.lpBMIH->biCompression == BI_YUY2 ) // convert to more compact and efficient BI_YUV9
  {
    switch ( OutputFormat )
    {
      case BI_YUV9:
      {
        pTVFrame frame = newTVFrame( _convertYUY2YUV9( &tvFrame ) );
        return frame;
      }
      case BI_YUV12:
      {
        pTVFrame frame = newTVFrame( _convertYUY2YUV12( &tvFrame ) );
        return frame;
      }
      case BI_Y8:
      {
        pTVFrame frame = newTVFrame( _convertYUY2YUV9( &tvFrame ) );
        _clearcolor( frame );
        frame->lpBMIH->biBitCount = 8 ;
        return frame;
      }
    }
  }
  else if ( tvFrame.lpBMIH->biCompression == BI_Y411 )
  {
    switch ( OutputFormat )
    {
      case BI_YUV9:
      {
        pTVFrame frame = newTVFrame( _convertY411YUV9( &tvFrame ) );
        return frame;
      }
      case BI_YUV12:
      {
        //TODO
        ASSERT( FALSE );
        return NULL;
      }
      case BI_Y8:
      {
        pTVFrame frame = newTVFrame( _convertY411YUV9( &tvFrame ) );
        _clearcolor( frame );
        return frame;
      }
    }
  }
  else if ( tvFrame.lpBMIH->biCompression == BI_YUV12 )
  {
    switch ( OutputFormat )
    {
      case BI_YUV9:
      {
        pTVFrame frame = newTVFrame( yuv12yuv9( tvFrame.lpBMIH , tvFrame.lpData ) );
        return frame;
      }
      case BI_YUV12:
      {
        pTVFrame frame = makecopyTVFrame( &tvFrame );
        return frame;
      }
      case BI_Y8:
      {
        _clearcolor( &tvFrame );
        pTVFrame frame = makecopyTVFrame( &tvFrame );
        return frame;
      }
    }
  }
  else if ( (tvFrame.lpBMIH->biCompression == BI_Y8) || (tvFrame.lpBMIH->biCompression == BI_Y800) )
  {
    switch ( OutputFormat )
    {
      case BI_YUV9:
      {
        pTVFrame frame = newTVFrame( _convertY82YUV9( &tvFrame ) );
        return frame;
      }
      case BI_YUV12:
      {
        //TODO
        ASSERT( FALSE );
        return NULL;
      }
      case BI_Y8:
      {
        pTVFrame frame = makecopyTVFrame( &tvFrame );
        return frame;
      }
    }
  }
  else if ( (tvFrame.lpBMIH->biCompression == BI_RGB) )
  {
    if ( tvFrame.lpBMIH->biBitCount == 8 )
    {
      switch ( OutputFormat )
      {
        case BI_YUV9:
        {
          pTVFrame frame = newTVFrame( _convertRGB8Gray2YUV9( &tvFrame ) );
          return frame;
        }
        case BI_YUV12:
        {
          //TODO
          ASSERT( FALSE );
          break;
        }
        case BI_Y8:
        {
          pTVFrame frame = newTVFrame( _convertRGB8Gray2YUV9( &tvFrame ) );
          _clearcolor( frame );
          return frame;
        }
      }
    }
    else if ( tvFrame.lpBMIH->biBitCount == 24 )
    {
      switch ( OutputFormat )
      {
        case BI_YUV9:
        {
          //            pTVFrame frame = newTVFrame( _convertRGB8Gray2YUV9( &tvFrame ) );
          pTVFrame frame = newTVFrame( rgb24yuv9( tvFrame.lpBMIH , tvFrame.lpData ) );
          return frame;
        }
        case BI_YUV12:
        {
          pTVFrame frame = newTVFrame( rgb24yuv12( tvFrame.lpBMIH , tvFrame.lpData ) );
          return frame;
        }
        case BI_Y8:
        {
          pTVFrame frame = newTVFrame( rgb24_to_y8( tvFrame.lpBMIH , tvFrame.lpData ) );
          _clearcolor( frame );
          return frame;
        }
      }
    }
  }
  else
  {
    SENDERR_0( "!!! Unsupported video format!" );
    return NULL;
  }
  return NULL;
}

__forceinline pTVFrame mediasample2tvframe( CMediaType *pMediaType , IMediaSample *pMediaSample , DWORD OutputFormat )
{
  TVFrame tvFrame;
  pTVFrame retV = NULL;
  BITMAPINFOHEADER BMIH;

  if ( (pMediaType) && (pMediaType->formattype == FORMAT_VideoInfo) )
  {

    ASSERT( (((VIDEOINFOHEADER*) pMediaType->pbFormat)->bmiHeader.biSize) == sizeof( BMIH ) );
    memcpy( &BMIH , &(((VIDEOINFOHEADER*) pMediaType->pbFormat)->bmiHeader) , ((VIDEOINFOHEADER*) pMediaType->pbFormat)->bmiHeader.biSize );
    tvFrame.lpBMIH = &BMIH;
    pMediaSample->GetPointer( &tvFrame.lpData );
    return setframeformat( tvFrame , OutputFormat );
  }
  else if ( (pMediaType) && (pMediaType->formattype == FORMAT_VideoInfo2) )
  {
    VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*) pMediaType->pbFormat;
    ASSERT( vih->bmiHeader.biSize == sizeof( BMIH ) );
    memcpy( &BMIH , &(vih->bmiHeader) , vih->bmiHeader.biSize );
    tvFrame.lpBMIH = &BMIH;
    pMediaSample->GetPointer( &tvFrame.lpData );
    return setframeformat( tvFrame , OutputFormat );
  }
  else
  {
    SENDERR_0( "!!! Can't request MediaType" );
    return NULL;
  }
}

__forceinline REFERENCE_TIME AvgTimePerFrame( CMediaType *pMediaType )
{
  if ( (pMediaType) && (pMediaType->formattype == FORMAT_VideoInfo) )
  {
    VIDEOINFOHEADER* pvhr = (VIDEOINFOHEADER*) pMediaType->pbFormat;
    return pvhr->AvgTimePerFrame;
  }
  else if ( (pMediaType) && (pMediaType->formattype == FORMAT_VideoInfo2) )
  {
    return ((VIDEOINFOHEADER2*) pMediaType->pbFormat)->AvgTimePerFrame;
  }
  else
  {
    SENDERR_0( "!!! Can't request MediaType" );
    return -1;
  }
}

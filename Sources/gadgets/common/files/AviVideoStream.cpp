// AviVideoStream.cpp: implementation of the CAviVideoStream class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AviVideoStream.h"
#include <video\videoformats.h>
#include <files\AviFile.h>
#include <video\tvframe.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define DEFAULT_KEYFRAMEEVERY	15
#define DEFAULT_BYTESPERSECOND	8000
#define THIS_MODULENAME "TVAvi.AviVideoStream"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAviVideoStream::CAviVideoStream( PAVIFILE hFile , UINT mode , long id , DWORD fourcc ) :
  CAviStream( hFile , mode ) ,
  m_pGetFrame( NULL ) ,
  m_pIntFmt( NULL ) ,
  m_dwIntFmtSize( 0 ) ,
  m_CompressorNotAvailableShown( false )
{
  CoInitialize( NULL );
  //	ZeroMemory(&m_IntFmt, sizeof(m_IntFmt));
  //	m_IntFmt.biSize = sizeof(m_IntFmt);
  BOOL bResult = ((mode == OF_CREATE) && (id < 0)) ? TRUE : OpenStream( streamtypeVIDEO , id );
  if ( bResult )
  {
    switch ( m_uMode )
    {
      case OF_READ:
        bResult = OpenRead();
        break;
      case OF_CREATE:
        bResult = (id < 0) ? TRUE : OpenOverwrite();
        if ( bResult )
          Resume();
        break;
      case OF_WRITE:
        bResult = OpenAppend();
        if ( bResult )
          Resume();
        break;
    }
  }
  if ( !bResult && m_hStream )
  {
    ::AVIStreamRelease( m_hStream );
    m_hStream = NULL;
  }
  if ( fourcc )
    SetFourCC( fourcc );
}

CAviVideoStream::CAviVideoStream( PAVIFILE hFile , PAVISTREAM hStream , AVISTREAMINFO* pInfo , UINT mode , DWORD fourcc ) :
  CAviStream( hFile , mode , hStream , pInfo , fourcc ) ,
  m_pGetFrame( NULL ) ,
  m_pIntFmt( NULL ) ,
  m_dwIntFmtSize( 0 ) ,
  m_CompressorNotAvailableShown( false )
{
  CoInitialize( NULL );
  //	ZeroMemory(&m_IntFmt, sizeof(m_IntFmt));
  //	m_IntFmt.biSize = sizeof(m_IntFmt);
  BOOL bResult = FALSE;
  switch ( m_uMode )
  {
    case OF_READ:
      bResult = OpenRead();
      break;
    case OF_CREATE:
      bResult = OpenOverwrite();
      if ( bResult )
        Resume();
      break;
    case OF_WRITE:
      bResult = OpenAppend();
      if ( bResult )
        Resume();
      break;
  }
  if ( !bResult && m_hStream )
  {
    ::AVIStreamRelease( m_hStream );
    m_hStream = NULL;
  }
  SetFourCC( 0x3234706d );
  m_Label = pInfo->szName;
}


CAviVideoStream::~CAviVideoStream()
{
  if ( m_pGetFrame )
    ::AVIStreamGetFrameClose( m_pGetFrame );
  m_pGetFrame = NULL;
  free( m_pIntFmt );
  m_dwIntFmtSize = 0;
  CoUninitialize();
}


BOOL CAviVideoStream::OpenRead()
{
  m_LastError = ::AVIStreamReadFormat( m_hStream , ::AVIStreamStart( m_hStream ) , NULL , (LONG*) &m_dwIntFmtSize );
  if ( !TraceSuccess( m_LastError ) )
    return FALSE;
  m_pIntFmt = (LPBITMAPINFOHEADER) realloc( m_pIntFmt , m_dwIntFmtSize );
  m_LastError = ::AVIStreamReadFormat( m_hStream , ::AVIStreamStart( m_hStream ) , m_pIntFmt , (LONG*) &m_dwIntFmtSize/*(long*)&m_IntFmt.biSize*/ );
  if ( !TraceSuccess( m_LastError ) )
    return FALSE;
  if ( m_pIntFmt->biCompression == BI_YUV9 )
  {
    m_dwBufSize = (m_pIntFmt->biSizeImage) ? m_pIntFmt->biSizeImage : (9 * m_pIntFmt->biWidth * m_pIntFmt->biHeight / 8);
    if ( m_pIntFmt->biSizeImage != m_dwBufSize ) m_pIntFmt->biSizeImage = m_dwBufSize;
  }
  else if ( m_pIntFmt->biCompression == BI_Y8 || m_pIntFmt->biCompression == BI_Y800 )
  {
    m_dwBufSize = (m_pIntFmt->biSizeImage) ? m_pIntFmt->biSizeImage : (m_pIntFmt->biWidth * m_pIntFmt->biHeight);
    if ( m_pIntFmt->biSizeImage != m_dwBufSize ) m_pIntFmt->biSizeImage = m_dwBufSize;
  }
  else if ( m_pIntFmt->biCompression == BI_Y16 )
  {
    m_dwBufSize = m_pIntFmt->biWidth * m_pIntFmt->biHeight * sizeof( short );
    if ( m_pIntFmt->biSizeImage != m_dwBufSize ) m_pIntFmt->biSizeImage = m_dwBufSize;
  }
  else if ( m_pIntFmt->biCompression == BI_YUV12 )
  {
    m_dwBufSize = m_pIntFmt->biWidth * m_pIntFmt->biHeight * sizeof( short );
    if ( m_pIntFmt->biSizeImage != m_dwBufSize ) m_pIntFmt->biSizeImage = m_dwBufSize;
  }
  else if ( m_pIntFmt->biCompression == BI_RGB )
  {
    m_dwBufSize = ((((UINT) m_pIntFmt->biBitCount * m_pIntFmt->biWidth + 31) & ~31) / 8) * m_pIntFmt->biHeight;
    if ( m_pIntFmt->biSizeImage != m_dwBufSize ) m_pIntFmt->biSizeImage = m_dwBufSize;
  }
  else
  {
    m_pGetFrame = ::AVIStreamGetFrameOpen( m_hStream , NULL );
    if ( !m_pGetFrame )
    {
      BITMAPINFOHEADER BIH;

      BIH.biSize = sizeof( BITMAPINFOHEADER );
      BIH.biWidth = m_pIntFmt->biWidth;
      BIH.biHeight = m_pIntFmt->biHeight;
      BIH.biPlanes = 1;
      BIH.biBitCount = 24;
      BIH.biCompression = BI_RGB;
      BIH.biSizeImage = 0;
      BIH.biXPelsPerMeter = 0;
      BIH.biYPelsPerMeter = 0;
      BIH.biClrUsed = 0;
      BIH.biClrImportant = 0;

      m_pGetFrame = ::AVIStreamGetFrameOpen( m_hStream , (LPBITMAPINFOHEADER) &BIH );
    }
    if ( !m_pGetFrame )
    {
      char fourcc[ 5 ];
      fourcc[ 4 ] = 0;
      memcpy( fourcc , &m_pIntFmt->biCompression , 4 );
      SENDERR_1( "Can't play video with FOURCC \"%s\"" , fourcc );
      return FALSE;
    }
  }
  return TRUE;
}

BOOL CAviVideoStream::OpenOverwrite()
{
  m_CompressorNotAvailableShown = false;
  if ( !OpenAppend() )
    return FALSE;
  m_nCurFrame = -1;
  return TRUE;
}

BOOL CAviVideoStream::OpenAppend()
{
  m_CompressorNotAvailableShown = false;
  m_LastError = ::AVIStreamReadFormat( m_hStream , ::AVIStreamStart( m_hStream ) , NULL , (LONG*) &m_dwIntFmtSize );
  if ( !TraceSuccess( m_LastError ) )
    return FALSE;
  m_pIntFmt = (LPBITMAPINFOHEADER) realloc( m_pIntFmt , m_dwIntFmtSize );
  m_LastError = ::AVIStreamReadFormat( m_hStream , ::AVIStreamStart( m_hStream ) , m_pIntFmt , (LONG*) &m_dwIntFmtSize/*(long*)&m_IntFmt.biSize*/ );
  if ( !TraceSuccess( m_LastError ) )
    return FALSE;
  m_nCurFrame = (int) m_StreamInfo.dwLength - 1;
  return TRUE;
}

void CAviVideoStream::DoWrite( void* lpData , LPCTSTR Label )
{
  if ( Label ) m_Label = Label;
  pTVFrame Frame = (pTVFrame) lpData;
  //    TRACE("+++ Render frame 0x%x\n",lpData);
  DWORD dwFlags = (m_nCurFrame % 2) ? 0 : AVIIF_KEYFRAME;
  if ( !m_ComressOps.fccHandler )
    m_LastError = ::AVIStreamWrite( m_hStream , (LONG) GetCurFrameID() , 1 , GetData( Frame ) , (LONG) Frame->lpBMIH->biSizeImage , dwFlags , NULL , NULL );
  else
  {
    LPBITMAPINFOHEADER bmih = NULL;
    switch ( m_pIntFmt->biCompression )
    {
      case BI_YUV9:
        bmih = yuv9rgb24( Frame->lpBMIH , GetData( Frame ) );
        break;
      case BI_YUV12:
        bmih = yuv12rgb24( Frame->lpBMIH , GetData( Frame ) );
        break;
      case BI_Y8:
      case BI_Y800:
        bmih = y8rgb24( Frame->lpBMIH , GetData( Frame ) );
        break;
      default:
      {
        if ( !m_CompressorNotAvailableShown )
        {
          char fourcc[ 5 ];
          fourcc[ 4 ] = 0;
          memcpy( fourcc , &m_pIntFmt->biCompression , 4 );
          SENDERR_1( "Can't compress video with FOURCC \"%s\"" , fourcc );
          m_CompressorNotAvailableShown = true;
        }
        freeTVFrame( Frame );
        return;
      }
    }
    m_LastError = ::AVIStreamWrite( m_hStream , (LONG) GetCurFrameID() , 1 , (LPBYTE) bmih + bmih->biSize , (LONG) bmih->biSizeImage , dwFlags , NULL , NULL );
    free( bmih );
  }
  freeTVFrame( Frame );
  if ( TraceSuccess( m_LastError ) )
    m_StreamInfo.dwLength++;
}

BOOL CAviVideoStream::CheckFormat( void* lpData , LPCTSTR label )
{
  pTVFrame Frame = (pTVFrame) lpData;
  if ( m_hStream )
  {
    LPBITMAPINFOHEADER pBMIH = Frame->lpBMIH;
    if ( (pBMIH->biWidth == m_pIntFmt->biWidth) && (pBMIH->biHeight == m_pIntFmt->biHeight) )
      return TRUE;
  }
  else if ( m_uMode == OF_CREATE )
  {
    ASSERT( m_pIntFmt == NULL );
    if ( !m_pIntFmt )
    {
      m_pIntFmt = (LPBITMAPINFOHEADER) malloc( Frame->lpBMIH->biSize );
      memcpy( m_pIntFmt , Frame->lpBMIH , Frame->lpBMIH->biSize );
    }
    m_StreamInfo.fccType = streamtypeVIDEO;
    if ( m_OverwriteFrameRate )
    {
      m_StreamInfo.dwScale = 10;
      m_StreamInfo.dwRate = (int) (m_FrameRate*m_StreamInfo.dwScale);
    }
    else
    {
      m_StreamInfo.dwScale = 1;
      m_StreamInfo.dwRate = 25;
    }
    m_StreamInfo.dwQuality = -1;
    m_StreamInfo.dwSampleSize = m_pIntFmt->biSizeImage;
    if ( label )
    {
      int len = (int) strlen( label );
      if ( len >= 63 )
      {
        memcpy( m_StreamInfo.szName , label , 63 );
      }
      else
        strcpy( m_StreamInfo.szName , label );
    }
    m_sTickCount = GetTickCount();
    if ( !m_ComressOps.fccHandler )
      m_StreamInfo.dwSuggestedBufferSize = m_pIntFmt->biHeight*m_pIntFmt->biWidth * 3; // 24 bit
    else
      m_StreamInfo.dwSuggestedBufferSize = m_pIntFmt->biSizeImage;
    ::SetRect( &m_StreamInfo.rcFrame , 0 , 0 , m_pIntFmt->biWidth , m_pIntFmt->biHeight );
    m_dwBufSize = m_pIntFmt->biSizeImage;
    BITMAPINFOHEADER outFmt;
    memcpy( &outFmt , m_pIntFmt , sizeof( BITMAPINFOHEADER )/*sizeof(m_IntFmt)*/ );
    if ( !m_ComressOps.fccHandler )
    {
      m_StreamInfo.fccHandler = m_pIntFmt->biCompression;
      m_LastError = ::AVIFileCreateStream( m_pFile , &m_hStream , &m_StreamInfo );
      if ( !TraceSuccess( m_LastError ) )
      {
        freeTVFrame( Frame );
        return FALSE;
      }
    }
    else
    {
      PAVISTREAM tmpStream;
      m_LastError = ::AVIFileCreateStream( m_pFile , &tmpStream , &m_StreamInfo );
      if ( !TraceSuccess( m_LastError ) )
      {
        freeTVFrame( Frame );
        return FALSE;
      }
      m_LastError = ::AVIMakeCompressedStream( &m_hStream , tmpStream , &m_ComressOps , NULL );
      ::AVIStreamRelease( tmpStream );
      if ( !TraceSuccess( m_LastError ) )
      {
        freeTVFrame( Frame );
        return FALSE;
      }
      outFmt.biCompression = 0;
      outFmt.biBitCount = 24;
      outFmt.biSizeImage = 0;
    }
    m_LastError = ::AVIStreamSetFormat( m_hStream , 0 , &outFmt , (LONG)sizeof( outFmt ) );
    if ( !TraceSuccess( m_LastError ) )
    {
      ::AVIStreamRelease( m_hStream );
      m_hStream = NULL;
      freeTVFrame( Frame );
      return FALSE;
    }
    return TRUE;
  }
  freeTVFrame( Frame );
  return FALSE;
}

void* CAviVideoStream::DoRead()
{
  if ( IsEOF() )
    return NULL;
  pTVFrame Frame = NULL;
  if ( !m_pGetFrame )
  {
    if ( (m_pIntFmt->biCompression != BI_Y8) && (m_pIntFmt->biCompression != BI_Y800)
      && (m_pIntFmt->biCompression != BI_YUV9) && (m_pIntFmt->biCompression != BI_YUV12)
      && (m_pIntFmt->biCompression != BI_Y16) && (m_pIntFmt->biCompression != BI_RGB) )
      return NULL;
    void* pBuffer = (void*) malloc( m_dwBufSize );
    m_LastError = ::AVIStreamRead( m_hStream , (LONG) GetCurFrameID() , 1 , pBuffer , (LONG) m_dwBufSize , NULL , NULL );
    if ( !TraceSuccess( m_LastError ) )
    {
      free( pBuffer );
      return NULL;
    }
    if ( (m_pIntFmt->biCompression == BI_YUV9) || (m_pIntFmt->biCompression == BI_Y16) || (m_pIntFmt->biCompression == BI_YUV12) )
    {
      //Frame = newTVFrame(m_pIntFmt, (LPBYTE)pBuffer);
      LPBITMAPINFOHEADER pBMIH = (LPBITMAPINFOHEADER) malloc( m_dwIntFmtSize + m_dwBufSize );
      memcpy( pBMIH , m_pIntFmt , m_dwIntFmtSize );
      memcpy( (LPBYTE) pBMIH + m_dwIntFmtSize , pBuffer , m_dwBufSize );
      free( pBuffer );
      Frame = newTVFrame( pBMIH , NULL );
    }
    else
    {
      LPBITMAPINFOHEADER pBMIH = (LPBITMAPINFOHEADER) malloc( m_dwIntFmtSize + m_dwBufSize );
      memcpy( pBMIH , m_pIntFmt , m_dwIntFmtSize );
      memcpy( (LPBYTE) pBMIH + m_dwIntFmtSize , pBuffer , m_dwBufSize );
      free( pBuffer );
      Frame = newTVFrame( pBMIH , NULL );
      makeYUV9( Frame );
    }
  }
  else
  {
    LPBITMAPINFOHEADER pTmpFrame = (LPBITMAPINFOHEADER)::AVIStreamGetFrame( m_pGetFrame , (LONG) GetCurFrameID() );
    if ( pTmpFrame && pTmpFrame->biSizeImage )
    {
      DWORD dwBufSize = pTmpFrame->biSize + pTmpFrame->biSizeImage;
      void* pBuffer = malloc( dwBufSize );
      memcpy( pBuffer , pTmpFrame , dwBufSize );
      Frame = newTVFrame( (LPBITMAPINFOHEADER) pBuffer , NULL );
      if ( (Frame->lpBMIH->biCompression != BI_YUV9) &&
        (Frame->lpBMIH->biCompression != BI_Y8) && (m_pIntFmt->biCompression != BI_Y800) )
        makeYUV9( Frame );
    }
  }
  return Frame;
}

BOOL CAviVideoStream::SetFourCC( DWORD fourCC )
{
  if ( m_uMode == OF_READ )
    return FALSE;
  m_ComressOps.fccType = streamtypeVIDEO;
  m_ComressOps.fccHandler = fourCC;
  m_ComressOps.dwKeyFrameEvery = 2;
  m_ComressOps.dwBytesPerSecond = DEFAULT_BYTESPERSECOND;
  m_ComressOps.dwFlags = AVICOMPRESSF_KEYFRAMES | AVICOMPRESSF_DATARATE | AVICOMPRESSF_VALID;
  return TRUE;
}
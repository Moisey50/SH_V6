// VideoFilters.cpp: implementation of the CVideoFilters class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VideoFilters.h"
#include <imageproc\simpleip.h>
#include <imageproc\rectangles.h>
#include <imageproc\resample.h>
#include <imageproc\edgefilters.h>
#include <gadgets\RectFrame.h>
#include <imageproc\enchance.h>
#include "TVVideo.h"
#include <imageproc\rotate.h>
#include <imageproc\colors.h>
#include <imageproc\gammacorrection.h>
#include <imageproc\deinterlace.h>
#include <imageproc\recognition\htriggerbin.h>
#include <imageproc\meander.h>
#include <imageproc\cut.h>
#include <imageproc\draw_over.h>
#include <math\Intf_sup.h>
#include <gadgets\ContainerFrame.h>
#include <helpers/propertykitEx.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#ifdef _TRACE_DATAFRAMERELEASE
#define GADEGTNAME m_Name
#else
#define GADEGTNAME
#endif

IMPLEMENT_VIDEO_FILTER( Any2Yuv9 , "conversion" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* Any2Yuv9::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  //pTVFrame frame = makecopyTVFrame2(VideoFrame);
  pTVFrame frame = ( pTVFrame ) malloc( sizeof( TVFrame ) );
  frame->lpData = NULL;
  switch ( VideoFrame->lpBMIH->biCompression )
  {
    case BI_YUV9:
      break;
    case BI_Y8:
    case BI_Y800:
      frame->lpBMIH = y8yuv9( VideoFrame->lpBMIH , VideoFrame->lpData );
      break;
    case BI_YUV12:
      frame->lpBMIH = yuv12yuv9( VideoFrame->lpBMIH , VideoFrame->lpData );
      break;
    case BI_Y16:
      frame->lpBMIH = y16yuv8( VideoFrame->lpBMIH , VideoFrame->lpData );
    default:
      freeTVFrame( frame );
      return NULL;
  }
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

// VideoNegative
IMPLEMENT_VIDEO_FILTER( VideoNegative , "color&brightness" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* VideoNegative::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  _negative( frame );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

IMPLEMENT_VIDEO_FILTER( Deinterlace , "frame" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* Deinterlace::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  _deinterlace( frame );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}


IMPLEMENT_VIDEO_FILTER( VideoNormalize , "color&brightness" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* VideoNormalize::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  _normalize( frame );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

//IMPLEMENT_VIDEO_FILTER( VideoY16toY8 , "conversion" , TVDB400_PLUGIN_NAME , ; , ; );
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoY16toY8 , "conversion" , TVDB400_PLUGIN_NAME , m_iShift = 8 , );

CDataFrame* VideoY16toY8::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  CVideoFrame* retVal = NULL;
  if ( VideoFrame->lpBMIH->biCompression == BI_Y16 )
  {
    pTVFrame frame = ( pTVFrame ) malloc( sizeof( TVFrame ) );
    frame->lpData = NULL;
    frame->lpBMIH = y16yuv8( VideoFrame->lpBMIH , VideoFrame->lpData , m_iShift );
    retVal = CVideoFrame::Create( frame );
    retVal->CopyAttributes( pDataFrame );
  }
  return retVal;
}

bool VideoY16toY8::PrintProperties( FXString& Text )
{
  FXPropertyKit pc( Text ) ;
  pc.WriteInt( "Shift" , m_iShift ) ;
  Text = pc ;
  return true ;
}

bool VideoY16toY8::ScanProperties( LPCTSTR Text , bool& bInvalidate )
{
  FXPropertyKit pc( Text ) ;
  pc.GetInt( "Shift" , m_iShift ) ;
  return true ;
}

bool VideoY16toY8::ScanSettings( FXString& Text )
{
  Text.Format( "template(Spin(Shift,-7,8))" , m_iShift );
  return true ;
}

IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoY8toY16 , "conversion" , TVDB400_PLUGIN_NAME , m_iShift = 0 , ; );

CDataFrame* VideoY8toY16::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  CVideoFrame* retVal = NULL;
  if ( VideoFrame->lpBMIH->biCompression == BI_Y8
    || VideoFrame->lpBMIH->biCompression == BI_Y800
    || VideoFrame->lpBMIH->biCompression == BI_YUV9
    || VideoFrame->lpBMIH->biCompression == BI_YUV12 )
  {
    pTVFrame frame = ( pTVFrame ) malloc( sizeof( TVFrame ) );
    frame->lpData = NULL;
    frame->lpBMIH = y8y16( VideoFrame->lpBMIH , VideoFrame->lpData , m_iShift );
    retVal = CVideoFrame::Create( frame );
    retVal->CopyAttributes( pDataFrame );
  }
  return retVal;
}
bool VideoY8toY16::PrintProperties( FXString& Text )
{
  FXPropertyKit pc( Text ) ;
  pc.WriteInt( "Shift" , m_iShift ) ;
  Text = pc ;
  return true ;
}

bool VideoY8toY16::ScanProperties( LPCTSTR Text , bool& bInvalidate )
{
  FXPropertyKit pc( Text ) ;
  pc.GetInt( "Shift" , m_iShift ) ;
  return true ;
}

bool VideoY8toY16::ScanSettings( FXString& Text )
{
  Text.Format( "template(Spin(Shift,-7,8))" , m_iShift );
  return true ;
}

IMPLEMENT_VIDEO_FILTER( VideoY8toYUV9 , "conversion" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* VideoY8toYUV9::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  CVideoFrame* retVal = NULL;
  if ( VideoFrame->lpBMIH->biCompression == BI_Y8 )
  {
    pTVFrame frame = ( pTVFrame ) malloc( sizeof( TVFrame ) );
    frame->lpData = NULL;
    frame->lpBMIH = y8yuv9( VideoFrame->lpBMIH , VideoFrame->lpData );
    retVal = CVideoFrame::Create( frame );
    retVal->CopyAttributes( pDataFrame );;
  }
  return retVal;
}


IMPLEMENT_VIDEO_FILTER( Block8 , "frame" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* Block8::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = NULL;
  CVideoFrame* retVal = NULL;

  if ( ( VideoFrame->lpBMIH->biWidth % 8 == 0 ) && ( VideoFrame->lpBMIH->biHeight % 8 == 0 ) )
  {
    frame = makecopyTVFrame( VideoFrame );
  }
  else
  {
    int ow = VideoFrame->lpBMIH->biWidth;
    int oh = VideoFrame->lpBMIH->biHeight;
    int w = ( ow&( ~7 ) ); int h = ( oh&( ~7 ) );
    int isize = w * h;

    switch ( VideoFrame->lpBMIH->biCompression )
    {
      case BI_Y16:
        isize *= 2;
        break;
      case BI_YUV9:
        isize = ( 9 * isize ) / 8;
        break;
      case BI_YUV12:
        isize = ( 12 * isize ) / 8;
        break;

    }

    frame = ( pTVFrame ) malloc( sizeof( TVFrame ) );

    frame->lpData = NULL;
    frame->lpBMIH = ( LPBITMAPINFOHEADER ) malloc( VideoFrame->lpBMIH->biSize + isize );
    memcpy( frame->lpBMIH , VideoFrame->lpBMIH , VideoFrame->lpBMIH->biSize );
    frame->lpBMIH->biWidth = w;
    frame->lpBMIH->biHeight = h;
    frame->lpBMIH->biSizeImage = isize;

    LPBYTE dst = GetData( frame );
    LPBYTE src = GetData( VideoFrame );
    int i;
    for ( i = 0; i < h; i++ )
    {
      memcpy( dst , src , w );
      dst += w; src += ow;
    }
        //dst = GetData(frame)+w*h;
    src = GetData( VideoFrame ) + ow * oh;
    if ( VideoFrame->lpBMIH->biCompression == BI_YUV9 )
    {
      w /= 4; ow /= 4;
      h /= 2;
      for ( i = 0; i < h; i++ )
      {
        memcpy( dst , src , w );
        dst += w; src += ow;
      }
    }
    else if ( VideoFrame->lpBMIH->biCompression == BI_YUV12 )
    {
      w /= 2; ow /= 2;
      for ( i = 0; i < h; i++ )
      {
        memcpy( dst , src , w );
        dst += w; src += ow;
      }
    }
  }
  if ( frame )
  {
    retVal = CVideoFrame::Create( frame );
    retVal->CopyAttributes( pDataFrame );;
  }
  return retVal;
}

IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoFlip , "frame" , TVDB400_PLUGIN_NAME , m_Type = 0 , ;);

#define HORIZ 0
#define VERT  1
#define BOTH  2

CDataFrame* VideoFlip::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = NULL ;
  switch ( m_Type )
  {
    case HORIZ:
      frame = makecopyTVFrame( VideoFrame ) ;
      _fliph( frame );
      break ;
    case VERT:
      frame = makecopyTVFrame( VideoFrame ) ;
      _flipv( frame );
      break ;
    case BOTH:
      frame = _flipboth( VideoFrame ) ;
      break;
  }
  if ( frame )
  {
    CVideoFrame* retVal = CVideoFrame::Create( frame );
    retVal->CopyAttributes( pDataFrame );;
    return retVal;
  }
  return NULL ;
}

bool VideoFlip::ScanSettings( FXString& text )
{
  text = "template(ComboBox(Flip(Vertical(0),Horizontal(1),Both(2))))";
  return true;
}

bool VideoFlip::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "Flip" , m_Type );
  text = pc;
  return true;
}

bool VideoFlip::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "Flip" , m_Type );
  return true;
}

IMPLEMENT_VIDEO_FILTER( HTriggerBinarize , "binarizing" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* HTriggerBinarize::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = htriggerbin( VideoFrame );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

// VideoSimpleBinarize
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoSimpleBinarize , "binarizing" , TVDB400_PLUGIN_NAME , m_Level = 128; m_MaxLevel = 255 , ;);

CDataFrame* VideoSimpleBinarize::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  m_MaxLevel = ( frame->lpBMIH->biCompression == BI_Y16 ) ? 65535 : 255;
  _simplebinarize( frame , m_Level );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

bool VideoSimpleBinarize::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "Level" , m_Level );
  text = pc;
  return true;
}

bool VideoSimpleBinarize::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "Level" , m_Level );
  return true;
}

bool VideoSimpleBinarize::ScanSettings( FXString& text )
{
  text.Format( "template(Spin(Level,0,%d))" , m_MaxLevel );
  return true;
}


IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoNormalizeEx , "color&brightness" , TVDB400_PLUGIN_NAME , min = 0; max = 255; m_MaxLevel = 255 , ;);

CDataFrame* VideoNormalizeEx::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  m_MaxLevel = ( frame->lpBMIH->biCompression == BI_Y16 ) ? 65535 : 255;
  _normalize_ex2( frame , max , min );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

bool VideoNormalizeEx::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "Min" , min );
  pc.WriteInt( "Max" , max );
  text = pc;
  return true;
}

bool VideoNormalizeEx::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "Min" , ( int& ) min );
  pc.GetInt( "Max" , ( int& ) max );
  return true;
}

bool VideoNormalizeEx::ScanSettings( FXString& text )
{
  text.Format( "template(Spin(Min,0,%d),Spin(Max,0,%d))" , m_MaxLevel , m_MaxLevel );
  return true;
}

IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoMassNormalize , "color&brightness" , TVDB400_PLUGIN_NAME , percent = 10 , ;);

CDataFrame* VideoMassNormalize::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  _normalize_mass( frame , percent );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

bool VideoMassNormalize::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "Percent" , percent );
  text = pc;
  return true;
}

bool VideoMassNormalize::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "Percent" , percent );
  return true;
}

bool VideoMassNormalize::ScanSettings( FXString& text )
{
  text.Format( "template(Spin(Percent,0,100))" );
  return true;
}


// VideoClearColor
IMPLEMENT_VIDEO_FILTER( VideoClearColor , "color&brightness" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* VideoClearColor::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  _clearcolor( frame );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( GammaCorrection , "color&brightness" , TVDB400_PLUGIN_NAME , ; m_gamma = 1.0 , ;);

CDataFrame* GammaCorrection::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  _gamma( frame , m_gamma );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

bool GammaCorrection::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  CString tmpS; tmpS.Format( "%g" , m_gamma );
  pc.WriteString( "Gamma" , tmpS );
  text = pc;
  return true;
}

bool GammaCorrection::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  FXString tmpS;
  if ( pc.GetString( "Gamma" , tmpS ) )
    m_gamma = atof( tmpS );
  return true;
}

bool GammaCorrection::ScanSettings( FXString& text )
{
  text.Format( "template(EditBox(Gamma))" );
  return true;
}


// VideoPercentBinarize
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoPercentBinarize , "binarizing" , TVDB400_PLUGIN_NAME , m_Percent = 10 , ;);

CDataFrame* VideoPercentBinarize::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  _percentbinarize( frame , m_Percent );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

bool VideoPercentBinarize::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "Percent" , m_Percent );
  text = pc;
  return true;
}

bool VideoPercentBinarize::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "Percent" , m_Percent );
  return true;
}

bool VideoPercentBinarize::ScanSettings( FXString& text )
{
  text = "template(Spin(Percent,0,100))";
  return true;
}


// VideoMassBinarize
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( MassBinarize , "binarizing" , TVDB400_PLUGIN_NAME , m_Ratio = 1.0; , ;);

CDataFrame* MassBinarize::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  _massbinarize( frame , m_Ratio );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

bool MassBinarize::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  CString tmpS; tmpS.Format( "%g" , m_Ratio );
  pc.WriteString( "Ratio" , tmpS );
  text = pc;
  return true;
}

bool MassBinarize::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  FXString tmpS;
  if ( pc.GetString( "Ratio" , tmpS ) )
    m_Ratio = atof( tmpS );
  return true;
}

bool MassBinarize::ScanSettings( FXString& text )
{
  text.Format( "template(EditBox(Ratio))" );
  return true;
}

//PseudoColor
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( PseudoColor , "color&brightness" , \
  TVDB400_PLUGIN_NAME , \
  m_iMinIntensity = 0 ; m_iMaxIntensity = 255 ; \
  m_pTable = NULL ; m_iUsedMin = -1 ; m_iUsedMax = 01 ; \
  , if ( m_pTable ) delete[] m_pTable ; );

CDataFrame* PseudoColor::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );

  pTVFrame pFrame = makeNewYUV12Frame(
    VideoFrame->lpBMIH->biWidth , VideoFrame->lpBMIH->biHeight ) ;
  memset( GetData( pFrame ) , 0 , GetI8Size( pFrame ) ) ;
  if ( !m_pTable )
  {
    m_pTable = new UVComps[ 65536 ] ;
  }
  if ( m_iMinIntensity != m_iUsedMin || m_iMaxIntensity != m_iUsedMax )
  {
    FormColorTable( m_pTable , m_iMinIntensity , m_iMaxIntensity ) ;
    m_iUsedMin = m_iMinIntensity ;
    m_iUsedMax = m_iMaxIntensity ;
  }
  _pseudocolor( pFrame , VideoFrame , m_pTable ,
    m_iMinIntensity , m_iMaxIntensity ) ;
  CVideoFrame* retVal = CVideoFrame::Create( pFrame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

bool PseudoColor::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  CString tmpS;
  tmpS.Format( "%i" , m_iMinIntensity );
  pc.WriteString( "Min" , tmpS );
  tmpS.Format( "%i" , m_iMaxIntensity );
  pc.WriteString( "Max" , tmpS );
  text = pc;
  return true;
}

bool PseudoColor::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  FXString tmpS;
  if ( pc.GetString( "Min" , tmpS ) )
    m_iMinIntensity = atoi( tmpS );
  if ( pc.GetString( "Max" , tmpS ) )
    m_iMaxIntensity = atoi( tmpS );
  return true;
}

bool PseudoColor::ScanSettings( FXString& text )
{
  text.Format( "template(Spin(Max,0,65535),Spin(Min,0,65535))" );
  return true;
}

//RangeContrast
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( \
  RangeContrast , "color&brightness" , \
  TVDB400_PLUGIN_NAME , \
  m_iMinIntensity = 0 ; m_iMaxIntensity = 255 ; \
  m_pTable = NULL ; m_bFilled = false ; \
  , if ( m_pTable ) delete[] m_pTable ; );

CDataFrame* RangeContrast::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  if ( !VideoFrame || !VideoFrame->lpBMIH )
  {
    ( ( CDataFrame* ) pDataFrame )->AddRef() ;
    return ( ( CDataFrame* ) pDataFrame ) ;
  }

  if ( VideoFrame->lpBMIH->biCompression == BI_Y800
    || VideoFrame->lpBMIH->biCompression == BI_Y8
    || VideoFrame->lpBMIH->biCompression == BI_YUV9
    || VideoFrame->lpBMIH->biCompression == BI_YUV12
    || VideoFrame->lpBMIH->biCompression == BI_Y16 )
  {
    if ( !m_bFilled )
    {
      int iRange = m_iMaxIntensity - m_iMinIntensity ;
      if ( iRange <= 0 )
        return NULL ;
      if ( m_pTable )
      {
        delete[] m_pTable ;
        m_pTable = NULL ;
      }

      m_pTable = new int[ iRange ] ;
      if ( m_pTable )
      {
        double dOutRange = ( VideoFrame->lpBMIH->biCompression != BI_Y16 ) ? 255. : 65535. ;
        double dStep = dOutRange / ( double ) iRange ;
        for ( int i = 0 ; i < iRange ; i++ )
          m_pTable[ i ] = ROUND( i * dStep ) ;
        m_bFilled = true ;
      }
    }
    pTVFrame pOut = makecopyTVFrame( VideoFrame ) ;

    if ( VideoFrame->lpBMIH->biCompression != BI_Y16 )  // 8 bits
    {
      LPBYTE pImage = GetData( pOut ) ;
      LPBYTE pEnd = pImage + GetI8Size( pOut ) ;
      while ( pImage < pEnd )
      {
        if ( *pImage < m_iMinIntensity )
          *pImage = 0 ;
        else if ( *pImage < m_iMaxIntensity )
          *pImage = m_pTable[ *pImage - m_iMinIntensity ] ;
        else
          *pImage = 255 ;
        pImage++ ;
      }
    }
    else
    {
      LPWORD pImage = GetData16( pOut ) ;
      LPWORD pEnd = pImage + GetI8Size( pOut ) ;
      while ( pImage < pEnd )
      {
        if ( *pImage < m_iMinIntensity )
          *pImage = 0 ;
        else if ( *pImage < m_iMaxIntensity )
          *pImage = m_pTable[ *pImage - m_iMinIntensity ] ;
        else
          *pImage = 65535 ;
        pImage++ ;
      }
    }
    CVideoFrame* retVal = CVideoFrame::Create( pOut );
    retVal->CopyAttributes( pDataFrame );;
    CFramesIterator* Iter = pDataFrame->CreateFramesIterator( nulltype );
    if ( Iter )
    {
      CContainerFrame * pOutContainer = CContainerFrame::Create() ;
      pOutContainer->AddFrame( retVal ) ;

      CDataFrame * pNext = NULL ;
      while ( pNext = Iter->Next() )
      {
        if ( pNext != VideoFrame )
        {
          pNext->AddRef() ;
          if ( !pOutContainer )
            pOutContainer = CContainerFrame::Create() ;
          if ( pOutContainer )
            pOutContainer->AddFrame( pNext ) ;
          else
            break ;
        }
      }
      delete Iter ;
      if ( pOutContainer )
      {
        pOutContainer->CopyAttributes( pDataFrame );
        return pOutContainer ;
      }
      return NULL ;
    }
    else
      return retVal;
  }
  else
  {
    ( ( CDataFrame* ) pDataFrame )->AddRef() ;
    return ( ( CDataFrame* ) pDataFrame ) ;
  }
}

bool RangeContrast::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  CString tmpS;
  tmpS.Format( "%i" , m_iMinIntensity );
  pc.WriteString( "Min" , tmpS );
  tmpS.Format( "%i" , m_iMaxIntensity );
  pc.WriteString( "Max" , tmpS );
  text = pc;
  return true;
}

bool RangeContrast::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  FXString tmpS;
  if ( pc.GetString( "Min" , tmpS ) && !tmpS.IsEmpty() )
  {
    int itmp = atoi( tmpS );
    if ( itmp < m_iMaxIntensity )
      m_iMinIntensity = itmp ;
  }
  if ( pc.GetString( "Max" , tmpS ) && !tmpS.IsEmpty() )
  {
    int itmp = atoi( tmpS );
    if ( itmp > m_iMinIntensity && itmp < 65535 )
      m_iMaxIntensity = itmp ;
  }
  m_bFilled = false ;
  return true;
}

bool RangeContrast::ScanSettings( FXString& text )
{
  text.Format( "template(Spin(Max,0,65535),Spin(Min,0,65535))" );
  return true;
}


//colorbalance
IMPLEMENT_VIDEO_FILTER( ColorBalance , "color&brightness" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* ColorBalance::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  colorbalance( frame );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

//colorbalance
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( ColorBalanceM , "color&brightness" , TVDB400_PLUGIN_NAME , m_U = 0; m_V = 0; , ;);

CDataFrame* ColorBalanceM::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  colorshift( frame , m_U , m_V );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

bool ColorBalanceM::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "U" , m_U );
  pc.WriteInt( "V" , m_V );
  text = pc;
  return true;
}

bool ColorBalanceM::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "U" , m_U );
  pc.GetInt( "V" , m_V );
  return true;
}

bool ColorBalanceM::ScanSettings( FXString& text )
{
  text = "template(Spin(U,-128,128),Spin(V,-128,128))";
  return true;
}



IMPLEMENT_VIDEO_FILTER( Equalize , "color&brightness" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* Equalize::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  _equalize_hist( frame );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}



// VideoClearFrames
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoClearFrames , "frame" , TVDB400_PLUGIN_NAME , m_Color = 0; m_Marge = 5 , ;);

CDataFrame* VideoClearFrames::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  _clear_frames( frame , m_Color , m_Marge );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

bool VideoClearFrames::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "Fill_color" , m_Color );
  pc.WriteInt( "Margin" , m_Marge );
  text = pc;
  return true;
}

bool VideoClearFrames::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "Fill_color" , ( int& ) m_Color );
  pc.GetInt( "Margin" , ( int& ) m_Marge );
  return true;
}

bool VideoClearFrames::ScanSettings( FXString& text )
{
  text = "template(Spin(Fill_color,0,255),Spin(Margin,0,100))";
  return true;
}


//VideoSharpen
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoSharpen , "frequency" , TVDB400_PLUGIN_NAME , m_FrNmb = CONOVOLUTION_PARAM , ;);

CDataFrame* VideoSharpen::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );

  pTVFrame frame = _sharpen( VideoFrame , m_FrNmb );

  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

bool VideoSharpen::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "ConvParameter" , m_FrNmb );
  text = pc;
  return true;
}

bool VideoSharpen::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "ConvParameter" , m_FrNmb );
  return true;
}

bool VideoSharpen::ScanSettings( FXString& text )
{
  text = "template(Spin(ConvParameter,1,999))";
  return true;
}


// VideoLowPass
IMPLEMENT_VFILTER_WITH_STDSETUP_ANDASYNC( VideoLowPass ,
  "frequency" , TVDB400_PLUGIN_NAME ,
  m_FrNmb = CONOVOLUTION_PARAM ,
  m_ROI = CRect( 0 , 0 , 0 , 0 ););

CDataFrame* VideoLowPass::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  CVideoFrame* retVal = NULL ;
  m_Protect.Lock() ;
  CRect Intersect ;
  CRect Image( 0 , 0 , GetWidth( VideoFrame ) , GetHeight( VideoFrame ) ) ;
  BOOL bIntersected = Intersect.IntersectRect( m_ROI , Image ) ;
  m_Protect.Unlock() ;
  if ( !bIntersected )
  {
    pTVFrame frame = _lpass( VideoFrame , m_FrNmb );
    retVal = CVideoFrame::Create( frame );
  }
  else
  {
    pTVFrame pForFilter = _cut_rect( VideoFrame , Intersect , FALSE );
    pTVFrame pFiltered = _lpass( pForFilter , m_FrNmb ) ;

    pTVFrame pMatched = _draw_over( VideoFrame , pFiltered ,
      Intersect.left , Intersect.top ) ;
    freeTVFrame( pForFilter ) ;
    freeTVFrame( pFiltered ) ;
    retVal = CVideoFrame::Create( pMatched );
  }
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

bool VideoLowPass::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "ConvParameter" , m_FrNmb );
  CRect RectAsLeftTopAndSizes( m_ROI ) ;
  RectAsLeftTopAndSizes.right -= RectAsLeftTopAndSizes.left ;
  RectAsLeftTopAndSizes.bottom -= RectAsLeftTopAndSizes.top ;
  WriteRect( pc , "ROI" , RectAsLeftTopAndSizes ) ;

  text = pc;
  return true;
}

bool VideoLowPass::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "ConvParameter" , m_FrNmb );
  if (GetRect(pc, "ROI", m_ROI)) // right is width , bottom is height
  {
    m_ROI.right += m_ROI.left; // m_ROI is rectangle as CRect
    m_ROI.bottom += m_ROI.top; // 
    _correct_rect4(m_ROI);
  }
  return true;
}

bool VideoLowPass::ScanSettings( FXString& text )
{
  text = "template(Spin(ConvParameter,1,999),EditBox(ROI))";
  return true;
}

void VideoLowPass::AsyncTransaction(
  CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  CTextFrame * pText = pParamFrame->GetTextFrame( "SetROI" ) ;
  if ( pText )
  {
    FXPropKit2 pk = pText->GetString() ;
    CRect ROI ;
    if ( GetRect( pk , "ROI" , ROI ) )
    {
      ROI.right += ROI.left ;
      ROI.bottom += ROI.top ;
      _correct_rect4( ROI ) ;
      m_Protect.Lock() ;
      m_ROI = ROI ;
      m_Protect.Unlock() ;
    }
  }
  pParamFrame->Release( pParamFrame );
} ;


// VideoLowPassM
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoLowPassM , "frequency" , TVDB400_PLUGIN_NAME , m_FrNmb = CONOVOLUTION_PARAM; m_iMatrixSide = 0; m_pSums = NULL;  m_dwLastWidth = 0; , if ( m_pSums ) delete[] m_pSums ;);

CDataFrame* VideoLowPassM::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  if ( m_iMatrixSide == 0 )
  {
    pTVFrame frame = _lpass( VideoFrame , m_FrNmb );
    CVideoFrame* retVal = CVideoFrame::Create( frame );
    retVal->CopyAttributes( pDataFrame );;
    return retVal;
  }
  else if ( m_iMatrixSide > 2 )
  {
    int dwWidth = GetWidth( VideoFrame ) ;
    if ( m_dwLastWidth < ( DWORD ) dwWidth )
    {
      if ( m_pSums )
      {
        delete[] m_pSums ;
        m_pSums = NULL ;
      }
    }
    if ( !m_pSums )
    {
      m_pSums = new DWORD[ dwWidth ] ;
      if ( !m_pSums )
        return NULL;
      m_dwLastWidth = dwWidth ;
    }
    memset( m_pSums , 0 , dwWidth * sizeof( DWORD ) );
    pTVFrame pResult = makecopyTVFrame( VideoFrame );
    int iHeight = pResult->lpBMIH->biHeight;

    switch ( VideoFrame->lpBMIH->biCompression )
    {
      case BI_YUV9:
      case BI_Y8:
      {
        LPBYTE src = GetData( VideoFrame );
        LPBYTE dst = GetData( pResult );
        LPBYTE pRow ;
        LPBYTE pDest ;
        LPBYTE pCoordForSub ;

        int iX , iY ;
        for ( iY = 0 ; iY < m_iMatrixSide / 2 ; iY++ )
        {
          pRow = src + iY * dwWidth ;
          for ( iX = 0 ; iX < dwWidth ; iX++ )
          {
            DWORD dwVal = *( pRow + iX ) ;
            m_pSums[ iX ] += dwVal ;
          }
        }
        for ( ; iY < m_iMatrixSide ; iY++ )
        {
          DWORD dwSum = 0 ;
          pRow = src + iY * dwWidth ;
          pDest = dst + ( iY - ( m_iMatrixSide / 2 ) ) * dwWidth ;
          for ( iX = 0 ; iX < m_iMatrixSide / 2 ; iX++ )
          {
            DWORD dwVal = *( pRow + iX ) ;
            m_pSums[ iX ] += dwVal ;
            dwSum += m_pSums[ iX ] ;
          }
          int iDivider = ( iY + 1 )*( iX + 1 );
          for ( ; iX < m_iMatrixSide ; iX++ )
          {
            DWORD dwVal = *( pRow + iX ) ;
            m_pSums[ iX ] += dwVal ;
            dwSum += m_pSums[ iX ] ;
            *( pDest++ ) = ( BYTE ) ( dwSum / iDivider ) ;
            iDivider += iY + 1  ;
          }
          iDivider = ( iY + 1 )* m_iMatrixSide ;
          for ( ; iX < dwWidth ; iX++ )
          {
            DWORD dwVal = *( pRow + iX ) ;
            m_pSums[ iX ] += dwVal ;
            dwSum += m_pSums[ iX ] - m_pSums[ iX - m_iMatrixSide ] ;
            *( pDest++ ) = ( BYTE ) ( dwSum / iDivider ) ;
          }
          iDivider -= ( iY + 1 ) ;
          for ( ; iX < dwWidth + m_iMatrixSide / 2 ; iX++ )
          {
            dwSum -= m_pSums[ iX - m_iMatrixSide ] ;
            *( pDest++ ) = ( BYTE ) ( dwSum / iDivider ) ;
            iDivider -= ( iY + 1 ) ;
          }
        }
        for ( ; iY < iHeight ; iY++ )
        {
          DWORD dwSum = 0 ;
          pRow = src + iY * dwWidth ;
          pDest = dst + ( iY - m_iMatrixSide / 2 ) * dwWidth ;
          pCoordForSub = pRow - dwWidth * m_iMatrixSide ;
          for ( iX = 0 ; iX < m_iMatrixSide / 2 ; iX++ )
          {
            DWORD dwVal = *( pRow + iX ) ;
            m_pSums[ iX ] += dwVal - *( pCoordForSub + iX )  ;
            dwSum += m_pSums[ iX ] ;
          }
          int iDivider = m_iMatrixSide * ( iX + 1 ) ;
          for ( ; iX < m_iMatrixSide ; iX++ )
          {
            DWORD dwVal = *( pRow + iX ) ;
            m_pSums[ iX ] += dwVal - *( pCoordForSub + iX ) ;
            dwSum += m_pSums[ iX ] ;
            *( pDest++ ) = ( BYTE ) ( dwSum / iDivider ) ;
            iDivider += m_iMatrixSide ;
          }
          iDivider = m_iMatrixSide * m_iMatrixSide ;
          for ( ; iX < dwWidth ; iX++ )
          {
            DWORD dwVal = *( pRow + iX ) ;
            m_pSums[ iX ] += dwVal - *( pCoordForSub + iX ) ;
            dwSum += m_pSums[ iX ] - m_pSums[ iX - m_iMatrixSide ] ;
            *( pDest++ ) = ( BYTE ) ( dwSum / iDivider ) ;
          }
          for ( ; iX < dwWidth + m_iMatrixSide / 2 ; iX++ )
          {
            dwSum -= m_pSums[ iX - m_iMatrixSide ] ;
            iDivider -= m_iMatrixSide ;
            *( pDest++ ) = ( BYTE ) ( dwSum / iDivider ) ;
          }
        }
        for ( ; iY < iHeight + m_iMatrixSide / 2 ; iY++ )
        {
          DWORD dwSum = 0 ;
          pDest = dst + ( iY - ( m_iMatrixSide / 2 ) ) * dwWidth ;
          pCoordForSub = src + ( iY - m_iMatrixSide ) * dwWidth ;
          for ( iX = 0 ; iX < m_iMatrixSide / 2 ; iX++ )
          {
            m_pSums[ iX ] -= *( pCoordForSub + iX ) ;
            dwSum += m_pSums[ iX ] ;
          }
          int iYMult = ( iHeight + m_iMatrixSide - iY - 1 ) ;
          for ( ; iX < m_iMatrixSide ; iX++ )
          {
            m_pSums[ iX ] -= *( pCoordForSub + iX ) ;
            dwSum += m_pSums[ iX ] ;
            int iDivider = ( iX + 1 ) * iYMult ;
            *( pDest++ ) = ( BYTE ) ( dwSum / iDivider ) ;
          }
          int iDivider = m_iMatrixSide * iYMult ;
          for ( ; iX < dwWidth ; iX++ )
          {
            m_pSums[ iX ] -= *( pCoordForSub + iX );
            dwSum += m_pSums[ iX ] - m_pSums[ iX - m_iMatrixSide ] ;
            *( pDest++ ) = ( BYTE ) ( dwSum / iDivider ) ;
          }
          iDivider -= m_iMatrixSide - ( iY - iHeight ) ;
          for ( ; iX < dwWidth + m_iMatrixSide / 2 ; iX++ )
          {
            dwSum -= m_pSums[ iX - m_iMatrixSide ] ;
            *( pDest++ ) = ( BYTE ) ( dwSum / iDivider ) ;
            iDivider -= m_iMatrixSide - ( iY - iHeight ) ;
          }
        }
        CVideoFrame* retVal = CVideoFrame::Create( pResult );
        retVal->CopyAttributes( pDataFrame );;
        return retVal;
      }

      case BI_Y16:
      {
        LPWORD src = ( LPWORD ) GetData( VideoFrame );
        LPWORD dst = ( LPWORD ) GetData( pResult );
        LPWORD pRow ;
        LPWORD pDest ;
        LPWORD pCoordForSub ;

        int iX , iY ;
        for ( iY = 0 ; iY < m_iMatrixSide / 2 ; iY++ )
        {
          pRow = src + iY * dwWidth ;
          for ( iX = 0 ; iX < dwWidth ; iX++ )
          {
            DWORD dwVal = *( pRow + iX ) ;
            m_pSums[ iX ] += dwVal ;
          }
        }
        for ( ; iY < m_iMatrixSide ; iY++ )
        {
          DWORD dwSum = 0 ;
          pRow = src + iY * dwWidth ;
          pDest = dst + ( iY - ( m_iMatrixSide / 2 ) ) * dwWidth ;
          for ( iX = 0 ; iX < m_iMatrixSide / 2 ; iX++ )
          {
            DWORD dwVal = *( pRow + iX ) ;
            m_pSums[ iX ] += dwVal ;
            dwSum += m_pSums[ iX ] ;
          }
          int iDivider = ( iY + 1 )*( iX + 1 );
          for ( ; iX < m_iMatrixSide ; iX++ )
          {
            DWORD dwVal = *( pRow + iX ) ;
            m_pSums[ iX ] += dwVal ;
            dwSum += m_pSums[ iX ] ;
            *( pDest++ ) = ( WORD ) ( dwSum / iDivider ) ;
            iDivider += iY + 1  ;
          }
          iDivider = ( iY + 1 )* m_iMatrixSide ;
          for ( ; iX < dwWidth ; iX++ )
          {
            DWORD dwVal = *( pRow + iX ) ;
            m_pSums[ iX ] += dwVal ;
            dwSum += m_pSums[ iX ] - m_pSums[ iX - m_iMatrixSide ] ;
            *( pDest++ ) = ( WORD ) ( dwSum / iDivider ) ;
          }
          iDivider -= ( iY + 1 ) ;
          for ( ; iX < dwWidth + m_iMatrixSide / 2 ; iX++ )
          {
            dwSum -= m_pSums[ iX - m_iMatrixSide ] ;
            *( pDest++ ) = ( WORD ) ( dwSum / iDivider ) ;
            iDivider -= ( iY + 1 ) ;
          }
        }
        for ( ; iY < iHeight ; iY++ )
        {
          DWORD dwSum = 0 ;
          pRow = src + iY * dwWidth ;
          pDest = dst + ( iY - m_iMatrixSide / 2 ) * dwWidth ;
          pCoordForSub = pRow - dwWidth * m_iMatrixSide ;
          for ( iX = 0 ; iX < m_iMatrixSide / 2 ; iX++ )
          {
            DWORD dwVal = *( pRow + iX ) ;
            m_pSums[ iX ] += dwVal - *( pCoordForSub + iX )  ;
            dwSum += m_pSums[ iX ] ;
          }
          int iDivider = m_iMatrixSide * ( iX + 1 ) ;
          for ( ; iX < m_iMatrixSide ; iX++ )
          {
            DWORD dwVal = *( pRow + iX ) ;
            m_pSums[ iX ] += dwVal - *( pCoordForSub + iX ) ;
            dwSum += m_pSums[ iX ] ;
            *( pDest++ ) = ( WORD ) ( dwSum / iDivider ) ;
            iDivider += m_iMatrixSide ;
          }
          iDivider = m_iMatrixSide * m_iMatrixSide ;
          for ( ; iX < dwWidth ; iX++ )
          {
            DWORD dwVal = *( pRow + iX ) ;
            m_pSums[ iX ] += dwVal - *( pCoordForSub + iX ) ;
            dwSum += m_pSums[ iX ] - m_pSums[ iX - m_iMatrixSide ] ;
            *( pDest++ ) = ( WORD ) ( dwSum / iDivider ) ;
          }
          for ( ; iX < dwWidth + m_iMatrixSide / 2 ; iX++ )
          {
            dwSum -= m_pSums[ iX - m_iMatrixSide ] ;
            iDivider -= m_iMatrixSide ;
            *( pDest++ ) = ( WORD ) ( dwSum / iDivider ) ;
          }
        }
        for ( ; iY < iHeight + m_iMatrixSide / 2 ; iY++ )
        {
          DWORD dwSum = 0 ;
          pDest = dst + ( iY - ( m_iMatrixSide / 2 ) ) * dwWidth ;
          pCoordForSub = src + ( iY - m_iMatrixSide ) * dwWidth ;
          for ( iX = 0 ; iX < m_iMatrixSide / 2 ; iX++ )
          {
            m_pSums[ iX ] -= *( pCoordForSub + iX ) ;
            dwSum += m_pSums[ iX ] ;
          }
          int iYMult = ( iHeight + m_iMatrixSide - iY - 1 ) ;
          for ( ; iX < m_iMatrixSide ; iX++ )
          {
            m_pSums[ iX ] -= *( pCoordForSub + iX ) ;
            dwSum += m_pSums[ iX ] ;
            int iDivider = ( iX + 1 ) * iYMult ;
            *( pDest++ ) = ( WORD ) ( dwSum / iDivider ) ;
          }
          int iDivider = m_iMatrixSide * iYMult ;
          for ( ; iX < dwWidth ; iX++ )
          {
            m_pSums[ iX ] -= *( pCoordForSub + iX );
            dwSum += m_pSums[ iX ] - m_pSums[ iX - m_iMatrixSide ] ;
            *( pDest++ ) = ( WORD ) ( dwSum / iDivider ) ;
          }
          iDivider -= m_iMatrixSide - ( iY - iHeight ) ;
          for ( ; iX < dwWidth + m_iMatrixSide / 2 ; iX++ )
          {
            dwSum -= m_pSums[ iX - m_iMatrixSide ] ;
            *( pDest++ ) = ( WORD ) ( dwSum / iDivider ) ;
            iDivider -= m_iMatrixSide - ( iY - iHeight ) ;
          }
        }
        CVideoFrame* retVal = CVideoFrame::Create( pResult );
        retVal->CopyAttributes( pDataFrame );;
        return retVal;
      }
    }
  }
  return NULL;
}

bool VideoLowPassM::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "ConvParameter" , m_FrNmb );
  pc.WriteInt( "MatrixSide" , m_iMatrixSide );
  text = pc;
  return true;
}

bool VideoLowPassM::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "ConvParameter" , m_FrNmb );
  pc.GetInt( "MatrixSide" , m_iMatrixSide );
  return true;
}

bool VideoLowPassM::ScanSettings( FXString& text )
{
  text = "template(Spin(ConvParameter,0,999),Spin(MatrixSide,0,100))";
  return true;
}


// VideoHighPass
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoHighPass , "frequency" , TVDB400_PLUGIN_NAME , m_FrNmb = CONOVOLUTION_PARAM , ;);

CDataFrame* VideoHighPass::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = _hpass( VideoFrame , m_FrNmb );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

bool VideoHighPass::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "ConvParameter" , m_FrNmb );
  text = pc;
  return true;
}

bool VideoHighPass::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "ConvParameter" , m_FrNmb );
  return true;
}

bool VideoHighPass::ScanSettings( FXString& text )
{
  text = "template(Spin(ConvParameter,1,999))";
  return true;
}


// Video81PassFilter
IMPLEMENT_VIDEO_FILTER( Video81PassFilter , "edges" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* Video81PassFilter::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = _8_1_passfilter( VideoFrame );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

// VideoHighPass1DV
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoHighPass1DV , "frequency" , TVDB400_PLUGIN_NAME , m_FrNmb = CONOVOLUTION_PARAM; , ;);

CDataFrame* VideoHighPass1DV::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = _hpass_1DV( VideoFrame , m_FrNmb );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

bool VideoHighPass1DV::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "ConvParameter" , m_FrNmb );
  text = pc;
  return true;
}

bool VideoHighPass1DV::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "ConvParameter" , m_FrNmb );
  return true;
}

bool VideoHighPass1DV::ScanSettings( FXString& text )
{
  text = "template(Spin(ConvParameter,1,999))";
  return true;
}

// VideoLowPass1DH
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoLowPass1DH , "frequency" , TVDB400_PLUGIN_NAME , m_FrNmb = CONOVOLUTION_PARAM , ;);

CDataFrame* VideoLowPass1DH::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = _lpass_1DH( VideoFrame , m_FrNmb );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

bool VideoLowPass1DH::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "ConvParameter" , m_FrNmb );
  text = pc;
  return true;
}

bool VideoLowPass1DH::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "ConvParameter" , m_FrNmb );
  return true;
}

bool VideoLowPass1DH::ScanSettings( FXString& text )
{
  text = "template(Spin(ConvParameter,1,999))";
  return true;
}

// VideoHighPass1DH
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoHighPass1DH , "frequency" , TVDB400_PLUGIN_NAME , m_FrNmb = CONOVOLUTION_PARAM , ;);

CDataFrame* VideoHighPass1DH::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = _hpass_1DH( VideoFrame , m_FrNmb );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

bool VideoHighPass1DH::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "ConvParameter" , m_FrNmb );
  text = pc;
  return true;
}

bool VideoHighPass1DH::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "ConvParameter" , m_FrNmb );
  return true;
}

bool VideoHighPass1DH::ScanSettings( FXString& text )
{
  text = "template(Spin(ConvParameter,1,999))";
  return true;
}

// VideoLowPass1DV
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoLowPass1DV , "frequency" , TVDB400_PLUGIN_NAME , m_FrNmb = CONOVOLUTION_PARAM , ;);

CDataFrame* VideoLowPass1DV::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = _lpass_1DV( VideoFrame , m_FrNmb );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

bool VideoLowPass1DV::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "ConvParameter" , m_FrNmb );
  text = pc;
  return true;
}

bool VideoLowPass1DV::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "ConvParameter" , m_FrNmb );
  return true;
}

bool VideoLowPass1DV::ScanSettings( FXString& text )
{
  text = "template(Spin(ConvParameter,1,999))";
  return true;
}

// VideoEdgeDetector
IMPLEMENT_VIDEO_FILTER( VideoEdgeDetector , "edges" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* VideoEdgeDetector::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  if ( frame )
    _edge_detector( frame );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}



// VideoEdgeDetector
IMPLEMENT_VIDEO_FILTER( VideoFeatureDetector , "featuredetectors" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* VideoFeatureDetector::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );

  pTVFrame frame = _FeatureDetector( VideoFrame );

  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}


//VideoFlatten
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoFlatten , "color&brightness" , TVDB400_PLUGIN_NAME , m_Times = CONOVOLUTION_PARAM; , ;);

CDataFrame* VideoFlatten::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );

  pTVFrame frame = makecopyTVFrame( VideoFrame );
  if ( frame )
    _enchance2( frame , m_Times );

  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

bool VideoFlatten::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "ConvParameter" , m_Times );
  text = pc;
  return true;
}

bool VideoFlatten::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "ConvParameter" , m_Times );
  return true;
}

bool VideoFlatten::ScanSettings( FXString& text )
{
  text = "template(Spin(ConvParameter,1,1000))";
  return true;
}


IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoFlatten2 , "color&brightness" , TVDB400_PLUGIN_NAME , m_Times = CONOVOLUTION_PARAM / 100; , ;);

CDataFrame* VideoFlatten2::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  if ( frame )
    _enchance3( frame , m_Times );
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );
  return retVal;
}

bool VideoFlatten2::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "ConvParameter" , m_Times );
  text = pc;
  return true;
}

bool VideoFlatten2::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "ConvParameter" , m_Times );
  return true;
}

bool VideoFlatten2::ScanSettings( FXString& text )
{
  text = "template(Spin(ConvParameter,1,10))";
  return true;
}


// VideoEnlarge
IMPLEMENT_VIDEO_FILTER( VideoEnlarge , "resample" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* VideoEnlarge::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  if ( !_enlarge( frame ) )
  {
    freeTVFrame( frame );
    return NULL;;
  }
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );
  return retVal;
}

// VideoDiminish
IMPLEMENT_VIDEO_FILTER( VideoDiminish , "resample" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* VideoDiminish::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  if ( !_diminish( frame ) )
  {
    freeTVFrame( frame );
    return NULL;;
  }
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );
  return retVal;
}

// VideoDiminishX
IMPLEMENT_VIDEO_FILTER( VideoDiminishX , "resample" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* VideoDiminishX::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  if ( !_diminishX( frame ) )
  {
    freeTVFrame( frame );
    return NULL;;
  }
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );
  return retVal;
}

// VideoEnlargeY
IMPLEMENT_VIDEO_FILTER( VideoEnlargeY , "resample" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* VideoEnlargeY::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  if ( !_enlargeY( frame ) )
  {
    freeTVFrame( frame );
    return NULL;;
  }
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );
  return retVal;
}

// VideoResample
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoResample , "resample" , TVDB400_PLUGIN_NAME , newWidth = 320; newHeight = 240 , ;);

CDataFrame* VideoResample::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  if ( !_resample( frame , newWidth , newHeight ) )
  {
    freeTVFrame( frame );
    return NULL;;
  }
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );
  return retVal;
}

bool VideoResample::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "Width" , newWidth );
  pc.WriteInt( "Height" , newHeight );
  text = pc;
  return true;
}

bool VideoResample::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "Width" , newWidth );
  pc.GetInt( "Height" , newHeight );
  return true;
}

bool VideoResample::ScanSettings( FXString& text )
{
  text = "template(Spin(Width,64,1600),Spin(Height,48,1200))";
  return true;
}

// VideoErode
//IMPLEMENT_VIDEO_FILTER(VideoErode, "logic", TVDB400_PLUGIN_NAME, ;, ;);
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoErode , "logic" , TVDB400_PLUGIN_NAME , m_iNInversed = 1 , ;);

CDataFrame* VideoErode::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame pFrame = makecopyTVFrame( VideoFrame );
  int iIterCnt = 0 ;
  do
  {
    if ( !_erode( pFrame ) )
    {
      freeTVFrame( pFrame );
      return NULL;;
    }
  } while ( ++iIterCnt < m_iNInversed) ;
  CVideoFrame* retVal = CVideoFrame::Create( pFrame );
  retVal->CopyAttributes( pDataFrame );
  return retVal;
}

bool VideoErode::PrintProperties( FXString& text )
{
  CFilterGadget::PrintProperties( text );
  FXPropertyKit pk;
  pk.WriteInt( "NInversed" , m_iNInversed );
  text += pk;
  return true;
}

bool VideoErode::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  CFilterGadget::ScanProperties( text , Invalidate );
  FXPropertyKit pk( text );
  pk.GetInt( "NInversed" , m_iNInversed );
  return true;
}

bool VideoErode::ScanSettings( FXString& text )
{
  text = "template(Spin(NInversed,1,5))";
  return true;
}

// VideoDilate
//IMPLEMENT_VIDEO_FILTER(VideoDilate, "logic", TVDB400_PLUGIN_NAME, ;, ;);
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoDilate , "logic" , TVDB400_PLUGIN_NAME , m_iNInversed = 1 , ;);

CDataFrame* VideoDilate::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  if ( !_dilate( frame , m_iNInversed ) )
  {
    freeTVFrame( frame );
    return NULL;;
  }
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}
bool VideoDilate::PrintProperties( FXString& text )
{
  CFilterGadget::PrintProperties( text );
  FXPropertyKit pk;
  pk.WriteInt( "NInversed" , m_iNInversed );
  text += pk;
  return true;
}

bool VideoDilate::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  CFilterGadget::ScanProperties( text , Invalidate );
  FXPropertyKit pk( text );
  pk.GetInt( "NInversed" , m_iNInversed );
  return true;
}

bool VideoDilate::ScanSettings( FXString& text )
{
  text = "template(Spin(NInversed,1,5))";
  return true;
}

// VideoErodeVertical
IMPLEMENT_VIDEO_FILTER( VideoErodeVertical , "logic" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* VideoErodeVertical::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  if ( !_erodev( frame ) )
  {
    freeTVFrame( frame );
    return NULL;;
  }
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

// VideoDilateVertical
IMPLEMENT_VIDEO_FILTER( VideoDilateVertical , "logic" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* VideoDilateVertical::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );

  if ( !_dilatev( frame ) )
  {
    freeTVFrame( frame );
    return NULL;;
  }
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

// VideoErodeHorizontal
IMPLEMENT_VIDEO_FILTER( VideoErodeHorizontal , "logic" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* VideoErodeHorizontal::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );
  if ( !_erodeh( frame ) )
  {
    freeTVFrame( frame );
    return NULL;;
  }
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

// VideoDilateHorizontal
IMPLEMENT_VIDEO_FILTER( VideoDilateHorizontal , "logic" , TVDB400_PLUGIN_NAME , ; , ;);

CDataFrame* VideoDilateHorizontal::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );

  if ( !_dilateh( frame ) )
  {
    freeTVFrame( frame );
    return NULL;
  }
  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

//VideoHMeander
IMPLEMENT_VIDEO_FILTER_WITH_STDSETUP( VideoHMeander , "convolution" , TVDB400_PLUGIN_NAME , m_nQSize = 25 , ;);

CDataFrame* VideoHMeander::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );

  pTVFrame frame = _hmeander( VideoFrame , m_nQSize );

  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );;
  return retVal;
}

bool VideoHMeander::PrintProperties( FXString& text )
{
  CFilterGadget::PrintProperties( text );
  FXPropertyKit pk;
  pk.WriteInt( "QSize" , m_nQSize );
  text += pk;
  return true;
}

bool VideoHMeander::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  CFilterGadget::ScanProperties( text , Invalidate );
  FXPropertyKit pk( text );
  pk.GetInt( "QSize" , m_nQSize );
  return true;
}

bool VideoHMeander::ScanSettings( FXString& text )
{
  text = "template(Spin(QSize,1,100))";
  return true;
}

#include "StdAfx.h"
#include <gadgets\VideoFrame.h>
#include <gadgets\MetafileFrame.h>
#include "SlideViewRender.h"
#include <imageproc\rotate.h>

CSlideViewRender::CSlideViewRender( LPCTSTR name ) :
CSlideView( name ) ,
m_pDispEventFunc( NULL ) ,
m_pDispEventParam( NULL ) ,
m_ShowMetafiles( false ) 
{
}

CSlideViewRender::~CSlideViewRender( void )
{
  m_FramesBuffer.RemoveAll();
}

void CSlideViewRender::Render( const CDataFrame* pDataFrame )
{
  if ( ( pDataFrame ) && ( !Tvdb400_IsEOS( pDataFrame ) ) )
  {
    int PrevCntr = m_Cntr;
    const FXPropertyKit * pAttrib = pDataFrame->Attributes() ;
    int iRemove = 0 ;
    if ( !pAttrib->IsEmpty()
      && pAttrib->GetInt( _T( "RemoveAll" ) , iRemove )
      && iRemove )
    {
      SetResetData() ;
    }
    CVideoFrame* videoFrame = NULL ;
    if ( m_ShowMetafiles )
    {
      videoFrame = MergeLayers( pDataFrame );
      if ( videoFrame )
      {
        LoadFrame( videoFrame );
        videoFrame->Release();
      }
    }
    else
    {
      videoFrame = (CVideoFrame*)pDataFrame->GetVideoFrame( DEFAULT_LABEL );
      if ( videoFrame )
        LoadFrame( videoFrame );
    }
    if ( ( m_FramesBuffer.GetSize() != GetSlidesLen() ) || m_bResetData )
    {
      TRACE( "+++ CSlideViewRender::Render Reset buffer len %d!=%d\n" ,
        m_FramesBuffer.GetSize() , GetSlidesLen() );
      m_FramesBuffer.RemoveAll();
      m_FramesBuffer.SetSize( GetSlidesLen() );
    }
    if ( videoFrame )
    {
      ( ( CDataFrame* )pDataFrame )->AddRef();
      if ( m_Shift )
      {
        m_FramesBuffer.InsertAt( 0 , ( CDataFrame* )pDataFrame );
        m_FramesBuffer.RemoveAt( GetSlidesLen() );
        if ( ( m_pDispEventFunc ) && ( m_SelectedItem != -1 ) )
          m_pDispEventFunc( m_FramesBuffer.GetAt( m_SelectedItem ) , m_pDispEventParam );
      }
      else
      {
        m_FramesBuffer.SetAt( PrevCntr , ( CDataFrame* )pDataFrame );
        if ( ( m_pDispEventFunc ) && ( PrevCntr == m_SelectedItem ) )
          m_pDispEventFunc( m_FramesBuffer.GetAt( m_SelectedItem ) , m_pDispEventParam );
      }
    }
  }
}

void   CSlideViewRender::OnItemSelected()
{
  m_FramesBuffer.Lock();
  if ( m_pDispEventFunc )
    m_pDispEventFunc( m_FramesBuffer.GetAt( m_SelectedItem ) , m_pDispEventParam );
  m_FramesBuffer.Unlock();
}

void CSlideViewRender::ApplySettings(
  bool   Shift ,
  bool   Rescale ,
  double Scale ,
  bool   Monochrome ,
  int    TargetWidth ,
  int    FramesLen ,
  int    FramesInRow ,
  bool   ShowMetafiles ,
  int    iIntegrationRadius
  )
{
  bool rescalereq = false;
  //FXAutolock al(m_LockDrData);
  CAutoLockMutex al( m_LockOMutex , 1000
  #ifdef _DEBUG
    , "CSlideViewRender::ApplySettings"
  #endif
  ) ;
  SetScale( Scale );
  SetMonochrome( Monochrome );
  rescalereq |= m_Shift != Shift;
  m_Shift = Shift;
  rescalereq |= m_Rescale != Rescale;
  m_Rescale = Rescale;
  rescalereq |= m_TargetWidth != TargetWidth;
  m_TargetWidth = TargetWidth;
  rescalereq |= m_FramesLen != FramesLen;
  m_FramesLen = FramesLen;
  rescalereq |= m_FramesInRow != FramesInRow;
  m_FramesInRow = FramesInRow;
  rescalereq |= m_ShowMetafiles != ShowMetafiles;
  m_ShowMetafiles = ShowMetafiles;
  m_iIntegrationRadius = iIntegrationRadius ;
  if ( rescalereq )
    OnBufferReset();
}

CVideoFrame* CSlideViewRender::MergeLayers( const CDataFrame* pDataFrame )
{
  CDC mdc; // memory DC
  mdc.CreateCompatibleDC( NULL );

  CBitmap cbm;
  const CVideoFrame* vf = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  LPBITMAPINFOHEADER pBMP;
  if ( vf->lpBMIH->biCompression == BI_Y16 )
  {
    pBMP = y16rgb24( vf->lpBMIH , vf->lpData );
    _fliph( pBMP );
  }
  else if ( vf->lpBMIH->biCompression == BI_Y8 )
  {
    pBMP = y8rgb24( vf->lpBMIH , vf->lpData );
    _fliph( pBMP );
  }
  else if ( vf->lpBMIH->biCompression == BI_YUV12 )
  {
    pBMP = yuv12rgb24( vf->lpBMIH , vf->lpData );
    _fliph( pBMP );
  }
  else if ( vf->lpBMIH->biCompression == BI_YUV9 )
  {
    pBMP = yuv9rgb24( vf->lpBMIH , vf->lpData );
    _fliph( pBMP );
  }
  LPBITMAPINFOHEADER pBMP32 = rgb24rdb32( pBMP );
  free( pBMP );

  VERIFY( cbm.CreateBitmap( GetWidth( vf ) , GetHeight( vf ) , 1 , 32 , LPBYTE( pBMP32 ) + pBMP32->biSize ) == TRUE );

  BITMAP bm;
  cbm.GetObject( sizeof( BITMAP ) , &bm );
  CBitmap* pOld = ( CBitmap* )mdc.SelectObject( &cbm );
  { // Draw metafiles  DrawMetafiles(mdc.m_hDC);
    DWORD width = bm.bmWidth;
    DWORD height = bm.bmHeight;

    CFramesIterator* mfIter = pDataFrame->CreateFramesIterator( metafile );
    if ( mfIter )
    {
      CMetafileFrame* mfFrame = ( CMetafileFrame* )mfIter->Next( DEFAULT_LABEL );
      bool MapModeChanged = false;
      int iMap = ::GetMapMode( mdc.m_hDC );
      if ( iMap == MM_TEXT )
      {
        ::SetMapMode( mdc.m_hDC , MM_HIMETRIC );
        MapModeChanged = true;
      }
      CPoint tl = 0;
      DPtoLP( mdc.m_hDC , &tl , 1 );

      CPoint tstPoint( 1000 , 1000 );
      DPtoLP( mdc.m_hDC , &tstPoint , 1 );

      CRgn   rgn;
      VERIFY( rgn.CreateRectRgn( 0 , 0 , 0 + ( int )( width * 1 + 0.5 ) , 0 + ( int )( height * 1 + 0.5 ) ) );
      ::SelectClipRgn( mdc.m_hDC , rgn );

      CPoint br( tl.x + ( int )( width + 0.5 ) , tl.y - ( int )( height + 0.5 ) );

      while ( mfFrame )
      {
        CRect clRect( 0 , 0 , width , height );

        HENHMETAFILE hmf = mfFrame->GetMF();

        if ( hmf )
          PlayEnhMetaFile( mdc.m_hDC , hmf , CRect( tl.x , tl.y , br.x , br.y ) );
        DeleteEnhMetaFile( hmf );

        mfFrame = ( CMetafileFrame* )mfIter->Next( DEFAULT_LABEL );
      }
      delete mfIter; mfIter = NULL;
      if ( MapModeChanged )
        CDC::FromHandle( mdc.m_hDC )->SetMapMode( iMap );
    }
  }
  mdc.SelectObject( pOld );

  pTVFrame tvf = ( pTVFrame )malloc( sizeof( TVFrame ) );
  int isize = sizeof( BITMAPINFOHEADER ) + bm.bmWidthBytes*bm.bmHeight;
  tvf->lpBMIH = ( LPBITMAPINFOHEADER )malloc( isize );
  tvf->lpData = NULL;
  memset( tvf->lpBMIH , 0 , isize );
  tvf->lpBMIH->biSize = sizeof( BITMAPINFOHEADER );
  tvf->lpBMIH->biWidth = bm.bmWidth;
  tvf->lpBMIH->biHeight = bm.bmHeight;
  tvf->lpBMIH->biPlanes = bm.bmPlanes;
  tvf->lpBMIH->biBitCount = bm.bmBitsPixel;
  LPBYTE data = ( LPBYTE )tvf->lpBMIH + sizeof( BITMAPINFOHEADER );
  cbm.GetBitmapBits( bm.bmWidthBytes*bm.bmHeight , data );
  //compress2yuv9(tvf);
  DeleteObject( cbm );
  _fliph( tvf );
  free( pBMP32 );
  CVideoFrame* pFrame = CVideoFrame::Create( tvf );
  pFrame->CopyAttributes( vf );
  return pFrame;
}
double CSlideViewRender::GetAverageValue(
  int iBufIndex , int iX , int iY )
{
  if ( m_iIntegrationRadius >= 0
    && ( 0 <= iBufIndex ) && ( iBufIndex < m_FramesBuffer.GetCount() ) )
  {
    CDataFrame * pData = m_FramesBuffer[ iBufIndex ] ;
    if ( pData )
    {
      CVideoFrame * pVF = pData->GetVideoFrame() ;
      if ( pVF && pVF->lpBMIH )
      {
        DWORD dwWidth = GetWidth( pVF ) ;
        DWORD dwHeight = GetHeight( pVF ) ;
        switch ( pVF->lpBMIH->biCompression )
        {
          case BI_Y8:
          case BI_YUV9:
          case BI_YUV12:
          case BI_Y800:
          {
            LPBYTE pImage = GetData( pVF ) ;
            LPBYTE pBegin = pImage + dwHeight * iY + iX ;
            if ( !m_iIntegrationRadius )
              return ( double )( *pBegin ) ;
            pBegin -= m_iIntegrationRadius ;

            DWORD dwStep = dwWidth - m_iIntegrationRadius * 2 + 1 ;
            DWORD dwInRow = m_iIntegrationRadius * 2 + 1 ;
            double dResult = 0. ;
            for ( DWORD dwYCnt = 0 ; dwYCnt < dwInRow ; dwYCnt++ )
            {
              for ( DWORD dwXCnt = 0 ; dwXCnt < dwInRow ; dwXCnt++ )
                dResult += *( pBegin++ ) ;
              pBegin += dwStep ;
            }
            return dResult / ( dwInRow * dwInRow ) ;
          }
          break ;
          case BI_Y16:
          {
            LPWORD pImage = GetData16( pVF ) ;
            LPWORD pBegin = pImage + dwHeight * iY + iX ;
            if ( !m_iIntegrationRadius )
              return ( double )( *pBegin ) ;
            pBegin -= m_iIntegrationRadius ;

            DWORD dwStep = dwWidth - m_iIntegrationRadius * 2 + 1 ;
            DWORD dwInRow = m_iIntegrationRadius * 2 + 1 ;
            double dResult = 0. ;
            for ( DWORD dwYCnt = 0 ; dwYCnt < dwInRow ; dwYCnt++ )
            {
              for ( DWORD dwXCnt = 0 ; dwXCnt < dwInRow ; dwXCnt++ )
                dResult += *( pBegin++ ) ;
              pBegin += dwStep ;
            }
            return dResult / ( dwInRow * dwInRow ) ;
          }
          default:
            break ; ;
        }
      }
    }
  }
  return -1. ;
}



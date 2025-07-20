#include "StdAfx.h"
#include <shbase\shbase.h>
#include <shbase\dxrender.h>
#include <gadgets\ContainerFrame.h>
#include <gadgets\RectFrame.h>
#include <gadgets\VideoFrame.h>
#include <imageproc\draw_over.h>


CDXRender::CDXRender( LPCTSTR name ) :
  CDXView( name )
{}

CDXRender::~CDXRender( void )
{}

void    CDXRender::Render( const CDataFrame* pDataFrame )
{
  if ( (pDataFrame) && (!Tvdb400_IsEOS( pDataFrame )) )
  {
    CFramesIterator* Iterator = pDataFrame->CreateFramesIterator( vframe );
    CFramesIterator* mfIter = pDataFrame->CreateFramesIterator( metafile );
    if ( mfIter )
    {
      m_Lock.Lock();
      m_Metafiles.RemoveAll();
      CMetafileFrame* mfFrame = (CMetafileFrame*) mfIter->Next( DEFAULT_LABEL );
      //TRACE("\tCDIBRender::Render 0x%x\n",mfFrame);
      while ( mfFrame )
      {
        m_Metafiles.Add( *mfFrame );
        mfFrame = (CMetafileFrame*) mfIter->Next( DEFAULT_LABEL );
        //TRACE("\tCDIBRender::Render 0x%x\n",mfFrame);
      }
      m_Lock.Unlock();
      delete mfIter; mfIter = NULL;
    }
    else
    {
      m_Lock.Lock();
      m_Metafiles.RemoveAll();
      m_Lock.Unlock();
    }
    if ( Iterator )
    {
      const CRectFrame* rectFrame = pDataFrame->GetRectFrame( DEFAULT_LABEL );
      const CVideoFrame* cutFrame = (CVideoFrame*) Iterator->Next( DEFAULT_LABEL );
      const CVideoFrame* bgFrame = (CVideoFrame*) Iterator->Next( DEFAULT_LABEL );
      if ( rectFrame && cutFrame && bgFrame )
      {
        m_Rect = *rectFrame;
        CVideoFrame* Frame = CVideoFrame::Create( _draw_over( bgFrame , cutFrame , m_Rect.left , m_Rect.top ) );
        if ( (Frame) && (Frame->lpBMIH) )
          LoadFrame( Frame );
        else
          LoadFrame( NULL );
        Frame->Release( Frame );
      }
      else if ( cutFrame && cutFrame->lpBMIH )
      {
        if ( rectFrame )
          m_Rect = *rectFrame;
        else
          m_Rect.SetRectEmpty();
        LoadFrame( cutFrame );
      }
      else
      {
        m_Rect.SetRectEmpty();
        LoadFrame( NULL );
      }
      delete Iterator;
    }
    else
    {
      m_Rect.SetRectEmpty();
      const CVideoFrame* Frame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
      if ( (Frame) && (Frame->lpBMIH) )
        LoadFrame( Frame );
      else
        LoadFrame( NULL );
    }
  }
  else
    LoadFrame( NULL );

  /*    if ((pDataFrame) && (!Tvdb400_IsEOS(pDataFrame)))
    {
      CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(vframe);
      if (Iterator)
      {
        const CRectFrame* rectFrame = pDataFrame->GetRectFrame(DEFAULT_LABEL);
        const CVideoFrame* cutFrame = (CVideoFrame*)Iterator->Next(DEFAULT_LABEL);
        const CVideoFrame* bgFrame = (CVideoFrame*)Iterator->Next(DEFAULT_LABEL);
        if (rectFrame && cutFrame && bgFrame)
        {
          m_Rect = *rectFrame;
          CVideoFrame* Frame = CVideoFrame::Create(_draw_over(bgFrame, cutFrame, m_Rect.left, m_Rect.top));
          if ((Frame) && (Frame->lpBMIH))
            LoadFrame(Frame);
          else
            LoadFrame(NULL);
          Frame->Release(Frame);
        }
        else if (cutFrame && cutFrame->lpBMIH)
        {
                  if (rectFrame)
                       m_Rect = *rectFrame;
                  else
                      m_Rect.SetRectEmpty();
          LoadFrame(cutFrame);
        }
        else
        {
          m_Rect.SetRectEmpty();
          LoadFrame(NULL);
        }
        delete Iterator;
      }
      else
      {
        m_Rect.SetRectEmpty();
        const CVideoFrame* Frame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
        if ((Frame) && (Frame->lpBMIH))
          LoadFrame(Frame);
        else
          LoadFrame(NULL);
      }
      } */
}

BOOL CDXRender::DrawOverBitmap()
{
  HDC                 hdcImage;
  hdcImage = CreateCompatibleDC( NULL );
  if ( !hdcImage )
  {
    TRACE( "UpdateFrame: Create compatible DC failed\n" );
    return FALSE;
  }

  HGDIOBJ oldObj = SelectObject( hdcImage , m_hBM );
  if ( !oldObj )
  {
    TRACE( "UpdateFrame: Can't select bitmap to current DC\n" );
    return FALSE;
  }
  FillRect( hdcImage , CRect( 0 , 0 , m_ViewWidth , m_ViewHeight ) , m_BkBrush );
  if ( CDIBView::Draw( hdcImage , (RECT&)CRect( 0 , 0 , m_ViewWidth , m_ViewHeight ) ) )
  {
    DrawMetafiles( hdcImage );

    CDC dc; dc.Attach( hdcImage );
    if ( m_Selection )
    {
      CPen*    oPen;
      if ( (m_Selection->LineEnable) && (m_Selection->DrawLine) )
      {
        if ( m_Selection->LPressed )
        {
          oPen = (CPen*) dc.SelectObject( m_Selection->Pen );
          CPoint a , b; a = m_Selection->p1; b = m_Selection->p2;
          SubPix2Scr( a ); SubPix2Scr( b );
          dc.MoveTo( a ); dc.LineTo( b );
          dc.SelectObject( &oPen );
        }
        else
        {
          oPen = (CPen*) dc.SelectObject( m_Selection->Pen );
          CPoint a( m_Selection->Object.left , m_Selection->Object.top ) ,
            b( m_Selection->Object.right , m_Selection->Object.bottom );
          SubPix2Scr( a ); SubPix2Scr( b );
          dc.MoveTo( a ); dc.LineTo( b );
          dc.SelectObject( &oPen );
        }
      }
      else if ( (m_Selection->RectEnable) && (m_Selection->DrawRect) )
      {
        if ( m_Selection->RPressed )
        {
          oPen = (CPen*) dc.SelectObject( m_Selection->Pen );
          CPoint a , b; a = m_Selection->p1; b = m_Selection->p2;
          SubPix2Scr( a ); SubPix2Scr( b );
          CPoint pl[ 5 ]; pl[ 0 ] = a; pl[ 1 ] = CPoint( b.x , a.y ); pl[ 2 ] = b; pl[ 3 ] = CPoint( a.x , b.y ); pl[ 4 ] = a;
          dc.Polyline( pl , 5 );
          dc.SelectObject( &oPen );
        }
        else
        {
          oPen = (CPen*) dc.SelectObject( m_Selection->Pen );
          CPoint a( m_Selection->Object.left , m_Selection->Object.top ) ,
            b( m_Selection->Object.right , m_Selection->Object.bottom );
          SubPix2Scr( a ); SubPix2Scr( b );
          CPoint pl[ 5 ]; pl[ 0 ] = a; pl[ 1 ] = CPoint( b.x , a.y ); pl[ 2 ] = b; pl[ 3 ] = CPoint( a.x , b.y ); pl[ 4 ] = a;
          dc.Polyline( pl , 5 );
          dc.SelectObject( &oPen );
        }
      }
    }
    if ( m_DrawEx ) m_DrawEx( dc.m_hDC , (RECT&) CRect( 0 , 0 , m_ViewWidth , m_ViewHeight ) , this , m_DrawExParam );
    dc.Detach();
  }
  SelectObject( hdcImage , oldObj );
  DeleteDC( hdcImage );
  return TRUE;
}

void CDXRender::DrawMetafiles( HDC hdc )
{
  bool MapModeChanged = false;
  int iMap = ::GetMapMode( hdc );
  if ( iMap == MM_TEXT )
  {
    ::SetMapMode( hdc , MM_HIMETRIC );
    MapModeChanged = true;
  }

  CPoint tl = m_ScrOffset;
  DPtoLP( hdc , &tl , 1 );

  CPoint tstPoint( 1000 , 1000 );
  DPtoLP( hdc , &tstPoint , 1 );

  CRgn   rgn;
  VERIFY( rgn.CreateRectRgn( m_ScrOffset.x , m_ScrOffset.y , m_ScrOffset.x + (int) (m_Width*m_ScrScale + 0.5) , m_ScrOffset.y + (int) (m_Height*m_ScrScale + 0.5) ) );
  ::SelectClipRgn( hdc , rgn );

  CPoint br( tl.x + (int) (m_Width*m_ScrScale + 0.5) , tl.y - (int) (m_Height*m_ScrScale + 0.5) );
  //CPoint br(m_ScrOffset.x+(int)(m_Width*m_ScrScale+0.5),m_ScrOffset.y+(int)(m_Height*m_ScrScale+0.5));
//DPtoLP(hdc,&br,1);
  for ( int i = 0; i < (int) m_Metafiles.GetSize(); i++ )
  {
    CRect clRect;
    GetClientRect( clRect );

    HENHMETAFILE hmf = m_Metafiles[ i ].GetMF();

    if ( hmf )
      PlayEnhMetaFile( hdc , hmf , CRect( tl.x , tl.y , br.x , br.y ) );
    DeleteEnhMetaFile( hmf );
  }
  if ( MapModeChanged )
    CDC::FromHandle( hdc )->SetMapMode( iMap );
}

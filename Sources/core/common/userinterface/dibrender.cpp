// DIBRender.cpp: implementation of the CDIBRender class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <shbase\shbase.h>
#include <gadgets\ContainerFrame.h>
#include <gadgets\RectFrame.h>
#include <gadgets\VideoFrame.h>
#include <imageproc\draw_over.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDIBRender::CDIBRender( LPCTSTR name ) :
CDIBView( name )
{
  m_Rect.SetRectEmpty();
  m_RedPen.CreatePen( PS_SOLID , 1 , RGB( 255 , 0 , 0 ) );
  m_PurplePen.CreatePen( PS_SOLID , 2 , RGB( 255 , 0 , 0 ) );
}

CDIBRender::~CDIBRender()
{
  m_RedPen.DeleteObject();
  m_PurplePen.DeleteObject();
}

BOOL CDIBRender::Create( CWnd* pParentWnd , DWORD dwAddStyle )
{
  BOOL res = CDIBView::Create( pParentWnd , dwAddStyle );
  if ( res )
  {
  }
  return res;
}

bool CDIBRender::Draw( HDC hdc , RECT& rc )
{
  bool res = CDIBView::Draw( hdc , rc );
  if ( res )
  {
    m_Lock.Lock();
    if ( m_Metafiles.GetSize() ) DrawMetafiles( hdc );
    m_Lock.Unlock();
  }
  return res;
}

void CDIBRender::Render( const CDataFrame* pDataFrame )
{
  if ( ( pDataFrame ) && ( !Tvdb400_IsEOS( pDataFrame ) ) )
  {
    CFramesIterator* Iterator = pDataFrame->CreateFramesIterator( vframe );
    CFramesIterator* mfIter = pDataFrame->CreateFramesIterator( metafile );
    if ( mfIter )
    {
      m_Lock.Lock();
      m_Metafiles.RemoveAll();
      CMetafileFrame* mfFrame = ( CMetafileFrame* ) mfIter->Next( DEFAULT_LABEL );
      //TRACE("\tCDIBRender::Render 0x%x\n",mfFrame);
      while ( mfFrame )
      {
        m_Metafiles.Add( *mfFrame );
        mfFrame = ( CMetafileFrame* ) mfIter->Next( DEFAULT_LABEL );
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
      const CVideoFrame* cutFrame = ( CVideoFrame* ) Iterator->Next( DEFAULT_LABEL );
      const CVideoFrame* bgFrame = ( CVideoFrame* ) Iterator->Next( DEFAULT_LABEL );
      if ( rectFrame && cutFrame && bgFrame )
      {
        m_Rect = *rectFrame;
        CVideoFrame* Frame = CVideoFrame::Create( _draw_over( bgFrame , cutFrame , m_Rect.left , m_Rect.top ) );
        if ( ( Frame ) && ( Frame->lpBMIH ) )
          LoadFrame( Frame , pDataFrame );
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
        LoadFrame( cutFrame , pDataFrame );
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
      if ( ( Frame ) && ( Frame->lpBMIH ) )
        LoadFrame( Frame , pDataFrame );
      else
        LoadFrame( NULL );
    }
  }
  else
    LoadFrame( NULL );
}

void CDIBRender::DrawMetafiles( HDC hdc )
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
  VERIFY( rgn.CreateRectRgn( m_ScrOffset.x , m_ScrOffset.y , m_ScrOffset.x + ( int ) ( m_Width*m_ScrScale + 0.5 ) , m_ScrOffset.y + ( int ) ( m_Height*m_ScrScale + 0.5 ) ) );
  ::SelectClipRgn( hdc , rgn );

  CPoint br( tl.x + ( int ) ( m_Width*m_ScrScale + 0.5 ) , tl.y - ( int ) ( m_Height*m_ScrScale + 0.5 ) );
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

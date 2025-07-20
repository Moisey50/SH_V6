#include "stdafx.h"
#include "FDIBViewBase.h"


FDIBViewBase::FDIBViewBase(void)
{
}


FDIBViewBase::~FDIBViewBase(void)
{
}
BEGIN_MESSAGE_MAP(FDIBViewBase, CDIBViewBase)
  ON_WM_KEYDOWN()
END_MESSAGE_MAP()


void FDIBViewBase::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  CDIBViewBase::OnKeyDown(nChar, nRepCnt, nFlags);
}
void* FDIBViewBase::DoPrepareData(pTVFrame frame, LPVOID lpBuf)
{
  if (frame->lpBMIH->biCompression==BI_Y16)
  {
    if ((!m_DataMonochrome) && lpBuf)
    {
      free(lpBuf);
      lpBuf=NULL;
      m_DataMonochrome=true;
    }
    return y16rgb8(frame->lpBMIH,frame->lpData,lpBuf);
  }
  else if ( (frame->lpBMIH->biCompression==BI_Y8)
    || (frame->lpBMIH->biCompression==BI_Y800)
    || ((m_Monochrome) && (frame->lpBMIH->biCompression==BI_YUV9)))
  {
    if ((!m_DataMonochrome) && lpBuf)
    {
      free(lpBuf);
      lpBuf=NULL;
      m_DataMonochrome=true;
    }
    return yuv9rgb8(frame->lpBMIH,frame->lpData,lpBuf);
  }
  else if (frame->lpBMIH->biCompression==BI_YUV9)
  {
    if ((m_DataMonochrome) && lpBuf)
    {
      free(lpBuf);
      lpBuf=NULL;
      m_DataMonochrome=false;
    }
    return yuv9rgb24(frame->lpBMIH,frame->lpData,lpBuf);
  }
  else if ((frame->lpBMIH->biCompression==BI_YUV12) && (m_Monochrome))
  {
    if ((!m_DataMonochrome) && lpBuf)
    {
      free(lpBuf);
      lpBuf=NULL;
      m_DataMonochrome=true;
    }
    return yuv9rgb8(frame->lpBMIH,frame->lpData,lpBuf);
  }
  else if (frame->lpBMIH->biCompression==BI_YUV12)
  {
    if ((m_DataMonochrome) && lpBuf)
    {
      free(lpBuf);
      lpBuf=NULL;
      m_DataMonochrome=false;
    }
    return yuv12rgb24(frame->lpBMIH,frame->lpData,lpBuf);
  }
  else if (frame->lpBMIH->biCompression==BI_RGB)
  {
    if ((m_DataMonochrome) && lpBuf)
    {
      free(lpBuf);
      lpBuf=NULL;
      m_DataMonochrome=false;
    }
    return rgbrgb24(frame->lpBMIH,frame->lpData,lpBuf);
  }
  if (lpBuf) free(lpBuf);
  return NULL;
}
int  FDIBViewBase::Pic2Scr( double len )
{
  return ( int ) ( ( len + 0.5 )*m_ScrScale + 0.5 );
}

int  FDIBViewBase::Scr2Pic( double len )
{
  return ( int ) ( len / m_ScrScale /*+ 0.5*/ );
}

void FDIBViewBase::Scr2Pic( CPoint &point )
{
  if ( m_ScrScale < 0.01 ) { point = CPoint( 0 , 0 ); return; }
  point.x -= m_ScrOffset.x;
  point.y -= m_ScrOffset.y;
  point.x = (int) (point.x / m_ScrScale /*+ 0.5 */);
  point.y = (int) (point.y / m_ScrScale /*+ 0.5 */);

  point -= m_IntBmOffset;
}

cmplx FDIBViewBase::Scr2PicCmplx( CPoint &point )
{
  if ( m_ScrScale < 0.01 ) 
  { 
    point = CPoint( 0 , 0 ); 
    return cmplx();
  }
  point.x -= m_ScrOffset.x;
  point.y -= m_ScrOffset.y;
  cmplx OnPic( point.x / m_ScrScale , point.y / m_ScrScale ) ;
  point.x = ROUND( OnPic.real() );
  point.y = ROUND( OnPic.imag() );
  point -= m_IntBmOffset;
  OnPic -= cmplx( m_IntBmOffset.x , m_IntBmOffset.y ) ;
  return OnPic ;
}

void FDIBViewBase::SubPix2Scr( CPoint &point )
{
  point += m_IntBmOffset;
  if ( m_ScrScale < 0.01 ) { point = CPoint( 0 , 0 ); return; }
  point.x = ( int ) ( ( point.x + 0.5 )*m_ScrScale + 0.5 );
  point.y = ( int ) ( ( point.y + 0.5 )*m_ScrScale + 0.5 );
  point.x += m_ScrOffset.x;
  point.y += m_ScrOffset.y;
}

void FDIBViewBase::Pic2Scr( CPoint &point )
{
  point += m_IntBmOffset;
  if ( m_ScrScale < 0.01 ) { point = CPoint( 0 , 0 ); return; }
  point.x = ( int ) ( point.x*m_ScrScale + 0.5 );
  point.y = ( int ) ( point.y*m_ScrScale + 0.5 );
  point.x += m_ScrOffset.x;
  point.y += m_ScrOffset.y;
}


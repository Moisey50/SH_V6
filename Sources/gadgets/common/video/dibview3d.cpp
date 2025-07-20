#include "StdAfx.h"
#include "DIBView3D.h"
#include <imageproc/d3dproject.h>

CDIBView3D::CDIBView3D(LPCTSTR name):
CDIBView(name),
m_alpha(15),
m_beta(15),
m_rotZ(0),
m_grid(30),
m_bGray(false),
m_iLow(0) ,
m_iHigh(255)
{
    _prepare_mpalette();
}

CDIBView3D::~CDIBView3D()
{
}

bool CDIBView3D::PrepareData(const pTVFrame frame, DWORD dwTimeout )
{
  if (!m_LockOMutex.Lock( dwTimeout
#ifdef _DEBUG
    , "CDIBViewBase::PrepareData"
#endif
  ))
    return false ;
  if (frame->lpBMIH->biCompression == BI_YUV9
    || frame->lpBMIH->biCompression == BI_YUV12
    || frame->lpBMIH->biCompression == BI_Y8)
	{
    if (m_Frame)
    {
      freeTVFrame( m_Frame ); m_Frame = NULL;
      m_Width = 0; m_Height = 0;
      if (m_pConvertedBMIH)
      {
        free( m_pConvertedBMIH );
        m_pConvertedBMIH = NULL;
      }
    }
    m_SizeChanged = true;
    int alpha = ( m_alpha + m_rotZ ) % 360;
		int beta = (m_beta - m_rotZ + 720) % 360;

    LPBITMAPINFOHEADER lpBMIHResult = (LPBITMAPINFOHEADER)  _d3dproject(
      GetData(frame), frame->lpBMIH->biWidth, frame->lpBMIH->biHeight, 
      alpha, beta, m_grid, m_bGray , m_iLow , m_iHigh );

    if ( lpBMIHResult )
    {
      m_pConvertedBMIH = lpBMIHResult ;
      m_Frame = makeTVFrame( lpBMIHResult ) ;
    }
	}
  m_LockOMutex.Unlock() ;
  return ( m_Frame != NULL ) ;
}

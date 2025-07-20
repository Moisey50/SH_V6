#pragma once

#include "resource.h"
//#include "ImagView.h"
class CImageView ;
class BackImage
{
public:
  BackImage(void) { memset( this , 0 , sizeof(*this) ) ; } ;
  ~BackImage(void)
  {
    if ( m_Image )
      MbufFree( m_Image ) ;
    m_pImage = NULL ;
  }

  void SetImage( MIL_ID Id , DWORD dwExposure ) 
  { 
    if ( m_Image && Id )
    {
      MbufFree( m_Image ) ;
      m_pImage = NULL ;
    }
    m_Image = Id ;
    m_dwExposure = dwExposure ;
    if ( m_Image )
      m_pImage = (short int*)MbufInquire( 
      m_Image , M_HOST_ADDRESS , NULL ) ;
    else
      m_pImage = NULL ;
  }
  void SetHost( CImageView * pHost ) { m_pImaging = pHost ; } ;
  BackImage& operator = (BackImage& Orig)
  {
    if ( m_Image && (m_Image != Orig.m_Image) )
    {
      MbufFree( m_Image ) ;
      m_pImage = NULL ;
    }
    m_Image = Orig.m_Image ;
    if ( m_Image )
      m_pImage = (short int*)MbufInquire( 
        m_Image , M_HOST_ADDRESS , NULL ) ;
    else
      m_pImage = NULL ;

    m_dwExposure = Orig.m_dwExposure ;
    m_pImaging = Orig.m_pImaging ;
    return *this ;
  }
  CImageView * m_pImaging ;
  DWORD       m_dwExposure ;
  MIL_ID      m_Image ;
  short int * m_pImage ;
};


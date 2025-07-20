#pragma once

#define EMBED_INFO_PATTERN 0x55aa00ff

class VideoFrameEmbeddedInfo
{
public:
  VideoFrameEmbeddedInfo()
  {
    memset( this , 0 , sizeof( *this ) ) ;
    m_dwIdentificationPattern = EMBED_INFO_PATTERN ;
  }

  DWORD    m_dwIdentificationPattern ;
  uint64_t  m_CameraFrameID ;
  uint64_t  m_CameraFrameTime ;
  uint64_t  m_FrameFormat ;
  CRect    m_ROI ;
  double   m_dExposure_us ;
  double   m_dGain_dB ;
  double   m_dBrightness ;
  double   m_dGraphTime ;
  int      m_iGamma ;
  int      m_iSharpness ;
  int      m_iTriggerMode ;
  int      m_iTriggerSource ;
  int      m_iTriggerDelay ;
  int      m_ColorSign;
  int      m_iTriggerSelector ;
  int      m_iBurstLength ;

  void Inverse() // inverse byte order for horizontal flipped images
  {
    BYTE bTmp ;
    BYTE * p = (BYTE*)this ; // ptr to first
    BYTE * pRev = p + sizeof(*this) - 1 ; // ptr to last
    while ( p < pRev )
    {
      bTmp = *p ;
      *(p++) = *pRev ;
      *(pRev--) = bTmp ;
    }
  }
};

#define PVFEI VideoFrameEmbeddedInfo* 

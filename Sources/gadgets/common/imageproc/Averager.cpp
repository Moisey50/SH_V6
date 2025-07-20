// Averager.cpp: implementation of the CAverager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Averager.h"

#define DEFAULT_FRAMES_RANGE	10

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAverager::CAverager(int mode):
  m_Mode(mode),
  m_pFrame(NULL),
  m_pData(NULL),
  m_SliderBuffer(NULL),
  m_AvgVal(0),
  m_cFrames(0),
  m_FramesRange(DEFAULT_FRAMES_RANGE),
  m_FrameSize(0,0) ,
  m_iLastMin( 0 ) ,
  m_iLastMax( 0 ) ,
  m_bAllowedMaxChanged( true ),
  m_iAddCnt(0),
  m_iAddMax(10)
{
}

CAverager::~CAverager()
{
  Reset();
}

void CAverager::Reset( int iAverageFactor )
{
  FXAutolock al(m_Lock);
  if (m_pFrame) 
  {
    freeTVFrame(m_pFrame); 
    m_pFrame = NULL;
  }
  if (m_pData)  
  {
    free(m_pData);         
    m_pData=NULL;
  }
  if (m_SliderBuffer) 
  {    
    free(m_SliderBuffer); 
    m_SliderBuffer=NULL;
  }
  if ( iAverageFactor > 0 )
    m_FramesRange = iAverageFactor ;

  m_AvgVal = 0;
  m_cFrames = 0;
}

__forceinline void copyb2i(DWORD *dst, LPBYTE src, DWORD size)
{
  LPBYTE eod=src+size;
  while (src<eod)
  {
    *dst=*src; dst++; src++;
  }
}

void CAverager::AddFrame(pTVFrame Frame)
{
  if (!Frame) return;
  if (!Frame->lpBMIH) return;
  CSize frsz(Frame->lpBMIH->biWidth,Frame->lpBMIH->biHeight);
  if (m_FrameSize!=frsz)
  {
    //TRACE("CAverager: frame size changed from (%d,%d) to (%d,%d)\n",m_FrameSize.cx,m_FrameSize.cy,frsz.cx,frsz.cy);
    Reset();
    m_FrameSize=frsz;
  }
  FXAutolock al(m_Lock);
  m_AvgVal = 0;
  if ((!m_cFrames) || (m_WrkFormat!=Frame->lpBMIH->biCompression) || m_bAllowedMaxChanged )
  {
    m_bAllowedMaxChanged = false ;
    if (m_pFrame) 
    {
      freeTVFrame(m_pFrame); 
      m_pFrame = NULL;
    }
    m_pFrame = (pTVFrame)malloc(sizeof(TVFrame));
    m_pFrame->lpBMIH = (LPBITMAPINFOHEADER)malloc(getsize4BMIH(Frame));
    if (m_pData)  
    {
      free(m_pData);         
      m_pData=NULL;
    }
    m_pData=(LPDWORD)malloc(sizeof(DWORD)*GetImageSize(Frame->lpBMIH));
    if (m_SliderBuffer) 
    {
      free(m_SliderBuffer); 
      m_SliderBuffer=NULL;
    }
    m_AvgVal = 0;
    m_cFrames = 0;
    if (m_Mode==AVG_SLIDEWINDOW)
    {
      DWORD iS=GetImageSize(Frame->lpBMIH);
      m_SliderBuffer=(LPBYTE)malloc(GetImageSize(Frame->lpBMIH)*m_FramesRange);
      memset(m_SliderBuffer,0,GetImageSize(Frame->lpBMIH)*m_FramesRange);
      memcpy(m_SliderBuffer,GetData(Frame),GetImageSize(Frame));
    }
    copy2BMIH(m_pFrame->lpBMIH, Frame);
    copyb2i(m_pData,GetData(Frame),GetImageSize(Frame->lpBMIH));
    m_pFrame->lpData = NULL;
    m_cFrames++;
    m_WrkFormat=Frame->lpBMIH->biCompression;
    return;
  }
  DWORD dwBWSize8 = m_pFrame->lpBMIH->biWidth * m_pFrame->lpBMIH->biHeight ;
  ASSERT(m_pFrame);
  switch (Frame->lpBMIH->biCompression)
  {
  case BI_Y8:
  case BI_Y800:
    {
      LPBYTE Dst = GetData(m_pFrame);
      LPBYTE Src = GetData(Frame);
      LPDWORD Avg = m_pData;
      LPBYTE End = Dst + dwBWSize8 ;
      switch (m_Mode)
      {
      case AVG_INFINITE_UNIFORM:
        while (Dst < End)
        {
          *Avg+=*Src; 
          *Dst=(BYTE)(*Avg/m_cFrames);
          Dst++; Src++; Avg++;
        }
        break;
      case AVG_INFINITE_PASTFADING:
        while (Dst < End)
        {
          *Avg=(3* *Avg+*Src)/4; 
          *Dst=(BYTE)(*Avg);
          Dst++; Src++; Avg++;
        }
        break;
      case AVG_SLIDEWINDOW:
        if (m_cFrames<m_FramesRange)
        {
          int iDivider = m_cFrames + 1 ;
          memcpy(m_SliderBuffer+GetImageSize(m_pFrame->lpBMIH)*m_cFrames,GetData(Frame),GetImageSize(m_pFrame->lpBMIH));
          while (Dst < End)
          {
            *Avg+=*Src; 
            *Dst=(BYTE)(*Avg/ iDivider);
            Dst++; Src++; Avg++;
          }
        }
        else
        {
          int off=m_cFrames%m_FramesRange;
          LPBYTE oldF=m_SliderBuffer+GetImageSize(m_pFrame->lpBMIH)*off;
          LPBYTE oldFs=oldF; LPBYTE endS=oldF+GetImageSize(m_pFrame->lpBMIH);
          DWORD* aS=Avg;
          while (oldFs<endS)
          {
            *aS-=*oldFs; oldFs++; aS++;
          }
          memcpy(oldF,GetData(Frame),GetImageSize(m_pFrame->lpBMIH));
          while (Dst < End)
          {
            *Avg+=*Src; 
            *Dst=(BYTE)(*Avg/m_FramesRange);
            Dst++; Src++; Avg++;
          }
        }
        break;
      case AVG_ADD_AND_NORMALIZE:
        {
           m_iLastMax = 0 ;
           m_iLastMin = 65535 ;
           LPBYTE EndSrc = Src + dwBWSize8 ;
           while (Src < EndSrc )
           {
             *Avg += *Src ;
             if ( *Avg < (DWORD)m_iLastMin )
               m_iLastMin = *Avg ;
             if ( *Avg > (DWORD)m_iLastMax )
               m_iLastMax = *Avg ;
              Avg++ ;
              Src++ ;
           }
           Avg = m_pData;
           int iAmpl = m_iLastMax - m_iLastMin ;
           
           if ( iAmpl > 0 )
           {
             while (Dst < End)
             {
               *Avg -= m_iLastMin ;
               if ( iAmpl > m_iAllowedMax )
                 *Avg = (*Avg * m_iAllowedMax)/iAmpl ;
               *(Dst++)=(BYTE)((*(Avg++)) * 255 / iAmpl );
             }
           }
           else
             memcpy( Dst , Src , dwBWSize8 ) ;
        }
        break ;
      case AVG_SIMPLE_ADD:
        if ( ++m_iAddCnt > m_iAddMax )
        {
          memcpy( Dst , Src , dwBWSize8) ;
          m_iAddCnt = 0 ;
        }
        else
        {
          while ( Dst < End )
          {
            *Dst += *Src;
            Dst++; Src++;
          }
        }
        break;
      }
      m_cFrames++;
      break;
    }
  case BI_YUV9:
    {
      LPBYTE Dst = GetData(m_pFrame);
      LPBYTE Src = GetData(Frame);
      LPDWORD Avg = m_pData;
      LPBYTE End = Dst + (9 * dwBWSize8)/8;
      switch (m_Mode)
      {
      case AVG_INFINITE_UNIFORM:
        while (Dst < End)
        {
          *Avg+=*Src; 
          *Dst=(BYTE)(*Avg/m_cFrames);
          Dst++; Src++; Avg++;
        }
        break;
      case AVG_INFINITE_PASTFADING:
        while (Dst < End)
        {
          *Avg=(3* *Avg+*Src)/4; 
          *Dst=(BYTE)(*Avg);
          Dst++; Src++; Avg++;
        }
        break;
      case AVG_SLIDEWINDOW:
        if (m_cFrames<m_FramesRange)
        {
          int iDivider = m_cFrames + 1 ;
          memcpy(m_SliderBuffer+GetImageSize(m_pFrame->lpBMIH)*m_cFrames,GetData(Frame),GetImageSize(m_pFrame->lpBMIH));
          while (Dst < End)
          {
            *Avg+=*Src; 
            *Dst=(BYTE)(*Avg/iDivider);
            Dst++; Src++; Avg++;
          }
        }
        else
        {
          int off=m_cFrames%m_FramesRange;
          LPBYTE oldF=m_SliderBuffer+GetImageSize(m_pFrame->lpBMIH)*off;
          LPBYTE oldFs=oldF; LPBYTE endS=oldF+GetImageSize(m_pFrame->lpBMIH);
          DWORD* aS=Avg;
          while (oldFs<endS)
          {
            *aS-=*oldFs; oldFs++; aS++;
          }
          memcpy(oldF,GetData(Frame),GetImageSize(m_pFrame->lpBMIH));
          while (Dst < End)
          {
            *Avg+=*Src; 
            *Dst=(BYTE)(*Avg/m_FramesRange);
            Dst++; Src++; Avg++;
          }
        }
        break;
      case AVG_ADD_AND_NORMALIZE:
        {
          m_iLastMax = 0 ;
          m_iLastMin = 65535 ;
          LPBYTE EndSrc = Src + dwBWSize8 ;
          while (Src < EndSrc )
          {
            *Avg += *Src ;
            if ( *Avg < (DWORD)m_iLastMin )
              m_iLastMin = *Avg ;
            if ( *Avg > (DWORD)m_iLastMax )
              m_iLastMax = *Avg ;
            Avg++ ;
            Src++ ;
          }
          Avg = m_pData;
          int iAmpl = m_iLastMax - m_iLastMin ;
          if ( iAmpl > 0 )
          {
            while (Dst < End)
            {
              *Avg -= m_iLastMin ;
              if ( iAmpl > m_iAllowedMax )
                *Avg = (*Avg * m_iAllowedMax)/iAmpl ;
              *(Dst++)=(BYTE)((*(Avg++)) * 255 / iAmpl );
            }
          }
          else
            memcpy( Dst , Src , dwBWSize8 ) ;
          // copy color data without averaging
          memcpy( GetData(m_pFrame) + dwBWSize8 , GetData(Frame) + dwBWSize8 , 
            GetImageSize( Frame ) - dwBWSize8 ) ;
        }
        break ;
      case AVG_SIMPLE_ADD:
        if ( ++m_iAddCnt > m_iAddMax )
        {
          memcpy( Dst , Src , dwBWSize8 ) ;
          m_iAddCnt = 0 ;
        }
        else
        {
          while ( Dst < End )
          {
            *Dst += *Src;
            Dst++; Src++;
          }
        }
        break;
      }
      m_cFrames++;
      break;
    }
  case BI_YUV12:
    {
      LPBYTE Dst = GetData(m_pFrame);
      LPBYTE Src = GetData(Frame);
      LPDWORD Avg = m_pData;
      LPBYTE End = Dst + ( 3 * dwBWSize8 )/2;
      switch (m_Mode)
      {
      case AVG_INFINITE_UNIFORM:
        while (Dst < End)
        {
          *Avg+=*Src; 
          *Dst=(BYTE)(*Avg/m_cFrames);
          Dst++; Src++; Avg++;
        }
        break;
      case AVG_INFINITE_PASTFADING:
        while (Dst < End)
        {
          *Avg=(3* *Avg+*Src)/4; 
          *Dst=(BYTE)(*Avg);
          Dst++; Src++; Avg++;
        }
        break;
      case AVG_SLIDEWINDOW:
        if (m_cFrames<m_FramesRange)
        {
          int iDivider = m_cFrames + 1 ;
          memcpy(m_SliderBuffer+GetImageSize( m_pFrame->lpBMIH)*m_cFrames ,
            GetData( Frame ) , GetImageSize(m_pFrame->lpBMIH)) ;
          while (Dst < End)
          {
            *Avg+=*Src; 
            *Dst=(BYTE)(*Avg/iDivider);
            Dst++; Src++; Avg++;
          }
        }
        else
        {
          int off=m_cFrames%m_FramesRange;
          LPBYTE oldF=m_SliderBuffer+GetImageSize(m_pFrame->lpBMIH)*off;
          LPBYTE oldFs=oldF; 
          LPBYTE endS=oldF+GetImageSize(m_pFrame->lpBMIH);
          DWORD* aS=Avg;
          while (oldFs<endS)
          {
            *aS-=*oldFs; oldFs++; aS++;
          }
          memcpy(oldF,GetData(Frame),GetImageSize(m_pFrame->lpBMIH));
          while (Dst < End)
          {
            *Avg+=*Src; 
            *Dst=(BYTE)(*Avg/m_FramesRange);
            Dst++; Src++; Avg++;
          }
        }
        break;
      case AVG_ADD_AND_NORMALIZE:
        {
          m_iLastMax = 0 ;
          m_iLastMin = 65535 ;
          LPBYTE EndSrc = Src + dwBWSize8 ;
          while (Src < EndSrc )
          {
            *Avg += *Src ;
            if ( *Avg < (DWORD)m_iLastMin )
              m_iLastMin = *Avg ;
            if ( *Avg > (DWORD)m_iLastMax )
              m_iLastMax = *Avg ;
            Avg++ ;
            Src++ ;
          }
          Avg = m_pData;
          int iAmpl = m_iLastMax - m_iLastMin ;
          if ( iAmpl > 0 )
          {
            while (Dst < End)
            {
              *Avg -= m_iLastMin ;
              if ( iAmpl > m_iAllowedMax )
                *Avg = (*Avg * m_iAllowedMax)/iAmpl ;
              *(Dst++)=(BYTE)((*(Avg++)) * 255 / iAmpl );
            }
          }
          else
            memcpy( Dst , Src , dwBWSize8 ) ;
          // copy color data without averaging
          memcpy( GetData(m_pFrame) + dwBWSize8 , GetData(Frame) + dwBWSize8 , 
            GetImageSize( Frame ) - dwBWSize8 ) ;
        }
        break ;
      case AVG_SIMPLE_ADD:
        if ( ++m_iAddCnt > m_iAddMax )
        {
          memcpy( Dst , Src , dwBWSize8 ) ;
          m_iAddCnt = 0 ;
        }
        else
        {
          while ( Dst < End )
          {
            *Dst += *Src;
            Dst++; Src++;
          }
        }
        break;
      }
      m_cFrames++;
      break; // from Y8
    }
  case BI_Y16:
    {
      LPWORD Dst = (LPWORD)GetData(m_pFrame);
      LPWORD Src = (LPWORD)GetData(Frame);
      DWORD *Avg = m_pData;
      LPWORD End = Dst + dwBWSize8 ;
      switch (m_Mode)
      {
      case AVG_INFINITE_UNIFORM:
        while (Dst < End)
        {
          *Avg+=*Src; 
          *Dst=(WORD)(*Avg/m_cFrames);
          Dst++; Src++; Avg++;
        }
        break;
      case AVG_INFINITE_PASTFADING:
        while (Dst < End)
        {
          *Avg=(3* *Avg+*Src)/4; 
          *Dst=(WORD)(*Avg);
          Dst++; Src++; Avg++;
        }
        break;
      case AVG_SLIDEWINDOW:
        if (m_cFrames < m_FramesRange)
        {
          int iDivider = m_cFrames + 1 ;
          memcpy(m_SliderBuffer+GetImageSize(m_pFrame->lpBMIH)*m_cFrames,
            GetData(Frame),GetImageSize(m_pFrame->lpBMIH));
          while (Dst < End)
          {
            *Avg += *Src ; 
            *Dst=(WORD)(*Avg/iDivider);
            Dst++; Src++; Avg++;
          }
        }
        else
        {
          int off = m_cFrames % m_FramesRange;
//          LPWORD oldF = ( LPWORD ) ( m_SliderBuffer + GetImageSize( m_pFrame->lpBMIH ) * off );
          LPWORD oldF=(LPWORD)(m_SliderBuffer+GetImageSize(m_pFrame->lpBMIH) * off );
          LPWORD oldFs = oldF; 
          LPWORD endS = ( LPWORD ) ( ( LPBYTE ) oldF + GetImageSize( m_pFrame->lpBMIH ) );
          DWORD* aS=Avg;
          while (oldFs<endS)
          {
            if ( *aS >= *oldFs )
              *aS -= *oldFs ; 
            else
              *aS = 0 ;
            oldFs++ ; 
            aS++ ;
          }
          memcpy(oldF,GetData(Frame),GetImageSize(m_pFrame->lpBMIH));
          while (Dst < End)
          {
            *Avg += *Src ; 
            *Dst = (WORD)(*Avg/m_FramesRange);
            Dst++; Src++; Avg++;
          }
        }
        break;
      case AVG_ADD_AND_NORMALIZE:
        {
          m_iLastMax = 0 ;
          m_iLastMin = 1000000000 ;
          LPWORD EndSrc = Src + dwBWSize8 ;
          while (Src < EndSrc )
          {
            *Avg += *(Src++) ;
            if ( *Avg < (DWORD)m_iLastMin )
              m_iLastMin = *Avg ;
            if ( *Avg > (DWORD)m_iLastMax )
              m_iLastMax = *Avg ;
            Avg++ ;
          }
          Avg = m_pData;
          int iAmpl = m_iLastMax - m_iLastMin ;
          if ( iAmpl > 0 )
          {
            while (Dst < End)
            {
              *Avg -= m_iLastMin ;
              if ( iAmpl > m_iAllowedMax )
                *Avg = (*Avg * m_iAllowedMax)/iAmpl ;
              *(Dst++)=(WORD)((*(Avg++)) * 65535 / iAmpl );
            }
          }
          else
            memcpy( Dst , Src , dwBWSize8 * sizeof(WORD) ) ;
        }
        break ;
      case AVG_SIMPLE_ADD:
        if ( ++m_iAddCnt > m_iAddMax )
        {
          memcpy( Dst , Src , dwBWSize8 * 2 ) ;
          m_iAddCnt = 0 ;
        }
        else
        {
          while ( Dst < End )
          {
            *Dst += *Src;
            Dst++; Src++;
          }
        }
        break;
      }
    }
    m_cFrames++;
    break; // from Y16
  }
  m_AvgVal /= (m_pFrame->lpBMIH->biWidth * m_pFrame->lpBMIH->biHeight);
}

LPCTSTR CAverager::GetModeName(int i)
{
  switch (i)
  {
  case AVG_INFINITE_UNIFORM:
    return "InfiniteUniform";
  case AVG_INFINITE_PASTFADING:
    return "InfinitePastFading";
  case AVG_SLIDEWINDOW:
    return "SlidingWindow";
  case AVG_ADD_AND_NORMALIZE:
    return "AddAndNormalize" ;
  case AVG_SIMPLE_ADD:
    return "SimpleAdd" ;
  }
  return "Unknown";
}

void CAverager::SetMode(int mode) 
{ 
  FXAutolock al(m_Lock);
  if (m_pFrame) 
  {
    freeTVFrame( m_pFrame );
    m_pFrame = NULL;
  }
  if (m_pData)  
  {
    free( m_pData );
    m_pData = NULL;
  }
  if (m_SliderBuffer) 
  {
    free( m_SliderBuffer );
    m_SliderBuffer = NULL;
  }
  m_AvgVal = 0;
  m_cFrames = 0;
  m_Mode = mode; 
}

void CAverager::SetFramesRange(int range) 
{ 
  FXAutolock al(m_Lock);
  if ( m_pFrame )
  {
    freeTVFrame( m_pFrame );
    m_pFrame = NULL;
  }
  if ( m_pData )
  {
    free( m_pData );
    m_pData = NULL;
  }
  if ( m_SliderBuffer )
  {
    free( m_SliderBuffer );
    m_SliderBuffer = NULL;
  }
  m_AvgVal = 0;
  m_cFrames = 0;
  m_FramesRange = range; 
}

pTVFrame CAverager::GetAvgFrame() 
{ 
  FXAutolock al(m_Lock);
  pTVFrame retV=makecopyTVFrame(m_pFrame);
  return retV; 
}

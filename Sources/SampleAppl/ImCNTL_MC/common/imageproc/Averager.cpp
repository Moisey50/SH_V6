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
m_FramesRange(DEFAULT_FRAMES_RANGE)
{
}

CAverager::~CAverager()
{
	Reset();
}

void CAverager::Reset()
{
    CAutolock al(m_Lock);
	if (m_pFrame) freeTVFrame(m_pFrame); m_pFrame = NULL;
    if (m_pData)  free(m_pData);         m_pData=NULL;
	if (m_SliderBuffer) free(m_SliderBuffer); m_SliderBuffer=NULL;
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
    CAutolock al(m_Lock);
    m_AvgVal = 0;
    if ((!m_cFrames) || (m_WrkFormat!=Frame->lpBMIH->biCompression))
	{
	    if (m_pFrame) freeTVFrame(m_pFrame); m_pFrame = NULL;
        if (m_pData)  free(m_pData);         m_pData=NULL;
	    if (m_SliderBuffer) free(m_SliderBuffer); m_SliderBuffer=NULL;
	    m_AvgVal = 0;
	    m_cFrames = 0;
		m_pFrame = (pTVFrame)malloc(sizeof(TVFrame));
		m_pFrame->lpBMIH = (LPBITMAPINFOHEADER)malloc(getsize4BMIH(Frame));
        m_pData=(LPDWORD)malloc(sizeof(DWORD)*GetImageSize(Frame->lpBMIH));
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
	ASSERT(m_pFrame);
    switch (Frame->lpBMIH->biCompression)
    {
    case BI_Y8:
        {
	        LPBYTE Dst = GetData(m_pFrame);
	        LPBYTE Src = GetData(Frame);
            LPDWORD Avg = m_pData;
	        LPBYTE End = Dst + m_pFrame->lpBMIH->biWidth * m_pFrame->lpBMIH->biHeight;
	        switch (m_Mode)
	        {
	        case AVG_INFINITE_UNIFORM:
                m_cFrames++;
	            while (Dst < End)
	            {
                    *Avg+=*Src; 
                    *Dst=(BYTE)(*Avg/m_cFrames);
                    Dst++; Src++; Avg++;
                }
                break;
            case AVG_INFINITE_PASTFADING:
                m_cFrames++;
	            while (Dst < End)
	            {
                    *Avg=(3* *Avg+*Src)/4; 
                    *Dst=(BYTE)(*Avg);
                    Dst++; Src++; Avg++;
                }
                break;
            case AVG_SLIDEWINDOW:
                m_cFrames++;
                if (m_cFrames<m_FramesRange)
                {
                    memcpy(m_SliderBuffer+GetImageSize(m_pFrame->lpBMIH)*m_cFrames,GetData(Frame),GetImageSize(m_pFrame->lpBMIH));
	                while (Dst < End)
	                {
                        *Avg+=*Src; 
                        *Dst=(BYTE)(*Avg/m_cFrames);
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
            }
            break;
        }
    case BI_YUV9:
        {
	        LPBYTE Dst = GetData(m_pFrame);
	        LPBYTE Src = GetData(Frame);
            LPDWORD Avg = m_pData;
	        LPBYTE End = Dst + 9*m_pFrame->lpBMIH->biWidth * m_pFrame->lpBMIH->biHeight/8;
	        switch (m_Mode)
	        {
	        case AVG_INFINITE_UNIFORM:
                m_cFrames++;
	            while (Dst < End)
	            {
                    *Avg+=*Src; 
                    *Dst=(BYTE)(*Avg/m_cFrames);
                    Dst++; Src++; Avg++;
                }
                break;
            case AVG_INFINITE_PASTFADING:
                m_cFrames++;
	            while (Dst < End)
	            {
                    *Avg=(3* *Avg+*Src)/4; 
                    *Dst=(BYTE)(*Avg);
                    Dst++; Src++; Avg++;
                }
                break;
            case AVG_SLIDEWINDOW:
                m_cFrames++;
                if (m_cFrames<m_FramesRange)
                {
                    memcpy(m_SliderBuffer+GetImageSize(m_pFrame->lpBMIH)*m_cFrames,GetData(Frame),GetImageSize(m_pFrame->lpBMIH));
	                while (Dst < End)
	                {
                        *Avg+=*Src; 
                        *Dst=(BYTE)(*Avg/m_cFrames);
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
            }
            break;
        }
    case BI_YUV12:
        {
	        LPBYTE Dst = GetData(m_pFrame);
	        LPBYTE Src = GetData(Frame);
            LPDWORD Avg = m_pData;
	        LPBYTE End = Dst + 3*m_pFrame->lpBMIH->biWidth * m_pFrame->lpBMIH->biHeight/2;
	        switch (m_Mode)
	        {
	        case AVG_INFINITE_UNIFORM:
                m_cFrames++;
	            while (Dst < End)
	            {
                    *Avg+=*Src; 
                    *Dst=(BYTE)(*Avg/m_cFrames);
                    Dst++; Src++; Avg++;
                }
                break;
            case AVG_INFINITE_PASTFADING:
                m_cFrames++;
	            while (Dst < End)
	            {
                    *Avg=(3* *Avg+*Src)/4; 
                    *Dst=(BYTE)(*Avg);
                    Dst++; Src++; Avg++;
                }
                break;
            case AVG_SLIDEWINDOW:
                m_cFrames++;
                if (m_cFrames<m_FramesRange)
                {
                    memcpy(m_SliderBuffer+GetImageSize(m_pFrame->lpBMIH)*m_cFrames,GetData(Frame),GetImageSize(m_pFrame->lpBMIH));
	                while (Dst < End)
	                {
                        *Avg+=*Src; 
                        *Dst=(BYTE)(*Avg/m_cFrames);
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
            }
            break;
        }
    case BI_Y16:
        {
	        LPWORD Dst = (LPWORD)GetData(m_pFrame);
	        LPWORD Src = (LPWORD)GetData(Frame);
            DWORD *Avg = m_pData;
	        LPWORD End = Dst + m_pFrame->lpBMIH->biWidth * m_pFrame->lpBMIH->biHeight;
	        switch (m_Mode)
	        {
	        case AVG_INFINITE_UNIFORM:
                m_cFrames++;
	            while (Dst < End)
	            {
                    *Avg+=*Src; 
                    *Dst=(WORD)(*Avg/m_cFrames);
                    Dst++; Src++; Avg++;
                }
                break;
            case AVG_INFINITE_PASTFADING:
                m_cFrames++;
	            while (Dst < End)
	            {
                    *Avg=(3* *Avg+*Src)/4; 
                    *Dst=(WORD)(*Avg);
                    Dst++; Src++; Avg++;
                }
                break;
            case AVG_SLIDEWINDOW:
                m_cFrames++;
                if (m_cFrames<m_FramesRange)
                {
                    memcpy(m_SliderBuffer+GetImageSize(m_pFrame->lpBMIH)*m_cFrames,GetData(Frame),GetImageSize(m_pFrame->lpBMIH));
	                while (Dst < End)
	                {
                        *Avg+=*Src; 
                        *Dst=(WORD)(*Avg/m_cFrames);
                        Dst++; Src++; Avg++;
                    }
                }
                else
                {
                    int off=m_cFrames%m_FramesRange;
                    LPWORD oldF=(LPWORD)(m_SliderBuffer+GetImageSize(m_pFrame->lpBMIH)*off);
                    LPWORD oldFs=oldF; LPWORD endS=(LPWORD)((LPBYTE)oldF+GetImageSize(m_pFrame->lpBMIH));
                    DWORD* aS=Avg;
                    while (oldFs<endS)
                    {
                        *aS-=*oldFs; oldFs++; aS++;
                    }
                    memcpy(oldF,GetData(Frame),GetImageSize(m_pFrame->lpBMIH));
	                while (Dst < End)
	                {
                        *Avg+=*Src; 
                        *Dst=(WORD)(*Avg/m_FramesRange);
                        Dst++; Src++; Avg++;
                    }
                }
                break;
            }
            break;
        }
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
	}
	return "Unknown";
}

void CAverager::SetMode(int mode) 
{ 
	CAutolock al(m_Lock);
	if (m_pFrame) freeTVFrame(m_pFrame); m_pFrame = NULL;
	if (m_pData)  free(m_pData);         m_pData=NULL;
	if (m_SliderBuffer) free(m_SliderBuffer); m_SliderBuffer=NULL;
	m_AvgVal = 0;
	m_cFrames = 0;
	m_Mode = mode; 
}

void CAverager::SetFramesRange(int range) 
{ 
	CAutolock al(m_Lock);
	if (m_pFrame) freeTVFrame(m_pFrame); m_pFrame = NULL;
	if (m_pData)  free(m_pData);         m_pData=NULL;
	if (m_SliderBuffer) free(m_SliderBuffer); m_SliderBuffer=NULL;
	m_AvgVal = 0;
	m_cFrames = 0;
	m_FramesRange = range; 
}

pTVFrame CAverager::GetAvgFrame() 
{ 
	CAutolock al(m_Lock);
	pTVFrame retV=makecopyTVFrame(m_pFrame);
	return retV; 
}

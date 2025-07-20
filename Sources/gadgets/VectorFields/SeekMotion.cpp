// SeekMotion.cpp: implementation of the CSeekMotion class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SeekMotion.h"
#include "sumofabsdifferences.h"
#include <math\intf_sup.h>

__forceinline bool EstimateMV(vfvector &v, LPBYTE cur, LPBYTE ref, unsigned int off, unsigned int step, int ROI)
{
    double sadMin=100000;
    double sad=0;
    POINT tr; 

    sad=_sad8x8(cur+off, ref+off, step);
    if (sad<sadMin)
    {
        v.x=0;
        v.y=0;
        sadMin=sad;
    }
    for (tr.y=-ROI; tr.y<=ROI; tr.y++)
    {
        for (tr.x=-ROI; tr.x<=ROI; tr.x++)
        {
            sad=_sad8x8(cur+off+tr.y*step+tr.x, ref+off, step) + sqrt((double)(tr.x*tr.x+tr.y*tr.y));
            if (sad+1<sadMin)
            {
                v.x=tr.x;
                v.y=tr.y;
                sadMin=sad;
            }
        }
    } 
    return true;
}

__forceinline bool EstimateMV(vfvector &v, vfvector& pred, LPBYTE cur, LPBYTE ref, unsigned int off, unsigned int step, int ROI)
{
    double sadMin=100000;
    double sad=0;
    POINT tr; 

    sad=_sad8x8(cur+off, ref+off, step);
    if (sad<sadMin)
    {
        v.x=0;
        v.y=0;
        sadMin=sad;
    }
    for (tr.y=-ROI+(int)pred.y; tr.y<=ROI+(int)pred.y; tr.y++)
    {
        for (tr.x=-ROI+(int)pred.x; tr.x<=ROI+(int)pred.x; tr.x++)
        {
            sad=_sad8x8(cur+off+tr.y*step+tr.x, ref+off, step) + sqrt((double)(tr.x*tr.x+tr.y*tr.y));
            if (sad+1<sadMin)
            {
                v.x=tr.x;
                v.y=tr.y;
                sadMin=sad;
            }
        }
    } 
    return true;
}

__forceinline bool EstimateMV4x4(vfvector &v, LPBYTE cur, LPBYTE ref, unsigned int off, unsigned int step, int ROI)
{
    double sadMin=100000;
    double sad=0;
    POINT tr; 

    sad=_sad8x8(cur+off, ref+off, step);
    if (sad<sadMin)
    {
        v.x=0;
        v.y=0;
        sadMin=sad;
    }
    for (tr.y=-ROI; tr.y<=ROI; tr.y++)
    {
        for (tr.x=-ROI; tr.x<=ROI; tr.x++)
        {
            sad=_sad4x4(cur+off+tr.y*step+tr.x, ref+off, step) + sqrt((double)(tr.x*tr.x+tr.y*tr.y));
            if (sad+1<sadMin)
            {
                v.x=tr.x;
                v.y=tr.y;
                sadMin=sad;
            }
        }
    } 
    return true;
}

__forceinline bool EstimateMV4x4(vfvector &v, vfvector& pred, LPBYTE cur, LPBYTE ref, unsigned int off, unsigned int step, int ROI)
{
    double sadMin=100000;
    double sad=0;
    POINT tr; 

    sad=_sad8x8(cur+off, ref+off, step);
    if (sad<sadMin)
    {
        v.x=0;
        v.y=0;
        sadMin=sad;
    }
    for (tr.y=-ROI+(int)pred.y; tr.y<=ROI+(int)pred.y; tr.y++)
    {
        for (tr.x=-ROI+(int)pred.x; tr.x<=ROI+(int)pred.x; tr.x++)
        {
            sad=_sad4x4(cur+off+tr.y*step+tr.x, ref+off, step) + sqrt((double)(tr.x*tr.x+tr.y*tr.y));
            if (sad+1<sadMin)
            {
                v.x=tr.x;
                v.y=tr.y;
                sadMin=sad;
            }
        }
    } 
    return true;
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSeekMotion::CSeekMotion(const pTVFrame current, const pTVFrame ref):
          m_Cur(NULL), m_Ref(NULL),
          //m_Width(0), m_Height(0),
          m_MaxBlckX(0),m_MaxBlckY(0), m_BlcksNmb(0),
          vfvectors(NULL)
{
    m_Cur=current;
    m_Ref=ref;
    m_SeekArea=DEF_AREA_SEARCH;
}

CSeekMotion::~CSeekMotion()
{
    if (vfvectors) free(vfvectors); vfvectors=NULL;
}

bool CSeekMotion::Calc(int blckSize)
{
    int      Width, Height;
    if (m_Cur->lpBMIH->biWidth!=m_Ref->lpBMIH->biWidth) return false;
    if (m_Cur->lpBMIH->biHeight!=m_Ref->lpBMIH->biHeight) return false;
    Width=m_Cur->lpBMIH->biWidth;
    Height=m_Cur->lpBMIH->biHeight;
    switch(blckSize) 
    {
    case BLCKSIZE_8x8:
        {
            m_MaxBlckX=(Width-16)/8;
            m_MaxBlckY=(Height-16)/8;
            m_StepX=m_StepY=8;
            m_BlcksNmb=m_MaxBlckX*m_MaxBlckY;
            vfvectors=(pvfvector)malloc(sizeof(vfvector)*m_BlcksNmb);
            memset(vfvectors,0,sizeof(vfvector)*m_BlcksNmb);
            for (int y=0; y<m_MaxBlckY; y++)
            {
                for (int x=0; x<m_MaxBlckX; x++)
                {
                    EstimateMV(vfvectors[x+y*m_MaxBlckX],GetData(m_Cur), GetData(m_Ref), (x+1+8*(y+1)*(m_MaxBlckX+2))*8,Width,m_SeekArea);
                }
            }
        }
        break;
    case BLCKSIZE_4x4:
        {
            m_MaxBlckX=(Width-8)/4;
            m_MaxBlckY=(Height-8)/4;
            m_StepX=m_StepY=4;
            m_BlcksNmb=m_MaxBlckX*m_MaxBlckY;
            vfvectors=(pvfvector)malloc(sizeof(vfvector)*m_BlcksNmb);
            memset(vfvectors,0,sizeof(vfvector)*m_BlcksNmb);
            for (int y=0; y<m_MaxBlckY; y++)
            {
                for (int x=0; x<m_MaxBlckX; x++)
                {
                    EstimateMV4x4(vfvectors[x+y*m_MaxBlckX],GetData(m_Cur), GetData(m_Ref), (x+1+4*(y+1)*(m_MaxBlckX+2))*4,Width,m_SeekArea);
                }
            }
        }
        break;
    default:
        return false;
    }
    return true;
}

bool CSeekMotion::Calc(int blckSize,CVectorFieldFrame* pvf)
{
    int      Width, Height;
    if (m_Cur->lpBMIH->biWidth!=m_Ref->lpBMIH->biWidth) return false;
    if (m_Cur->lpBMIH->biHeight!=m_Ref->lpBMIH->biHeight) return false;
    Width=m_Cur->lpBMIH->biWidth;
    Height=m_Cur->lpBMIH->biHeight;
    switch(blckSize) 
    {
    case BLCKSIZE_8x8:
        {
            m_MaxBlckX=(Width-16)/8;
            m_MaxBlckY=(Height-16)/8;
            m_StepX=m_StepY=8;
            m_BlcksNmb=m_MaxBlckX*m_MaxBlckY;
            vfvectors=(pvfvector)malloc(sizeof(vfvector)*m_BlcksNmb);
            memset(vfvectors,0,sizeof(vfvector)*m_BlcksNmb);
            for (int y=0; y<m_MaxBlckY; y++)
            {
                for (int x=0; x<m_MaxBlckX; x++)
                {
                    vfvector v=pvf->vfvectors[x+1+(y+1)*pvf->sizeX];
                    EstimateMV(vfvectors[x+y*m_MaxBlckX],v,GetData(m_Cur), GetData(m_Ref), (x+1+8*(y+1)*(m_MaxBlckX+2))*8,Width,m_SeekArea);
                }
            }
        }
        break;
    case BLCKSIZE_4x4:
        {
            m_MaxBlckX=(Width-8)/4;
            m_MaxBlckY=(Height-8)/4;
            m_StepX=m_StepY=4;
            m_BlcksNmb=m_MaxBlckX*m_MaxBlckY;
            vfvectors=(pvfvector)malloc(sizeof(vfvector)*m_BlcksNmb);
            memset(vfvectors,0,sizeof(vfvector)*m_BlcksNmb);
            for (int y=0; y<m_MaxBlckY; y++)
            {
                for (int x=0; x<m_MaxBlckX; x++)
                {
                    vfvector v=pvf->vfvectors[x+1+(y+1)*pvf->sizeX];
                    EstimateMV4x4(vfvectors[x+y*m_MaxBlckX],v,GetData(m_Cur), GetData(m_Ref), (x+1+4*(y+1)*(m_MaxBlckX+2))*4,Width,m_SeekArea);
                }
            }
        }
        break;
    default:
        return false;
    }
    return true;
}

bool  CSeekMotion::GetVectors(CVectorFieldFrame& vf)
{
    if (vf.vfvectors) free(vf.vfvectors);

    vf.sizeX=m_MaxBlckX+2;
    vf.sizeY=m_MaxBlckY+2;
    vf.stepX=m_StepX;
    vf.stepY=m_StepY;
    vf.vfvectors=(pvfvector)malloc(sizeof(vfvector)*vf.sizeX*vf.sizeY);
    memset(vf.vfvectors,0,sizeof(vfvector)*vf.sizeX*vf.sizeY);
    for (int i=0; i<m_MaxBlckY; i++)
    {
        memcpy(vf.vfvectors+(i+1)*vf.sizeX+1,vfvectors+i*m_MaxBlckX,m_MaxBlckX*sizeof(vfvector));
    }
    return true;
}

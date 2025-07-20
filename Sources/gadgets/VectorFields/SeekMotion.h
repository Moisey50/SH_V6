// SeekMotion.h: interface for the CSeekMotion class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SEEKMOTION_H__5E0E2834_F60D_4304_8110_4682D218FF68__INCLUDED_)
#define AFX_SEEKMOTION_H__5E0E2834_F60D_4304_8110_4682D218FF68__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <video\TVFrame.h>
#include "VectorFieldFrame.h"

#define BLCKSIZE_8x8 1
#define BLCKSIZE_4x4 2

#define DEF_AREA_SEARCH 3

class CSeekMotion  
{
protected:
    int      m_SeekArea;
    const pTVFrame m_Cur;
    const pTVFrame m_Ref;
    //int      m_Width, m_Height;
    int      m_MaxBlckX,m_MaxBlckY, m_StepX, m_StepY, m_BlcksNmb;
    pvfvector  vfvectors;
public:
	        CSeekMotion(const pTVFrame current, const pTVFrame ref);
           ~CSeekMotion();
    void    SetSeekArea(int area) { m_SeekArea=area; }
    int     GetSeekArea()         { return m_SeekArea; }
    bool    Calc(int blckSize);
    bool    Calc(int blckSize,CVectorFieldFrame* pvf);
    bool    GetVectors(CVectorFieldFrame& vf);
};

#endif // !defined(AFX_SEEKMOTION_H__5E0E2834_F60D_4304_8110_4682D218FF68__INCLUDED_)

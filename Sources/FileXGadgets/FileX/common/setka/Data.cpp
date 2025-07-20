// Data.cpp: implementation of the CData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Data.h"
#include <files/imgfiles.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CData::CData():
m_pSrcFrame(NULL),
m_pDstFrame(NULL),
m_pRefFrame(NULL),
m_pRectFrame(NULL),
m_pVertFrame(NULL),
m_pVFltFrame(NULL),
m_pHorzFrame(NULL),
m_pHFltFrame(NULL),
m_pVLinesFrame(NULL),
m_pHLinesFrame(NULL)
{
}

CData::~CData()
{
	Reset();
}

void CData::Reset()
{
	freeTVFrame(m_pSrcFrame);
	m_pSrcFrame = NULL;
	freeTVFrame(m_pDstFrame);
	m_pDstFrame = NULL;
	freeTVFrame(m_pRefFrame);
	m_pRefFrame = NULL;
	freeTVFrame(m_pRectFrame);
	m_pRectFrame = NULL;
	freeTVFrame(m_pVertFrame);
	m_pVertFrame = NULL;
	freeTVFrame(m_pVFltFrame);
	m_pVFltFrame = NULL;
	freeTVFrame(m_pHorzFrame);
	m_pHorzFrame = NULL;
	freeTVFrame(m_pHFltFrame);
	m_pHFltFrame = NULL;
	freeTVFrame(m_pVLinesFrame);
	m_pVLinesFrame = NULL;
	freeTVFrame(m_pHLinesFrame);
	m_pHLinesFrame = NULL;
	m_VertLines.RemoveAll();
	m_HorzLines.RemoveAll();
	m_Nodes.RemoveAll();
	m_Mapper.Reset();
	m_bRulerSet = FALSE;
	m_Dist.RemoveAll();
}


BOOL CData::LoadImage(LPCTSTR fileName)
{
	if (fileName)
		m_pSrcFrame = newTVFrame(loadDIB(fileName), NULL);
	return (m_pSrcFrame != NULL);
}

BOOL CData::LoadFrame(const pTVFrame Frame)
{
	if (Frame)
		m_pSrcFrame = makecopyTVFrame(Frame);
	return (m_pSrcFrame != NULL);
}

// Data.h: interface for the CData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATA_H__2DB7A754_0AE2_4D38_823C_1B10FEDBCF21__INCLUDED_)
#define AFX_DATA_H__2DB7A754_0AE2_4D38_823C_1B10FEDBCF21__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <video\TVFrame.h>
#include <imageproc\clusters\clusters.h>
#include "Lines.h"
#include "Node.h"
#include "Mapper.h"
#include "Distortion.h"

class CData
{
public:
	CData();
	virtual ~CData();

	BOOL	LoadImage(LPCTSTR fileName);
	BOOL	LoadFrame(const pTVFrame Frame);
	void	Reset();
	void	SetRuler(RECT& rc) { memcpy(&m_Ruler, &rc, sizeof(RECT)); m_bRulerSet = TRUE; };

	CMapper*	GetMapper() { return &m_Mapper; };
	BOOL		GetImgCenter(POINT& pt)
	{
		if (!m_pSrcFrame) return FALSE;
		pt.x = m_pSrcFrame->lpBMIH->biWidth / 2;
		pt.y = m_pSrcFrame->lpBMIH->biHeight / 2;
		return TRUE;
	}


	friend class CProcStepLoader;
	friend class CProcStepFilter;
	friend class CProcStepRefine;
	friend class CProcStepRectify;
	friend class CProcStepVert;
	friend class CProcStepVFlt;
	friend class CProcStepHorz;
	friend class CProcStepHFlt;
	friend class CProcStepRefineVLines;
	friend class CProcStepApproxVLines;
	friend class CProcStepRefineHLines;
	friend class CProcStepApproxHLines;
	friend class CProcStepShowLines;
	friend class CProcStepNodes;
	friend class CProcStepRefNodes;
	friend class CProcStepMapper;
	friend class CSetkaDlg;

private:
	pTVFrame	m_pSrcFrame;
	pTVFrame	m_pDstFrame;
	pTVFrame	m_pRefFrame;
	pTVFrame	m_pRectFrame;
	pTVFrame	m_pVertFrame;
	pTVFrame	m_pVFltFrame;
	pTVFrame	m_pHorzFrame;
	pTVFrame	m_pHFltFrame;
	pTVFrame	m_pVLinesFrame;
	pTVFrame	m_pHLinesFrame;
	CClusters	m_VertLinesClusters;
	CClusters	m_HorzLinesClusters;
	CLines		m_VertLines;
	CLines		m_HorzLines;
	CNodes		m_Nodes;
	CMapper		m_Mapper;
	RECT		m_Ruler;
	BOOL		m_bRulerSet;
	CDistortion	m_Dist;
};

#endif // !defined(AFX_DATA_H__2DB7A754_0AE2_4D38_823C_1B10FEDBCF21__INCLUDED_)

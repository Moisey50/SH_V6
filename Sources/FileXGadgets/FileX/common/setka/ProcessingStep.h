// ProcessingStep.h: interface for the CProcessingStep class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROCESSINGSTEP_H__80B9B94F_A63C_4BA0_9DD0_5A031561E9F0__INCLUDED_)
#define AFX_PROCESSINGSTEP_H__80B9B94F_A63C_4BA0_9DD0_5A031561E9F0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Data.h"
#include <video\DIBView.h>

class CProcessingStep  
{
public:
	CProcessingStep(const char* strDescr = "");
	virtual ~CProcessingStep();

	virtual BOOL	Process(CData* Data, void* params) = 0;
	virtual void	Draw(CData* Data, HDC hdc, RECT& rc, CDIBViewBase* view) = 0;
	BOOL			IsComplete() { return m_bComplete; };
	void			SetComplete(BOOL bComplete = TRUE) { m_bComplete = bComplete; };
	const char*		GetDescription() { return m_strDescr; };
	void			SetKeyStep(BOOL bKeyStep) { m_bKeyStep = bKeyStep; };
	BOOL			IsKeyStep() { return m_bKeyStep; };

private:
	BOOL	m_bComplete;
	CString	m_strDescr;
	BOOL	m_bKeyStep;
};

#endif // !defined(AFX_PROCESSINGSTEP_H__80B9B94F_A63C_4BA0_9DD0_5A031561E9F0__INCLUDED_)

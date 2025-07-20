// DibRect.h: interface for the CDibRect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIBRECT_H__1C4E701D_3635_4408_A55A_2633DCE24363__INCLUDED_)
#define AFX_DIBRECT_H__1C4E701D_3635_4408_A55A_2633DCE24363__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <video\DibVSq.h>

class CDibRect : public CDibVSq
{
	BOOL m_bHasRect;
	void* m_pClient;
public:
	CDibRect();
	virtual ~CDibRect();
	virtual void AddSquare(CSquare* Square);
	void SetClient(void* pClient);
protected:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_DIBRECT_H__1C4E701D_3635_4408_A55A_2633DCE24363__INCLUDED_)

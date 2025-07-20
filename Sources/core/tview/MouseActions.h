// MouseActions.h: interface for the CMouseActions class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOUSEACTIONS_H__816B41AE_7BE7_4EE5_A0B7_1BC941CC3959__INCLUDED_)
#define AFX_MOUSEACTIONS_H__816B41AE_7BE7_4EE5_A0B7_1BC941CC3959__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMouseActions  
{
	const enum { MA_NONE, MA_DRAG, MA_LINE, MA_RECT };
	UINT m_Mode;
	CRect m_ActionRect;
public:
	CMouseActions() { m_Mode = MA_NONE; };
	virtual ~CMouseActions() {};

	BOOL IsDragging() { return m_Mode == MA_DRAG; };
	BOOL IsLine() { return m_Mode == MA_LINE; };
	BOOL IsRect() { return m_Mode == MA_RECT; };
	BOOL IsNone() { return m_Mode == MA_NONE; };
	CPoint& StartPoint() { return m_ActionRect.TopLeft(); };
	CPoint& EndPoint() { return m_ActionRect.BottomRight(); };
	CSize GetTotalShift() { return m_ActionRect.Size(); };
	void StartDrag(CPoint& pt) { m_Mode = MA_DRAG; StartPoint() = EndPoint() = pt; };
	void StartLine(CPoint& pt) { m_Mode = MA_LINE; StartPoint() = EndPoint() = pt; };
	void StartRect(CPoint& pt) { m_Mode = MA_RECT; StartPoint() = EndPoint() = pt; };
	void Stop() { m_Mode = MA_NONE; };
};

#endif // !defined(AFX_MOUSEACTIONS_H__816B41AE_7BE7_4EE5_A0B7_1BC941CC3959__INCLUDED_)

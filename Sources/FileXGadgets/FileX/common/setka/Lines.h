// Lines.h: interface for the CLines class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LINES_H__75326D96_029D_4210_B029_B880AF000EF0__INCLUDED_)
#define AFX_LINES_H__75326D96_029D_4210_B029_B880AF000EF0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "afxtempl.h"

typedef struct LINEAPPROXPARAMS
{
	bool	valid;		// approximation validness
	double	a, b, c;	// parabolic approximation
	double	k;			// x4 approximation
	double	argMin, argMax;	// argument limits
	double	meanLevel;	// line's approximate value in the middle point
	int		index;		// line's ordinal
}LINEAPPROXPARAMS, *LPLINEAPPROXPARAMS;

class CLine : public CArray<POINT, POINT&>
{
public:
	CLine() { memset(&m_Params, 0, sizeof(m_Params)); };
	virtual ~CLine() { };

	LPLINEAPPROXPARAMS	GetParams()	{ return &m_Params; };
private:
	LINEAPPROXPARAMS	m_Params;
};

class CLines  
{
public:
	CLines();
	virtual ~CLines();

	CLine*	AddLine();
	void	MoveLine(int indexSrc, int indexDst);
	CLine*	GetAt(int index);
	int		Index(CLine* Line);
	void	RemoveAt(int index, int count = 0);
	int		GetCount();
	void	RemoveAll();

private: 
	CArray<CLine*, CLine*>	m_Lines;
};

#endif // !defined(AFX_LINES_H__75326D96_029D_4210_B029_B880AF000EF0__INCLUDED_)

// GDIBank.h: interface for the CGDIBank class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GDIBANK_H__960C085F_160B_4B21_A55E_9938AFBF2F03__INCLUDED_)
#define AFX_GDIBANK_H__960C085F_160B_4B21_A55E_9938AFBF2F03__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef FX_EXT_SHBASE
    #include <shabse\shbase.h>
#else

class FX_EXT_SHBASE CPensBank : public CMapPtrToPtr
{
public:
	CPensBank();
	~CPensBank();

	CPen*	GetPen(int nPenStyle, int nWidth, COLORREF crColor);

protected:
	void*	Hash(int nPenStyle, int nWidth, COLORREF crColor);
};

class FX_EXT_SHBASE CBrushesBank : public CMapPtrToPtr
{
public:
	CBrushesBank();
	~CBrushesBank();

	CBrush*	GetBrush(int nBrushStyle, int nHatch, COLORREF crColor);

protected:
	void*	Hash(int nBrushStyle, int nHatch, COLORREF crColor);
};

class FX_EXT_SHBASE CGDIBank  
{
	CPensBank		m_Pens;
	CBrushesBank	m_Brushes;
public:
	CGDIBank();
	~CGDIBank();

	CPen* GetPen(int nPenStyle, int nWidth, COLORREF crColor)
	{
		return m_Pens.GetPen(nPenStyle, nWidth, crColor);
	}
	CBrush* GetBrush(int nBrushStyle, int nHatch, COLORREF crColor)
	{
		return m_Brushes.GetBrush(nBrushStyle, nHatch, crColor);
	}
};

#endif // #ifndef FX_EXT_SHBASE

#endif // !defined(AFX_GDIBANK_H__960C085F_160B_4B21_A55E_9938AFBF2F03__INCLUDED_)

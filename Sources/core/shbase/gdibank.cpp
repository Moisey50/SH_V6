// GDIBank.cpp: implementation of the CGDIBank class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <shbase\shbase.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Pens
#ifdef _DEBUG
int cnt=0;
#endif

CPensBank::CPensBank()
{
}

CPensBank::~CPensBank()
{
	POSITION pos = GetStartPosition();
	void* key;
	CPen* pen;
	while (pos)
	{
		GetNextAssoc(pos, key, (LPVOID&)pen);
		if (pen)
		{
			pen->DeleteObject();
			delete pen;
		}
	}
}

CPen* CPensBank::GetPen(int nPenStyle, int nWidth, COLORREF crColor)
{
	void* key = Hash(nPenStyle, nWidth, crColor);
	CPen* pen = NULL;
	if (!Lookup(key, (LPVOID&)pen) || !pen)
	{
		pen = new CPen;
        #ifdef _DEBUG
            cnt++;
            TRACE("+++ Objects nmb %d\n",cnt);
        #endif
		pen->CreatePen(nPenStyle, nWidth, crColor);
        
		SetAt(key, pen);
	}
	return pen;
}

void* CPensBank::Hash(int nPenStyle, int nWidth, COLORREF crColor)
{
	DWORD key = crColor;
	BYTE msk = (0xE0 & (BYTE)(nPenStyle << 5)) | (0x1F & (BYTE)nWidth);
	key = (key & 0x00FFFFFF) | (((DWORD)msk) << 24);
	return (void*)(size_t)key;
}

// Brushes

CBrushesBank::CBrushesBank()
{
}

CBrushesBank::~CBrushesBank()
{
	POSITION pos = GetStartPosition();
	void* key;
	CBrush* brush;
	while (pos)
	{
		GetNextAssoc(pos, key, (LPVOID&)brush);
		if (brush)
		{
			brush->DeleteObject();
			delete brush;
		}
	}
}

CBrush*	CBrushesBank::GetBrush(int nBrushStyle, int nHatch, COLORREF crColor)
{
	void* key = Hash(nBrushStyle, nHatch, crColor);
	CBrush* brush = NULL;
	if (!Lookup(key, (LPVOID&)brush) || !brush)
	{
		brush = new CBrush;
        #ifdef _DEBUG
            cnt++;
            TRACE("+++ Objects nmb %d\n",cnt);
        #endif
		LOGBRUSH lBrush;
		lBrush.lbStyle = (UINT)nBrushStyle;
		lBrush.lbHatch = (LONG)nHatch;
		lBrush.lbColor = crColor;
		brush->CreateBrushIndirect(&lBrush);
        
		SetAt(key, brush);
	}
	return brush;
}

void* CBrushesBank::Hash(int nBrushStyle, int nHatch, COLORREF crColor)
{
	DWORD key = crColor;
	BYTE msk = (0xE0 & (BYTE)(nHatch << 5)) | (0x1F & (BYTE)nBrushStyle);
	key = (key & 0x00FFFFFF) | (((DWORD)msk) << 24);
	return (void*) (size_t) key;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGDIBank::CGDIBank()
{

}

CGDIBank::~CGDIBank()
{

}

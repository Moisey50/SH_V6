// WMFGadget.h: interface for the WMF class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WMFGADGET_H__D9382CDE_02DD_4324_83C3_9740A57323F3__INCLUDED_)
#define AFX_WMFGADGET_H__D9382CDE_02DD_4324_83C3_9740A57323F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Gadgets\gadbase.h>
#include <gadgets\FigureFrame.h>

#define SCALE 10

class WMF : public CFilterGadget  
{
private:
	CArray <CFont*,CFont*> m_Fonts;
    CUIntArray m_Sizes;
    HPEN    m_RectanglePen;
    CRect   m_PictRect;
public:
	        WMF();
	virtual void ShutDown();
    virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
    bool    Pic2Scr(const DPOINT& point, CPoint& res);

    bool    DrawTexts(HDC dc, const CDataFrame *df);
	bool    DrawFigures(HDC dc, const CDataFrame *df);
	bool    DrawRectangles(HDC dc, const CDataFrame* df);
	bool    DrawQuantity(HDC dc, const CDataFrame *df);
    bool    CreateFontWithSize( UINT uiSize ) ;
    CFont * GetFont( UINT uiSize )
    {
        for ( int i = 0 ; i <= m_Fonts.GetUpperBound(); i++ )
        {
            if ( m_Sizes[i] == uiSize ) return m_Fonts[i];
        }
        if (CreateFontWithSize( uiSize ))
            return m_Fonts[m_Fonts.GetUpperBound()];
        else
            return NULL ;
    }
    DECLARE_RUNTIME_GADGET(WMF);
};

#endif // !defined(AFX_WMFGADGET_H__D9382CDE_02DD_4324_83C3_9740A57323F3__INCLUDED_)

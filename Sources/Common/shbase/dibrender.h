// DIBRender.h: interface for the CDIBRender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIBRENDER_H__FC26D7DC_550D_4660_9D21_48F8851CE955__INCLUDED_)
#define AFX_DIBRENDER_H__FC26D7DC_550D_4660_9D21_48F8851CE955__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <video\DIBView.h>
#include <gadgets\MetafileFrame.h>

class FX_EXT_SHBASE CDIBRender : public CDIBView
{
public:
	CRect	    m_Rect;
	CPen        m_RedPen;
    CPen        m_PurplePen;
    FXLockObject m_Lock;
    CMFHelpers  m_Metafiles;
	void        DrawMetafiles(HDC hdc);
public:
	        CDIBRender(LPCTSTR name="");
	virtual ~CDIBRender();
	virtual BOOL Create(CWnd* pParentWnd, DWORD dwAddStyle=0);
	void    Render(const CDataFrame* pDataFrame);
	bool    Draw(HDC hdc,RECT& rc);
};


#endif // !defined(AFX_DIBRENDER_H__FC26D7DC_550D_4660_9D21_48F8851CE955__INCLUDED_)

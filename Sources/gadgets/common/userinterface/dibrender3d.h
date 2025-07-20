// DIBRender3D.h: interface for the CDIBRender3D class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIBRENDER3D_H__FC26D7DC_550D_4660_9D21_48F8851CE955__INCLUDED_)
#define AFX_DIBRENDER3D_H__FC26D7DC_550D_4660_9D21_48F8851CE955__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <video\DIBView.h>
#include <gadgets\gadbase.h>
#include <gadgets\MetafileFrame.h>
#include <video\DIBView3D.h>


class CDIBRender3D : public CDIBView3D
{
public:
	CRect	    m_Rect;
	CPen        m_RedPen;
    CPen        m_PurplePen;
public:
	        CDIBRender3D(LPCTSTR name="");
	virtual ~CDIBRender3D();
	virtual BOOL Create(CWnd* pParentWnd, DWORD dwAddStyle=0);
	void    Render(const CDataFrame* pDataFrame);
	bool    Draw(HDC hdc,RECT& rc);
};


#endif // !defined(AFX_DIBRENDER3D_H__FC26D7DC_550D_4660_9D21_48F8851CE955__INCLUDED_)

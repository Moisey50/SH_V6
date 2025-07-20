// RenderViewFrame.cpp : implementation file
//

#include "stdafx.h"
#include "RenderViewFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRenderViewFrame

CRenderViewFrame::CRenderViewFrame(CRenderGlyph* Glyph) :
m_pGlyph(Glyph),
m_bBuiltIn(TRUE),
m_ViewWnd(NULL)
{
}

CRenderViewFrame::~CRenderViewFrame()
{
}


BEGIN_MESSAGE_MAP(CRenderViewFrame, CStatic)
	//{{AFX_MSG_MAP(CRenderViewFrame)
	ON_WM_SIZE()
//	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
    ON_MESSAGE(UM_GLIPH_CHANGESIZE, OnGlyphChangeSize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRenderViewFrame message handlers

BOOL CRenderViewFrame::Create(const RECT& rect, CWnd* pParentWnd, BOOL bBuiltIn)
{
	m_bBuiltIn = bBuiltIn;
	return CStatic::Create(NULL, WS_CHILD | SS_CENTERIMAGE | SS_WHITEFRAME | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, rect, pParentWnd, AFX_IDW_PANE_LAST);
}

void CRenderViewFrame::OnSize(UINT nType, int cx, int cy) 
{
	CStatic::OnSize(nType, cx, cy);
    if (m_ViewWnd && IsWindow( m_ViewWnd->m_hWnd ) )
        m_ViewWnd->MoveWindow(0,0,cx,cy);
}

BOOL CRenderViewFrame::OnEraseBkgnd( CDC* pDC )
{
	UpdateWindow();
	return TRUE;
}

LRESULT CRenderViewFrame::OnGlyphChangeSize(WPARAM wparam, LPARAM lparam)
{
    int cx=LOWORD(lparam);
    int cy=HIWORD(lparam);
    m_pGlyph->UpdateSize(CSize(cx, cy));
    return 0;
}



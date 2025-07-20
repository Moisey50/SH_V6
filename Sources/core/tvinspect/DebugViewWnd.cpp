#include "stdafx.h"
#include "DebugViewWnd.h"

CDebugViewWnd::CDebugViewWnd(IGraphbuilder* pBuilder):
m_pBuilder(pBuilder),
m_pView(NULL)
{
}

CDebugViewWnd::~CDebugViewWnd()
{
	ASSERT(m_pBuilder);
	m_pBuilder->Release();
	m_pBuilder = NULL;
}

BOOL CDebugViewWnd::Create(CWnd* pParentWnd)
{
	if (!CMDIChildWnd::Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW, rectDefault, (CMDIFrameWnd*)pParentWnd))
		return FALSE;
	m_pView = Tvdb400_GetSketchViewMod();
	if (!GET_WND(m_pView)->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		m_pView->Release();
		m_pView = NULL;
		m_pBuilder = NULL;
		return FALSE;
	}

	m_pBuilder->AddRef();
	m_pView->SetBuilder(m_pBuilder);
	m_pView->ComposeGraph();

	if (::IsWindow(GetSafeHwnd()) && m_pView)
	{
		CRect rc;
		GetClientRect(rc);
		GET_WND(m_pView)->MoveWindow(rc);
	}

	return TRUE;
}

BEGIN_MESSAGE_MAP(CDebugViewWnd, CMDIChildWnd)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void CDebugViewWnd::OnDestroy()
{
	CMDIChildWnd::OnDestroy();
	if (m_pView)
		m_pView->Release();
	m_pView = NULL;
}


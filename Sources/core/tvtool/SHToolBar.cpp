#include "stdafx.h"
#include "SHToolBar.h"

BEGIN_MESSAGE_MAP(CSHToolBar, CToolBar)
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

CSHToolBar::CSHToolBar() :
m_hCursor(NULL)
{
	m_hCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
}

BOOL CSHToolBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{

	CURSORINFO cursorInfo;
	cursorInfo.cbSize = sizeof(CURSORINFO);
	if (GetCursorInfo(&cursorInfo))
	{
		if (cursorInfo.hCursor != m_hCursor)
		{
			if (!SetClassLongPtr(GetSafeHwnd(), GCLP_HCURSOR, (LONG_PTR)m_hCursor))
			{
				TRACE("SetClassLong fails with error %d\n", GetLastError());
			}
		}
	}

	return CToolBar::OnSetCursor(pWnd, nHitTest, message);
}
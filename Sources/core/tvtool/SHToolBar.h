#pragma once

class CSHToolBar : public CToolBar
{
	HCURSOR	m_hCursor;
protected:
	//{{AFX_MSG(CSHToolBar)
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CSHToolBar();
};

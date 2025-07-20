#pragma once
#include "afxwin.h"
// #include <Gadgets\tvdbase.h>
#include <Gadgets\gadbase.h>
#include <Gadgets\tview.h>
#include "GraphViewDlg.h"

class CDebugWnd :
	public CFrameWnd
{
	CWnd* m_pHostWnd;
	IGadgetsBar* m_pGadgetsBar;
	IGraphbuilder* m_pBuilder;
	CWnd* m_pClientFrame;
	CGraphViewDlg* m_pGraphView;
public:
	CDebugWnd(IGraphbuilder* pBuilder);
	~CDebugWnd(void);
public:
	virtual BOOL Create(CWnd* pHostWnd);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnRegisterDragDrop(WPARAM wParam, LPARAM lParam);
};

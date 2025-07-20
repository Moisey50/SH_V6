#pragma once
#include "afxwin.h"
#include <gadgets\shkernel.h>
#include <gadgets\tview.h>
#include "graphviewdlg.h"

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
	virtual ~CDebugWnd(void);
public:
	virtual BOOL Create(CWnd* pHostWnd);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnRegisterDragDrop(WPARAM wParam, LPARAM lParam);
protected:
    virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
public:
};

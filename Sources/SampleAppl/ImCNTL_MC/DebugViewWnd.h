#pragma once
#include "afxwin.h"

#include <Gadgets\tview.h>
#include <Gadgets\gadbase.h>
// #include <Gadgets\tvdbase.h>

class CDebugViewWnd :
	public CMDIChildWnd
{
	IGraphbuilder* m_pBuilder;
	ISketchView* m_pView;
public:
	CDebugViewWnd(IGraphbuilder* pBuilder);
	~CDebugViewWnd();
	ISketchView* GetIView() { return m_pView; };
	BOOL Create(CWnd* pParentWnd);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
};

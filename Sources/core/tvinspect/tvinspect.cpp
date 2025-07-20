// tvinspect.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <gadgets\tvinspect.h>
#include "DebugWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL StartInspect(IGraphbuilder* pBuilder, CWnd* pHostWnd)
{
	CDebugWnd* pWnd = new CDebugWnd(pBuilder);
	if (!pWnd->Create(pHostWnd))
	{
		delete pWnd;
		return FALSE;
	}
	pWnd->ShowWindow(SW_SHOW);
	return TRUE;
}

BOOL FinishInspect()
{
	return TRUE;
}

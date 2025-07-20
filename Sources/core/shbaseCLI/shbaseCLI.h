// shbaseCLI.h : main header file for the shbaseCLI DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "Interfaces.h"

// CshbaseCLIApp
// See shbaseCLI.cpp for the implementation of this class
//

class CshbaseCLIApp : public CWinApp,
    public IApplication

{
public:
	CshbaseCLIApp();

// Overrides
public:
	virtual BOOL InitInstance();
    BOOL AttachWnd(HWND hwnd);
	BOOL DetachWnd();

	DECLARE_MESSAGE_MAP()
};

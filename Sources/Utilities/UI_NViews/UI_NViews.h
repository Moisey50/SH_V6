
// UI_NViews.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CUI_NViewsApp:
// See UI_NViews.cpp for the implementation of this class
//

class CUI_NViewsApp : public CWinApp
{
public:
	CUI_NViewsApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CUI_NViewsApp theApp;

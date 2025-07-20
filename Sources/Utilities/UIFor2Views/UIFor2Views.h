
// UIFor2Views.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CUIFor2ViewsApp:
// See UIFor2Views.cpp for the implementation of this class
//

class CUIFor2ViewsApp : public CWinApp
{
public:
	CUIFor2ViewsApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CUIFor2ViewsApp theApp;

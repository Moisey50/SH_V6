// PaperOffset.h : main header file for the PaperOffset DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#include <gadgets\gadbase.h>


// CPaperOffsetApp
// See PaperOffset.cpp for the implementation of this class
//

class CPaperOffsetApp : public CWinApp
{
public:
	CPaperOffsetApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

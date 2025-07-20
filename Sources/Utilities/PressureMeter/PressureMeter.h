// PressureMeter.h : main header file for the PRESSUREMETER application
//

#if !defined(AFX_PRESSUREMETER_H__1E378E3E_7CE2_4A51_8AD8_C2943D8D948E__INCLUDED_)
#define AFX_PRESSUREMETER_H__1E378E3E_7CE2_4A51_8AD8_C2943D8D948E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CPressureMeterApp:
// See PressureMeter.cpp for the implementation of this class
//

class CPressureMeterApp : public CWinApp
{
public:
	CPressureMeterApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPressureMeterApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CPressureMeterApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRESSUREMETER_H__1E378E3E_7CE2_4A51_8AD8_C2943D8D948E__INCLUDED_)

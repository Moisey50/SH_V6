// Tuner.h : main header file for the TUNER application
//

#if !defined(AFX_TUNER_H__7F1414C6_8027_11D5_9463_F04F70C1402B__INCLUDED_)
#define AFX_TUNER_H__7F1414C6_8027_11D5_9463_F04F70C1402B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTunerApp:
// See Tuner.cpp for the implementation of this class
//

class CTunerApp : public CWinApp
{
public:
	CTunerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTunerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTunerApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TUNER_H__7F1414C6_8027_11D5_9463_F04F70C1402B__INCLUDED_)

// avt2_11.h : main header file for the AVT2_11 DLL
//

#if !defined(AFX_AVT2_11_H__5BA3E778_54E3_4525_8F79_654A0982C2F6__INCLUDED_)
#define AFX_AVT2_11_H__5BA3E778_54E3_4525_8F79_654A0982C2F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CAvtLanApp
// See avt_lan.cpp for the implementation of this class
//

class CAvtLanApp : public CWinApp
{
public:
	CAvtLanApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAvtLanApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CAvtLanApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AVT2_11_H__5BA3E778_54E3_4525_8F79_654A0982C2F6__INCLUDED_)

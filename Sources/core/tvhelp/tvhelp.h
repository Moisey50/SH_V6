// tvhelp.h : main header file for the TVHELP DLL
//

#if !defined(AFX_TVHELP_H__2FA2A0A3_58DC_4812_A789_D14A79239559__INCLUDED_)
#define AFX_TVHELP_H__2FA2A0A3_58DC_4812_A789_D14A79239559__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTvhelpApp
// See tvhelp.cpp for the implementation of this class
//

class CTvhelpApp : public CWinApp
{
public:
	CTvhelpApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTvhelpApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTvhelpApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TVHELP_H__2FA2A0A3_58DC_4812_A789_D14A79239559__INCLUDED_)

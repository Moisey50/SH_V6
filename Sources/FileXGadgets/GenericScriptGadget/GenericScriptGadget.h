// GenericScriptGadget.h : main header file for the GenericScriptGadget DLL

#if !defined(AFX_GenericScriptGadget_H__C5C8C33C_8324_4A47_B157_5565A5884D14__INCLUDED_)
#define AFX_GenericScriptGadget_H__C5C8C33C_8324_4A47_B157_5565A5884D14__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// GenericScriptGadgetApp
// See GenericScriptGadget.cpp for the implementation of this class
//
//#define TVDB400_PLUGIN_NAME	"GenericScriptGadget"

class GenericScriptGadgetApp : public CWinApp
{
public:
	GenericScriptGadgetApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GenericScriptGadgetApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(GenericScriptGadgetApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GenericScriptGadget_H__C5C8C33C_8324_4A47_B157_5565A5884D14__INCLUDED_)

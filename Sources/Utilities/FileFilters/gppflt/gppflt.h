// gppflt.h : main header file for the GPPFLT DLL
//

#if !defined(AFX_GPPFLT_H__0F633C54_047E_4DD4_9D43_196EB6133AF1__INCLUDED_)
#define AFX_GPPFLT_H__0F633C54_047E_4DD4_9D43_196EB6133AF1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include <gdiplus.h>

/////////////////////////////////////////////////////////////////////////////
// CGppfltApp
// See gppflt.cpp for the implementation of this class
//

class CGppfltApp : public CWinApp
{
    ULONG_PTR                    m_gdiplusToken;
    Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;
public:
	CGppfltApp();
    DWORD Load(LPBITMAPINFOHEADER* pic, const char* fName);
    bool  Save(LPBITMAPINFOHEADER  pic, const char* fName);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGppfltApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CGppfltApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
    virtual BOOL InitInstance();
    virtual int ExitInstance();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GPPFLT_H__0F633C54_047E_4DD4_9D43_196EB6133AF1__INCLUDED_)

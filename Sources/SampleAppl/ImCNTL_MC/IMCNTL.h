// ImCNTL.h : main header file for the IMCNTL application
//

#if !defined(AFX_IMCNTL_H__A7F453BA_3255_11D3_8D4E_000000000000__INCLUDED_)
#define AFX_IMCNTL_H__A7F453BA_3255_11D3_8D4E_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif


#include "resource.h"		// main symbols
//#include "ExposureControl.h"

/////////////////////////////////////////////////////////////////////////////
// CImCNTLApp:
// See ImCNTL.cpp for the implementation of this class
//

class CImCNTLApp : public CWinApp
{
public:
	int m_iViewExist;
	CString * m_pstrMsg;
	void LogMessageGuest(const char * msg,int iPart);
  CString BuildOperatorStatusMsg(const char * msg);
	BOOL m_bTechMode;	
	void LogMessage(const char * msg );
	void LogMessage( CString& msg );
	void LogMessage2(const char * msg );
	void LogMessage2( CString& msg );
	void LogMessage3(const char * msg );
	void LogMessage3( CString& msg );

	CImCNTLApp();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImCNTLApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CImCNTLApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

static CImCNTLApp * IApp() 
{
  return ( CImCNTLApp * )AfxGetApp() ;
}
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMCNTL_H__A7F453BA_3255_11D3_8D4E_000000000000__INCLUDED_)

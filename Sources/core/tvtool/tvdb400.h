// tvdb400.h : main header file for the TVDB400 application
//

#if !defined(AFX_TVDB400_H__6A76F3ED_C993_4AD5_8F1E_7378C3E0D3BA__INCLUDED_)
#define AFX_TVDB400_H__6A76F3ED_C993_4AD5_8F1E_7378C3E0D3BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include <fxfc\fxfc.h>
#include <userinterface\SplashWnd.h>
#include <helpers\FileX\DLLLoader.h>
/////////////////////////////////////////////////////////////////////////////
// CTvdb400App:
// See tvdb400.cpp for the implementation of this class
//

#define GADGETS_BAR_ID	TVDB_WORKSPACE_BAR_ID_FIRST
#define LOG_BAR_ID		(TVDB_WORKSPACE_BAR_ID_FIRST + 1)

class CTvdb400App : public CWinApp
{
private:
    CDLLLoader m_DLLLoader;
    bool       m_AutoStart;
    bool       m_bStartMinimized;
public:
	CTvdb400App();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTvdb400App)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL
private:
    bool ParseCommandLine(CCommandLineInfo& rCmdInfo);
// Implementation

public:
	//{{AFX_MSG(CTvdb400App)
	afx_msg void OnAppAbout();
  afx_msg void OnHelp( );
	//}}AFX_MSG
  bool bIsStartMinimized() { return m_bStartMinimized;  }
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TVDB400_H__6A76F3ED_C993_4AD5_8F1E_7378C3E0D3BA__INCLUDED_)

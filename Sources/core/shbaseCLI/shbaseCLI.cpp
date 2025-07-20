// shbaseCLI.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "shbaseCLI.h"
#include <gadgets\gadbase.h>
#include <gadgets\tview.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CshbaseCLIApp

BEGIN_MESSAGE_MAP(CshbaseCLIApp, CWinApp)
END_MESSAGE_MAP()

// CshbaseCLIApp construction

CshbaseCLIApp::CshbaseCLIApp()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CshbaseCLIApp object

CshbaseCLIApp theApp;

// CshbaseCLIApp initialization

BOOL CshbaseCLIApp::InitInstance()
{
    AFX_MODULE_STATE* afxState=AfxGetStaticModuleState( );

    AFX_MANAGE_STATE(afxState);
	CWinApp::InitInstance();
/*	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	InitApplication();

	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	pModuleState->m_bDLL = NULL;
    SetRegistryKey(_T("TheFileX\\StreamHandler"));
*/
    attachtviewDLL();
	return TRUE;
}

BOOL CshbaseCLIApp::AttachWnd(HWND hwnd)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (hwnd)
        m_pMainWnd=CWnd::FromHandlePermanent(hwnd);
	return TRUE;
}

BOOL CshbaseCLIApp::DetachWnd()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (m_pMainWnd)
    {
        m_pMainWnd->Detach();
        delete m_pMainWnd; m_pMainWnd=NULL;
    }
	return TRUE;
}

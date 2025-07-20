// PressureMeter.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "PressureMeter.h"
#include "PressureMeterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPressureMeterApp

BEGIN_MESSAGE_MAP(CPressureMeterApp, CWinApp)
	//{{AFX_MSG_MAP(CPressureMeterApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPressureMeterApp construction

CPressureMeterApp::CPressureMeterApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPressureMeterApp object

CPressureMeterApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPressureMeterApp initialization

BOOL CPressureMeterApp::InitInstance()
{
	// Standard initialization

	CPressureMeterDlg dlg;
	m_pMainWnd = &dlg;
	FXSIZE nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

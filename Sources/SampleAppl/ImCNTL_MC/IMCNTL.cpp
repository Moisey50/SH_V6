// ImCNTL.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ImCNTL.h"
#include "ImCNTLDlg.h"
#include "ImagView.h"
#include "helpers\Registry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImCNTLApp

BEGIN_MESSAGE_MAP(CImCNTLApp, CWinApp)
	//{{AFX_MSG_MAP(CImCNTLApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	//ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImCNTLApp construction

CImCNTLApp::CImCNTLApp()
{
	m_iViewExist = 1;
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CImCNTLApp object

CImCNTLApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CImCNTLApp initialization

BOOL CImCNTLApp::InitInstance()
{
	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

//#ifdef _AFXDLL
//	Enable3dControls();			// Call this when using MFC in a shared DLL
//#else
//	Enable3dControlsStatic();	// Call this when linking to MFC statically
//#endif

	// Parse the command line to see if launched as OLE server
	if (RunEmbedded() || RunAutomated())
	{
		// Register all OLE server (factories) as running.  This enables the
		//  OLE libraries to create objects from other applications.
		COleTemplateServer::RegisterAll();
	}
	else
	{
		// When a server application is launched stand-alone, it is a good idea
		//  to update the system registry in case it has been damaged.
		COleObjectFactory::UpdateRegistryAll();
	}

  CRegistry Reg( "File Company\\OpticJig" ) ;
  m_bTechMode = Reg.GetRegiInt 
    ( "Positions" , "UserMode mode: 0 - operator, 1 - technician" , 0) ;  
    
  Reg.InitKey( "File Company\\ImCNTL" ) ;
  

	CImCNTLDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
  
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

void CImCNTLApp::LogMessageGuest(const char *msg,int iPart)
{
  extern HWND hw;
  ::PostMessage (hw,WM_MY_MSG,(WPARAM)msg,(LPARAM)iPart);	
}

void CImCNTLApp::LogMessage(const char *msg)
{
  ( ( CImCNTLDlg * )m_pMainWnd )->m_ViewFrame->LogMessage( msg ) ;
}

void CImCNTLApp::LogMessage( CString& msg )
{
  ( ( CImCNTLDlg * )m_pMainWnd )->m_ViewFrame->LogMessage( ( LPCTSTR )msg ) ;
}

void CImCNTLApp::LogMessage2(const char *msg)
{
  ( ( CImCNTLDlg * )m_pMainWnd )->m_ViewFrame->LogMessage2( msg ) ;
}

void CImCNTLApp::LogMessage2( CString& msg )
{
  ( ( CImCNTLDlg * )m_pMainWnd )->m_ViewFrame->LogMessage2( ( LPCTSTR )msg ) ;
}

void CImCNTLApp::LogMessage3(const char *msg)
{
  ( ( CImCNTLDlg * )m_pMainWnd )->m_ViewFrame->LogMessage3( msg ) ;
}

void CImCNTLApp::LogMessage3( CString& msg )
{
  ( ( CImCNTLDlg * )m_pMainWnd )->m_ViewFrame->LogMessage3( ( LPCTSTR )msg ) ;
}

CString CImCNTLApp::BuildOperatorStatusMsg(const char * msg)
{
  CString sNew = "";
  CString sOld ( msg );
  BOOL bAsyncMode = TRUE;
  if ( sOld.Find ("M2") >= 0)
  {
    sNew = "SYNC";
    if ( sOld.Find ("S1")>=0 )
      sNew += " SOS";
    else
      sNew += "    ";
    if ( sOld.Find ("V1")>=0 )
      sNew += " VSYNC";
    else
      sNew += "     ";
    int iExHighPos = sOld.Find("P");
    int iExLowPos = sOld.Find("E");
    if ( iExHighPos > 0 && iExLowPos > 0 )
    {
      CString sHigh = sOld.Mid(iExHighPos + 1) ;
      CString sLow = sOld.Mid(iExLowPos + 1);
      int iHigh, iLow;
      iHigh = atoi((LPCTSTR)sHigh);
      iLow = atoi((LPCTSTR)sLow);
      if ( iHigh && iLow )
      {
        int iExp = iHigh - iLow;
        if (iExp > 0)
        {
          CString sExp;
          sExp.Format(" EXP=%d scans", iExp);
          sNew += sExp;
        }
      }
    }
  }
  else if ( sOld.Find ("M") >= 0 )
  {
    sNew = "ASYNC";
    
    int iExHighPos = sOld.Find("K");
    int iExLowPos = sOld.Find("L");
    if ( iExHighPos > 0 && iExLowPos > 0 )
    {
      CString sHigh = sOld.Mid(iExHighPos + 1);
      CString sLow = sOld.Mid(iExLowPos + 1);
      int iHigh, iLow;
      iHigh = atoi((LPCTSTR)sHigh);
      iLow = atoi((LPCTSTR)sLow);
      if ( iHigh && iLow )
      {
        int iExp = iHigh - iLow;
        if (iExp > 0)
        {
          CString sExp;
          sExp.Format(" EXP=%d ms", iExp);
          sNew += sExp;
        }
      }
    }
  }

  return sNew;
}

BOOL CImCNTLApp::PreTranslateMessage(MSG* pMsg) 
{
	//Disables Enter key function - Elad's request
	switch ( pMsg->message)
  {
    case WM_KEYDOWN:
      {
        switch ( pMsg->wParam )
        {
        case VK_RETURN:
          return 1;
        }
      }
  }
	
	return CWinApp::PreTranslateMessage(pMsg);
}




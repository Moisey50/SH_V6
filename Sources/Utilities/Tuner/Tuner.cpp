// Tuner.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Tuner.h"
#include "TunerDlg.h"
#include <helpers\profileininifile.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// void WriteLogV( const char *lpszFormat , va_list argList )
// {
//   CString tmpS;
//   tmpS.FormatV( lpszFormat , argList );
//   TRACE( tmpS );
// }
// 
// void WriteLog( const char * lpszFormat , ... )
// {
// #ifdef _DEBUG
//   va_list argList;
//   va_start( argList , lpszFormat );
//   WriteLogV( lpszFormat , argList );
//   va_end( argList );
// #endif
// }


/////////////////////////////////////////////////////////////////////////////
// CTunerApp

BEGIN_MESSAGE_MAP( CTunerApp , CWinApp )
  //{{AFX_MSG_MAP(CTunerApp)
  //}}AFX_MSG
  ON_COMMAND( ID_HELP , CWinApp::OnHelp )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTunerApp construction

CTunerApp::CTunerApp()
{}

/////////////////////////////////////////////////////////////////////////////
// The one and only CTunerApp object

CTunerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CTunerApp initialization

BOOL CTunerApp::InitInstance()
{
  HBSetRegistryKey( _T( "HomeBuilt" ) );
  CTunerDlg dlg;
  m_pMainWnd = &dlg;
  CCommandLineInfo cmdInfo;
  ParseCommandLine( cmdInfo );

  if ( cmdInfo.m_strFileName.GetLength() != 0 )
  {
    dlg.m_CmdLine = cmdInfo.m_strFileName;
  }
  int nResponse = (int) dlg.DoModal();
  if ( nResponse == IDOK )
  {
  }
  else if ( nResponse == IDCANCEL )
  {
  }
  return FALSE;
}

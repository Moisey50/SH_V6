#ifndef _TVDB400_INSPECTOR_INCLUDED
#define _TVDB400_INSPECTOR_INCLUDED
#include "tview.h"

#ifdef _DEBUG
    #define TVINSPECT_DLL_NAME  "tvinspectd.dll"
    #define TVINSPECT_LIB_NAME  "tvinspectd.lib"
#else
 #define TVINSPECT_DLL_NAME  "tvinspect.dll"
 #define TVINSPECT_LIB_NAME  "tvinspect.lib"
#endif

#ifndef TVINSPECT_DLL
    #define AFX_EXT_TVINSPECT __declspec(dllimport)
    #pragma comment(lib, TVINSPECT_LIB_NAME)
#else
    #define AFX_EXT_TVINSPECT __declspec(dllexport)
#endif


AFX_EXT_TVINSPECT BOOL StartInspect(IGraphbuilder* pBuilder, CWnd* pHostWnd);
AFX_EXT_TVINSPECT BOOL FinishInspect();


#endif
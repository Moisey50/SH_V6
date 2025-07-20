#ifndef FXFC_DLL_INCLUDE
#define FXFC_DLL_INCLUDE
// fxfc.h : header file
//
#pragma once

#ifdef _DEBUG
  #define FXFC_DLL_NAME  "fxfcd.dll"
  #define FXFC_LIB_NAME  "fxfcd.lib"
  #define FX_CONFIG "Debug"
#else
  #define FXFC_DLL_NAME  "fxfc.dll"
  #define FXFC_LIB_NAME  "fxfc.lib"
  #define FX_CONFIG "Release"
#endif

#ifndef FXFC_DLL
    #define FXFC_EXPORT __declspec(dllimport)
    #pragma comment(lib, FXFC_LIB_NAME)
#else
    #define FXFC_EXPORT __declspec(dllexport)
#endif

#ifdef _WIN64
#define FXSIZE __int64
#define _FX_PLATF "x64"
#else
#define FXSIZE int
#define _FX_PLATF "x86"
#endif

#ifndef USE_FXFC_Strings
    #define USE_FXFC_Strings
#endif

#ifdef NO_ATL_DEBUG
#ifdef _DEBUG
#ifdef ATLTRACE 
#undef ATLTRACE
#undef ATLTRACE2

#define ATLTRACE CustomTrace
#define ATLTRACE2 ATLTRACE
#endif // ATLTRACE
#endif // _DEBUG

inline void CustomTrace( const TCHAR* format , ... )
{
  const int TraceBufferSize = 1024;
  TCHAR buffer[ TraceBufferSize ];

  va_list argptr;
  va_start( argptr , format );
  _vstprintf_s( buffer , format , argptr );
  va_end( argptr );

  ::OutputDebugString( buffer );
}

inline void CustomTrace( int dwCategory , int line , const TCHAR* format , ... )
{
  va_list argptr; va_start( argptr , format );
  CustomTrace( format , argptr );
  va_end( argptr );
}
#endif // NO_ATL_DEBUG

#include "Winnls.h"

inline DWORD Win32FromHResult( HRESULT hr )
{
  if ( hr == S_OK )
    return ERROR_SUCCESS;
  if ( (hr & 0xFFFF0000) == MAKE_HRESULT( SEVERITY_ERROR , FACILITY_WIN32 , 0 ) )
    return HRESULT_CODE( hr );

  return ERROR_CAN_NOT_COMPLETE;
}



#include <fxfc\fxstruct.h>
#include <fxfc\fxstrings.h>
#include <fxfc\fxext.h>
#include <fxfc\fxlogsystem.h>
#include <fxfc\fxarrays.h>
#include <fxfc\fxfilefind.h>
#include <fxfc\fxtime.h>
#include <fxfc\fxfile.h>
#include <fxfc\fxexception.h>
#include <fxfc\fxtimer.h>
#include <fxfc\fxparser.h>
#include <fxfc\lockobject.h>
#include <fxfc\propertykit.h>
#include <fxfc\lasterr2mes.h>
#include <fxfc\lockptrarray.h>
#include <fxfc\instancecntr.h>
#include <fxfc\staticqueue.h>
#include <fxfc\fxworker.h>
#include <fxfc\fxgraphmsgqueue.h>
#include <fxfc\fxplex.h>
#include <fxfc\fxmapptrtoptr.h>


#if  !defined(SHBASE_CLI) && !defined(NO_INTF_SUP)
using namespace std;
#include <complex>
#include <vector>
// #include "math\Intf_sup.h"
// #include <fxfc\FXRegistry.
#endif

#endif //FXFC_DLL_INCLUDE
#include <fxfc\fxfc.h>
#if !defined(SHBASE__INCLUDED_)
#define SHBASE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include <video\shvideo.h>

#ifdef _DEBUG
    #define SHDBASE_DLL_NAME "shbased.dll"
    #define SHDBASE_LIB_NAME "shbased.lib"
#else
 #define SHDBASE_DLL_NAME "shbase.dll"
 #define SHDBASE_LIB_NAME "shbase.lib"
#endif

#ifndef SHBASE_DLL
    #define FX_EXT_SHBASE __declspec(dllimport)
    #pragma comment(lib, SHDBASE_LIB_NAME)
#else
    #define FX_EXT_SHBASE __declspec(dllexport)
#endif

void FX_EXT_SHBASE attachshbaseDLL();

#include <shbase\viewsectionparser.h>
#include <shbase\gdibank.h>
#include <shbase\textview.h>
#include <shbase\containerview.h>
#include <shbase\termwindow.h>
#include <shbase\resizebledialog.h>
#include <shbase\dibrender.h>
#include <shbase\dxrender.h>

#endif //#if !defined(SHBASE__INCLUDED_)

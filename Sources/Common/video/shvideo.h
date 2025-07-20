#if !defined(SHVIDEO__INCLUDED_)
#define SHVIDEO__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _DEBUG
    #define SHVIDEO_DLL_NAME "shvideod.dll"
    #define SHVIDEO_LIB_NAME "shvideod.lib"
#else
 #define SHVIDEO_DLL_NAME "shvideo.dll"
 #define SHVIDEO_LIB_NAME "shvideo.lib"
#endif

#ifndef FX_EXT_SHVIDEO
    #ifndef SHVIDEO_DLL
        #define FX_EXT_SHVIDEO __declspec(dllimport)
        #pragma comment(lib, SHVIDEO_LIB_NAME)
    #else
        #define FX_EXT_SHVIDEO __declspec(dllexport)
    #endif
#endif 

void FX_EXT_SHVIDEO attachshvideoDLL();

#include <video\TVFrame.h>
#include <video\videoformats.h>
#include <video\stdcodec.h>
#include <video\yuv9.h>
#include <video\dibview.h>
#include <video\dibviewbase.h>
#include <video\dxview.h>

#endif
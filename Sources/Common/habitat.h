#ifndef HABITAT_INC
#define HABITAT_INC

#include <buildnmb.h>

#define NO_CATCH_EXCEPTION_IN_DEBUG
#define DEFINE_SHPLUGIN_EXT _declspec(dllexport) LPCTSTR __stdcall GetSWVersion() { return STREAMHANDLER_VERSION; }

#if defined(UNICODE)
#pragma message ("Compiling UNICODE")
#else
#pragma message ("Compiling SBCS")
#endif

#define _CRT_RAND_S /// we use rand_s
#include <stdlib.h>

#define COMPANY_NAME 				_T("TheFileX")
#define COMPANY_NAME_SEC    _T("FileX")
#define PROFILE_DEFAULKEYROOT 		_T("SOFTWARE\\FileX\\COMMON\\")

#define APP_NAME_MAXLENGTH 256
#define GADGET_NAME_MAXLENGTH 256

#define STRING2(x) #x
#define STRING(x) STRING2(x)

/// Solution wide messages

#define UM_NEXT_FIELD		    (WM_USER+1)
#define UM_SHOWSAMPLE           (WM_USER+2)
#define UM_FLTWND_CHANGED       (WM_USER+3)
#define UM_CHECKSTATECHANGE 	(WM_USER+4)
#define UM_GLIPH_CHANGESIZE     (WM_USER+5)
#define UM_FGNOTIFY	            (WM_USER+6)


//#pragma message("File: habitat.h included in a project")
#include <string.h>
// COMMON ENVIRONMENT SETTINGS AND DEFINITIONS
#if (_MSC_VER<1300)
	#include <CHAR.H>
	#define FSIZE DWORD
	#define _WIN32_WINNT 0x0400
	#define WINVER 0x0400
    __forceinline int strcpy_s(char strDestination[],const char *strSource )
    {
        strcpy(strDestination,strSource);
        return 0;
    }
    __forceinline int strcpy_s(char strDestination[],size_t sz, const char *strSource )
    {
        strcpy(strDestination,strSource);
        return 0;
    }
    __forceinline int strcat_s(char strDestination[],const char *strSource)
    {
        strcat(strDestination,strSource);
        return 0;
    }
    __forceinline int memcpy_s(void *dest, size_t sizeInBytes,const void *src,size_t count)
    {
        if (sizeInBytes<count) return -1;
        memcpy(dest,src,count);
        return 0;
    }
    __forceinline char *strtok_s(char *strToken, const char *strDelimit,char **context)
    {
        *context=(char*)-1;
        return strtok(strToken, strDelimit);
    }
	__forceinline int _tcscpy_s(char *strDestination,size_t numberOfElements,const char *strSource )
	{
		_tcscpy(strDestination, strSource);
		return 0;
	}
    __forceinline int _tcscat_s(char strDestination[],int iBufLenElements, const char *strSource)
    {
        _tcscat(strDestination,strSource);
        return 0;
    }
    #define sprintf_s sprintf
    #define sscanf_s sscanf
    #define _strupr_s _strupr
//	#pragma message("+++ COMMON ENVIRONMENT SETTINGS AND DEFINITIONS ARE SET")
#else
	#define FSIZE ULONGLONG
	#pragma warning(disable :4996)
    #pragma warning(disable :4995)
	#pragma message("+++ Warrning! Defined _CRT_SECURE_NO_WARNINGS")
  #ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0600
  #endif
    #ifndef WINVER
	    #define WINVER 0x0502
    #else
        #if WINVER < 0x0502
            #pragma message("+++ Warrning! Increase WINVER up to 0x0502, was " STRING(WINVER) )
            #undef WINVER 
            #define WINVER 0x0502
        #endif
    #endif
#endif

#endif
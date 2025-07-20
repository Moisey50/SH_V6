#ifndef HABITAT_INC
#define HABITAT_INC

#include <buildnmb.h>

#if defined(UNICODE)
#pragma message ("Compiling UNICODE")
#else
#pragma message ("Compiling SBCS")
#endif


#define COMPANY_NAME 				_T("FileX")
#define PROFILE_DEFAULKEYROOT 		_T("SOFTWARE\\FileX\\COMMON\\")

#define APP_NAME_MAXLENGTH 256
#define GADGET_NAME_MAXLENGTH 256

#define STRING2(x) #x
#define STRING(x) STRING2(x)

//#pragma message("File: habitat.h included in a project")
#include <string.h>

// COMMON ENVIRONMENT SETTINGS AND DEFINITIONS
#if (_MSC_VER<1300)
	#include <TCHAR.H>
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
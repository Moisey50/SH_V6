#ifndef FXMODULES_INCLUDE
#define FXMODULES_INCLUDE
// fxmodules.h : header file
//
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable : 4995)
#include <intrin.h>
#pragma warning( default : 4995)

#pragma intrinsic(_ReturnAddress)

//*nix synonym of _ReturnAddress() is __builtin_return_address(0);

#define GetParentModuleHandle(a) GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,(LPCTSTR)_ReturnAddress(),a);
__forceinline HMODULE GetCurrentModule()
{ 
    HMODULE hModule = NULL;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,(LPCTSTR)GetCurrentModule,&hModule);
    return hModule;
}
#endif //#ifndef FXMODULES_INCLUDE
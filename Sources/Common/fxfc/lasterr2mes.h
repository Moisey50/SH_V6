#ifndef LASTERR2MES_INC
#define LASTERR2MES_INC

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

FXString FXFC_EXPORT FxLastErr2Mes(LPCTSTR Prefix=_T(""),LPCTSTR Suffix=_T(""));
FXString FXFC_EXPORT FxLastErr2Mes(DWORD LastErr, LPCTSTR Prefix=_T(""), LPCTSTR Suffix=_T(""));
FXString FXFC_EXPORT FxAviFileError2Mes(UINT Error);
FXString FXFC_EXPORT ProcessHRESULT( HRESULT hr , LPCTSTR Prefix = _T( "" ) , LPCTSTR Suffix = _T( "" ) );
#endif //#define LASTERR2MES_INC
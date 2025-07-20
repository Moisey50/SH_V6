// GetRCVersion.h: Interfaces for class CGetRCVersion.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GLOBALFUNCTIONS_H__44AC5D38_F5B0_4E8E_B50E_4138D0E91536__INCLUDED_)
#define AFX_GLOBALFUNCTIONS_H__44AC5D38_F5B0_4E8E_B50E_4138D0E91536__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <fxfc\fxfc.h>

class CGetRCVersion : public CObject  
{
public:
	CGetRCVersion();
	virtual ~CGetRCVersion();

public:
	static FXString GetFileVersionX();
	static FXString GetProductVersionX();
	static FXString GetVersionInfo(HMODULE hLib, FXString csEntry);
	static FXString FormatVersion(FXString& cs);

private:
	static FXString m_csFileVersion;
	static FXString m_csProductVersion;

};

#endif // !defined(AFX_GLOBALFUNCTIONS_H__44AC5D38_F5B0_4E8E_B50E_4138D0E91536__INCLUDED_)

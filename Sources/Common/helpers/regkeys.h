// RegKeys.h: interface for the CRegKeys class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGKEYS_H__6499D8A5_81B7_11D5_9462_000001360793__INCLUDED_)
#define AFX_REGKEYS_H__6499D8A5_81B7_11D5_9462_000001360793__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CRegKeys  
{
private:
    HKEY  m_RootKey;
    DWORD m_Idx;
public:
	CRegKeys(HKEY hKey, LPCTSTR lpSubKey,REGSAM samDesired=KEY_READ|KEY_SET_VALUE);
	virtual ~CRegKeys();
	bool GetNextKey(CString& data);
	BOOL GetNextKeyB(CString& name);
    BOOL DeleteKey(CString& name);
};


#endif // !defined(AFX_REGKEYS_H__6499D8A5_81B7_11D5_9462_000001360793__INCLUDED_)

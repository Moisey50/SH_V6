#ifndef REGISTRY_INC
#define REGISTRY_INC


#ifdef PROFILE_INI_FILES
#include <Futils.h>
#pragma message("+++ Profile in ini files build")

#define CALIBRATIONAPP_PATH	_TVT("FSolver")
#define CALIBRATION_PATH	_TVT("Cameras calibration")

inline LPBYTE DecodeRegistryString(const char*buffer,LPDWORD cbData)
{
    int nLen=strlen(buffer);
    if (nLen==0) return NULL;

	ASSERT(nLen%2 == 0);

	*cbData = nLen/2;
	LPBYTE pData = (LPBYTE)malloc(*cbData); 
	for (int i=0;i<nLen;i+=2)
	{
		(pData)[i/2] = (BYTE)
			(((buffer[i+1] - 'A') << 4) + (buffer[i] - 'A'));
	}
    return pData;
}

inline void HBSetRegistryKey(TVLPCSTR lpszRegistryKey )
{
    CString ProfileFileName;
    ProfileFileName.Format("%s%s.ini",(TVLPCSTR)GetAppStartDir(),AfxGetAppName());

    free((void*)AfxGetApp()->m_pszProfileName);
    AfxGetApp()->m_pszProfileName=_tcsdup(_TVT(ProfileFileName));
}

inline void HBSetRegistryKeyApp(TVLPCSTR lpszRegistryKey,TVLPCSTR appName=NULL)
{
    CString ProfileFileName;
    CString name;
    if (!appName)
    {
        GetModuleFileName(NULL,name.GetBufferSetLength(_MAX_PATH),_MAX_PATH);
        name=GetFileName(name);
        name.MakeLower( );
        int pos=name.Find(".exe");
        if (pos>0)
        {
            name.Delete(pos,4);
        }
    }
    else
        name=appName;
    ProfileFileName.Format("%s%s.ini",(TVLPCSTR)GetAppStartDir(),(TVLPCSTR)name);

    free((void*)AfxGetApp()->m_pszProfileName);
    AfxGetApp()->m_pszProfileName=_tcsdup(_TVT(ProfileFileName));
}

inline CString GetLMRegistryString(const char* MainKey, const char* SubKey, const char* Defaults)
{
    CString ProfileFileName;
    char buffer[_MAX_PATH+1];
    ProfileFileName.Format("%s%s.ini",(TVLPCSTR)GetAppStartDir(),MainKey);
    if (GetPrivateProfileString("common",SubKey,Defaults,buffer,_MAX_PATH,ProfileFileName))
    {
        return CString(buffer);
    }
    return CString(Defaults);
}

inline int GetLMRegistryInt(const char* MainKey, const char* SubKey, int Defaults)
{
    CString ProfileFileName;
    ProfileFileName.Format("%s%s.ini",(TVLPCSTR)GetAppStartDir(),MainKey);
    return GetPrivateProfileInt("common",SubKey,Defaults,ProfileFileName);
}

inline LPBYTE GetRegistryBinary(const char* MainKey, const char* SubKey, const char* ValueKey, LPDWORD cbData)
{
    CString ProfileFileName;
    char buffer[_MAX_REGVALUELEN+1];
    ProfileFileName.Format("%s%s.ini",(TVLPCSTR)GetAppStartDir(),MainKey);

    if (GetPrivateProfileString(SubKey,ValueKey,"",buffer,_MAX_REGVALUELEN,ProfileFileName))
    {
        return DecodeRegistryString(buffer,cbData);
/*        int nLen=strlen(buffer);
        if (nLen==0) return NULL;

		ASSERT(nLen%2 == 0);

		*cbData = nLen/2;
		LPBYTE pData = (LPBYTE)malloc(*cbData); 
		for (int i=0;i<nLen;i+=2)
		{
			(pData)[i/2] = (BYTE)
				(((buffer[i+1] - 'A') << 4) + (buffer[i] - 'A'));
		}
        return pData; */
    }
    return NULL;
}

#else

#pragma message("!!! Profile in registry build")

#define CALIBRATIONAPP_PATH	_TVT("FSolver\\")
#define CALIBRATION_PATH	_TVT("Cameras calibration\\")

#define HBSetRegistryKey SetRegistryKey

inline CString GetLMRegistryString(const char* MainKey, const char* SubKey, const char* Defaults)
{
    CString retVal;
    HKEY  HKey;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,MainKey,&HKey)!=ERROR_SUCCESS)
    {
        return (Defaults);
    }

    DWORD dwSize=_MAX_PATH;
    DWORD dwType=REG_SZ;
    if (RegQueryValueEx(HKey,SubKey,NULL,&dwType,(LPBYTE)retVal.GetBuffer(dwSize),&dwSize)!=ERROR_SUCCESS)
	{
		retVal.ReleaseBuffer();
		RegCloseKey(HKey);
		return (Defaults);
	}
    retVal.ReleaseBuffer();
	RegCloseKey(HKey);
    return(retVal);
}

inline int GetLMRegistryInt(const char* MainKey, const char* SubKey, int Defaults)
{
    int retVal=0;
    HKEY  HKey;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,MainKey,&HKey)!=ERROR_SUCCESS)
    {
        return (Defaults);
    }

    DWORD dwSize=sizeof(int);
    DWORD dwType=REG_DWORD;
    if (RegQueryValueEx(HKey,SubKey,NULL,&dwType,(LPBYTE)&retVal,&dwSize)!=ERROR_SUCCESS)
	{
		RegCloseKey(HKey);
		return (Defaults);
	}
	RegCloseKey(HKey);
    return(retVal);
}

inline void  WriteLMRegistryString(const char* MainKey, const char* SubKey, const char* NewValue)
{
    HKEY  HKey;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,MainKey,&HKey)!=ERROR_SUCCESS)
    {
        if (RegCreateKey(HKEY_LOCAL_MACHINE,MainKey,&HKey)!=ERROR_SUCCESS)
        {
            return;
        }

    }
    RegSetValueEx(HKey,SubKey,0,REG_SZ,(LPBYTE)NewValue,(DWORD)strlen(NewValue)+1);
    RegCloseKey(HKey);
}

inline CString GetCURegistryString(const char* MainKey, const char* SubKey, const char* Defaults)
{
    CString retVal;
    HKEY  HKey;

    if (RegOpenKey(HKEY_CURRENT_USER,MainKey,&HKey)!=ERROR_SUCCESS)
    {
        return (Defaults);
    }

    DWORD dwSize=_MAX_PATH;
    DWORD dwType=REG_SZ;
    if (RegQueryValueEx(HKey,SubKey,NULL,&dwType,(LPBYTE)retVal.GetBuffer(dwSize),&dwSize)!=ERROR_SUCCESS)
	{
		retVal.ReleaseBuffer();
		RegCloseKey(HKey);
		return (Defaults);
	}
    retVal.ReleaseBuffer();
	RegCloseKey(HKey);
    return(retVal);
}

inline void  WriteCURegistryString(const char* MainKey, const char* SubKey, const char* NewValue)
{
    HKEY  HKey;

    if (RegOpenKey(HKEY_CURRENT_USER,MainKey,&HKey)!=ERROR_SUCCESS)
    {
        if (RegCreateKey(HKEY_CURRENT_USER,MainKey,&HKey)!=ERROR_SUCCESS)
        {
            return;
        }

    }
    RegSetValueEx(HKey,SubKey,0,REG_SZ,(LPBYTE)NewValue,(DWORD)strlen(NewValue)+1);
    RegCloseKey(HKey);
}

inline LPBYTE GetRegistryBinary(const char* MainKey, const char* SubKey, const char* ValueKey, LPDWORD cbData)
{
	HKEY hKey;
    CString key=CString(MainKey)+SubKey;
	if (RegOpenKey(HKEY_CURRENT_USER, key, &hKey) == ERROR_SUCCESS)
	{
		DWORD cbData = 0;
		DWORD type = REG_BINARY;
		if (RegQueryValueEx(hKey, ValueKey, NULL, &type, NULL, &cbData) == ERROR_SUCCESS)
		{
			LPBYTE lpData = (LPBYTE)malloc(cbData);
			if (RegQueryValueEx(hKey, ValueKey, NULL, &type, lpData, &cbData) == ERROR_SUCCESS)
			{
                RegCloseKey(hKey);
                return lpData;
			}
			free(lpData);
		}
		RegCloseKey(hKey);
	}
    return NULL;
}

#endif

/// Classes root will be written in a registry in any case

inline void  WriteCRRegistryString(const char* MainKey, const char* SubKey, const char* NewValue)
{
    HKEY  HKey;

    if (RegOpenKey(HKEY_CLASSES_ROOT,MainKey,&HKey)!=ERROR_SUCCESS)
    {
        if (RegCreateKey(HKEY_CLASSES_ROOT,MainKey,&HKey)!=ERROR_SUCCESS)
        {
            return;
        }

    }
    RegSetValueEx(HKey,SubKey,0,REG_SZ,(LPBYTE)NewValue,(NewValue)?(DWORD)strlen(NewValue)+1:0);
    RegCloseKey(HKey);
}

inline bool  RegistryKeyDelete(const char* MainKey, const char* SubKey)
{
    HKEY  HKey;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,MainKey,&HKey)==ERROR_SUCCESS)
    {
        RegDeleteKey(HKey,SubKey);
        RegCloseKey(HKey);
        return true;
    }
    return false;
}


#endif
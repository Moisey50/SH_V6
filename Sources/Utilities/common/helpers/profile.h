#ifndef PROFILE_INC
#define PROFILE_INC

#pragma warning(disable:4996)

inline bool GetLMRegistryString(const char* MainKey, const char* SubKey, const char* Defaults, char *buffer, DWORD& buffersize )
{
    HKEY  HKey;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,MainKey,&HKey)!=ERROR_SUCCESS)
    {
        strcpy(buffer, Defaults);
        return false;
    }

    DWORD dwType=REG_SZ;
    if (RegQueryValueEx(HKey,SubKey,NULL,&dwType,(LPBYTE)buffer,&buffersize)!=ERROR_SUCCESS)
	{
        strcpy(buffer, Defaults);
		RegCloseKey(HKey);
		return false;
	}
	RegCloseKey(HKey);
    return true;
}

inline bool  WriteLMRegistryString(const char* MainKey, const char* SubKey, const char* NewValue)
{
    HKEY  HKey;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,MainKey,&HKey)!=ERROR_SUCCESS)
    {
        if (RegCreateKey(HKEY_LOCAL_MACHINE,MainKey,&HKey)!=ERROR_SUCCESS)
        {
            return false;
        }

    }
    if (strlen(SubKey)!=0)
	{
        if (RegSetValueEx(HKey,SubKey,0,REG_SZ,(LPBYTE)NewValue,(DWORD)strlen(NewValue)+1)!=ERROR_SUCCESS)
			return false;
	}
    RegCloseKey(HKey);
	return true;
}

inline bool  RegistryValueDelete(const char* MainKey, const char* SubKey)
{
    HKEY  HKey;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,MainKey,&HKey)==ERROR_SUCCESS)
    {
        RegDeleteValue(HKey,SubKey);
        RegCloseKey(HKey);
        return true;
    }
    return false;
}

inline bool  RegistryKeyDelete(const char* MainKey, const char* SubKey)
{
    HKEY  HKey;
    bool  res=true;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,MainKey,&HKey)==ERROR_SUCCESS)
    {
        res=(RegDeleteKey(HKey,SubKey)==ERROR_SUCCESS);
        RegCloseKey(HKey);
        return res;
    }
    return false;
}

#pragma warning(default:4996)

#endif

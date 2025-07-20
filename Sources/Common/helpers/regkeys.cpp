//  $File : RegKeys.cpp: implementation of the CRegKeys class.
//  (C) Copyright The FileX Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)

#include "stdafx.h"
#include "RegKeys.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRegKeys::CRegKeys(HKEY hKey, LPCTSTR lpSubKey,REGSAM samDesired)
{
    m_RootKey=NULL;
    if (RegOpenKeyEx(hKey,lpSubKey,0,samDesired,&m_RootKey)!=ERROR_SUCCESS)
    {
        TRACE("!!! Warrning! No image input filters installed!\n");
    }
    m_Idx=0;
}

CRegKeys::~CRegKeys()
{
    RegCloseKey(m_RootKey);
}


bool CRegKeys::GetNextKey(CString &data)
{
#define BUFFER_SIZE 2048
    char buffername[BUFFER_SIZE]; DWORD sizebn=BUFFER_SIZE;
    char bufferval[BUFFER_SIZE];  DWORD sizebv=BUFFER_SIZE;
    DWORD type;
    DWORD res=RegEnumValue(m_RootKey,m_Idx,buffername,&sizebn,NULL,&type,(unsigned char*)bufferval, &sizebv);
    m_Idx++;
    data=bufferval;
    return (res==ERROR_SUCCESS);
}

BOOL CRegKeys::GetNextKeyB(CString& name)
{
#define BUFFER_SIZE 2048
    char buffername[BUFFER_SIZE]; DWORD sizebn=BUFFER_SIZE;
    char bufferval[BUFFER_SIZE];  DWORD sizebv=BUFFER_SIZE;
    DWORD type;
    DWORD res=::RegEnumValue(m_RootKey,m_Idx,buffername,&sizebn,NULL,&type,(unsigned char*)bufferval, &sizebv);
    m_Idx++;
    name=buffername;
    return (res==ERROR_SUCCESS);
}

BOOL CRegKeys::DeleteKey(CString& name)
{
    DWORD res=::RegDeleteValue(m_RootKey,name);
    return (res==ERROR_SUCCESS);
}

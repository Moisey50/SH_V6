//  $File : FileList.cpp: implementation of the CFileList class.
//  (C) Copyright The FileX Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)

#include "stdafx.h"
#include "FileList.h"
#include <io.h>
#include <files\imgfiles.h>

#ifndef FXFC_EXPORT
FXString    FxExtractPath(const FXString& fName); 
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileList::CFileList()
{
    m_Position=0;
}

CFileList::~CFileList()
{
    RemoveAll();
}

BOOL CFileList::Dir(LPCTSTR fpath, LPCTSTR masks)
{
    FXString filespec = fpath;
    FXString path =::FxExtractPath(filespec)+"\\";
    if (filespec==path)
        filespec+="*.*";
    INT_PTR hFileFirst;
    _finddata_t FDS;
    RemoveAll();
    m_Position=0;
    int mskcnt=0;
    _get_mask_item_cnt(masks, mskcnt);

	if ((hFileFirst=_findfirst(filespec, &FDS))!=-1)
	{
		if (_cmp_extentions(FDS.name,masks, mskcnt)) Add(FXString(FDS.name));
		while (_findnext(hFileFirst, &FDS)!=-1)
        {
            if (_cmp_extentions(FDS.name,masks, mskcnt)) Add(FXString(FDS.name));
        }

	}
    m_Path=path;
    return(hFileFirst != 0);
}

BOOL CFileList::SetPosition(const FXString& fName)
{
    DWORD i=0;
    FXString sfname=::FxExtractPath(fName);
    while (i<GetSize())
    {
        if (sfname.CompareNoCase(GetAt(i))==0)
        {
            m_Position=i;
            return(TRUE);
        }
        i++;
    }
    return(FALSE);
}

FXString CFileList::Get()
{
    FXString retVal="";
    if (m_Position<GetSize())
    {
        retVal=m_Path+GetAt(m_Position);
    }
    return(retVal);
}

FXString CFileList::GetPathName()
{
    return m_Path+Get();
}

void CFileList::Get(FXString &fName)
{
    fName="";
    if (m_Position<GetSize())
    {
        fName=m_Path+GetAt(m_Position);
    }
}

void CFileList::GetPathName(FXString &fName)
{
    fName="";
    if (m_Position<GetSize())
    {
        fName=m_Path+GetAt(m_Position);
    }
}

void CFileList::Delete()
{
   if (GetSize()==0) return;
   RemoveAt(m_Position);
   if (m_Position>=GetSize()) 
   {
       if (GetSize()!=0)
           m_Position=GetSize()-1;
        else
           m_Position=0;
   }
}

BOOL CFileList::Next()
{
    m_Position++;
    if (m_Position<GetSize()) return(TRUE);
    m_Position--;
    return(FALSE);
}

BOOL CFileList::Previous()
{
    if (m_Position==0) return(FALSE);
    m_Position--;
    return(TRUE);
}

void CFileList::SetFirst()
{
    m_Position=0;
}

void CFileList::SetLast()
{
    if (GetSize())
        m_Position=GetSize()-1;
}




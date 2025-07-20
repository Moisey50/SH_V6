//  $File : FileFilter.cpp: implementation of the CFileFilter class.
//  (C) Copyright The FileX Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)

#include "stdafx.h"
#include "FileFilter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define _NAME_GETNAME "?GetInfo@@YGPBDXZ"
#define _NAME_SAVE    "?Save@@YG_NPAUtagBITMAPINFOHEADER@@PBD_N@Z"
#define _NAME_LOAD    "?Load@@YGKPAPAUtagBITMAPINFOHEADER@@PBD@Z"
#define _NAME_FREEBMIH    "?FreeBMIH@@YGXPAPAUtagBITMAPINFOHEADER@@@Z"
#define _NAME_GETFILTERSTRING "?GetFilterString@@YGPBDXZ"

typedef LPCTSTR		 (FAR __stdcall *GETINFO)(void);
typedef bool         (FAR __stdcall *SAVE)(LPBITMAPINFOHEADER pic, LPCTSTR fName, bool ShowDlg);
typedef DWORD        (FAR __stdcall *LOAD)(BITMAPINFOHEADER **pic, LPCTSTR fName);
typedef void         (FAR __stdcall *FREEBMIH)(BITMAPINFOHEADER **pic);
typedef const char   (FAR __stdcall *GETFILTERSTRING)(void);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileFilter::CFileFilter(const char *fname):
        m_GetInfo(NULL),m_Save(NULL)
{
    m_Library=::LoadLibrary(fname);
    if (!m_Library) return;
    m_GetInfo = GetProcAddress(m_Library,_NAME_GETNAME);
    m_Save    = GetProcAddress(m_Library,_NAME_SAVE);
    m_Load    = GetProcAddress(m_Library,_NAME_LOAD);
    m_FreeBMIH = GetProcAddress(m_Library,_NAME_FREEBMIH);
    m_GetFilterString = GetProcAddress(m_Library,_NAME_GETFILTERSTRING);
}

CFileFilter::~CFileFilter()
{
    if (m_Library) ::FreeLibrary(m_Library);
}

bool CFileFilter::Save(LPBITMAPINFOHEADER pic, LPCTSTR fName, bool ShowDlg)
{
    if ((!m_Library) || (!m_Save)) 
    { 
      m_ErrorMessage="Filter library isn't loaded"; 
      return(false); 
    }
    return (((SAVE)m_Save)(pic, fName, ShowDlg));
}

LPCTSTR CFileFilter::GetInfo()
{
    if (!m_GetInfo)
    {
        return ("Err: Library not found");
    }
    return(((GETINFO)m_GetInfo)());
}

LPCTSTR CFileFilter::GetFilterString()
{
    if (!m_GetInfo)
    {
        return NULL;
    }
    return(((GETINFO)m_GetFilterString)());

}


LPBITMAPINFOHEADER CFileFilter::Load(const char *fName)
{
    if ((!m_Library) || (!m_Load)) { m_ErrorMessage="Filter library isn't loaded"; return(NULL); }
    BITMAPINFOHEADER *pic;
    int size=((LOAD)m_Load)(&pic, fName);
    if (!size) { m_ErrorMessage="The file format isn't supported"; return NULL;}
    BITMAPINFOHEADER *res=(BITMAPINFOHEADER*)malloc(size);
    memcpy(res,pic,size);
    ((FREEBMIH)m_FreeBMIH)(&pic);
    return(res);
}

bool CFileFilter::IsLoaded()
{
   return(
            (m_Library!=NULL) && 
            (m_GetInfo!=NULL) && 
            (m_Save   !=NULL) && 
            (m_Load   !=NULL) && 
            (m_GetFilterString !=NULL) && 
            (m_FreeBMIH!=NULL)
         );
}


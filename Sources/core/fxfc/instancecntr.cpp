// InstanceCntr.cpp: implementation of the FXInstanceCntr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <fxfc\fxfc.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FXInstanceCntr::FXInstanceCntr(LPCTSTR name)
{
    if (name==NULL)
    {
        m_InstanceName=FxGetAppName();
    }
    else
        m_InstanceName=name;
    m_hMapping = CreateFileMapping( (HANDLE) (-1L),
                                 NULL,
                                 PAGE_READWRITE,
                                 0,
                                 sizeof(int),
                                 name );
    if( m_hMapping )
    {
        if( GetLastError() != ERROR_ALREADY_EXISTS )
        {
            m_pInstanceCntr=(int*)MapViewOfFile(m_hMapping,FILE_MAP_WRITE,0,0,sizeof(int));
            *m_pInstanceCntr=1;
            m_InstNmb=*m_pInstanceCntr;
        }
        else
        {
            m_pInstanceCntr=(int*)MapViewOfFile(m_hMapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(int));
            *m_pInstanceCntr=*m_pInstanceCntr+1;
            m_InstNmb=*m_pInstanceCntr;
        }
    }
    else
    {
        m_pInstanceCntr=NULL;
        m_InstNmb=0;
    }
}

FXInstanceCntr::~FXInstanceCntr()
{
    if (m_pInstanceCntr)
    {
        *m_pInstanceCntr=*m_pInstanceCntr-1;
        UnmapViewOfFile(m_pInstanceCntr); m_pInstanceCntr=NULL;
    }
    if (m_hMapping)
        CloseHandle(m_hMapping); 
    m_hMapping=NULL;
}

int FXInstanceCntr::GetInstanceNmb(LPCTSTR name)
{
    int retVal=0;
    HANDLE hFM=OpenFileMapping(FILE_MAP_READ,FALSE,name);
    if (!hFM)
    {
        TRACE("+++ Warrning! No file mapping \"%s\" exists\n",name);
        return 0;
    }
    int        *pIC=(int*)MapViewOfFile(hFM,FILE_MAP_READ,0,0,sizeof(int));
    if (pIC)
    {
        retVal=*pIC;
        UnmapViewOfFile(pIC);
    }
    CloseHandle(hFM); 
    return retVal;
}
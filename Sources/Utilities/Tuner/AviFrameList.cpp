//  $File : AviFrameList.cpp: implementation of the CAviFrameList class.
//  (C) Copyright HomeBuilt Group and File X Ltd 2002. 

//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#include "stdafx.h"
#include <files\futils.h>
#include "AviFrameList.h"
#include <files\imgfiles.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAviFrameList::CAviFrameList():
    m_FName("")
{
    char* tmpname=_tempnam("C:\\", "aviframe");
    m_TempFileName=tmpname; m_TempFileName+=".bmp";
    free(tmpname);
    m_MultiSection=false;
    m_Stream=NULL;
}

CAviFrameList::~CAviFrameList()
{
    TRY
    {
        CFile::Remove(m_TempFileName);
    }
    CATCH( CFileException, e )
    {
    }
    END_CATCH
    m_AviFile.Close();
}

bool CAviFrameList::Open(const char* fName)
{
    m_FName=::GetFileName(fName);
    FXSIZE pntPos=m_FName.Find('.');
    if (pntPos>=0)
      m_FName=m_FName.Left(pntPos);
    if (m_AviFile.Open(fName,CAviFile::modeRead)==FALSE) return false;
    m_MultiSection=false; //m_AviFile.IsMultySectionAVI();
    m_Stream=m_AviFile.GetStream(streamtypeVIDEO,0);
    if (!m_Stream)
    {
        AfxMessageBox("Can't open file");
    }
    m_Comment="No time marker in a file";
    return true;
}

FXString CAviFrameList::Get()
{
    DWORD timeID=-1;
    pTVFrame ptv=NULL;
    try
    {
        ptv=(pTVFrame)m_AviFile.Read(m_Stream, m_Position);
    }
    catch(...)
    {
    }
    if (ptv)
    {
        saveDIB(m_TempFileName, ptv);
        freeTVFrame(ptv);
    } 
    else 
    {
        //TRACE(m_AviFile.GetErrorMes());
        return FXString("");
    }
    return m_TempFileName;
}

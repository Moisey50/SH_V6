// MatrixReader.cpp: implementation of the CMatrixReader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <imageproc\recognition\MatrixReader.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMatrixReader::CMatrixReader()
{
    m_mxBlck=NULL;
}

CMatrixReader::~CMatrixReader()
{
    if (m_mxBlck) MtrxBlckDone(m_mxBlck);
}

bool CMatrixReader::Read(const pTVFrame ptv, FXString& Result)
{
    if (m_mxBlck) MtrxBlckDone(m_mxBlck);
    m_mxBlck=MtrxSetFrame(ptv);
    //memcpy(GetData(ptv),GetData(m_mxBlck->orgFrame),GetImageSize(m_mxBlck->orgFrame));
    Result=m_mxBlck->result;
    return (Result.GetLength()!=0);
}

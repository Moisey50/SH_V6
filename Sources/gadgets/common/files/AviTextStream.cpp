// AviTextStream.cpp: implementation of the CAviTextStream class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AviTextStream.h"
#include "files\AviFile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAviTextStream::CAviTextStream(PAVIFILE hFile, UINT mode, long id):
CAviStream(hFile, mode)
{
	m_IntFmt.Empty();
	BOOL bResult = ((mode == OF_CREATE) && (id < 0)) ? TRUE : OpenStream(streamtypeTEXT, id);
	if (bResult)
	{
		switch (m_uMode)
		{
		case OF_READ:
			bResult = OpenRead();
			break;
		case OF_CREATE:
			bResult = (id < 0) ? TRUE : OpenOverwrite();
			if (bResult)
				Resume();
			break;
		case OF_WRITE:
			bResult = OpenAppend();
			if (bResult)
				Resume();
			break;
		}
	}
	if (!bResult && m_hStream)
	{
		::AVIStreamRelease(m_hStream);
		m_hStream = NULL;
	}
}

CAviTextStream::CAviTextStream(PAVIFILE hFile, PAVISTREAM hStream, AVISTREAMINFO* pInfo, UINT mode):
CAviStream(hFile, mode, hStream, pInfo)
{
	m_IntFmt.Empty();
	BOOL bResult = FALSE;
	switch (m_uMode)
	{
	case OF_READ:
		bResult = OpenRead();
		break;
	case OF_CREATE:
		bResult = OpenOverwrite();
		if (bResult)
			Resume();
		break;
	case OF_WRITE:
		bResult = OpenAppend();
		if (bResult)
			Resume();
		break;
	}
	if (!bResult && m_hStream)
	{
		::AVIStreamRelease(m_hStream);
		m_hStream = NULL;
	}
    m_Label=pInfo->szName;
}

CAviTextStream::~CAviTextStream()
{
}

BOOL CAviTextStream::OpenRead()
{
	m_LastError = ::AVIStreamReadFormat(m_hStream, ::AVIStreamStart(m_hStream), NULL, (long*)&m_dwBufSize);
	if (!TraceSuccess(m_LastError) || !m_dwBufSize)
		return FALSE;
	m_LastError = ::AVIStreamReadFormat(m_hStream, ::AVIStreamStart(m_hStream), m_IntFmt.GetBufferSetLength(m_dwBufSize), (long*)&m_dwBufSize);
	m_IntFmt.ReleaseBuffer();	
    AVISTREAMINFO    strhdr; 
    AVIStreamInfo(m_hStream, &strhdr, sizeof(strhdr)); 
    m_dwBufSize=strhdr.dwSuggestedBufferSize +1;
	if (!TraceSuccess(m_LastError))
		return FALSE;
	return TRUE;
}

BOOL CAviTextStream::OpenOverwrite()
{
	if (!OpenAppend())
		return FALSE;
	m_nCurFrame = -1;
	return TRUE;
}

BOOL CAviTextStream::OpenAppend()
{
	m_LastError = ::AVIStreamReadFormat(m_hStream, ::AVIStreamStart(m_hStream), NULL, (long*)&m_dwBufSize);
	if (!TraceSuccess(m_LastError))
		return FALSE;
	m_nCurFrame = (int)m_StreamInfo.dwLength - 1;
	return TRUE;
}

void CAviTextStream::DoWrite(void* lpData, LPCTSTR Label)
{
    if (Label) m_Label=Label;
	char* Text = (char*)lpData;
	DWORD dwFlags = (m_nCurFrame % 24) ? 0 : AVIIF_KEYFRAME;
	m_LastError = ::AVIStreamWrite(m_hStream, (LONG)GetCurFrameID(), 1, Text, m_dwBufSize, dwFlags, NULL, NULL);
	free(Text);
	if (TraceSuccess(m_LastError))
 		m_StreamInfo.dwLength++;
}

BOOL CAviTextStream::CheckFormat(void* lpData, LPCTSTR label)
{
	if (m_hStream)
	{
		if (strlen((LPCTSTR)lpData) == m_dwBufSize - 1)
			return TRUE;
	}
	else if (m_uMode == OF_CREATE)
	{
		m_dwBufSize = (DWORD)strlen((LPCTSTR)lpData) + 1;
		strcpy(m_IntFmt.GetBufferSetLength(m_dwBufSize), (LPCTSTR)lpData);
		m_IntFmt.ReleaseBuffer();
		m_StreamInfo.fccType = streamtypeTEXT;
        if (m_OverwriteFrameRate)
        {
		    m_StreamInfo.dwScale = 10;
		    m_StreamInfo.dwRate = (int)(m_FrameRate*m_StreamInfo.dwScale);
        }
        else
        {
		    m_StreamInfo.dwScale = 1;
		    m_StreamInfo.dwRate = 25;
        }
		m_StreamInfo.dwQuality = -1;
		m_StreamInfo.dwSampleSize = m_dwBufSize;
        m_sTickCount=GetTickCount();
        if (label)
        {
            int len=(int)strlen(label);
            if (len>=63)
            {
                memcpy(m_StreamInfo.szName,label,63);
            }
            else
                strcpy(m_StreamInfo.szName,label);
        }
		m_LastError = ::AVIFileCreateStream(m_pFile, &m_hStream, &m_StreamInfo);
		if (TraceSuccess(m_LastError))
		{
			m_LastError = ::AVIStreamSetFormat(m_hStream, 0, (void*)LPCTSTR(m_IntFmt), (LONG)m_dwBufSize);
			if (TraceSuccess(m_LastError))
				return TRUE;
			::AVIStreamRelease(m_hStream);
			m_hStream = NULL;
		}
	}
	free(lpData);
	return FALSE;
}

void* CAviTextStream::DoRead()
{
	if (IsEOF())
		return NULL;
	char* buffer = (char*)calloc(m_dwBufSize, sizeof(char));
	m_LastError = ::AVIStreamRead(m_hStream, (LONG)GetCurFrameID(), 1, buffer, (LONG)m_dwBufSize, NULL, NULL);
	if (!TraceSuccess(m_LastError))
	{
		free(buffer);
		return NULL;
	}
	return buffer;
}
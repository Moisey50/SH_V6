// AviStream.cpp: implementation of the CAviStream class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AviStream.h"
#include "files\AviFile.h"
#include <video\tvframe.h>
#include <gadgets\gadbase.h>

#define THIS_MODULENAME "TVAvi.AviStream"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAviStream::CAviStream(PAVIFILE hFile, UINT mode, PAVISTREAM hStream, AVISTREAMINFO* pInfo, DWORD fourcc):
     m_pFile(hFile)
    ,m_hStream(hStream)
    ,m_nCurFrame(-1)
    ,m_dwBufSize(0)
    ,m_uMode(mode)
    ,m_LastError(0)
    ,m_WriteQueue(20)
    ,m_CalcFrameRate(FALSE)
    ,m_OverwriteFrameRate(FALSE)
    ,m_FrameRate(25.0)
    ,m_sTickCount(-1)
{
	ZeroMemory(&m_StreamInfo, sizeof(m_StreamInfo));
	ZeroMemory(&m_ComressOps, sizeof(m_ComressOps));
	switch (m_uMode)
	{
	case OF_READ:	// read-only mode
		break;
	case OF_CREATE:	// write-only mode, existing stream is truncated
	case OF_WRITE:	// write-only mode, existing stream is appended
		SetTicksIdle(1);
		Create();
		break;
	default:
		ASSERT(FALSE);
	}
	if (pInfo)
		memcpy(&m_StreamInfo, pInfo, sizeof(AVISTREAMINFO));
    ASSERT(fourcc==0);
}

CAviStream::~CAviStream()
{
	UINT mode = m_uMode;
	m_uMode = (UINT)(-1);
	if (mode != OF_READ)
		Destroy();
	if (m_hStream)
		::AVIStreamRelease(m_hStream);
	m_hStream = NULL;
	m_pFile = NULL;
}

void CAviStream::SetFrameRate(BOOL CalcFrameRate, BOOL OverwriteFrameRate, double FrameRate)
{
    m_CalcFrameRate=CalcFrameRate;
    m_OverwriteFrameRate=OverwriteFrameRate;
    m_FrameRate=FrameRate;
}

BOOL CAviStream::SetFourCC(DWORD fourCC)
{
	if (m_uMode == OF_READ)
		return FALSE;
    ASSERT(FALSE);
	return TRUE;
}


BOOL CAviStream::IsValid()
{
	if (m_hStream)
		return TRUE;
	if ((m_uMode == OF_CREATE) && (m_nCurFrame == -1))
		return TRUE;
	return FALSE;
}

BOOL CAviStream::SetNextFrame()
{
	if (!m_hStream)
		return FALSE;
	if (m_nCurFrame >= (int)m_StreamInfo.dwLength)
		return FALSE;
	m_nCurFrame++;
	return TRUE;
}

BOOL CAviStream::SeekTo(int nFrame)
{
	if (!m_hStream)
		return FALSE;
	if ((nFrame < -1) || (nFrame > (int)m_StreamInfo.dwLength))
		return FALSE;
	m_nCurFrame = nFrame;
	return TRUE;
}

BOOL CAviStream::IsEOF()
{
	return (m_nCurFrame >= (int)m_StreamInfo.dwLength);
}

int CAviStream::GetCurFrameID()
{
	if (!m_hStream)
		return -1;
	return m_nCurFrame + (int)m_StreamInfo.dwStart;
}

int CAviStream::GetLength()
{
	return (int)m_StreamInfo.dwLength;
}

double CAviStream::GetTimeLength()
{
    return ((double)GetLength()*m_StreamInfo.dwScale)/m_StreamInfo.dwRate;
}

void* CAviStream::ReadFrame()
{
	if (!m_hStream)
		return NULL;
	return DoRead();
}

BOOL CAviStream::WriteFrame(void* pFrame, LPCTSTR Label)
{
	if (m_uMode == OF_READ)
		return FALSE;
	if (!CheckFormat(pFrame, Label))
		return FALSE;
	m_Lock.Lock();
    if (!m_WriteQueue.PutQueueObject(pFrame))
    {
        SENDLOGMSG(MSG_CRITICAL_LEVEL-1,THIS_MODULENAME,0,"Frame queue overflow");
        freeTVFrame((pTVFrame)pFrame);
        m_Lock.Unlock();
        return FALSE;
    }
	m_Lock.Unlock();
	return TRUE;
}

void CAviStream::Destroy()
{
	BOOL bFinished = FALSE;
	while (!bFinished)
	{
		m_Lock.Lock();
        bFinished = (m_WriteQueue.ItemsInQueue() == 0);
		m_Lock.Unlock();
		Sleep(10);
	}
	FXWorker::Destroy();
}

BOOL CAviStream::OpenStream(DWORD fccType, int id)
{
    TRACE("OpenStream(%c%c%c%c)\n",fccType);
	m_LastError = ::AVIFileGetStream(m_pFile, &m_hStream, fccType, id);
	if (!TraceSuccess(m_LastError))
		return FALSE;
	m_LastError = ::AVIStreamInfo(m_hStream, &m_StreamInfo, (LONG)sizeof(m_StreamInfo));
	return TraceSuccess(m_LastError);
}

int CAviStream::DoJob()
{
	m_Lock.Lock();
    BOOL bFinished = (m_WriteQueue.ItemsInQueue() == 0);
	while (!bFinished)
	{
		SetNextFrame();
        void *d;
        if (m_WriteQueue.GetQueueObject(d))
        {
            DoWrite(d, m_Label);
        }
        bFinished = (m_WriteQueue.ItemsInQueue() == 0);
	}
    m_Lock.Unlock();
	return WR_CONTINUE;
}

BOOL CAviStream::CheckFormat(void* lpData, LPCTSTR label)
{
	return FALSE;
}

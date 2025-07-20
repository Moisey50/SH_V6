// WaveFrame.cpp: implementation of the CQuantityFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <gadgets\WaveFrame.h>
#pragma comment(lib,"winmm.lib")

CWaveFrame::CWaveFrame(const pWaveData wd)
{
    m_DataType = wave;
    if (!wd)
    {
        waveformat=NULL;
        data=NULL;
        hWaveIn=NULL; 
    }
    else
    {
        waveformat=(PWAVEFORMATEX)malloc(sizeof(WAVEFORMATEX));
        memcpy(waveformat, wd->waveformat, sizeof(WAVEFORMATEX));
        
        data=(LPWAVEHDR)malloc(sizeof(WAVEHDR));
        memcpy(data,wd->data, sizeof(WAVEHDR));
        data->dwUser=NULL;
        data->lpData=(LPSTR)malloc(wd->data->dwBufferLength);
        memcpy(data->lpData,wd->data->lpData,wd->data->dwBufferLength);
        data->lpNext=NULL;
        
        hWaveIn=wd->hWaveIn;
    }
}

CWaveFrame::CWaveFrame(const CWaveFrame* wf)
{
	m_DataType = wave;
	if (!wf)
	{
		waveformat = NULL;
		data = NULL;
		hWaveIn = NULL;
	}
	else
	{
		waveformat = (PWAVEFORMATEX)malloc(sizeof(WAVEFORMATEX));
		memcpy(waveformat, wf->waveformat, sizeof(WAVEFORMATEX));
		data = (LPWAVEHDR)malloc(sizeof(WAVEHDR));
		memcpy(data, wf->data, sizeof(WAVEHDR));
		data->dwUser = NULL;
		data->lpData = (LPSTR)malloc(wf->data->dwBufferLength);
		memcpy(data->lpData, wf->data->lpData, wf->data->dwBufferLength);
		data->lpNext = NULL;
		hWaveIn = wf->hWaveIn;

		CopyAttributes(wf);
	}
}

CWaveFrame::~CWaveFrame () 
{
    UnprepareHeader(hWaveIn, data, sizeof(WAVEHDR));
    if (waveformat)
    {
        free(waveformat);
        waveformat=NULL;
    }
    if (data)
    {
        if (data->lpData)
        {
            free(data->lpData);
            data->lpData=NULL;
        }
        free(data);
        data=NULL;
    }
};
void CWaveFrame::UnprepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
{
    if (hwi)
    {
        waveInUnprepareHeader(hwi, pwh, cbwh);
        waveInPrepareHeader  (hwi, pwh, cbwh);
        waveInAddBuffer      (hwi, pwh, cbwh);
    }
    else if (data)
        data->dwFlags=WHDR_DONE;
}
pWaveData CWaveFrame::GetData() 
{ 
    return (pWaveData)this;
}

CWaveFrame* CWaveFrame::GetWaveFrame(LPCTSTR label) 
{ 
    if (!label || m_Label==label) 
        return this; 
    return NULL; 
}

const CWaveFrame* CWaveFrame::GetWaveFrame(LPCTSTR label) const
{ 
    if (!label || m_Label==label) 
        return this; 
    return NULL; 
}

CWaveFrame* CWaveFrame::Create(pWaveData data) 
{ 
    return new CWaveFrame(data);
}

BOOL CWaveFrame::IsNullFrame() const
{ 
    return (waveformat==NULL);
}

BOOL CWaveFrame::Serialize( LPBYTE* ppData, UINT* cbData ) const
{
    ASSERT(FALSE);
    return FALSE;
}

BOOL CWaveFrame::Serialize( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const
{
    ASSERT(FALSE);
    return FALSE;
}

BOOL CWaveFrame::Restore(LPBYTE lpData, UINT cbData)
{
    ASSERT(FALSE);
    return FALSE;
}

// AviStream.h: interface for the CAviStream class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AVISTREAM_H__0ABCE5E0_C075_46BC_BBAD_D9EA68CF0C4B__INCLUDED_)
#define AFX_AVISTREAM_H__0ABCE5E0_C075_46BC_BBAD_D9EA68CF0C4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vfw.h>
#include <gadgets\gadbase.h>

class CAviStream : public FXWorker
{
protected:
	PAVIFILE			m_pFile;
	PAVISTREAM			m_hStream;
	AVISTREAMINFO		m_StreamInfo;
	AVICOMPRESSOPTIONS	m_ComressOps;
	UINT				m_uMode;
	int					m_nCurFrame;
	DWORD				m_dwBufSize;
	FXLockObject		m_Lock;
	FXStaticQueue<void*>	m_WriteQueue;
	HRESULT				m_LastError;
    CString             m_Label;
    BOOL                m_CalcFrameRate;
    BOOL                m_OverwriteFrameRate;
    double              m_FrameRate;
    DWORD               m_sTickCount;
public:
	CAviStream(PAVIFILE hFile, UINT mode, PAVISTREAM hStream = NULL, AVISTREAMINFO* pInfo = NULL, DWORD fourcc=0);
	virtual ~CAviStream();
	// attributes & status
    void                SetFrameRate(BOOL CalcFrameRate, BOOL OverwriteFrameRate, double FrameRate);
	virtual BOOL		SetFourCC(DWORD fourCC);
	virtual BOOL		IsValid();
	// positioning
	BOOL				SetNextFrame();
	BOOL				SeekTo(int nFrame);
	BOOL				IsEOF();
	int					GetCurFrameID();
	int					GetLength();
    double  			GetTimeLength();
	// per-frame I/O
	void*				ReadFrame();
	BOOL				WriteFrame(void* pFrame, LPCTSTR Label);
    LPCTSTR             GetLabel() { return (m_Label.GetLength()!=0)?(LPCTSTR)m_Label:NULL; }
    double              GetStreamTime() { if (m_sTickCount==-1) return 0; return (GetTickCount()-m_sTickCount)*0.001; }
public:
	virtual void		Destroy();
protected:
	BOOL				OpenStream(DWORD fccType, int id);
private:
	virtual void*		DoRead()=0;
	virtual int			DoJob();
	virtual void		DoWrite(void* lpData, LPCTSTR Label)=0;
	virtual BOOL		CheckFormat(void* lpData, LPCTSTR label);
};

#endif // !defined(AFX_AVISTREAM_H__0ABCE5E0_C075_46BC_BBAD_D9EA68CF0C4B__INCLUDED_)

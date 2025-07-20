// AviFile.h: interface for the CAviFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AVIFILE_H__07238AC1_46C5_4840_A9BA_C24524A5B33B__INCLUDED_)
#define AFX_AVIFILE_H__07238AC1_46C5_4840_A9BA_C24524A5B33B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vfw.h>

FXString    VerboseAviError(HRESULT error);
BOOL        TraceSuccess(HRESULT hResult);
FXString    GetAviFileName(LPCTSTR fmt);

class CFps;

class CAviFile  
{
	HRESULT			m_LastAVIError;
	CString			m_FileNameFormat;
    CString         m_RealFileName;
	UINT			m_uMode;
	PAVIFILE		m_pFile;
	AVIFILEINFO		m_FileInfo;
	CMapPtrToPtr	m_Streams;
	FXLockObject	m_Lock;
    DWORD           m_VIDC_FOURCC;
	bool			m_OpenErrorShown;
    BOOL            m_CalcFrameRate;
    BOOL            m_OverwriteFrameRate;
    double          m_FrameRate;
public:
	enum OpenFlags
	{
		modeRead		= OF_READ,
		modeAppend		= OF_WRITE,
		modeOverwrite	= OF_CREATE,
	};
	CAviFile();
	virtual ~CAviFile();
	
	// Managing file
	virtual BOOL	Open(LPCTSTR fileName, UINT uMode);
	virtual void	Close();
    void            SetFrameRate(BOOL CalcFrameRate, BOOL OverwriteFrameRate, double FrameRate);
	BOOL			IsOpen();
	LPCTSTR			GetRealFileName();
    LPCTSTR			GetFileNameFormat();
    DWORD           GetLength() { return m_FileInfo.dwLength; }
	// Managing streams
	int				GetStreamCount();
	void*			GetStream(long id);
	void*			GetStream(DWORD type, long id);
	DWORD			GetStreamType(void* pStream);
	void			SetVideoFourCC(DWORD fourCC);
    DWORD			GetVideoFourCC();
	void			ReleaseStream(void* pStream);
	
	// Seeking stream
	int				GetPos(void* pStream);
	BOOL			SeekToFrame(void* pStream, int nFrame);
	void			SeekToBegin(void* pStream);
    void            SeekToEnd(void* pStream);
	BOOL			IsEOF(void* pStream);
	int				GetLength(void* pStream);
	double  		GetTimeLength(void* pStream);
	// Stream I/O (per frame)
	void*			Read(void* pStream, DWORD& ID);
	BOOL			Write(void* pStream, void* pFrame, LPCTSTR Label);
	// Status
	void ResetErrors() { m_OpenErrorShown=false; }
	void ErrorShown()  { m_OpenErrorShown=true; }
private:
    bool            OverwriteFPS(CFps* fps);
};

#endif // !defined(AFX_AVIFILE_H__07238AC1_46C5_4840_A9BA_C24524A5B33B__INCLUDED_)

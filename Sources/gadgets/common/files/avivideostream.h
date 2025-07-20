// AviVideoStream.h: interface for the CAviVideoStream class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AVIVIDEOSTREAM_H__E63F44B4_21E9_4D3A_A616_363B765B7182__INCLUDED_)
#define AFX_AVIVIDEOSTREAM_H__E63F44B4_21E9_4D3A_A616_363B765B7182__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AviStream.h"

class CAviVideoStream : public CAviStream
{
	LPBITMAPINFOHEADER	m_pIntFmt;
	DWORD				m_dwIntFmtSize;
	PGETFRAME		m_pGetFrame;
  bool        m_CompressorNotAvailableShown;
  int         m_iApproxFrameRate ;
public:
	CAviVideoStream(PAVIFILE hFile, UINT mode = OF_READ, long id = 0, DWORD fourcc=0 );
	CAviVideoStream(PAVIFILE hFile, PAVISTREAM hStream, AVISTREAMINFO* pInfo, UINT mode = OF_READ, DWORD fourcc=0 );
	virtual ~CAviVideoStream();
private:
	BOOL				OpenRead();
	BOOL				OpenOverwrite();
	BOOL				OpenAppend();
    BOOL                SetFourCC(DWORD fourCC);
	virtual void*		DoRead();
	virtual void		DoWrite(void* lpData, LPCTSTR Label);
	virtual BOOL		CheckFormat(void* lpData, LPCTSTR label);
    virtual void    OnInit() { CoInitialize(NULL); };
    virtual void    OnExit() { CoUninitialize(); };
};

#endif // !defined(AFX_AVIVIDEOSTREAM_H__E63F44B4_21E9_4D3A_A616_363B765B7182__INCLUDED_)

// AviTextStream.h: interface for the CAviTextStream class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AVITEXTSTREAM_H__B77DACCC_C747_4D29_8CFE_CF9A24EB71BE__INCLUDED_)
#define AFX_AVITEXTSTREAM_H__B77DACCC_C747_4D29_8CFE_CF9A24EB71BE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AviStream.h"

class CAviTextStream : public CAviStream
{
	CString	m_IntFmt;
public:
	CAviTextStream(PAVIFILE hFile, UINT mode = OF_READ, long id = 0);
	CAviTextStream(PAVIFILE hFile, PAVISTREAM hStream, AVISTREAMINFO* pInfo, UINT mode = OF_READ);
	virtual ~CAviTextStream();
private:
	BOOL				OpenRead();
	BOOL				OpenOverwrite();
	BOOL				OpenAppend();
	virtual void*		DoRead();
	virtual void		DoWrite(void* lpData, LPCTSTR Label);
	virtual BOOL		CheckFormat(void* lpData, LPCTSTR label);
};

#endif // !defined(AFX_AVITEXTSTREAM_H__B77DACCC_C747_4D29_8CFE_CF9A24EB71BE__INCLUDED_)

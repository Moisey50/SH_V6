// CodecEnumerator.h: interface for the CCodecEnumerator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CODECENUMERATOR_H__C4CFD59B_5AC5_46B9_9857_25341D04A70A__INCLUDED_)
#define AFX_CODECENUMERATOR_H__C4CFD59B_5AC5_46B9_9857_25341D04A70A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>

typedef struct _codecInfo
{
    DWORD   fccHandler;
    FXString    cName;
    FXString    Description;
}codecInfo;

class CCodecEnumerator  
{
private:
    CArray<codecInfo,codecInfo> m_Codecs;
public:
	CCodecEnumerator();
	virtual ~CCodecEnumerator();
    int GetCodecCount() { return (int)m_Codecs.GetSize(); }
    codecInfo& GetCodecItem(int i) { return m_Codecs[i]; }
};

#endif // !defined(AFX_CODECENUMERATOR_H__C4CFD59B_5AC5_46B9_9857_25341D04A70A__INCLUDED_)

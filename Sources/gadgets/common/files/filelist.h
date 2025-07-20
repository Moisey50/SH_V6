// FileList.h: interface for the CFileList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILELIST_H__0A4CE5F3_E1AB_11D2_8457_000001359766__INCLUDED_)
#define AFX_FILELIST_H__0A4CE5F3_E1AB_11D2_8457_000001359766__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <files\srclist.h>

class CFileList : public CSrcList
{
protected:
    FXString m_Path;
    FXArray<FXString,FXString&> m_List;
public:
	FXString GetPathName();
	        CFileList();
	virtual ~CFileList();
    bool    IsStream() {return false;};
	BOOL    Dir(LPCTSTR path, LPCTSTR masks);
    DWORD   GetSize()   {return (DWORD)m_List.GetSize();};
    void    RemoveAll() {m_List.RemoveAll();};
    int     Add(FXString& newElement ) { return (int)m_List.Add(newElement);};
    FXString GetAt( int nIndex ) const { return m_List.GetAt(nIndex);};
    FXString GetName() const {return m_List.GetAt(m_Position);};
    void    RemoveAt( int nIndex, int nCount = 1 ) {m_List.RemoveAt(nIndex,nCount);};
    BOOL    SetPosition(const FXString& fName);
    BOOL    Next();
    BOOL    Previous();
    BOOL    NextAvailable()     { return m_List.GetSize()>(int)m_Position+1; }
    BOOL    PreviousAvailable() { return (int)m_Position-1>=0; };
	void	Delete();
    void    SetFirst();
    void    SetLast();
    FXString Get();
	void    GetPathName(FXString& fName);
	void    Get(FXString & fName);
};

#endif // !defined(AFX_FILELIST_H__0A4CE5F3_E1AB_11D2_8457_000001359766__INCLUDED_)

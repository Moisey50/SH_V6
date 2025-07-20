// FileFilter.h: interface for the CFileFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEFILTER_H__B029D3CA_7AB8_11D5_9462_000001360793__INCLUDED_)
#define AFX_FILEFILTER_H__B029D3CA_7AB8_11D5_9462_000001360793__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFileFilter  
{
private:
    HINSTANCE m_Library;
    FARPROC   m_GetInfo;
    FARPROC   m_Save;
    FARPROC   m_Load;
    FARPROC   m_FreeBMIH;
    FARPROC   m_GetFilterString;
    CString   m_ErrorMessage;
public:
	LPCTSTR		GetFilterString();
	bool		IsLoaded();
            	CFileFilter(const char *fname);
	virtual     ~CFileFilter();
    bool        Save(LPBITMAPINFOHEADER pic, LPCTSTR fName, bool ShowDlg=false);
    LPBITMAPINFOHEADER Load(LPCTSTR fName);
	LPCTSTR		GetInfo();
    LPCTSTR		GetErrorMessage() {return(m_ErrorMessage);};
};

#endif // !defined(AFX_FILEFILTER_H__B029D3CA_7AB8_11D5_9462_000001360793__INCLUDED_)

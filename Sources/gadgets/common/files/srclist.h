// SrcList.h: interface for the CSrcList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SRCLIST_H__9814C9A2_B755_11D5_9463_0080AD70FF26__INCLUDED_)
#define AFX_SRCLIST_H__9814C9A2_B755_11D5_9463_0080AD70FF26__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSrcList
{
protected:
    DWORD     m_Position;
    DWORD     m_TimeID;
    FXString  m_Comment;
public:
	CSrcList();
	virtual ~CSrcList();
    virtual BOOL    Next()=0;
    virtual BOOL    Previous()=0;
    virtual void    SetFirst()=0;
    virtual void    SetLast()=0;
    virtual FXString Get()=0;
    virtual DWORD   GetSize()=0;
    virtual void	Delete()=0;
    virtual FXString GetName() const =0;
    virtual FXString GetFileName() { return GetName();};
    virtual DWORD   GetSectionsNmb()    { return 0; };
///
    virtual bool    IsStream() {return true;};
    DWORD   GetItemNo() {return(m_Position);};
    BOOL    IsEmpty() { return(GetSize()==0); };
    DWORD   GetTimeID() { return m_TimeID; };
    FXString& GetComment() { return m_Comment; };
    void    SeekTo(DWORD pos) {m_Position=pos; if (m_Position>=GetSize()) m_Position=GetSize()-1;};
};

#endif // !defined(AFX_SRCLIST_H__9814C9A2_B755_11D5_9463_0080AD70FF26__INCLUDED_)

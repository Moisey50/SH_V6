// AviFrameList.h: interface for the CAviFrameList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AVIFRAMELIST_H__D4E0CFA3_B6B0_11D5_9463_0080AD70FF26__INCLUDED_)
#define AFX_AVIFRAMELIST_H__D4E0CFA3_B6B0_11D5_9463_0080AD70FF26__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <files\SrcList.h>
#include <files\AviFile.h>

class CAviFrameList : public CSrcList
{
private:
    CAviFile m_AviFile;
    FXString  m_TempFileName;
    FXString  m_FName;
    bool     m_MultiSection;
    void*    m_Stream;     
public:
	CAviFrameList();
	virtual ~CAviFrameList();
    bool    Open(const char* fName);
// Overwritables
    virtual BOOL    Next()      
    { 
        if (((m_Position)<GetSize()-1))
        { 
            m_Position=m_Position+1; 
            return TRUE; 
        } 
        return FALSE;
    };
    virtual BOOL    Previous()  
    { 
        if (m_Position) 
        {
            m_Position=m_Position-1; 
            return TRUE; 
        }; 
        return FALSE; 
    };
    virtual void    SetFirst()  {m_Position=0;};
    virtual void    SetLast()   {m_Position=GetSize()-1; };
    FXString Get();
    virtual DWORD   GetSize()   {return m_AviFile.GetLength(m_Stream);};
    virtual void	Delete()    {};
    virtual FXString GetName() const {return m_TempFileName;};
    virtual FXString GetFileName() { FXString retS; retS.Format("%s_%06d.bmp",(LPCTSTR)m_FName,m_Position); return retS; };
    DWORD   GetSectionsNmb()    { return 1; };
};

#endif // !defined(AFX_AVIFRAMELIST_H__D4E0CFA3_B6B0_11D5_9463_0080AD70FF26__INCLUDED_)

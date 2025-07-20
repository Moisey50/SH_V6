#pragma once

class CBrowseFolder
{
protected:
    CWnd*       m_pParentWnd;
    FXString    m_Result;
public:
    FXString    m_Title;
    FXString    m_StartFolder;
public:
             CBrowseFolder(CWnd* pParentWnd = NULL );
            ~CBrowseFolder(void);
     int     DoModal( );
     LPCTSTR GetPath() { return m_Result; }
};

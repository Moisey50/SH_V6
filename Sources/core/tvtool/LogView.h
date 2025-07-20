#if !defined(AFX_LOGVIEW_H__8C9B7760_B094_4677_888E_3DCD426088D4__INCLUDED_)
#define AFX_LOGVIEW_H__8C9B7760_B094_4677_888E_3DCD426088D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LogView.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CLogView window
#include <userinterface\WorkspaceBar.h>

class CLogMsg
{
public:
  int      msgLevel , msgId;
  CString  m_Source;
  CString  msgText;
  CTime    tm;
  FILETIME TimeStampAsFileTime;
public:
  CLogMsg()
  {
    msgLevel = msgId = -1;
    msgText.Empty();
  }
  CLogMsg( int level , LPCTSTR src , int id , LPCTSTR txt ) :
    msgLevel( level ) , m_Source( src ) , msgId( id )
  {
    tm = CTime::GetCurrentTime();
    GetSystemTimeAsFileTime(&TimeStampAsFileTime); //Gets the current system time
    if ( txt ) 
      msgText = txt;
  };
};

class CLogView : public CWorkspaceBar
{
private:
  FXLockObject m_PrintLock;
  CListCtrl   m_ListCtrl;
  int         m_ColumnsSumWidth;
  FXStaticQueue <CLogMsg*> m_LogMsgQueue;
  UINT_PTR        m_PrintTimer;
  int         m_Level;
  bool		m_WriteInFile;
  CString     m_LogFileName;
  CStdioFile  m_File;
  // Construction
public:
  CLogView();
  void PrintMsg( int msgLevel , LPCTSTR src , int msgId , LPCTSTR  msgText );
  int  GetLogLevelThreshold()
  {
    return m_Level;
  }
  void SetLogLevelThreshold( int i )
  {
    m_Level = i;
  }
  void WriteInFile( bool Write )
  {
    m_WriteInFile = Write;
  };
  bool IsWriteInFile()
  {
    return m_WriteInFile;
  };
  void SetLogFileName( LPCTSTR name );
  LPCTSTR GetLogFileName()
  {
    return m_LogFileName;
  };
  // Attributes
public:

  // Operations
public:

  // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CLogView)
    //}}AFX_VIRTUAL

  // Implementation
public:
  bool WriteLogMessage( CLogMsg* lm );
  ~CLogView();
  BOOL Create( CWnd* pParentWnd );
  // Generated message map functions
protected:
  //{{AFX_MSG(CLogView)
  afx_msg void OnTimer( UINT_PTR nIDEvent );
  afx_msg void OnDestroy();
  afx_msg void OnSize( UINT nType , int cx , int cy );
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGVIEW_H__8C9B7760_B094_4677_888E_3DCD426088D4__INCLUDED_)

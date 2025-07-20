#if !defined(AFX_FLOATWND_H__3313D32D_5692_4BC9_A427_C9EDF68DEE87__INCLUDED_)
#define AFX_FLOATWND_H__3313D32D_5692_4BC9_A427_C9EDF68DEE87__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FloatWnd.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CFloatWnd dialog

class CFloatWnd : public CDialog
{
  CWnd*           m_ParentWnd;
  CMapStringToPtr m_Channels;
  CWnd*           m_pActiveWnd;
  CString         m_Name;
  bool            m_OkToDestroy;
  CString         m_MainViewed ;
  // Construction
public:
  CFloatWnd( CString name , CWnd* pParent = NULL );   // standard constructor
  void InsertChannel( LPCTSTR uid , CWnd* pGlyphFrame );
  BOOL DestroyChannel( LPCTSTR uid );
  BOOL SetActiveChannel( LPCTSTR uid , bool bMainViewed = false );
  BOOL RenameChannel( LPCTSTR OldUID , LPCTSTR NewUID ) ;
  BOOL IsEmpty();
  BOOL PreTranslateMessage( MSG* pMsg );
  // Dialog Data
    //{{AFX_DATA(CFloatWnd)
  enum { IDD = IDD_FLOAT_WND };
  CTabCtrl m_DispTab;
  //}}AFX_DATA
public:
  CWnd* ShowChannel( LPCTSTR uid , BOOL Show = TRUE );
  int SeekTabItem( LPCTSTR name );
  CString GetTabItemName( int n );
  BOOL SetTabItemName( int n , LPCTSTR pNewItemName );
  void CorrectSize();
  CString GetCurrentTabSelection();
  CWnd* GetWindow() { return m_pActiveWnd; }
  LPCTSTR GetName() { return m_Name; }

  // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CFloatWnd)
public:
  virtual BOOL Create( UINT nIDTemplate , CWnd* pParentWnd = NULL );
protected:
  virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
  virtual void OnOK();
  virtual void OnCancel();
  //}}AFX_VIRTUAL

// Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CFloatWnd)
  afx_msg void OnSize( UINT nType , int cx , int cy );
  afx_msg void OnSelchangeDispTab( NMHDR* pNMHDR , LRESULT* pResult );
  afx_msg void OnDestroy();
  afx_msg void OnClose();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnMove( int x , int y );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLOATWND_H__3313D32D_5692_4BC9_A427_C9EDF68DEE87__INCLUDED_)

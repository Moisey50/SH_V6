#if !defined(AFX_TERMWINDOW_H__C65659D2_E005_11D1_AF55_000001359766__INCLUDED_)
#define AFX_TERMWINDOW_H__C65659D2_E005_11D1_AF55_000001359766__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TermWindow.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTermWindow window

class FX_EXT_SHBASE CTermWindow : public CWnd
{
  // Construction
  CBrush		m_Brush;
  int			  m_nSelLine;
  int			  m_nTopLine;
  FXString	m_CurLine;
  FXStaticQueue<FXString>	m_WriteQueue;
  CFont		m_Font;
  BOOL		m_bDrawPartial;
  BOOL		m_OnlyOneFrame;
  UINT_PTR    m_Timer;
  bool    m_Invalidate;
  bool    m_bClear;
  bool    m_bViewTiming;
  bool    m_bViewLabel;
  DWORD   m_dwPrevTickCount;
  FXString m_Prefix ;
  int     m_iLevel ;

  //FXLockObject m_renderLock;
public:
  CTermWindow();
  // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CTermWindow)
public:
  virtual BOOL Create(CWnd* pParentWnd);
  //}}AFX_VIRTUAL

// Implementation
public:
  virtual ~CTermWindow();
  BOOL IsValid() { return ::IsWindow(GetSafeHwnd()); };
  void Render(const CDataFrame* pDataFrame);
  bool AppendText(LPCTSTR Str);
  bool SetText(LPCTSTR Str);
  bool IsViewLabel() { return m_bViewLabel; }
  bool IsViewTiming() { return m_bViewLabel; }
  void SetViewLabel(bool bSet) { m_bViewLabel = bSet; }
  void SetViewTiming(bool bSet) { m_bViewTiming = bSet; }
  void SetClear( bool bSet ) { m_bClear = true ; }
  // Generated message map functions
protected:
  void ShowHistory(bool Show) { m_OnlyOneFrame = !Show; }
  int GetLinesInView();
  int GetLineHeight();
  int GetLinesCount();
  int GetTopIndex();
  int GetBottomIndex();
  int IndexFromPt(CPoint& pt);
  void SelectLine(int index);
  void PutLine(FXString& Line);
  void UpdateVertBar();
  void AutoScroll();
  void FormFrameTextView(
    const CDataFrame * pDataFrame, FXString& Result ,
    int iLevel = -1 );
  //{{AFX_MSG(CTermWindow)
  afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnPaint();
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnDestroy();
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg BOOL OnMouseWheel( UINT nFlags , short zDelta , CPoint pt );
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TERMWINDOW_H__C65659D2_E005_11D1_AF55_000001359766__INCLUDED_)

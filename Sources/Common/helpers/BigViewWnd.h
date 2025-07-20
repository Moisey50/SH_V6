#if !defined(AFX_BIGVIEWWND_H__472409C2_F97F_4A58_B30C_179563A79175__INCLUDED_)
#define AFX_BIGVIEWWND_H__472409C2_F97F_4A58_B30C_179563A79175__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BigViewWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBigViewWnd window

class CBigViewWnd : public CWnd
{
// Construction
public:
	CBigViewWnd();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBigViewWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	int m_iNStrings;
	int SetOutputText( const char * OutText );
	CString m_BigText;

  COLORREF m_LaserTempColor ;
  COLORREF m_CollimTempColor ;
  COLORREF m_WindowTempColor ;
  COLORREF m_PolygonTempColor;
  COLORREF m_EosSideTempColor;
  COLORREF m_SosSideTempColor;

	virtual ~CBigViewWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBigViewWnd)
	afx_msg void OnPaint();
  afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BIGVIEWWND_H__472409C2_F97F_4A58_B30C_179563A79175__INCLUDED_)

#if !defined(AFX_TEXTVIEW_H__61DA7C7A_ED5E_4800_B0A9_8005D7B68715__INCLUDED_)
#define AFX_TEXTVIEW_H__61DA7C7A_ED5E_4800_B0A9_8005D7B68715__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextView window

class FX_EXT_SHBASE CTextView : public CStatic
{
	CString         m_Text;
  COLORREF        m_TextColor ;
  COLORREF        m_BkColor ;
	CFont           m_Font;
	FXLockObject    m_Lock;
// Construction
public:
	CTextView();

// Attributes
public:
	BOOL IsValid() { return ::IsWindow(GetSafeHwnd()); };
	void SetText(LPCTSTR text);
    void Render(const CDataFrame* pDataFrame);
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextView)
	public:
	virtual BOOL Create(CWnd* pParentWnd);
	virtual BOOL DestroyWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	~CTextView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTextView)
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTVIEW_H__61DA7C7A_ED5E_4800_B0A9_8005D7B68715__INCLUDED_)

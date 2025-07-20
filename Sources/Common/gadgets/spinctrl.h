#if !defined(AFX_SPINCTRL_H__CE5A727B_0B1F_4CC4_91AE_658C101983CF__INCLUDED_)
#define AFX_SPINCTRL_H__CE5A727B_0B1F_4CC4_91AE_658C101983CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpinCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSpinCtrl window

class CSpinCtrl : public CStatic
{
private:
    CEdit           m_Buddy;
    CSpinButtonCtrl m_Ctrl;
    int             m_iMin , m_iMax ;
public:
	     CSpinCtrl();
    int  SetPos( int nPos );
    void SetRange( int nLower, int nUpper );
    void GetRange( int& iMin , int& iMax ) ;
    int  GetPos( );
    void Enable( BOOL bEnable )
    {
      m_Buddy.EnableWindow( bEnable ) ;
      m_Ctrl.EnableWindow( bEnable ) ;
    }
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpinCtrl)
	public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID = 0xffff);
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSpinCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSpinCtrl)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

class CSpinChkCtrl : public CStatic
{
private:
    CEdit           m_Buddy;
    CSpinButtonCtrl m_Ctrl;
    CButton         m_Button;
    int             m_iMin , m_iMax ;
public:
	  CSpinChkCtrl();
    int  SetPos( int nPos );
    void SetRange( int nLower, int nUpper );
    void GetRange( int& iMin , int& iMax ) ;
    int  GetPos( );
    bool GetCheck();
    void SetCheck(bool set);
    void Enable( BOOL bEnable )
    {
      //m_Button.EnableWindow( bEnable ) ;
      m_Buddy.EnableWindow( bEnable ) ;
      m_Ctrl.EnableWindow( bEnable ) ;
    }
    BOOL bIsCntlEnabled() { return ::IsWindowEnabled( m_Ctrl.m_hWnd ) ; } ;
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpinChkCtrl)
	public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID = 0xffff);
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSpinChkCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSpinChkCtrl)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

class CChkCtrl : public CStatic
{
private:
   CButton         m_Button;
public:
  CChkCtrl();
  bool GetCheck();
  void SetCheck( bool set );
  void Enable( BOOL bEnable )
  {
    m_Button.EnableWindow( bEnable ) ;
  }
public:

  // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSpinChkCtrl)
public:
  virtual BOOL Create( DWORD dwStyle , const RECT& rect , 
    CWnd* pParentWnd , UINT nID = 0xffff , LPCTSTR name = _T("Unknown") );
protected:
  virtual BOOL OnCommand( WPARAM wParam , LPARAM lParam );
  //}}AFX_VIRTUAL

// Implementation
public:
  virtual ~CChkCtrl();

  // Generated message map functions
protected:
  //{{AFX_MSG(CSpinChkCtrl)
  afx_msg void OnSize( UINT nType , int cx , int cy );
  afx_msg void OnSetFocus( CWnd* pOldWnd );
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
};



/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPINCTRL_H__CE5A727B_0B1F_4CC4_91AE_658C101983CF__INCLUDED_)

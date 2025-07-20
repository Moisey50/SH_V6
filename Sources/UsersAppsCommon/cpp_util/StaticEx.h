#if !defined(AFX_STATICEX_H__1A4546FD_0BB4_4516_960F_17DFDC6ED3D2__INCLUDED_)
#define AFX_STATICEX_H__1A4546FD_0BB4_4516_960F_17DFDC6ED3D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StaticEx.h : header file
//

static const UINT NEAR PN_MESSAGE_UPDATEDATA = RegisterWindowMessage(_T("PN_MESSAGE_UPDATEDATA"));
static const UINT NEAR PN_MESSAGE_LOADDATA = RegisterWindowMessage(_T("PN_MESSAGE_LOADDATA"));

#define ON_PN_UPDATEDATA(memberFxn)  \
     ON_REGISTERED_MESSAGE(PN_MESSAGE_UPDATEDATA,memberFxn)


#define ON_PN_LOADDATA(memberFxn)  \
     ON_REGISTERED_MESSAGE(PN_MESSAGE_LOADDATA,memberFxn)
/////////////////////////////////////////////////////////////////////////////
// CStaticEx window
#include "RelationalCheckList.h"
class CRelationalCheckList;

class CStaticEx : public CWnd//CButton//CStatic
{
// Construction
public:
	CStaticEx();
    CStaticEx(CRelationalCheckList* pBuddy);
// Attributes
public:

// Operations
public:
  BOOL Create(CWnd* pParentWnd, UINT nStyle=0, UINT nID=0);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStaticEx)
	public:
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL
   static LRESULT CALLBACK HookProc(int nCode,WPARAM wParam,LPARAM lParam);
// Implementation
public:
	void HideItems();
	virtual ~CStaticEx();
   CRelationalCheckList* m_pBuddy;
	// Generated message map functions
protected:
	//{{AFX_MSG(CStaticEx)
	afx_msg void OnPaint();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
    static CStaticEx* m_pThis;
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATICEX_H__1A4546FD_0BB4_4516_960F_17DFDC6ED3D2__INCLUDED_)

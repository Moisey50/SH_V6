#if !defined(AFX_RELATIONALCHECKLIST_H__1CA5F91C_D5CE_4B74_8F7E_1A4990500A48__INCLUDED_)
#define AFX_RELATIONALCHECKLIST_H__1CA5F91C_D5CE_4B74_8F7E_1A4990500A48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RelationalCheckList.h : header file
//

#include <afxtempl.h>

/////////////////////////////////////////////////////////////////////////////
// CRelationalCheckList window
#include "StaticEx.h"
class CStaticEx;

class CRelationalCheckList : public CListBox
{

  struct __item
  {
	  struct __wnd{
        CRect m_rect;
		CWnd* m_pwnd;
	    CWnd* m_pwndOrgParent;

	  };
	CArray< __wnd*, __wnd*> m_lwndItems;
	BOOL                    m_bRelational;
	CRect                   m_nRect;
	BOOL                    m_bChecked;
	int                     m_nIndex;
	DWORD                   m_dwFlags;
	CSize                   m_rcPanel;
	BOOL                    m_bSelected;
	BOOL                    m_bEnabled;
	__item()
	{
        m_bRelational = FALSE;
		m_bChecked    = FALSE;
		m_bSelected   = FALSE;
		m_bEnabled     = TRUE;
		m_dwFlags     = 0;
	}

	~__item()
	{
      for(int i=0; i<m_lwndItems.GetSize(); i++)
		  delete m_lwndItems[i];
	}
  };
  CStaticEx*             m_pwndPanel;
  CFont                  m_font;
// Construction
public:
	CRelationalCheckList();
    CArray<__item::__wnd*,__item::__wnd*> *m_pCurArray;
	CArray<__item*,__item*>m_lItems;
	int  m_CurDropDownItem;
// Attributes
public:

// Operations
public:
   int  AddString(LPCTSTR lpszItem);
   int  AddRelationalString(LPCTSTR lpszItem);
   void ResetContent();
   int  DeleteString(UINT nIndex);
   void AddWnd(int nIndex,CWnd* pwnd,CRect rc);
   void RemoveWnd(int nIndex,CWnd* pwnd);

   void SetCheck(int nIndex,int nCheck);
   BOOL IsEnabled(int nIndex);
   void Enable(int nIndex,BOOL bEnabled = TRUE);
   int  GetCheck(int nIndex );
   UINT GetCheckStyle();
   void SetCheckStyle(UINT nStyle);

   virtual CRect OnGetCheckPosition(CRect rectItem,CRect rectCheckBox);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRelationalCheckList)
	public:
     virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL ItemIsChecked(int nIndex);
	void SetPanelSize(int nIndex,CSize rc);
	virtual ~CRelationalCheckList();

	// Generated message map functions
protected:
	//{{AFX_MSG(CRelationalCheckList)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG


    __item* getItem(CPoint pt);
	void showPanel(__item* pitem);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RELATIONALCHECKLIST_H__1CA5F91C_D5CE_4B74_8F7E_1A4990500A48__INCLUDED_)

#if !defined(AFX_GADGETSTREE_H__9B52C93A_9EAD_4EEA_9469_E51AB5729F1C__INCLUDED_)
#define AFX_GADGETSTREE_H__9B52C93A_9EAD_4EEA_9469_E51AB5729F1C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GadgetsTree.h : header file
//

//#define IDC_GADGETS_TREE	7100

/////////////////////////////////////////////////////////////////////////////
// CGadgetsTree window

class CGadgetsTree : public CTreeCtrl
{
	CImageList	m_Icons;
// Construction
public:
	CGadgetsTree();

// Attributes
public:
	enum
	{
		GTI_CAPTURES,
		GTI_FILTERS,
		GTI_CTRLS,
		GTI_SPLITTERS,
		GTI_OTHER,
		//----------//
		GTI_COUNT
	};
    bool m_GroupByMedia;
private:
	HTREEITEM	m_hRootItem[GTI_COUNT];
	LPCTSTR		m_ItemLabel[GTI_COUNT];

	HTREEITEM EnsureGroup(HTREEITEM hRoot, LPCTSTR group);
	HTREEITEM FindSubGroup(HTREEITEM hRoot, LPCTSTR group);
	HTREEITEM FindItem(HTREEITEM hRoot, LPCTSTR name, LPVOID lpData);
	BOOL DeleteBranch(HTREEITEM hItem);
// Operations
public:
	HTREEITEM InsertItem(UINT type, LPCTSTR lineage, LPCTSTR item);
	BOOL DeleteItem(UINT type, LPCTSTR lineage, LPCTSTR item, LPVOID lpData);
	BOOL DeleteItem(LPCTSTR lineage, LPCTSTR item, LPVOID lpData);
	BOOL IsGroup(HTREEITEM hItem);
    BOOL DeleteAllItems();
    bool IsGroupByMedia() { return m_GroupByMedia; }
    void ShowGroupByMedia(bool show) { m_GroupByMedia=show; }
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGadgetsTree)
	public:
//	virtual BOOL Create(CWnd* pParentWnd, const RECT& rect);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL CreateItemIcons();
	void CreateRootItems();
	~CGadgetsTree();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGadgetsTree)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GADGETSTREE_H__9B52C93A_9EAD_4EEA_9469_E51AB5729F1C__INCLUDED_)

#if !defined(AFX_CONTAINERVIEW_H__DC25C287_39FD_4B93_85B2_8CE8EC3849F7__INCLUDED_)
#define AFX_CONTAINERVIEW_H__DC25C287_39FD_4B93_85B2_8CE8EC3849F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ContainerView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CContainerView window

class FX_EXT_SHBASE CContainerView : public CTreeCtrl
{
protected:
    FXLockObject    m_Lock;
    volatile bool   m_IsAboutToQuit;
public:
			 CContainerView();
	virtual ~CContainerView();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CContainerView)
	public:
	virtual BOOL Create(CWnd* pParentWnd);
	virtual BOOL DestroyWindow();
	//}}AFX_VIRTUAL
	void		Render(const CDataFrame* pDataFrame);
	CDataFrame* CutUnselected(const CDataFrame* pDataFrame);
	// Generated message map functions
protected:
	//{{AFX_MSG(CContainerView)
	afx_msg void OnDestroy();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnCheckStateChange(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
private:
	HTREEITEM	DoRender(HTREEITEM hParentItem, HTREEITEM hItem, CDataFrame* pDataFrame);
	void		DescribeFrame(const CDataFrame* pDataFrame, CString& txt, CFramesIterator*& Iterator);
	bool		CompareDescriptions(LPCTSTR dsc1, LPCTSTR dsc2);
	void		CutUnselected(CDataFrame* pDataFrame, CFramesIterator* Iterator, HTREEITEM hItem);
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTAINERVIEW_H__DC25C287_39FD_4B93_85B2_8CE8EC3849F7__INCLUDED_)


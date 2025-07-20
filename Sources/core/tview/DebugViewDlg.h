#if !defined(AFX_DEBUGVIEWDLG_H__49DFD731_4769_4DD6_907F_ADF4187ED458__INCLUDED_)
#define AFX_DEBUGVIEWDLG_H__49DFD731_4769_4DD6_907F_ADF4187ED458__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DebugViewDlg.h : header file
//
#include <gadgets\shkernel.h>
#include <shbase\shbase.h>


/////////////////////////////////////////////////////////////////////////////
// CDebugViewDlg dialog
#define UID_DEBUG_RENDER "tvdb400_debug_render"

class CDebugViewDlg : public CDialog
{
private:
    CDIBRender		*m_VideoView;
	CContainerView	*m_ContainerView;
public:
	void MoveView(int ID, LPRECT rc);
	               CDebugViewDlg(CWnd* pParent = NULL);   // standard constructor
    void           Render(const CDataFrame* pDataFrame);
    CRenderGadget* GetGadget(IGraphbuilder* Builder);
    BOOL           GetUID(FXString& uid) { uid=UID_DEBUG_RENDER; return (GetSafeHwnd()!=NULL); }
// Dialog Data
	//{{AFX_DATA(CDebugViewDlg)
	enum { IDD = IDD_DEBUGVIEW };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDebugViewDlg)
	public:
	virtual BOOL Create(CWnd* pParentWnd);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDebugViewDlg)
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEBUGVIEWDLG_H__49DFD731_4769_4DD6_907F_ADF4187ED458__INCLUDED_)

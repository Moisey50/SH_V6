#if !defined(AFX_PRXYWND_H__F30919F0_0E85_4922_9446_E7379FE3F65B__INCLUDED_)
#define AFX_PRXYWND_H__F30919F0_0E85_4922_9446_E7379FE3F65B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrxyWnd.h : header file
//

typedef struct tagScrollCmd
{
    UINT nSBCode;
    UINT nPos; 
    CScrollBar* pScrollBar;
}ScrollCmd, *pScrollCmd;

/////////////////////////////////////////////////////////////////////////////
// CPrxyWnd window

typedef void(__stdcall *PrxyWndCallback)(UINT nID, int nCode, void* cbParam, void* pParam); 

class CPrxyWnd : public CStatic
{
protected:
    PrxyWndCallback m_Callback;
    void*           m_UserParam;
    CWnd*           m_InnerWnd;
public:
	CPrxyWnd();
    void Init(CWnd* pWnd, PrxyWndCallback pwc, void *pUserParam) { m_InnerWnd=pWnd; m_Callback=pwc; m_UserParam=pUserParam; }
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrxyWnd)
	public:
	virtual BOOL Create(CWnd* pParentWnd);
	virtual BOOL DestroyWindow();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL
	virtual ~CPrxyWnd();
protected:
	//{{AFX_MSG(CPrxyWnd)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRXYWND_H__F30919F0_0E85_4922_9446_E7379FE3F65B__INCLUDED_)

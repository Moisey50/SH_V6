#if !defined(AFX_STEREOVIEW_H__1919AF44_D7F7_4B69_A99A_FD9E6FD0A756__INCLUDED_)
#define AFX_STEREOVIEW_H__1919AF44_D7F7_4B69_A99A_FD9E6FD0A756__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// stereoview.h : header file
//
#include <video\TVFrame.h>
/////////////////////////////////////////////////////////////////////////////
// CStereoView window

class CStereoView : public CWnd
{
private:
    CString     m_Name;
    FXLockObject m_LockBMIH, m_DrawLock;
    LPBITMAPINFOHEADER m_lpBMIH;
    int         m_lpBMIHSize;
    CPoint      m_ScrOffset;
public:
	CStereoView(LPCTSTR name="");
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStereoView)
	public:
	virtual BOOL Create(CWnd* pParentWnd, DWORD dwAddStyle=0);
	//}}AFX_VIRTUAL
public:
	virtual ~CStereoView();
    virtual  bool LoadFrames(const pTVFrame frame1, const pTVFrame frame2, bool bForceInvalidate = true);
    virtual  bool Draw(HDC hdc,RECT& rc);
protected:
	//{{AFX_MSG(CStereoView)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STEREOVIEW_H__1919AF44_D7F7_4B69_A99A_FD9E6FD0A756__INCLUDED_)

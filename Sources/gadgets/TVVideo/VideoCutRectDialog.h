#if !defined(AFX_VIDEOCUTRECTDIALOG_H__A93B1808_39D2_478C_BAB0_ABD54B80C45A__INCLUDED_)
#define AFX_VIDEOCUTRECTDIALOG_H__A93B1808_39D2_478C_BAB0_ABD54B80C45A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VideoCutRectDialog.h : header file
//

#include <Gadgets\VideoFrame.h>
#include "DibRect.h"
#include "resource.h"
#include <gadgets\stdsetup.h>

/////////////////////////////////////////////////////////////////////////////
// VideoCutRectDialog dialog

class VideoCutRectDialog : public CGadgetSetupDialog
{
	CDibRect m_View;
// Construction
public:
	VideoCutRectDialog(CGadget* pGadget, CWnd* pParent = NULL);
	void LoadFrame(const pTVFrame frame);
	void OnRectChanged(CRect* rect);

// Dialog Data
	//{{AFX_DATA(VideoCutRectDialog)
	enum { IDD = IDD_VIDEO_CUTRECT_DIALOG };
	int		m_Left;
	int		m_Top;
	int		m_Width;
	int		m_Height;
	//}}AFX_DATA


// Overrides
    virtual bool Show(CPoint point, LPCTSTR uid);
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VideoCutRectDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void UploadParams();

	// Generated message map functions
	//{{AFX_MSG(VideoCutRectDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeHeight();
	afx_msg void OnChangeLeft();
	afx_msg void OnChangeTop();
	afx_msg void OnChangeWidth();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
  BOOL m_b4PixelsStep;
  afx_msg void OnBnClicked4pixelsRounding();
  BOOL m_bAllowROIControlFromVideoInput;
  afx_msg void OnBnClickedAllowControlFromInput();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDEOCUTRECTDIALOG_H__A93B1808_39D2_478C_BAB0_ABD54B80C45A__INCLUDED_)

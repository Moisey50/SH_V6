#if !defined(AFX_TXTRENDERSETUPDIALOG_H__54D39B63_D066_488A_99B4_91C7CA638B69__INCLUDED_)
#define AFX_TXTRENDERSETUPDIALOG_H__54D39B63_D066_488A_99B4_91C7CA638B69__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TxtRenderSetupDialog.h : header file
//
#include <Gadgets\gadbase.h>
#include "Resource.h"
#include <gadgets\stdsetup.h>

/////////////////////////////////////////////////////////////////////////////
// CTxtRenderSetupDialog dialog
class TextWriter;
class CTxtRenderSetupDialog : public CGadgetSetupDialog
{
// Construction
public:
    CTxtRenderSetupDialog(CGadget* pGadget, CWnd* pParent = NULL);   // standard constructor
// Dialog Data
	//{{AFX_DATA(CTxtRenderSetupDialog)
	enum { IDD = IDD_TXT_RENDER_DLG };
	BOOL	m_bOverwrite;
  BOOL  m_bLogMode ;
	CString	m_Filename;
	BOOL	m_WriteID;
	//}}AFX_DATA


// Overrides
    virtual bool Show(CPoint point, LPCTSTR uid);
    virtual void UploadParams();
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTxtRenderSetupDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTxtRenderSetupDialog)
	afx_msg void OnCloseFile();
	afx_msg void OnBrowseFilename();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnBnClickedWriteId();
  BOOL m_bWriteFigureAsText;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TXTRENDERSETUPDIALOG_H__54D39B63_D066_488A_99B4_91C7CA638B69__INCLUDED_)

#if !defined(AFX_LOGPP_H__9172BC34_90B5_431C_927D_B35B0F4D1407__INCLUDED_)
#define AFX_LOGPP_H__9172BC34_90B5_431C_927D_B35B0F4D1407__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LogPP.h : header file
//

#include <gadgets\shkernel.h>
/////////////////////////////////////////////////////////////////////////////
// CLogPP dialog
class CLogPP : public CPropertyPage
{
	DECLARE_DYNCREATE(CLogPP)
private:
    int         m_SetLogLevel;
    bool        m_WriteFile;
    FXString    m_FileName;
public:
	CLogPP();
	virtual ~CLogPP();
    void InitLogLevel(int i) { m_SetLogLevel=(i<MSG_INFO_LEVEL)?MSG_INFO_LEVEL:((i>MSG_SYSTEM_LEVEL)?MSG_SYSTEM_LEVEL:i);}
    int  GetLogLevel()       { return m_SetLogLevel; }
    void SetWriteFile(bool set);
    bool GetWriteFile() { return m_WriteFile; }
    void EnableFileNamesEdit(bool enable);
    void SetLogFileName(LPCTSTR name);
    LPCTSTR GetLogFileName() { return m_FileName; };
// Dialog Data
	//{{AFX_DATA(CLogPP)
	enum { IDD = IDD_LOGSETUP };
	CSliderCtrl	m_LogLevel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CLogPP)
	public:
	virtual BOOL OnSetActive();
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CLogPP)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnCheckWritefile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedBrowse();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGPP_H__9172BC34_90B5_431C_927D_B35B0F4D1407__INCLUDED_)

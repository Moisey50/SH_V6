#if !defined(AFX_WRKBNCHPP_H__9766D284_519E_45D7_8F0B_7A89E8B8837F__INCLUDED_)
#define AFX_WRKBNCHPP_H__9766D284_519E_45D7_8F0B_7A89E8B8837F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WrkBnchPP.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWrkBnchPP dialog

class CWrkBnchPP : public CPropertyPage
{
	DECLARE_DYNCREATE(CWrkBnchPP)
public:
	CWrkBnchPP();
	virtual ~CWrkBnchPP();

// Dialog Data
	//{{AFX_DATA(CWrkBnchPP)
	enum { IDD = IDD_WRKBNCHSETUP };
	BOOL	m_GadgetTreeShowType;
    BOOL    m_bUseMasterExecutionStatus;
    BOOL    m_SaveGadgetPositions;
    BOOL    m_SaveFltWindowsPos;
    //}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CWrkBnchPP)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CWrkBnchPP)
	afx_msg void OnNgroupbtgadgets();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WRKBNCHPP_H__9766D284_519E_45D7_8F0B_7A89E8B8837F__INCLUDED_)

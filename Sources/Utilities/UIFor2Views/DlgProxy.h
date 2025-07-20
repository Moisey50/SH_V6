
// DlgProxy.h: header file
//

#pragma once

class CUIFor2ViewsDlg;


// CUIFor2ViewsDlgAutoProxy command target

class CUIFor2ViewsDlgAutoProxy : public CCmdTarget
{
	DECLARE_DYNCREATE(CUIFor2ViewsDlgAutoProxy)

	CUIFor2ViewsDlgAutoProxy();           // protected constructor used by dynamic creation

// Attributes
public:
	CUIFor2ViewsDlg* m_pDialog;

// Operations
public:

// Overrides
	public:
	virtual void OnFinalRelease();

// Implementation
protected:
	virtual ~CUIFor2ViewsDlgAutoProxy();

	// Generated message map functions

	DECLARE_MESSAGE_MAP()
	DECLARE_OLECREATE(CUIFor2ViewsDlgAutoProxy)

	// Generated OLE dispatch map functions

	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};


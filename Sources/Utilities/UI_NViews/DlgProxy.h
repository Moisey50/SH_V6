
// DlgProxy.h: header file
//

#pragma once

class CUI_NViews;


// CUI_NViewsAutoProxy command target

class CUI_NViewsAutoProxy : public CCmdTarget
{
	DECLARE_DYNCREATE(CUI_NViewsAutoProxy)

	CUI_NViewsAutoProxy();           // protected constructor used by dynamic creation

// Attributes
public:
	CUI_NViews* m_pDialog;

// Operations
public:

// Overrides
	public:
	virtual void OnFinalRelease();

// Implementation
protected:
	virtual ~CUI_NViewsAutoProxy();

	// Generated message map functions

	DECLARE_MESSAGE_MAP()
	DECLARE_OLECREATE(CUI_NViewsAutoProxy)

	// Generated OLE dispatch map functions

	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};


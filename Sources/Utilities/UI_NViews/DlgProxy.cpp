
// DlgProxy.cpp : implementation file
//

#include "stdafx.h"
#include "framework.h"
#include "UI_NViews.h"
#include "DlgProxy.h"
#include "UI_NViewsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUI_NViewsAutoProxy

IMPLEMENT_DYNCREATE(CUI_NViewsAutoProxy, CCmdTarget)

CUI_NViewsAutoProxy::CUI_NViewsAutoProxy()
{
	EnableAutomation();

	// To keep the application running as long as an automation
	//	object is active, the constructor calls AfxOleLockApp.
	AfxOleLockApp();

	// Get access to the dialog through the application's
	//  main window pointer.  Set the proxy's internal pointer
	//  to point to the dialog, and set the dialog's back pointer to
	//  this proxy.
	ASSERT_VALID(AfxGetApp()->m_pMainWnd);
	if (AfxGetApp()->m_pMainWnd)
	{
		ASSERT_KINDOF(CUI_NViews, AfxGetApp()->m_pMainWnd);
		if (AfxGetApp()->m_pMainWnd->IsKindOf(RUNTIME_CLASS(CUI_NViews)))
		{
			m_pDialog = reinterpret_cast<CUI_NViews*>(AfxGetApp()->m_pMainWnd);
			m_pDialog->m_pAutoProxy = this;
		}
	}
}

CUI_NViewsAutoProxy::~CUI_NViewsAutoProxy()
{
	// To terminate the application when all objects created with
	// 	with automation, the destructor calls AfxOleUnlockApp.
	//  Among other things, this will destroy the main dialog
	if (m_pDialog != nullptr)
		m_pDialog->m_pAutoProxy = nullptr;
	AfxOleUnlockApp();
}

void CUI_NViewsAutoProxy::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CCmdTarget::OnFinalRelease();
}

BEGIN_MESSAGE_MAP(CUI_NViewsAutoProxy, CCmdTarget)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CUI_NViewsAutoProxy, CCmdTarget)
END_DISPATCH_MAP()

// Note: we add support for IID_IUI_NViews to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the
//  dispinterface in the .IDL file.

// {fe74e53c-481a-40a0-a402-7e9e628b8940}
static const IID IID_IUI_NViews =
{0xfe74e53c,0x481a,0x40a0,{0xa4,0x02,0x7e,0x9e,0x62,0x8b,0x89,0x40}};

BEGIN_INTERFACE_MAP(CUI_NViewsAutoProxy, CCmdTarget)
	INTERFACE_PART(CUI_NViewsAutoProxy, IID_IUI_NViews, Dispatch)
END_INTERFACE_MAP()

// The IMPLEMENT_OLECREATE2 macro is defined in pch.h of this project
// {8696a96f-4602-496e-bcc1-845efce89e34}
IMPLEMENT_OLECREATE2(CUI_NViewsAutoProxy, "UI_NViews.Application", 0x8696a96f,0x4602,0x496e,0xbc,0xc1,0x84,0x5e,0xfc,0xe8,0x9e,0x34)


// CUI_NViewsAutoProxy message handlers

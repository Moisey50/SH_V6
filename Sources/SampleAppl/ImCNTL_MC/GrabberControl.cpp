// GrabberControl.cpp : implementation file
//

#include "stdafx.h"
#include "ImCNTL.h"
#include "ImagView.h"
#include "GrabberControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGrabberControl dialog


CGrabberControl::CGrabberControl(CWnd* pParent /*=NULL*/)
	: CDialog(CGrabberControl::IDD, pParent)
{
	EnableAutomation();

	//{{AFX_DATA_INIT(CGrabberControl)
	m_Trigger = 0 ;
	m_TriggerMode = 0 ;
	m_TriggerSource = 0 ;
	//}}AFX_DATA_INIT
}


void CGrabberControl::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CDialog::OnFinalRelease();
}

void CGrabberControl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGrabberControl)
	DDX_Radio(pDX, IDC_TRIGGER, m_Trigger);
	DDX_Radio(pDX, IDC_TRIGGER_MODE, m_TriggerMode);
	DDX_Radio(pDX, IDC_TRIGGER_SOURCE, m_TriggerSource);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGrabberControl, CDialog)
	//{{AFX_MSG_MAP(CGrabberControl)
	ON_BN_CLICKED(IDC_TRIGGER, OnTrigger)
	ON_BN_CLICKED(IDC_TRIGGER2, OnTrigger)
	ON_BN_CLICKED(IDC_TRIGGER3, OnTrigger)
	ON_BN_CLICKED(IDC_TRIGGER_MODE, OnTriggerMode)
	ON_BN_CLICKED(IDC_TRIGGER_MODE2, OnTriggerMode)
	ON_BN_CLICKED(IDC_TRIGGER_MODE3, OnTriggerMode)
	ON_BN_CLICKED(IDC_TRIGGER_MODE4, OnTriggerMode)
	ON_BN_CLICKED(IDC_TRIGGER_SOURCE, OnTriggerSource)
	ON_BN_CLICKED(IDC_TRIGGER_SOURCE2, OnTriggerSource)
	ON_BN_CLICKED(IDC_TRIGGER_SOURCE3, OnTriggerSource)
	ON_BN_CLICKED(IDC_TRIGGER_SOURCE4, OnTriggerSource)
	ON_BN_CLICKED(IDC_TRIGGER_SOURCE5, OnTriggerSource)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CGrabberControl, CDialog)
	//{{AFX_DISPATCH_MAP(CGrabberControl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_IGrabberControl to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {9DDD339D-06AC-11D3-84DD-00A0C9616FBC}
static const IID IID_IGrabberControl =
{ 0x9ddd339d, 0x6ac, 0x11d3, { 0x84, 0xdd, 0x0, 0xa0, 0xc9, 0x61, 0x6f, 0xbc } };

BEGIN_INTERFACE_MAP(CGrabberControl, CDialog)
	INTERFACE_PART(CGrabberControl, IID_IGrabberControl, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGrabberControl message handlers

void CGrabberControl::OnTrigger() 
{
  UpdateData( 1 )	 ;

}

void CGrabberControl::OnTriggerMode() 
{
	// TODO: Add your control notification handler code here
	
}

void CGrabberControl::OnTriggerSource() 
{
	// TODO: Add your control notification handler code here
	
}

// OptionsPS.cpp : implementation file
//

#include "stdafx.h"
#include "tvdb400.h"
#include "OptionsPS.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsPS

IMPLEMENT_DYNAMIC(COptionsPS, CPropertySheet)

COptionsPS::COptionsPS(CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet("Options", pParentWnd, iSelectPage)
{
	AddPage(&m_LogPP);
    AddPage(&m_WrkBnchPP);
    AddPage(&m_ViewPP);
}

COptionsPS::~COptionsPS()
{
}


BEGIN_MESSAGE_MAP(COptionsPS, CPropertySheet)
	//{{AFX_MSG_MAP(COptionsPS)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsPS message handlers

// WrkBnchPP.cpp : implementation file
//

#include "stdafx.h"
#include "tvdb400.h"
#include "WrkBnchPP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWrkBnchPP property page

IMPLEMENT_DYNCREATE(CWrkBnchPP, CPropertyPage)

CWrkBnchPP::CWrkBnchPP() : CPropertyPage(CWrkBnchPP::IDD)
, m_bUseMasterExecutionStatus(FALSE)
, m_SaveGadgetPositions(FALSE)
, m_SaveFltWindowsPos(FALSE)
{
	//{{AFX_DATA_INIT(CWrkBnchPP)
	m_GadgetTreeShowType = FALSE;
	//}}AFX_DATA_INIT
}

CWrkBnchPP::~CWrkBnchPP()
{
}

void CWrkBnchPP::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CWrkBnchPP)
    DDX_Check(pDX, IDC_GROUPGADGETS, m_GadgetTreeShowType);
    DDX_Check(pDX, IDC_USEMASTEREXECUTIONSTATUS, m_bUseMasterExecutionStatus);
    DDX_Check(pDX, IDC_SAVEGDGTPOS, m_SaveGadgetPositions);
    DDX_Check(pDX, IDC_SAVEFLTWNDPOS, m_SaveFltWindowsPos);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWrkBnchPP, CPropertyPage)
	//{{AFX_MSG_MAP(CWrkBnchPP)
	ON_BN_CLICKED(IDC_GROUPGADGETS, OnNgroupbtgadgets)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWrkBnchPP message handlers

BOOL CWrkBnchPP::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	return TRUE;  
}

void CWrkBnchPP::OnNgroupbtgadgets() 
{
	m_GadgetTreeShowType=!m_GadgetTreeShowType;
    UpdateData(FALSE);
}


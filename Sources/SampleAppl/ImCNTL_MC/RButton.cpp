// RButton.cpp : implementation file
//

#include "stdafx.h"
#include "imcntl.h"
#include "RButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRButton dialog


CRButton::CRButton(CWnd* pParent /*=NULL*/)
	: CDialog(CRButton::IDD, pParent)
  , m_SpotSearchCoordinates(_T(""))
{
	//{{AFX_DATA_INIT(CRButton)
	m_iViewMode = 0;
	m_bShowMicrons = 1 ;
	m_StringScales = _T("");
	m_AutoSaveFileName = _T("c:\\ATJ");
	m_IsAutoSave = FALSE;
	//}}AFX_DATA_INIT
}


void CRButton::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRButton)
	DDX_Radio(pDX, IDC_NOVIEW, m_iViewMode);
	DDX_Check(pDX, IDC_SHOW_MICRONS, m_bShowMicrons);
	DDX_Text(pDX, IDC_SCALES, m_StringScales);
	DDX_Text(pDX, IDC_AUTOSAVE_FILE_NAME, m_AutoSaveFileName);
	DDX_Check(pDX, IDC_AUTOMATIC_SAVE, m_IsAutoSave);
	//}}AFX_DATA_MAP
  DDX_Text(pDX, IDC_SPOT_ON_VIEW_COORD, m_SpotSearchCoordinates);
}


BEGIN_MESSAGE_MAP(CRButton, CDialog)
	//{{AFX_MSG_MAP(CRButton)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
  ON_BN_CLICKED(IDC_AUTOMATIC_SAVE, &CRButton::OnBnClickedAutomaticSave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRButton message handlers

void CRButton::OnBnClickedAutomaticSave()
{
  if(m_IsAutoSave)
    m_IsAutoSave	= 0;
  else
  {
    m_IsAutoSave = 1;
    m_bWriteHeadlines = true;
  }
}

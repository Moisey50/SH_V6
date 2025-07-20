// SketchViewPP.cpp : implementation file
//

#include "stdafx.h"
#include "tvdb400.h"
#include "SketchViewPP.h"
#include "fxfc/FXRegistry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSketchViewPP property page

IMPLEMENT_DYNCREATE(CSketchViewPP, CPropertyPage)

CSketchViewPP::CSketchViewPP() : CPropertyPage(CSketchViewPP::IDD)
, m_ViewActivity(FALSE)
{
	//{{AFX_DATA_INIT(CSketchViewPP)
	m_sViewType = _T("");
	m_ShowSplash = FALSE;
	m_OffsetValue = _T("");
	//}}AFX_DATA_INIT
  FXRegistry Reg( "TheFileX\\SHStudio" ) ;
  m_ShowSplash =Reg.GetRegiInt("root","ShowSplash",1);
}

CSketchViewPP::~CSketchViewPP()
{
}

BOOL CSketchViewPP::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	return TRUE;
}

void CSketchViewPP::InitViewType(ViewType vt)
{
  if (vt.m_EvType==ViewType::EVADE_CLASSIC)
  {
    m_sViewType="Classic";
  }
  else if (vt.m_EvType==ViewType::EVADE_MODERN)
  {
    m_sViewType="Modern";
  }
  m_OffsetValue.Format("%d", vt.offset.x);;
}

ViewType CSketchViewPP::GetViewType()
{
  ViewType vt;
  CString offset;
  vt.offset.x=vt.offset.y=atoi(m_OffsetValue);
  if (m_sViewType=="Classic")
  {
    vt.m_EvType=ViewType::EVADE_CLASSIC;
    return vt;
  }
  else if (m_sViewType=="Modern")
  {
    vt.m_EvType=ViewType::EVADE_MODERN;
    return vt;
  }
  return vt;
}

void CSketchViewPP::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSketchViewPP)
    DDX_CBString(pDX, IDC_VIEWTYPE, m_sViewType);
    DDX_Check(pDX, IDC_SHOW_SPLASH, m_ShowSplash);
    DDX_CBString(pDX, IDC_OFFSET, m_OffsetValue);
    //}}AFX_DATA_MAP
    DDX_Check(pDX, IDC_VIEWACTIVITY, m_ViewActivity);
}


BEGIN_MESSAGE_MAP(CSketchViewPP, CPropertyPage)
	//{{AFX_MSG_MAP(CSketchViewPP)
	ON_BN_CLICKED(IDC_SHOW_SPLASH, OnShowSplash)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSketchViewPP message handlers

void CSketchViewPP::OnShowSplash() 
{
    UpdateData(TRUE);
    FXRegistry Reg( "TheFileX\\SHStudio" ) ;
    Reg.WriteRegiInt( "root" , "ShowSplash" , m_ShowSplash );
}

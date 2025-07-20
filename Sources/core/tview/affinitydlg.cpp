// AffinityDlg.cpp : implementation file
//

#include "stdafx.h"
#include <gadgets\gadbase.h>
#include <math\Intf_sup.h>
#include "AffinityDlg.h"


// CAffinityDlg dialog

IMPLEMENT_DYNAMIC( CAffinityDlg , CDialog )

static int Priorities[] =
{
  THREAD_PRIORITY_IDLE ,
  THREAD_PRIORITY_LOWEST ,
  THREAD_PRIORITY_BELOW_NORMAL ,
  THREAD_PRIORITY_NORMAL ,
  THREAD_PRIORITY_ABOVE_NORMAL ,
  THREAD_PRIORITY_HIGHEST ,
  THREAD_PRIORITY_TIME_CRITICAL
} ;

CAffinityDlg::CAffinityDlg(CGadget* Gadget, CWnd* pParent /*=NULL*/)
	: CDialog(CAffinityDlg::IDD, pParent),
	m_pGadget(Gadget)
{
	m_ProcMask=FxAffinityGetProcessorMask();
}

CAffinityDlg::~CAffinityDlg()
{
}

void CAffinityDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange( pDX );
  DDX_Control( pDX , IDC_PRIORITY , m_PrioritySelect );
}


BEGIN_MESSAGE_MAP(CAffinityDlg, CDialog)
	ON_EN_CHANGE(IDC_AFFINITY_EDT, &CAffinityDlg::OnEnChangeAffinityEdt)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, &CAffinityDlg::OnBnClickedOk)
  ON_LBN_SELCHANGE( IDC_PRIORITY , &CAffinityDlg::OnLbnSelchangeList1 )
END_MESSAGE_MAP()


// CAffinityDlg message handlers


BOOL CAffinityDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
  DWORD mask = m_pGadget->GetAffinity();
  CString AffAsString ;
  AffAsString.Format( "0x%08X" , mask ) ;
	GetDlgItem(IDC_AFFINITY_EDT)->SetWindowText( (LPCTSTR)AffAsString );
  m_PrioritySelect.AddString( _T( "Idle" ) ) ;
  m_PrioritySelect.AddString( _T( "Lowest" ) ) ;
  m_PrioritySelect.AddString( _T( "Below Normal" ) ) ;
  m_PrioritySelect.AddString( _T( "Normal" ) ) ;
  m_PrioritySelect.AddString( _T( "Above Normal" ) ) ;
  m_PrioritySelect.AddString( _T( "Highest" ) ) ;
  m_PrioritySelect.AddString( _T( "Time Critical" ) ) ;
  m_Priority = m_pGadget->GetPriority() ;
  int i = 0 ;
  while ( (Priorities[ i ] != m_Priority) && (i < ARRSZ( Priorities )) )
    i++ ;
  m_PrioritySelect.SetCurSel( i < ARRSZ( Priorities ) ? i : 3 ) ;
  return TRUE;
}

void CAffinityDlg::OnDestroy()
{
	CDialog::OnDestroy();
}

void CAffinityDlg::OnEnChangeAffinityEdt()
{
// 	UpdateData(TRUE);
// 	CString tmpS;
// 	CWnd* pWnd = GetDlgItem(IDC_AFFINITY_EDT);
// 	pWnd->GetWindowText(tmpS);
// 	DWORD mask=ConvToBinary((LPCTSTR)tmpS);
// 	mask &= m_ProcMask;
//   pWnd->SetWindowText(buffer);
}

void CAffinityDlg::OnBnClickedOk()
{
	bool Invalidate=false;
	UpdateData(TRUE);
	CString tmpS;
	GetDlgItem(IDC_AFFINITY_EDT)->GetWindowText(tmpS);
  DWORD mask= (DWORD) ConvToBinary((LPCTSTR)tmpS);
  mask &= m_ProcMask ;
  m_pGadget->SetAffinity(mask);
	OnOK();
}


void CAffinityDlg::OnLbnSelchangeList1()
{
  int i = m_PrioritySelect.GetCurSel() ;
  if ( i >= 0 && i < ARRSZ( Priorities ) )
    m_pGadget->SetPriority( Priorities[ i ] ) ;
}

// InsertMonitorDlg.cpp : implementation file
//

#include "stdafx.h"
#include <Gadgets\tview.h>
#include "InsertMonitorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInsertMonitorDlg dialog


CInsertMonitorDlg::CInsertMonitorDlg( LPCTSTR GadgetUID , CWnd* pParent /*=NULL*/ )
  : CDialog( CInsertMonitorDlg::IDD , pParent ) ,
  m_GadgetUID( GadgetUID )
{
  //{{AFX_DATA_INIT(CInsertMonitorDlg)
  m_InsertMethod = 0;
  m_Selected = -1;
  //}}AFX_DATA_INIT
  m_Monitors.SetSize( 0 , 1 );
}


void CInsertMonitorDlg::DoDataExchange( CDataExchange* pDX )
{
  CDialog::DoDataExchange( pDX );
  //{{AFX_DATA_MAP(CInsertMonitorDlg)
  DDX_Radio( pDX , IDC_INSERT_METHOD , m_InsertMethod );
  DDX_CBIndex( pDX , IDC_MONITORS_LIST , m_Selected );
  //}}AFX_DATA_MAP
}

int CInsertMonitorDlg::GetMonitorName( CString& name )
{
  if ( m_InsertMethod == IM_EXISTING )
    name = m_Monitors.GetAt( m_Selected );
  else
    name.Empty();
  return m_InsertMethod;
}

BEGIN_MESSAGE_MAP( CInsertMonitorDlg , CDialog )
  //{{AFX_MSG_MAP(CInsertMonitorDlg)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInsertMonitorDlg message handlers

BOOL CInsertMonitorDlg::OnInitDialog()
{
  CDialog::OnInitDialog();
  FXString wndName; 
  wndName.Format( "Insert monitor for '%s'" , m_GadgetUID );
  SetWindowText( wndName );
  CPoint pt;
  if ( GetCursorPos( &pt ) )
  {
    SetWindowPos( NULL , pt.x , pt.y , 0 , 0 , SWP_NOSIZE | SWP_NOZORDER );
  }
  if ( m_Monitors.GetSize() )
  {
    CComboBox* pBox = (CComboBox*) GetDlgItem( IDC_MONITORS_LIST );
    for ( int i = 0; i < m_Monitors.GetSize(); i++ )
      pBox->AddString( m_Monitors.GetAt( i ) );
    pBox->SelectString( -1 , m_Monitors.GetAt( 0 ) );
  }
  else
  {
    GetDlgItem( IDC_RADIO3 )->ShowWindow( SW_HIDE );
    GetDlgItem( IDC_RADIO3 )->ShowWindow( IDC_MONITORS_LIST );
  }
  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}

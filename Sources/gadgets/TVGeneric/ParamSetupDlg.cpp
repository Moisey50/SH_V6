// ParamSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "tvgeneric.h"
#include "paramsetupdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "CParamSetupDlg"

/////////////////////////////////////////////////////////////////////////////
// CParamSetupDlg dialog


CParamSetupDlg::CParamSetupDlg( CGadget* Gadget , LPCTSTR key , CWnd* pParent /*=NULL*/ )
  : CGadgetSetupDialog( Gadget , CParamSetupDlg::IDD , pParent ) ,
  m_Gadget( Gadget )
{
  //{{AFX_DATA_INIT(CParamSetupDlg)
  m_Params = _T( "" );
  //}}AFX_DATA_INIT
  m_Key = key;
}


void CParamSetupDlg::DoDataExchange( CDataExchange* pDX )
{
  CDialog::DoDataExchange( pDX );
  //{{AFX_DATA_MAP(CParamSetupDlg)
  DDX_Text( pDX , IDC_PARAMS , m_Params );
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( CParamSetupDlg , CGadgetSetupDialog )
  //{{AFX_MSG_MAP(CParamSetupDlg)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CParamSetupDlg::UploadParams()
{
  bool Invalidate = false;
  m_Params.Replace( "\r\n" , "" );
  m_Gadget->ScanProperties( m_Params , Invalidate );
}

/////////////////////////////////////////////////////////////////////////////
// CParamSetupDlg message handlers

bool comparekey( FXString& key , FXString& data )
{
  if ( key.GetLength() == 0 ) 
    return true;
  for ( FXSIZE i = 0; i < key.GetLength(); i++ )
  {
    if ( toupper( key[ i ] ) != toupper( data[ i ] ) )
    {
      return false;
    }
  }
  return true;
}

bool CParamSetupDlg::Show( CPoint point , LPCTSTR uid )
{
  FXString DlgHead;
  DlgHead.Format( "%s Setup Dialog" , uid );
  if ( !m_hWnd )
  {
    FX_UPDATERESOURCE fur( pThisDll->m_hResource );
    if ( !Create( IDD_SETUP_DLG , NULL ) )
    {
      SENDERR_0( "Failed to create Setup Dialog" );
      return false;
    }
  }
  SetWindowText( DlgHead );
  SetWindowPos( NULL , point.x , point.y , 0 , 0 , SWP_NOSIZE | SWP_NOZORDER );
  ShowWindow( SW_SHOWNORMAL );
  return true;
}

BOOL CParamSetupDlg::OnInitDialog()
{
  CGadgetSetupDialog::OnInitDialog();
  m_Params = "";
  if ( m_Gadget )
  {
    FXParser ip;
    FXString data;
    FXString tmpS;
    m_Gadget->PrintProperties( tmpS );
    ip = (LPCTSTR) tmpS;
    FXSIZE pos = 0;
    while ( ip.GetString( pos , data ) )
    {
      if ( comparekey( m_Key , data ) )
      {
        m_Params += data;
        m_Params += "\r\n";
      }
    }
    UpdateData( FALSE );
  }
  return TRUE;
}

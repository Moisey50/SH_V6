// FlwRenderStp.cpp : implementation file
//

#include "stdafx.h"
#include "flwgadget.h"
#include "FlwRenderStp.h"
#include "FLWRenderGadget.h"
#include <gadgets\stdsetup.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "FLWRenderSetupDialog"

/////////////////////////////////////////////////////////////////////////////
// FLWRenderStp dialog

FLWRenderStp::FLWRenderStp( CGadget* pGadget , CWnd* pParent ) :
  CGadgetSetupDialog( pGadget , FLWRenderStp::IDD , pParent )
  , m_bDoLog( FALSE )
  , m_NPreallocatedBuffers( 0 )
  , m_BuffSize_KB( 0 )
{
  //{{AFX_DATA_INIT(FLWRenderStp)
  m_FileName = _T( "" );
  //}}AFX_DATA_INIT
  FXString text;
  pGadget->PrintProperties( text );
  FXPropertyKit pk( text );
  FXString tmpS;
  if ( pk.GetString( "filename" , tmpS ) )
    m_FileName = tmpS;
  pk.GetInt( "DoLog" , m_bDoLog ) ;
}

void FLWRenderStp::DoDataExchange( CDataExchange* pDX )
{
  CDialog::DoDataExchange( pDX );
  //{{AFX_DATA_MAP(FLWRenderStp)
  DDX_Text( pDX , IDC_FILENAME , m_FileName );
  //}}AFX_DATA_MAP
  DDX_Check( pDX , IDC_DO_LOG , m_bDoLog );
  DDX_Text( pDX , IDC_N_PREALLOCATED_BUFFERS , m_NPreallocatedBuffers );
  DDX_Text( pDX , IDC_BUF_SZ_KB , m_BuffSize_KB );
}


BEGIN_MESSAGE_MAP( FLWRenderStp , CDialog )
  //{{AFX_MSG_MAP(FLWRenderStp)
  ON_BN_CLICKED( IDC_BROWSE_FILENAME , OnBrowseFilename )
  ON_BN_CLICKED( IDC_CLOSE_FILE , OnCloseFile )
  ON_WM_TIMER()
  ON_WM_DESTROY()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FLWRenderStp message handlers

bool FLWRenderStp::Show( CPoint point , LPCTSTR uid )
{
  FXString DlgHead;
  DlgHead.Format( "%s Setup Dialog" , uid );
  if ( !m_hWnd )
  {
    FX_UPDATERESOURCE fur( pThisDll->m_hResource );
    if ( !Create( IDD_FLW_RENDER_DLG , NULL ) )
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

BOOL FLWRenderStp::OnInitDialog()
{
  CDialog::OnInitDialog();
  m_Timer = SetTimer( FLWRenderStp::IDD , 100 , NULL );
  FXString text;

  m_pGadget->PrintProperties( text );
  FXPropertyKit pk( text );
  FXString tmpS;
  if ( pk.GetString( "filename" , tmpS ) )
    m_FileName = tmpS;
  pk.GetInt( "DoLog" , m_bDoLog ) ;
  pk.GetInt( "NBuffers" , m_NPreallocatedBuffers ) ;
  pk.GetInt( "BufferSize_KB" , m_BuffSize_KB ) ;

  UpdateData( FALSE );
  return TRUE;
}

void FLWRenderStp::OnDestroy()
{
  if ( m_Timer )
    KillTimer( m_Timer );
  CDialog::OnDestroy();
}

void FLWRenderStp::UploadParams()
{
  FXPropertyKit pk;
  bool Invalidate = false;
  pk.WriteString( "filename" , m_FileName );
  pk.WriteInt( "DoLog" , m_bDoLog ) ;
  pk.WriteInt( "NBuffers" , m_NPreallocatedBuffers ) ;
  pk.WriteInt( "BufferSize_KB" , m_BuffSize_KB ) ;
  m_pGadget->ScanProperties( pk , Invalidate );
  CGadgetSetupDialog::UploadParams();
}

void FLWRenderStp::OnBrowseFilename()
{
  CFileDialog fd( FALSE , "flw" , m_FileName , OFN_FILEMUSTEXIST , 
    "FLW files (*.flw)|*.flw|all files (*.*)|*.*||" , this );
  if ( fd.DoModal() == IDOK )
  {
    m_FileName = fd.GetPathName();
    GetDlgItem( IDC_FILENAME )->SetWindowText( m_FileName );
  }
}

void FLWRenderStp::OnCloseFile()
{
  FLWRender* Gadget = (FLWRender*) m_pGadget;
  if ( Gadget->IsFileOpen() )
    Gadget->CloseFile();
}

void FLWRenderStp::OnTimer( UINT_PTR nIDEvent )
{
  if ( nIDEvent == FLWRenderStp::IDD )
  {
    GetDlgItem( IDC_CLOSE_FILE )->EnableWindow( ((FLWRender*) m_pGadget)->IsFileOpen() );
  }
  CDialog::OnTimer( nIDEvent );
}


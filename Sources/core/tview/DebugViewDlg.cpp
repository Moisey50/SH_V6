// DebugViewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DebugViewDlg.h"
#include <gadgets\TextFrame.h>
#include <Gadgets\VideoFrame.h>
#include <gadgets\QuantityFrame.h>
#include <gadgets\RectFrame.h>
#include "fxfc/FXRegistry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDebugViewDlg dialog

CDebugViewDlg::CDebugViewDlg( CWnd* pParent /*=NULL*/ )
  : CDialog( CDebugViewDlg::IDD , pParent ) ,
  m_VideoView( NULL ) ,
  m_ContainerView( NULL )
{
  //{{AFX_DATA_INIT(CDebugViewDlg)
    // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
}


void CDebugViewDlg::DoDataExchange( CDataExchange* pDX )
{
  CDialog::DoDataExchange( pDX );
  //{{AFX_DATA_MAP(CDebugViewDlg)
    // NOTE: the ClassWizard will add DDX and DDV calls here
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( CDebugViewDlg , CDialog )
  //{{AFX_MSG_MAP(CDebugViewDlg)
  ON_WM_DESTROY()
  ON_WM_SIZE()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDebugViewDlg message handlers

BOOL CDebugViewDlg::Create( CWnd* pParentWnd )
{
  if ( !CDialog::Create( CDebugViewDlg::IDD , pParentWnd ) ) return false;
  m_ContainerView = new CContainerView();
  m_ContainerView->Create( GetDlgItem( IDC_CONTAINERVIEW ) );
  m_VideoView = new CDIBRender;
  m_VideoView->Create( GetDlgItem( IDC_VIDEO_FRM ) );
  m_VideoView->SetScale( -1 );
  { // Load last window position
//     WINDOWPLACEMENT* wp = NULL;
//     UINT wpSize;
//     AfxGetApp()->GetProfileBinary( "wndpos" , UID_DEBUG_RENDER , ( LPBYTE* ) &wp , &wpSize );
//     if ( wp && ( wpSize == sizeof( WINDOWPLACEMENT ) ) )
//     {
//       SetWindowPlacement( wp );
//     }
//     if ( wp ) delete wp;
    WINDOWPLACEMENT wp ;
    DWORD wpSize = sizeof( wp );
    FXRegistry Reg( "TheFileX\\SHStudio" ) ;
    if ( Reg.GetRegiBinary( "wndpos" , UID_DEBUG_RENDER , ( LPBYTE ) &wp , &wpSize )
      && ( wpSize == sizeof( WINDOWPLACEMENT ) ) )
    {
      SetWindowPlacement( &wp );
    }
  }
  return TRUE;
}

void CDebugViewDlg::OnDestroy()
{
  if ( m_VideoView )
  {
    m_VideoView->DestroyWindow();
    delete m_VideoView ;
    m_VideoView = NULL;
  }
  if ( m_ContainerView )
  {
    m_ContainerView->DestroyWindow();
    delete m_ContainerView ;
    m_ContainerView = NULL;
  }
  { /// Save window position
    WINDOWPLACEMENT wp;
    GetWindowPlacement( &wp );
    FXRegistry Reg( "TheFileX\\SHStudio" ) ;
    Reg.WriteRegiBinary( "wndpos" , UID_DEBUG_RENDER , ( LPBYTE ) &wp , sizeof( wp ) );
    CDialog::OnDestroy();
  }
}

CRenderGadget* CDebugViewDlg::GetGadget( IGraphbuilder* Builder )
{
  CRenderGadget* rg = ( CRenderGadget* ) Builder->GetGadget( UID_DEBUG_RENDER );
  return rg;
}

void CDebugViewDlg::Render( const CDataFrame* pDataFrame )
{
    //Output to terminal wnd...
  m_ContainerView->Render( pDataFrame );
  pDataFrame = m_ContainerView->CutUnselected( pDataFrame );
  m_VideoView->Render( pDataFrame );
}


void CDebugViewDlg::OnSize( UINT nType , int cx , int cy )
{
  CDialog::OnSize( nType , cx , cy );
  if ( m_ContainerView->GetSafeHwnd() && m_VideoView->GetSafeHwnd() )
  {
    int gap = 10;
    CRect rc;
    GetClientRect( rc );
    CRect vv_rc;
    vv_rc.left = gap;
    vv_rc.right = rc.right - gap; vv_rc.right = ( vv_rc.right < vv_rc.left ) ? vv_rc.left : vv_rc.right;
    vv_rc.bottom = rc.bottom - gap;
    vv_rc.top = vv_rc.bottom - 3 * ( rc.right - 2 * gap ) / 4;
    MoveView( IDC_VIDEO_FRM , vv_rc );
    CRect cv_rc;
    cv_rc.left = gap;
    cv_rc.right = rc.right - gap; cv_rc.right = ( cv_rc.right < cv_rc.left ) ? cv_rc.left : cv_rc.right;
    cv_rc.top = gap;
    cv_rc.bottom = vv_rc.top - gap;
    MoveView( IDC_CONTAINERVIEW , cv_rc );
  }
}

void CDebugViewDlg::MoveView( int ID , LPRECT rc )
{
  GetDlgItem( ID )->MoveWindow( rc );
  switch ( ID )
  {
    case IDC_VIDEO_FRM:
    m_VideoView->MoveWindow( 0 , 0 , rc->right - rc->left , rc->bottom - rc->top );
    break;
    case IDC_CONTAINERVIEW:
    m_ContainerView->MoveWindow( 0 , 0 , rc->right - rc->left , rc->bottom - rc->top );
    break;
  }
}

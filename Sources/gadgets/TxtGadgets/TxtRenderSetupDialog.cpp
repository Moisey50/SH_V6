// TxtRenderSetupDialog.cpp : implementation file
//

#include "stdafx.h"
#include "txtgadgets.h"
#include "TxtRenderSetupDialog.h"
#include "TextGadgetsImpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef THIS_MODULENAME
#undef THIS_MODULENAME
#endif

#define THIS_MODULENAME "CTxtRenderSetupDialog"

/////////////////////////////////////////////////////////////////////////////
// CTxtRenderSetupDialog dialog

CTxtRenderSetupDialog::CTxtRenderSetupDialog( CGadget* pGadget , CWnd* pParent ) :
CGadgetSetupDialog( pGadget , CTxtRenderSetupDialog::IDD , pParent )
{
  //{{AFX_DATA_INIT(CTxtRenderSetupDialog)
  TextWriter* Gadget = ( TextWriter* )pGadget;
  m_bOverwrite = Gadget->m_bOverwrite;
  m_bLogMode = Gadget->m_bLogMode ;
  m_Filename = Gadget->m_FileTemplate;
  m_WriteID = Gadget->m_WriteSync;
  m_bWriteFigureAsText = Gadget->m_bWriteFigureAsText ? 1 : 0 ;
  //}}AFX_DATA_INIT
}


void CTxtRenderSetupDialog::DoDataExchange( CDataExchange* pDX )
{
  CGadgetSetupDialog::DoDataExchange( pDX );
  //{{AFX_DATA_MAP(CTxtRenderSetupDialog)
  DDX_Check( pDX , IDC_OVERWRITE , m_bOverwrite );
  DDX_Check( pDX , IDC_LOG_MODE , m_bLogMode );
  DDX_Text( pDX , IDC_FILENAME , m_Filename );
  DDX_Check( pDX , IDC_WRITE_ID , m_WriteID );
  //}}AFX_DATA_MAP
  DDX_Check( pDX , IDC_WRITE_FIGURE_AS_TEXT , m_bWriteFigureAsText );
}


BEGIN_MESSAGE_MAP( CTxtRenderSetupDialog , CGadgetSetupDialog )
  //{{AFX_MSG_MAP(CTxtRenderSetupDialog)
  ON_BN_CLICKED( IDC_CLOSE_FILE , OnCloseFile )
  ON_BN_CLICKED( IDC_BROWSE_FILENAME , OnBrowseFilename )
  //}}AFX_MSG_MAP
  //	ON_BN_CLICKED(IDC_WRITE_ID, &CTxtRenderSetupDialog::OnBnClickedWriteId)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTxtRenderSetupDialog message handlers
bool CTxtRenderSetupDialog::Show( CPoint point , LPCTSTR uid )
{
  FXString DlgHead;
  DlgHead.Format( "%s Setup Dialog" , uid );
  if ( !m_hWnd )
  {
    FX_UPDATERESOURCE fur( pThisDll->m_hResource );
    if ( !Create( IDD_TXT_RENDER_DLG , NULL ) )
    {
      SENDERR_0( "Failed to create Setup Dialog" );
      return false;
    }
  }
  SetWindowText( DlgHead );
  SetWindowPos( NULL , point.x , point.y , 0 , 0 , SWP_NOSIZE | SWP_NOZORDER );
  ShowWindow( SW_SHOWNORMAL );
  TextWriter* Gadget = ( TextWriter* )m_pGadget;
  m_Filename = Gadget->m_FileTemplate;
  m_bOverwrite = Gadget->m_bOverwrite;
  m_bLogMode = Gadget->m_bLogMode ;
  m_WriteID = Gadget->m_WriteSync;
  m_bWriteFigureAsText = Gadget->m_bWriteFigureAsText ? 1 : 0 ;
  UpdateData( FALSE );
  return true;
}
void CTxtRenderSetupDialog::UploadParams()
{
  TextWriter* Gadget = ( TextWriter* )m_pGadget;
  if ( m_bOverwrite || ( Gadget->m_FileTemplate != ( LPCTSTR )m_Filename ) )
    Gadget->CloseFile();
  FXString FileName ;
  FileName.Format( "File=%s;" , (LPCTSTR) m_Filename ) ;
  bool Invalidate = false ;
  Gadget->ScanProperties( FileName , Invalidate ) ;
  Gadget->m_FileTemplate = ( LPCTSTR )m_Filename;
  Gadget->m_bOverwrite = ( m_bOverwrite != FALSE );
  Gadget->m_bLogMode = ( m_bLogMode != FALSE ) ;
  Gadget->m_WriteSync = ( m_WriteID != FALSE );
  Gadget->m_bWriteFigureAsText = (m_bWriteFigureAsText != FALSE) ;
  CGadgetSetupDialog::UploadParams();
}

void CTxtRenderSetupDialog::OnCloseFile()
{
  TextWriter* Gadget = ( TextWriter* )m_pGadget;
  Gadget->CloseFile();
}

void CTxtRenderSetupDialog::OnBrowseFilename()
{
  CFileDialog fd( FALSE , "txt" , m_Filename , OFN_HIDEREADONLY , "Text files (*.txt)|*.txt|all files (*.*)|*.*||" , this );
  if ( fd.DoModal() == IDOK )
  {
    m_Filename = fd.GetPathName();
    GetDlgItem( IDC_FILENAME )->SetWindowText( m_Filename );
  }
}


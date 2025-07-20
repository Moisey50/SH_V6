// UndoDialog.cpp : implementation file
//

#include "stdafx.h"
#include "tvdb400.h"
#include "UndoDialog.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUndoDialog dialog


CUndoDialog::CUndoDialog( CWnd* pParent /*=NULL*/ )
  : CDialog( CUndoDialog::IDD , pParent ) ,
  m_IView( NULL ) ,
  m_iCurrent( -1 )
{
  //{{AFX_DATA_INIT(CUndoDialog)
    // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
  m_UndoStates.Create( IDB_UNDO_STATES , 16 , 1 , RGB( 0 , 128 , 128 ) );
}


void CUndoDialog::DoDataExchange( CDataExchange* pDX )
{
  CDialog::DoDataExchange( pDX );
  //{{AFX_DATA_MAP(CUndoDialog)
    // NOTE: the ClassWizard will add DDX and DDV calls here
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP( CUndoDialog , CDialog )
  //{{AFX_MSG_MAP(CUndoDialog)
  ON_NOTIFY( LVN_ITEMCHANGED , IDC_UNDO_LIST , OnItemchangedUndoList )
  ON_WM_SIZE()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CUndoDialog::ReloadCommands()
{
  m_iCurrent = -1;
  CListCtrl* List = (CListCtrl*) GetDlgItem( IDC_UNDO_LIST );
  List->DeleteAllItems();

  CStringArray cmds;
  int iCurrent = m_IView->CanRedo( &cmds );
  while ( cmds.GetSize() )
  {
    int i = (int)cmds.GetSize() - 1;
    int j = List->GetItemCount();
    List->InsertItem( List->GetItemCount() , "Redo:" , 1 );
    List->SetItemText( List->GetItemCount() - 1 , 1 , cmds.GetAt( i ) );
    cmds.RemoveAt( i );
  }
  //List->InsertItem(List->GetItemCount(), "  ", 2);
  m_IView->CanUndo( &cmds );
  while ( cmds.GetSize() )
  {
    int i = (int)cmds.GetSize() - 1;
    int j = List->GetItemCount();
    List->InsertItem( List->GetItemCount() , "Undo:" , 0 );
    List->SetItemText( List->GetItemCount() - 1 , 1 , cmds.GetAt( i ) );
    cmds.RemoveAt( i );
  }
  m_iCurrent = iCurrent;
  List->EnsureVisible( m_iCurrent , FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// CUndoDialog message handlers

BOOL CUndoDialog::OnInitDialog()
{
  CDialog::OnInitDialog();

  ASSERT( m_IView );

  CMainFrame* MainFrame = (CMainFrame*)::AfxGetMainWnd();
  CRect rc;
  MainFrame->GetBarButtonRect( ID_VIEW_UNDOREDOMANAGER , rc );
  CPoint pt = rc.BottomRight();
  //	GetCursorPos(&pt);
  SetWindowPos( NULL , pt.x , pt.y , 0 , 0 , SWP_NOZORDER | SWP_NOSIZE );

  CListCtrl* List = (CListCtrl*) GetDlgItem( IDC_UNDO_LIST );
  //	CRect rc;
  List->GetClientRect( rc );
  List->InsertColumn( 0 , "Undo/redo" , LVCFMT_LEFT , 70 );
  List->InsertColumn( 1 , "Commands" , LVCFMT_LEFT , rc.Width() - 90 );
  List->SetExtendedStyle( LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
  List->SetImageList( &m_UndoStates , LVSIL_SMALL );

  ReloadCommands();

  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}

void CUndoDialog::OnItemchangedUndoList( NMHDR* pNMHDR , LRESULT* pResult )
{
  NM_LISTVIEW* pNMListView = (NM_LISTVIEW*) pNMHDR;
  if ( m_iCurrent > -1 )
  {
    CListCtrl* List = (CListCtrl*) GetDlgItem( IDC_UNDO_LIST );
    POSITION pos = List->GetFirstSelectedItemPosition();
    int iSelected = List->GetNextSelectedItem( pos );
    if ( (iSelected != m_iCurrent) && (iSelected >= 0) && (iSelected < List->GetItemCount()) )
    {
      m_IView->Undo( iSelected - m_iCurrent );
      ReloadCommands();
    }
  }
  *pResult = 0;
}


void CUndoDialog::OnSize( UINT nType , int cx , int cy )
{
  CDialog::OnSize( nType , cx , cy );
  CWnd* List = GetDlgItem( IDC_UNDO_LIST );
  if ( List )
  {
    CRect rc;
    GetClientRect( rc );
    List->MoveWindow( rc );
    CHeaderCtrl* Header = ((CListCtrl*) List)->GetHeaderCtrl();
    HDITEM hItem;
    hItem.mask = HDI_WIDTH;
    Header->GetItem( 0 , &hItem );
    hItem.cxy = rc.Width() - hItem.cxy - 20;
    Header->SetItem( 1 , &hItem );
  }
}

// GadgetsDockbar.cpp : implementation file
//

#include "stdafx.h"
#include "SketchView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BOOL ComplexNameToClassAndLineage( CString& name , CString& Class , CString& Lineage )
{
  Lineage.Empty();
  int pos = name.ReverseFind( '.' );
  if ( pos < 0 )
    return FALSE;
  Class = name.Left( pos ); // cut extension
  pos = Class.ReverseFind( '.' );
  if ( pos >= 0 ) // lineage
  {
    Lineage = Class.Left( pos );
    Class = Class.Mid( pos + 1 );
  }
  if ( Class.IsEmpty() )
    return FALSE;
  //CString start("C");
  CString start( "" );
  start += Class.GetAt( 0 );
  start.MakeUpper();
  CString end( "" );
  if ( Class.GetLength() > 1 )
    end = Class.Mid( 1 );
  end.MakeLower();
  Class = start + end;
  Lineage.MakeLower();
  pos = 0;
  while ( pos < Lineage.GetLength() )
  {
    CString frst = Lineage.GetAt( pos );
    frst.MakeUpper();
    Lineage.SetAt( pos , frst.GetAt( 0 ) );
    pos = Lineage.Find( '.' , pos ) + 1;
    if ( !pos )
      break;
  }
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CGadgetsDockbar

CGadgetsDockbar::CGadgetsDockbar() :
  m_pDlg( NULL ) ,
  m_pLibDlg( NULL )
{}

CGadgetsDockbar::~CGadgetsDockbar()
{}

CGadgetsTree* CGadgetsDockbar::GetTree()
{
  if ( !m_pDlg )
    return NULL;
  return &m_pDlg->m_GadgetsTree;
}

CGadgetsTree* CGadgetsDockbar::GetLibTree()
{
  if ( !m_pLibDlg )
    return NULL;
  return &m_pLibDlg->m_GadgetsTree;
}

BEGIN_MESSAGE_MAP( CGadgetsDockbar , CWorkspaceBar )
  //{{AFX_MSG_MAP(CGadgetsDockbar)
  ON_WM_SIZE()
  ON_NOTIFY( TCN_SELCHANGE , IDC_GADGETS_TABCTRL , OnTabSelChange )
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CGadgetsDockbar::Create( CWnd* pParent , LPCTSTR title , DWORD dwStyle , UINT uID )
{
  if ( !CWorkspaceBar::Create( pParent , title , dwStyle , uID ) )
    return FALSE;
  CRect rect;
  GetClientRect( rect );
  //m_pSite = m_pDlg;
  m_Tab.Create( WS_CHILD | WS_VISIBLE | TCS_FOCUSNEVER | TCS_RIGHT , rect , this , IDC_GADGETS_TABCTRL );
  m_Tab.InsertItem( TAB_GADGETSTREE , "GadgetsTree" );
  m_Tab.InsertItem( TAB_USERLIBRARY , "UserLibrary" );
  CFont* Font = GetFont();
  m_Tab.SetFont( Font );
  m_pSite = &m_Tab;
  m_pDlg = new CGadgetsTreeDlg( this );
  m_pDlg->Create( IDD_GADGETS_TREE_DLG , this );
  m_pLibDlg = new CGadgetsTreeDlg( this , TRUE );
  m_pLibDlg->Create( IDD_GADGETS_TREE_DLG , this );
  m_pLibDlg->ShowWindow( SW_HIDE );

  return TRUE;
}

void CGadgetsDockbar::PopulateTree( IGraphbuilder* pBuilder , BOOL bDeleteExisting )
{
  //	ASSERT(pBuilder);
  CGadgetsTree* pTree = GetTree();
  if ( pTree && bDeleteExisting )
    pTree->DeleteAllItems();
  if ( pBuilder )
  {
    CUIntArray Types;
    CStringArray Classes , Lineages;
    pBuilder->EnumGadgetClassesAndLineages( Types , Classes , Lineages );
    for ( int i = 0; i < Types.GetSize(); i++ )
    {
      if (
        (Lineages.GetAt( i ) != LINEAGE_DEBUG) &&
        (Lineages.GetAt( i ) != LINEAGE_COMPLEX) &&
        (Lineages.GetAt( i ) != LINEAGE_VIRTUAL)
        )
        AddGadgetToTree( Types.GetAt( i ) , Classes.GetAt( i ) , Lineages.GetAt( i ) );
    }
  }
}

void CGadgetsDockbar::PopulateUserLibrary( LPCTSTR path , LPCTSTR ext )
{
  CGadgetsTree* pTree = GetLibTree();
  if ( pTree )
  {
    pTree->DeleteAllItems();
    m_ComplexPaths.RemoveAll();
  }
  if ( !path )
    return;
  CString templ;
  templ.Format( "%s\\*.%s" , path , ext );
  CFileFind ff;
  BOOL bFound = ff.FindFile( templ );
  CString Class , Lineage;
  if ( !bFound )
  {
    TRACE( "!!! Warrning! TvUser directory is empty!:Error message %s\n" , FxLastErr2Mes() );
  }
  else
  {
    while ( bFound )
    {
      bFound = ff.FindNextFile();
      CString name = ff.GetFileName();
      if ( ComplexNameToClassAndLineage( name , Class , Lineage ) )
      {
        HTREEITEM hItem = pTree->InsertItem( CGadgetsTree::GTI_OTHER , Lineage , Class );
        if ( hItem )
        {
          INT_PTR index = m_ComplexPaths.Add( ff.GetFilePath() );
          DWORD_PTR ptr = (DWORD_PTR) (LPCTSTR( m_ComplexPaths.GetAt( index ) ));
          pTree->SetItemData( hItem , ptr );
        }
      }
    }
    Invalidate( TRUE );
  }
}

void CGadgetsDockbar::AddGadgetToTree( UINT type , LPCTSTR Class , LPCTSTR Lineage )
{
  UINT root = CGadgetsTree::GTI_OTHER;
  if ( type == IGraphbuilder::TVDB400_GT_CAPTURE )
    root = CGadgetsTree::GTI_CAPTURES;
  else if ( type == IGraphbuilder::TVDB400_GT_FILTER )
    root = CGadgetsTree::GTI_FILTERS;
  else if ( type == IGraphbuilder::TVDB400_GT_CTRL )
    root = CGadgetsTree::GTI_CTRLS;
  CGadgetsTree* pTree = GetTree();
  if ( pTree )
    pTree->InsertItem( root , Lineage , Class );
}

void CGadgetsDockbar::SetShowType( BOOL bShow , IGraphbuilder* pBuilder )
{
  CGadgetsTree* pTree = GetTree();
  if ( pTree )
  {
    pTree->DeleteAllItems();
    pTree->ShowGroupByMedia( bShow != FALSE );
    pTree->CreateRootItems();
  }
  if ( pBuilder )
    PopulateTree( pBuilder , FALSE );
  Invalidate( TRUE );
}

void CGadgetsDockbar::OnSize( UINT nType , int cx , int cy )
{
  CWorkspaceBar::OnSize( nType , cx , cy );
  if ( ::IsWindow( m_Tab.GetSafeHwnd() ) )
  {
    CRect rc;
    m_Tab.GetClientRect( rc );
    m_Tab.AdjustRect( FALSE , rc );
    if ( m_pDlg )
      m_pDlg->MoveWindow( rc , m_pDlg->IsWindowVisible() );
    if ( m_pLibDlg )
      m_pLibDlg->MoveWindow( rc , m_pDlg->IsWindowVisible() );
  }
}

void CGadgetsDockbar::OnTabSelChange( NMHDR* pNMHDR , LRESULT* pResult )
{
  switch ( m_Tab.GetCurSel() )
  {
    case TAB_GADGETSTREE:
      if ( m_pLibDlg )
        m_pLibDlg->ShowWindow( SW_HIDE );
      if ( m_pDlg )
        m_pDlg->ShowWindow( SW_SHOW );
      break;
    case TAB_USERLIBRARY:
      if ( m_pDlg )
        m_pDlg->ShowWindow( SW_HIDE );
      if ( m_pLibDlg )
        m_pLibDlg->ShowWindow( SW_SHOW );
      break;
  }
}

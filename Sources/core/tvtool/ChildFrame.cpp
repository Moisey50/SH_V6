// ChildFrame.cpp : implementation file
//

#include "stdafx.h"
#include "tvdb400.h"
#include "childframe.h"
#include "mainfrm.h"
#include <string>
#include <vector>
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream

#include <classes/drect.h>
#include <classes/dpoint.h>
#include <gadgets\shkernel.h>
#include "undodialog.h"
#include "userinterface\aggregatedlg.h"
#include <gadgets\stdsetup.h>
#include <fxfc/fxregistry.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "Tvdb400.ChildFrame"

using namespace std ;

static UINT WM_SaveRootGraph = RegisterWindowMessage( "SaveSHRootGraph" ) ;

void GuessGraphName( FXString& graphName )
{
  do
  {
    if ( !FxGetAppPath( graphName ) )
      break;
    graphName.TrimRight( "/\\" );
    FXSIZE posSlash = graphName.ReverseFind( '/' );
    FXSIZE posBSlash = graphName.ReverseFind( '\\' );
    FXSIZE pos = (posSlash > posBSlash) ? posSlash : posBSlash;
    if ( pos < 0 )
      break;
    graphName = graphName.Left( pos );
    graphName += _T( "\\Graphs\\*.tvg" );
    CFileFind ff;
    if ( !ff.FindFile( graphName ) || !ff.FindNextFile() )
      break;
    graphName = ff.GetFilePath();
    return;
  } while ( false );
  graphName.Empty();
}

LPCTSTR GetGraphTitle( FXString& graphName )
{
  FXSIZE posSlash = graphName.ReverseFind( '/' );
  FXSIZE posBSlash = graphName.ReverseFind( '\\' );
  FXSIZE pos = (posSlash > posBSlash) ? posSlash : posBSlash;
  return (LPCTSTR) graphName + pos + 1;
}

BOOL EnsurePath( CString& path )
{
  CFileStatus fs;
  if ( CFile::GetStatus( path , fs ) && ((fs.m_attribute & CFile::directory) == CFile::directory) )
    return TRUE;
  return ::CreateDirectory( path , NULL );
}

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE( CChildFrame , CMDIChildWnd )

CChildFrame::CChildFrame() :
  m_pBuilder( NULL ) ,
  m_pWndView( NULL ) ,
  m_bLongTitle( FALSE ) ,
  m_GraphName( "new_graph.tvg" )
{}

CChildFrame::~CChildFrame()
{}


BEGIN_MESSAGE_MAP( CChildFrame , CMDIChildWnd )
  //{{AFX_MSG_MAP(CChildFrame)
  ON_WM_CREATE()
  ON_WM_DESTROY()
  ON_WM_SETFOCUS()
  ON_COMMAND( ID_FILE_SAVEGRAPH , OnFileSaveGraph )
  ON_UPDATE_COMMAND_UI( ID_FILE_SAVEGRAPH , OnUpdateFileSaveGraph )
  //ON_COMMAND(ID_OPERATE_START, OnOperateStart)
  //ON_UPDATE_COMMAND_UI(ID_OPERATE_START, OnUpdateOperateStart)
  //ON_COMMAND(ID_OPERATE_STOP, OnOperateStop)
  //ON_UPDATE_COMMAND_UI(ID_OPERATE_STOP, OnUpdateOperateStop)
  //ON_COMMAND(ID_OPERATE_PAUSE, OnOperatePause)
  //ON_UPDATE_COMMAND_UI(ID_OPERATE_PAUSE, OnUpdateOperatePause)
  //ON_COMMAND(ID_OPERATE_STEPFORWARD, OnOperateStepForward)
  //ON_UPDATE_COMMAND_UI(ID_OPERATE_STEPFORWARD, OnUpdateOperateStepForward)
  ON_COMMAND( ID_OPERATE_STARTSTOP , &CChildFrame::OnOperateStartstop )
  ON_COMMAND( ID_EDIT_DELETE , OnEditDelete )
  ON_COMMAND( ID_EDIT_CLEARALL , OnEditClearAll )
  ON_COMMAND( ID_EDIT_AGGREGATE , OnEditAggregate )
  ON_COMMAND( ID_EDIT_EXPAND , OnEditExpand )
  ON_WM_SETCURSOR()
  ON_COMMAND( ID_EDIT_SELECTALL , OnEditSelectAll )
  ON_COMMAND( IDC_SETTINGS , OnSettings )
  ON_WM_MOUSEWHEEL()
  ON_COMMAND( ID_EDIT_UNDO , OnEditUndo )
  ON_UPDATE_COMMAND_UI( ID_EDIT_UNDO , OnUpdateEditUndo )
  ON_COMMAND( ID_EDIT_REDO , OnEditRedo )
  ON_UPDATE_COMMAND_UI( ID_EDIT_REDO , OnUpdateEditRedo )
  ON_COMMAND( ID_VIEW_UNDOREDOMANAGER , OnViewUndoRedoManager )
  ON_UPDATE_COMMAND_UI( ID_VIEW_UNDOREDOMANAGER , OnUpdateViewUndoRedoManager )
  ON_COMMAND( ID_OPERATE_DEBUG , OnOperateDebug )
  ON_UPDATE_COMMAND_UI( ID_OPERATE_DEBUG , OnUpdateOperateDebug )
  ON_COMMAND( ID_EDIT_COPY , OnEditCopy )
  ON_UPDATE_COMMAND_UI( ID_EDIT_COPY , OnUpdateEditCopy )
  ON_COMMAND( ID_EDIT_PASTE , OnEditPaste )
  ON_UPDATE_COMMAND_UI( ID_EDIT_PASTE , OnUpdateEditPaste )
  ON_WM_HELPINFO()
  //}}AFX_MSG_MAP
  ON_COMMAND( ID_VIEW_PERFORMANCEDIALOG2 , &CChildFrame::OnViewPerformancedialog2 )
  ON_UPDATE_COMMAND_UI( ID_VIEW_PERFORMANCEDIALOG2 , &CChildFrame::OnUpdateViewPerformancedialog2 )
  ON_WM_SIZE()
  ON_COMMAND( ID_EDIT_RESETAFFINITY , &CChildFrame::OnEditResetaffinity )
  ON_UPDATE_COMMAND_UI( ID_EDIT_RESETAFFINITY , &CChildFrame::OnUpdateEditResetaffinity )
  ON_WM_MOVE()
  ON_COMMAND( ID_USEDGADGETS_VIEW , &CChildFrame::OnUsedgadgetsView )
  ON_COMMAND( ID_USEDGADGETS_FORMLIST , &CChildFrame::OnUsedgadgetsFormlist )
  ON_COMMAND( ID_FORM_COPY_BATCH_FILE , &CChildFrame::OnFormActiveExeAndDLLCopyBatch )
END_MESSAGE_MAP()

void CChildFrame::SetBuilder( IGraphbuilder* pBuilder , CExecutionStatus* pES )
{
  if ( m_pBuilder )
    m_pBuilder->Release();
  m_pBuilder = pBuilder;
  if ( !m_pBuilder )
  {
    m_pBuilder = Tvdb400_CreateBuilder( pES );
    m_pBuilder->SetMsgQueue( FxGetGraphMsgQueue( ::PrintMsg ) );
    m_pBuilder->GetPluginLoader()->RegisterPlugins( m_pBuilder );
    m_pBuilder->ViewSectionSetConfig( ShConfig.s_SaveGadgetPositions , ShConfig.s_SaveFltWindowsPos );
  }
  GetIView()->SetBuilder( m_pBuilder );
  if ( pBuilder )
  {
    pBuilder->AddRef();
    GetIView()->ComposeGraph();
    pBuilder->SetViewed( true ) ;
  }

  CDRect Pos( -1. , -1. , -1. , -1. ) ;
  m_pBuilder->ViewSectionGetGraphPos( Pos ) ;
  if ( !SetGraphPosition( Pos ) )
  {
    CRect WindowRect ;
    GetWindowRect( &WindowRect ) ;
    CWnd * pParent = GetParent() ;
    if ( pParent )
    {
      CRect ParentWindow ;
      pParent->GetWindowRect( &ParentWindow ) ;
      CDRect GraphPos ;
      CalcRelativeRect( WindowRect , ParentWindow , GraphPos ) ;
      m_pBuilder->ViewSectionSetGraphPos( GraphPos ) ;
    }
  }
  
}

BOOL CChildFrame::SetGraphPosition( CDRect& Pos )
{
  CWnd * pParent = GetParent() ;
  CRect ParentClient ;
  pParent->GetClientRect( &ParentClient ) ;
  if ( Pos.right > 0. )
  {
    int iX = ROUND( ParentClient.Width() * Pos.left ) ;
    int iY = ROUND( ParentClient.Height() * Pos.top ) ;
    int iW = ROUND( ParentClient.Width() * Pos.right ) ; // this is width, not right 
    int iH = ROUND( ParentClient.Height() * Pos.bottom ) ; // this is height, not bottom

    MoveWindow( iX , iY , iW , iH , TRUE ) ;
    return TRUE ;
  }
  return FALSE ;
}

void CChildFrame::SetGraphName( LPCTSTR graphName )
{
  m_GraphName = graphName;
  FXString title( _T( "" ) );
  if ( !m_GraphName.IsEmpty() )
  {
    title = (m_bLongTitle) ? (LPCTSTR)m_GraphName : GetGraphTitle( m_GraphName );
    FXRegistry Reg( "TheFileX\\SHStudio" ) ;
    Reg.WriteRegiString( _T( "settings" ) , _T( "graphname" ) , m_GraphName );
  }
  SetTitle( title );
  SetWindowText( title );
}

void CChildFrame::ToggleLongName()
{
  m_bLongTitle = !m_bLongTitle;
  SetGraphName( m_GraphName );
}

BOOL CChildFrame::LoadGraph( LPCTSTR filename )
{
  GetIView()->DeleteGraph();
  switch ( GetBuilder()->Load( filename , NULL ) )
  {
    case MSG_ERROR_LEVEL:
      SENDFAIL_0( _T( "Graph loading failed. See log for details" ) );
      return FALSE;
    case MSG_WARNING_LEVEL:
      SENDWARN_0( _T( "Graph loading completed with warnings. See log for details" ) );
  }
  GetIView()->ComposeGraph();
  SetGraphName( filename );
  GetIView()->InitUndoManager();
  GetIView()->SetViewOffset( GetBuilder()->ViewSectionGetViewOffset() ) ;
  GetIView()->SetViewScale( GetBuilder()->ViewSectionGetViewScale() ) ;
  return TRUE;
}

BOOL CChildFrame::LoadScript( LPCTSTR Script , LPCTSTR fName )
{
  GetIView()->DeleteGraph();
  switch ( GetBuilder()->Load( NULL , Script ) )
  {
    case MSG_ERROR_LEVEL:
      SENDFAIL_0( _T( "Graph loading failed. See log for details" ) );
      return FALSE;
    case MSG_WARNING_LEVEL:
      SENDWARN_0( _T( "Graph loading completed with warnings. See log for details" ) );
  }
  GetIView()->ComposeGraph();
  SetGraphName( fName );
  GetIView()->InitUndoManager();
  GetIView()->SetViewOffset( GetBuilder()->ViewSectionGetViewOffset() ) ;
  GetIView()->SetViewScale( GetBuilder()->ViewSectionGetViewScale() ) ;
  return TRUE;
}

BOOL CChildFrame::IncludePlugin( FXString& Plugin )
{
  if ( !m_pBuilder )
    return FALSE;
  int Result = m_pBuilder->GetPluginLoader()->CheckPluginExist( Plugin );
  if ( Result == IPluginLoader::PL_SUCCESS )
  {
    switch ( AfxMessageBox( "Plugin with this name exist. OK to substitute?" , MB_OKCANCEL ) )
    {
      case IDOK:
        // call substitute here
      case IDCANCEL:
        return FALSE;
    }
  }
  else if ( Result == IPluginLoader::PL_PLUGINNOTFOUND )
    return (m_pBuilder->GetPluginLoader()->IncludePlugin( GetBuilder() , Plugin ) == IPluginLoader::PL_SUCCESS);
  return FALSE;
}


BOOL CChildFrame::PreCreateWindow( CREATESTRUCT& cs )
{
  if ( !CMDIChildWnd::PreCreateWindow( cs ) )
    return FALSE;
  //cs.lpCreateParams
  cs.dwExStyle &= ~(WS_EX_CLIENTEDGE | WS_EX_NOACTIVATE);
  cs.lpszClass = AfxRegisterWndClass( 0 );
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

int CChildFrame::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
  if ( CMDIChildWnd::OnCreate( lpCreateStruct ) == -1 )
    return -1;

  m_pWndView = Tvdb400_GetSketchViewMod();

  // create a view to occupy the client area of the frame
  if ( !GET_WND( m_pWndView )->Create( NULL , NULL , AFX_WS_DEFAULT_VIEW ,
    CRect( 0 , 0 , 0 , 0 ) , this , AFX_IDW_PANE_FIRST , NULL ) )
  {
    TRACE0( "Failed to create view window\n" );
    return -1;
  }

  SetTitle( m_GraphName );
  SetWindowText( m_GraphName );
  //Setting icon for ChildFrame
  HICON hI = GetIcon( FALSE );
  if ( !hI )
  {
    SetIcon( AfxGetApp()->LoadIcon( IDI_ICON_DOC ) , FALSE );
  }
  //To make CChildFrame active after creation
  SetFocus();


  return 0;
}

BOOL CChildFrame::DestroyWindow()
{
  CStringArray tmp;
  if ( GetBuilder()->IsDirty() )
  {
    CString mes;
    mes.Format( "Graph '%s' has been modified. Do you want to save changes?" , m_GraphName );
    switch ( ::AfxMessageBox( mes , MB_YESNOCANCEL | MB_ICONQUESTION ) )
    {
      case IDYES:
        GetBuilder()->Stop();
        OnFileSaveGraph();

        break;
      case IDNO:
        break;
      case IDCANCEL:
        return FALSE;
    }
  }
  //GetBuilder()->Stop();
//    m_pBuilder->ShutDown();
  GetBuilder()->SetViewed( false ) ;
  m_pBuilder->Release();
  FXRegistry Reg( "TheFileX\\SHStudio" ) ;
  Reg.WriteRegiInt( "childframe" , "zoomed" , IsZoomed() );
  return CMDIChildWnd::DestroyWindow();
}

void CChildFrame::OnDestroy()
{
  if ( m_PerformanceDlg2.m_hWnd )
    m_PerformanceDlg2.DestroyWindow();
  m_pWndView->Release();
  m_pWndView = NULL;
  CMDIChildWnd::OnDestroy();
  GetParentFrame()->SendMessage( WM_PARENTNOTIFY , WM_DESTROY , (LPARAM) this );
}

void CChildFrame::OnSetFocus( CWnd* pOldWnd )
{
  GET_WND( m_pWndView )->SetFocus();
}

BOOL CChildFrame::OnCmdMsg( UINT nID , int nCode , void* pExtra , AFX_CMDHANDLERINFO* pHandlerInfo )
{
  // let the view have first crack at the command
/*   TRACE("OnCmdMsg(UINT nID=0x%x, int nCode=0x%x, void* pExtra=0x%x, AFX_CMDHANDLERINFO* pHandlerInfo=0x%x) \n",
        nID, nCode, pExtra, pHandlerInfo);
  if ((m_pWndView) && (GET_WND(m_pWndView)->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo)))
    return TRUE; */
  return CMDIChildWnd::OnCmdMsg( nID , nCode , pExtra , pHandlerInfo );
}

void CChildFrame::OnFileSaveGraph()
{
  if ( m_GraphName.IsEmpty() )
  {
    FXString title = m_pBuilder->GetID();
    if ( !title.IsEmpty() )
      m_GraphName.Format( _T( "%s.tvg" ) , title );
  }
  else
  {
    int iBSlash = (int) m_GraphName.ReverseFind( _T( '\\' ) ) ;
    int iSlash = (int) m_GraphName.ReverseFind( _T( '/' ) ) ;
    if ( iBSlash > iSlash )
      iSlash = iBSlash ;

    FXString GraphName = (iSlash >= 0) ? m_GraphName.Mid( iSlash + 1 ) : m_GraphName ;

    int iPointPos = (int) GraphName.Find( _T( '.' ) ) ;
    if ( iPointPos > 0 && (GraphName.Mid(iPointPos).MakeLower() != _T(".tvg") ) )
    {
      FXString MainGraphName = GraphName.Left( iPointPos ) ;
      int iBufLen = (int) MainGraphName.GetLength() + 5 ;
      TCHAR * pRootGraphName = new TCHAR[ iBufLen * sizeof(TCHAR)] ;
      _tcscpy_s( pRootGraphName , iBufLen , (LPCTSTR) MainGraphName ) ;
      CRuntimeClass * pRunTimeClass = GetRuntimeClass() ;
      if ( pRunTimeClass && pRunTimeClass->m_lpszClassName )
      {
        int iRTCLen = (int) _tcslen( pRunTimeClass->m_lpszClassName ) + 5 ;
        TCHAR * pClassBuf = new TCHAR[ iRTCLen ] ;
        _tcscpy_s( pClassBuf , iRTCLen , pRunTimeClass->m_lpszClassName ) ;

        CWnd * pMainWnd = GetMDIFrame() ;
        pMainWnd->PostMessage( WM_SaveRootGraph , (WPARAM)pClassBuf , (LPARAM)pRootGraphName ) ;
        return ;

      }
    }
  }
  BOOL bWasEmpty = m_GraphName.IsEmpty() ;
  if ( bWasEmpty )
  {
    FXRegistry Reg( "TheFileX\\SHStudio" ) ;
    m_GraphName = Reg.GetRegiString( _T( "settings" ) , _T( "graphname" ) , _T( "" ) );
  }
  if ( (bWasEmpty = m_GraphName.IsEmpty()) )
    GuessGraphName( m_GraphName );
  CFileDialog fd( FALSE , _T( "tvg" ) , m_GraphName , OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT , _T( "tvdb graphs (*.tvg)|*.tvg|all files (*.*)|*.*||" ) );
  if ( (!bWasEmpty && (GetAsyncKeyState( VK_CONTROL ) & 0x8000))  ||  fd.DoModal() == IDOK )
  {
    m_pBuilder->ViewSectionSetConfig( ShConfig.s_SaveGadgetPositions , ShConfig.s_SaveFltWindowsPos );
    m_pBuilder->ViewSectionSetViewOffset( m_pWndView->GetViewOffset() ) ;
    m_pBuilder->ViewSectionSetViewScale(  m_pWndView->GetViewScale() ) ;
    CRect WindowRect ;
    GetWindowRect( &WindowRect ) ;
    CWnd * pParent = GetParent() ;
    if ( pParent )
    {
      CRect ParentWindow ;
      pParent->GetWindowRect( &ParentWindow ) ;
      CDRect GraphPos ;
      CalcRelativeRect( WindowRect , ParentWindow , GraphPos ) ;
      m_pBuilder->ViewSectionSetGraphPos( GraphPos ) ;
    }
    CString fileName = fd.GetPathName();
    if ( !GetBuilder()->Save( fileName ) )
      AfxMessageBox( _T( "Failed to save graph!" ) );
    else
    {
      SetGraphName( fd.GetPathName() );
      CWnd * pMainWnd = AfxGetMainWnd() ;
      if ( pMainWnd )
        pMainWnd->SetWindowText( fd.GetPathName() ) ;
    }
    GetIView()->InitUndoManager();
  }
}

void CChildFrame::OnUpdateFileSaveGraph( CCmdUI* pCmdUI )
{
  CStringArray tmp;
//   pCmdUI->Enable( GetBuilder()->IsDirty() );
  pCmdUI->Enable( TRUE );
}

BOOL CChildFrame::GraphStart()
{
  GetBuilder()->Start();
  return TRUE;
}

void CChildFrame::OnOperateStart()
{
  GetBuilder()->Start();
}

void CChildFrame::OnUpdateOperateStart( CCmdUI* pCmdUI )
{
  pCmdUI->Enable( !GetBuilder()->IsRuning() );
}

void CChildFrame::OnOperateStop()
{
  GetBuilder()->Stop();
}

void CChildFrame::OnUpdateOperateStop( CCmdUI* pCmdUI )
{
  pCmdUI->Enable( GetBuilder()->IsRuning() );
}

void CChildFrame::OnOperateStartstop()
{
  if ( GetBuilder()->IsRuning() )
    GetBuilder()->Stop();
  else
    GetBuilder()->Start();
}


void CChildFrame::OnOperatePause()
{
  GetBuilder()->Pause();
}

void CChildFrame::OnUpdateOperatePause( CCmdUI* pCmdUI )
{
  pCmdUI->Enable( GetBuilder()->IsRuning() );
}

void CChildFrame::OnOperateStepForward()
{
  GetBuilder()->StepFwd();
}

void CChildFrame::OnUpdateOperateStepForward( CCmdUI* pCmdUI )
{
  pCmdUI->Enable( GetBuilder()->IsPaused() );
}

void CChildFrame::OnEditDelete()
{
  GetIView()->DeleteSelected();
}

void CChildFrame::OnEditClearAll()
{
  GetIView()->DeleteGraph();
}

void CChildFrame::OnEditAggregate()
{
  CAggregateDlg ad;
  ad.m_BlockName = _T( "" );
  if ( ad.DoModal() == IDOK )
  {
    CString name = ad.m_BlockName;
    if ( ad.m_bAddToLibrary )
    {
      name.Replace( ' ' , '_' );
      CString uid = name.Mid( name.ReverseFind( '.' ) + 1 );
      uid += _T( "1" );
      name.MakeLower();
      FXString path;
      GetUserLibraryPath( path );
      FxVerifyCreateDirectory( path );
      path += _T( "\\" ) + name + _T( ".cog" );
      CFileStatus fs;
      if ( CFile::GetStatus( path , fs ) )
      {
        switch ( ::AfxMessageBox( "Complex gadget with this name already exists in this group. Overwrite?" , MB_YESNOCANCEL ) )
        {
          case IDYES:
            break;
          case IDNO:
            OnEditAggregate();
          case IDCANCEL:
            return;
        }
      }
      GetIView()->AggregateSelected( uid , path );
      ::AfxGetMainWnd()->PostMessage( WM_CHILDACTIVATE );
    }
    else
    {
      GetIView()->AggregateSelected( name );
    }
  }
}

void CChildFrame::OnEditExpand()
{
  GetIView()->ExpandSelected();
}

BOOL CChildFrame::OnSetCursor( CWnd* pWnd , UINT nHitTest , UINT message )
{
  if ( (nHitTest == HTCAPTION && !m_bLongTitle) || (nHitTest != HTCAPTION && m_bLongTitle) )
    ToggleLongName();
  return CMDIChildWnd::OnSetCursor( pWnd , nHitTest , message );
}

void CChildFrame::OnEditSelectAll()
{
  GetIView()->SelectAll();
}

void CChildFrame::OnSettings()
{
  Tvdb400_RunSetupDialog( GetBuilder() );
  GetIView()->UpdateWindow();
}

BOOL CChildFrame::OnMouseWheel( UINT nFlags , short zDelta , CPoint pt )
{
  CWnd*w = GetIView()->GetWnd();
  if ( w )
    return (BOOL)(w->SendMessage( WM_MOUSEWHEEL ,
    MAKEWPARAM( nFlags , zDelta ) , MAKELPARAM( pt.x , pt.y ) ));
  return CMDIChildWnd::OnMouseWheel( nFlags , zDelta , pt );
}

void CChildFrame::OnEditUndo()
{
  GetIView()->Undo();
}

void CChildFrame::OnUpdateEditUndo( CCmdUI* pCmdUI )
{
  pCmdUI->Enable( GetIView()->CanUndo() );
}

void CChildFrame::OnEditRedo()
{
  GetIView()->Undo( -1 );
}

void CChildFrame::OnUpdateEditRedo( CCmdUI* pCmdUI )
{
  pCmdUI->Enable( GetIView()->CanRedo() );
}

void CChildFrame::OnViewUndoRedoManager()
{
  CUndoDialog ud;
  ud.SetView( GetIView() );
  ud.DoModal();
}

void CChildFrame::OnUpdateViewUndoRedoManager( CCmdUI* pCmdUI )
{
  pCmdUI->Enable( (GetIView()->CanUndo() + GetIView()->CanRedo()) );
}

void CChildFrame::OnOperateDebug()
{
  GetIView()->SetDebugMode( !GetIView()->IsDebugging() );
}

void CChildFrame::OnUpdateOperateDebug( CCmdUI* pCmdUI )
{
  pCmdUI->SetCheck( (GetIView()->IsDebugging()) ? 1 : 0 );
}

void CChildFrame::OnEditCopy()
{
  GetIView()->Copy();
}

void CChildFrame::OnUpdateEditCopy( CCmdUI* pCmdUI )
{
  pCmdUI->Enable( GetIView()->CanCopy() );
}

void CChildFrame::OnEditPaste()
{
  GetIView()->Paste();
}

void CChildFrame::OnUpdateEditPaste( CCmdUI* pCmdUI )
{
  pCmdUI->Enable( GetIView()->CanPaste() );
}


BOOL CChildFrame::OnHelpInfo( HELPINFO* pHelpInfo )
{
  FXString GadgetName = (FXString) (GetIView()->GetCurrentGadgetName());

  if ( GadgetName.GetLength() == 0 )
    ShowHelp( ID_CONTEXT_HELP , _T( "@SÒUDIO" ) );
  else
  {
    FXString Class;
    FXString Lineage;
    m_pBuilder->GetGadgetClassAndLineage( GadgetName , Class , Lineage );
    FXString HelpPdfFileName( Class ) ;
    FXString HelpFilePath = FXGetExeDirectory() + "/PlugIns/" + HelpPdfFileName + "*.pdf" ;
    FXFileFind ff ;
    bool bFound = ff.FindFile( HelpFilePath ) ;
    while ( bFound )
    {
      bFound = ff.FindNextFile();
      if ( ff.IsDots() || ff.IsDirectory() )
        continue;
      FXString HelpFindPath = ff.GetFilePath() ;
      if ( HelpFindPath.GetLength() )
      {
        HelpFindPath.Insert( 0 , "Start /B " ) ;
        system( HelpFindPath ) ;
        return TRUE ;
      }
    }
    // if no pdf, call old style help
    ShowHelp( ID_CONTEXT_HELP , Lineage + _T( "." ) + Class );
  }
  return TRUE;
}

void CChildFrame::OnViewPerformancedialog2()
{
  m_PerformanceDlg2.SetBuilder( m_pBuilder );
  if ( !m_PerformanceDlg2.m_hWnd )
    m_PerformanceDlg2.Create( CPerforamceDlg2::IDD , NULL );
  else
  {
    m_PerformanceDlg2.DestroyWindow();
    return;
  }
  m_PerformanceDlg2.ShowWindow( SW_SHOWNORMAL );
}


void CChildFrame::OnUpdateViewPerformancedialog2( CCmdUI *pCmdUI )
{
  pCmdUI->Enable( m_pBuilder != NULL );
  if ( m_pBuilder != NULL )
    pCmdUI->SetCheck( (m_PerformanceDlg2.m_hWnd) ? 1 : 0 );
}

void CChildFrame::OnSize( UINT nType , int cx , int cy )
{
  CMDIChildWnd::OnSize( nType , cx , cy );
  if ( !m_pBuilder )
    return ;
  CWnd * pParent = GetParent() ;
  CRect WindowRect ;
  GetWindowRect( &WindowRect ) ;
  if ( pParent )
  {
    CRect ParentWindow ;
    pParent->GetWindowRect( &ParentWindow ) ;
    CDRect GraphPos ;
    CalcRelativeRect( WindowRect , ParentWindow , GraphPos ) ;
    m_pBuilder->ViewSectionSetGraphPos( GraphPos ) ;
  }
}


void ResetAffinity( IGraphbuilder* pBuilder )
{
  if ( pBuilder )
  {
    CStringArray src , dst;
    pBuilder->EnumGadgets( src , dst );

    for ( int i = 0; i < src.GetCount(); i++ )
    {
      CGadget *gadget = pBuilder->GetGadget( src[ i ] );
      if ( gadget )
      {
        if ( gadget->IsKindOf( RUNTIME_GADGET( Complex ) ) )
        {
          ResetAffinity( ((Complex*) gadget)->Builder() );
        }
        else
        {
          gadget->SetAffinity( -1 );
        }
      }
    }
    for ( int j = 0; j < dst.GetCount(); j++ )
    {
      CGadget *gadget = pBuilder->GetGadget( dst[ j ] );
      if ( gadget )
      {
        if ( gadget->IsKindOf( RUNTIME_GADGET( Complex ) ) )
        {
          ResetAffinity( ((Complex*) gadget)->Builder() );
        }
        else
        {
          gadget->SetAffinity( -1 );
        }
      }
    }
    pBuilder->SetDirty();
  }
}

void CChildFrame::OnEditResetaffinity()
{
  ResetAffinity( m_pBuilder );
}

void CChildFrame::OnUpdateEditResetaffinity( CCmdUI *pCmdUI )
{
  pCmdUI->Enable( m_pBuilder != NULL );
}


void CChildFrame::OnMove( int x , int y )
{
  CMDIChildWnd::OnMove( x , y );

  if ( !m_pBuilder )
    return ;
  CWnd * pParent = GetParent() ;
  CRect WindowRect ;
  GetWindowRect( &WindowRect ) ;
  if ( pParent )
  {
    CRect ParentWindow ;
    pParent->GetWindowRect( &ParentWindow ) ;
    CDRect GraphPos ;
    CalcRelativeRect( WindowRect , ParentWindow , GraphPos ) ;
    m_pBuilder->ViewSectionSetGraphPos( GraphPos ) ;
  }
}


void CChildFrame::OnUsedgadgetsView()
{
  FXStringArray FXActiveGadgets  ;
  GetBuilder()->EnumAndArrangeGadgets( FXActiveGadgets ) ;
  vector<string> VActiveDLLs ;
  FxSendLogMsg( 1 , "Builder" , 0 , "Gadgets In Graph:" ) ;
  for ( int i = 0 ; i < FXActiveGadgets.GetCount() ; i++ )
  {
    CGadget * pGadget = GetBuilder()->GetGadget( (LPCTSTR) FXActiveGadgets[ i ] ) ;
    if ( pGadget )
    {
      CRuntimeGadget * pRT = pGadget->GetRuntimeGadget() ;
      FxSendLogMsg( 1 , "Builder" , 0 , "Gadget %s (%s)  PlugIn DLL: %s" ,
        (LPCTSTR) FXActiveGadgets[ i ] , pRT->m_lpszClassName , pRT->m_lpszPlugin ) ;
      if ( VActiveDLLs.size() == 0 )
        VActiveDLLs.push_back( pRT->m_lpszPlugin ) ;
      else
      {
        for ( int j = 0 ; j < (int) VActiveDLLs.size() ; j++ )
        {
          if ( VActiveDLLs[ j ] == pRT->m_lpszPlugin )
            break ;
          else
          {
            if ( j == (int) VActiveDLLs.size() - 1 )
              VActiveDLLs.push_back( pRT->m_lpszPlugin ) ;
          }
        }
      }
    }
  }
  for ( size_t i = 0 ; i < VActiveDLLs.size() ; i++ )
  {
    if ( i == 0 )
      FxSendLogMsg( 1 , "Builder" , 0 , "PlugIns for Graph:" ) ;
    FxSendLogMsg( 1 , "Builder" , 0 , "%s" , VActiveDLLs[ i ].c_str() ) ;
  }

}


void CChildFrame::OnUsedgadgetsFormlist()
{
  FXStringArray FXActiveGadgets  ;
  EnumAndArrangeAllGadgets( GetBuilder() , FXActiveGadgets ) ;
  if ( FXActiveGadgets.GetSize() )
  {
    FXString Out ;
    FXFile OutFile( _T("LastGadgetsList.txt") , FXFile::modeWrite | FXFile::modeCreate ) ;
    vector<string> VActiveDLLs ;
    Out.Format( "Gadgets In Graph: \r\n" ) ;
    OutFile.Write( ( LPCTSTR ) Out , Out.GetLength() ) ;
    for ( int i = 0 ; i < FXActiveGadgets.GetCount() ; i++ )
    {
      Out.Format( "Gadget %s" , (LPCTSTR) FXActiveGadgets[ i ] ) ;

      size_t iPos = FXActiveGadgets[ i ].Find( ':' ) ;
      if ( iPos > 0 )
      {
        FXString DLL = FXActiveGadgets[ i ].Mid( iPos + 1 ) ;
        if ( VActiveDLLs.size() == 0 )
          VActiveDLLs.push_back( ( LPCTSTR ) DLL ) ;
        else
        {
          for ( int j = 0 ; j < ( int ) VActiveDLLs.size() ; j++ )
          {
            if ( DLL == VActiveDLLs[ j ].c_str() )
              break ;
            else
            {
              if ( j == ( int ) VActiveDLLs.size() - 1 )
                VActiveDLLs.push_back( ( LPCTSTR ) DLL ) ;
            }
          }
        }
      }
      else
        Out += ( " is COMPLEX, No DLL" ) ;
      Out += "\r\n" ;
      OutFile.Write( ( LPCTSTR ) Out , Out.GetLength() ) ;
    }
    for ( size_t i = 0 ; i < VActiveDLLs.size() ; i++ )
    {
      if ( i == 0 )
      {
        Out.Format( "PlugIns for Graph:\r\n" ) ;
        OutFile.Write( (LPCTSTR) Out , Out.GetLength()  ) ;
      }
      Out.Format( "%d %s\r\n" , i + 1 , VActiveDLLs[ i ].c_str() ) ;
      OutFile.Write( (LPCTSTR) Out , Out.GetLength() ) ;
    }
    OutFile.Close() ;
    FxSendLogMsg( 1 , "Builder" , 0 , "File 'LastGadgetsList.txt' is created/modified in your working directory" ) ;
    return ;
  }
  FxSendLogMsg( 7 , "Builder" , 0 , "No gadgets" ) ;
}



void CChildFrame::OnFormActiveExeAndDLLCopyBatch()
{

  FXStringArray FXActiveGadgets  ;
  EnumAndArrangeAllGadgets( GetBuilder() , FXActiveGadgets ) ;
  if ( FXActiveGadgets.GetSize() )
  {
    FXString GraphName = GetBuilder()->GetID() ;
    FXSIZE iSlashPos = GraphName.ReverseFind( '/' ) ;
    FXSIZE iBSlashPos = GraphName.ReverseFind( '\\' ) ;

    if ( iSlashPos < iBSlashPos )
      iSlashPos = iBSlashPos ;
    if ( iSlashPos >= 0 )
      GraphName = GraphName.Mid( iSlashPos + 1 ) ;

    FXString FileName = GraphName + "_FormWorkingCopy.bat" ;
    FXFile OutFile( (LPCTSTR)FileName , FXFile::modeWrite | FXFile::modeCreate ) ;

    FXRegistry Reg( "TheFileX\\SHStudio" ) ;
    FXString ConfigFileName = Reg.GetRegiString( "Configuration" , 
      "DependenciesForDeploy" , "Externals/DependenciesConfiguration.conf" ) ;
    FXString OutputDir = getenv( "SH_OUTPUT" ) ;
    ConfigFileName = ( OutputDir + "/../" ) + ConfigFileName ;
    std::ifstream ConfigFile( ConfigFileName ) ;

    if ( !ConfigFile.is_open() )
    {
      FxSendLogMsg( 7 , "Form Deploy Dir" , 0 ,
        "Configuration File '%s' does not exists" , ( LPCTSTR ) ConfigFileName ) ;
      return ;
    }
    FXString DeployConfiguration ;
    char c = ConfigFile.get() ;
    while ( ConfigFile.good() )
    {
      DeployConfiguration += c ;
      c = ConfigFile.get() ;
    }
    ConfigFile.close() ;

    FXString Out ;

    // Text for folder names forming and directory creation
    Out.Format( "::  Batch file for graph %s deployment\n\n" ,
      ( LPCTSTR ) GraphName ) ;
    OutFile.Write( ( LPCTSTR ) Out , Out.GetLength() ) ;


    FXString Folders = GetStringVarValue( DeployConfiguration , "Folders" ,
      '{' , '}' ) ;
    
    Out.Format( ( LPCTSTR ) Folders , ( LPCTSTR ) GraphName ) ;
    OutFile.Write( ( LPCTSTR ) Out , Out.GetLength() ) ;

    FXString GraphPath( m_GraphName ) ;
    if ( !GraphPath.IsEmpty() )
    {
      Out.Format( "::  Copy graph file\ncopy %s %%OutputFolder%%*.*\n" , (LPCTSTR)GraphPath ) ;
      OutFile.Write( ( LPCTSTR ) Out , Out.GetLength() ) ;
    }

//     "::Folders\n\n"
//       "set InputFolder=%%SH_OUTPUT%%\\15.0\\x86\\Release\\\n"
//       "set PluginsInput=%%InputFolder%%Plugins\\\n"
//       "set OutputFolder=%%InputFolder%%..\\DeployDirFor_%s\\\n"
//       "set PluginsOut=%%OutputFolder%%\\Plugins\\\n"
//       "mkdir %%PluginsOut%%\n"
//       "set ExternalsFolder=%%SH_OUTPUT%%\\..\\Externals\\\n" ,
//       ( LPCTSTR ) GraphName , ( LPCTSTR ) GraphName ) ;

    // Text for SH EXEs and DLLs copy to target directory


//     Out += "::SH core copy\n\n"
//       "Copy /Y %InputFolder%SHStudio.exe %OutputFolder%*.*\n"
//       "Copy /Y %InputFolder%UI_NViews.exe %OutputFolder%*.*\n"
//       "Copy /Y %InputFolder%fxfc.dll %OutputFolder%*.*\n"
//       "Copy /Y %InputFolder%gadbase.dll %OutputFolder%*.*\n"
//       "Copy /Y %InputFolder%NetApi.dll %OutputFolder%*.*\n"
//       "Copy /Y %InputFolder%shbase.dll %OutputFolder%*.*\n"
//       "Copy /Y %InputFolder%shbaseCLI.dll %OutputFolder%*.*\n"
//       "Copy /Y %InputFolder%shkernel.dll %OutputFolder%*.*\n"
//       "Copy /Y %InputFolder%shvideo.dll %OutputFolder%*.*\n"
//       "Copy /Y %InputFolder%shwrapper.dll %OutputFolder%*.*\n"
//       "Copy /Y %InputFolder%stdsetup.dll %OutputFolder%*.*\n"
//       "Copy /Y %InputFolder%tvhelp.dll %OutputFolder%*.*\n"
//       "Copy /Y %InputFolder%tview.dll %OutputFolder%*.*\n"
//       "Copy /Y %InputFolder%tvinspect.dll %OutputFolder%*.*\n"
//       "Copy /Y %InputFolder%tiff.dll %OutputFolder%*.*\n"
//       "Copy /Y %InputFolder%jpeg62.dll %OutputFolder%*.*\n"
//       "Copy /Y %InputFolder%zlib1.dll %OutputFolder%*.*\n"
//       "Copy /Y %InputFolder%lzma.dll %OutputFolder%*.*\n\n"
//       "::  Copy plugin gadgets DDLs\n\n" ;

    Out = GetStringVarValue( DeployConfiguration , "SH_EXEs_and_DLLs" ,
      '{' , '}' ) ;
    Out += "::  Copy plugin gadgets DDLs\n\n" ;
    OutFile.Write( ( LPCTSTR ) Out , Out.GetLength() ) ;

    bool bBaslerExists = false ;
    vector<string> VActiveDLLs ;
    for ( int i = 0 ; i < FXActiveGadgets.GetCount() ; i++ )
    {
//         CRuntimeGadget * pRT = pGadget->GetRuntimeGadget() ;
      size_t iPos = FXActiveGadgets[ i ].Find( ':' ) ;
      if ( iPos > 0 )
      {
        FXString DLL = FXActiveGadgets[ i ].Mid( iPos + 2 ) ; // remove also space after colon
        if ( VActiveDLLs.size() == 0 )
          VActiveDLLs.push_back( (LPCTSTR) DLL ) ;
        else
        {
          for ( int j = 0 ; j < ( int ) VActiveDLLs.size() ; j++ )
          {
            if ( DLL == VActiveDLLs[ j ].c_str() )
              break ;
            else
            {
              if ( j == ( int ) VActiveDLLs.size() - 1 )
                VActiveDLLs.push_back( (LPCTSTR)DLL ) ;
            }
          }
        }
      }
    }
    for ( size_t i = 0 ; i < VActiveDLLs.size() ; i++ )
    {
      Out.Format( "Copy /Y %%PluginsInput%%%s.dll %%PluginsOut%%*.*\n"
        , VActiveDLLs[ i ].c_str() ) ;
      OutFile.Write( (LPCTSTR) Out , Out.GetLength() ) ;
      Out = GetStringVarValue( DeployConfiguration ,
        VActiveDLLs[ i ].c_str() ,
        '{' , '}' ) ;
      if ( !Out.IsEmpty() )
        OutFile.Write( ( LPCTSTR ) Out , Out.GetLength() ) ;
    }

    Out = "pause\n" ;
    OutFile.Write( ( LPCTSTR ) Out , Out.GetLength() ) ;

    OutFile.Close() ;
    FxSendLogMsg( 1 , "Builder" , 0 , 
      "File '%s' is created/modified in your working directory" , (LPCTSTR)FileName ) ;
    return ;
  }
  FxSendLogMsg( 7 , "Builder" , 0 , "No gadgets" ) ;
}


int CChildFrame::EnumAndArrangeAllGadgets( 
  IGraphbuilder * pBuilder , FXStringArray& Gadgets )
{
  pBuilder->EnumAndArrangeGadgets( Gadgets ) ;
  FXStringArray GadgetsInComplexes ;
  for ( int i = 0 ; i < Gadgets.GetCount() ; i++ )
  {
    CGadget * pGadget = pBuilder->GetGadget( ( LPCTSTR ) Gadgets[ i ] ) ;
    if ( pGadget )
    {
      CRuntimeGadget * pRT = pGadget->GetRuntimeGadget() ;
      if ( _tcscmp( pRT->m_lpszClassName , "Complex" ) == 0 )
      {
        Complex * pComplex = ( Complex* ) pGadget ;
        EnumAndArrangeAllGadgets(
          pComplex->Builder() , GadgetsInComplexes ) ;
        FXString Insertion = Gadgets[ i ] + '.' ;
        for ( int iSub = 0 ; iSub < GadgetsInComplexes.size() ; iSub++ )
        {
          GadgetsInComplexes[ iSub ].Insert( 0 , Insertion ) ;
        }
      }
      else
      {
        Gadgets[ i ] += " (" ;
        Gadgets[ i ] += pRT->m_lpszClassName ;
        Gadgets[ i ] += ")  PlugIn DLL: " ;
        Gadgets[ i ] += pRT->m_lpszPlugin ;
      }
    }
  }
  Gadgets.Append( GadgetsInComplexes ) ;

  return (int)Gadgets.Size() ;
}

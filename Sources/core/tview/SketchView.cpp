// SketchView.cpp : implementation file
//

#include "stdafx.h"
#include <scriptdefinitions.h>
#include "sketchview.h"
#include "resource.h"
#include "graphtreecomposer.h"
#include "selectgadgetclassdlg.h"
#include "renderviewframe.h"
#include "insertmonitordlg.h"
#include "floatwnd.h"
#include <gadgets\shkernel.h>
#include "debugrender.h"
#include <security\basesecurity.h>
#include "memdc.h"
#include "debugenvelopdlg.h"
#include "tieddebugrender.h"
#include "simpleenterdlg.h"
#include "AffinityDlg.h"
#include <fxfc/CSystemMonitorsEnumerator.h>
#include "fxfc/FXRegistry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "SketchView"
#define LINE_SCROLL_DISTANCE 5
#define BOUND(x,min,max)        ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#define FLASH_TIMER	1
#define FLASH_LAPSE	200
#define TIMES_FLASHING 3
#define UPDATE_TIMER 2
#define UPDATE_LAPSE 500
#define UDATE_LAPSE_FOR_GROUP 100
#define CONNECTOR_STATE_TIMER 4
#define CONNECTOR_STATE_LAPSE 200

#define VIEW_GAP 100

#ifndef GLYPHS_DRAW_EDGE
#define NORMAL_BORDER_COLOR	RGB(0, 0, 0)
#define NORMAL_INNER_COLOR		RGB(255, 255, 255)
#define SELECT_BORDER_COLOR	RGB(0, 0, 255)
#define SELECT_INNER_COLOR		RGB(255, 255, 255)
#define INVALIDE_BORDER_COLOR	RGB(255, 0, 0)
#define INVALIDE_INNER_COLOR	RGB(255, 255, 255)
#define VIRTUAL_BORDER_COLOR	RGB(192, 192, 192)
#define SELECT_VIRTUAL_COLOR	RGB(176, 160, 255)
#define DATA_INNER_COLOR		RGB(0, 255, 0)
#define SKIPPED_INNER_COLOR	RGB(255, 0, 0)
#else
#define NORMAL_BORDER_COLOR	EDGE_BUMP
#define SELECT_BORDER_COLOR	EDGE_BUMP
#define NORMAL_INNER_COLOR		RGB(232, 255, 232)
#define SELECT_INNER_COLOR		RGB(255, 232, 232)
#define VIRTUAL_BORDER_COLOR	EDGE_ETCHED
#define SELECT_VIRTUAL_COLOR	EDGE_ETCHED
#define DATA_INNER_COLOR		RGB(0, 255, 0)
#define SKIPPED_INNER_COLOR	RGB(255, 0, 0)
#endif

static LPCTSTR specIcons[] =  // these gadgets have glyphs with specific icons
{
  "CSwitch"
};

__forceinline void GetTmpCopyFileName( CString& fileName )
{
  size_t ReturnValue;
  char   buffer[ 1024 ];
  if ( getenv_s( &ReturnValue , buffer , 1024 , "APPDATA" ) == 0 )
  {
    //if (GetStartingDirectory(fileName))
    fileName = buffer;
    fileName += _T( "selection.copy" );
  }
  else
    fileName.Empty();
}

extern CDynLinkLibrary* pThisDll;
/////////////////////////////////////////////////////////////////////////////
// CSketchView
#pragma warning(disable :4355)
CSketchView::CSketchView() :
  m_Graph( this ) ,
  m_SelectedGlyphs( this ) ,
  m_VirtualGlyphs( this ) ,
  m_BarsEnabled( 0 ) ,
  m_pLastActiveGlyph( NULL ) ,
  m_DebugWnd( NULL ) ,
  m_bDebugging( FALSE ) ,
  m_LastShownChange( -1 ) ,
  m_sCurrGadgetName( "" ) ,
  m_sCurrGadgetInfo( "" ) ,
  m_bRescaled( FALSE ) ,
  m_ViewActivity( true ) ,
  m_DropTarget( this ) ,
  m_bWasRescaled( FALSE ) ,
  m_pTargetWnd( NULL )
{
  m_pGraphBuilder = NULL;
  m_GadgetsFont.CreateFont( -6 , 0 , 0 , 0 , FW_NORMAL , FALSE , FALSE , FALSE , ANSI_CHARSET , OUT_TT_PRECIS , CLIP_TT_ALWAYS , PROOF_QUALITY , VARIABLE_PITCH | FF_SWISS , "MS Sans Serif" );
  m_GlyphIcons.Create( IDB_GADGET_ICONS , 16 , 5 , RGB( 0 , 128 , 128 ) );
  m_ModesIcons.Create( IDB_MODE_ICONS , 3 , 5 , RGB( 0 , 128 , 128 ) );
  CBitmap IconsEx;
  IconsEx.LoadBitmap( IDB_GADGET_ICONS_EX );
  m_GlyphIcons.Add( &IconsEx , RGB( 0 , 128 , 128 ) );
  m_hAccel = ::LoadAccelerators( pThisDll->m_hResource , MAKEINTRESOURCE( IDR_TVIEWACCELERATORS ) );
  memset( &m_ContextMenuInfo , 0 , sizeof( m_ContextMenuInfo ) );
  if ( !m_hAccel )
  {
    TRACE( FxLastErr2Mes() );
  }

  bool bLocalMachine = true ;
  char EvaluationOrLicense = isevaluation( bLocalMachine ) ;
  if ( (bLocalMachine && EvaluationOrLicense) 
    || (EvaluationOrLicense >= EOL_Evaluation ) )
  {
    if ( isexpired() )
    {
      delete this;
    }
  } ;
  FXRegistry Reg( "TheFileX\\SHStudio" ) ;

  m_ViewActivity = (Reg.GetRegiInt( _T( "settings" ) , _T( "viewactivity" ) , m_ViewActivity ) != FALSE);
}

CSketchView::~CSketchView()
{
  m_CopyRgn.DeleteObject();
  m_CopyRgn.m_hObject = NULL;
  if ( m_hAccel != NULL )
    DestroyAcceleratorTable( m_hAccel );
  m_hAccel = NULL;
  //	RevokeDragDrop((HWND) this);
}


BEGIN_MESSAGE_MAP( CSketchView , CWnd )
  //{{AFX_MSG_MAP(CSketchView)
  ON_WM_PAINT()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_MOUSEMOVE()
  ON_WM_DESTROY()
  ON_WM_RBUTTONDOWN()
  ON_WM_CREATE()
  ON_WM_TIMER()
  ON_WM_LBUTTONDBLCLK()
  ON_WM_VSCROLL()
  ON_WM_HSCROLL()
  ON_WM_MOUSEWHEEL()
  ON_WM_RBUTTONUP()
  ON_WM_ERASEBKGND()
  ON_MESSAGE( WM_MOUSELEAVE , OnMouseLeave )
  ON_MESSAGE( UM_FLTWND_CHANGED , OnFltWndChanged )
  ON_COMMAND( ID_CONTEXT_RENAME , OnContextRename )
  ON_COMMAND( ID_CONTEXT_SETUP , OnContextSetup )
  ON_COMMAND( ID_CONTEXT_AFFINITY , OnContextAffinity )
  ON_COMMAND( ID_CONTEXT_CMPLXLOCAL , OnContextCmplxLocal )
  ON_COMMAND( ID_MODE_REJECT , OnModeReject )
  ON_COMMAND( ID_MODE_TRANSMIT , OnModeTransmit )
  ON_COMMAND( ID_MODE_PROCESS , OnModeProcess )
  ON_COMMAND( ID_MODE_APPEND , OnModeAppend )
  ON_COMMAND( ID_MODE_REPLACE , OnModeReplace )
  ON_COMMAND( ID_THREADS_INCREASE , OnThreadsIncrease )
  ON_COMMAND( ID_THREADS_DECREASE , OnThreadsDecrease )
  ON_COMMAND( ID_PIN_SETLABEL , OnPinSetLabel )
  ON_COMMAND( ID_PIN_LOG_ON , OnPinLogging )
  ON_COMMAND( ID_PIN_LOG_OFF , OnPinLogging )
  //}}AFX_MSG_MAP

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSketchView message handlers

BOOL CSketchView::PreCreateWindow( CREATESTRUCT& cs )
{
  if ( !CWnd::PreCreateWindow( cs ) )
    return FALSE;

  cs.dwExStyle |= WS_EX_CLIENTEDGE;
  cs.style &= ~WS_BORDER;
  //To eliminate flicker of built-in frames
  cs.style |= WS_CLIPCHILDREN;
  cs.lpszClass = AfxRegisterWndClass( CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS ,
    ::LoadCursor( NULL , IDC_ARROW ) , HBRUSH( COLOR_WINDOW + 1 ) , NULL );

  return TRUE;
}

int CSketchView::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
  if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
    return -1;
  m_ToolTip.Create( IDD_TVTOOLTIP , this );
  SetTimer( UPDATE_TIMER , UPDATE_LAPSE , NULL );
  SetTimer( CONNECTOR_STATE_TIMER , CONNECTOR_STATE_LAPSE , NULL );
  return 0;
}

void CSketchView::OnDestroy()
{
  KillTimer( UPDATE_TIMER );
  KillTimer( CONNECTOR_STATE_TIMER );

  ClearView();
  if ( m_pGraphBuilder )
  {
    DWORD pRefs = m_pGraphBuilder->Release();
    if ( !pRefs )
    {
      m_pGraphBuilder = NULL;
    }
  }
  DeleteDebugRender();

  m_ToolTip.DestroyWindow();
  RevokeDragDrop( GetSafeHwnd() );
  CWnd::OnDestroy();
}


void CSketchView::SetBuilder( IGraphbuilder* Builder )
{
  if ( m_pGraphBuilder )
    m_pGraphBuilder->Release();
  Builder->AddRef();
  m_pGraphBuilder = Builder;
}

void CSketchView::ClearView()
{
  m_pLastActiveGlyph = NULL;
  m_SelectedGlyphs.DestroyIn();
  m_SelectedGlyphs.Select( &m_Graph );
  ClearSelected();
}

void CSketchView::DeleteGraph()
{
  m_pLastActiveGlyph = NULL;
  m_SelectedGlyphs.DestroyIn();
  m_SelectedGlyphs.Select( &m_Graph );
  DeleteSelected();
}

void CSketchView::SelectAll()
{
  m_SelectedGlyphs.DestroyIn();
  m_SelectedGlyphs.Select( &m_Graph );
  Invalidate();
}

void CSketchView::ComposeGraph( CStringArray* SrcGadgets , CStringArray* DstGadgets )
{
  KillTimer( UPDATE_TIMER );
  CGraphTreeComposer Composer;
  Composer.ComposeGraph( GetBuilder() , SrcGadgets , DstGadgets );
  POSITION pos = Composer.GetStartPosition();
  FXPropertyKit pkBuilder , uidPK;
  FXString props;
  if ( GetBuilder()->GetProperties( props ) )
    pkBuilder = props;
  while ( pos )
  {
    FXString uidGadget;
    CPoint pt;
    Composer.GetNextGadget( pos , uidGadget , pt );
    pt.x = pt.x * 100 + 50;
    pt.y = pt.y * 100 - 50;
    if ( pkBuilder.GetString( uidGadget , props ) )
    {
      uidPK = props;
      int x , y;
      if ( uidPK.GetInt( "x" , x ) )
        pt.x = x;
      if ( uidPK.GetInt( "y" , y ) )
        pt.y = y;
    }
    CStringArray inputs , outputs , duplex;
    FXString strClass , strLineage;
    if ( GetBuilder()->GetGadgetClassAndLineage( uidGadget , strClass , strLineage ) 
      == IGraphbuilder::TVDB400_GT_ANY )
    {
      SetTimer( UPDATE_TIMER , UPDATE_LAPSE , NULL );
      return; // failure
    }
    if ( strLineage == LINEAGE_DEBUG )
      continue; //We do not show debug gadgets in a view
    UINT gClass = GetBuilder()->ListGadgetConnectors( uidGadget , inputs , outputs , duplex );
    if ( gClass == IGraphbuilder::TVDB400_GT_ANY ) // failure
    {
      SetTimer( UPDATE_TIMER , UPDATE_LAPSE , NULL );
      return;
    }
    CGadgetGlyph* pGlyph = NULL;
    if ( gClass == IGraphbuilder::TVDB400_GT_CTRL )
    {
      pGlyph = new CRenderGlyph( this , uidGadget , inputs , outputs , duplex , GetViewType() );
      if ( static_cast<CRenderGlyph*>(pGlyph)->IsConstructed() == false )
      {
        delete pGlyph;
        pGlyph = NULL;
      }
    }
    else if ( gClass == IGraphbuilder::TVDB400_GT_COMPLEX )
      pGlyph = new CComplexGlyph( this , uidGadget , inputs , outputs , duplex , GetViewType() );
    else if ( gClass == IGraphbuilder::TVDB400_GT_PORT )
      pGlyph = new CPortalGlyph( this , uidGadget , inputs , outputs , duplex , GetViewType() );
    else
      pGlyph = new CGadgetGlyph( this , uidGadget , inputs , outputs , duplex , GetViewType() );
    if ( !pGlyph )
    {
      GetBuilder()->UnregisterGadget( uidGadget );
      SetTimer( UPDATE_TIMER , UPDATE_LAPSE , NULL );
      return;
    }
    switch( gClass )
    {
      case IGraphbuilder::TVDB400_GT_CAPTURE: pGlyph->SetBodyColor( 0xd0d0ff ) ; break ;
      case IGraphbuilder::TVDB400_GT_FILTER: pGlyph->SetBodyColor( 0xd0ffd0 ) ; break ;
      case IGraphbuilder::TVDB400_GT_PORT:
      {
        pGlyph->SetBodyColor( 0xd0d0d0 ) ;
        CGadget * pGadget = GetBuilder()->GetGadget( uidGadget ) ;
        if ( pGadget )
        {
          pGadget->SetModifiedUIDsPtr( ( LockingFIFO<FXString> * )GetBuilder()->GetModifiedUIDSsArray() ) ;
        }
      }
      break ;
      case IGraphbuilder::TVDB400_GT_CTRL: pGlyph->SetBodyColor( 0xf0f0d0 ) ; break ;
//       default:
//       {
// 
//         pGlyph->SetBodyColor( 0xc0c0ff ) ;
//       }
      break ;

    }
    AddGlyphToGraph( pGlyph );
    if ( strLineage == LINEAGE_VIRTUAL )
      m_VirtualGlyphs.Add( pGlyph );
    pGlyph->SetIcon( DefineGlyphIcon( strClass , gClass ) );
    pGlyph->OffsetPos( pt );
  }
  pos = Composer.GetStartPosition();
  CStringArray allInputs , allDuplex;
  while ( pos )
  {
    FXString uidGadget;
    CPoint pt;
    Composer.GetNextGadget( pos , uidGadget , pt );
    CStringArray inputs , outputs , duplex;
    if ( !GetBuilder()->ListGadgetConnectors( uidGadget , inputs , outputs , duplex ) )
    {
      SetTimer( UPDATE_TIMER , UPDATE_LAPSE , NULL );
      return;
    }
    allInputs.Append( inputs );
    allDuplex.Append( duplex );
  }
  CStringArray regConnections;
  while ( allInputs.GetSize() )
  {
    FXString uidOutputConnector;
    CStringArray uidOutputs;
    if ( GetBuilder()->IsConnected( allInputs[ 0 ] , uidOutputs ) )
    {
      while ( uidOutputs.GetSize() )
      {
        uidOutputConnector = uidOutputs[ 0 ];
        CGlyph* Glyph1 = m_Graph.FindByUID( allInputs[ 0 ] );
        CGlyph* Glyph2 = m_Graph.FindByUID( uidOutputConnector );
        if ( Glyph1 && Glyph2 )
        {
          Glyph1->ConnectTo( Glyph2 );
          CString connection;
          connection.Format( "%s,%s" , uidOutputConnector , allInputs[ 0 ] );
          regConnections.Add( connection );
        }
        uidOutputs.RemoveAt( 0 );
      }
    }
    allInputs.RemoveAt( 0 );
  }
  /// TODO ?
  while ( allDuplex.GetSize() )
  {
    CStringArray uidOutputs;
    CString uidOutputConnector;
    if ( GetBuilder()->IsConnected( allDuplex[ 0 ] , uidOutputs ) )
    {
      while ( uidOutputs.GetSize() )
      {
        uidOutputConnector = uidOutputs[ 0 ];
        CString connection1 , connection2;
        connection1.Format( "%s,%s" , uidOutputConnector , allDuplex[ 0 ] );
        connection2.Format( "%s,%s" , allDuplex[ 0 ] , uidOutputConnector );
        bool bAlreadyExists = false;
        for ( int j = 0; j < regConnections.GetSize(); j++ )
        {
          if ( connection1 == regConnections[ j ] ||
            connection2 == regConnections[ j ] )
          {
            bAlreadyExists = true;
            break;
          }
        }
        if ( !bAlreadyExists )
        {
          CGlyph* Glyph1 = m_Graph.FindByUID( allDuplex[ 0 ] );
          CGlyph* Glyph2 = m_Graph.FindByUID( uidOutputConnector );
          if ( Glyph1 && Glyph2 )
          {
            Glyph1->ConnectTo( Glyph2 );
            CString connection;
            connection.Format( "%s,%s" , uidOutputConnector , allDuplex[ 0 ] );
            regConnections.Add( connection );
          }
        }
        uidOutputs.RemoveAt( 0 );
      }
    }
    allDuplex.RemoveAt( 0 );
  }
  Invalidate();
  SetTimer( UPDATE_TIMER , UPDATE_LAPSE , NULL );
}

void CSketchView::UpdateGraph( CStringArray& NewGadgets )
{
  CStringArray SrcGadgets , DstGadgets;
  GetBuilder()->EnumGadgets( SrcGadgets , DstGadgets );
  int i = 0;
  while ( i < SrcGadgets.GetSize() )
  {
    CString tmpS = SrcGadgets.GetAt( i++ );
    if ( m_Graph.FindByUID( tmpS ) )
      SrcGadgets.RemoveAt( --i );
  }
  i = 0;
  while ( i < DstGadgets.GetSize() )
  {
    CString tmpS = DstGadgets.GetAt( i++ );
    if ( m_Graph.FindByUID( tmpS ) )
      DstGadgets.RemoveAt( --i );
  }
  NewGadgets.Append( SrcGadgets );
  NewGadgets.Append( DstGadgets );
  ComposeGraph( &SrcGadgets , &DstGadgets );
}

void CSketchView::UpdateProperties()
{
  FXString graphProps;
  if ( GetBuilder()->GetProperties( graphProps ) && (graphProps != m_LastGraphProps) )
  {
    m_LastGraphProps = graphProps;
    FXPropertyKit pk( m_LastGraphProps ) , uidPK;
    FXStringArray uids , props;
    pk.EnumKeys( uids , props );
    while ( uids.GetSize() )
    {
      FXString uid , text;
      uid = uids[ 0 ];
      pk.GetString( uid , text );
      uidPK = text;
      uids.RemoveAt( 0 );
      props.RemoveAt( 0 );
      CGlyph* pGlyph = m_Graph.FindByUID( uid );
      int x , y;
      if ( pGlyph && uidPK.GetInt( "x" , x ) && uidPK.GetInt( "y" , y ) )
      {
        CPoint pt( x , y ) , off = pGlyph->GetPos();
        pt -= off;
        pGlyph->OffsetPos( pt );
      }
    }
  }
}

bool CSketchView::InsertGadget()
{
  bool bResult = false;
  if ( GetBuilder() )
  {
    CStringArray Classes , Lineages;
    CUIntArray Types;
    GetBuilder()->EnumGadgetClassesAndLineages( Types , Classes , Lineages );
    CString ClassName = _T( "" );
    if ( Classes.GetSize() == 1 )
      ClassName = Classes.GetAt( 0 );
    else if ( Classes.GetSize() > 1 )
    {
      CSelectGadgetClassDlg dlg;
      dlg.SetGadgetsInfo( &Types , &Classes , &Lineages );
      if ( dlg.DoModal() == IDOK )
        ClassName = dlg.m_GadgetClass;
      else
        bResult = true;
    }
    if ( !bResult && GetBuilder()->CreateGadget( ClassName ) )
    {
      ::SetClassLongPtr( GetSafeHwnd() , GCLP_HCURSOR ,
        (LONG_PTR) LoadCursor( NULL , IDC_CROSS ) );
      return true;
    }
  }
  ::SetClassLongPtr( GetSafeHwnd() , GCLP_HCURSOR ,
    (LONG_PTR) LoadCursor( NULL , IDC_ARROW ) );
  return bResult;
}

bool CSketchView::InsertGadget( LPCTSTR lpszClassName , CPoint& ptScreen , LPCTSTR params , LPCTSTR uidopt )
{
  bool bResult = FALSE;
  KillTimer( UPDATE_TIMER );
  while ( !bResult )
  {
    CRect rc;
    if ( m_FloatWndManager.PtInside( ptScreen ) )
      break; // point outside graph site area
    GetClientRect( rc );
    ScreenToClient( &ptScreen );
    if ( !rc.PtInRect( ptScreen ) )
      break; // point outside graph site area
    if ( !GetBuilder()->CreateGadget( lpszClassName ) )
      break;
    m_Map.ViewToAbs( &ptScreen );
    UINT type;
    CGlyph* Glyph = InsertGadgetAt( ptScreen , &type , uidopt );
    if ( !Glyph )
      break;
    Glyph->SetIcon( DefineGlyphIcon( lpszClassName , type ) );
    DrawNewGlyph( Glyph );
    FXString uid , name;
    Glyph->GetUID( uid );
    if ( params )
    {
      bool Invalidate = false;
      CString text( params );
      if ( !GetBuilder()->ScanProperties( uid , text , Invalidate ) )
        break;
    }
    FXPropertyKit uidPK , pk;
    uidPK.WriteInt( "x" , ptScreen.x );
    uidPK.WriteInt( "y" , ptScreen.y );
    pk.WriteString( uid , LPCTSTR( uidPK ) );
    GetBuilder()->SetProperties( LPCTSTR( pk ) );
    GetBuilder()->GetScript( m_LastGraphScript );
    name.Format( "Insert %s %s %s" , lpszClassName , uid , ((params) ? params : "") );
    FixCurrentState( name );
    bResult = true;
  }
  SetTimer( UPDATE_TIMER , UPDATE_LAPSE , NULL );
  return bResult;
}

void CSketchView::RegisterSketchDragDrop()
{
  RevokeDragDrop( GetSafeHwnd()/*m_hWnd*/ );
  RegisterDragDrop( GetSafeHwnd()/*m_hWnd*/ , &m_DropTarget );
}

void CSketchView::RemoveGadget( CGlyph* pGlyph , FXString& uid )
{
  TRACE( "+++ It's about to delete Gadget '%s'\n" , uid );
  if ( GetBuilder() )
  {
    GetBuilder()->RemoveGlyph( uid );
    GetBuilder()->UnregisterGadget( uid );
  }
  m_FloatWndManager.DestroyChannel( uid );
}

void CSketchView::ClearSelected()
{
  m_pLastActiveGlyph = NULL;
  CRgn rgn;
  if ( m_SelectedGlyphs.GetRgn( &rgn ) )
  {
    CRgn* Rgn = m_Map.AbsToView( &rgn );
    rgn.DeleteObject();
    InvalidateRgn( Rgn , TRUE );
    delete Rgn;
  }
  while ( m_SelectedGlyphs.GetChildrenCount() )
  {
    CGlyph* pGlyph = m_SelectedGlyphs.GetChildAt( 0 ); m_SelectedGlyphs.Remove( pGlyph );

    pGlyph->CleanUp();

    if ( m_Graph.Remove( pGlyph ) ) delete pGlyph;
  }
  m_ToolTip.Pop();
  CString name = _T( "Clear selection" );
  FixCurrentState( name );
}

void CSketchView::DeleteSelected()
{
  m_pLastActiveGlyph = NULL;
  CRgn rgn;
  if ( m_SelectedGlyphs.GetRgn( &rgn ) )
  {
    CRgn* Rgn = m_Map.AbsToView( &rgn );
    rgn.DeleteObject();
    InvalidateRgn( Rgn , TRUE );
    delete Rgn;
  }
  while ( m_SelectedGlyphs.GetChildrenCount() )
  {
    CGlyph* pGlyph = m_SelectedGlyphs.GetChildAt( 0 );
    FXString UID ;
    bool bExists = pGlyph->GetUID( UID ) ;
    UINT uiKind = (bExists) ? GetBuilder()->KindOf( UID ) : 0 ;
    IGraphbuilder* pBuilder = GetBuilder() ;
    FXString path = pBuilder->GetID() + _T( '.' ) + UID ;
    CREATEGRAPHVIEW cgv;
    cgv.path = LPCTSTR( path ) ;
    cgv.builder = GetBuilder()->GetSubBuilder( UID );
    if ( bExists && cgv.path && cgv.builder )
    {
      CDRect pos ;  // zeros by default
      cgv.builder->ViewSectionSetGraphPos( pos ) ;
      AfxGetMainWnd()->SendMessage( VM_TVDB400_DELETESUBVIEW ,
        (WPARAM) uiKind , (LPARAM) &cgv );
    }

    m_SelectedGlyphs.Remove( pGlyph );
    m_VirtualGlyphs.Remove( pGlyph );

    pGlyph->DestroyIn();

    if ( m_Graph.Remove( pGlyph ) )
    {
      delete pGlyph;
    }
  }
  m_ToolTip.Pop();
  CString name = _T( "Delete selection" );
  FixCurrentState( name );
  m_pLastActiveGlyph = NULL;
}

BOOL CSketchView::Disconnect( FXString& uid )
{
  return GetBuilder()->Disconnect( uid );
}

BOOL CSketchView::Disconnect( FXString& uid1 , FXString& uid2 )
{
  return GetBuilder()->Disconnect( uid1 , uid2 );
}

void CSketchView::AggregateSelected( LPCTSTR uidopt , LPCTSTR name )
{
  if ( !m_SelectedGlyphs.GetChildrenCount() )
    return;
  m_pLastActiveGlyph = NULL;
  FXString uid = (uidopt) ? uidopt : (LPCTSTR) AutoBlockName();
  if ( uid.IsEmpty() )
    uid = AutoBlockName();
  uid.Replace( ' ' , '_' );
  CStringArray Gadgets;
  CPoint pt ;
  for ( int i = 0; i < m_SelectedGlyphs.GetChildrenCount(); i++ )
  {
    CGlyph* pChild = m_SelectedGlyphs.GetChildAt( i );
    if ( i == 0 )
      pt = pChild->GetPos() ;
    FXString uidChild;
    pChild->GetUID( uidChild );
    Gadgets.Add( uidChild );
  }
  m_SelectedGlyphs.DestroyIn();
  CStringArray uidsDebugWnd;
  m_Graph.EnumOpenDebugWnds( uidsDebugWnd );
  m_Graph.CleanUp();
  if ( name )
    GetBuilder()->Save( name , &Gadgets );
  GetBuilder()->AggregateBlock( uid , Gadgets , name );
  ComposeGraph();
  CGlyph* newOne = m_Graph.FindByUID( uid );
  if ( newOne )
  {
    CPoint offset;
    offset = newOne->GetPos();
    pt -= offset;
    newOne->OffsetPos( pt );
  }

  while ( uidsDebugWnd.GetSize() )
  {
    FXString uidWnd = (LPCTSTR) uidsDebugWnd.GetAt( 0 );
    uidsDebugWnd.RemoveAt( 0 );
    ToggleDebugRender( uidWnd );
  }
  Invalidate();
  CString stage = _T( "Aggregate selection" );
  FixCurrentState( stage );
}

void CSketchView::ExpandSelected()
{
  if ( !m_SelectedGlyphs.GetChildrenCount() ) return;
  m_pLastActiveGlyph = NULL;
  CStringArray uidsDebugWnd;
  m_Graph.EnumOpenDebugWnds( uidsDebugWnd );
  for ( int i = 0; i < m_SelectedGlyphs.GetChildrenCount(); i++ )
  {
    FXString uid;
    m_SelectedGlyphs.GetChildAt( i )->GetUID( uid );
    GetBuilder()->ExtractBlock( uid );
  }
  m_SelectedGlyphs.DestroyIn();
  m_Graph.CleanUp();
  ComposeGraph();
  Invalidate();
  while ( uidsDebugWnd.GetSize() )
  {
    FXString uidWnd = (LPCTSTR) uidsDebugWnd.GetAt( 0 );
    uidsDebugWnd.RemoveAt( 0 );
    ToggleDebugRender( uidWnd );
  }
  CString name = _T( "Expand selection" );
  FixCurrentState( name );
}

void CSketchView::InitUndoManager()
{
  m_UndoManager.Reset();
  FixCurrentState( "Basement" );
  ASSERT( m_pGraphBuilder != NULL );
  m_pGraphBuilder->SetDirty( FALSE );
}

void CSketchView::FlashGlyph( CGlyph* pGlyph )
{
  BOOL bFlashingNow = (m_FlashingGlyphs.GetSize() > 0);
  for ( int i = 0; i < 2 * TIMES_FLASHING; i++ )
    m_FlashingGlyphs.Add( pGlyph );
  if ( !bFlashingNow )
    SetTimer( FLASH_TIMER , FLASH_LAPSE , NULL );
}

void CSketchView::RenameGlyph( CGlyph* pGlyph , LPCTSTR uid )
{
  FXString uidOld;
  if ( pGlyph && pGlyph->GetUID( uidOld ) )
  {
    CStringArray uidsToChange , uidsNewNames;
    FXString uidNew( uid );
    if ( GetBuilder()->RenameGadget( uidOld , uidNew , &uidsToChange , &uidsNewNames ) )
    {
      ASSERT( uidNew == uid );
      ASSERT( uidsToChange.GetSize() == uidsNewNames.GetSize() );
      while ( uidsToChange.GetSize() )
      {
        uidOld = uidsToChange.GetAt( 0 );
        uidsToChange.RemoveAt( 0 );
        uidNew = uidsNewNames.GetAt( 0 );
        uidsNewNames.RemoveAt( 0 );
        pGlyph = m_Graph.FindByUID( uidOld );
        if ( pGlyph )
        {
          pGlyph->SetUID( uidNew );
          CGadget* pGadget = GetBuilder()->GetGadget( uidNew ) ;
          if ( pGadget )
          {
            pGadget->SetThreadName( uidNew );
            CRuntimeGadget* pRTGadget = pGadget->GetRuntimeGadget() ;
            if ( _tcscmp( pRTGadget->m_pBaseGadget->m_lpszClassName , "CRenderGadget" ) == 0 )
            {
              CString MonitorName = m_FloatWndManager.RenameChannel( uidOld , uidNew ) ;
              double x , y , w , h ;
              FXString Selected ;
              GetBuilder()->GetFloatWnd( MonitorName , x , y , w , h , &Selected ) ;
              if ( Selected == uidOld )
                GetBuilder()->SetFloatWnd( MonitorName , x , y , w , h , uid ) ;
            }
          }
          GetBuilder()->SetDirty();
        }
      }
    }
  }
}

CWnd* CSketchView::CreateRenderFrame( LPRECT rc , CRenderGlyph* pHost , CString& Monitor )
{
  FXString uid;
  if ( !pHost->GetUID( uid ) )
    return NULL;
  FXString mon;
  BOOL bMon = GetBuilder()->GetRenderMonitor( uid , mon ); // check if gadget wants any particular output
  if ( (bMon) && (_tcscmp( mon , LINEAGE_DEBUG ) == 0) )
  {
    return NULL; // The way of debug renderer creation is non standard
  }
  CRenderViewFrame* pWnd = new CRenderViewFrame( pHost );
  BOOL bCreateFloatWnd = TRUE;
  if ( !bMon ) // if not - let user select output
  {
    CInsertMonitorDlg dlg( uid );
    m_FloatWndManager.EnumMonitors( dlg.m_Monitors );
    if ( dlg.DoModal() != IDOK )
    {
      delete pWnd;
      return NULL;
    }
    if ( !dlg.GetMonitorName( Monitor ) )
    {
      Monitor = SET_INPLACERENDERERMONITOR;
      bCreateFloatWnd = FALSE;
    }
  }
  else
  {
    Monitor = mon;
    bCreateFloatWnd = Monitor != SET_INPLACERENDERERMONITOR;
  }
  // create output frame
  CRect rect( rc );
  if ( !bCreateFloatWnd )	// this is built-in window (create in-place)
  {
    m_Map.AbsToView( rect );
    if ( pWnd->Create( rect , this , TRUE ) )
      return pWnd;
  }
  else // this is floating window with title = Monitor OR automatically generated
  {
    CRect TmpRect( rc ) ;
    CFloatWnd* FloatWnd = m_FloatWndManager.CreateChannel( this , Monitor , uid , TmpRect );
    FloatWnd->GetClientRect( rect );
    if ( pWnd->Create( rect , (CWnd*) FloatWnd , FALSE ) )
    {
      m_FloatWndManager.SetChannelFrame( Monitor , uid , pWnd );
      return pWnd;
    }
  }
  delete pWnd;
  return NULL;
}

BOOL CSketchView::Undo( int steps )
{
  LPCTSTR script = m_UndoManager.UndoStart( steps );
  if ( script )
  {
    DeleteGraph();
    BOOL WasRunning = GetBuilder()->IsRuning();
    if ( WasRunning )
    {
      GetBuilder()->Stop();
    }
    GetBuilder()->Load( NULL , script );
    ComposeGraph();
    Invalidate();
    if ( WasRunning )
    {
      GetBuilder()->Start();
    }
    m_UndoManager.UndoEnd();
    return TRUE;
  }
  m_UndoManager.UndoEnd();
  return FALSE;
}

BOOL CSketchView::CanCopy()
{
  return (m_SelectedGlyphs.GetChildrenCount() > 0);
}

void CSketchView::Copy()
{
  CStringArray Gadgets;
  for ( int i = 0; i < m_SelectedGlyphs.GetChildrenCount(); i++ )
  {
    CGlyph* pChild = m_SelectedGlyphs.GetChildAt( i );
    FXString uidChild;
    pChild->GetUID( uidChild );
    Gadgets.Add( uidChild );
  }
  CString tmpFileName;
  GetTmpCopyFileName( tmpFileName );
  if ( tmpFileName.IsEmpty() )
    return;
  GetBuilder()->Save( tmpFileName , &Gadgets );
}

BOOL CSketchView::CanPaste()
{
  CString tmpFileName;
  GetTmpCopyFileName( tmpFileName );
  if ( tmpFileName.IsEmpty() )
    return FALSE;
  CFileStatus fs;
  return CFile::GetStatus( tmpFileName , fs );
}

void CSketchView::Paste()
{
  if ( !DoPaste() )
    return;
  FixCurrentState( "Paste selection" );
  Invalidate();
}

BOOL CSketchView::DoPaste()
{
  CString tmpFileName;
  GetTmpCopyFileName( tmpFileName );
  if ( tmpFileName.IsEmpty() )
    return FALSE;
  CStringArray NoGadgets;
  FXString uid = _T( "selection_copy" );
  if ( !GetBuilder()->AggregateBlock( uid , NoGadgets ) )
    return FALSE;
  IGraphbuilder* iBuilder = GetBuilder()->GetSubBuilder( uid );
  if ( !iBuilder )
    return FALSE;
  iBuilder->Load( tmpFileName );
  CMapStringToString Renames;
  GetBuilder()->ExtractBlock( uid , &Renames );
  m_SelectedGlyphs.DestroyIn();
  CStringArray NewGadgets;
  UpdateGraph( NewGadgets );
  BOOL bFirst = TRUE;
  CPoint minPt , offset;
  while ( NewGadgets.GetSize() )
  {
    CString uid = NewGadgets.GetAt( 0 );
    NewGadgets.RemoveAt( 0 );
    CString old;
    Renames.Lookup( uid , old );
    CGlyph* pGlyph = m_Graph.FindByUID( uid );
    if ( pGlyph )
    {
      //ASSERT(pGlyph);
      CGlyph* pGlyphSrc = m_Graph.FindByUID( old );
      if ( pGlyphSrc )
      {
        //ASSERT(pGlyphSrc);
        if ( bFirst )
        {
          offset = pGlyph->GetPos() - pGlyphSrc->GetPos();
          minPt = pGlyph->GetPos();
          bFirst = FALSE;
        }
        else
        {
          CPoint Off( pGlyphSrc->GetPos() - pGlyph->GetPos() + offset ) ;
          pGlyph->OffsetPos( Off );
          if ( minPt.x > pGlyph->GetPos().x )
            minPt.x = pGlyph->GetPos().x;
          if ( minPt.y > pGlyph->GetPos().y )
            minPt.y = pGlyph->GetPos().y;
        }
        m_SelectedGlyphs.Add( pGlyph );
      }
    }
  }
  minPt = -minPt ;
  m_SelectedGlyphs.OffsetPos( minPt );
  return TRUE;
}

void CSketchView::EmptyCopyBuffer()
{
  if ( !CanPaste() )
    return;
  CString tmpFileName;
  GetTmpCopyFileName( tmpFileName );
  if ( !tmpFileName.IsEmpty() )
    CFile::Remove( tmpFileName );
}

CGlyph* CSketchView::InsertGadgetAt( CPoint pt , UINT* type , LPCTSTR uidopt )
{
  FXString uidGadget( "" );
  if ( uidopt )
  {
    CStringArray Gadgets , DstGadgets;
    GetBuilder()->EnumGadgets( Gadgets , DstGadgets );
    Gadgets.Append( DstGadgets );
    uidGadget = uidopt;
    FXString base = uidGadget;
    base.TrimRight( "0123456789" );
    int count = 1;
    if ( uidGadget.GetLength() > base.GetLength() )
      count = atoi( uidGadget.Mid( base.GetLength() ) );
    bool bFound;
    do
    {
      bFound = false;
      uidGadget.Format( "%s%d" , base , count++ );
      for ( int i = 0; i < Gadgets.GetSize(); i++ )
      {
        if ( Gadgets.GetAt( i ) == (LPCTSTR) uidGadget )
        {
          bFound = true;
          Gadgets.RemoveAt( i );
          break;
        }
      }
    } while ( bFound );
  }

  UINT GadgetType = GetBuilder()->RegisterCurGadget( uidGadget );
  if ( type )
    *type = GadgetType;
  if ( GadgetType == IGraphbuilder::TVDB400_GT_ANY )
    return NULL; // failure
  CStringArray inputs , outputs , duplex;
  if ( !GetBuilder()->ListGadgetConnectors( uidGadget , inputs , outputs , duplex ) )
  {
    GetBuilder()->UnregisterGadget( uidGadget );
    return NULL;
  }
  CGadgetGlyph* pGlyph = NULL;
  if ( GadgetType == IGraphbuilder::TVDB400_GT_CTRL )
  {
    pGlyph = new CRenderGlyph( this , uidGadget , inputs , outputs , duplex , GetViewType() );

    if ( static_cast<CRenderGlyph*>(pGlyph)->IsConstructed() == false )
    {
      pGlyph->DestroyIn();
      delete pGlyph;
      pGlyph = NULL;
    }
  }
  else if ( GadgetType == IGraphbuilder::TVDB400_GT_COMPLEX )
    pGlyph = new CComplexGlyph( this , uidGadget , inputs , outputs , duplex , GetViewType() );
  else if ( GadgetType == IGraphbuilder::TVDB400_GT_PORT )
    pGlyph = new CPortalGlyph( this , uidGadget , inputs , outputs , duplex , GetViewType() );
  else
    pGlyph = new CGadgetGlyph( this , uidGadget , inputs , outputs , duplex , GetViewType() );
  if ( pGlyph )
  {
    switch ( GadgetType )
    {
      case IGraphbuilder::TVDB400_GT_CAPTURE: pGlyph->SetBodyColor( 0xd0d0ff ) ; break ;
      case IGraphbuilder::TVDB400_GT_FILTER: pGlyph->SetBodyColor( 0xd0ffd0 ) ; break ;
      case IGraphbuilder::TVDB400_GT_PORT: pGlyph->SetBodyColor( 0xd0d0d0 ) ; break ;
      case IGraphbuilder::TVDB400_GT_CTRL: pGlyph->SetBodyColor( 0xf0f0d0 ) ; break ;
        break ;

    }
    AddGlyphToGraph( pGlyph );
    pGlyph->OffsetPos( pt );
  }
  else
  {
    GetBuilder()->UnregisterGadget( uidGadget );
    SetTimer( UPDATE_TIMER , UPDATE_LAPSE , NULL );
  }
  return pGlyph;
}



void CSketchView::RedrawMouseAction()
{
  CClientDC dc( this );
  if ( m_MouseAction.IsLine() )
  {
    CRect rc( m_MouseAction.StartPoint() , m_MouseAction.EndPoint() );
    LOGPEN pen;
    GetMyPen( this , &pen );
    DrawLine( &dc , rc , &pen , R2_XORPEN );
  }
  else if ( m_MouseAction.IsRect() )
  {
    CPoint pt1 = m_MouseAction.StartPoint();
    CPoint pt3 = m_MouseAction.EndPoint();
    CPoint pt2( pt3.x , pt1.y );
    CPoint pt4( pt1.x , pt3.y );
    LOGPEN pen;
    GetMyPen( this , &pen );
    DrawLine( &dc , pt1 , pt2 , &pen , R2_XORPEN );
    DrawLine( &dc , pt2 , pt3 , &pen , R2_XORPEN );
    DrawLine( &dc , pt3 , pt4 , &pen , R2_XORPEN );
    DrawLine( &dc , pt4 , pt1 , &pen , R2_XORPEN );
  }
}

void CSketchView::DrawNewGlyph( CGlyph* pGlyph )
{
  m_SelectedGlyphs.DestroyIn();
  CRgn rgn;
  if ( pGlyph->GetRgn( &rgn ) )
  {
    CRgn* Rgn = m_Map.AbsToView( &rgn );
    rgn.DeleteObject();
    InvalidateRgn( Rgn , TRUE );
    delete Rgn;
  }
  Invalidate( FALSE );
}

BOOL CSketchView::ConnectGlyphs( CGlyph* pStartGlyph , CGlyph* pEndGlyph )
{
  return pStartGlyph->ConnectTo( pEndGlyph );
}

BOOL CSketchView::IsSelectedGlyph( void* user )
{
  FXString uid;
  ((CGlyph*) user)->GetUID( uid );
  if ( m_SelectedGlyphs.FindByUID( uid ) )
    return TRUE;
  return FALSE;
}

BOOL CSketchView::IsVirtualGlyph( void* user )
{
  FXString uid;
  ((CGlyph*) user)->GetUID( uid );
  if ( m_VirtualGlyphs.FindByUID( uid ) )
    return TRUE;
  return FALSE;
}

BOOL CSketchView::IsInvaldeGlyph( void* user )
{
  FXString uid;
  CGlyph* glyph = ((CGlyph*) user);
  if ( !glyph->IsGadgetGlyph() ) return FALSE;
  glyph->GetUID( uid );
  return !GetBuilder()->IsValid( uid );
}

void CSketchView::AddGlyphToGraph( CGlyph* pGlyph )
{
  m_Graph.Add( pGlyph );
}

void CSketchView::RemoveGlyphFromGraph( CGlyph* pGlyph )
{
  m_Graph.Remove( pGlyph );
  m_SelectedGlyphs.Remove( pGlyph );
  m_VirtualGlyphs.Remove( pGlyph );
}

CString CSketchView::AutoBlockName()
{
  static int idBlock = 0;
  CString uid;
  uid.Format( "Block%d" , ++idBlock );
  return uid;
}

int CSketchView::DefineGlyphIcon( LPCTSTR strGadgetClass , UINT uGadgetType )
{
  const int nBaseIcons = 11;
  int nSpecIcons = sizeof( specIcons ) / sizeof( char* );
  while ( nSpecIcons-- )
  {
    if ( !_tcscmp( specIcons[ nSpecIcons ] , strGadgetClass ) )
      break;
  }
  if ( nSpecIcons >= 0 )
    return 2 * nSpecIcons + nBaseIcons;
  switch ( uGadgetType )
  {
  case IGraphbuilder::TVDB400_GT_CAPTURE:
    return 0;
  case IGraphbuilder::TVDB400_GT_FILTER:
    return 1;
  case IGraphbuilder::TVDB400_GT_CTRL:
    return 2;
  case IGraphbuilder::TVDB400_GT_COMPLEX:
    return 4;
  case IGraphbuilder::TVDB400_GT_OTHER:
    return 1;
  }
  return -1;
}

void CSketchView::RedrawGlyph( CGlyph* pGlyph )
{
  CRgn rgn;
  if ( pGlyph->GetRgn( &rgn ) )
  {
    CRgn* Rgn = m_Map.AbsToView( &rgn );
    InvalidateRgn( Rgn );
    rgn.DeleteObject();
    delete Rgn;
  }
}

void CSketchView::FixCurrentState( LPCTSTR name )
{
  FXString builderScript;
  FXString viewScript;
  if ( GetBuilder()->GetScript( builderScript ) )
  {
    m_UndoManager.AddState( builderScript , name );
  }
}

//////////////////////////////////////////////////////////////////////
// Debug renderer (tester)
//////////////////////////////////////////////////////////////////////

void CSketchView::SetDebugMode( BOOL on )
{
  m_bDebugging = on;
  if ( !m_bDebugging )
  {
    DisconnectDebugRender();
    DeleteDebugRender();
  }
  else
    CreateDebugRender( this );
};

BOOL CSketchView::CreateDebugRender( CWnd* pParentWnd )
{
  DeleteDebugRender();

  if ( !GetBuilder()->CreateGadget( RUNTIME_GADGET( CDebugRender )->m_lpszClassName ) )
  {
    if ( !((IGraphbuilder*) GetBuilder())->RegisterGadgetClass( RUNTIME_GADGET( CDebugRender ) ) ||
      !GetBuilder()->CreateGadget( RUNTIME_GADGET( CDebugRender )->m_lpszClassName ) )
      return FALSE;
  }
  FXString uid( UID_DEBUG_RENDER );
  if ( GetBuilder()->RegisterCurGadget( uid ) != IGraphbuilder::TVDB400_GT_CTRL )
    return FALSE;
  CStringArray inputs , outputs , duplex;
  if ( !GetBuilder()->ListGadgetConnectors( uid , inputs , outputs , duplex ) )
  {
    GetBuilder()->UnregisterGadget( uid );
    return FALSE;
  }
  m_DebugWnd = new CDebugViewDlg;
  m_DebugWnd->Create( this );
  m_DebugWnd->ShowWindow( SW_SHOW );
  CDebugRender* rg = (CDebugRender*) GetBuilder()->GetGadget( UID_DEBUG_RENDER );
  if ( rg )
    rg->Attach( (CWnd*) m_DebugWnd );
  return TRUE;
}

void CSketchView::DeleteDebugRender()
{
  if ( m_DebugWnd )
  {
    FXString uid( UID_DEBUG_RENDER );
    if ( GetBuilder() ) GetBuilder()->UnregisterGadget( uid );
    m_DebugWnd->DestroyWindow();
    delete m_DebugWnd;
    m_DebugWnd = NULL;
  }
}

void CSketchView::DisconnectDebugRender()
{
  FXString uid;
  if ( (!m_DebugWnd) || (!m_DebugWnd->GetGadget( GetBuilder() )) || (!m_DebugWnd->GetUID( uid )) )
    return;
  CStringArray inputs , outputs , duplex;
  CRenderGadget* dr = m_DebugWnd->GetGadget( GetBuilder() );
  if ( !GetBuilder()->ListGadgetConnectors( uid , inputs , outputs , duplex ) || !inputs.GetSize() )
    return;
  if ( dr )
  {
    CStringArray uidsTo;
    if ( GetBuilder()->IsConnected( inputs[ 0 ] , uidsTo ) )
      GetBuilder()->Disconnect( inputs[ 0 ] );
  }
}

BOOL CSketchView::ConnectDebugRender( FXString& uidOutput )
{
  FXString uid;
  if ( (!m_DebugWnd) || (!m_DebugWnd->GetGadget( GetBuilder() )) || (!m_DebugWnd->GetUID( uid )) )
    return FALSE;

  CStringArray inputs , outputs , duplex;
  if ( !GetBuilder()->ListGadgetConnectors( uid , inputs , outputs , duplex ) || !inputs.GetSize() )
    return FALSE;
  FXString uidOut;
  if ( GetBuilder()->GetOutputIDs( uidOutput , uidOut , NULL ) )
    return GetBuilder()->Connect( uidOut , FXString( inputs[ 0 ] ) );
  return FALSE;
}

void CSketchView::ToggleDebugRender( const FXString& uid , BOOL bDestroy )
{
  CGlyph* pGlyph = m_Graph.FindByUID( uid );
  if ( pGlyph )
  {
    CWnd* DebugWnd = pGlyph->GetDebugWnd();
    if ( DebugWnd )
    {
      FXString uidOut = uid , uidIn = uid;
      uidOut.Replace( ">" , "&gt;" );
      uidOut.Replace( "<" , "&lt;" );
      uidOut += _T( "_Output" );
      uidIn.Replace( ">" , "&gt;" );
      uidIn.Replace( "<" , "&lt;" );
      uidIn += _T( "_Input" );
      if ( bDestroy )
      {
        GetBuilder()->UnregisterGadget( uidOut );
        GetBuilder()->UnregisterGadget( uidIn );
        DebugWnd->DestroyWindow();
        return;
      }
      if ( DebugWnd->IsWindowVisible() )
      {
        CStringArray inputs , outputs , duplex;
        if ( GetBuilder()->ListGadgetConnectors( uidOut , inputs , outputs , duplex ) && inputs.GetSize() )
        {
          CString input = inputs.GetAt( 0 );
          GetBuilder()->Disconnect( input );
        }
        inputs.RemoveAll();
        if ( GetBuilder()->ListGadgetConnectors( uidIn , inputs , outputs , duplex ) && inputs.GetSize() )
        {
          CString input = inputs.GetAt( 0 );
          GetBuilder()->Disconnect( input );
        }
        DebugWnd->ShowWindow( SW_HIDE );
      }
      else
      {
        FXString uidOutPin;
        CStringArray uidInComplementary;
        if ( GetBuilder()->GetOutputIDs( uid , uidOutPin , &uidInComplementary ) )
        {
          if ( uidOutPin.IsEmpty() )
            ((CDebugEnvelopDlg*) DebugWnd)->m_pOutFrame->ShowWindow( SW_HIDE );
          else
          {
            CTiedDebugRender* Render = (CTiedDebugRender*) GetBuilder()->GetGadget( uidOut );
            if ( !Render )
            {
              if ( !GetBuilder()->CreateGadget( RUNTIME_GADGET( CTiedDebugRender )->m_lpszClassName ) )
              {
                if ( !GetBuilder()->RegisterGadgetClass( RUNTIME_GADGET( CTiedDebugRender ) ) ||
                  !GetBuilder()->CreateGadget( RUNTIME_GADGET( CTiedDebugRender )->m_lpszClassName ) )
                  return;
              }
              if ( GetBuilder()->RegisterCurGadget( uidOut ) != IGraphbuilder::TVDB400_GT_CTRL )
                return;
              Render = (CTiedDebugRender*) GetBuilder()->GetGadget( uidOut );
              if ( !Render )
                return;
            }
            CStringArray inputs , outputs , duplex;
            if ( !GetBuilder()->ListGadgetConnectors( uidOut , inputs , outputs , duplex ) || !inputs.GetSize() )
              return;
            Render->Attach( ((CDebugEnvelopDlg*) DebugWnd)->m_pOutFrame );
            ((CDebugEnvelopDlg*) DebugWnd)->SetOutGadget( Render );
            FXString input = (FXString) inputs.GetAt( 0 );
            if ( !GetBuilder()->Connect( uidOutPin , input ) )
              return;
          }
          if ( !uidInComplementary.GetSize() )
            ((CDebugEnvelopDlg*) DebugWnd)->m_pInFrame->ShowWindow( SW_HIDE );
          else
          {
            CTiedDebugRender* Render = (CTiedDebugRender*) GetBuilder()->GetGadget( uidIn );
            if ( !Render )
            {
              if ( !GetBuilder()->CreateGadget( RUNTIME_GADGET( CTiedDebugRender )->m_lpszClassName ) )
              {
                if ( !GetBuilder()->RegisterGadgetClass( RUNTIME_GADGET( CTiedDebugRender ) ) ||
                  !GetBuilder()->CreateGadget( RUNTIME_GADGET( CTiedDebugRender )->m_lpszClassName ) )
                  return;
              }
              if ( GetBuilder()->RegisterCurGadget( uidIn ) != IGraphbuilder::TVDB400_GT_CTRL )
                return;
              Render = (CTiedDebugRender*) GetBuilder()->GetGadget( uidIn );
              if ( !Render )
                return;
            }
            CStringArray inputs , outputs , duplex;
            if ( !GetBuilder()->ListGadgetConnectors( uidIn , inputs , outputs , duplex ) || !inputs.GetSize() )
              return;
            Render->Attach( ((CDebugEnvelopDlg*) DebugWnd)->m_pInFrame );
            ((CDebugEnvelopDlg*) DebugWnd)->SetInGadget( Render );
            while ( uidInComplementary.GetSize() )
            {
              FXString pin = (FXString) uidInComplementary.GetAt( 0 );
              uidInComplementary.RemoveAt( 0 );
              FXString input = (FXString) inputs.GetAt( 0 );
              GetBuilder()->Connect( pin , input );
            }
          }
          DebugWnd->ShowWindow( SW_SHOW );
          ((CDebugEnvelopDlg*) DebugWnd)->ArrangeWindows();
          DebugWnd->BringWindowToTop();
        }
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////
// Draw functions
/////////////////////////////////////////////////////////////////////////

void CSketchView::Paint()
{
  CPaintDC pDC( this ); // device context for painting
  CMemDC dc( &pDC );
  CRect    rc;

  CPoint offset;
  GetClientRect( rc );
  m_ViewRect = CRect( 0 , 0 , 0 , 0 );

  //calculate boundrect BEFORE drawing the grapn to coorect offset for scrollbars
  CRect rcTemp;
  CRgn rgnTemp , rgnTemp2;
  m_Graph.GetRgn( &rgnTemp );
  if ( rgnTemp.m_hObject )
    rgnTemp.GetRgnBox( m_ViewRect );
  m_SelectedGlyphs.GetRgn( &rgnTemp2 );
  if ( rgnTemp2.m_hObject )
    rgnTemp2.GetRgnBox( rcTemp );
  m_ViewRect.UnionRect( m_ViewRect , rcTemp );
  //convert to scale
  m_Map.AbsToViewScale( m_ViewRect );

  //correct offset
  offset = m_Map.GetOffset();
  //area left to zero or to the most left gadget should not be visible
  if ( offset.x > 0 && -offset.x < m_ViewRect.left )
    offset.x = -m_ViewRect.left;
  //area right to the default right window border (rc.right) or to the most right gadget should not be visble
  if ( offset.x < 0 && -offset.x + rc.right > m_ViewRect.right )
    offset.x = rc.right - m_ViewRect.right;
  //the same for vertical limits
  if ( offset.y > 0 && -offset.y < m_ViewRect.top )
    offset.y = -m_ViewRect.top;
  if ( offset.y < 0 && -offset.y + rc.bottom > m_ViewRect.bottom )
    offset.y = rc.bottom - m_ViewRect.bottom;
  m_Map.SetOffset( offset );

  //fraw gadgets
  m_ViewRect = CRect( 0 , 0 , 0 , 0 );
  m_Graph.Draw( &dc );
  m_SelectedGlyphs.Draw( &dc );

  //convert to scale
  m_Map.AbsToViewScale( m_ViewRect );

  if ( m_ViewRect.right > rc.right || m_ViewRect.left < rc.left )
  {
    ShowScrollBar( SB_HORZ );
    GetClientRect( rc );
    SCROLLINFO info;
    info.cbSize = sizeof( SCROLLINFO );
    info.fMask = SIF_PAGE | SIF_RANGE;
    info.nMin = min( rc.left , m_ViewRect.left );
    info.nMax = max( rc.right , m_ViewRect.right );//+VIEW_GAP; 
    info.nPage = rc.right - rc.left;
    info.nPos = 0;
    info.nTrackPos = 0;
    SetScrollInfo( SB_HORZ , &info , TRUE );
  }
  else
  {
    SetScrollPos( SB_HORZ , 0 , FALSE );
    ShowScrollBar( SB_HORZ , FALSE );
    m_Map.ResetOffsetX();
  }
  if ( m_ViewRect.bottom > rc.bottom || m_ViewRect.top < rc.top )
  {
    ShowScrollBar( SB_VERT );
    GetClientRect( rc );
    SCROLLINFO info;
    info.cbSize = sizeof( SCROLLINFO );
    info.fMask = SIF_PAGE | SIF_RANGE;
    info.nMin = min( rc.top , m_ViewRect.top );
    info.nMax = max( rc.bottom , m_ViewRect.bottom );//+VIEW_GAP;
    info.nPage = rc.bottom - rc.top;
    info.nPos = 0;
    info.nTrackPos = 0;
    SetScrollInfo( SB_VERT , &info , TRUE );
  }
  else
  {
    SetScrollPos( SB_VERT , 0 , FALSE );
    ShowScrollBar( SB_VERT , FALSE );
    m_Map.ResetOffsetY();
  }
  if ( m_CopyRgn.m_hObject )
  {
    CRgn* Rgn = m_Map.AbsToView( &m_CopyRgn );
    dc.FrameRgn( Rgn , m_GDI.GetBrush( BS_SOLID , 0 , RGB( 200 , 200 , 255 ) ) , 1 , 1 );
    delete Rgn;
  }
  if ( m_bWasRescaled )
    m_bWasRescaled = FALSE;
  if ( m_bRescaled )
  {
    m_bRescaled = FALSE;
    m_bWasRescaled = TRUE;
    Invalidate(); // comment this line to get redraw bug for inplace renders
  }
}

void CSketchView::OnPaint()
{
  Paint();
}

void CSketchView::DrawRect( CDC* pDC , LPRECT rect , COLORREF penColor , COLORREF bgColor )
{
  ASSERT( pDC );
  CPen* OldPen = pDC->SelectObject( m_GDI.GetPen( PS_SOLID , 1 , penColor ) );
  CBrush* OldBrush = pDC->SelectObject( m_GDI.GetBrush( BS_SOLID , 0 , bgColor ) );
  CRect rc( rect );
  m_ViewRect |= rc;
  m_Map.AbsToView( rc );
  pDC->Rectangle( rc );
  pDC->SelectObject( OldBrush );
  pDC->SelectObject( OldPen );
}

void CSketchView::DrawRoundRect( CDC* pDC , LPRECT rect , 
  COLORREF penColor , COLORREF bgColor , CPoint * pEllipse )
{
  ASSERT( pDC );
  CPen* OldPen = pDC->SelectObject( m_GDI.GetPen( PS_SOLID , 1 , penColor ) );
  CBrush* OldBrush = pDC->SelectObject( m_GDI.GetBrush( BS_SOLID , 0 , bgColor ) );
  CRect rc( rect );
  m_Map.AbsToView( rc );
  CPoint ptEllipse = pEllipse ? *pEllipse : CPoint( 3 , 3 );
  pDC->RoundRect( rc , ptEllipse );
  pDC->SelectObject( OldBrush );
  pDC->SelectObject( OldPen );
}

void CSketchView::DrawBigRoundRect( CDC* pDC , LPRECT rect , COLORREF penColor , COLORREF bgColor )
{
  ASSERT( pDC );
  CPen* OldPen = pDC->SelectObject( m_GDI.GetPen( PS_SOLID , 1 , penColor ) );
  CBrush* OldBrush = pDC->SelectObject( m_GDI.GetBrush( BS_SOLID , 0 , bgColor ) );
  CRect rc( rect );
  m_ViewRect |= rc;
  m_Map.AbsToView( rc );
  CPoint ptEllipse( 7 , 7 );
  pDC->RoundRect( rc , ptEllipse );
  pDC->SelectObject( OldBrush );
  pDC->SelectObject( OldPen );
}

void CSketchView::DrawEdgeRect( CDC* pDC , LPRECT rect , COLORREF penColor , COLORREF bgColor )
{
  ASSERT( pDC );
  CPen* OldPen = pDC->SelectObject( m_GDI.GetPen( PS_SOLID , 1 , bgColor ) );
  CBrush* OldBrush = pDC->SelectObject( m_GDI.GetBrush( BS_SOLID , 0 , bgColor ) );
  CRect rc( rect );
  m_ViewRect |= rc;
  m_Map.AbsToView( rc );
  pDC->Rectangle( rc );
  pDC->DrawEdge( rc , (UINT) penColor , BF_RECT );
  pDC->SelectObject( OldBrush );
  pDC->SelectObject( OldPen );
}

void CSketchView::DrawLine( CDC* pDC , LPRECT rect , LOGPEN* pen , int rop )
{
  ASSERT( pDC );
  int penStyle = (pen) ? pen->lopnStyle : PS_SOLID;
  int penWidth = (pen) ? pen->lopnWidth.x : 1;
  COLORREF penColor = (pen) ? pen->lopnColor : RGB( 0 , 0 , 0 );
  CPen* OldPen = pDC->SelectObject( m_GDI.GetPen( penStyle , penWidth , penColor ) );
  int OldRop = pDC->SetROP2( rop );
  CRect rc( rect );
  m_Map.AbsToView( rc );
  pDC->MoveTo( rc.left , rc.top );
  pDC->LineTo( rc.right , rc.bottom );
  pDC->SelectObject( OldPen );
  pDC->SetROP2( OldRop );
}

void CSketchView::DrawLine( CDC* pDC , CPoint& pt1 , CPoint& pt2 , LOGPEN* pen , int rop )
{
  CRect rc( pt1 , pt2 );
  DrawLine( pDC , rc , pen , rop );
}

void CSketchView::DrawText( CDC* pDC , LPCTSTR text , LPRECT rect , COLORREF color , LOGFONT* font )
{
  ASSERT( pDC );
  CFont Font;
  if ( !Font.CreateFontIndirect( font ) )
    return;
  CRect rc( rect );
  m_Map.AbsToView( rc );
#ifdef GLYPHS_DRAW_EDGE
  rc.DeflateRect( 1 , 1 );
#endif
  CFont* OldFont = pDC->SelectObject( &Font );
  COLORREF oldColor = pDC->SetTextColor( color );
  int oldMode = GetBkMode( pDC->GetSafeHdc() );
  SetBkMode( pDC->GetSafeHdc() , TRANSPARENT );
  CString tmpS = text;
  CString outS;
  CSize sz = pDC->GetTextExtent( tmpS );

  pDC->SetTextJustification( 0 , 0 );
  int oneChar = sz.cx / tmpS.GetLength();
  if ( (sz.cx + oneChar) > (rc.right - rc.left) )
  {
    for ( int i = 0; i < tmpS.GetLength(); i++ )
    {
      outS += tmpS[ i ];
      CSize sz = pDC->GetTextExtent( outS );
      if ( sz.cx + oneChar >= (rc.right - rc.left) )
      {
        tmpS.Insert( i + 1 , EOL ); i++;
        outS.Empty();
      }
    }
  }
  pDC->DrawText( tmpS , -1 , rc , DT_TOP | DT_WORDBREAK );
  pDC->SetTextColor( oldColor );
  pDC->SelectObject( OldFont );
  Font.DeleteObject();
  SetBkMode( pDC->GetSafeHdc() , oldMode );
}

void CSketchView::DrawIcon( CDC* pDC , int idIcon , LPRECT rect , 
  COLORREF* penColor , COLORREF* bgColor , LPRECT rcResult )
{
  ASSERT( pDC );
  if ( idIcon < 0 || idIcon >= m_GlyphIcons.GetImageCount() )
    return;
  CRect rc( rect );
  IMAGEINFO ii;
  m_GlyphIcons.GetImageInfo( idIcon , &ii );
  CPoint pt;
  //pt.x = (rc.right + rc.left + ii.rcImage.left - ii.rcImage.right) / 2;
  //pt.y = (rc.bottom + rc.top + ii.rcImage.top - ii.rcImage.bottom) / 2;
  pt.x = rc.right + ii.rcImage.left - ii.rcImage.right - 4 ;
  pt.y = rc.bottom + ii.rcImage.top - ii.rcImage.bottom - 3 ;
  if ( penColor && bgColor )
  {
    CRect rc1( ii.rcImage );
    rc1.OffsetRect( -ii.rcImage.left , -ii.rcImage.top );
    rc1.OffsetRect( pt );
    rc1.InflateRect( 2 , 2 );
#ifndef GLYPHS_DRAW_EDGE
    DrawRoundRect( pDC , rc1 , *penColor , *bgColor );
#else
    if ( *penColor == EDGE_BUMP )
      pDC->DrawEdge( rc1 , EDGE_RAISED , BF_RECT );
    else
      pDC->DrawEdge( rc1 , *penColor , BF_RECT );
#endif
  }
  m_Map.AbsToView( &pt );
  m_GlyphIcons.Draw( pDC , idIcon , pt , ILD_NORMAL );
  if ( rcResult )
  {
    rcResult->left = pt.x;
    rcResult->top = pt.y;
    rcResult->right = pt.x + ii.rcImage.right - ii.rcImage.left;
    rcResult->bottom = pt.y + ii.rcImage.bottom - ii.rcImage.top;
  }
}

void CSketchView::DrawModeIcon( CDC* pDC , int idIcon , LPRECT rect )
{
  if ( idIcon < 0 || idIcon >= m_ModesIcons.GetImageCount() )
    return;
  CRect rc( rect );
  IMAGEINFO ii;
  m_ModesIcons.GetImageInfo( idIcon , &ii );
  CPoint pt;
  pt.x = (rc.right + rc.left + ii.rcImage.left - ii.rcImage.right) / 2;
  pt.y = (rc.bottom + rc.top + ii.rcImage.top - ii.rcImage.bottom) / 2;
  m_Map.AbsToView( &pt );
  m_ModesIcons.Draw( pDC , idIcon , pt , ILD_NORMAL );
}

BOOL CSketchView::GetMyPen( void* user , LOGPEN* pen )
{
  memset( pen , 0 , sizeof( LOGPEN ) );
  if ( user == this )
  {
    pen->lopnColor = NORMAL_BORDER_COLOR;
    pen->lopnStyle = PS_DOT;
    pen->lopnWidth.x = 1;
    return TRUE;
  }
  BOOL bVirtual = IsVirtualGlyph( user );
  if ( IsSelectedGlyph( user ) )
  {
    pen->lopnColor = (bVirtual) ? SELECT_VIRTUAL_COLOR : SELECT_BORDER_COLOR;
    pen->lopnStyle = PS_SOLID;
    pen->lopnWidth.x = 1;
    return TRUE;
  }
  else
  {
    pen->lopnColor = (bVirtual) ? VIRTUAL_BORDER_COLOR : NORMAL_BORDER_COLOR;
    pen->lopnStyle = PS_SOLID;
    pen->lopnWidth.x = 1;
    return TRUE;
  }
  return FALSE; // reserved for invisible users
}

//#define NO_DATA_PASSED 0
//#define DATA_PASSED    1
//#define DATA_SKIPPED   2


BOOL CSketchView::GetMyRectColors( void* user , COLORREF* crBorder , COLORREF* crBody , int isDataPassed /* = NO_DATA_PASSED*/ )
{
  BOOL IsInvalde = IsInvaldeGlyph( user );
  BOOL bVirtual = IsVirtualGlyph( user );
  if ( IsInvalde )
  {
    *crBorder = INVALIDE_BORDER_COLOR;
    if ( isDataPassed == DATA_SKIPPED )
      *crBody = SKIPPED_INNER_COLOR;
    else if ( isDataPassed == DATA_PASSED )
      *crBody = DATA_INNER_COLOR;
    else
      *crBody = INVALIDE_INNER_COLOR;
    return TRUE;
  }
  else if ( IsSelectedGlyph( user ) )
  {
    *crBorder = (bVirtual) ? SELECT_VIRTUAL_COLOR : SELECT_BORDER_COLOR;
    if ( isDataPassed == DATA_SKIPPED )
      *crBody = SKIPPED_INNER_COLOR;
    else if ( isDataPassed == DATA_PASSED )
      *crBody = DATA_INNER_COLOR;
    else
      *crBody = SELECT_INNER_COLOR;
    return TRUE;
  }
  else
  {
    *crBorder = (bVirtual) ? VIRTUAL_BORDER_COLOR : NORMAL_BORDER_COLOR;
    if ( isDataPassed == DATA_SKIPPED )
      *crBody = SKIPPED_INNER_COLOR;
    else if ( isDataPassed == DATA_PASSED )
      *crBody = DATA_INNER_COLOR;
    else
      *crBody = NORMAL_INNER_COLOR;
    return TRUE;
  }
  return FALSE; // reserved for invisible users
}

BOOL CSketchView::GetMyFont( void* user , COLORREF* crText , LOGFONT* font )
{
  BOOL bVirtual = IsVirtualGlyph( user );
  if ( IsSelectedGlyph( user ) )
  {
    *crText = (bVirtual) ? SELECT_VIRTUAL_COLOR : SELECT_BORDER_COLOR;
    m_GadgetsFont.GetLogFont( font );
    return TRUE;
  }
  else
  {
    *crText = (bVirtual) ? VIRTUAL_BORDER_COLOR : NORMAL_BORDER_COLOR;
    m_GadgetsFont.GetLogFont( font );
    return TRUE;
  }
  return FALSE; // reserved for invisible users
}

int CSketchView::GetMyIcon( void* user )
{
  int idIcon = ((CGlyph*) user)->GetIcon();
  if ( idIcon < -1 )
    return -1;
  if ( IsSelectedGlyph( user ) )
  {
    if ( idIcon < 5 )
      return idIcon + 6;
    return idIcon + 1;
  }
  else
    return idIcon;
  return -1;
}

int CSketchView::GetMyModeIcon( void* user )
{
  FXString uid;
  if ( !((CGlyph*) user)->GetUID( uid ) || !GetBuilder() )
    return -1;
  int mode;
  if ( !GetBuilder()->GetGadgetMode( uid , mode ) )
    return -1;
  switch ( mode )
  {
  case CGadget::mode_reject:
    return 0;
  case CGadget::mode_transmit:
    return 1;
  case CGadget::mode_process:
    return 2;
  }
  return -1;
}


/////////////////////////////////////////////////////////////////////////
// Interface and GDI functions
/////////////////////////////////////////////////////////////////////////
void CSketchView::ShowScrollBar( UINT nBar , BOOL bShow )
{
  if ( bShow )
  {
    switch ( nBar )
    {
    case SB_HORZ:
      m_BarsEnabled |= SCR_HORZ;
    case SB_VERT:
      m_BarsEnabled |= SCR_VERT;
    case SB_BOTH:
      m_BarsEnabled |= SCR_VERT | SCR_HORZ;
    }
  }
  else
  {
    switch ( nBar )
    {
    case SB_HORZ:
      m_BarsEnabled &= ~SCR_HORZ;
    case SB_VERT:
      m_BarsEnabled &= ~SCR_VERT;
    case SB_BOTH:
      m_BarsEnabled &= ~SCR_VERT | SCR_HORZ;
    }
  }
  CWnd::ShowScrollBar( nBar , bShow );
}

BOOL CSketchView::SetScrollInfo( int nBar , LPSCROLLINFO lpScrollInfo , BOOL bRedraw )
{
  if ( nBar == SB_HORZ ) m_XPage = lpScrollInfo->nPage;
  if ( nBar == SB_VERT ) m_YPage = lpScrollInfo->nPage;
  return CWnd::SetScrollInfo( nBar , lpScrollInfo , bRedraw );
}

void CSketchView::OnTimer( UINT_PTR nIDEvent )
{
  if ( nIDEvent == FLASH_TIMER )
  {
    int iCntr = 0 ;
    while ( m_FlashingGlyphs.GetCount() && (iCntr <= 3))
    {
      CGlyph* pFlashingGlyph = (CGlyph*) m_FlashingGlyphs.GetAt( 0 );
      if ( !pFlashingGlyph )
        break ;
      if ( IsSelectedGlyph( pFlashingGlyph ) )
        m_SelectedGlyphs.Remove( pFlashingGlyph );
      else
        m_SelectedGlyphs.Add( pFlashingGlyph );
      m_FlashingGlyphs.RemoveAt( 0 );
      iCntr++ ;
    }
    Invalidate( FALSE );
    if ( !m_FlashingGlyphs.GetSize() )
      KillTimer( FLASH_TIMER );
  }
  else if ( nIDEvent == UPDATE_TIMER )
  {
    /*		CString script; // Removed A.Ch. 09.02.2011
    if (GetBuilder() && GetBuilder()->GetScript(script))
    {
    UpdateProperties();
    Invalidate(FALSE);
    } */
    CStringArray uids;
    LockingFIFO<FXString> * pUIDs =
      ( LockingFIFO<FXString> * )m_pGraphBuilder->GetModifiedUIDSsArray() ;
    if ( pUIDs && pUIDs->size() )
    {

      FXString uid ;
      while ( pUIDs->getfront( uid ) )
      {
        CGlyph* pGlyph = m_Graph.FindByUID( uid );
        if ( pGlyph )
          pGlyph->SetNeedUpdate();
      }
    }

    if ( GetBuilder() && (m_UndoManager.GetChanges() != m_LastShownChange) )
    {
      while ( uids.GetSize() )
      {
        CString uid = uids.GetAt( 0 );
        uids.RemoveAt( 0 );
        CGlyph* Glyph = m_Graph.FindByUID( uid );
        if ( Glyph )
          Glyph->SetNeedUpdate();
      }
      m_LastShownChange = m_UndoManager.GetChanges();
    }
    Invalidate( TRUE );
    KillTimer( UPDATE_TIMER ) ;
  }
  else if ( nIDEvent == CONNECTOR_STATE_TIMER )
  {
    if ( m_ViewActivity )
    {
      //Invalidate(FALSE);
      UpdateActivity();
    }
    if ( m_pGraphBuilder )
    {
      LockingFIFO<FXString> * pUIDs =
        ( LockingFIFO<FXString> * )m_pGraphBuilder->GetModifiedUIDSsArray() ;
      if ( pUIDs && pUIDs->size() )
      {
//         SetTimer( UPDATE_TIMER , UDATE_LAPSE_FOR_GROUP , NULL ) ;
        FXString uid ;
        while ( pUIDs->getfront( uid ) )
        {
          CGlyph* pGlyph = m_Graph.FindByUID( uid );
          if ( pGlyph )
            pGlyph->SetNeedUpdate();
        }
        Invalidate( TRUE );
      }
    }
  }
  CWnd::OnTimer( nIDEvent );
}

void    CSketchView::UpdateActivity()
{
  CDC* dc = GetDC();
  //CMemDC mDc(dc);
  //m_Graph.Draw(mDc, this);
  m_Graph.Draw( dc , true );
  ReleaseDC( dc );
}

void CSketchView::OnVScroll( UINT nSBCode , UINT nPos , CScrollBar* pScrollBar )
{
  int     iMax;
  int     iMin;
  int     iPos;
  int     dn;
  RECT    rc;

  if ( m_BarsEnabled & SCR_VERT )
  {
    GetScrollRange( SB_VERT , &iMin , &iMax );
    iPos = GetScrollPos( SB_VERT );
    GetClientRect( &rc );
    switch ( nSBCode )
    {
    case SB_LINEDOWN:
      dn = LINE_SCROLL_DISTANCE;
      break;
    case SB_LINEUP:
      dn = -LINE_SCROLL_DISTANCE;
      break;
    case SB_PAGEDOWN:
      dn = rc.bottom / 4 + 1;
      break;
    case SB_PAGEUP:
      dn = -rc.bottom / 4 + 1;
      break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
      dn = nPos - iPos;
      break;
    default:
      dn = 0;
      break;
    }
    if ( dn = BOUND( iPos + dn , iMin , iMax - (int) m_YPage ) - iPos )
    {
      m_Map.MoveMap( 0 , -dn );
      SetScrollPos( SB_VERT , iPos + dn , TRUE );
      UpdateWindow();
      Invalidate( TRUE );
      GetBuilder()->ViewSectionSetViewOffset( m_Map.GetOffset() ) ;
    }
  }
}

void CSketchView::OnHScroll( UINT nSBCode , UINT nPos , CScrollBar* pScrollBar )
{
  int     iMax;
  int     iMin;
  int     iPos;
  int     dn;
  RECT    rc;

  if ( m_BarsEnabled & SCR_HORZ )
  {
    GetScrollRange( SB_HORZ , &iMin , &iMax );
    iPos = GetScrollPos( SB_HORZ );
    GetClientRect( &rc );
    switch ( nSBCode )
    {
    case SB_LINEDOWN:
      dn = LINE_SCROLL_DISTANCE;
      break;
    case SB_LINEUP:
      dn = -LINE_SCROLL_DISTANCE;
      break;
    case SB_PAGEDOWN:
      dn = rc.right / 4 + 1;
      break;
    case SB_PAGEUP:
      dn = -rc.right / 4 + 1;
      break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
      dn = nPos - iPos;
      break;
    default:
      dn = 0;
      break;
    }
    UpdateWindow();
    if ( dn = BOUND( iPos + dn , iMin , iMax - (int) m_XPage ) - iPos )
    {
      m_Map.MoveMap( -dn , 0 );
      SetScrollPos( SB_HORZ , iPos + dn , TRUE );
      UpdateWindow();
      Invalidate( TRUE );
      GetBuilder()->ViewSectionSetViewOffset( m_Map.GetOffset() ) ;
    }
  }
}

void CSketchView::OnLButtonDown( UINT nFlags , CPoint point )
{
  CWnd* pTmpWnd = SetFocus();
  TRACE( "Focus was on window 0x%x\n" , pTmpWnd );
  m_Map.ViewToAbs( &point );
  CGlyph* pGlyph = InsertGadgetAt( point );
  if ( pGlyph )
  {
    DrawNewGlyph( pGlyph );
    ::SetClassLongPtr( GetSafeHwnd() , GCLP_HCURSOR , (LONG_PTR) LoadCursor( NULL , IDC_ARROW ) );
  }
  else
  {
    ProcMouseEntry( nFlags , point );
    SetCapture();
  }
  CWnd::OnLButtonDown( nFlags , point );
}

void CSketchView::OnLButtonUp( UINT nFlags , CPoint point )
{
  if ( !m_MouseAction.IsNone() )
    ReleaseCapture();
  RedrawMouseAction();
  if ( m_MouseAction.IsLine() )
  {
    CGlyph* pStartGlyph = m_Graph.Intersect( m_MouseAction.StartPoint() , TRUE );
    if ( !pStartGlyph )
      pStartGlyph = m_Graph.Intersect( m_MouseAction.StartPoint() , FALSE );
    m_Map.ViewToAbs( &point );
    CGlyph* pEndGlyph = m_Graph.Intersect( point , TRUE );
    if ( !pEndGlyph )
      pEndGlyph = m_Graph.Intersect( point , FALSE );
    if ( pStartGlyph && pEndGlyph )
    {
      FXString uid1 , uid2;
      if ( pStartGlyph->GetUID( uid1 ) && pEndGlyph->GetUID( uid2 ) && GetBuilder()->Connect( uid1 , uid2 ) )
      {
        ConnectGlyphs( pStartGlyph , pEndGlyph );
        FXString name( _T( "Connect " ) ) ;
        name += (uid1 + _T( " and " )) + uid2;
        FixCurrentState( name );
        CPoint CancelPt( -1 , -1 ) ;
        pStartGlyph->Intersect( CancelPt , TRUE ); // Cancel hot spotting of the pin 
        m_SelectedGlyphs.DestroyIn();      // Cancel any selection
      }
    }
    Invalidate( FALSE );
  }
  else if ( m_MouseAction.IsRect() )
  {
    m_SelectedGlyphs.Select( &m_Graph );
    m_SelectedGlyphs.IntersectRect( CRect( m_MouseAction.StartPoint() , m_MouseAction.EndPoint() ) );
    CRgn rgn;
    if ( m_SelectedGlyphs.GetRgn( &rgn ) )
    {
      CRgn* Rgn = m_Map.AbsToView( &rgn );
      rgn.DeleteObject();
      InvalidateRgn( Rgn , FALSE );
      delete Rgn;
    }
  }
  else if ( m_MouseAction.IsDragging() )
  {
    if ( m_CopyRgn.m_hObject )
    {
      CRgn* Rgn = m_Map.AbsToView( &m_CopyRgn );
      CClientDC dc( this );
      //dc.FillRgn(Rgn, m_GDI.GetBrush(BS_SOLID, 0, RGB(255, 255, 255)));
      delete Rgn;
      CPoint Start = m_MouseAction.StartPoint() ;
      CPoint End = m_MouseAction.EndPoint() ;
      if ( (Start.x != End.x) || (Start.y != End.y) )
      {
        CPoint minPt = m_SelectedGlyphs.GetChildAt( 0 )->GetPos();
        int i = 1;
        while ( i < m_SelectedGlyphs.GetChildrenCount() )
        {
          CPoint pt = m_SelectedGlyphs.GetChildAt( i++ )->GetPos();
          if ( minPt.x > pt.x )
            minPt.x = pt.x;
          if ( minPt.y > pt.y )
            minPt.y = pt.y;
        }
        Copy();
        DoPaste();
        CPoint offset = minPt + m_MouseAction.StartPoint() - m_MouseAction.EndPoint();
        m_SelectedGlyphs.OffsetPos( offset );
      }
      m_CopyRgn.DeleteObject();
      m_CopyRgn.m_hObject = NULL;
      FixCurrentState( "Paste selection" );
      Invalidate( FALSE );
    }
    else 
    {
      CPoint Start = m_MouseAction.StartPoint() ;
      CPoint End = m_MouseAction.EndPoint() ;
      if ( (Start.x != End.x) || (Start.y != End.y) )
      {
        CString name = _T( "Move selection" );
        FixCurrentState( name );
      }
    }
  }
  m_MouseAction.Stop();
  CWnd::OnLButtonUp( nFlags , point );
}

void CSketchView::OnMouseMove( UINT nFlags , CPoint point )
{
  RedrawMouseAction();

  if ( m_pLastActiveGlyph )
    RedrawGlyph( m_pLastActiveGlyph );

  m_LastMousePoint = point;
  m_Map.ViewToAbs( &point );

  //CHG: Moved from 1 here
  CGlyph* pGlyph = m_Graph.Intersect( point , TRUE );
  if ( !pGlyph )
  {
    pGlyph = m_Graph.Intersect( point , FALSE );
    if ( !pGlyph )
    {
      if ( !m_sCurrGadgetName.IsEmpty() )
      {
        CGadget * pGadget = GetBuilder()->GetGadget( m_sCurrGadgetName ) ;
        if ( pGadget &&  pGadget->CanBeGrouped() && pGadget->GetGroupSelected() )
          pGadget->SetGroupSelected( FALSE ) ;
      }
      m_sCurrGadgetName = "";
      m_sCurrGadgetInfo = "";
    }
  }



  if ( m_MouseAction.IsDragging() )
  {
    CPoint offset = point - m_MouseAction.StartPoint();
    CRgn rgn;
    if ( m_SelectedGlyphs.GetRgn( &rgn ) )
    {
      CRgn* Rgn = m_Map.AbsToView( &rgn );
      CClientDC dc( this );
      rgn.DeleteObject();
      rgn.m_hObject = NULL;
      delete Rgn;
    }
    if ( m_CopyRgn.m_hObject )
    {
      CRgn* Rgn = m_Map.AbsToView( &m_CopyRgn );
      CClientDC dc( this );
      delete Rgn;
      m_CopyRgn.OffsetRgn( offset );
    }
    else
    {
      m_SelectedGlyphs.OffsetPos( offset );
      int i = 0 , count = m_SelectedGlyphs.GetChildrenCount();
      FXPropertyKit pk , uidPK;
      while ( i < count )
      {
        CGlyph* pGlyph = m_SelectedGlyphs.GetChildAt( i );
        FXString uid;//, label;
        //int c = 0;
        if ( pGlyph && pGlyph->IsGadgetGlyph() && pGlyph->IsDraggable() && pGlyph->GetUID( uid ) )
        {
          //label.Format(".%d", c++);
          //pk.WriteString(label, uid);
          CPoint pt = pGlyph->GetPos();
          uidPK.Empty();
          uidPK.WriteInt( "x" , pt.x );
          uidPK.WriteInt( "y" , pt.y );
          pk.WriteString( uid , LPCTSTR( uidPK ) );
        }
        i++;
      }
      GetBuilder()->SetProperties( LPCTSTR( pk ) );
    }
    if ( m_SelectedGlyphs.GetRgn( &rgn ) )
    {
      CRgn* Rgn = m_Map.AbsToView( &rgn );
      InvalidateRgn( Rgn , FALSE );
      rgn.DeleteObject();
      rgn.m_hObject = NULL;
      delete Rgn;
    }
    Invalidate( FALSE );
    m_MouseAction.StartPoint() = point;
  }
  else if ( !m_MouseAction.IsNone() )
  {
    m_MouseAction.EndPoint() = point;
    RedrawMouseAction();
  }

  else
  {
    //1
    FXString uid;
    {
      CPoint pTmp = point;
      FXString gname;
      if ( pGlyph && pGlyph->GetUID( uid ) && uid.GetLength() )
      {
        if ( !m_sCurrGadgetName.IsEmpty() && (m_sCurrGadgetName != (LPCTSTR)uid) )
        {
          CGadget * pGadget = GetBuilder()->GetGadget( m_sCurrGadgetName ) ;
          if ( pGadget &&  pGadget->CanBeGrouped() && pGadget->GetGroupSelected() )
            pGadget->SetGroupSelected( FALSE ) ;
        }
        FXString info;
        m_sCurrGadgetName = uid;
        m_Map.AbsToView( &pTmp );
        ClientToScreen( &pTmp );

        if ( GetBuilder()->GetElementInfo( uid , info ) )
        {
          gname = info;
        }
        else
        {
          gname = uid;
        }
        if ( pGlyph->IsGadgetGlyph() )
        {
          CGadget * pGadget = GetBuilder()->GetGadget( m_sCurrGadgetName ) ;
          if ( pGadget &&  pGadget->CanBeGrouped() && !pGadget->GetGroupSelected() )
            pGadget->SetGroupSelected( TRUE ) ;
        }
      }
      else
      {
        gname = "";
      }
      m_sCurrGadgetInfo = gname;
      if ( pGlyph != m_pLastActiveGlyph )
      {
        if ( m_DebugWnd )
          DisconnectDebugRender();
        if ( pGlyph )
        {
          RedrawGlyph( pGlyph );
          if ( IsDebugging() && m_DebugWnd )
            ConnectDebugRender( uid );
        }
      }
    }
    if ( m_pLastActiveGlyph )
    {
      m_pLastActiveGlyph->Intersect( point , FALSE );
      RedrawGlyph( m_pLastActiveGlyph );
    }

    //2
  }

  //CHG: Moved from 2 here
  m_pLastActiveGlyph = pGlyph;

  TrackMouseEvent();

}

void CSketchView::OnRButtonDown( UINT nFlags , CPoint point )
{
  RedrawMouseAction();
  m_Map.ViewToAbs( &point );
  CGlyph* pGlyph = m_Graph.Intersect( point , TRUE );
  if ( !pGlyph )
    pGlyph = m_Graph.Intersect( point , FALSE );
  if ( pGlyph && pGlyph->HasContextMenu() )
  {
    m_ContextMenuInfo.pGlyph = pGlyph;
    m_ContextMenuInfo.pt = point;
    CMenu menu , modeMenu;
    menu.CreatePopupMenu();
    pGlyph->FillContextMenu( &menu );
    CPoint pt = point;
    Map( pt );
    ClientToScreen( &pt );
    menu.TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON , pt.x , pt.y , this , NULL );
    menu.DestroyMenu();
    pGlyph->DestroyContextSubmenus();
  }
  else
  {
    m_MouseAction.StartDrag( point );
    SelectAll();
    SetClassLongPtr( GetSafeHwnd() , GCLP_HCURSOR , (LONG_PTR)::LoadCursor( NULL , MAKEINTRESOURCE( 32649 ) ) );
    CPoint pt;
    GetCursorPos( &pt );
    SetCursorPos( pt.x , pt.y );
  }
  CWnd::OnRButtonDown( nFlags , point );
}

void CSketchView::OnRButtonUp( UINT nFlags , CPoint point )
{
  if ( m_MouseAction.IsDragging() )
  {
    m_MouseAction.Stop();
    m_SelectedGlyphs.DestroyIn();
    Invalidate();
    SetClassLongPtr( GetSafeHwnd() , GCLP_HCURSOR , (LONG_PTR)::LoadCursor( NULL , IDC_ARROW ) );
    CPoint pt;
    GetCursorPos( &pt );
    SetCursorPos( pt.x , pt.y );
    CString name = _T( "Move all" );
    FixCurrentState( name );
  }
  CWnd::OnRButtonUp( nFlags , point );
}

void CSketchView::OnLButtonDblClk( UINT nFlags , CPoint point )
{
  RedrawMouseAction();
  m_Map.ViewToAbs( &point );
  CGlyph* pGlyph = m_Graph.Intersect( point , TRUE );
  if ( !pGlyph )
    pGlyph = m_Graph.Intersect( point , FALSE );
  if ( pGlyph )
  {
    FXString uid;
    if ( pGlyph->GetUID( uid ) )
    {
      CREATEGRAPHVIEW cgv;
      IGraphbuilder* pBuilder = GetBuilder() ;
      FXString path = pBuilder->GetID() + _T( '.' ) + uid ;
      cgv.path = LPCTSTR( path ) ;
      cgv.builder = GetBuilder()->GetSubBuilder( uid );
      if ( cgv.path && cgv.builder )
      {
        pGlyph->StopDebuggingPins();
        if ( m_pTargetWnd && ::IsWindow( m_pTargetWnd->GetSafeHwnd() ) )
        {
          m_pTargetWnd->SendMessage( VM_TVDB400_CREATENEWVIEW ,
            GetBuilder()->KindOf( uid ) , (LPARAM) &cgv );
        }
        else
        {
          AfxGetMainWnd()->SendMessage( VM_TVDB400_CREATENEWVIEW ,
            GetBuilder()->KindOf( uid ) , (LPARAM) &cgv );
        }
      }
      else if ( pGlyph->IsDraggable() ) // gadget
      {
        HMONITOR hMonitor = MonitorFromWindow( GetSafeHwnd() , MONITOR_DEFAULTTONEAREST );
        MONITORINFO mi;
        m_Map.AbsToView( &point );
        mi.cbSize = sizeof( mi );
        if ( GetMonitorInfo( hMonitor , &mi ) )
        {
          point.x += mi.rcMonitor.left;
          point.y += mi.rcMonitor.top;
        }
        pGlyph->ShowSetupDlgAt( point );
      }
      else
      {
        ToggleDebugRender( uid );
      }
    }
  }
  else // just reset to defult zoom
  {
    m_Map.SetScale( 1 );
    Invalidate();
  }
  CWnd::OnLButtonDblClk( nFlags , point );
}

BOOL CSketchView::OnMouseWheel( UINT nFlags , short zDelta , CPoint pt )
{
  if ( nFlags == MK_CONTROL )
  {
    double scale = m_Map.GetScale();
    if ( zDelta < 0 && scale > .01 )
    {
      m_Map.SetScale( scale / 1.1 );
      m_bRescaled = TRUE;
    }
    else if ( zDelta > 0 && scale < 10. )
    {
      m_Map.SetScale( scale * 1.1 );
      m_bRescaled = TRUE;
    }
    GetBuilder()->ViewSectionSetViewScale( m_Map.GetScale() ) ;
    Invalidate();
  }

  return TRUE;
}

void CSketchView::ProcMouseEntry( UINT nFlags , CPoint point )
{
  // First, check if a selected glyph is clicked
  CGlyph* pGlyph = m_SelectedGlyphs.Intersect( point , TRUE );
  if ( !pGlyph )
    pGlyph = m_SelectedGlyphs.Intersect( point , FALSE );
  if ( pGlyph )
  {
    if ( nFlags & MK_CONTROL )				// if Ctrl is pressed
    {										//  remove glyph from selected
      m_SelectedGlyphs.Remove( pGlyph );
      Invalidate( FALSE );
    }
    else if ( !pGlyph->IsDraggable() )
      m_MouseAction.StartLine( point );
    else if ( m_SelectedGlyphs.IsDraggable() )// else
    {
      if ( nFlags & MK_SHIFT )
        m_SelectedGlyphs.GetRgn( &m_CopyRgn , TRUE );
      m_MouseAction.StartDrag( point );		//  start dragging selected
    }
    return;
  }
  // Second, check if a not-selected glyph is clicked
  pGlyph = m_Graph.Intersect( point , TRUE );
  if ( !pGlyph )
    pGlyph = m_Graph.Intersect( point , FALSE );
  if ( pGlyph )
  {
    if ( !(nFlags & MK_CONTROL) )				// if Ctrl is not pressed
      m_SelectedGlyphs.DestroyIn();	//  drop selected glyphs
    m_SelectedGlyphs.Add( pGlyph );			// Add glyph to selected
    Invalidate( FALSE );
    // Begin mouse action on selected glyph(s)
    if ( m_SelectedGlyphs.IsDraggable() )
    {
      if ( nFlags & MK_SHIFT )
        m_SelectedGlyphs.GetRgn( &m_CopyRgn , TRUE );
      m_MouseAction.StartDrag( point );
    }
    else
      m_MouseAction.StartLine( point );
    return;
  }
  // No glyph is clicked, drop selected glyphs
  CRgn rgn;
  if ( m_SelectedGlyphs.GetRgn( &rgn ) )
  {
    CRgn* Rgn = m_Map.AbsToView( &rgn );
    rgn.DeleteObject();
    m_SelectedGlyphs.DestroyIn();
    InvalidateRgn( &rgn , FALSE );
    delete Rgn;
  }
  // Start a rectangular selection
  RedrawMouseAction();
  m_MouseAction.StartRect( point );
}

BOOL CSketchView::OnCommand( WPARAM wParam , LPARAM lParam )
{
  switch ( wParam & 0xFFFF )
  {
  case ID_EDIT_COPY:
    Copy();
    return TRUE;
  case ID_EDIT_SELECT_ALL:
    SelectAll();
    return TRUE;
  case ID_EDIT_PASTE:
    Paste();
    return TRUE;
  case ID_EDIT_CUT:
    Copy();
    DeleteSelected();
    return TRUE;
  case ID_EDIT_REDO:
    Undo( -1 );
    return TRUE;
  case ID_EDIT_UNDO:
    Undo();
    return TRUE;
  case ID_EDIT_DELETE:
    DeleteSelected();
    return TRUE;
  case ID_EDIT_AGGREGATE:
    AggregateSelected();
    return TRUE;
  case ID_EDIT_EXPAND:
    ExpandSelected();
    return TRUE;
  default:
    return CWnd::OnCommand( wParam , lParam );
  }
}

BOOL CSketchView::PreTranslateMessage( MSG* pMsg )
{
  if ( pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST )
  {
    ::TranslateAccelerator( m_hWnd , m_hAccel , pMsg );
  }
  return CWnd::PreTranslateMessage( pMsg );
}


LRESULT CSketchView::WindowProc( UINT message , WPARAM wParam , LPARAM lParam )
{
  if ( message == ID_HELP )
  {
    TRACE( "HELP!!!" );
  }
  return CWnd::WindowProc( message , wParam , lParam );
}

CString CSketchView::GetCurrentGadgetName()
{
  return m_sCurrGadgetName;
}

CString CSketchView::GetCurrentGadgetInfo()
{
  return m_sCurrGadgetInfo;
}

BOOL CSketchView::OnEraseBkgnd( CDC* pDC )
{
  // TODO: Add your message handler code here and/or call default

  //return CWnd::OnEraseBkgnd(pDC);
  return FALSE;
}

LRESULT CSketchView::OnFltWndChanged( WPARAM wparam , LPARAM lparam )
{
  FXString name = (LPCTSTR) lparam;
  CFloatWnd* fltWnd = (CFloatWnd*) wparam;

  const CSystemMonitorsEnumerator * pMonitorsEnum =
    CSystemMonitorsEnumerator::GetMonitorEnums() ;

  CRect rcDesktop = pMonitorsEnum->m_FullDesktopRect , rc ;
  fltWnd->GetWindowRect( rc );

  double x = (double) (rc.left - rcDesktop.left) / (double) rcDesktop.Width();
  double y = (double) rc.top / (double) rcDesktop.Height();
  double w = (double) rc.Width() / (double) rcDesktop.Width();
  double h = (double) rc.Height() / (double) rcDesktop.Height();
  FXString SelectedTab = (FXString) (fltWnd->GetCurrentTabSelection()) ;
  GetBuilder()->SetFloatWnd( name , x , y , w , h , SelectedTab );
  return TRUE;
}

LRESULT CSketchView::OnMouseLeave( WPARAM wparam , LPARAM lparam )
{
  m_sCurrGadgetName = "";
  //Everything is OK
  return FALSE;
}

void CSketchView::TrackMouseEvent()
{
  m_tme.cbSize = sizeof( m_tme );
  m_tme.hwndTrack = m_hWnd;
  m_tme.dwFlags = TME_LEAVE;
  ::TrackMouseEvent( &m_tme );
  //TODO: add error handler - but where?

}

void CSketchView::CascadeFloatWindows()
{
  CStringArray wndNames;
  m_FloatWndManager.EnumMonitors( wndNames );
  int iWndCount = (int) wndNames.GetSize();

  //There are no float windows
  if ( iWndCount == 0 )
    return;

  CFloatWnd* pCurFloatWnd = NULL;
  //Start position of cascading

  //ScreenToClient(&curStartPos);
  CRect viewRect;
  CRect rect;
  GetClientRect( &viewRect );
  ClientToScreen( &viewRect );
  CPoint curStartPos( viewRect.TopLeft() );
  int offset = ::GetSystemMetrics( SM_CYCAPTION );
  for ( int i = 0; i < iWndCount; i++ )
  {
    pCurFloatWnd = m_FloatWndManager.FindMonitor( wndNames.GetAt( i ) );
    pCurFloatWnd->GetClientRect( &rect );
    //pCurFloatWnd->MoveWindow(curStartPos.x, curStartPos.y, rect.Width(), rect.Height());
    pCurFloatWnd->SetWindowPos( NULL , curStartPos.x , curStartPos.y , 0 , 0 , SWP_NOSIZE | SWP_NOZORDER );
    curStartPos += CPoint( offset , offset );

  }
  return;
}

void CSketchView::OnContextRename()
{
  CGlyph* pGlyph = m_ContextMenuInfo.pGlyph;
  FXString uid;
  if ( pGlyph && pGlyph->GetUID( uid ) )
  {
    CRgn rgn;
    CRect rc;
    if ( pGlyph->GetRgn( &rgn , TRUE ) )
    {
      rgn.GetRgnBox( rc );
      rgn.DeleteObject();
    }
    else
    {
      rc.TopLeft() = m_ContextMenuInfo.pt;
      rc.BottomRight() = rc.TopLeft();
    }
    Map( rc );
    ClientToScreen( rc );
    rc.DeflateRect( 1 , 1 , 1 , 1 );
    CSimpleEnterDlg ed;
    ed.m_Enter = uid;
    ed.m_rc = rc;
    if ( ed.DoModal() == IDOK && ed.m_Enter != (LPCTSTR) uid )
    {
      uid = ed.m_Enter;
      RenameGlyph( pGlyph , uid );
    }
  }
}

void CSketchView::OnContextSetup()
{
  if ( m_ContextMenuInfo.pGlyph )
  {
    CPoint point = m_ContextMenuInfo.pt;
    HMONITOR hMonitor = MonitorFromWindow( GetSafeHwnd() , MONITOR_DEFAULTTONEAREST );
    MONITORINFO mi;
    mi.cbSize = sizeof( mi );
    if ( GetMonitorInfo( hMonitor , &mi ) )
    {
      point.x += mi.rcMonitor.left;
      point.y += mi.rcMonitor.top;
    }
    m_ContextMenuInfo.pGlyph->ShowSetupDlgAt( point );
  }
}

void CSketchView::OnContextAffinity()
{
  if ( m_ContextMenuInfo.pGlyph )
    m_ContextMenuInfo.pGlyph->ShowAffinityDlgAt( m_ContextMenuInfo.pt );
}

void CSketchView::OnContextCmplxLocal()
{
  CGlyph* pGlyph = m_ContextMenuInfo.pGlyph;
  FXString uid;
  if ( pGlyph && pGlyph->GetUID( uid ) )
  {
    GetBuilder()->SetLocalComplexGadget( uid );
  }
}

void CSketchView::OnPinSetLabel()
{
  CGlyph* pGlyph = m_ContextMenuInfo.pGlyph;
  FXString uid;
  if ( pGlyph && pGlyph->GetUID( uid ) )
  {
    FXString Label;
    if ( GetBuilder()->GetPinLabel( uid , Label ) )
    {
      CRgn rgn;
      CRect rc;
      if ( pGlyph->GetRgn( &rgn , TRUE ) )
      {
        rgn.GetRgnBox( rc );
        rgn.DeleteObject();
      }
      else
      {
        rc.TopLeft() = m_ContextMenuInfo.pt;
        rc.BottomRight() = rc.TopLeft();
      }
      Map( rc );
      ClientToScreen( rc );
      rc.DeflateRect( 1 , 1 , 1 , 1 );
      CSimpleEnterDlg ed;
      ed.m_Title = _T( "Set label" );
      ed.m_Enter = Label;
      ed.m_rc = rc;
      if ( ed.DoModal() == IDOK && ed.m_Enter != (LPCTSTR) Label )
      {
        GetBuilder()->SetPinLabel( uid , ed.m_Enter );
      }
    }
  }
}

void CSketchView::OnPinLogging()
{
  CGlyph* pGlyph = m_ContextMenuInfo.pGlyph;
  FXString uid;
  if ( pGlyph && pGlyph->GetUID( uid ) )
  {
    if ( uid.Find( ">>" ) > 0
      || uid.Find( "<<" ) > 0
      || uid.Find( "<>" ) > 0 )
    {
      CConnectorGlyph* pCGlyph = (CConnectorGlyph*) pGlyph;
      CConnector* pConnector = pCGlyph->GetConnector();
      if ( pConnector )
      {
        FXString pinUID;
        if ( pCGlyph->GetUID( pinUID ) )
          pConnector->SetName( pinUID );
        if ( pConnector->GetLogMode() )
          pConnector->SetLogMode( 0 );
        else
          pConnector->SetLogMode( 1 );
      }
    }
  }
}

void CSketchView::OnModeReject()
{
  CGlyph* pGlyph = m_ContextMenuInfo.pGlyph;
  FXString uid;
  if ( pGlyph && pGlyph->GetUID( uid ) && GetBuilder() )
    GetBuilder()->SetGadgetMode( uid , CGadget::mode_reject );
  GetBuilder()->SetGadgetStatus( uid , STATUS_MODIFIED , true );
}

void CSketchView::OnModeTransmit()
{
  CGlyph* pGlyph = m_ContextMenuInfo.pGlyph;
  FXString uid;
  if ( pGlyph && pGlyph->GetUID( uid ) && GetBuilder() )
    GetBuilder()->SetGadgetMode( uid , CGadget::mode_transmit );
  GetBuilder()->SetGadgetStatus( uid , STATUS_MODIFIED , true );
}

void CSketchView::OnModeProcess()
{
  CGlyph* pGlyph = m_ContextMenuInfo.pGlyph;
  FXString uid;
  if ( pGlyph && pGlyph->GetUID( uid ) && GetBuilder() )
    GetBuilder()->SetGadgetMode( uid , CGadget::mode_process );
  GetBuilder()->SetGadgetStatus( uid , STATUS_MODIFIED , true );
}

void CSketchView::OnModeAppend()
{
  CGlyph* pGlyph = m_ContextMenuInfo.pGlyph;
  FXString uid;
  if ( pGlyph && pGlyph->GetUID( uid ) && GetBuilder() )
    GetBuilder()->SetOutputMode( uid , CFilterGadget::modeAppend );
  GetBuilder()->SetGadgetStatus( uid , STATUS_MODIFIED , true );
}

void CSketchView::OnModeReplace()
{
  CGlyph* pGlyph = m_ContextMenuInfo.pGlyph;
  FXString uid;
  if ( pGlyph && pGlyph->GetUID( uid ) && GetBuilder() )
    GetBuilder()->SetOutputMode( uid , CFilterGadget::modeReplace );
  GetBuilder()->SetGadgetStatus( uid , STATUS_MODIFIED , true );
}

void CSketchView::OnThreadsIncrease()
{
  CGlyph* pGlyph = m_ContextMenuInfo.pGlyph;
  FXString uid;
  if ( pGlyph && pGlyph->GetUID( uid ) && GetBuilder() )
  {
    int n;
    if ( GetBuilder()->GetGadgetThreadsNumber( uid , n ) )
    {
      n++;
      GetBuilder()->SetGadgetThreadsNumber( uid , n );
    }
  }
}

void CSketchView::OnThreadsDecrease()
{
  CGlyph* pGlyph = m_ContextMenuInfo.pGlyph;
  FXString uid;
  if ( pGlyph && pGlyph->GetUID( uid ) && GetBuilder() )
  {
    int n;
    if ( GetBuilder()->GetGadgetThreadsNumber( uid , n ) )
    {
      n--;
      GetBuilder()->SetGadgetThreadsNumber( uid , n );
    }
  }
}

void CSketchView::SetViewActivity( bool set )
{
  m_ViewActivity = set;
  FXRegistry Reg( "TheFileX\\SHStudio" ) ;
  Reg.WriteRegiInt( _T( "settings" ) , _T( "viewactivity" ) , m_ViewActivity != false );
}


void CSketchView::ShowAffinityDlg( FXString& uid , CPoint& point )
{
  CGadget* Gadget = GetBuilder()->GetGadget( uid );
  CAffinityDlg m_AD( Gadget );
  if ( m_AD.DoModal() == IDOK )
  {
    TRACE( "New Affinity set\n" );
  }
}

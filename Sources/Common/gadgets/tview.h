#ifndef _TVDB400_SKETCH_VIEW_INCLUDED
#define _TVDB400_SKETCH_VIEW_INCLUDED

#include "shkernel.h"

#ifdef _DEBUG
#define TVVIEW_DLL_NAME  "tviewd.dll"
#define TVVIEW_LIB_NAME  "tviewd.lib"
#else
#define TVVIEW_DLL_NAME  "tview.dll"
#define TVVIEW_LIB_NAME  "tview.lib"
#endif

#ifndef TVIEW_DLL
#define AFX_EXT_TVIEW __declspec(dllimport)
#pragma comment(lib, TVVIEW_LIB_NAME)
#else
#define AFX_EXT_TVIEW __declspec(dllexport)
#endif

void AFX_EXT_TVIEW attachtviewDLL();

class ViewType
{
public:
  const enum EvadeType
  {
    EVADE_CLASSIC ,
    EVADE_MODERN
  };
  EvadeType m_EvType;
  CPoint offset;
  ViewType() : m_EvType( EVADE_CLASSIC ) , offset( CPoint( 4 , 4 ) ) {};
};

#define GET_WND(view) ((view)->GetWnd())

#define TVDB_WORKSPACE_BAR_ID_FIRST	(AFX_IDW_DOCKBAR_FLOAT + 1)

#define GET_BAR(bar) ((bar)->GetBar())

static const UINT VM_TVDB400_CREATENEWVIEW = ::RegisterWindowMessage( _T( "Tvdb400_CreateNewView" ) );
static const UINT VM_TVDB400_REGISTERDRAGDROP = ::RegisterWindowMessage( _T( "Tvdb400_RegisterDragDrop" ) );
static const UINT VM_TVDB400_DELETESUBVIEW = ::RegisterWindowMessage( _T( "Tvdb400_DeleteSubView" ) );
// Message for show of video object setup dialog
static const UINT VM_TVDB400_SHOWVOSETUPDLG = ::RegisterWindowMessage( _T( "Tvdb400_ShowVOSetupDialog" ) );

typedef struct tagCREATEGRAPHVIEW
{
  LPCTSTR path;
  IGraphbuilder* builder;
}CREATEGRAPHVIEW , *LPCREATEGRAPHVIEW;

class ISketchView
{
public:
  enum { GT_CAPTURE , GT_FILTER , GT_RENDER , };
  virtual void _stdcall AddRef() = 0;
  virtual void _stdcall Release() = 0;
  virtual void _stdcall SetBuilder( IGraphbuilder* Builder ) = 0;
  virtual void _stdcall SetTargetWnd( CWnd* pWnd ) = 0;
  virtual void _stdcall DeleteGraph() = 0;
  virtual void _stdcall SelectAll() = 0;
  virtual void _stdcall ComposeGraph() = 0;
  virtual CWnd* _stdcall GetWnd() = 0;
  virtual bool _stdcall InsertGadget() = 0;
  virtual void _stdcall RegisterSketchDragDrop() = 0;
  virtual void _stdcall DeleteSelected() = 0;
  virtual void _stdcall AggregateSelected( LPCTSTR uid = NULL , LPCTSTR name = NULL ) = 0;
  virtual void _stdcall ExpandSelected() = 0;
  virtual void _stdcall InitUndoManager() = 0;
  virtual void _stdcall UpdateWindow() = 0;
  virtual int _stdcall  CanUndo( CStringArray* IDs = NULL ) = 0;
  virtual int _stdcall  CanRedo( CStringArray* IDs = NULL ) = 0;
  virtual BOOL _stdcall Undo( int steps = 1 ) = 0;
  virtual BOOL _stdcall IsDebugging() = 0;
  virtual void _stdcall SetDebugMode( BOOL on ) = 0;
  virtual BOOL _stdcall CanCopy() = 0;
  virtual void _stdcall Copy() = 0;
  virtual BOOL _stdcall CanPaste() = 0;
  virtual void _stdcall Paste() = 0;
  virtual void _stdcall EmptyCopyBuffer() = 0;
  virtual void _stdcall SetViewType( ViewType vt ) = 0;
  virtual CString _stdcall GetCurrentGadgetName() = 0;
  virtual CString _stdcall GetCurrentGadgetInfo() = 0;
  virtual void _stdcall CascadeFloatWindows() = 0;
  virtual bool GetViewActivity() = 0;
  virtual void SetViewActivity( bool set ) = 0;
  virtual CPoint GetViewOffset() = 0 ;
  virtual void SetViewOffset( CPoint offset ) = 0 ;
  virtual double GetViewScale() = 0 ;
  virtual void SetViewScale( double scale ) = 0 ;
};

AFX_EXT_TVIEW ISketchView* Tvdb400_GetSketchView();
AFX_EXT_TVIEW ISketchView* Tvdb400_GetSketchViewMod();
AFX_EXT_TVIEW bool ShowHelp( int cmd = ID_HELP_INDEX , LPCTSTR topic = NULL );

class IGadgetsBar
{
public:
  virtual void _stdcall AddRef() = 0;
  virtual void _stdcall Release() = 0;
  virtual void _stdcall SetView( ISketchView* IView ) = 0;
  virtual BOOL _stdcall Create( CWnd* pParent , LPCTSTR title = "" , DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_LEFT
    | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC , UINT uID = TVDB_WORKSPACE_BAR_ID_FIRST ) = 0;
  virtual CToolBar* _stdcall GetBar() = 0;
  virtual void _stdcall PopulateTree( IGraphbuilder* pBuilder ) = 0;
  virtual void _stdcall PopulateUserLibrary( LPCTSTR path , LPCTSTR ext = _T( "*.cog" ) ) = 0;
  virtual void _stdcall SetShowType( BOOL bShow , IGraphbuilder* pBuilder ) = 0;
  virtual BOOL _stdcall IsTypeShown() = 0;
};

AFX_EXT_TVIEW IGadgetsBar* Tvdb400_GetGadgetsBar();

#endif
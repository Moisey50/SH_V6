// tview.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>
#include <Gadgets\tview.h>
#include <gadgets\stdsetup.h>
#include "SketchView.h"
#include "SketchViewMod.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static AFX_EXTENSION_MODULE TviewDLL = { NULL , NULL };
CDynLinkLibrary* pThisDll = NULL;

extern "C" int APIENTRY

DllMain( HINSTANCE hInstance , DWORD dwReason , LPVOID lpReserved )
{
  // Remove this if you use lpReserved
  UNREFERENCED_PARAMETER( lpReserved );

  if ( dwReason == DLL_PROCESS_ATTACH )
  {
    TRACE0( "TVIEW.DLL Initializing!\n" );

    // Extension DLL one-time initialization
    if ( !AfxInitExtensionModule( TviewDLL , hInstance ) )
      return 0;

    // Insert this DLL into the resource chain
    // NOTE: If this Extension DLL is being implicitly linked to by
    //  an MFC Regular DLL (such as an ActiveX Control)
    //  instead of an MFC application, then you will want to
    //  remove this line from DllMain and put it in a separate
    //  function exported from this Extension DLL.  The Regular DLL
    //  that uses this Extension DLL should then explicitly call that
    //  function to initialize this Extension DLL.  Otherwise,
    //  the CDynLinkLibrary object will not be attached to the
    //  Regular DLL's resource chain, and serious problems will
    //  result.

    pThisDll = new CDynLinkLibrary( TviewDLL );
  }
  else if ( dwReason == DLL_PROCESS_DETACH )
  {
    TRACE0( "TVIEW.DLL Terminating!\n" );
    // Terminate the library before destructors are called
    AfxTermExtensionModule( TviewDLL );
    //if (pThisDll) delete pThisDll;
  }
  return 1;   // ok
}

class CSketchViewWrapper : public ISketchView
{
protected:
  CSketchView *m_View;
  int m_RefCount;
  CSketchViewWrapper( CSketchView *sv ) : m_RefCount( 0 ) , m_View( sv ) {};
public:
  CSketchViewWrapper() : m_RefCount( 0 ) { m_View = new CSketchView; };
  ~CSketchViewWrapper()
  {
    delete m_View;
  };
  virtual void _stdcall AddRef()
  {
    m_RefCount++;
#ifdef TVDB400_TRACE_GLYPHS
    CGlyphsHeap::Get()->AddRef();
#endif
  };
  virtual void _stdcall Release()
  {
    m_RefCount--;
    if ( !m_RefCount )
    {
      m_View->DestroyWindow();
      delete this;
#ifdef TVDB400_TRACE_GLYPHS
      CGlyphsHeap::Get()->Release();
#endif
    }
  };
  virtual void _stdcall SetBuilder( IGraphbuilder* Builder ) { m_View->SetBuilder( Builder ); m_View->InitUndoManager(); };
  virtual void _stdcall SetTargetWnd( CWnd* pWnd ) { m_View->SetTargetWnd( pWnd ); };
  virtual void _stdcall ClearView() { m_View->ClearView(); };
  virtual void _stdcall DeleteGraph() { m_View->DeleteGraph(); };
  virtual void _stdcall SelectAll() { m_View->SelectAll(); };
  virtual void _stdcall ComposeGraph() { m_View->ComposeGraph(); };
  virtual CWnd* _stdcall GetWnd() { return m_View; };
  virtual bool _stdcall InsertGadget() { return m_View->InsertGadget(); };
  virtual void _stdcall RegisterSketchDragDrop() { return m_View->RegisterSketchDragDrop(); };
  virtual void _stdcall DeleteSelected() { m_View->DeleteSelected(); };
  virtual void _stdcall AggregateSelected( LPCTSTR uid = NULL , LPCTSTR name = NULL ) { m_View->AggregateSelected( uid , name ); };
  virtual void _stdcall ExpandSelected() { m_View->ExpandSelected(); };
  virtual void _stdcall InitUndoManager() { m_View->InitUndoManager(); };
  virtual void _stdcall UpdateWindow() { m_View->Invalidate(); m_View->UpdateWindow(); };
  virtual int  _stdcall CanUndo( CStringArray* IDs = NULL ) { return m_View->CanUndo( IDs ); };
  virtual int  _stdcall CanRedo( CStringArray* IDs = NULL ) { return m_View->CanRedo( IDs ); };
  virtual BOOL _stdcall Undo( int steps = 1 ) { return m_View->Undo( steps ); };
  virtual BOOL _stdcall IsDebugging() { return m_View->IsDebugging(); };
  virtual void _stdcall SetDebugMode( BOOL on ) { m_View->SetDebugMode( on ); };
  virtual BOOL _stdcall CanCopy() { return m_View->CanCopy(); };
  virtual void _stdcall Copy() { m_View->Copy(); };
  virtual BOOL _stdcall CanPaste() { return m_View->CanPaste(); };
  virtual void _stdcall Paste() { m_View->Paste(); };
  virtual void _stdcall EmptyCopyBuffer() { m_View->EmptyCopyBuffer(); };
  virtual CString _stdcall GetCurrentGadgetName() { return m_View->GetCurrentGadgetName(); };
  virtual CString _stdcall GetCurrentGadgetInfo() { return m_View->GetCurrentGadgetInfo(); };

  virtual void _stdcall SetViewType( ViewType vt ) { m_View->SetViewType( vt ); };
  virtual void _stdcall CascadeFloatWindows() { m_View->CascadeFloatWindows(); }
  virtual bool GetViewActivity() { return m_View->GetViewActivity(); }
  virtual void SetViewActivity( bool set ) { m_View->SetViewActivity( set ); }
  virtual CPoint GetViewOffset()  { return m_View->GetViewOffset(); }
  virtual void SetViewOffset( CPoint Offset ) { m_View->SetViewOffset( Offset ) ; }
  virtual double GetViewScale() { return m_View->GetViewScale() ; }
  virtual void SetViewScale( double dScale ) { m_View->SetViewScale( dScale ) ; }
};

class CSketchViewWrapperMod : public CSketchViewWrapper
{
protected:
  CSketchViewMod *m_View;
public:
  CSketchViewWrapperMod() : CSketchViewWrapper( (CSketchView*) (m_View = new CSketchViewMod) ) {};
};

ISketchView* Tvdb400_GetSketchView()
{
  ISketchView* View = (ISketchView*) (new CSketchViewWrapper);
  View->AddRef();
  return View;
}

ISketchView* Tvdb400_GetSketchViewMod()
{
  ISketchView* View = (ISketchView*) (new CSketchViewWrapperMod);
  View->AddRef();
  return View;
}

class CGadgetsBarWrapper : public IGadgetsBar
{
  CGadgetsDockbar m_Bar;
  int m_RefCount;
public:
  class CGadgetsBarWrapper() : m_RefCount( 0 ) {};
  virtual void _stdcall AddRef() { m_RefCount++; };
  virtual void _stdcall Release() { m_RefCount--; if ( !m_RefCount ) delete this; };
  virtual void _stdcall SetView( ISketchView* IView ) { m_Bar.SetView( IView ); };
  virtual BOOL _stdcall Create( CWnd* pParent , LPCTSTR title = "" , 
    DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_LEFT
    | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC , 
    UINT uID = TVDB_WORKSPACE_BAR_ID_FIRST )
  {
    return m_Bar.Create( pParent , title , dwStyle , uID );
  };
  virtual CToolBar* _stdcall GetBar() { return (CToolBar*) &m_Bar; };
  virtual void _stdcall PopulateTree( IGraphbuilder* pBuilder ) { m_Bar.PopulateTree( pBuilder ); };
  virtual void _stdcall PopulateUserLibrary( LPCTSTR path , LPCTSTR ext = _T( "*.cog" ) ) 
  { m_Bar.PopulateUserLibrary( path , ext ); };
  virtual void _stdcall SetShowType( BOOL bShow , IGraphbuilder* pBuilder ) { m_Bar.SetShowType( bShow , pBuilder ); };
  virtual BOOL _stdcall IsTypeShown() { return m_Bar.IsTypeShown(); };
};


IGadgetsBar* Tvdb400_GetGadgetsBar()
{
  IGadgetsBar* Bar = (IGadgetsBar*) (new CGadgetsBarWrapper);
  Bar->AddRef();
  return Bar;
}

static bool _attached = false;

void attachtviewDLL()
{
  if ( !_attached )
  {
    new CDynLinkLibrary( TviewDLL );
    attachstdsetupDLL();
    _attached = true;
  }
}

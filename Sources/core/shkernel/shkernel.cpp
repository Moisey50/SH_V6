// shkernel.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <afxwin.h>
#include <afxdllx.h>
#include <gadgets\shkernel.h>
#include "graphbuilder.h"
#include "netbuilder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static AFX_EXTENSION_MODULE shkernelDLL = {NULL, NULL};

extern "C" int APIENTRY
DllMain( HINSTANCE hInstance , DWORD dwReason , LPVOID lpReserved )
{
  // Remove this if you use lpReserved
  UNREFERENCED_PARAMETER( lpReserved );

  if ( dwReason == DLL_PROCESS_ATTACH )
  {
    TRACE0( "shkernel.dll Initializing!\n" );

    // Extension DLL one-time initialization
    if ( !AfxInitExtensionModule( shkernelDLL , hInstance ) )
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

    new CDynLinkLibrary( shkernelDLL );

  }
  else if ( dwReason == DLL_PROCESS_DETACH )
  {
    TRACE0( "shkernel.dll Terminating!\n" );

    // Terminate the library before destructors are called
    AfxTermExtensionModule( shkernelDLL );
  }
  return 1;   // ok
}

static bool _attached = false;

void attachshkernelDLL()
{
  if ( !_attached )
  {
    new CDynLinkLibrary( shkernelDLL );
    attachshbaseDLL();
    _attached = true;
  }
}

// User lib path helpers

BOOL GetUserLibraryPath( FXString& path )
{

  size_t ReturnValue;
  char   buffer[ MAX_PATH ];
  getenv_s( &ReturnValue , buffer , MAX_PATH , "APPDATA" );
  path = buffer;
  path.TrimRight( "/\\" );
  path += _T( "\\StreamHandler" );
  FxVerifyCreateDirectory( path );
  path += _T( "\\TvUser" );
  return TRUE;
}

LPCTSTR GetUserLibExtension()
{
  return _T( "cog" );
}

IGraphbuilder::IGraphbuilder() :
  m_pMsgQueue( NULL ) , m_pStatus( NULL )
{
  m_cRefs = 0;
  m_pGraphSetupObject = NULL ;
  AddRef();
}

IGraphbuilder::~IGraphbuilder()
{
  ASSERT( _heapchk() == _HEAPOK );
  if ( m_pMsgQueue )
    m_pMsgQueue->Release();
  m_pMsgQueue = NULL;
  ASSERT( _heapchk() == _HEAPOK );
}

void IGraphbuilder::AddRef()
{
  m_cRefs++;
}

DWORD IGraphbuilder::Release()
{
  DWORD retV = --m_cRefs;
  if ( !m_cRefs )
  {
    ShutDown();	// The very last call to Release() destroys the current graph
    delete this;
  }
  return retV;
}

void IGraphbuilder::SetMsgQueue( IGraphMsgQueue* pMsgQueue )
{
  if ( m_pMsgQueue )
    m_pMsgQueue->Release();
  m_pMsgQueue = pMsgQueue;
  if ( m_pMsgQueue )
    m_pMsgQueue->AddRef();
}

IGraphMsgQueue* IGraphbuilder::GetMsgQueue()
{
  return m_pMsgQueue;
}

BOOL IGraphbuilder::IsSetupOn()
{
  return ((m_pGraphSetupObject != NULL) && m_pGraphSetupObject->IsOn());
}


IGraphbuilder* Tvdb400_CreateBuilder( CExecutionStatus* Status )
{
  IGraphbuilder* GraphBuilder = (IGraphbuilder*)new CGraphBuilder( Status );
  REGISTER_RUNTIME_GADGET( Complex , GraphBuilder );
  return GraphBuilder;
}

IGraphbuilder* Tvdb400_GetNetBuilder( LPCTSTR host , WORD port )
{
  IGraphbuilder* GraphBuilder = (IGraphbuilder*)new CNetBuilder( host , port );
  return GraphBuilder;
}


//
// Complex
//

IMPLEMENT_RUNTIME_GADGET( Complex , CGadget , LINEAGE_COMPLEX );
#undef THIS_MODULENAME 
#define THIS_MODULENAME "TvdBase.ComplexGadget"

Complex::Complex() :
  m_pBuilder( NULL ) ,
  m_LoadPath( "" ) ,
  m_pStatus( NULL )
{
  SetBuilder( Tvdb400_CreateBuilder() );
  Status().WriteBool( STATUS_MODIFIED , false );
}

void Complex::ShutDown()
{
  CGadget::ShutDown();
  UnregisterConnectors();
  if ( m_pBuilder )
  {
    //Disconnect from parent execsatus
    m_pBuilder->SetExecutionStatus( NULL );
    m_pBuilder->Release();
  }
  m_pBuilder = NULL;
  if ( m_pStatus )
    m_pStatus->Release();
  m_pStatus = NULL;
}

void Complex::Collapse()
{
  UnregisterConnectors();
  if ( m_pBuilder )
    m_pBuilder->Release();
  m_pBuilder = NULL;
  if ( m_pStatus )
    m_pStatus->Release();
  m_pStatus = NULL;
}

void Complex::InitExecutionStatus( CExecutionStatus* Status )
{
  if ( m_pStatus )
    m_pStatus->Release();
  m_pStatus = CExecutionStatus::Create( Status );
  if ( GetBuilder() )
    GetBuilder()->SetExecutionStatus( m_pStatus );
}

int Complex::GetInputsCount()
{
  if ( (GetBuilder()) && (GetBuilder()->IsDirty()) )
  {
    UnregisterConnectors();
    RegisterConnectors();
    GetBuilder()->SetDirty( FALSE );
    Status().WriteBool( STATUS_MODIFIED , true );
  }
  return (int)m_Inputs.GetSize();
}

int Complex::GetOutputsCount()
{
  if ( (GetBuilder()) && (GetBuilder()->IsDirty()) )
  {
    UnregisterConnectors();
    RegisterConnectors();
    GetBuilder()->SetDirty( FALSE );
    Status().WriteBool( STATUS_MODIFIED , true );
  }
  return (int) m_Outputs.GetSize();
}

int Complex::GetDuplexCount()
{
  if ( GetBuilder()->IsDirty() )
  {
    UnregisterConnectors();
    RegisterConnectors();
    GetBuilder()->SetDirty( FALSE );
    Status().WriteBool( STATUS_MODIFIED , true );
  }
  return (int) m_Duplex.GetSize();
}

CInputConnector* Complex::GetInputConnector( int n )
{
  if ( GetBuilder()->IsDirty() )
  {
    UnregisterConnectors();
    RegisterConnectors();
    GetBuilder()->SetDirty( FALSE );
    Status().WriteBool( STATUS_MODIFIED , true );
  }
  if ( n >= 0 && n < GetInputsCount() )
    return (CInputConnector*) m_Inputs.GetAt( n );
  return NULL;
}

COutputConnector* Complex::GetOutputConnector( int n )
{
  if ( GetBuilder()->IsDirty() )
  {
    UnregisterConnectors();
    RegisterConnectors();
    GetBuilder()->SetDirty( FALSE );
    Status().WriteBool( STATUS_MODIFIED , true );
  }
  if ( n >= 0 && n < GetOutputsCount() )
    return (COutputConnector*) m_Outputs.GetAt( n );
  return NULL;
}

CDuplexConnector* Complex::GetDuplexConnector( int n )
{
  if ( GetBuilder()->IsDirty() )
  {
    UnregisterConnectors();
    RegisterConnectors();
    GetBuilder()->SetDirty( FALSE );
    Status().WriteBool( STATUS_MODIFIED , true );
  }
  if ( n >= 0 && n < GetDuplexCount() )
    return (CDuplexConnector*) m_Duplex.GetAt( n );
  return NULL;
}

bool Complex::PrintProperties( FXString& text )
{
  if ( m_LoadPath.IsEmpty() )	// incorporated complex gadget
  {
    FXString script;
    if ( !GetBuilder() || !GetBuilder()->GetScript( script ) )
      return false;
    text.Format( "{%s}" , script );
  }
  else // external complex gadget
  {
    CString tmpS = (LPCTSTR)FxGetFileName( m_LoadPath ); // remove path to file
    text.Format( "\"%s\"" , tmpS );
  }
  return true;
}

bool Complex::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  if ( !GetBuilder() )
    return false;
  CString param = text;
  param.TrimLeft( " \t\r\n" );
  param.TrimRight( " \t\r\n" );
  if ( param.IsEmpty() )
    return false;
  char first = param.GetAt( 0 );
  if ( first == '"' ) // external script
  {
    int pos = param.ReverseFind( '"' ) + 1;
    m_LoadPath = param.Left( pos );
    m_LoadPath.TrimLeft( "\" \t\r\n" );
    m_LoadPath.TrimRight( "\" \t\r\n" );
    m_LoadPath = FxGetFileName( m_LoadPath ); // repmove path if any (bug with saving full path)
    FXString libPath;
    GetUserLibraryPath( libPath );
    m_LoadPath = libPath + '\\' + m_LoadPath;
    if ( m_LoadPath.IsEmpty() )
      return false;
    UnregisterConnectors();
    if ( GetBuilder()->Load( m_LoadPath ) >= MSG_ERROR_LEVEL )
    {
      m_LoadPath.Empty();
      return false;
    }
    RegisterConnectors();
    param = param.Mid( pos );
    CGadget::ScanProperties( param , Invalidate );
    m_bModified = false;
    return true;
  }
  else if ( first == '{' ) // incorporated script
  {
    m_LoadPath.Empty();
    int pos = param.ReverseFind( '}' ) + 1;
    CString paramEx = param.Mid( pos );
    param = param.Left( pos );
    param.TrimLeft( _T( "{ \r\t\n" ) );
    param.TrimRight( _T( "} \r\t\n" ) );
    UnregisterConnectors();
    if ( GetBuilder()->Load( NULL , param ) >= MSG_ERROR_LEVEL )
    {
      m_LoadPath.Empty();
      return false;
    }
    RegisterConnectors();
    CGadget::ScanProperties( paramEx , Invalidate );
    m_bModified = false;
    return true;
  }
  return false; // syntax error
}

static int CompareAscending( const void *a , const void *b )
{
  CString *pA = (CString*) a;
  CString *pB = (CString*) b;
  return (pA->CompareNoCase( *pB ));
}


bool Complex::ScanSettings( FXString& text )
{
  int i;
  bool retV = false;
  CString MyName;
  text.Empty();

  IGraphbuilder* iGB = GetBuilder();
  if ( !iGB ) 
    return retV;

  if ( !iGB->GetID() ) return false;
  MyName = iGB->GetID();

  CStringArray srcGadgets;
  CStringArray dstGadgets;
  iGB->EnumGadgets( srcGadgets , dstGadgets );
  srcGadgets.Append( dstGadgets ) ;

  qsort( (void*) &srcGadgets[ 0 ] , srcGadgets.GetCount() , sizeof( CString* ) , CompareAscending ) ;

  for ( i = 0; i < srcGadgets.GetSize(); i++ )
  {
    FXString settings;
    if ( iGB->ScanSettings( srcGadgets[ i ] , settings ) )
    {
      FXString sItem; sItem.Format( _T( "settings(name(%s),%s)\n" ) , srcGadgets[ i ] , settings );
      text += sItem;
      retV = true;
    }
  }
//   for ( i = 0; i < dstGadgets.GetSize(); i++ )
//   {
//     FXString settings;
//     if ( iGB->ScanSettings( dstGadgets[ i ] , settings ) )
//     {
//       FXString sItem; sItem.Format( _T( "settings(name(%s),%s)\n" ) , dstGadgets[ i ] , settings );
//       text += sItem;
//       retV = true;
//     }
//   }
  return retV;
}

void Complex::SetBuilder( IGraphbuilder* pBuilder , BOOL bLoadLocalPlugins )
{
  UnregisterConnectors();
  if ( GetBuilder() )
    GetBuilder()->Release();
  m_pBuilder = pBuilder;
  if ( bLoadLocalPlugins && m_pBuilder )
    m_pBuilder->GetPluginLoader()->RegisterPlugins( m_pBuilder );
  RegisterConnectors();
  if ( m_pStatus )
    GetBuilder()->SetExecutionStatus( m_pStatus );
  Status().WriteBool( STATUS_MODIFIED , true );
}

void Complex::SetLoadPath( LPCTSTR lpszPath )
{
  m_LoadPath = lpszPath;
}

void Complex::SetGraphMsgQueue( IGraphMsgQueue* MsgQueue )
{
  if ( GetBuilder() )
    GetBuilder()->SetMsgQueue( MsgQueue );
}

IGraphbuilder* Complex::GetBuilder()
{
  return (IGraphbuilder*) m_pBuilder;
}

void Complex::UnregisterConnectors()
{
  m_Outputs.RemoveAll();
  m_Inputs.RemoveAll();
  m_Duplex.RemoveAll();
}

void Complex::RegisterConnectors()
{
  int i;
  if ( !GetBuilder() )
    return;
  int nInputs = GetBuilder()->GetGraphInputsCount();
  for ( i = 0; i < nInputs; i++ )
    m_Inputs.Add( GetBuilder()->GetGraphInput( i ) );
  int nOutputs = GetBuilder()->GetGraphOutputsCount();
  for ( i = 0; i < nOutputs; i++ )
    m_Outputs.Add( GetBuilder()->GetGraphOutput( i ) );
  int nDuplex = GetBuilder()->GetGraphDuplexPinsCount();
  for ( i = 0; i < nDuplex; i++ )
    m_Duplex.Add( GetBuilder()->GetGraphDuplexPin( i ) );
}

int Complex::DoJob()
{
  return FXWorker::WR_EXIT;
}


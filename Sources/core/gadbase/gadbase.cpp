// gadbase.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <afxwin.h>
#include <afxdllx.h>
#include <gadgets\gadbase.h>
#include <gadgets\containerframe.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// DLL entry point

static AFX_EXTENSION_MODULE gadbaseDLL = {NULL, NULL};

extern "C" int APIENTRY
DllMain( HINSTANCE hInstance , DWORD dwReason , LPVOID lpReserved )
{
  // Remove this if you use lpReserved
  UNREFERENCED_PARAMETER( lpReserved );

  if ( dwReason == DLL_PROCESS_ATTACH )
  {
    TRACE0( "gadbase.DLL Initializing!\n" );

    // Extension DLL one-time initialization
    if ( !AfxInitExtensionModule( gadbaseDLL , hInstance ) )
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

    new CDynLinkLibrary( gadbaseDLL );

  }
  else if ( dwReason == DLL_PROCESS_DETACH )
  {
    TRACE0( "gadbase.DLL Terminating!\n" );

    // Terminate the library before destructors are called
    AfxTermExtensionModule( gadbaseDLL );
  }
  return 1;   // ok
}

static bool _attached = false;

void attachgadbaseDLL()
{
  if ( !_attached )
  {
    new CDynLinkLibrary( gadbaseDLL );
    _attached = true;
  }
}

FXArray<CUserDataFrame::CUserDataType> CUserDataFrame::m_UserTypes ;
FXLockObject                           CUserDataFrame::m_Lock ;

#define THIS_MODULENAME "gadbase.cpp"

//
// FXWorker
//   CGadget
//     CCaptureGadget
//     CFilterGadget
//     CRenderGadget
//     

BOOL CRuntimeGadget::IsDerivedFrom( const CRuntimeGadget* pBaseClass ) const
{
  CRuntimeGadget* RuntimeGadget = (CRuntimeGadget*) this;
  while ( RuntimeGadget )
  {
    if ( !_tcscmp( RuntimeGadget->m_lpszClassName , pBaseClass->m_lpszClassName ) )
      return TRUE;
    RuntimeGadget = RuntimeGadget->m_pBaseGadget;
  }
  return FALSE;
}

CRuntimeGadget CGadget::classCGadget =
{
  _T( "CGadget" ),
  NULL,
  NULL,
  NULL,
  NULL
};

CGadget::CGadget() :
  m_SetupObject( NULL ) ,
  m_bModified( FALSE ) ,
  m_pFnGetGraphTime( NULL ) ,
  m_pFnGetGadgetName( NULL ) ,
  m_pHost( NULL ) ,
  m_IsAboutToShutDown( FALSE ) ,
  m_Mode( mode_process ) ,
  m_Invalid( false ) ,
  m_pStatus( NULL ) ,
  m_pModifiedUIDs( NULL )
{
  Create();
  SetTicksIdle( 0 );
#ifdef _DEBUG
  m_ShtDnCls = 0;
#endif
}

// void CGadget::InitExecutionStatus( CExecutionStatus* Status )
// {
//   bool Suspended = ( ::WaitForSingleObject( m_evResume , 0 ) != WAIT_OBJECT_0 );
//   if ( !Suspended )
//     Suspend();
//   if ( m_pStatus != NULL )
//     ::SetEvent( m_pStatus->GetStartHandle() );
//   if ( m_pStatus != Status )
//   {
//     if ( m_pStatus )
//       m_pStatus->Release();
//     m_pStatus = CExecutionStatus::Create( Status );
//   }
//   Resume();
// }

BOOL CGadget::IsAboutToShutDown()
{
  return m_IsAboutToShutDown;
}

void CGadget::ShutDown()
{
  //FXAutolock al(m_PinsLock);
  m_IsAboutToShutDown = TRUE;
  if ( m_SetupObject )
    m_SetupObject->Delete();
  m_SetupObject = NULL;
#ifdef _DEBUG
  // Check for second call to shutdown function
  if ( m_ShtDnCls != 0 )
  {
    TRACE( "!!! Warrning: Multiple shutdown calling for gadget '%s'\n" , m_Name );
    ASSERT( m_ShtDnCls == 0 );
  }
  m_ShtDnCls++;
#endif
  int i;
  for ( i = 0; i < GetInputsCount(); i++ )
  {
    if ( GetInputConnector( i ) != NULL )
    {
      GetInputConnector( i )->Disconnect();
      GetInputConnector( i )->Close();
    }
  }
  for ( i = 0; i < GetOutputsCount(); i++ )
  {
    if ( GetOutputConnector( i ) )
      GetOutputConnector( i )->Disconnect();
  }
  Destroy(); // this waits until quiry&processing thread is actually closed
  if ( m_pStatus )
    m_pStatus->Release();
  m_pStatus = NULL;
}


int CGadget::GetDuplexCount()
{
  return 0;
}

CDuplexConnector* CGadget::GetDuplexConnector( int n )
{
  return NULL;
}

void CGadget::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pDataFrame )
{
  if ( pDataFrame )
    pDataFrame->Release();
}


BOOL CGadget::IsKindOf( CRuntimeGadget* RuntimeGadget )
{
  return GetRuntimeGadget()->IsDerivedFrom( RuntimeGadget );
}

BOOL CGadget::IsSetupOn()
{
  return ((m_SetupObject != NULL) && m_SetupObject->IsOn());
}

bool CGadget::PrintProperties( FXString& text )
{
  FXPropertyKit pk( text );
  int i;
  FXString uidPin , pinSettings;
  for ( i = 0; i < GetOutputsCount(); i++ )
  {
    uidPin.Format( _T( "_GADGET_OUTPUT%d" ) , i + 1 );
    CConnector* Connector = GetOutputConnector( i );
    FXPropertyKit pinPK;
    if ( Connector )
    {
//       if ( _tcslen( Connector->GetName() ) != 0 )
//         pinPK.WriteString( _T( "Name" ) , Connector->GetName() );
      if ( _tcslen( Connector->GetLabel() ) != 0 )
        pinPK.WriteString( _T( "Label" ) , Connector->GetLabel() );
    }
    if ( !pinPK.IsEmpty() )
      pk.WriteString( uidPin , pinPK );
  }
  for ( i = 0; i < GetInputsCount(); i++ )
  {
    uidPin.Format( _T( "_GADGET_INPUT%d" ) , i + 1 );
    CConnector* Connector = GetInputConnector( i );
    FXPropertyKit pinPK;
    if ( Connector )
    {
//       if ( _tcslen( Connector->GetName() ) != 0 )
//         pinPK.WriteString( _T( "Name" ) , Connector->GetName() );
      if ( _tcslen( Connector->GetLabel() ) != 0 )
        pinPK.WriteString( _T( "Label" ) , Connector->GetLabel() );
    }
    if ( !pinPK.IsEmpty() )
      pk.WriteString( uidPin , pinPK );
  }
  for ( i = 0; i < GetDuplexCount(); i++ )
  {
    uidPin.Format( _T( "_GADGET_DUPLEX%d" ) , i + 1 );
    CConnector* Connector = GetDuplexConnector( i );
    FXPropertyKit pinPK;
    if ( Connector )
    {
//       if ( _tcslen( Connector->GetName() ) != 0 )
//         pinPK.WriteString( _T( "Name" ) , Connector->GetName() );
      if ( _tcslen( Connector->GetLabel() ) != 0 )
        pinPK.WriteString( _T( "Label" ) , Connector->GetLabel() );
    }
    if ( !pinPK.IsEmpty() )
      pk.WriteString( uidPin , pinPK );
  }
  DWORD Affinity = GetAffinity() ;
  if ( Affinity != m_ProcessorMask )
    pk.WriteInt( "Affinity" , Affinity );
  if ( GetCoresNumber() > 1 )
    pk.WriteInt( "_GADGET_CORES" , GetCoresNumber() );
  if ( m_Mode != mode_process )
    pk.WriteInt( "_GADGET_MODE" , m_Mode );
  if ( m_Priority != THREAD_PRIORITY_NORMAL )
  {
    pk.WriteInt( "_GADGET_PRIORITY" , m_Priority ) ;
  }
  text = pk;
  return true;	// properties printed ok
}

bool CGadget::PrintProperties( FXString& text , LPCTSTR pPropName )
{
  return PrintProperties( text ) ;
}


bool CGadget::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pk( text );
  int affinity = 0;
  if ( pk.GetInt( "Affinity" , affinity ) && ((DWORD) affinity != GetAffinity()) )
    SetAffinity( affinity );
  int i;
  FXString uidPin , pinSettings;
  for ( i = 0; i < GetOutputsCount(); i++ )
  {
    uidPin.Format( _T( "_GADGET_OUTPUT%d" ) , i + 1 );
    CConnector* Connector = GetOutputConnector( i );
    if ( Connector && pk.GetString( uidPin , pinSettings ) )
    {
      FXPropertyKit pinPK( pinSettings );
      FXString name , label;
      if ( (pinPK.GetString( _T( "Name" ) , name )) && (name.GetLength() != 0) )
        Connector->SetName( name );
      if ( (pinPK.GetString( _T( "Label" ) , label )) && (label.GetLength() != 0) )
        Connector->SetLabel( label );
    }
  }
  for ( i = 0; i < GetInputsCount(); i++ )
  {
    uidPin.Format( _T( "_GADGET_INPUT%d" ) , i + 1 );
    CConnector* Connector = GetInputConnector( i );
    if ( Connector && pk.GetString( uidPin , pinSettings ) )
    {
      FXPropertyKit pinPK( pinSettings );
      FXString name , label;
      if ( (pinPK.GetString( _T( "Name" ) , name )) && (name.GetLength() != 0) )
        Connector->SetName( name );
      if ( (pinPK.GetString( _T( "Label" ) , label )) && (label.GetLength() != 0) )
        Connector->SetLabel( label );
    }
  }
  for ( i = 0; i < GetDuplexCount(); i++ )
  {
    uidPin.Format( _T( "_GADGET_DUPLEX%d" ) , i + 1 );
    CConnector* Connector = GetDuplexConnector( i );
    if ( Connector && pk.GetString( uidPin , pinSettings ) )
    {
      FXPropertyKit pinPK( pinSettings );
      FXString name , label;
      if ( (pinPK.GetString( _T( "Name" ) , name )) && (name.GetLength() != 0) )
        Connector->SetName( name );
      if ( (pinPK.GetString( _T( "Label" ) , label )) && (label.GetLength() != 0) )
        Connector->SetLabel( label );
    }
  }
  int nCores;
  if ( pk.GetInt( "_GADGET_CORES" , nCores ) && (nCores != GetCoresNumber()) )
    SetCoresNumber( nCores );
  int mode;
  if ( pk.GetInt( "_GADGET_MODE" , mode ) )
    m_Mode = mode;
  int Priority ;
  if ( pk.GetInt( "_GADGET_PRIORITY" , Priority ) )
  {
    SetPriority( Priority ) ;
  }

  return true;	// properties loaded ok
}

CRuntimeGadget* CGadget::GetRuntimeGadget()
{
  return &CGadget::classCGadget;
}

IMPLEMENT_RUNTIME_GADGET( CCaptureGadget , CGadget , _T( "Generic capture" ) );
#undef THIS_MODULENAME 
#define THIS_MODULENAME _T("TvdBase.CaptureGadget")

CCaptureGadget::CCaptureGadget() :
  m_pOutput( NULL ) ,
  m_FrameCounter( 0 ) ,
  m_bRun( FALSE )
{}

void CCaptureGadget::ShutDown()
{
  if ( m_evExit )
    SetEvent( m_evExit ) ;
  CGadget::ShutDown();
  Destroy();
  if ( m_pStatus )
    m_pStatus->Release();
  m_pStatus = NULL;
  OnStop();
}

void CCaptureGadget::OnStart()
{
  if ( m_pStatus )
    m_pStatus->Start() ;
  m_bRun = TRUE;
}

void CCaptureGadget::OnStop()
{
  if ( m_pStatus )
    m_pStatus->Stop() ;
  m_bRun = FALSE;
}

int CCaptureGadget::GetInputsCount()
{
  return 0;
}

int CCaptureGadget::GetOutputsCount()
{
  return (m_pOutput) ? 1 : 0;
}

CInputConnector* CCaptureGadget::GetInputConnector( int n )
{
  return NULL;
}

COutputConnector* CCaptureGadget::GetOutputConnector( int n )
{
  return (!n) ? m_pOutput : NULL;
}

// void CCaptureGadget::InitExecutionStatus( CExecutionStatus* Status )
// {
//   bool Suspended = (::WaitForSingleObject( m_evResume , 0 ) != WAIT_OBJECT_0);
//   if ( !Suspended )
//     Suspend();
//   if ( m_pStatus != NULL )
//     ::SetEvent( m_pStatus->GetStartHandle() );
//   if ( m_pStatus != Status )
//   {
//     if ( m_pStatus )
//       m_pStatus->Release();
//     m_pStatus = CExecutionStatus::Create( Status );
//   }
//   Resume();
// }

int CCaptureGadget::DoJob()
{
  CDataFrame* pDataFrame = NULL;
  if ( !m_pStatus ) 
    return WR_EXIT;
  switch ( m_pStatus->GetStatus() )
  {
    case CExecutionStatus::STOP:
      if ( m_bRun )
      {
        m_bRun = FALSE;
        OnStop();
        pDataFrame = GetNextFrame( NULL );
        if ( pDataFrame )
          Tvdb400_SetEOS( pDataFrame );
        else
        {
          pDataFrame = CDataFrame::Create( transparent );
          Tvdb400_SetEOS( pDataFrame );
        }
        m_FrameCounter = 0;
        break;
      }
      else
      {
        HANDLE pEvents[] = { m_evExit, m_pStatus->GetStartHandle()/*, m_evResume*/ };
        DWORD cEvents = sizeof( pEvents ) / sizeof( HANDLE );
        DWORD retVal = ::WaitForMultipleObjects( cEvents , pEvents , FALSE , INFINITE );
        return WR_CONTINUE;
      }
    case CExecutionStatus::PAUSE:
    {
      HANDLE pEvents[] = {m_evExit, m_pStatus->GetStartHandle(),m_pStatus->GetStpFwdHandle()};
      DWORD cEvents = sizeof( pEvents ) / sizeof( HANDLE );
      DWORD retVal = ::WaitForMultipleObjects( cEvents , pEvents , FALSE , INFINITE );
      if ( retVal != 2 )
        return WR_CONTINUE;
    }
    case CExecutionStatus::RUN:
    {
    #ifdef _DEBUG
      {
        FXString Name;
        GetGadgetName( Name );
        //TRACE("Capture \"%s\", graph time = %f\n", Name, GetGraphTime() * 1.e-3);
      }
    #endif
      if ( !m_bRun )
        OnStart();
      double ts = GetHRTickCount();
      pDataFrame = GetNextFrame( &ts );
      AddCPUUsage( GetHRTickCount() - ts );
      if ( pDataFrame )
      {
        if ( pDataFrame->GetId() == NOSYNC_FRAME )
          pDataFrame->ChangeId( ++m_FrameCounter );
        pDataFrame->SetTime( GetGraphTime() * 1.e-3 );
      }
      break;
    }
    case CExecutionStatus::EXIT:
      return WR_EXIT;
    default:
      ASSERT( FALSE );
      return WR_CONTINUE;
  }
  if ( pDataFrame )
  {
    if ( (!m_pOutput) || (!m_pOutput->Put( pDataFrame )) )
      pDataFrame->Release();
  }
  return WR_CONTINUE;
}

CDataFrame* CCaptureGadget::GetNextFrame( double* StartTime )
{
  return NULL;
}

IMPLEMENT_RUNTIME_GADGET( CFilterGadget , CGadget , "Generic filter" );
#undef THIS_MODULENAME 
#define THIS_MODULENAME _T("TvdBase.FilterGadget")

CFilterGadget::CFilterGadget() :
  m_OutputMode( modeReplace ) ,
  m_pInput( NULL ) ,
  m_pOutput( NULL )
{}

void CFilterGadget::ShutDown()
{
  CGadget::ShutDown();
}

int CFilterGadget::GetInputsCount()
{
  return (m_pInput) ? 1 : 0;
}

int CFilterGadget::GetOutputsCount()
{
  return (m_pOutput) ? 1 : 0;
}

CInputConnector* CFilterGadget::GetInputConnector( int n )
{
  return (!n) ? m_pInput : NULL;
}

COutputConnector* CFilterGadget::GetOutputConnector( int n )
{
  return (!n) ? m_pOutput : NULL;
}

bool CFilterGadget::PrintProperties( FXString& text )
{
  FXPropertyKit pk( text );
  pk.WriteInt( "_OUTPUTMODE" , (int) m_OutputMode );
  text = pk;
  return CGadget::PrintProperties( text );
}

bool CFilterGadget::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pk( text );
  int om = -1;
  if ( (pk.GetInt( "_OUTPUTMODE" , om )) && (m_OutputMode != (OutputMode) om) )
    m_OutputMode = (OutputMode) om;
  return CGadget::ScanProperties( text , Invalidate );
}

int CFilterGadget::DoJob()
{
  CDataFrame* pDataFrame = NULL;
  if ( (m_pInput) && (m_pInput->Get( pDataFrame )) )
  {
    ASSERT( pDataFrame );
    if ( (m_pOutput) && (Tvdb400_IsEOS( pDataFrame )) ) // if EOS - just pass dataframe through without processing for common types gadgets
    {
      if ( !m_pOutput->Put( pDataFrame ) )
        pDataFrame->Release();
      OnEOS();
    }
    else switch ( m_Mode )
    {
      case mode_reject:
        pDataFrame->Release();
        break;
      case mode_transmit:
        if ( !m_pOutput || !m_pOutput->Put( pDataFrame ) )
          pDataFrame->Release();
        break;
      case mode_process:
      {
        double ts = GetHRTickCount();
        CDataFrame* pResultFrame = NULL;
        if ( !m_Invalid )
        {
        #if (!defined(NO_CATCH_EXCEPTION_IN_DEBUG) || !defined(_DEBUG))
          __try
          #else
        #pragma message(__FILE__"[" STRING(__LINE__) "] : warning HB0001: The exception catching is disabled!")
        #endif
          {
            pResultFrame = DoProcessing( pDataFrame );
            AddCPUUsage( GetHRTickCount() - ts );
            switch ( m_OutputMode )
            {
              case modeAppend:
              {
                if ( pResultFrame && (pDataFrame != pResultFrame) )
                {
                  if ( (pDataFrame->IsContainer()) )
                  {
                    CContainerFrame* pCopyFrame = (CContainerFrame*) pDataFrame->Copy();
                    pCopyFrame->AddFrame( pResultFrame );
                    if ( !m_pOutput || !m_pOutput->Put( pCopyFrame ) )
                      pCopyFrame->Release();
                  }
                  else
                  {
                    pDataFrame->AddRef() ;
                    if ( pResultFrame->IsContainer() )
                      ((CContainerFrame*) pResultFrame)->AddFrame( pDataFrame ) ;
                    else
                    {
                      CContainerFrame * pNewContainer = CContainerFrame::Create() ;
                      pNewContainer->AddFrame( pDataFrame ) ;
                      pNewContainer->AddFrame( pResultFrame ) ;
                      pResultFrame = pNewContainer ;
                    }
                    if ( (m_pOutput) && (!m_pOutput->Put( pResultFrame )) )
                      pResultFrame->Release();
                  }
                }
                else
                {
                  pDataFrame->AddRef() ;
                  if ( (m_pOutput) && (!m_pOutput->Put( pDataFrame )) )
                    pDataFrame->Release();
                }
                break;
              }
              case modeReplace:
              {
                if ( pResultFrame )
                {
                  if ( (m_pOutput) && (!m_pOutput->Put( pResultFrame )) )
                    pResultFrame->RELEASE( pResultFrame );
                }
                break;
              }
            }
          }
        #if (!defined(NO_CATCH_EXCEPTION_IN_DEBUG) || !defined(_DEBUG))
          __except ( 1 )
          {
            {
              SENDERR_2( "First-chance exception 0x%x in gadget '%s'" , GetExceptionCode() , m_Name );
              m_Invalid = true;
            }
          }
        #endif
        }
        pDataFrame->Release();
        break;
      }
      default:
        ASSERT( FALSE );
        pDataFrame->Release();
    }
  }
  if ( m_pInput && ( (m_pInput->GetNFramesInQueue() > (m_pInput->GetQueueSize() - 1) ) ) )
    Sleep( 5 ) ;

  return WR_CONTINUE;
}

CDataFrame* CFilterGadget::DoProcessing( const CDataFrame* pDataFrame )
{
  return NULL;
}

void CFilterGadget::SetOutputMode( CFilterGadget::OutputMode om )
{
  m_OutputMode = om;
}

CFilterGadget::OutputMode CFilterGadget::GetOutputMode()
{
  return m_OutputMode;
}

void CFilterGadget::OnEOS()
{}

// CPortGadget

IMPLEMENT_RUNTIME_GADGET( CPortGadget , CFilterGadget , "Generic port" );
#undef THIS_MODULENAME 
#define THIS_MODULENAME _T("TvdBase.PortGadget")

CPortGadget::CPortGadget() : CFilterGadget()
{}


IMPLEMENT_RUNTIME_GADGET( CCtrlGadget , CCaptureGadget , "Generic ctrl" );
#undef THIS_MODULENAME 
#define THIS_MODULENAME _T("TvdBase.CtrlGadget")

CCtrlGadget::CCtrlGadget() :
  m_Monitor( NULL )
  , m_hParentWnd( NULL )
{}

void CCtrlGadget::ShutDown()
{
  CCaptureGadget::ShutDown();
  if ( m_Monitor )
    free( m_Monitor );
  m_Monitor = NULL;
}

LPCTSTR CCtrlGadget::GetMonitor()
{
  return (LPCTSTR) m_Monitor;
}

void CCtrlGadget::SetMonitor( LPCTSTR monitor )
{
  if ( m_Monitor )
    free( m_Monitor );
  m_Monitor = NULL ;
  m_Monitor = (LPTSTR) calloc( strlen( monitor ) + 1 , sizeof( TCHAR ) );
  strcpy( m_Monitor , monitor );
}

void CCtrlGadget::Create()
{}


bool CCtrlGadget::PrintProperties( FXString& text )
{
  if ( !m_Monitor )
    return false;

  FXPropertyKit pc;
  pc.WriteString( _T( "Name" ) , m_Monitor );
  text = pc;
  return true;
}

bool CCtrlGadget::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  if ( (strlen( text ) != 0) && (strchr( text , '=' ) == NULL) ) // old version
  {
    FXString monitor( text );
    monitor.TrimLeft( _T( "\" \t\r\n" ) );
    monitor.TrimRight( _T( "\" \t\r\n" ) );
    SetMonitor( monitor );
    return true;
  }
  FXString monitor;
  FXPropertyKit pc( text );
  if ( (pc.GetString( _T( "Name" ) , monitor )) && (monitor.GetLength() != 0) )
  {
    SetMonitor( monitor );
    return true;
  }
  else
  {
    return true;
  }
  return false;
}

IMPLEMENT_RUNTIME_GADGET( CRenderGadget , CCtrlGadget , "Generic render" );
#undef THIS_MODULENAME 
#define THIS_MODULENAME "TvdBase.RenderGadget"

CRenderGadget::CRenderGadget() :
  m_pInput( NULL )
{}

void CRenderGadget::ShutDown()
{
  CCtrlGadget::ShutDown();
}

int CRenderGadget::GetInputsCount()
{
  return 1;
}

int CRenderGadget::GetOutputsCount()
{
  return 0;
}

CInputConnector* CRenderGadget::GetInputConnector( int n )
{
  return (!n) ? m_pInput : NULL;
}

COutputConnector* CRenderGadget::GetOutputConnector( int n )
{
  return NULL;
}

int CRenderGadget::DoJob()
{
  FXAutolock al( m_PinsLock );
  CDataFrame* pDataFrame = NULL;
  while ( (m_pInput) && (m_pInput->Get( pDataFrame )) )
  {
    double ts = GetHRTickCount();
    ASSERT( pDataFrame );
    if ( !Tvdb400_IsEOS( pDataFrame ) || ReceiveEOS( pDataFrame ) )
    {
      double ts = GetHRTickCount();
      Render( pDataFrame );
      AddCPUUsage( GetHRTickCount() - ts );
    }
    pDataFrame->Release();
  }
  return WR_CONTINUE;
}

void CRenderGadget::Render( const CDataFrame* pDataFrame )
{}

bool CRenderGadget::ReceiveEOS( const CDataFrame* pDataFrame )
{
  return true;
}

IMPLEMENT_RUNTIME_GADGET( CCollectorGadget , CGadget , LINEAGE_GENERIC );
#undef THIS_MODULENAME 
#define THIS_MODULENAME "TvdBase.CollectorGadget"

#define BUFFER_DEPTH 10

CCollectorGadget::CCollectorGadget( void ) :
  m_FramesBuffer( NULL ) ,
  m_nInputs( 0 ) ,
  m_nInBuffer( 0 ) ,
  m_pOutput( NULL )
{
  m_evDataReady = ::CreateEvent( NULL , FALSE , FALSE , NULL );
}

void CCollectorGadget::ShutDown()
{
  CGadget::ShutDown();
  RemoveInputs();
  if ( m_pOutput )
  {
    delete m_pOutput;
    m_pOutput = NULL;
  }
  ::CloseHandle( m_evDataReady );
}

int	 CCollectorGadget::DoJob()
{
  const HANDLE Events[] = {m_evExit, m_evDataReady};
  const DWORD cEvents = sizeof( Events ) / sizeof( HANDLE );
  DWORD waitRet = ::WaitForMultipleObjects( cEvents , Events , FALSE , 100 );
  if ( waitRet == WAIT_OBJECT_0 + 1 ) // if data ready
  {
    FXAutolock lock( m_Lock , "DoJob()" );
    int pos;
    double ts = GetHRTickCount();
    if ( FindComplete( pos ) )
      PrepareToSend( pos );
    AddCPUUsage( GetHRTickCount() - ts );
  }
  return WR_CONTINUE;
}

int CCollectorGadget::GetInputsCount()
{
  return m_nInputs;
}

CInputConnector* CCollectorGadget::GetInputConnector( int n )
{
  CInputConnector* pInput = NULL;
  FXAutolock lock( m_Lock , "GetInputConnector()" );
  if ( (n >= 0) && (n < (int)m_Inputs.GetSize()) )
    pInput = (CInputConnector*) m_Inputs.GetAt( n );
  return pInput;
}

void CCollectorGadget::CreateInputs( int n , basicdatatype type , bool bLock )
{
  FXAutolock* lock = (bLock) ? new FXAutolock( m_Lock , "CreateInputs()" ) : NULL;
  ClearBuffers();
  while ( (int) m_Inputs.GetSize() < n )
    m_Inputs.Add( new CInputConnector( type , CCollectorGadget_SendData , this ) );
  while ( (int) m_Inputs.GetSize() > n )
  {
    CInputConnector* pInput = (CInputConnector*) m_Inputs.GetAt( m_Inputs.GetSize() - 1 );
    m_Inputs.RemoveAt( m_Inputs.GetSize() - 1 );
    if ( pInput )
    {
      pInput->Disconnect();
      delete pInput;
    }
  }
  m_nInputs = (int)m_Inputs.GetSize();
  if ( m_FramesBuffer )
  {
    m_FramesBuffer = (CDataFrame**) realloc( m_FramesBuffer , m_nInputs*BUFFER_DEPTH * sizeof( CDataFrame** ) );
  }
  else
    m_FramesBuffer = (CDataFrame**) malloc( m_nInputs*BUFFER_DEPTH * sizeof( CDataFrame** ) );
  memset( m_FramesBuffer , 0 , m_nInputs*BUFFER_DEPTH * sizeof( CDataFrame** ) );
  if ( lock )
    delete lock;
}

void CCollectorGadget::RemoveInputs()
{
  FXAutolock lock( m_Lock , "RemoveInputs()" );
  while ( m_Inputs.GetSize() )
  {
    CInputConnector* pInput = (CInputConnector*) m_Inputs.GetAt( 0 );
    m_Inputs.RemoveAt( 0 );
    delete pInput;
  }
  ClearBuffers();
  if ( m_FramesBuffer ) free( m_FramesBuffer );
  m_FramesBuffer = NULL;
  m_nInBuffer = 0;
  m_nInputs = 0;
}

int CCollectorGadget::GetOutputsCount()
{
  return (m_pOutput != NULL) ? 1 : 0;
}

COutputConnector* CCollectorGadget::GetOutputConnector( int n )
{
  return (n < GetOutputsCount()) ? m_pOutput : NULL;
}

void CCollectorGadget::PrepareToSend( int pos )
{
  int i;
  CDataFrame** frames = GetFrame( pos , 0 );
  CDataFrame*  pRetV = DoProcessing( frames , m_nInputs );
  if ( pRetV )
  {
    //pDataFrame->CopyAttributes(frames[0]);
    if ( (!m_pOutput) || (!m_pOutput->Put( pRetV )) )
      pRetV->Release();
  }
  for ( i = 0; i <= pos; i++ )
  {
    for ( int j = 0; j < m_nInputs; j++ )
    {
      CDataFrame** pDF = GetFrame( i , j );
      if ( *pDF )
      {
        (*pDF)->RELEASE( *pDF );
        *pDF = NULL;
      }
    }
  }
  memcpy( m_FramesBuffer , m_FramesBuffer + m_nInputs * (pos + 1) , sizeof( CDataFrame** )*m_nInputs*(BUFFER_DEPTH - pos - 1) );
  memset( m_FramesBuffer + m_nInputs * (BUFFER_DEPTH - pos - 1) , 0 , sizeof( CDataFrame** )*m_nInputs*(pos + 1) );
  m_nInBuffer -= (pos + 1);
}


CDataFrame* CCollectorGadget::DoProcessing( CDataFrame const*const* frames , int nmb )
{
  //TRACE("Found complete data pack with ID=%d\n", (*frames)->GetId());
  ASSERT( *frames != NULL );
  return NULL;
}


void CCollectorGadget::Input( CDataFrame* pDataFrame , CConnector* lpInput )
{
  FXAutolock lock( m_Lock , "Input()" );
  int pos , pin = GetInputID( lpInput );
  if ( (m_pOutput) && (Tvdb400_IsEOS( pDataFrame )) ) // if EOS - just pass dataframe through without processing for common types gadgets
  {
    if ( !m_pOutput->Put( pDataFrame ) )
      pDataFrame->Release();
    ClearBuffers();
  }
  else if ( GetInputsCount() == 1 )
  {
    if ( !Add( pDataFrame , lpInput ) )
      pDataFrame->Release();
    else
      SetEvent( m_evDataReady );
  }
  else
  {
    //TRACE("CCollectorGadget::Input pin %d\n",pin);
    if ( pin < 0 )
    {
      pDataFrame->Release();
      return;
    }
    if ( FindInBuffers( pDataFrame->GetId() , pos ) )
    {
      if ( pin < 0 )
      {
        TRACE( "!!! Wrong pin 0x%x\n" , lpInput );
        pDataFrame->Release();
        return;
      }
      if ( *GetFrame( pos , pin ) != NULL )
      {
        TRACE( "!!! Data already got from pin 0x%x\n" , lpInput );
        pDataFrame->Release();
        return;
      }
      *GetFrame( pos , pin ) = pDataFrame;
      if ( IsComplete( pos ) )
        SetEvent( m_evDataReady );
    }
    else
    {
      if ( !Add( pDataFrame , lpInput ) )
        pDataFrame->Release();
    }
    //TraceBuffers();
  }
}

void CCollectorGadget::ClearBuffers()
{
  if ( m_FramesBuffer )
  {
    for ( int i = 0; i < m_nInBuffer; i++ )
    {
      for ( int j = 0; j < m_nInputs; j++ )
      {
        CDataFrame** pDF = GetFrame( i , j );
        if ( *pDF )
        {
          (*pDF)->RELEASE( *pDF );
          *pDF = NULL;
        }
      }
    }
    m_nInBuffer = 0;
  }
}

bool CCollectorGadget::FindInBuffers( DWORD ID , int& pos )
{
  if ( m_FramesBuffer )
  {
    for ( int i = 0; i < m_nInBuffer; i++ )
    {
      for ( int j = 0; j < m_nInputs; j++ )
      {
        CDataFrame** pDF = GetFrame( i , j );
        if ( *pDF )
        {
          if ( (*pDF)->GetId() == ID )
          {
            pos = i;
            return true;
          }
        }
      }
    }
  }
  return false;
}

int CCollectorGadget::GetInputID( CConnector* lpInput )
{
  for ( int i = 0; i < (int) m_Inputs.GetSize(); i++ )
  {
    if ( (CConnector*) m_Inputs.GetAt( i ) == lpInput )
      return i;
  }
  return -1;
}

bool CCollectorGadget::IsComplete( int pos )
{
  bool retV = true;
  CDataFrame** ptr = GetFrame( pos , 0 );
  for ( int i = 0; i < m_nInputs; i++ )
  {
    retV &= (*ptr != NULL);
    ptr++;
  }
  return retV;
}

bool CCollectorGadget::FindComplete( int& pos )
{
  for ( pos = 0; pos < m_nInBuffer; pos++ )
  {
    if ( IsComplete( pos ) )
      return true;
  }
  return false;
}

bool CCollectorGadget::Add( CDataFrame* pDataFrame , CConnector* lpInput )
{
  int i;
  if ( m_nInBuffer < BUFFER_DEPTH )
  {
    int pin = GetInputID( lpInput );
    ASSERT( *GetFrame( m_nInBuffer , pin ) == NULL );
    *GetFrame( m_nInBuffer , pin ) = pDataFrame;
    m_nInBuffer++;
    return true;
  }
  else //throw out outdated frames
  {
    CDataFrame** ptr = m_FramesBuffer;
    for ( i = 0; i < m_nInputs; i++ )
    {
      if ( *ptr != NULL )
      {
        (*ptr)->RELEASE( *ptr );
        *ptr = NULL;
      }
      ptr++;
    }
    memcpy( m_FramesBuffer , m_FramesBuffer + m_nInputs , sizeof( CDataFrame** )*m_nInputs*(BUFFER_DEPTH - 1) );
    for ( i = 0; i < m_nInputs; i++ )
    {
      *GetFrame( m_nInBuffer - 1 , i ) = NULL;
    }
    int pin = GetInputID( lpInput );
    *GetFrame( m_nInBuffer - 1 , pin ) = pDataFrame;
    return true;
  }
  return false;
}

#ifdef _DEBUG
void CCollectorGadget::TraceBuffers()
{
  if ( m_FramesBuffer )
  {
    for ( int i = 0; i < m_nInBuffer; i++ )
    {
      FXString OutS;
      for ( int j = 0; j < m_nInputs; j++ )
      {
        CDataFrame** pDF = GetFrame( i , j );
        FXString pinInfo;
        if ( *pDF )
          pinInfo.Format( " %d" , (*pDF)->GetId() );
        else
          pinInfo = " -";
        OutS += pinInfo;
      }
      TRACE( "%s\n" , OutS );
    }
  }
  //TRACE( "End buffer ===============================\n" );
}
#endif

////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_RUNTIME_GADGET( CTwoPinCollector , CGadget , "Video.logic" );

IMPLEMENT_RUNTIME_GADGET( CCollectorRenderer , CRenderGadget , LINEAGE_GENERIC );

#define BUFFER_DEPTH 10

CCollectorRenderer::CCollectorRenderer( void ) :
  m_FramesBuffer( NULL ) ,
  m_nInputs( 0 ) ,
  m_nInBuffer( 0 )
{
  Destroy();
}

void CCollectorRenderer::ShutDown()
{
  CRenderGadget::ShutDown();
  RemoveInputs();
}

int CCollectorRenderer::GetInputsCount()
{
  return m_nInputs;
}

CInputConnector* CCollectorRenderer::GetInputConnector( int n )
{
  CInputConnector* pInput = NULL;
  FXAutolock lock( m_Lock );
  if ( (n >= 0) && (n < (int) m_Inputs.GetSize()) )
    pInput = (CInputConnector*) m_Inputs.GetAt( n );
  return pInput;
}

void CCollectorRenderer::CreateInputs( int n , basicdatatype type )
{
  FXAutolock lock( m_Lock );
  ClearBuffers();
  while ( (int) m_Inputs.GetSize() < n )
    m_Inputs.Add( new CInputConnector( type , CCollectorRenderer_SendData , this ) );
  while ( (int) m_Inputs.GetSize() > n )
  {
    CInputConnector* pInput = (CInputConnector*) m_Inputs.GetAt( m_Inputs.GetSize() - 1 );
    if ( pInput )
    {
      pInput->Disconnect();
      delete pInput;
    }
    m_Inputs.RemoveAt( m_Inputs.GetSize() - 1 );
  }
  m_nInputs = (int)m_Inputs.GetSize();
  if ( m_FramesBuffer )
  {
    m_FramesBuffer = (CDataFrame**) realloc( m_FramesBuffer , m_nInputs*BUFFER_DEPTH * sizeof( CDataFrame** ) );
  }
  else
    m_FramesBuffer = (CDataFrame**) malloc( m_nInputs*BUFFER_DEPTH * sizeof( CDataFrame** ) );
  memset( m_FramesBuffer , 0 , m_nInputs*BUFFER_DEPTH * sizeof( CDataFrame** ) );
}

void CCollectorRenderer::RemoveInputs()
{
  FXAutolock lock( m_Lock );

  while ( m_Inputs.GetSize() )
  {
    CInputConnector* pInput = (CInputConnector*) m_Inputs.GetAt( 0 );
    m_Inputs.RemoveAt( 0 );
    delete pInput;
  }
  ClearBuffers();
  if ( m_FramesBuffer ) free( m_FramesBuffer );
  m_FramesBuffer = NULL;
  m_nInBuffer = 0;
  m_nInputs = 0;
}

void CCollectorRenderer::PrepareToSend( int pos )
{
  int i;
  CDataFrame** frames = GetFrame( pos , 0 );
  for ( i = 0; i < m_nInputs; i++ )
  {
    if ( *(frames + i) )
      (*(frames + i))->AddRef();
  }
  Render( frames , m_nInputs );
  for ( int i = 0; i < m_nInputs; i++ )
  {
    (*(frames + i))->Release( *(frames + i) );
  }
  for ( i = 0; i <= pos; i++ )
  {
    for ( int j = 0; j < m_nInputs; j++ )
    {
      CDataFrame** pDF = GetFrame( i , j );
      if ( *pDF )
      {
        (*pDF)->RELEASE( *pDF );
        *pDF = NULL;
      }
    }
  }
  memcpy( m_FramesBuffer , m_FramesBuffer + m_nInputs * (pos + 1) , sizeof( CDataFrame** )*m_nInputs*(BUFFER_DEPTH - pos - 1) );
  memset( m_FramesBuffer + m_nInputs * (BUFFER_DEPTH - pos - 1) , 0 , sizeof( CDataFrame** )*m_nInputs*(pos + 1) );
  m_nInBuffer -= (pos + 1);
}


void CCollectorRenderer::Render( CDataFrame const*const* frames , int nmb )
{
  //TRACE("Found complete data pack with ID=%d\n", (*frames)->GetId());
  ASSERT( *frames != NULL );
  for ( int i = 0; i < nmb; i++ )
  {
    ASSERT( *(frames + i) != NULL );
  }
}


void CCollectorRenderer::Input( CDataFrame* pDataFrame , CConnector* lpInput )
{
  FXAutolock lock( m_Lock );
  int pos;
  if ( FindInBuffers( pDataFrame->GetId() , pos ) )
  {
    int pin = GetInputID( lpInput );
    if ( pin < 0 )
    {
      TRACE( "!!! Wrong pin 0x%x\n" , lpInput );
      pDataFrame->Release();
      return;
    }
    if ( *GetFrame( pos , pin ) != NULL )
    {
      TRACE( "!!! Data already got from pin 0x%x\n" , lpInput );
      pDataFrame->Release();
      return;
    }
    *GetFrame( pos , pin ) = pDataFrame;
    if ( IsComplete( pos ) )
      PrepareToSend( pos );
  }
  else
  {
    if ( !Add( pDataFrame , lpInput ) )
      pDataFrame->Release();
  }
}

void CCollectorRenderer::ClearBuffers()
{
  if ( m_FramesBuffer )
  {
    for ( int i = 0; i < m_nInBuffer; i++ )
    {
      for ( int j = 0; j < m_nInputs; j++ )
      {
        CDataFrame** pDF = GetFrame( i , j );
        if ( *pDF )
        {
          (*pDF)->RELEASE( *pDF );
          *pDF = NULL;
        }
      }
    }
    m_nInBuffer = 0;
  }
}

bool CCollectorRenderer::FindInBuffers( DWORD ID , int& pos )
{
  if ( m_FramesBuffer )
  {
    for ( int i = 0; i < m_nInBuffer; i++ )
    {
      for ( int j = 0; j < m_nInputs; j++ )
      {
        CDataFrame** pDF = GetFrame( i , j );
        if ( *pDF )
        {
          if ( (*pDF)->GetId() == ID )
          {
            pos = i;
            return true;
          }
        }
      }
    }
  }
  return false;
}

int CCollectorRenderer::GetInputID( CConnector* lpInput )
{
  for ( int i = 0; i < (int) m_Inputs.GetSize(); i++ )
  {
    if ( (CConnector*) m_Inputs.GetAt( i ) == lpInput )
      return i;
  }
  return -1;
}

bool CCollectorRenderer::IsComplete( int pos )
{
  bool retV = true;
  CDataFrame** ptr = GetFrame( pos , 0 );
  for ( int i = 0; i < m_nInputs; i++ )
  {
    retV &= (*ptr != NULL);
    ptr++;
  }
  return retV;
}

bool CCollectorRenderer::Add( CDataFrame* pDataFrame , CConnector* lpInput )
{
  int i;
  if ( m_nInBuffer < BUFFER_DEPTH )
  {
    int pin = GetInputID( lpInput );
    ASSERT( *GetFrame( m_nInBuffer , pin ) == NULL );
    *GetFrame( m_nInBuffer , pin ) = pDataFrame;
    m_nInBuffer++;
    return true;
  }
  else //throw out outdated frames
  {
    CDataFrame** ptr = m_FramesBuffer;
    for ( i = 0; i < m_nInputs; i++ )
    {
      if ( *ptr != NULL )
      {
        (*ptr)->RELEASE( *ptr );
        *ptr = NULL;
      }
      ptr++;
    }
    memcpy( m_FramesBuffer , m_FramesBuffer + m_nInputs , sizeof( CDataFrame** )*m_nInputs*(BUFFER_DEPTH - 1) );
    for ( i = 0; i < m_nInputs; i++ )
    {
      *GetFrame( m_nInBuffer - 1 , i ) = NULL;
    }
    int pin = GetInputID( lpInput );
    *GetFrame( m_nInBuffer - 1 , pin ) = pDataFrame;
    return true;
  }
  return false;
}

IMPLEMENT_RUNTIME_GADGET( CVirtualGadget , CGadget , LINEAGE_VIRTUAL );

CVirtualGadget::CVirtualGadget()
{}

CVirtualGadget::~CVirtualGadget()
{
  ShutDown();
}

void CVirtualGadget::ShutDown()
{
  while ( m_Inputs.GetSize() )
  {
    CInputConnector* pInput = (CInputConnector*) m_Inputs.GetAt( 0 );
    m_Inputs.RemoveAt( 0 );
    delete pInput;
  }
  while ( m_Outputs.GetSize() )
  {
    COutputConnector* pOutput = (COutputConnector*) m_Outputs.GetAt( 0 );
    m_Outputs.RemoveAt( 0 );
    delete pOutput;
  }
  while ( m_Duplex.GetSize() )
  {
    CDuplexConnector* pDuplex = (CDuplexConnector*) m_Duplex.GetAt( 0 );
    m_Duplex.RemoveAt( 0 );
    delete pDuplex;
  }
}

int CVirtualGadget::GetInputsCount()
{
  return (int)m_Inputs.GetSize();
}

int CVirtualGadget::GetOutputsCount()
{
  return (int) m_Outputs.GetSize();
}

int CVirtualGadget::GetDuplexCount()
{
  return (int) m_Duplex.GetSize();
}

CInputConnector* CVirtualGadget::GetInputConnector( int n )
{
  int cInputs = (int) m_Inputs.GetSize();
  while ( cInputs <= n )
  {
    CInputConnector* pInput = new CInputConnector( transparent , VirtualGadgetInputFn , this );
    m_Inputs.Add( pInput );
    cInputs++;
  }
  return (CInputConnector*) m_Inputs.GetAt( n );
}

COutputConnector* CVirtualGadget::GetOutputConnector( int n )
{
  int cOutputs = (int) m_Outputs.GetSize();
  while ( cOutputs <= n )
  {
    COutputConnector* pOutput = new COutputConnector( transparent );
    m_Outputs.Add( pOutput );
    cOutputs++;
  }
  return (COutputConnector*) m_Outputs.GetAt( n );
}

CDuplexConnector* CVirtualGadget::GetDuplexConnector( int n )
{
  int cDuplex = (int) m_Duplex.GetSize();
  while ( cDuplex <= n )
  {
    CDuplexConnector* pDuplex = new CDuplexConnector( this , transparent , transparent );
    m_Duplex.Add( pDuplex );
    cDuplex++;
  }
  return (CDuplexConnector*) m_Duplex.GetAt( n );
}

bool CVirtualGadget::PrintProperties( FXString& text )
{
  text = (LPCTSTR) m_Params;
  return true;
}

bool CVirtualGadget::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  if ( Invalidate )
    m_Params.Empty();
  m_Params += text;
  return true;
}

bool CVirtualGadget::ScanSettings( FXString& text )
{
  text = m_Params;
  return false;
}

int CVirtualGadget::DoJob()
{
  return FXWorker::WR_EXIT;
}
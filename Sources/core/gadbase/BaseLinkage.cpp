#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "BaseLinkage.cpp"
#include <gadgets\gadbase.h>

void ProcessPinLogMessage(void * pMsg, FXString& outp)
{
  CDataFrame * pDataFrame = (CDataFrame*)pMsg;
  pDataFrame->ToLogString(outp);
  pDataFrame->Release();
}

//
// CConnector
//   CInputConnector
//     CBufferedInputConnector
//   COutPin
//     COutputConnector
//     CDuplexConnector
//
FXLoggerToFile CConnector::m_PinLogger( _T("PinLogger") , &ProcessPinLogMessage ) ;


CConnector::CConnector( datatype dtype ) :
  m_FramesPassed( 0 ) ,
  m_FramesSkipped( 0 ) ,
  m_iLogging( 0 ) ,
  m_bIsVisible( true )
{
  m_DataType = dtype;
}

CConnector::~CConnector()
{}

bool CConnector::Connect( CConnector* pConnector )
{
  CWire* pWire = NULL;
  CConnector* pOut = GetOutputPin();
  CConnector* pIn = pConnector->GetInputPin();
  if ( pOut && pIn && Tvdb400_TypesCompatible( pOut->GetDataType() , pIn->GetDataType() ) )
  {
    pWire = new CWire( pOut , pIn );
    pOut->m_Wires.AddLocked( pWire );
    pIn->m_Wires.AddLocked( pWire );
    return true;
  }
  pOut = pConnector->GetOutputPin();
  pIn = GetInputPin();
  if ( pOut && pIn && Tvdb400_TypesCompatible( pOut->GetDataType() , pIn->GetDataType() ) )
  {
    pWire = new CWire( pOut , pIn );
    pOut->m_Wires.AddLocked( pWire );
    pIn->m_Wires.AddLocked( pWire );
    return true;
  }
  return false;
}

bool CConnector::Disconnect( CConnector* pConnector )
{
  bool bResult = false;
  int i;
  m_Wires.LockPtr();
  for ( i = 0; i < (int) m_Wires.GetSize(); i++ )
  {
    CWire* Wire = (CWire*) m_Wires.GetAt( i );
    if ( !pConnector ||
      (Wire->m_pConnector1 == GetInputPin() && Wire->m_pConnector2 == pConnector->GetOutputPin()) ||
      (Wire->m_pConnector2 == pConnector->GetInputPin() && Wire->m_pConnector1 == GetOutputPin()) )
    {
      Wire->m_pConnector1->m_Wires.RemoveElement( Wire );
      Wire->m_pConnector2->m_Wires.RemoveElement( Wire );
      delete Wire;
      i--;
      bResult = true;
    }
  }
  m_Wires.UnlockPtr();
  return bResult;
}

bool CConnector::GetPinInfo( FXString& result )
{
  datatype outtype , intype;
  if ( GetOutputPin( &outtype ) )
  {
    if ( GetInputPin( &intype ) )
      result.Format( "(out: %s, \"%s\", in: %s) %s" , Tvdb400_TypeToStr( outtype ) , m_Label , Tvdb400_TypeToStr( intype ) , m_Name );
    else
      result.Format( "(%s, \"%s\") %s" , Tvdb400_TypeToStr( outtype ) , m_Label , m_Name );
  }
  else if ( GetInputPin( &intype ) )
    result.Format( "(%s) %s" , Tvdb400_TypeToStr( intype ) , m_Name );
  else
  {
    ASSERT( FALSE );
    return false;
  }
  return true;
}

int CConnector::GetComplementary( FXPtrArray& Connectors )
{
  m_Wires.LockPtr();
  int count = (int)m_Wires.GetSize() , i = 0;
  CWire* pWire = NULL;
  while ( (i < count) && (pWire = (CWire*) m_Wires.GetAt( i )) )
  {
    if ( pWire->m_pConnector1 == this )
      Connectors.Add( pWire->m_pConnector2 );
    else
      Connectors.Add( pWire->m_pConnector1 );
    i++;
  }
  m_Wires.UnlockPtr();
  return count;
}
int CConnector::LogIfNecessary(const CDataFrame * pDataFrame)
{
  if ( m_iLogging )
  {
    ((CDataFrame*)pDataFrame)->AddRef();
    return m_PinLogger.AddMsg( (void*)pDataFrame , &m_Name );
  }
  return 0;
}

void CConnector::ProcessPinLogMessage(void * pMsg, FXString& outp)
{
  CDataFrame * pDataFrame = (CDataFrame*)pMsg;
  pDataFrame->ToLogString(outp);
  pDataFrame->Release();
}

///////////////////////////////////////////////////////////////////////////////////
// CInputConnector
///////////////////////////////////////////////////////////////////////////////////

CInputConnector::CInputConnector( datatype dtype , FN_SENDINPUTDATA fnSendInputData , void* lpHostGadget ) :
  CConnector( dtype ) ,
  m_evClose( NULL ) ,
  m_evHasData( NULL ) ,
  m_pFnSendInputData( fnSendInputData ) ,
  m_pHostGadget( lpHostGadget )
{
  if ( m_DataType == transparent )
    m_DataType = nulltype;
  if ( !m_pFnSendInputData )
  {
    m_evClose = ::CreateEvent( NULL , TRUE , FALSE , NULL );
    m_evHasData = ::CreateEvent( NULL , TRUE , FALSE , NULL );
  }
}

CInputConnector::~CInputConnector()
{
  Disconnect();
  Close();
  while ( m_FramesQueue.ItemsInQueue() )
  {
    CDataFrame* mdf = NULL;
    m_FramesQueue.GetQueueObject( mdf );
    if ( mdf )
      mdf->Release( mdf );
  }
  if ( m_evHasData )
    ::CloseHandle( m_evHasData );
  if ( m_evClose )
    ::CloseHandle( m_evClose );
}

bool CInputConnector::IsDirectConnector()
{
  return (m_pFnSendInputData != NULL);
}

bool CInputConnector::Send( CDataFrame* pDataFrame )
{
  LogIfNecessary(pDataFrame);

  if ( m_pFnSendInputData )
  {
    m_pFnSendInputData( pDataFrame , m_pHostGadget , this );
    m_FramesPassed++;
    return true;
  }
  else if ( pDataFrame->IsRegistered() )
  {
    int iLoopCntr = 0 ;
    while ( !m_FramesQueue.PutQueueObject( pDataFrame ) && iLoopCntr++ < 300)
    {
      Sleep( 10 );
    }
    SetDataAvailable( true );
    m_FramesPassed++;
    return true;
  }
  else if ( m_FramesQueue.PutQueueObject( pDataFrame ) )
  {
    SetDataAvailable( true );
    m_FramesPassed++;
    return true;
  }
  return false;
}

bool CInputConnector::Get( CDataFrame*& pDataFrame )
{
  FXAutolock al( m_PinLock );
  const HANDLE Events[] = {m_evClose, m_evHasData};
  const DWORD cEvents = sizeof( Events ) / sizeof( HANDLE );
  DWORD waitRet = ::WaitForMultipleObjects( cEvents , Events , FALSE , 10 );
  if ( waitRet != WAIT_OBJECT_0 + 1 ) // if no data
  {
    return false;
  }
  if ( m_FramesQueue.GetQueueObject( pDataFrame ) )
  {
    if ( !m_FramesQueue.ItemsInQueue() )
    {
      SetDataAvailable( false );
      m_FramesPassed++;
    }
    return true;
  }
  return false;
}

void CInputConnector::Close()
{
  if ( !IsDirectConnector() )
  {
    SetDataAvailable( false );
    ::SetEvent( m_evClose );
  }
}

void CInputConnector::SetDataAvailable( bool bSet )
{
  if ( bSet )
    ::SetEvent( m_evHasData );
  else
    ::ResetEvent( m_evHasData );
}

///////////////////////////////////////////////////////////////////////////////////
// CBufferedInputConnector
///////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI ConnectorWatchFunc( void* pThis )
{
  CBufferedInputConnector* ctp = (CBufferedInputConnector*) pThis;
  return (ctp->Watch());
}

CBufferedInputConnector::CBufferedInputConnector( datatype dtype , FN_SENDINPUTDATA fnSendInputData , void* lpHostGadget ) :
  CInputConnector( dtype , fnSendInputData , lpHostGadget ) ,
  m_InputQueue( 20 )
{
  m_WrkThread = ::CreateThread( NULL , 0 , ConnectorWatchFunc , this , CREATE_SUSPENDED , NULL );
  ResumeThread( m_WrkThread );
}

CBufferedInputConnector::~CBufferedInputConnector()
{
  ::SetEvent( m_evClose );
  if ( m_WrkThread )
  {
    ::WaitForSingleObject( m_WrkThread , INFINITE ); 
    ::FxReleaseHandle( m_WrkThread ); 
    m_WrkThread = NULL;
  }
  ClearQueue();
}

DWORD CBufferedInputConnector::Watch()
{
  const HANDLE Events[] = {m_evHasData, m_evClose};
  const DWORD cEvents = sizeof( Events ) / sizeof( HANDLE );

  while ( (::WaitForMultipleObjects( cEvents , Events , FALSE , INFINITE ) == WAIT_OBJECT_0) &&
    (::WaitForSingleObject( m_evClose , 0 ) != WAIT_OBJECT_0) )
  {
    CDataFrame* pDataFrame = NULL;
    if ( Get( pDataFrame ) )
    {
    #ifdef _DEBUG
      //TRACE("+++ BIC %s got 0x%x with ID %d refCnt= %d\n",((FXWorker*)m_pHostGadget)->GetName(), pDataFrame, pDataFrame->GetId(),pDataFrame->AddRef(0));
    #endif
      LogIfNecessary(pDataFrame);

      if ( !m_InputQueue.PutQueueObject( pDataFrame ) )
      {
        pDataFrame->Release();
      }
      ASSERT( m_pHostGadget ); /// we must remain reference on base gadget to resume threads
      ((FXWorker*) m_pHostGadget)->Resume();
    }
  }
  return 0;
}

void CBufferedInputConnector::ClearQueue()
{
  CDataFrame* pDataFrame;
  while ( m_InputQueue.ItemsInQueue() )
  {
    if ( m_InputQueue.GetQueueObject( pDataFrame ) ) pDataFrame->Release();
  }
}


///////////////////////////////////////////////////////////////////////////////////
// COutputConnector
///////////////////////////////////////////////////////////////////////////////////

COutputConnector::COutputConnector( datatype dtype ) :
  CConnector( dtype ) ,
  m_fnOCB( NULL ) ,
  m_pExtClient( NULL ) ,
  m_ExtID( "" ) ,
  m_EOSSent( FALSE )
{}

COutputConnector::~COutputConnector()
{
  FXAutolock lck( m_Lock );
  Disconnect();
}

void COutputConnector::SetCallback( LPCTSTR id , OutputCallback fn , void* pClient )
{
  FXAutolock lck( m_Lock );
  m_ExtID = id;
  m_fnOCB = fn;
  m_pExtClient = pClient;
}

bool COutputConnector::Put( CDataFrame* pDataFrame )
{
  LogIfNecessary(pDataFrame);
  FXAutolock lck( m_Lock );
  m_Wires.LockPtr();
  bool EOS = Tvdb400_IsEOS( pDataFrame );
  if ( (!m_EOSSent) || (!EOS) )
  {
    int cOutputs = (int)m_Wires.GetSize() + ((m_fnOCB != NULL) ? 1 : 0);
    if ( !cOutputs )
    {
      pDataFrame->Release();
      m_Wires.UnlockPtr();
      m_FramesPassed++;
      return true;
    }
    if ( !m_Label.IsEmpty() )
    {
      if ( m_Label[ 0 ] == '+' )
      {
        FXString tmpS;
        tmpS = FXString( pDataFrame->GetLabel() ) + '.' + &(((LPCSTR) m_Label)[ 1 ]);
        pDataFrame->SetLabel( tmpS );
      }
      else
        pDataFrame->SetLabel( m_Label );
    }
    if ( cOutputs > 1 )
      pDataFrame->AddRef( cOutputs - 1 );
    if ( m_fnOCB )
    {
      if ( !m_fnOCB( pDataFrame , m_ExtID , m_pExtClient ) )
      {
        pDataFrame->Release();
      }
      cOutputs--;
    }
    while ( cOutputs-- )
    {
      if ( !((CWire*) m_Wires.GetAt( cOutputs ))->Push( pDataFrame , this , m_DataType == transparent ) )
      {
        m_FramesSkipped++;
        pDataFrame->Release();
      }
    }
    m_EOSSent = EOS;
  }
  else
    pDataFrame->Release( pDataFrame );
  m_Wires.UnlockPtr();
  m_FramesPassed++;
  return true;
}


CDuplexConnector::CDuplexConnector( CGadget* Host , datatype outtype , datatype intype ) :
  COutputConnector( outtype ) ,
  m_pHostGadget( Host ) ,
  m_InputType( intype )
{
  if ( m_InputType == transparent )
    m_InputType = nulltype;
}

CDuplexConnector::~CDuplexConnector()
{}

bool CDuplexConnector::Send( CDataFrame* pDataFrame )
{
  if ( pDataFrame->IsRegistered() )
    return true ;
  if ( !Tvdb400_TypesCompatible( pDataFrame->GetDataType() , m_InputType ) )
    return false;
  LogIfNecessary(pDataFrame);
  m_pHostGadget->AsyncTransaction( this , pDataFrame );
  m_FramesPassed++;
  return true;
}


//
// CWire
//

CWire::CWire( CConnector* pConnector1 , CConnector* pConnector2 ) :
  m_pConnector1( pConnector1 ) ,
  m_pConnector2( pConnector2 )
{
  //TRACE(" -- Creating wire 0x%x (0x%x, 0x%x)\n", this, pConnector1, pConnector2);
}

CWire::~CWire()
{
  //TRACE( " -- Deleting wire 0x%x (0x%x, 0x%x)\n" , this , m_pConnector1 , m_pConnector2 );
  CDataFrame* pDataFrame = CDataFrame::Create( transparent );
  Tvdb400_SetEOS( pDataFrame );
  //TRACE( " !! EOS Dataframe 0x%x is created!\n" , pDataFrame );
  CConnector* pin1 = m_pConnector1->GetInputPin();
  CConnector* pin2 = m_pConnector2->GetInputPin();
  if ( !pin1 && !pin2 )
  {
    pDataFrame->Release();
  }
  else
  {
    if ( pin1 && pin2 )
      pDataFrame->AddRef();
    do
    {
      if ( pin1 && Push( pDataFrame , m_pConnector2 , FALSE ) )
        pin1 = NULL;
      if ( pin2 && Push( pDataFrame , m_pConnector1 , FALSE ) )
        pin2 = NULL;
    } while ( (pin1 || pin2) && !SleepEx( 1 , FALSE ) );
  }
  //TRACE( " -- Deleted wire 0x%x (0x%x, 0x%x)\n" , this , m_pConnector1 , m_pConnector2 );
}

bool CWire::Push( CDataFrame* pDataFrame , CConnector* pSender , BOOL bTypeCheck )
{
  // Determine recipient
  ASSERT( (pSender == m_pConnector1) || (pSender == m_pConnector2) );
  datatype intype;
  CConnector* pInput = ((pSender == m_pConnector1) ? m_pConnector2->GetInputPin( &intype ) : m_pConnector1->GetInputPin( &intype ));
  if ( !pInput )
    return false;

  // Make type compatibility check
  if ( bTypeCheck && !Tvdb400_TypesCompatible( pDataFrame->GetDataType() , intype ) )
    return false;

  // Try to send data
  return pInput->Send( pDataFrame );
}

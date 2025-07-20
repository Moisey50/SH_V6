#include "StdAfx.h"
#include "fxfc\fxfc.h"

#define THIS_MODULENAME "FXWorker"

const DWORD FXWorker::m_ProcessorMask = FxAffinityGetProcessorMask() ;


class CThreads : public FXArray<WORKERTHREAD , WORKERTHREAD&>
{};

///////////////////////////////////
// Worker

void FXWorker::_workerFunc( void* pwrkthread )
{
  WORKERTHREAD* wkthrd = ((WORKERTHREAD*) pwrkthread);
  wkthrd->pWorker->Work( wkthrd );
};

FXWorker::FXWorker() :
  m_evExit( NULL ) ,
  m_evResume( NULL ) ,
  m_dwTicksIdle( DEFAULT_TICKS_IDLE ) ,
  m_CPUUsage( 0 ) ,
  m_LastProcTime( 0 ) ,
  m_LastCheck( GetHRTickCount() ) ,
  m_MultyCoreAllowed( false ) ,
  m_ThreadsNmb( 0 ) ,
  m_Affinity( m_ProcessorMask ) ,
  m_Priority( THREAD_PRIORITY_NORMAL )
{
  ::ZeroMemory( &m_WIInfo , sizeof( m_WIInfo ) );
  m_Threads = new CThreads;
}

FXWorker::~FXWorker()
{
  Destroy();
  delete m_Threads;
}

double FXWorker::GetCPUUsage()
{
  double period = GetHRTickCount() - m_LastCheck;
  if ( period == 0 ) return 0.0;
  double retV = (double) m_CPUUsage / period;
  m_CPUUsage = 0;
  m_LastCheck = GetHRTickCount();
  return retV;
}

DWORD FXWorker::SetTicksIdle( DWORD dwTicksIdle )
{
  m_dwTicksIdle = dwTicksIdle;
  return m_dwTicksIdle;
}

bool FXWorker::SetAffinity( DWORD Affinity )
{
  m_Affinity = Affinity;
  if ( m_Threads->GetSize() == 1 )
  {
    SetCoreThreadAffinity( m_Affinity ) ;
  }
  return true;
}

DWORD FXWorker::GetAffinity()
{
  if ( m_Threads->GetSize() == 1 )
  {
    m_Affinity = GetCoreThreadAffinity( 0 ) ;
  }
  return m_Affinity;
}

bool FXWorker::SetPriority( DWORD Priority )
{
  m_Priority = Priority;
  SetCoreThreadPriority( m_Priority ) ;
  return true;
}

DWORD FXWorker::GetPriority()
{
  m_Priority = GetCoreThreadPriority() ;
  return m_Priority;
}

bool FXWorker::SetCoreThreadAffinity( DWORD Affinity , int nCore , bool Lock )
{
  if ( Lock )
    m_ThreadsLock.Lock();
  if ( (nCore < 0) || (nCore >= (int) m_Threads->GetSize()) )
  {
    if ( Lock )
      m_ThreadsLock.Unlock();
    return false;
  }
  WORKERTHREAD wth = m_Threads->GetAt( nCore );
  if ( wth.hThread )
  {
    bool Suspended = (::WaitForSingleObject( m_evResume , 0 ) != WAIT_OBJECT_0);
    if ( !Suspended )
      Suspend();
    SetThreadAffinityMask( wth.hThread , Affinity );

    wth.dwAffinity = Affinity;
    Sleep( 100 );
    if ( !Suspended )
      Resume();
  }
  if ( Lock )
    m_ThreadsLock.Unlock();
  return true;
}

DWORD FXWorker::GetCoreThreadAffinity( int nCore )
{
  FXAutolock lock( m_ThreadsLock );
  if ( (nCore < 0) || (nCore >= (int) m_Threads->GetSize()) )
    return 0;
  WORKERTHREAD wth = m_Threads->GetAt( nCore );
  return wth.dwAffinity;
}

bool FXWorker::SetCoreThreadPriority( DWORD Priority , int nCore )
{
  if ( (nCore < 0) || (nCore >= (int) m_Threads->GetSize()) )
    return false;

  WORKERTHREAD wth = m_Threads->GetAt( nCore );
  if ( wth.hThread )
  {
    SetThreadPriority( wth.hThread , Priority );
    wth.dwPriority = Priority;
  }
  return true;
}

DWORD FXWorker::GetCoreThreadPriority( int nCore )
{
  FXAutolock lock( m_ThreadsLock );
  if ( (nCore < 0) || (nCore >= (int) m_Threads->GetSize()) )
    return 0;
  WORKERTHREAD wth = m_Threads->GetAt( nCore );
  wth.dwPriority = GetThreadPriority( wth.hThread ) ;
  return wth.dwPriority;
}

void FXWorker::SetThreadName( LPCTSTR name )
{
  FXAutolock al( m_ThreadsLock );
  m_Name = name;
  SetThreadsNames();
}

void FXWorker::SetThreadsNames()
{
  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.dwFlags = 0;
  for ( int i = 0; i < (int) m_Threads->GetSize(); i++ )
  {
    WORKERTHREAD wth = m_Threads->GetAt( i );
    char threadname[ MAX_PATH ];
    sprintf_s( threadname , MAX_PATH , "%s(%02d)" , (LPCTSTR) m_Name , i );
    info.dwThreadID = wth.dwThreadId;
    info.szName = threadname;
    DoSetThreadName( info );
  }
}

DWORD FXWorker::GetThreadId( int nCore )
{
  FXAutolock lock( m_ThreadsLock );
  if ( (nCore < 0) || (nCore >= (int) m_Threads->GetSize()) )
    return 0;
  WORKERTHREAD wth = m_Threads->GetAt( nCore );
  return wth.dwUID;
}

int FXWorker::GetCoresNumber()
{
  return m_ThreadsNmb;
}

BOOL FXWorker::SetCoresNumber( int nCores )
{
  if ( nCores < 1 ) return FALSE;
  FXAutolock lock( m_ThreadsLock );
  while ( nCores < (int) m_Threads->GetSize() )
  {
    ::SetEvent( m_Threads->GetAt( m_Threads->GetSize() - 1 ).evExit );
    ::WaitForSingleObject( m_Threads->GetAt( m_Threads->GetSize() - 1 ).hThread , INFINITE );
    CloseHandle( m_Threads->GetAt( m_Threads->GetSize() - 1 ).evExit );
    FxReleaseHandle( m_Threads->GetAt( m_Threads->GetSize() - 1 ).hThread );
    m_Threads->RemoveAt( m_Threads->GetSize() - 1 );
    m_ThreadsNmb--;
    return TRUE;
  }

  if ( nCores == m_Threads->GetSize() ) return TRUE;

  if ( (nCores > 1) && (m_MultyCoreAllowed == false) ) return FALSE;

  int nMoreCores = nCores - (int)m_Threads->GetSize();
  LPSECURITY_ATTRIBUTES lpSA = (m_WIInfo.bHasSA) ? &m_WIInfo.SA : NULL;
  while ( nMoreCores-- > 0 )
  {
    { // hide WORKERTHREAD wth;
      WORKERTHREAD wth;

      wth.hThread = NULL;
      wth.dwAffinity = FxAffinityGetProcessorMask();
      wth.dwPriority = THREAD_PRIORITY_NORMAL ;
      wth.dwUID = m_ThreadsNmb + 1;
      wth.dwThreadId = 0; // will be initialized in thread creation
      wth.pWorker = this;
      wth.evExit = ::CreateEvent( NULL , TRUE , FALSE , NULL );
      wth.evReady = ::CreateEvent( NULL , TRUE , FALSE , NULL );
      m_Threads->Add( wth );
    }
//     TRACE( "~~~~~~~~~~~~~~~~~~~~~~~~~~~ m_Threads->GetSize()=%d, "
//       "m_cThreadsUID=%d \n" , m_Threads->GetSize() , m_ThreadsNmb );
    if ( m_WIInfo.hProcess )
      (*m_Threads)[ m_ThreadsNmb ].hThread = ::CreateRemoteThread( m_WIInfo.hProcess , lpSA , m_WIInfo.dwStack , (LPTHREAD_START_ROUTINE) _workerFunc , &((*m_Threads)[ m_ThreadsNmb ]) , 0 , &(*m_Threads)[ m_ThreadsNmb ].dwThreadId );
    else
      (*m_Threads)[ m_ThreadsNmb ].hThread = ::CreateThread( lpSA , m_WIInfo.dwStack , (LPTHREAD_START_ROUTINE) _workerFunc , &((*m_Threads)[ m_ThreadsNmb ]) , 0 , &(*m_Threads)[ m_ThreadsNmb ].dwThreadId );
    ::WaitForSingleObject( (*m_Threads)[ m_ThreadsNmb ].evReady , INFINITE );
    if ( !(*m_Threads)[ m_ThreadsNmb ].hThread )
    {
      m_Threads->RemoveAt( m_ThreadsNmb );
      SENDERR_2( "Fail in creation of the thread #%d in '%s'" , m_ThreadsNmb + 1 , m_Name );
      return FALSE;
    }
    m_ThreadsNmb++;
    //TRACE( "++++++++++++++++++++ Added thread 0x%x (id = %d)\n" , (*m_Threads)[ m_ThreadsNmb - 1 ].hThread , (*m_Threads)[ m_ThreadsNmb - 1 ].dwUID );
  }
  if ( m_Name.GetLength() != 0 )
  {
    SetThreadsNames();
    for ( DWORD i = 0; i < m_ThreadsNmb; i++ )
    {
      SetCoreThreadAffinity( m_Affinity , i , false );
      SetCoreThreadPriority( m_Priority , i ) ;
    }
  }
  return TRUE;
}

BOOL FXWorker::Create( HANDLE hProcess , 
  LPSECURITY_ATTRIBUTES lpSA , DWORD dwStack , 
  DWORD dwFlags , LPDWORD lpThreadID , int nCores )
{
  if ( m_Threads->GetSize() )
    return FALSE;
  ASSERT( !m_evExit );
  ASSERT( !m_evResume );

  m_evExit = ::CreateEvent( NULL , TRUE , FALSE , NULL );
  if ( !m_evExit )
    return FALSE;
  m_evResume = ::CreateEvent( NULL , TRUE , FALSE , NULL );
  if ( !m_evResume )
  {
    FxReleaseHandle( m_evExit );
    return FALSE;
  }

  m_WIInfo.hProcess = hProcess;
  if ( lpSA )
  {
    m_WIInfo.bHasSA = TRUE;
    memcpy( &m_WIInfo.SA , lpSA , sizeof( m_WIInfo.SA ) );
  }
  else
    m_WIInfo.bHasSA = FALSE;
  m_WIInfo.dwStack = dwStack;

  while ( nCores-- > 0 )
  {
    if ( !SetCoresNumber( m_ThreadsNmb + 1 ) )
      break;
  }

  if ( (dwFlags & CREATE_SUSPENDED) == 0 )
    Resume();

  return TRUE;
}

BOOL FXWorker::Suspend()
{
  return ::ResetEvent( m_evResume );
}

BOOL FXWorker::Resume()
{
  return ::SetEvent( m_evResume );
}

void FXWorker::Destroy()
{

  FXAutolock lock( m_ThreadsLock );
  if ( m_Threads->GetSize() )
  {
    ASSERT( m_evExit );
    ASSERT( m_evResume );

    ::SetEvent( m_evExit );
    while ( m_Threads->GetSize() )
    {
      WORKERTHREAD wth = m_Threads->GetAt( 0 );
      while ( ::WaitForSingleObject( wth.hThread , 100 ) != WAIT_OBJECT_0 )
      {
        MSG msg;
        if ( ::PeekMessage( &msg , NULL , 0 , 0 , PM_REMOVE ) )
        {
          ::TranslateMessage( &msg );
          ::DispatchMessage( &msg );
        }
      }
      CloseHandle( wth.evExit );
      CloseHandle( wth.evReady );
      FxReleaseHandle( wth.hThread );
      m_Threads->RemoveAt( 0 );
      m_ThreadsNmb--;
    }
  }
  FxReleaseHandle( m_evResume ); // it's what FxReleaseHandle actually does: CloseHandle(m_evResume); m_evResume=NULL;
  FxReleaseHandle( m_evExit );

  if ( !m_Name.IsEmpty() )
    TRACE( "\n.................Worker %s is destroyed  " , (LPCTSTR)m_Name ) ;
}

int FXWorker::DoJob()
{
  return WR_EXIT;
}

void FXWorker::OnThreadStart()
{}

void FXWorker::OnThreadEnd()
{}

void FXWorker::Work( WORKERTHREAD* pwth )
{
  ::CoInitialize( NULL );
  WORKERTHREAD wth = *pwth; // Pointer to data can be changed, so copy whole data
  ::SetEvent( wth.evReady ); //Inform starter that data copied
  OnThreadStart();

  //TRACE( " >>>>>>>>>>>>>>>>>>>>> %s:FXWorker::Work(%d)\n" , m_Name , wth.dwUID );

  HANDLE pEvents[] = {m_evExit, wth.evExit, m_evResume};
  DWORD  cEvents = sizeof( pEvents ) / sizeof( HANDLE );

  HANDLE pExitEvents[] = {m_evExit, wth.evExit};
  DWORD  cExitEvents = sizeof( pExitEvents ) / sizeof( HANDLE );

  DWORD retWait = ::WaitForMultipleObjects( cExitEvents , pExitEvents , FALSE , 0 );
  while ( (retWait != WAIT_OBJECT_0) && (retWait != WAIT_OBJECT_0 + 1) )
  {
    if ( ::WaitForSingleObject( m_evResume , 0 ) == WAIT_OBJECT_0 )
    {
      switch ( DoJob() )
      {
        case WR_EXIT:
          goto WorkExit;
        case WR_CONTINUE:
        {
          retWait = ::WaitForMultipleObjects( cExitEvents , pExitEvents , FALSE , m_dwTicksIdle );
          if ( (retWait == WAIT_OBJECT_0) || (retWait == WAIT_OBJECT_0 + 1) )
            goto WorkExit;
        }
        break;
        default:
          ASSERT( FALSE );
      }
    }
    else // in pause state, wait while exit or resume
    {
      retWait = ::WaitForMultipleObjects( cEvents , pEvents , FALSE , INFINITE );
      if ( (retWait == WAIT_OBJECT_0) || (retWait == WAIT_OBJECT_0 + 1) )
        goto WorkExit;
    }
    retWait = ::WaitForMultipleObjects( cExitEvents , pExitEvents , FALSE , 0 );
  }
WorkExit:
  OnThreadEnd();
  ::CoUninitialize();
  //TRACE( " <<<<<<<<<<<<<<<<<<<<<<<<<<<< WORKER THREAD %s:(Main exit) %d EXITS!\n" , m_Name , wth.dwUID );
}

// Worker
///////////////////////////////////

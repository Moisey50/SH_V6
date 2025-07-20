#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "BaseExecution.cpp"
#include <gadgets\gadbase.h>


//
// CExecutionStatus
//   CMasterExecutionStatus
//

CExecutionStatus::CExecutionStatus( CExecutionStatus* Status , LPCTSTR Name ) :
  m_pStatus( NULL )
{
  if ( !Status )
  {
    m_pStatus = new EXECUTIONSTATUS;
    memset( m_pStatus , 0 , sizeof( EXECUTIONSTATUS ) );
    if ( !m_pStatus ) return;
    AddRef();
    if ( Name != NULL )
    {
      m_pStatus->m_evStart = ::CreateEvent( NULL , TRUE , FALSE , FXString( Name ) + _T( "Start" ) );
      m_pStatus->m_evPause = ::CreateEvent( NULL , TRUE , FALSE , FXString( Name ) + _T( "Pause" ) );
      m_pStatus->m_evStepFwd = ::CreateEvent( NULL , TRUE , FALSE , FXString( Name ) + _T( "StepFwd" ) );
      m_pStatus->m_evStop = ::CreateEvent( NULL , TRUE , FALSE , FXString( Name ) + _T( "Stop" ) );
      m_Name = Name;
    }
    else
    {
      m_pStatus->m_evStart = ::CreateEvent( NULL , TRUE , FALSE , NULL );
      m_pStatus->m_evPause = ::CreateEvent( NULL , TRUE , FALSE , NULL );
      m_pStatus->m_evStepFwd = ::CreateEvent( NULL , TRUE , FALSE , NULL );
      m_pStatus->m_evStop = ::CreateEvent( NULL , TRUE , FALSE , NULL );
      m_Name = _T( "Noname" );
    }
    TRACE( "+++ Create CExecutionStatus with name \"%s\"\n" , m_Name );
  }
  else
  {
    m_pStatus = Status->m_pStatus;
    AddRef();
  }
}

CExecutionStatus::~CExecutionStatus()
{
  if ( m_pStatus )
  {
    LONG count = InterlockedDecrement( &m_pStatus->m_cRefs );
    if ( count == 0 )
    {
      if ( m_pStatus->m_evStart )
      {
        ::CloseHandle( m_pStatus->m_evStart );
        m_pStatus->m_evStart = NULL;
      }
      if ( m_pStatus->m_evPause )
      {
        ::CloseHandle( m_pStatus->m_evPause );
        m_pStatus->m_evPause = NULL;
      }
      if ( m_pStatus->m_evStepFwd )
      {
        ::CloseHandle( m_pStatus->m_evStepFwd );
        m_pStatus->m_evStepFwd = NULL;
      }
      if ( m_pStatus->m_evStop )
      {
        ::CloseHandle( m_pStatus->m_evStop );
        m_pStatus->m_evStop = NULL;
      }
      delete m_pStatus;
      m_pStatus = NULL;
      TRACE( "+++ Destoyed CExecutionStatus with name \"%s\"\n" , m_Name );
    }
  }
}

CExecutionStatus* CExecutionStatus::Create( CExecutionStatus* Status , LPCTSTR Name )
{
  return new CExecutionStatus( Status , Name );
}

LONG CExecutionStatus::AddRef()
{
  return     InterlockedIncrement( &m_pStatus->m_cRefs );
}

LONG CExecutionStatus::Release()
{
  LONG count = 0;
  if ( m_pStatus )
    count = m_pStatus->m_cRefs - 1;
  delete this;
  return count;
}

UINT CExecutionStatus::GetStatus()
{
  if ( (!m_pStatus) || (!m_pStatus->m_evPause) || (!m_pStatus->m_evStepFwd) || (!m_pStatus->m_evStart) || (!m_pStatus->m_evStop) )
    return EXIT;

  DWORD pauseStatus = ::WaitForSingleObject( m_pStatus->m_evPause , 0 );
  if ( pauseStatus == WAIT_OBJECT_0 )
    return PAUSE;
  else if ( pauseStatus != WAIT_TIMEOUT )
  {
    TRACE( "+++ Requsted exit with pauseStatus =%d\n" , pauseStatus );
    return EXIT;
  }
  // Moisey: I did put 1 ms for CPU load avoiding in STOP state
  DWORD startStatus = ::WaitForSingleObject( m_pStatus->m_evStart , 0 /*1*/ );
  if ( startStatus == WAIT_OBJECT_0 )
    return RUN;
  else if ( startStatus != WAIT_TIMEOUT )
  {
    TRACE( "+++ Requsted exit with startStatus =%d\n" , startStatus );
    return EXIT;
  }
  return STOP;
}

BOOL CExecutionStatus::IsForwardTriggerOn()
{
  if ( (!m_pStatus) || (!m_pStatus->m_evStepFwd) ) return FALSE;
  return (::WaitForSingleObject( m_pStatus->m_evStepFwd , 0 ) == WAIT_OBJECT_0);
}

void CExecutionStatus::Copy( LPEXECUTIONSTATUS Status )
{
  m_pStatus = Status;
}

void CExecutionStatus::Pause()
{
  ASSERT( m_pStatus->m_evPause && m_pStatus->m_evStart );
  ::SetEvent( m_pStatus->m_evPause );
  ::ResetEvent( m_pStatus->m_evStart );
  ::ResetEvent( m_pStatus->m_evStop );
}

void CExecutionStatus::Start()
{
  ASSERT( m_pStatus->m_evPause && m_pStatus->m_evStart );
  ::SetEvent( m_pStatus->m_evStart );
  ::ResetEvent( m_pStatus->m_evPause );
  ::ResetEvent( m_pStatus->m_evStop );
}

void CExecutionStatus::StepFwd()
{
  ASSERT( m_pStatus->m_evStepFwd );
  ::PulseEvent( m_pStatus->m_evStepFwd );
}

void CExecutionStatus::Stop()
{
  ASSERT( m_pStatus->m_evPause && m_pStatus->m_evStart );
  ::ResetEvent( m_pStatus->m_evStart );
  ::ResetEvent( m_pStatus->m_evPause );
  ::SetEvent( m_pStatus->m_evStop );
}

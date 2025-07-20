#include "stdafx.h"
#include <fxfc\fxfc.h>

// __forceinline LONG GetLockCount(LONG* LockCount)
// {
//     LONG res;
//     InterlockedExchange(&res,*LockCount);
//     return res;
// }

FXLockObject::FXLockObject()
{
#ifdef _DEBUG
  m_LockCount = 0;
  m_WhoLock[0] = 0 ;
#endif
  ::InitializeCriticalSection( &m_CritSect );
}

FXLockObject::~FXLockObject()
{
  Lock();
  ::DeleteCriticalSection( &m_CritSect );
}

BOOL	FXLockObject::Lock( DWORD dwTimeOut , LPCTSTR WhoLock )
{
#ifdef _DEBUG
  DWORD dwId = GetCurrentThreadId() ;
  int iLockCnt = m_LockCount ;
  if ( iLockCnt && !m_dwId)
  {
    ASSERT( !(dwId == m_dwId) ) ; // multiple lock from the same thread
  }
#endif
  if ( dwTimeOut == INFINITE )
  {
    ::EnterCriticalSection( &m_CritSect );
#ifdef _DEBUG
    //      if ( GetLockCount( &m_LockCount ) != 0 )
    if ( GetLockCount() != 0 )
    {
      TRACE( "\n    FXLockObject::Lock %d Error Old=%s(h%0X) New=%s(h%0X)" ,
        GetLockCount() ,
        (m_WhoLock[0] == 0) ? "Unknown" : m_WhoLock , m_dwId ,
        (WhoLock && *WhoLock) ? WhoLock : "Unknown" , dwId ) ;
    }
    if ( WhoLock )
    {
      int i = 0 ;
      while ( i < 29 && *WhoLock )
        m_WhoLock[ i++ ] = *(WhoLock++) ;

      m_WhoLock[ i ] = 0 ;
    }
    else
      m_WhoLock[ 0 ] = 0 ;
    m_dwId = dwId ;
    InterlockedIncrement( &m_LockCount );
#endif
    return TRUE;
  }
  else
  {
    BOOL res = FALSE;
    do
    {
      res = ::TryEnterCriticalSection( &m_CritSect );
      if ( res )
        break ;
      Sleep( 10 ) ;
      dwTimeOut -= 10 ;
    } while ( (!res) && ((int) dwTimeOut >= 0) ) ;
    if ( res )
    {
#ifdef _DEBUG
      //      if ( GetLockCount( &m_LockCount ) != 0 )
      if ( GetLockCount() != 0 )
      {
        TRACE( "\n    FXLockObject::Lock Error Old=%s(h%0X) New=%s(h%0X) Tout=%d" ,
          (m_WhoLock[ 0 ] == 0) ? "Unknown" : (LPCTSTR) m_WhoLock , m_dwId ,
          (*WhoLock) ? WhoLock : "Unknown" , dwId , dwTimeOut ) ;
      }
      //ASSERT(GetLockCount(&m_LockCount)==0);
      InterlockedIncrement( &m_LockCount );
      if ( WhoLock )
      {
        int i = 0 ;
        while ( i < 29 && *WhoLock )
        {
          m_WhoLock[ i++ ] = *(WhoLock++) ;
        }
        m_WhoLock[ i ] = 0 ;
      }
      else
        m_WhoLock[ 0 ] = 0 ;
      m_dwId = dwId ;
#endif
    }
    return res;
  }
}

BOOL	FXLockObject::LockAndProcMsgs(
  DWORD dwTimeOut , LPCTSTR WhoLock )
{
  double ts = GetHRTickCount();
  while ( (!Lock( 10 , WhoLock )) && (dwTimeOut > GetHRTickCount() - ts) )
  {
    MSG msg;
    if ( ::PeekMessage( &msg , NULL , 0 , 0 , PM_REMOVE ) )
    {
      ::TranslateMessage( &msg );
      ::DispatchMessage( &msg );
    }
  }
  return (dwTimeOut > GetHRTickCount() - ts);
}

void	FXLockObject::Unlock()
{
#ifdef _DEBUG
  DWORD dwId = GetCurrentThreadId() ;

  //  ASSERT( GetLockCount( &m_LockCount ) == 1 );
  ASSERT( GetLockCount() == 1 );
  InterlockedDecrement( &m_LockCount );
  m_WhoLock[0] = 0 ;
  m_dwId = NULL ;
#endif
  ::LeaveCriticalSection( &m_CritSect );
}

BOOL FXLockObject::IsLockedByThisThread()
{
#ifdef _DEBUG
  if ( m_LockCount )
    return ( m_dwId == GetCurrentThreadId() ) ;
#endif
  return FALSE ;
}
FXAutolock::FXAutolock( FXLockObject& lock , LPCTSTR WhoLock )
{
  m_pLock = &lock;
  lock.Lock( INFINITE , WhoLock );
}

FXAutolock::~FXAutolock()
{
  ASSERT( m_pLock );
  m_pLock->Unlock();
  m_pLock = NULL;
}

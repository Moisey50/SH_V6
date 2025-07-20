
#pragma once

class CObservedMutex
{
  HANDLE m_hMutex ;
  DWORD m_dwRes ;
  int   m_iLockCnt ;
  DWORD m_dwOwnerId ;
  bool  m_bTraceEnable = true ;
#ifdef _DEBUG
  TCHAR m_Owner[ 256 ] ;
#endif
public:
  CObservedMutex( LPCTSTR pName = NULL )
  {
    if ( !pName )
      m_hMutex = CreateMutex( NULL , FALSE , NULL ) ;
    else
    {
      m_hMutex = OpenMutex( NULL , FALSE , pName ) ;
      if ( !m_hMutex )
        m_hMutex = CreateMutex( NULL , FALSE , pName ) ;
    }
  #ifdef _DEBUG
    m_Owner[ 0 ] = 0;
  #endif
    m_dwOwnerId = NULL ;
    m_dwRes = 0 ;
    m_iLockCnt = 0 ;
    if ( !m_hMutex )
    {
      TRACE( "\nCObservedMutex - mutex %s is not created (%u)" , pName , GetLastError() ) ;
    }
    else
    {
      DWORD Res = WaitForSingleObject(m_hMutex , 0);
      if (Res == WAIT_OBJECT_0)
      {
        ReleaseMutex(m_hMutex);
      }
      else
      {
        FXString ErrMsg = FxLastErr2Mes();
        ASSERT(0);
      }
    }
  }
  ~CObservedMutex()
  {
    if ( m_hMutex )
    {
      CloseHandle( m_hMutex ) ;
      m_hMutex = NULL ;
    }
  }

  bool Lock( DWORD dwTimeOut 
#ifdef _DEBUG
    , LPCTSTR WhoIsLocked = NULL 
#endif
  )
  {
    if ( m_hMutex )
    {
      m_dwRes = WaitForSingleObject( m_hMutex , dwTimeOut ) ;
      if ( m_dwRes == WAIT_OBJECT_0 )
      {
        DWORD dwCurrentThreadId = GetCurrentThreadId();
      #ifdef _DEBUG
        if ( WhoIsLocked && *WhoIsLocked )
          strcpy_s( m_Owner , WhoIsLocked );
        else
          m_Owner[ 0 ] = 0 ;
        if ( ++m_iLockCnt > 1 )
        {
          TRACE( "\n Not first CObservedMutex locking (%d) hOld=%u hNew=%u" ,
            m_iLockCnt , m_dwOwnerId , dwCurrentThreadId ) ;
          if ( m_Owner[ 0 ] )
            TRACE( " Owner=%s" , m_Owner );
        }
      #endif
        m_dwOwnerId = dwCurrentThreadId;
        return true ;
      }
      else
      {
      #ifdef _DEBUG
        if ( m_bTraceEnable )
        {
          TRACE( "\nCObservedMutex: Can't lock mutex from %s(0x%0X), Locked by %s(0x%0X) Cnt=%d Stat=%d" ,
            WhoIsLocked ? WhoIsLocked : "Unknown" , GetCurrentThreadId() ,
            m_Owner[ 0 ] ? m_Owner : "Unknown" , m_dwOwnerId , m_iLockCnt , m_dwRes );
        }
      #endif
      }
    }
    return false;
  }
  void Unlock()
  {
    DWORD dwCurrentThreadId = GetCurrentThreadId();
  #ifdef _DEBUG
    ASSERT( dwCurrentThreadId == m_dwOwnerId );
  #endif
    if ( !ReleaseMutex( m_hMutex ) )
    {
    #ifdef _DEBUG
      TRACE( "\nCObservedMutex::Release - Can't release mutex from thread 0x%0X, Locked by %s(0x%0X), Err=%u" ,
        GetCurrentThread() ,
        m_Owner[ 0 ] ? m_Owner : "Unknown" , m_dwOwnerId ,
        GetLastError() ) ;
    #endif
    }
    else
    {
      if ( --m_iLockCnt <= 0 )
      {
      #ifdef _DEBUG
        m_Owner[ 0 ] = 0 ;
      #endif
        m_dwOwnerId = NULL ;
        m_iLockCnt = 0 ;
      }
      m_dwRes = 0 ;
    }
  }
  DWORD GetStatus()
  {
    return m_dwRes ;
  }
  bool IsMutexCreated()
  {
    return (m_hMutex != NULL) ;
  }
  int GetLockCnt()
  {
    return m_iLockCnt ;
  }
};

class CAutoLockMutex
{
  CObservedMutex * m_COMutex ;
  bool             m_bLocked ;

public:
  CAutoLockMutex( CObservedMutex& COMutex , DWORD dwTimeout = INFINITE
  #ifdef _DEBUG
    , LPCTSTR WhoIsLocked = NULL
  #endif
  )
  {
    m_COMutex = &COMutex ;
    if ( m_COMutex )
    {
      m_bLocked = m_COMutex->Lock( dwTimeout
      #ifdef _DEBUG
        , WhoIsLocked
      #endif
      ) ;
      if ( !m_bLocked )
      {
        TRACE( "  CAutoLockMutex: Can't lock " ) ;
      }
    }
  }

  ~CAutoLockMutex()
  {
    if ( m_COMutex 
      && m_COMutex->IsMutexCreated() 
      && m_bLocked )
    {
      m_COMutex->Unlock() ;
      m_bLocked = false ;
    }
  }

  bool Unlock()
  {
    if ( m_COMutex && m_COMutex->IsMutexCreated() )
    {
      m_COMutex->Unlock() ;
      m_bLocked = false ;
      return (m_COMutex->GetLockCnt() == 0) ;
    }
    return true ;
  }

  bool IsLocked()
  {
    return m_bLocked ;
  }
};

class CAutoLockMutex_H
{
public:
  CAutoLockMutex_H( HANDLE hMutex , DWORD dwTimeout = INFINITE
  #ifdef _DEBUG
    , LPCTSTR WhoIsLocked = NULL
  #endif
  )
  {
    m_dwRes = WaitForSingleObject( hMutex , dwTimeout ) ;
    if ( m_dwRes == WAIT_OBJECT_0 )
    {
      m_hMutex = hMutex ;
    #ifdef _DEBUG
      m_dwId = GetCurrentThreadId() ;
      if ( WhoIsLocked )
        strcpy_s( m_WhoIsLocked , WhoIsLocked );
    #endif
    }
    else
      m_hMutex = NULL ;
  } ;
  ~CAutoLockMutex_H()
  {
    if ( m_hMutex )
    {
      ReleaseMutex( m_hMutex ) ;
    #ifdef _DEBUG
      m_WhoIsLocked[0] = 0 ;
      m_dwId = 0 ;
    #endif
    }
  }

  bool Unlock()
  {
    if ( m_hMutex )
    {
      ReleaseMutex( m_hMutex ) ;
      m_hMutex = NULL ;
    #ifdef _DEBUG
      m_WhoIsLocked[ 0 ] = 0 ;
      m_dwId = 0 ;
    #endif
      return true ;
    }
    return false ;
  }
  DWORD GetStatus()
  {
    return m_dwRes ;
  }
  bool IsLocked()
  {
    return (m_hMutex != NULL) ;
  }
protected:
  HANDLE m_hMutex ;
  DWORD m_dwRes ;
  DWORD m_dwId ;
  TCHAR m_WhoIsLocked[50] ;
};

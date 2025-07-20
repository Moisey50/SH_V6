  
#ifndef __SH_MEMORY_H__
#define __SH_MEMORY_H__

#pragma pack(push,8)
#pragma warning(suppress : 4996) 
typedef struct  
{
  volatile LONG iNInputs ;
  volatile LONG  iNOutputs ;
  volatile LONG  iAreaSize ;
  volatile LONG  Pattern[2] ;
  volatile __int64  iNRequests ;
  volatile __int64  iNAnswers ;
  volatile __int64  iNTakenRequests ;
  volatile __int64  iNTakenAnswers ;
  volatile LONG  iNInputEvents ;
  volatile LONG  iNOutputEvents ;

} SHARED_MEMORY_HEADER;
#pragma pack(pop)

typedef struct  
{
  int  m_iIsMessage ;
  int  m_iId  ;
  DWORD m_dwSendTime ; // Windows Tick Count
  int  m_iMsgLen ;
  BYTE m_Data[1] ;
} MSG_INFO ;

class CSharedMessage : public MSG_INFO 
{
public:
  CSharedMessage( int iId = 0 , int iMsgLen = 0 , void * pData = NULL )
  {
    m_iIsMessage = 1 ;
    m_iId = iId ;
    m_iMsgLen = iMsgLen ;
    if ( pData )
      memcpy( m_Data , pData , iMsgLen - 2*sizeof(int) ) ;
    else  
      m_Data[0] = 0 ;
  }
  ~CSharedMessage(){} ;
} ;


typedef struct  
{
  int  m_iId  ;
  DWORD m_dwSendTime ; // Windows Tick Count
  int  m_iMsgLen ;
  BYTE m_Data[1] ;
} PURE_MSG_INFO ;

typedef struct  
{
  int  m_iId  ;
  int  m_iMsgLen ;
} ShMemMsgHeader ;

class CShMemoryBase
{
public:
  CShMemoryBase( 
    int iAreaSize = 4096 , 
    LPCTSTR AreaName = _T("FILE_X_AREA") , 
    LPCTSTR InEventName = NULL,
		LPCTSTR OutEventName = NULL) 
  {
    m_hFile = INVALID_HANDLE_VALUE ;
		m_hMap = m_hInEvent = m_hOutEvent = NULL ;
    m_pArea = NULL ;
    m_bDebugMode = 0 ;
    m_iNotNewArea = 0 ;
//    m_hFile = CreateFile(AreaName,
//      GENERIC_READ | GENERIC_WRITE,
//      FILE_SHARE_READ | FILE_SHARE_WRITE,
//      NULL,
//      OPEN_ALWAYS,
//      FILE_ATTRIBUTE_NORMAL,
//      NULL);
//    if ( m_hFile != INVALID_HANDLE_VALUE )
//    {
      m_hMap = CreateFileMapping( m_hFile,
        NULL,
        PAGE_READWRITE,
        0,
        iAreaSize ,
        AreaName);
      if ( m_hMap != INVALID_HANDLE_VALUE )
      {
        m_pArea = (BYTE*) MapViewOfFile(m_hMap,
          FILE_MAP_WRITE | FILE_MAP_READ,
          0, 0, iAreaSize );
			  DWORD err =	GetLastError();
        if ( m_pArea )
        {
          m_dwAreaSize  = iAreaSize ;
          SHARED_MEMORY_HEADER * pHeader = (SHARED_MEMORY_HEADER*)m_pArea ;
          if ( pHeader->Pattern[0] != 0xaa55cc33  || pHeader->Pattern[1] != 0x11223344)
          {
            memset( m_pArea , 0 , iAreaSize ) ;
            pHeader->iAreaSize = iAreaSize ;
            pHeader->Pattern[0] = 0xaa55cc33 ;
            pHeader->Pattern[1] = 0x11223344 ;
            pHeader->iNRequests = pHeader->iNAnswers = 0 ;
            pHeader->iNTakenRequests = pHeader->iNTakenAnswers = 0 ;
          }
          else
            m_iNotNewArea++ ;
          InterlockedIncrement( &pHeader->iNInputs ) ;
          if ( InEventName )
          {
            m_hInEvent = OpenEvent( EVENT_ALL_ACCESS , FALSE , InEventName ) ;
            if ( !m_hInEvent )
              m_hInEvent = CreateEvent( NULL , FALSE , FALSE , InEventName ) ;
            else
              pHeader->iNInputEvents++ ;
            if ( m_hInEvent )
              SetEvent( m_hInEvent ) ;
            _TCHAR MutexName[2000] ;
            _tcscpy_s( MutexName , InEventName ) ;
            _tcscat_s( MutexName , _T("_MUTEX") ) ;
            m_hProtectMutex = OpenMutex( MUTEX_ALL_ACCESS , NULL , MutexName ) ;
            if ( !m_hProtectMutex )
              m_hProtectMutex = CreateMutex( NULL , FALSE , MutexName ) ;
          }
          else
          {
            m_hInEvent = NULL ;
            m_hProtectMutex = NULL ;
          }

				  if ( OutEventName )
          {
            m_hOutEvent = OpenEvent( EVENT_ALL_ACCESS , FALSE , OutEventName ) ;
            if ( !m_hOutEvent )
              m_hOutEvent = CreateEvent( NULL , FALSE , FALSE , OutEventName ) ;
            else
              pHeader->iNOutputEvents++ ;
            if ( m_hOutEvent )
            {
              InterlockedIncrement(&(pHeader->iNOutputs)) ;
//               SetEvent( m_hOutEvent ) ;
            }
          }
          else
            m_hOutEvent = NULL ;

          m_szMemoryName = new TCHAR[_tcslen(AreaName) + 1] ;
//           LPTSTR p = (LPTSTR) m_szMemoryName ;
//           do 
//           {
//             *(p++) = *AreaName ;
//           } while ( *(AreaName++) );
          _tcscpy_s(  m_szMemoryName , _tcslen(AreaName) + 1 , AreaName) ;
          return ;
        }
      }
      Cleanup() ;
  }

  BOOL SetInEvent()
  {
    if ( m_hInEvent )
      return (SetEvent( m_hInEvent )) ;
    else
      return FALSE ;
  }
  BOOL ResetInEvent()
  {
    if ( m_hInEvent )
      return (ResetEvent(m_hInEvent));
    else
      return FALSE ;
  }

	BOOL SetOutEvent()
	{
    if ( m_hOutEvent )
      return(SetEvent( m_hOutEvent )) ;
    else
      return FALSE ;
	}

	BOOL ResetOutEvent()
	{
    if ( m_hOutEvent )
		  return (ResetEvent(m_hOutEvent));
    else
      return FALSE ;
	}
  HANDLE GetInEventHandle() { return m_hInEvent ; }
	HANDLE GetOutEventHandle() { return m_hOutEvent ; }
  bool WaitForEvent( HANDLE h , DWORD dwTimeout )
  {
    return  (WaitForSingleObject( h , dwTimeout ) == WAIT_OBJECT_0) ;
  }
  int WaitForOutEvent( DWORD dwTimeOut_ms )
  {
    DWORD dwWaitRes = WaitForSingleObject( m_hOutEvent , dwTimeOut_ms ) ;
    switch ( dwWaitRes )
    {
    case WAIT_OBJECT_0: return 1 ;
    case WAIT_TIMEOUT: return 0 ;
    case WAIT_ABANDONED: return -1 ;
    }
    return -2 ;
  }

  ~CShMemoryBase()
  {
    Cleanup() ;
  }
  
  void Cleanup() 
  {
    SHARED_MEMORY_HEADER * pHeader = (SHARED_MEMORY_HEADER*)m_pArea ;
    if (m_hInEvent)
    {
      SetEvent( m_hInEvent ) ;
      CloseHandle( m_hInEvent ) ;
    }
    if (m_hOutEvent)
    {
      SetEvent( m_hOutEvent ) ;
      CloseHandle( m_hOutEvent ) ;
    }
    if ( m_hProtectMutex )
      CloseHandle( m_hProtectMutex ) ;
    if (m_pArea)
    {
      if ( !InterlockedDecrement( &(pHeader->iNInputs) ))
        memset( pHeader , 0 , sizeof( *pHeader) ) ;
      UnmapViewOfFile( m_pArea ) ;
    }
    if ( m_hMap )
      CloseHandle( m_hMap ) ;
    if ( m_hFile && (m_hFile != INVALID_HANDLE_VALUE) )
      CloseHandle( m_hFile ) ;
    m_hFile = m_hMap = m_hInEvent = m_hOutEvent = m_hProtectMutex = NULL ;
    m_pArea = NULL ;
    delete m_szMemoryName ;
  }
  
  int GetAreaContent( UINT uiOffset , BYTE * pBuf , int iBufLen )
  {
    if ( !m_pArea )
      return -3 ;
    BYTE * p = m_pArea + uiOffset ;
    UINT uiCopySize = min( (UINT)iBufLen , m_dwAreaSize  - uiOffset) ;
    if ( uiCopySize > 0 )
    {
      if ( WaitForSingleObject( m_hProtectMutex , 10000 ) == WAIT_OBJECT_0)
      {
        memcpy( pBuf , p , uiCopySize ) ;
        SetEvent( m_hInEvent ) ;
        ReleaseMutex( m_hProtectMutex ) ;
        return uiCopySize ; 
      }
      return -2 ;
    }
    return -1 ;
  }

  int GetAreaContentDirect( UINT uiOffset , char * pBuf , int iBufLen )
  {
    if ( !m_pArea )
      return -3 ;
    if ( uiOffset < m_dwAreaSize  )
    {
      char * p = (char*) m_pArea + uiOffset ;
      int uiCopySize = min( ( UINT ) iBufLen , m_dwAreaSize  - uiOffset) ;
      memcpy( pBuf , p , uiCopySize ) ;
      return uiCopySize ;
    }
    return 0 ;
  }

  BOOL MarkAsReceived( UINT uiOffset )
  {
    if ( !m_pArea )
      return FALSE ;
    MSG_INFO * pSrc = ( MSG_INFO* ) ( m_pArea + uiOffset ) ;
    if ( pSrc->m_iIsMessage )
    {
      pSrc->m_iIsMessage = FALSE ;
      return TRUE ;
    }
    return FALSE ;
  }

  BYTE * GetMsgFromArea( UINT uiOffset , bool bOutEvent , int& iBufLen ,
    void * pBuf = NULL )
  {
    if ( !m_pArea )
      return NULL ;
    
    if ( WaitForSingleObject( m_hProtectMutex , 10000 ) == WAIT_OBJECT_0)
    {
      MSG_INFO * pSrc = (MSG_INFO*)(m_pArea + uiOffset) ;
      PURE_MSG_INFO * pSrcMsg = (PURE_MSG_INFO*)&(pSrc->m_iId) ;
      void * pResult = NULL ;
      if ( pSrc->m_iIsMessage ) // No message in shared memory
      {
        if ( !pBuf || ( iBufLen < pSrc->m_iMsgLen) )
        {
          iBufLen = pSrc->m_iMsgLen;
        }
        else
        {
          memcpy( pBuf , pSrcMsg->m_Data , pSrc->m_iMsgLen ) ;
          iBufLen = pSrc->m_iMsgLen ;
          pSrc->m_iIsMessage = 0 ;
          pResult = pBuf ;
          if ( bOutEvent )
            ResetOutEvent() ;
          else
            ResetInEvent() ;
        }
      }
      ReleaseMutex( m_hProtectMutex ) ;
      return (BYTE*) pResult ;
    }
    TRACE( "GetMsgFromArea: Can't take mutex %s" , (char*)GetShMemAreaName() ) ;
    iBufLen = 0 ; // sign about problem with mutex taking
    return NULL ;
  }
  LPCTSTR GetErrorString( int iErrCode )
  {
    
    switch ( iErrCode )
    {
      case -1: return "Too Long Message" ;
      case -2: return "Can't take channel (Mutex busy)" ;
      case -3: return "Shared Memory not initialized" ;
      case -4: return "Previous message not received on opposite side" ;
    }

  
    if ( iErrCode >= 0 )
    {
      sprintf( m_OutMsg , "Transfered %d bytes" , iErrCode ) ;
    }
    else
      sprintf( m_OutMsg , "Error code %d " , iErrCode ) ;
    return m_OutMsg ;
  }

  int PutMsgToArea( UINT uiOffset , bool bOutEvent , 
    void * pMsg , int iMsgLen )
  {
    if ( !m_pArea )
      return -3 ;
      // the allocation length is MSG_INFO plus msg length minus first
      // byte of msg, which is accounted in MSG_INFO declaration
    int iFullMsgLen = sizeof( MSG_INFO) - 1 + iMsgLen ;
    if ( uiOffset + (UINT)iFullMsgLen > m_dwAreaSize  )
      return -1 ;
    MSG_INFO * pDest = (MSG_INFO*)(m_pArea + uiOffset) ;
    PURE_MSG_INFO * pSrc = (PURE_MSG_INFO*)pMsg ;
    if ( WaitForSingleObject( m_hProtectMutex , 10000 ) == WAIT_OBJECT_0)
    {
      if ( !pDest->m_iIsMessage 
        || GetTickCount() > pDest->m_dwSendTime + m_dwTimeoutForTransfer )
      {
        memcpy( &(pDest->m_Data[0]) , pMsg , iMsgLen) ;
        pDest->m_iId = ++m_iMsgCnt ;
        pDest->m_dwSendTime = GetTickCount() ;
        pDest->m_iMsgLen = iMsgLen ; // Msg length after header 
        pDest->m_iIsMessage = 1 ;
        if ( bOutEvent )
          SetOutEvent() ;
        else
          SetInEvent() ;
      }
      else
        iFullMsgLen = -4 ; // previous message is not received on opposite side
      ReleaseMutex( m_hProtectMutex ) ;
      return iFullMsgLen ;
    }
    TRACE( "PutMsgToArea: Can't take mutex %s" , ( char* ) GetShMemAreaName() ) ;
    return -2 ;
  }

  int SetAreaContent( int uiOffset , BYTE * pBuf , int iBufLen )
  {
    if ( !m_pArea )
      return -3 ;
    char * p = (char*) m_pArea + uiOffset ;
    int uiCopySize = min( ( UINT ) iBufLen , m_dwAreaSize  - uiOffset) ;
    if ( uiCopySize > 0 )
    {
      if ( WaitForSingleObject( m_hProtectMutex , 10000 ) == WAIT_OBJECT_0)
      {
        memcpy( p+1 , pBuf+1 , uiCopySize-1 ) ;
        *p = *pBuf ;
        if (m_hInEvent)
          SetEvent( m_hInEvent ) ;
        ReleaseMutex( m_hProtectMutex ) ;
        return uiCopySize ;
      }
      TRACE( "SetAreaContent: Can't take mutex %s" , ( char* ) GetShMemAreaName() ) ;
      return -2 ;
    }
    //ASSERT(0) ;
    return -1 ;
  }
  int SetAreaContentDirect( UINT uiOffset , const char * pBuf , int iBufLen )
  {
    if ( !m_pArea )
      return -1 ;
    if ( uiOffset < m_dwAreaSize  )
    {
      char * p = (char*) m_pArea + uiOffset ;
      int uiCopySize = min( ( UINT ) iBufLen , m_dwAreaSize  - uiOffset) ;
      memcpy( p+1 , pBuf+1 , uiCopySize-1 ) ;
      *p = *pBuf ;
      return uiCopySize ;
    }
    return 0 ;
  }
  bool IsInitialized() { return (m_pArea != NULL) ; }
  BYTE * GetAreaPtr( UINT uiOffset )
  {
    if ( uiOffset >= 40  &&  uiOffset < m_dwAreaSize  )
      return m_pArea + uiOffset ;
    else
      return NULL ;
  }
  const TCHAR * GetShMemAreaName() { return m_szMemoryName ; } ;
  int IsNotNewArea() { return m_iNotNewArea ;}
  virtual const char * GetName() { return "CShMemoryBase" ; }
  virtual int GetNConnected() { return *((int*)m_pArea) ; }
  DWORD GetTimeout() { return m_dwTimeoutForTransfer ; }
  void SetTimeout( DWORD dwTimeout_ms = 1000 )
  {
    m_dwTimeoutForTransfer = dwTimeout_ms ;
  }
  BOOL SetInAreaSize( UINT uiSize )
  {
    if ( uiSize < m_dwAreaSize - 2048 ) // 1024 for shared memory header
    {                                   // and minimum 1024 for out area size
      m_dwInAreaSize = uiSize ;
      m_dwOutAreaSize = m_dwAreaSize - m_dwInAreaSize - 1024 ; // 1024 for header
      return TRUE ;
    }
    return FALSE ;
  }
protected:
  DWORD     m_dwAreaSize ;
  DWORD     m_dwInAreaSize ;
  DWORD     m_dwOutAreaSize ;
  BYTE *    m_pArea ;
  HANDLE    m_hFile ;
  HANDLE    m_hMap ;
  HANDLE    m_hInEvent,m_hOutEvent;
  HANDLE    m_hProtectMutex ;
  int       m_bDebugMode ;
  int       m_iNotNewArea ;
  int       m_iMsgCnt = 0 ;
  DWORD     m_dwTimeoutForTransfer = 1000 ; // 1 second
  LPTSTR    m_szMemoryName ;
  char      m_OutMsg[ 100 ] ;
  char      m_InMsg[ 100 ] ;
};

#endif // __SH_MEMORY_H__


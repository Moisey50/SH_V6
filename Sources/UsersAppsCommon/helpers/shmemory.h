  
#ifndef __SH_MEMORY_H__
#define __SH_MEMORY_H__

#pragma pack(push,8)
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
} SHARED_MEMORY_HEADER;
#pragma pack(pop)

typedef struct  
{
  int  m_iIsMessage ;
  int  m_iId  ;
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
          m_iAreaSize = iAreaSize ;
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
              m_iNotNewArea++ ;
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
              m_iNotNewArea++ ;
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
  
  int GetAreaContent( int iOffset , BYTE * pBuf , int iBufLen )
  {
    if ( !m_pArea )
      return -3 ;
    BYTE * p = m_pArea + iOffset ;
    int iCopySize = min(iBufLen , m_iAreaSize - iOffset) ;
    if ( iCopySize > 0 )
    {
      if ( WaitForSingleObject( m_hProtectMutex , 10000 ) == WAIT_OBJECT_0)
      {
        memcpy( pBuf , p , iCopySize ) ;
        SetEvent( m_hInEvent ) ;
        ReleaseMutex( m_hProtectMutex ) ;
        return iCopySize ; 
      }
      return -2 ;
    }
    return -1 ;
  }

  int GetAreaContentDirect( int iOffset , char * pBuf , int iBufLen )
  {
    if ( !m_pArea )
      return -3 ;
    if ( iOffset < m_iAreaSize )
    {
      char * p = (char*) m_pArea + iOffset ;
      int iCopySize = min(iBufLen , m_iAreaSize - iOffset) ;
      memcpy( pBuf , p , iCopySize ) ;
      return iCopySize ;
    }
    return 0 ;
  }

  BYTE * GetMsgFromArea( int iOffset , bool bOutEvent , 
    void * pBuf = NULL , int * piBufLen = NULL )
  {
    if ( !m_pArea )
      return NULL ;
    
    if ( WaitForSingleObject( m_hProtectMutex , 10000 ) == WAIT_OBJECT_0)
    {
      MSG_INFO * pSrc = (MSG_INFO*)(m_pArea + iOffset) ;
      PURE_MSG_INFO * pSrcMsg = (PURE_MSG_INFO*)&(pSrc->m_iId) ;
      void * pResult = NULL ;
      if ( pSrc->m_iIsMessage ) // No message in shared memory
      {
        if ( !pBuf || !piBufLen || *piBufLen < pSrc->m_iMsgLen )
          pResult = (void*) pSrc->m_iMsgLen;
        else
        {
          pResult = pBuf ;
          memcpy( pBuf , pSrcMsg , pSrc->m_iMsgLen ) ;
          *piBufLen = pSrc->m_iMsgLen ;
          pSrc->m_iIsMessage = 0 ;
        }
        if ( bOutEvent )
          ResetOutEvent() ;
        else
          ResetInEvent() ;
      }
      ReleaseMutex( m_hProtectMutex ) ;
      return (BYTE*) pResult ;
    }
    TRACE( "GetMsgFromArea: Can't take mutex %s" , GetShMemAreaName() ) ;
    return NULL ;
  }
  
  int PutMsgToArea( int iOffset , bool bOutEvent , 
    void * pMsg , int iMsgLen )
  {
    if ( !m_pArea )
      return -3 ;
      // the allocation length is MSG_INFO plus msg length minus first
      // byte of msg, which is accounted in MSG_INFO declaration
    int iFullMsgLen = iMsgLen + sizeof(int) ;
    if ( iOffset + iFullMsgLen > m_iAreaSize )
      return -1 ;
    MSG_INFO * pDest = (MSG_INFO*)(m_pArea + iOffset) ;
    PURE_MSG_INFO * pSrc = (PURE_MSG_INFO*)pMsg ;
    if ( WaitForSingleObject( m_hProtectMutex , 10000 ) == WAIT_OBJECT_0)
      {
//       if ( !pDest->m_iIsMessage )
//       {
        memcpy( &(pDest->m_iId) , pMsg , iMsgLen) ;
        pDest->m_iIsMessage = 1 ;
        if ( bOutEvent )
          SetOutEvent() ;
        else
          SetInEvent() ;
//    }
//       else
//         iFullMsgLen = -4 ; // previous message is not received on opposite side
      ReleaseMutex( m_hProtectMutex ) ;
      return iFullMsgLen ;
    }
    TRACE( "PutMsgToArea: Can't take mutex %s" , GetShMemAreaName() ) ;
    return -2 ;
  }

  int SetAreaContent( int iOffset , BYTE * pBuf , int iBufLen )
  {
    if ( !m_pArea )
      return -3 ;
    char * p = (char*) m_pArea + iOffset ;
    int iCopySize = min(iBufLen , m_iAreaSize - iOffset) ;
    if ( iCopySize > 0 )
    {
      if ( WaitForSingleObject( m_hProtectMutex , 10000 ) == WAIT_OBJECT_0)
      {
        memcpy( p+1 , pBuf+1 , iCopySize-1 ) ;
        *p = *pBuf ;
        if (m_hInEvent)
          SetEvent( m_hInEvent ) ;
        ReleaseMutex( m_hProtectMutex ) ;
        return iCopySize ;
      }
      TRACE( "SetAreaContent: Can't take mutex %s" , GetShMemAreaName() ) ;
      return -2 ;
    }
    ASSERT(0) ;
    return -1 ;
  }
  int SetAreaContentDirect( int iOffset , const char * pBuf , int iBufLen )
  {
    if ( !m_pArea )
      return -1 ;
    if ( iOffset < m_iAreaSize )
    {
      char * p = (char*) m_pArea + iOffset ;
      int iCopySize = min(iBufLen , m_iAreaSize - iOffset) ;
      memcpy( p+1 , pBuf+1 , iCopySize-1 ) ;
      *p = *pBuf ;
      return iCopySize ;
    }
    return 0 ;
  }
  bool IsInitialized() { return (m_pArea != NULL) ; }
  BYTE * GetAreaPtr( int iOffset )
  {
    if ( iOffset >= 40  &&  iOffset < m_iAreaSize )
      return m_pArea + iOffset ;
    else
      return NULL ;
  }
  const TCHAR * GetShMemAreaName() { return m_szMemoryName ; } ;
  int IsNotNewArea() { return m_iNotNewArea ;}
  virtual const char * GetName() { return "CShMemoryBase" ; }
  virtual int GetNConnected() { return *((int*)m_pArea) ; }
protected:
  int m_iAreaSize ;
  BYTE * m_pArea ;
  HANDLE m_hFile ;
  HANDLE m_hMap ;
  HANDLE m_hInEvent,m_hOutEvent;
  HANDLE m_hProtectMutex ;
  int m_bDebugMode ;
  int    m_iNotNewArea ;
  LPTSTR m_szMemoryName ;
};

#endif // __SH_MEMORY_H__


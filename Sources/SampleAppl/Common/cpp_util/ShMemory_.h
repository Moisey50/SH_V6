
#ifndef __SH_MEMORY_H__
#define __SH_MEMORY_H__

class CShMemoryBase
{
public:
  CShMemoryBase( 
    int iAreaSize = 4096 , 
    const char * AreaName = "FILE_X_AREA" , 
    const char * ProtEventName = NULL ) 
  {
    m_hFile = INVALID_HANDLE_VALUE ;
		m_hMap = m_hInEvent = m_hOutEvent = NULL ;
    m_pArea = NULL ;
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
        m_pArea = (char*) MapViewOfFile(m_hMap,
          FILE_MAP_WRITE | FILE_MAP_READ,
          0, 0, iAreaSize );
        if ( m_pArea )
        {
          m_iAreaSize = iAreaSize ;
          if ( ProtEventName )
          {
            m_hInEvent = OpenEvent( EVENT_ALL_ACCESS , FALSE , ProtEventName ) ;
						m_hOutEvent = OpenEvent( EVENT_ALL_ACCESS , FALSE , ProtEventName ) ;
            if ( !m_hInEvent )
            {
              m_hInEvent = CreateEvent( NULL , FALSE , FALSE , ProtEventName ) ;
							m_hOutEvent = CreateEvent( NULL , FALSE , FALSE , ProtEventName ) ;
              if ( m_hInEvent )
              {
                memset( m_pArea , 0 , iAreaSize ) ;
                int * p = (int*) m_pArea ;
                *p = 1 ;
                SetEvent( m_hInEvent ) ;
              }
            }
            else
            {
              if (WaitForSingleObject( m_hInEvent , 1000 ) == WAIT_OBJECT_0)
              {
                int * p = (int*) m_pArea ;
                (*p)++ ;
                SetEvent( m_hInEvent ) ;
              }
            }
          }
          else
            m_hInEvent = NULL ;

          return ;
        }
      }
      Cleanup() ;
//    }
//    else
//    {
//      int iErr = GetLastError() ;
//      if ( iErr == 0 )
//        iErr++ ;
//    }

  }

  void SetInEvent()
  {
    SetEvent( m_hInEvent ) ;
  }

	void SetOutEvent()
	{
    SetEvent( m_hOutEvent ) ;
	}

	void ResetOutEvent()
	{
		ResetEvent(m_hOutEvent);
	}

	void ResetInEvent()
	{
		ResetEvent(m_hInEvent);
	}


  HANDLE GetInEventHandle() { return m_hInEvent ; }
	HANDLE GetOutEventHandle() { return m_hOutEvent ; }

  ~CShMemoryBase()
  {
    Cleanup() ;
  }
  
  void Cleanup()
  {
    if (m_hInEvent)
    {
      if (WaitForSingleObject( m_hInEvent , 1000 ) == WAIT_OBJECT_0)
      {
        if (m_pArea)
        {
          int * p = (int*) m_pArea ;
          (*p)-- ;
          SetEvent( m_hInEvent ) ;
        }
      }
      CloseHandle( m_hInEvent ) ;
    }
    if (m_pArea)
      UnmapViewOfFile( m_pArea ) ;
    if ( m_hMap )
      CloseHandle( m_hMap ) ;
    if ( m_hFile )
      CloseHandle( m_hFile ) ;
    m_hFile = m_hMap = m_hInEvent = NULL ;
    m_pArea = NULL ;
  }
  
  int GetAreaContent( int iOffset , char * pBuf , int iBufLen )
  {
    if ( !m_pArea )
      return -1 ;
    if ( !m_hInEvent  ||  WaitForSingleObject( m_hInEvent , 1000 ) == WAIT_OBJECT_0)
    {
      if ( iOffset < m_iAreaSize )
      {
        char * p = (char*) m_pArea + iOffset ;
        int iCopySize = min(iBufLen , m_iAreaSize - iOffset) ;
        memcpy( pBuf , p , iCopySize ) ;
        SetEvent( m_hInEvent ) ;
        return iCopySize ;
      }
      return 0 ;
    }
    return -1 ;
  }
  
  int GetAreaContentDirect( int iOffset , char * pBuf , int iBufLen )
  {
    if ( !m_pArea )
      return -1 ;
    if ( iOffset < m_iAreaSize )
    {
      char * p = (char*) m_pArea + iOffset ;
      int iCopySize = min(iBufLen , m_iAreaSize - iOffset) ;
      memcpy( pBuf , p , iCopySize ) ;
      return iCopySize ;
    }
    return 0 ;
  }
  int SetAreaContent( int iOffset , const char * pBuf , int iBufLen )
  {
    if ( !m_pArea )
      return -1 ;
    if ( !m_hInEvent  ||  WaitForSingleObject( m_hInEvent , 1000 ) == WAIT_OBJECT_0)
    {
      if ( iOffset < m_iAreaSize )
      {
        char * p = (char*) m_pArea + iOffset ;
        int iCopySize = min(iBufLen , m_iAreaSize - iOffset) ;
        memcpy( p+1 , pBuf+1 , iCopySize-1 ) ;
        *p = *pBuf ;
        if (m_hInEvent)
          SetEvent( m_hInEvent ) ;
        return iCopySize ;
      }
      return 0 ;
    }
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
  char * GetAreaPtr( int iOffset )
  {
    if ( iOffset >= 40  &&  iOffset < m_iAreaSize )
      return m_pArea + iOffset ;
    else
      return NULL ;
  }
protected:
  int m_iAreaSize ;
  char * m_pArea ;
  HANDLE m_hFile ;
  HANDLE m_hMap ;
  HANDLE m_hInEvent,m_hOutEvent;
};

#endif // __SH_MEMORY_H__


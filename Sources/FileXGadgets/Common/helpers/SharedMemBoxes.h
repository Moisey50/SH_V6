  
#ifndef __SH_MEMORY_H__
#define __SH_MEMORY_H__

#define BOXES_BEGIN_AT 256

#define MAX_SEM_COUNT 1000
#define  H264SemaphoreName "H264Semaphore"

typedef struct  
{
  long long m_llAreaSize ;
  int m_iNBoxes ;
  int m_iBoxSize ;
  char m_AreaName[ BOXES_BEGIN_AT - 16] ;
} ShMemBoxesHeader ;

typedef struct  
{
  int m_ID ;
  int m_iIndex ;
  //int m_iPlayDecIndex;
} IPCamInfo ;

enum ShMemBoxError
{
  shMemBoxOK = 0 ,
  shMemBoxNotMapped = (-1) ,
  shMemBoxBusy = (-2) ,
  shMemBoxNotEnoughBufferLength = (-3) ,
  shMemBoxIndexError = (-4) ,
  shMemBoxTimeout = (-5)
};
class CSharedMemBoxes
{
public:
  CSharedMemBoxes( 
    int iNBoxes = 128,//64 , 
    int iBoxSizeBytes = sizeof(IPCamInfo) ,
    LPCTSTR pAreaName = _T("IPCAM_AREA") 
    ) 
  {
    _TCHAR MutexName[256] ;
    _tcscpy_s( MutexName , pAreaName ) ;
    _tcscat_s( MutexName , _T("_MUTEX") ) ;
    HANDLE hMutex = OpenMutex( MUTEX_ALL_ACCESS , NULL , MutexName ) ;
    if ( !hMutex )
    {
      m_hProtectMutex = CreateMutex( NULL , FALSE , MutexName ) ;
    }
    else
    {
      Sleep(30) ;
      m_hProtectMutex = hMutex ;
    }
    if ( WaitForSingleObject( m_hProtectMutex , 2000) == WAIT_OBJECT_0 )
    {
      m_hFile = INVALID_HANDLE_VALUE ;
      m_hMap = /*m_hProtectMutex =*/ NULL ;
      m_pArea = NULL ;
      int iAreaSize = BOXES_BEGIN_AT + iNBoxes * iBoxSizeBytes ;
      m_hMap = CreateFileMapping( m_hFile, 
        NULL,
        PAGE_READWRITE,
        0,
        iAreaSize ,
        pAreaName);
      if ( m_hMap != INVALID_HANDLE_VALUE )
      {
        m_pArea = (BYTE*) MapViewOfFile(m_hMap,
          FILE_MAP_WRITE | FILE_MAP_READ,
          0, 0, iAreaSize );
        DWORD err =	GetLastError();
        if ( m_pArea ) // there is area 
        {
          if ( hMutex == NULL ) // and this is created area (not attached to)
          {
            m_iAreaSize = iAreaSize ;
            ShMemBoxesHeader * pHeader = (ShMemBoxesHeader*)m_pArea ;

            pHeader->m_llAreaSize = iAreaSize ;
            pHeader->m_iNBoxes = iNBoxes ;
            pHeader->m_iBoxSize = iBoxSizeBytes ;
            _tcscpy_s(  pHeader->m_AreaName , sizeof( pHeader->m_AreaName) , pAreaName) ;
          }
          ReleaseMutex( m_hProtectMutex ) ;
          return ;
        }
      }
    }
    Cleanup() ;
  }

  ~CSharedMemBoxes()
  {
    Cleanup() ;
  }
  
  void Cleanup() 
  {
    ShMemBoxesHeader * pHeader = (ShMemBoxesHeader*)m_pArea ;

    if (m_pArea)
    {
      UnmapViewOfFile( m_pArea ) ;
      m_pArea = NULL ;
    }
    if ( m_hMap )
    {
      CloseHandle( m_hMap ) ;
      m_hMap = NULL ;
    }
    if ( m_hFile && (m_hFile != INVALID_HANDLE_VALUE) )
    {
      CloseHandle( m_hFile ) ;
    }
    if ( m_hProtectMutex )
    {
      CloseHandle( m_hProtectMutex ) ;
      m_hProtectMutex = NULL ;
    }
  }
  
  int GetNBoxes() { return ((ShMemBoxesHeader*)m_pArea)->m_iNBoxes ; }
  int GetBoxSize() { return ((ShMemBoxesHeader*)m_pArea)->m_iBoxSize ; }
  bool IsEmpty( LPBYTE p , int iLen )
  {
    while ( *(p++) == 0 )
    {
      if ( --iLen <= 0 )
        return true ;
    }
    return false ;
  }
  ShMemBoxError GetBoxContent( int iBoxNumber , BYTE * pBuf , int iBufLen )
  {
    if ( !m_pArea )
      return shMemBoxNotMapped ;
    if ( iBoxNumber < 0 || iBoxNumber >= GetNBoxes() )
      return shMemBoxIndexError ;
    if ( iBufLen < GetBoxSize() )
      return shMemBoxNotEnoughBufferLength ;
    BYTE * p = m_pArea + BOXES_BEGIN_AT + iBoxNumber * GetBoxSize() ;
    if ( WaitForSingleObject( m_hProtectMutex , 2000 ) == WAIT_OBJECT_0 )
    {
      memcpy( pBuf , p , GetBoxSize() ) ;
      ReleaseMutex( m_hProtectMutex ) ;
      return shMemBoxOK ;
    }
    return shMemBoxTimeout ;
  }
  /*
  ShMemBoxError SetBoxIfNotExist( int iBoxNumber , BYTE * pBuf , int iBufLen)
  {
    if ( !m_pArea )
      return shMemBoxNotMapped ;
    if ( iBoxNumber < 0 || iBoxNumber >= GetNBoxes() )
      return shMemBoxIndexError ;
    if ( iBufLen != GetBoxSize() )
      return shMemBoxNotEnoughBufferLength ;
	BYTE * p = m_pArea + BOXES_BEGIN_AT + iBoxNumber * GetBoxSize() ;
    if ( WaitForSingleObject( m_hProtectMutex , 2000 ) == WAIT_OBJECT_0 )
    {
      int istatus = -1;
      bool bBusy = !IsEmpty( p , GetBoxSize()) ;
      if ( !bBusy  )
	  {
        memcpy( p , pBuf , GetBoxSize() ) ;
		istatus = 0;
	  }
	  else
	  {
		IPCamInfo  newIPCamBusInfo;
		IPCamInfo  curIPCamBusInfo;
		memcpy( &newIPCamBusInfo , pBuf , GetBoxSize() ) ;
		memcpy( &curIPCamBusInfo , p , GetBoxSize() ) ;
		if (curIPCamBusInfo.m_ID == newIPCamBusInfo.m_ID && curIPCamBusInfo.m_iIndex == newIPCamBusInfo.m_iIndex)
		{
			istatus = 1;
		}
		else
		{
			istatus = 2;
		}
	  }

      ReleaseMutex( m_hProtectMutex ) ;
	  if (istatus == 0)
	  {
			return shMemBoxOK;
	  }
	  else if(istatus == 1)
	  {
		   return shMemBoxBusy;
	  }
	  else
	  {
		    return shMemBoxIndexError;
	  }
      //return (bBusy) ? shMemBoxBusy : shMemBoxOK ;
    }
    return shMemBoxTimeout ;
  }
  */
  ShMemBoxError SetBoxContent( int iBoxNumber , BYTE * pBuf , int iBufLen )
  {
    if ( !m_pArea )
      return shMemBoxNotMapped ;
    if ( iBoxNumber < 0 || iBoxNumber >= GetNBoxes() )
      return shMemBoxIndexError ;
    if ( iBufLen != GetBoxSize() )
      return shMemBoxNotEnoughBufferLength ;
    BYTE * p = m_pArea + BOXES_BEGIN_AT + iBoxNumber * GetBoxSize() ;
    if ( WaitForSingleObject( m_hProtectMutex , 2000 ) == WAIT_OBJECT_0 )
    {
      bool bBusy = !IsEmpty( p , GetBoxSize()) ;
      if ( !bBusy  )
        memcpy( p , pBuf , GetBoxSize() ) ;

      ReleaseMutex( m_hProtectMutex ) ;
      return (bBusy) ? shMemBoxBusy : shMemBoxOK ;
    }
    return shMemBoxTimeout ;
  }

  ShMemBoxError SetToFirstFree( int& iBoxNumber , BYTE * pBuf , int iBufLen )
  {
    if ( !m_pArea )
      return shMemBoxNotMapped ;
    //if ( iBoxNumber < 0 || iBoxNumber >= GetNBoxes() )
    //  return shMemBoxIndexError ;
    if ( iBufLen != GetBoxSize() )
      return shMemBoxNotEnoughBufferLength ;
    if ( WaitForSingleObject( m_hProtectMutex , 2000 ) == WAIT_OBJECT_0 )
    {
      bool bFound = false ;
      int iBoxCnt = 0 ;
      while ( !bFound && iBoxCnt < GetNBoxes() )
      {
        BYTE * p = m_pArea + BOXES_BEGIN_AT + iBoxCnt * GetBoxSize() ;
        if ( IsEmpty( p , GetBoxSize())  )
        {
          memcpy( p , pBuf , GetBoxSize() ) ;
          iBoxNumber = iBoxCnt ;
          break ;
        }
        iBoxCnt++ ;
      }

      ReleaseMutex( m_hProtectMutex ) ;
      return (iBoxCnt >= GetNBoxes()) ? shMemBoxBusy : shMemBoxOK ;
    }
    return shMemBoxTimeout ;
  }

  ShMemBoxError BoxClear( int iBoxNumber )
  {
    if ( !m_pArea )
      return shMemBoxNotMapped ;
    if ( iBoxNumber < 0 || iBoxNumber >= GetNBoxes() )
      return shMemBoxIndexError ;
    BYTE * p = m_pArea + BOXES_BEGIN_AT + iBoxNumber * GetBoxSize() ;
    memset( p , 0 , GetBoxSize() ) ;
    return shMemBoxOK ;
  }

  ShMemBoxError IsBusy( int iBoxNumber , bool& bBusy)
  {
    if ( !m_pArea )
      return shMemBoxNotMapped ;
    if ( iBoxNumber < 0 || iBoxNumber >= GetNBoxes() )
      return shMemBoxIndexError ;
    BYTE * p = m_pArea + BOXES_BEGIN_AT + iBoxNumber * GetBoxSize() ;
    if ( WaitForSingleObject( m_hProtectMutex , 2000 ) == WAIT_OBJECT_0 )
    {
      bBusy = !IsEmpty( p , GetBoxSize()) ;
      ReleaseMutex( m_hProtectMutex ) ;
      return shMemBoxOK ;
    }
    return shMemBoxTimeout ;
  }

  bool IsInitialized() { return (m_pArea != NULL) ; }
  BYTE * GetBoxesArray()
  {
    return m_pArea + BOXES_BEGIN_AT ;
  }
  const TCHAR * GetShMemAreaName() { return ((ShMemBoxesHeader*)m_pArea)->m_AreaName ; } ;
  virtual const char * GetName() { return "CShMemoryBase" ; }
protected:
  int m_iAreaSize ;
  BYTE * m_pArea ;
  HANDLE m_hFile ;
  HANDLE m_hMap ;
  HANDLE m_hProtectMutex ;
};

#endif // __SH_MEMORY_H__


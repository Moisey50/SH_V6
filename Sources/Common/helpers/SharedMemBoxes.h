  
#ifndef __SH_MEMORY_H__
#define __SH_MEMORY_H__

#include <helpers/AutoLockMutex.h>
#define BOXES_BEGIN_AT 256

typedef struct  
{
  long long m_llAreaSize ;
  int m_iNBoxes ;
  int m_iBoxSize ;
  int m_iNBusyBoxes ;
  char m_AreaName[ BOXES_BEGIN_AT - 20 ] ;
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
  CSharedMemBoxes() 
  {
    memset( this , 0 , sizeof(*this) ) ;
  }

  bool Initialize( 
    int iNBoxes = 64 , 
    int iBoxSizeBytes = 4 ,
    LPCTSTR pAreaName = _T("SHARED_MEMORY_BOX") 
    ) 
  {
    _TCHAR MutexName[256] ;
    _tcscpy_s( MutexName , pAreaName ) ;
    _tcscat_s( MutexName , _T("_MUTEX") ) ;
    HANDLE hMutex = OpenMutex( MUTEX_ALL_ACCESS , NULL , MutexName ) ;
    if ( !hMutex )
    {
      m_hProtectMutex = CreateMutex( NULL , TRUE , MutexName ) ;
    }
    else
    {
      Sleep(30) ;
      m_hProtectMutex = hMutex ;
      if ( WaitForSingleObject( m_hProtectMutex , 2000) != WAIT_OBJECT_0 )
      {
        FxSendLogMsg(MSG_ERROR_LEVEL,_T("CSharedMemBoxes()") , 0 , 
          "Can't take mutex %s" , MutexName ) ;
        Cleanup() ;
        return false ;
      }
    }
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
          memset( m_pArea , 0 , iAreaSize ) ;
          m_iAreaSize = iAreaSize ;
          ShMemBoxesHeader * pHeader = (ShMemBoxesHeader*)m_pArea ;

          pHeader->m_llAreaSize = iAreaSize ;
          pHeader->m_iNBoxes = iNBoxes ;
          pHeader->m_iBoxSize = iBoxSizeBytes ;
          _tcscpy_s(  pHeader->m_AreaName , pAreaName) ;
        }
        ReleaseMutex( m_hProtectMutex ) ;
      }
      return true ;
    }
    FxSendLogMsg(MSG_ERROR_LEVEL,_T("CSharedMemBoxes()") , 0 , 
      "Problem to map Shared Memory for %s" , pAreaName ) ;
    Cleanup() ;
    return false ;
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
    DWORD Status = WaitForSingleObject( m_hProtectMutex , 2000 ) ;
    if ( Status != WAIT_OBJECT_0 )
    {
      if ( Status == WAIT_TIMEOUT )
        return shMemBoxTimeout ;
      ASSERT( 0 ) ;
    }
    ShMemBoxError Result = (!m_pArea) ? shMemBoxNotMapped :
      (iBoxNumber < 0 || iBoxNumber >= GetNBoxes()) ? shMemBoxIndexError :
      (iBufLen < GetBoxSize()) ? shMemBoxNotEnoughBufferLength : shMemBoxOK ;
    if ( (Result == shMemBoxOK) || (Result == shMemBoxNotEnoughBufferLength) )
    {
      int iLen = (Result == shMemBoxNotEnoughBufferLength) ? iBufLen : GetBoxSize() ;
      BYTE * p = m_pArea + BOXES_BEGIN_AT + iBoxNumber * GetBoxSize() ;
      memcpy( pBuf , p , iLen ) ;
    }
    ReleaseMutex( m_hProtectMutex ) ;
    return Result ;
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
    CAutoLockMutex_H al( m_hProtectMutex , 2000 ) ;
    if ( al.GetStatus() != WAIT_OBJECT_0 )
    {
      if ( al.GetStatus() == WAIT_TIMEOUT )
        return shMemBoxTimeout ;
      ASSERT( 0 ) ;
    }
    if ( !m_pArea )
      return shMemBoxNotMapped ;
    if ( iBoxNumber < 0 || iBoxNumber >= GetNBoxes() )
      return shMemBoxIndexError ;
    if ( iBufLen > GetBoxSize() )
      return shMemBoxNotEnoughBufferLength ;
    BYTE * p = m_pArea + BOXES_BEGIN_AT + iBoxNumber * GetBoxSize() ;
    if ( WaitForSingleObject( m_hProtectMutex , 2000 ) == WAIT_OBJECT_0 )
    {
      bool bBusy = !IsEmpty( p , GetBoxSize()) ;
      if ( !bBusy  )
        memcpy( p , pBuf , iBufLen ) ;

      return (bBusy) ? shMemBoxBusy : shMemBoxOK ;
    }
    return shMemBoxTimeout ;
  }

  ShMemBoxError SetToFirstFree( int& iBoxNumber , BYTE * pBuf , int iBufLen )
  {
    CAutoLockMutex_H al( m_hProtectMutex , 2000 ) ;
    if ( al.GetStatus() != WAIT_OBJECT_0 )
    {
      if ( al.GetStatus() == WAIT_TIMEOUT )
        return shMemBoxTimeout ;
      ASSERT( 0 ) ;
    }
    if ( !m_pArea )
      return shMemBoxNotMapped ;
    //if ( iBoxNumber < 0 || iBoxNumber >= GetNBoxes() )
    //  return shMemBoxIndexError ;
    if ( iBufLen != GetBoxSize() )
      return shMemBoxNotEnoughBufferLength ;
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
    return (iBoxCnt >= GetNBoxes()) ? shMemBoxBusy : shMemBoxOK ;
  }

  ShMemBoxError BoxClear( int iBoxNumber )
  {
    CAutoLockMutex_H al( m_hProtectMutex , 2000 ) ;
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
    CAutoLockMutex_H al( m_hProtectMutex , 2000 ) ;
    if ( al.GetStatus() != WAIT_OBJECT_0 )
    {
      if ( al.GetStatus() == WAIT_TIMEOUT )
        return shMemBoxTimeout ;
      ASSERT( 0 ) ;
    }
    if ( !m_pArea )
      return shMemBoxNotMapped ;
    if ( iBoxNumber < 0 || iBoxNumber >= GetNBoxes() )
      return shMemBoxIndexError ;
    BYTE * p = m_pArea + BOXES_BEGIN_AT + iBoxNumber * GetBoxSize() ;
    bBusy = !IsEmpty( p , GetBoxSize()) ;
    return shMemBoxOK ;
  }

  bool IsInitialized() { return (m_pArea != NULL) ; }
  BYTE * GetBoxesArray()
  {
    return m_pArea + BOXES_BEGIN_AT ;
  }
  const TCHAR * GetShMemAreaName() { return ((ShMemBoxesHeader*)m_pArea)->m_AreaName ; } 
  virtual const char * GetName() { return "CSharedMemBoxes" ; }
  HANDLE GetMutexHandle() { return m_hProtectMutex ; }
  int GetNBusyBoxes() 
  { 
    CAutoLockMutex_H al( m_hProtectMutex , 2000 ) ;
    if ( al.GetStatus() != WAIT_OBJECT_0 )
    {
      if ( al.GetStatus() == WAIT_TIMEOUT )
        return shMemBoxTimeout ;
      ASSERT( 0 ) ;
    }
    if ( !m_pArea )
      return shMemBoxNotMapped ;
    return ((ShMemBoxesHeader*)m_pArea)->m_iNBusyBoxes ;
  }
  void SetNBusyBoxes( int iNBusyBoxes ) 
  { 
    CAutoLockMutex_H al( m_hProtectMutex , 2000 ) ;
    if ( al.GetStatus() != WAIT_OBJECT_0 )
    {
      if ( al.GetStatus() == WAIT_TIMEOUT )
        return ;
      ASSERT( 0 ) ;
    }
    if ( !m_pArea )
      return ;
    ((ShMemBoxesHeader*)m_pArea)->m_iNBusyBoxes = iNBusyBoxes ;
  }
  int CopyBusyBoxes( void * pDest , int iNArraySize )
  {
    CAutoLockMutex_H al( m_hProtectMutex , 2000 ) ;
    if ( al.GetStatus() != WAIT_OBJECT_0 )
    {
      if ( al.GetStatus() == WAIT_TIMEOUT )
        return shMemBoxTimeout ;
      ASSERT( 0 ) ;
    }
    if ( !m_pArea )
      return shMemBoxNotMapped ;
    int iNBusyBoxes = ((ShMemBoxesHeader*)m_pArea)->m_iNBusyBoxes ;
    int iNNecessaryLength = GetBoxSize() * iNBusyBoxes ;
    if ( (iNArraySize * GetBoxSize()) < iNNecessaryLength )
      return -1000 - iNBusyBoxes ;
    memcpy( pDest , m_pArea + BOXES_BEGIN_AT , iNNecessaryLength ) ;
    return iNBusyBoxes ;
  }

protected:
  int m_iAreaSize ;
  BYTE * m_pArea ;
  HANDLE m_hFile ;
  HANDLE m_hMap ;
  HANDLE m_hProtectMutex ;
};

#endif // __SH_MEMORY_H__


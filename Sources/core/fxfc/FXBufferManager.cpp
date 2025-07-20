#include "stdafx.h"
#include "fxfc/FXBufferManager.h"


FXBufferManager::FXBufferManager( int iBufferSize , int iNBuffers , bool bPageBoundary , LPCTSTR pName )
{
  if ( Allocate( iBufferSize , iNBuffers , bPageBoundary ) )
  m_ManagerName = "Buffer Manager" ;
  m_ManagerName += pName ;
}


BOOL FXBufferManager::Allocate( int iBufferSize , int iNBuffers , bool bPageBoundary)
{
  RemoveAllBuffers() ;

  m_VectorAccessMutex.lock() ;

  m_bGetEnabled = false ;
  m_bPageBoundary = bPageBoundary ;
  for ( int i = 0 ; i < iNBuffers ; i++ )
  {
    LPBYTE pNewBuf = NULL ;
    if ( m_bPageBoundary )
    {
      pNewBuf = ( LPBYTE ) VirtualAlloc( NULL , iBufferSize , MEM_COMMIT , PAGE_READWRITE ) ;
      if ( pNewBuf )
        VirtualLock( pNewBuf , iBufferSize ) ;
    }
    else
      pNewBuf = new BYTE[ iBufferSize ] ;
    
    if ( pNewBuf )
    {
      m_Buffers.push_back( pNewBuf ) ;
      m_BusyFlags.push_back( false ) ;
    }
    else
    {
      m_VectorAccessMutex.unlock() ;
      MessageBox( NULL , "Can't allocate memory" , ( LPCTSTR ) m_ManagerName , IDCANCEL ) ;
      RemoveAllBuffers() ;
      m_iBufferSize = 0 ;
      return FALSE ;
    }
  }
  m_iBufferSize = iBufferSize ;
  m_iNextBufferIndex = 0 ;
  m_dAverageLifeTime = 0. ;
  m_VectorAccessMutex.unlock() ;
  return TRUE ;
}

FXBufferManager::~FXBufferManager()
{
  RemoveAllBuffers() ;
}
;
BOOL FXBufferManager::RemoveAllBuffers()
{
  m_VectorAccessMutex.lock() ;
  for ( size_t i = 0 ; i < m_Buffers.size() ; i++ )
  {
    if ( m_bPageBoundary )
    {
      VirtualUnlock( m_Buffers[ i ] , m_iBufferSize ) ;
      VirtualFree( m_Buffers[ i ] , 0 , MEM_RELEASE ) ;
    }
    else
      delete[] m_Buffers[ i ] ;
  }
  m_Buffers.clear() ;
  m_BusyFlags.clear();
  m_VectorAccessMutex.unlock() ;
  return TRUE ;
};

int FXBufferManager::GetBuffer()
{
  if ( !m_bGetEnabled )
    return -1 ;

  m_VectorAccessMutex.lock() ;
  BusyFlagAndBuf * pStruct = ( BusyFlagAndBuf * ) m_Buffers[m_iNextBufferIndex] ;
  int iInitialIndex = m_iNextBufferIndex ;
  int iIndex = m_iNextBufferIndex++ ;
  while ( m_BusyFlags[ iIndex ] && ( iInitialIndex != m_iNextBufferIndex) )
  {
    iIndex = ( ++iIndex % ( int ) m_Buffers.size() ) ;
    m_iNextBufferIndex = ( ++m_iNextBufferIndex % ( int ) m_Buffers.size() ) ;
  }

  if ( iInitialIndex != m_iNextBufferIndex ) // free buffer found
  {
    m_BusyFlags[ iIndex ] = true ;
    m_iNextBufferIndex = (iIndex + 1) % ( int ) m_Buffers.size() ;
  }
  else
    iIndex = -1 ;
  m_VectorAccessMutex.unlock() ;
  // next buffer is busy
  return iIndex ;
};
BOOL FXBufferManager::ReleaseBuffer( int iBufNumber )
{
  m_VectorAccessMutex.lock() ;
  if ( m_BusyFlags[ iBufNumber ] )
  {
    m_BusyFlags[ iBufNumber ] = false ;
    m_VectorAccessMutex.unlock() ;
    return TRUE ;
  }
  //ASSERT( 0 ) ;  // buffer was not busy
  m_VectorAccessMutex.unlock() ;
  return FALSE ;
};

BOOL FXBufferManager::ReleaseBuffer( LPVOID pBuffer )
{
  m_VectorAccessMutex.lock() ;
  for ( auto It = m_Buffers.begin() ; It < m_Buffers.end() ; It++ )
  {
    if ( *It == pBuffer )
    {
      if ( m_BusyFlags[ It - m_Buffers.begin() ] )
      {
        m_BusyFlags[ It - m_Buffers.begin() ] = false ;
        m_VectorAccessMutex.unlock() ;
        return TRUE ;
      }
      else
      {
        ASSERT( 0 ) ; // buffer was not busy
        break ;
      }
    }
  }
  ASSERT( 0 ) ; // No such buffer or buffer was not busy
  m_VectorAccessMutex.unlock() ;
  return FALSE ;
};

BOOL FXBufferManager::ReleaseLastAllocatedBuffer()
{
  m_VectorAccessMutex.lock() ;
  // m_iNextBufferIndex always points to next after allocated buffer. 
  // This next can be busy, but it's not important
  int iBufferIndex = ( m_iNextBufferIndex == 0 ) ? GetNBuffers() - 1 : m_iNextBufferIndex - 1 ;
  ASSERT( m_BusyFlags[ iBufferIndex ] ) ;
  m_BusyFlags[ iBufferIndex ] = false ;
  m_iNextBufferIndex = iBufferIndex ;
  m_VectorAccessMutex.unlock() ;
  return TRUE ;
};


LPBYTE FXBufferManager::GetBufferPtr( int iBufNumber )
{
  if ( m_BusyFlags[ iBufNumber ] )
    return (LPBYTE)m_Buffers[ iBufNumber ] ;
  else
    return NULL ;
};

int FXBufferManager::GetNBusyBuffers()
{
  int iNBusy = 0 ;
  m_VectorAccessMutex.lock() ;
  for ( auto It = m_BusyFlags.begin() ; It < m_BusyFlags.end() ; It++ )
    iNBusy += (int)( *It ) ;

  m_VectorAccessMutex.unlock() ;
  return iNBusy ;
};

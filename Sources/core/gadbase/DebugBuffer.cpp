#include "stdafx.h"
#include "gadgets/DebugBuffer.h"

DebugBuffer::DebugBuffer( size_t uiDepth , COutputConnector * pOutputConnector )
{
  m_uiNFramesLimit = uiDepth ;
  m_pOutput = pOutputConnector ;
}

DebugBuffer::~DebugBuffer()
{
  for ( size_t Cnt = 0 ; Cnt < m_Frames.size() ; Cnt++ )
    ((CDataFrame*)(m_Frames[ Cnt ]))->Release() ;

}

size_t DebugBuffer::AddFrame( const CDataFrame * pNewFrame )
{
  while ( m_Frames.size() >= m_uiNFramesLimit )
  {
    m_VectorLock.Lock() ;
    ( ( CDataFrame* ) ( m_Frames[ 0 ] ) )->Release() ;
    m_Frames.erase( m_Frames.begin() ) ;
    m_VectorLock.Unlock() ;
  }
  ((CDataFrame*)pNewFrame)->AddRef() ;
  m_VectorLock.Lock() ;
  m_Frames.push_back( pNewFrame ) ;
  m_VectorLock.Unlock() ;

  return m_Frames.size() ;
}

bool DebugBuffer::Flush( DWORD DelayBetweenFrames_ms , bool bDelete )
{
  bool bResult = true ;
  if ( bDelete )
  {
    while ( m_Frames.size() )
    {
      m_VectorLock.Lock() ;
      bResult |= PutFrame( m_pOutput , m_Frames.front() ) ;
      m_Frames.erase( m_Frames.begin() ) ;
      m_VectorLock.Unlock() ;
      Sleep( DelayBetweenFrames_ms ) ;
    }
  }
  while ( m_Frames.size() >= m_uiNFramesLimit )
  {
    m_VectorLock.Lock() ;
    bResult |= PutFrame( m_pOutput , m_Frames.front() ) ;
    m_VectorLock.Unlock() ;
    Sleep( DelayBetweenFrames_ms ) ;
  }
  return bResult ;
}

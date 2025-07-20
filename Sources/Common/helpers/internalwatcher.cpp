#include "stdafx.h"
#include "helpers\InternalWatcher.h"

CInternalWatcher::CInternalWatcher(void)
{
  m_bExit = false ;
  m_evHasData = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
}

CInternalWatcher::~CInternalWatcher(void)
{
  CloseHandle( m_evHasData ) ;
  CDataFrame * p ;
  while( m_InputQueue.GetQueueObject( p ) )
  {
    p->Release(p) ;
  }
}

int CInternalWatcher::DoJob()
{
  if ( m_bExit )
    return WR_CONTINUE ;

  CDataFrame * pDataFrame = NULL ;
  while ( WaitForSingleObject( m_evHasData , INFINITE) == WAIT_OBJECT_0 )
  {
    if (m_bExit)
      break ;
    while ( m_InputQueue.ItemsInQueue() )
    {
      CDataFrame * pDataFrame = NULL ;
      if ( m_InputQueue.GetQueueObject( pDataFrame ) && pDataFrame )
        ProcessData( pDataFrame ) ;
    }
  }
  m_bExit = false ;
  return WR_CONTINUE ;
}

int CInternalWatcher::ProcessData( CDataFrame * pDataFrame )
{
  if ( pDataFrame )
  {
    pDataFrame->Release( pDataFrame ) ;
    return TRUE ;
  }
  return FALSE ;
}

CInputListen::CInputListen( CGadget * pHost , FN_SENDINPUTDATA pFunc )
{
  m_pFnSendInputData = pFunc ;
  m_pHostGadget = pHost ;
  Create() ;
  Resume() ;
}

CInputListen::~CInputListen()
{
}

int CInputListen::ProcessData( CDataFrame * pDataFrame )
{
  if ( m_pFnSendInputData )
  {
    double dBegin = GetHRTickCount() ;
    m_pFnSendInputData( pDataFrame , m_pHostGadget , NULL ) ;
    m_dAccumulatedTime += dBegin ;
    return TRUE ;
  }
  return FALSE ;
}



CInputPinWithQueue::CInputPinWithQueue( 
  CGadget * pHost , FN_SENDINPUTDATA pFunc , datatype type ) 
  : CInputConnector( type , NULL , pHost )
  , m_bExit( false )
{
  m_pFnSendInputData = pFunc ;
  m_pHostGadget = pHost ;
  Create() ;
  Resume() ;
}

CInputPinWithQueue::~CInputPinWithQueue()
{
  while ( m_FramesQueue.ItemsInQueue() )
  {
    CDataFrame * pDataFrame = NULL ;
    if ( m_FramesQueue.GetQueueObject( pDataFrame ) && pDataFrame )
      pDataFrame->Release() ;
  }
}

int CInputPinWithQueue::DoJob()
{
  if ( m_bExit )
    return WR_CONTINUE ;

  while ( WaitForSingleObject( m_evHasData , INFINITE ) == WAIT_OBJECT_0 )
  {
    if ( m_bExit )
      break ;
    while ( m_FramesQueue.ItemsInQueue() )
    {
      CDataFrame * pDataFrame = NULL ;
      if ( m_FramesQueue.GetQueueObject( pDataFrame ) && pDataFrame )
        ProcessData( pDataFrame ) ;
    }
  }
  return WR_CONTINUE ;
}

int CInputPinWithQueue::ProcessData( CDataFrame * pDataFrame )
{
  if ( m_pFnSendInputData )
  {
    double dBegin = GetHRTickCount() ;
    m_pFnSendInputData( pDataFrame , m_pHostGadget , NULL ) ;
    m_dAccumulatedTime += dBegin ;
    return TRUE ;
  }
  else if ( pDataFrame )
    pDataFrame->Release() ;

  return FALSE ;
}

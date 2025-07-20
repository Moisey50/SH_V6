// SharedMemory.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <fxfc/fxfc.h>
#include "ShMemControl.h"
//#include "get_time.h"

//#define TRACE_SH_MEM_EXCHANGE
#define TRACE_INCREASE_TIMEOUT (30000)

CShMemControl::CShMemControl( int iSendOffset , int iReceiveOffset , 
  int iAreaSizeBytes , LPCTSTR pAreaName , LPCTSTR pInEventName , 
  LPCTSTR pOutEventName , LPCTSTR pStatusEventName ) 
  : CShMemoryBase( iAreaSizeBytes ,pAreaName , pInEventName , pOutEventName) 
{
  m_iSendOffset = iSendOffset ;
  m_iReceiveOffset = iReceiveOffset ;
};

int CShMemControl::SendAnswer( void * pMsg , int iMsgLen ) 
{
#ifdef TRACE_SH_MEM_EXCHANGE
  double dEntryTime = get_current_time() ;
#endif
  MSG_INFO * pMsgInfo = (MSG_INFO*)( m_pArea + m_iReceiveOffset ) ;
  int iRes = PutMsgToArea( m_iReceiveOffset , true , pMsg, iMsgLen) ;
  if ( iRes > 0 )
  {
    SHARED_MEMORY_HEADER * pH = (SHARED_MEMORY_HEADER*)m_pArea ;
    pH->iNAnswers++ ;
  }
//   else
//   {
//     ((MSG_INFO*)pMsg)->m_iId = iRes ;
//   }
#ifdef TRACE_SH_MEM_EXCHANGE
  TRACE( "SendRequest: hThr=0x%08X Res=%d Inside=%8.3f %s\n" , GetCurrentThreadId() , iRes , get_current_time() - dEntryTime , GetShMemAreaName() ) ;
#endif
  return iRes ;
};

int CShMemControl::SendRequest( void * pMsg , int iMsgLen ) 
{
#ifdef TRACE_SH_MEM_EXCHANGE
  double dEntryTime = get_current_time() ;
#endif

  MSG_INFO * pMsgInfo = (MSG_INFO*)( m_pArea + m_iSendOffset ) ;
  int iRes = PutMsgToArea( m_iSendOffset , false , pMsg , iMsgLen) ;
  if ( iRes > 0 )
  {
    SHARED_MEMORY_HEADER * pH = (SHARED_MEMORY_HEADER*)m_pArea ;
    pH->iNRequests++ ;
  }
//  else
//   {
//     ((MSG_INFO*)pMsg)->m_iId = iRes ;
//   }
#ifdef TRACE_SH_MEM_EXCHANGE
  TRACE( "SendRequest: hThr=0x%08X Res=%d Inside=%8.3f %s\n" , GetCurrentThreadId() , iRes , get_current_time() - dEntryTime , GetShMemAreaName() ) ;
#endif
  return iRes ;
};
int CShMemControl::ReceiveRequest( void * pBuf , int iBufLen ) 
{
#ifdef TRACE_SH_MEM_EXCHANGE
  double dEntryTime = get_current_time() ;
#endif
  int iResult = 0 ;
  MSG_INFO * pMsgInfo = (MSG_INFO*)( m_pArea + m_iSendOffset ) ;
  BYTE * pMsg = GetMsgFromArea( 
    m_iSendOffset , false , pBuf , &iBufLen ) ;
  if ( pMsg )
  {
    SHARED_MEMORY_HEADER * pH = (SHARED_MEMORY_HEADER*)m_pArea ;
    pH->iNTakenRequests++ ;
    iResult = ( pMsg == pBuf ) ? iBufLen : -(int)pMsg ; // error code
  }
#ifdef TRACE_SH_MEM_EXCHANGE
  TRACE( "ReceiveRequest: hThr=0x%08X Res=%d Inside=%8.3f %s\n" , GetCurrentThreadId() , iResult , get_current_time() - dEntryTime , GetShMemAreaName() ) ;
#endif
  return iResult ; // timeout
};
int CShMemControl::ReceiveAnswer( void * pBuf , int iBufLen ) 
{
#ifdef TRACE_SH_MEM_EXCHANGE
  double dEntryTime = get_current_time() ;
#endif
  int iResult = 0 ;
  MSG_INFO * pMsgInfo = (MSG_INFO*)( m_pArea + m_iReceiveOffset ) ;
  void * pMsg = GetMsgFromArea( 
    m_iReceiveOffset , true , pBuf , &iBufLen ) ;
  if ( pMsg )
  {
    SHARED_MEMORY_HEADER * pH = (SHARED_MEMORY_HEADER*)m_pArea ;
    pH->iNTakenAnswers++ ;
    iResult = ( pMsg == pBuf ) ? iBufLen : -(int)pMsg ; // error code
  }
#ifdef TRACE_SH_MEM_EXCHANGE
  TRACE( "ReceiveAnswer: hThr=0x%08X Res=%d Inside=%8.3f %s\n" , GetCurrentThreadId() , iResult , get_current_time() - dEntryTime , GetShMemAreaName() ) ;
#endif
  return iResult ; 
};
int CShMemControl::WaitAndReceiveAnswer( 
  void * pBuf , int iBufLen , int iTimeout_ms) 
{
#ifdef TRACE_SH_MEM_EXCHANGE
  double dEntryTime = get_current_time() ;
#endif
#ifdef TRACE_INCREASE_TIMEOUT
  iTimeout_ms = TRACE_INCREASE_TIMEOUT ;
#endif
  int iResult = 0 ;
  if ( WaitForEvent( m_hOutEvent , (m_bDebugMode) ? INFINITE : iTimeout_ms ) )
  {
    MSG_INFO * pMsgInfo = (MSG_INFO*)( m_pArea + m_iReceiveOffset ) ;
    void * pMsg = GetMsgFromArea( 
      m_iReceiveOffset , true , pBuf , &iBufLen ) ;
    if ( pMsg )
    {
      SHARED_MEMORY_HEADER * pH = (SHARED_MEMORY_HEADER*)m_pArea ;
      pH->iNTakenAnswers++ ;
      iResult = ( pMsg == pBuf ) ? iBufLen : -(int)pMsg ; // error code
    }
  }
#ifdef TRACE_SH_MEM_EXCHANGE
  TRACE( "WaitAndReceiveAnswer: hThr=0x%X Res=%d Inside=%8.3f %s\n" , GetCurrentThreadId() , iResult , get_current_time() - dEntryTime , GetShMemAreaName() ) ;
#endif
  return iResult ; // timeout
};
int CShMemControl::SendRqWaitAndReceiveAnswer( 
  void * pBuf , int iMsgLen , int iTimeout_ms , int iBufLen ) 
{
  m_Lock.Lock() ;
#ifdef TRACE_SH_MEM_EXCHANGE
  double dEntryTime = get_current_time() ;
  TRACE( "SendRqWaitAndReceiveAnswer: hThr=0x%X Entry                   %s\n" , GetCurrentThreadId() , GetShMemAreaName() ) ;
#endif
  int iResult = 0 ;
  ResetOutEvent() ;
  ResetInEvent() ;
  MSG_INFO * pMsgInfo = (MSG_INFO*)( m_pArea + m_iReceiveOffset ) ;
  pMsgInfo->m_iIsMessage = 0 ;
  if ( SendRequest( pBuf , iMsgLen ) > 0 ) 
  {
  iBufLen += 10 ;
    iResult = WaitAndReceiveAnswer( pBuf , iBufLen , iTimeout_ms ) ;
    if ( !iResult ) //timeout
    {
      TRACE( "SndRcv %s: hT=0x%X Can't Rcv\n" , GetShMemAreaName() , GetCurrentThreadId()  ) ;
    }
  }
  else
  {
    TRACE( "SndRcv %s: hT=0x%X Can't Send Rq\n" , GetShMemAreaName() , GetCurrentThreadId() ) ;
  }
#ifdef TRACE_SH_MEM_EXCHANGE
  TRACE( "SendRqWaitAndReceiveAnswer: hThr=0x%X Res=%d Inside=%8.3f %s\n" , GetCurrentThreadId() , iResult , get_current_time() - dEntryTime , GetShMemAreaName() ) ;
#endif
  m_Lock.Unlock() ;
  return iResult ;
};

int CShMemControl::IsReceivedMsg() 
{
  char Received = 0 ;
  if ( GetAreaContentDirect( m_iReceiveOffset , &Received , 1) )
  {
    if ( Received  &&  GetAreaContentDirect( m_iReceiveOffset + 1 , &Received , 1))
      return (Received > 0) ? Received : 0 ;
  }
  return 0 ;
}
bool CShMemControl::IsSentMessageProcessed()
{
  char Processed = 0 ;
  if ( GetAreaContentDirect( m_iSendOffset , &Processed , 1) )
    return (Processed == 0) ;
  return false ;
}

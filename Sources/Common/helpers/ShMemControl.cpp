// SharedMemory.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
//#include <fxfc/fxfc.h>
#include <tchar.h>
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
  InitializeCriticalSection( &m_CriticalSection ) ;
};

CShMemControl::~CShMemControl() 
{
  DeleteCriticalSection( &m_CriticalSection ) ;
} ;

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

// returns received request message length 
// if return value is zero , no message in shared memory
// if return value is -1, then shared memory is not initialized, 
int CShMemControl::GetRequestLength()
{
  if ( !m_pArea )
    return -1 ;

  MSG_INFO * pMsgInfo = ( MSG_INFO* ) ( m_pArea + m_iSendOffset ) ;
  if ( pMsgInfo->m_iIsMessage )
    return pMsgInfo->m_iMsgLen ;
  return 0 ;
};

// returns transfered message length 
// if return value is zero :
//   if iBufLen is zero - communication problem, 
//   if iBufLennot zero - too short buffer and iBufLen 
//      consist of necessary length 
//   and additional call with proper length should succeed
int CShMemControl::ReceiveRequest( void * pBuf , int& iBufLen ) 
{
#ifdef TRACE_SH_MEM_EXCHANGE
  double dEntryTime = get_current_time() ;
#endif
  int iResult = 0 ;
  MSG_INFO * pMsgInfo = (MSG_INFO*)( m_pArea + m_iSendOffset ) ;
  BYTE * pMsg = GetMsgFromArea( 
    m_iSendOffset , false , iBufLen , pBuf ) ;
  if ( pMsg )
  {
    SHARED_MEMORY_HEADER * pH = (SHARED_MEMORY_HEADER*)m_pArea ;
    pH->iNTakenRequests++ ;
    iResult = iBufLen ; // transfered message length
  }
#ifdef TRACE_SH_MEM_EXCHANGE
  TRACE( "ReceiveRequest: hThr=0x%08X Res=%d Inside=%8.3f %s\n" , GetCurrentThreadId() , iResult , get_current_time() - dEntryTime , GetShMemAreaName() ) ;
#endif
  return iResult ; // if iBufLen is zero - communication problem, 
                   // if iBufLennot zero - too short buffer and iBufLen 
                   //    consist of necessary length 
                   // and additional call with proper length should succeed
};

BOOL CShMemControl::MarkRequestAsRead()
{
  return MarkAsReceived( m_iSendOffset ) ;
}


// returns received request message length 
// if return value is zero , no message in shared memory
// if return value is -1, then shared memory is not initialized, 
int CShMemControl::GetAnswerLength()
{
  if ( !m_pArea )
    return -1 ;

  MSG_INFO * pMsgInfo = ( MSG_INFO* ) ( m_pArea + m_iReceiveOffset ) ;
  if ( pMsgInfo->m_iIsMessage )
    return pMsgInfo->m_iMsgLen ;
  return 0 ;
};

// returns transfered message length 
// if return value is zero :
//   if iBufLen is zero - communication problem, 
//   if iBufLennot zero - too short buffer and iBufLen 
//      consist of necessary length 
//   and additional call with proper length should succeed

int CShMemControl::ReceiveAnswer( void * pBuf , int& iBufLen ) 
{
#ifdef TRACE_SH_MEM_EXCHANGE
  double dEntryTime = get_current_time() ;
#endif
  int iResult = 0 ;
  MSG_INFO * pMsgInfo = (MSG_INFO*)( m_pArea + m_iReceiveOffset ) ;
  void * pMsg = GetMsgFromArea( 
    m_iReceiveOffset , true , iBufLen , pBuf ) ;
  if ( pMsg )
  {
    SHARED_MEMORY_HEADER * pH = (SHARED_MEMORY_HEADER*)m_pArea ;
    pH->iNTakenAnswers++ ;
    iResult = iBufLen ; // Real msg len
  }
#ifdef TRACE_SH_MEM_EXCHANGE
  TRACE( "ReceiveAnswer: hThr=0x%08X Res=%d Inside=%8.3f %s\n" , GetCurrentThreadId() , iResult , get_current_time() - dEntryTime , GetShMemAreaName() ) ;
#endif
  return iResult ; 
};

BOOL CShMemControl::MarkAnswerAsRead()
{
  return MarkAsReceived( m_iReceiveOffset ) ;
}
// WaitAndReceiveAnswer receives answer in iTimeout_ms
// returns received message length 
// if return value is zero :
//   if iBufLen == 0 - communication problem, 
//   if iBufLen > 0  - too short buffer and iBufLen 
//        consist of necessary length and additional 
//        call with proper length should succeed
//   if iBufLen == -1  - timeout

int CShMemControl::WaitAndReceiveAnswer( 
  void * pBuf , int& iBufLen , int iTimeout_ms) 
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
      m_iReceiveOffset , true , iBufLen , pBuf ) ;
    if ( pMsg )
    {
      SHARED_MEMORY_HEADER * pH = (SHARED_MEMORY_HEADER*)m_pArea ;
      pH->iNTakenAnswers++ ;
      iResult = iBufLen ;
    }
    return iResult ;
  }
#ifdef TRACE_SH_MEM_EXCHANGE
  TRACE( "WaitAndReceiveAnswer: hThr=0x%X Res=%d Inside=%8.3f %s\n" , GetCurrentThreadId() , iResult , get_current_time() - dEntryTime , GetShMemAreaName() ) ;
#endif
  iBufLen = -1 ; // sign of timeout
  return iResult ; 
};

// SendRqWaitAndReceiveAnswer sends request msg placed in pBuf
//    and receives answer in iTimeout_ms into the same buffer
//    iMsgLen is length of request message
//    iBufLen is buffer size on entry into program
//          and received message length on exit
// return value is received message length 
// if return value is zero then on exit as following:
//   if iBufLen == 0 - communication problem, 
//   if iBufLen > 0  - too short buffer and iBufLen 
//        consist of necessary length and additional 
//        call with proper length should succeed
//   if iBufLen == -1   - timeout on receive (too long processing)
//   if iBufLen == -2   - timeout on send (somebody another took channel)

int CShMemControl::SendRqWaitAndReceiveAnswer( 
  void * pBuf , int iMsgLen , DWORD dwTimeout_ms , int& iBufLen ) 
{
  DWORD dwSleepTime = 0 ;
  try
  {
    while ( !TryEnterCriticalSection( &m_CriticalSection ) )
    {
      if ( ++dwSleepTime > dwTimeout_ms )
      {
        iBufLen = -2 ;
        return 0 ;
      }
      Sleep( 1 ) ;
    };
  }
  catch (...)
  {
//     char * ErrBuf[ 4000 ] ;
//     e->GetErrorMessage( (LPTSTR)ErrBuf , 3999 ) ;
    TRACE( "ShMem ERROR: " ) ;
    return 0 ;
  }

#ifdef TRACE_SH_MEM_EXCHANGE
  double dEntryTime = get_current_time() ;
  TRACE( "SendRqWaitAndReceiveAnswer: hThr=0x%X Entry                   %s\n" , GetCurrentThreadId() , GetShMemAreaName() ) ;
#endif
  int iResult = 0 ;
  ResetOutEvent() ;
  ResetInEvent() ;
  MSG_INFO * pMsgInfo = (MSG_INFO*)( m_pArea + m_iSendOffset ) ;
  pMsgInfo->m_iIsMessage = 0 ;
  if ( SendRequest( pBuf , iMsgLen ) > 0 ) 
  {
    iBufLen += 10 ;
    iResult = WaitAndReceiveAnswer( pBuf , iBufLen , dwTimeout_ms ) ;
    if ( !iResult ) //timeout
    {
      TRACE( "SndRcv %s: hT=0x%X Can't Rcv\n" , ( char* ) GetShMemAreaName() , GetCurrentThreadId()  ) ;
    }
  }
  else
  {
    TRACE( "SndRcv %s: hT=0x%X Can't Send Rq\n" , ( char* ) GetShMemAreaName() , GetCurrentThreadId() ) ;
  }
#ifdef TRACE_SH_MEM_EXCHANGE
  TRACE( "SendRqWaitAndReceiveAnswer: hThr=0x%X Res=%d Inside=%8.3f %s\n" , GetCurrentThreadId() , iResult , get_current_time() - dEntryTime , GetShMemAreaName() ) ;
#endif
  //m_Lock.Unlock() ;
  LeaveCriticalSection( &m_CriticalSection ) ;
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

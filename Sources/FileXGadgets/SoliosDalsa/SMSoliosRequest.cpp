#include "stdafx.h"
#include "SMSoliosRequest.h"


CSMSoliosRequest::~CSMSoliosRequest(void)
{
  if ( m_hImageOutEvt )
  {
    CloseHandle( m_hImageOutEvt ) ;
    m_hImageOutEvt = NULL ;
  }
}

int CSMSoliosRequest::ProcessRequest( 
  CameraControlMessage& Msg , int iMsgLength , 
  int iTimeout_ms , int iBufLen )
{
  int iExchStatus = SendRqWaitAndReceiveAnswer( 
    &Msg ,iMsgLength , iTimeout_ms , sizeof(Msg) ) ;
  if ( iExchStatus > 0 )
    return iExchStatus ;

  _stprintf_s( Msg.m_szStringData , sizeof( Msg.m_szStringData ) , "ERROR %d %s" , 
    iExchStatus , (iExchStatus == 0) ? 
    "Timeout" : (iExchStatus == -4)? "Prev. request not finished " : "" ) ;
  return iExchStatus ;
}

void CSMSoliosRequest::SetOutImageEventName( LPCTSTR szImEventName )
{
  if ( m_hImageOutEvt )
  {
    CloseHandle( m_hImageOutEvt ) ;
    m_hImageOutEvt = NULL ;
  }
  if ( szImEventName )
  {
    m_hImageOutEvt = OpenEvent( EVENT_ALL_ACCESS , FALSE , szImEventName ) ;
    if ( !m_hImageOutEvt )
      m_hImageOutEvt = CreateEvent( NULL , FALSE , FALSE , szImEventName ) ;
  }
}

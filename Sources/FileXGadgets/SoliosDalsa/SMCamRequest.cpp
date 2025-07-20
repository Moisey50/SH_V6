#include "stdafx.h"
#include "SMCamRequest.h"

CString ImageExchStatus ;

CSMCamRequest::~CSMCamRequest(void)
{
}

int CSMCamRequest::ProcessRequest( 
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
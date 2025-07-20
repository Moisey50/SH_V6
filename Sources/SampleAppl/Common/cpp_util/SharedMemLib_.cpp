// SharedMemory.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "SharedMemLib.h"


int CSharedMemLib::SendMessage( const char * pMsg , int iMsgLen ) 
{
  SharedMessage Msg ;
  if (iMsgLen > sizeof(Msg.Message))
    return -1 ; // too long message
  if (IsSentMessageProcessed())
  {
    Msg.Control[0] = 1 ;
    Msg.Control[1] = (char)iMsgLen ;
    memcpy( Msg.Message , pMsg , iMsgLen ) ;
    return SetAreaContent( m_iSendOffset , (char*)&Msg , sizeof(Msg) ) ;
  }
  return -2 ; // previous message is not processed
};
int CSharedMemLib::ReceiveMessage( char * pBuf , int iBufLen ) 
{
  if (!IsReceivedMsg())
    return 0 ;
  SharedMessage Msg ;
  int iRcvdLen = GetAreaContentDirect( m_iReceiveOffset , (char*)&Msg , sizeof(Msg)) ;
  int iCopyLen = min(iBufLen,Msg.Control[1]) ;
  memcpy(pBuf , Msg.Message , iCopyLen ) ;
  Msg.Control[0] = 0 ;
  SetAreaContentDirect( m_iReceiveOffset , Msg.Control , 1 ) ;
  return iCopyLen ;
};
int CSharedMemLib::IsReceivedMsg() 
{
  char Received = 0 ;
  if ( GetAreaContentDirect( m_iReceiveOffset , &Received , 1) )
  {
    if ( Received  &&  GetAreaContentDirect( m_iReceiveOffset + 1 , &Received , 1))
      return (Received > 0) ? Received : 0 ;
  }
  return 0 ;
}
bool CSharedMemLib::IsSentMessageProcessed()
{
  char Processed = 0 ;
  if ( GetAreaContentDirect( m_iSendOffset , &Processed , 1) )
    return (Processed == 0) ;
  return false ;
}



int CSharedMemLib::PutMessage(const char *  pMsg, int iMsgLen)
{
  SharedMessage Msg ;
  if (iMsgLen > sizeof(Msg.Message))
    return -1 ; // too long message
  Msg.Control[0] = 1 ;
  Msg.Control[1] = (char)iMsgLen ;
  memcpy( Msg.Message , pMsg , iMsgLen ) ;
  return SetAreaContentDirect( m_iSendOffset , (char*)&Msg , sizeof(Msg) ) ;
}

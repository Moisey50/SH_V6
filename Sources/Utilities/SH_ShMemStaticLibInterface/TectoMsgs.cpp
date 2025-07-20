#include "stdafx.h"
#include "TectoMsgs.h"

TectoMsg * TectoMsg::Copy()
{
  TectoMsg * pNew = CreateMsg( m_Id , m_MsgDataLen ) ;
  memcpy( pNew->m_MsgData , m_MsgData , m_MsgDataLen ) ;
  return pNew ;
}

TectoMsg * TectoMsg::CreateMsg( TectoMsgId Id , DWORD dwDataLen )
{
  DWORD dwMsgSize = /*sizeof( m_Id ) + sizeof( m_MsgDataLen )*/ 8 + dwDataLen ;
  if ( dwMsgSize > ((MAX_MSG_SIZE_MB * 1024 * 1024) - 2048) )
  {
    char Msg[ 1024 ] ;
    sprintf( Msg , "Too big data %u bytes" , dwMsgSize ) ;
    MessageBox( NULL , Msg , "Tecto IP Interface LIB" , IDCANCEL ) ;
    return NULL ;
  }
  TectoMsg * pMsg = (TectoMsg*)(new BYTE[ dwMsgSize ]) ;

  if ( pMsg )
  {
    pMsg->m_Id = Id ;
    pMsg->m_MsgDataLen = dwDataLen ;
    if ( dwDataLen )
      pMsg->m_MsgData[ 0 ] = 0 ;
    return pMsg ;
  }
  return NULL ;
}
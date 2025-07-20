#include "stdafx.h"
#include "SHMemGadget.h"
#include "fxfc/FXRegistry.h"
#include <gadgets/datachain.h>

USER_FILTER_RUNTIME_GADGET( ShMemComm , "Communication" );

void ReceiveFromShMemFunc( ShMemComm * pGadget )
{
  TRACE( "\nReceiveFromShMemFunc started" ) ;
  HANDLE hEvent = ( pGadget->m_GadgetMode == SHMM_Server ) ?
    pGadget->m_pShMem->GetInEventHandle() : pGadget->m_pShMem->GetOutEventHandle() ;
  while ( !pGadget->m_bFinishReceiving )
  {
    DWORD Res = WaitForSingleObject( hEvent , 50 ) ;
    if ( pGadget->m_bFinishReceiving )
      break ;
    switch ( Res )
    {
      case WAIT_OBJECT_0:
        pGadget->ProcessDataFromShMem() ;
        break ;
      case WAIT_TIMEOUT: break ;
      default: pGadget->ProcessErrorFromShMem( Res ) ; break ;
    }
  }
  TRACE( "\nReceiveFromShMemFunc exit" ) ;
}



ShMemComm::ShMemComm()
{
  m_OutputMode = modeReplace ;
//   m_GadgetMode = SHMM_Server ;
  init() ;
  m_pShMem = NULL ;
  m_pReceiveThread = NULL ;
  m_bFinishReceiving = false ;
  m_iShMemInSize_KB = 2 ;
  m_iShMemOutSize_KB = 2 ;
}


ShMemComm::~ShMemComm()
{
  CheckAndDeleteReceiveThread() ;
  if ( m_pShMem )
  {
    delete m_pShMem ;
    m_pShMem = NULL ;
  }
}

bool ShMemComm::CheckAndDeleteReceiveThread()
{
  if ( m_pReceiveThread )
  {
    m_bFinishReceiving = true ;
    SetEvent( ( m_GadgetMode == SHMM_Client ) ?
      m_pShMem->GetInEventHandle() : m_pShMem->GetOutEventHandle() ) ;
    m_pReceiveThread->join() ;
    delete m_pReceiveThread ;
    m_pReceiveThread = NULL ;
    m_bFinishReceiving = false ; //for next thread creation
    return true ;
  }
  return false ;
}
static const char * pGadgetMode = "Server;Client;TectoServer;TectoSImulator;" ;

void ShMemComm::PropertiesRegistration()
{
  addProperty( SProperty::COMBO , _T( "GadgetMode" ) , ( int * ) &m_GadgetMode ,
    SProperty::Int , pGadgetMode ) ;
  addProperty( SProperty::SPIN , _T( "ShMemInSize_KB" ) , &m_iShMemInSize_KB ,
    SProperty::Int , 1 , 16384 ) ;
  addProperty( SProperty::SPIN , _T( "ShMemOutSize_KB" ) , &m_iShMemOutSize_KB ,
    SProperty::Int , 1 , 16384 ) ;

  addProperty( SProperty::EDITBOX , "Area_Name" , &m_ShMemAreaName , SProperty::String ) ;
  addProperty( SProperty::EDITBOX , "In_Event_Name" , &m_InEventName , SProperty::String ) ;
  addProperty( SProperty::EDITBOX , "Out_Event_Name" , &m_OutEventName , SProperty::String ) ;
//   addProperty( SProperty::EDITBOX , "Status Event Name" , &m_StatusEventName , SProperty::String ) ;

  SetInvalidateOnChange( _T( "GadgetMode" ) , true ) ;
  SetInvalidateOnChange( _T( "Area_Name" ) , true ) ;
  SetInvalidateOnChange( _T( "In_Event_Name" ) , true ) ;
  SetInvalidateOnChange( _T( "Out_Event_Name" ) , true ) ;

  if ( !m_ShMemAreaName.IsEmpty() && !m_InEventName.IsEmpty() && !m_OutEventName.IsEmpty() )
  {
    CheckAndDeleteReceiveThread() ;
    if ( m_pShMem )
    {
      delete m_pShMem ;
      m_pShMem = NULL ;
    }

    m_pShMem = new CShMemControl( 1024 , 1024 + m_iShMemOutSize_KB * 1024 , 
      1024 + (1024 * (m_iShMemInSize_KB + m_iShMemOutSize_KB)) , m_ShMemAreaName ,
      m_InEventName , m_OutEventName ) ;

    if ( !m_pShMem )
    {
      SEND_GADGET_ERR( "Can't allocate shared memory. Size In=%dKB Out=%dKB" , m_iShMemInSize_KB , m_iShMemOutSize_KB ) ;
    }

    m_bFinishReceiving = false ;
    m_pReceiveThread = new std::thread( ReceiveFromShMemFunc , this ) ;
  }
};

void ShMemComm::ConnectorsRegistration()
{
  addInputConnector( transparent , "DataInput" );
  addOutputConnector( transparent , "DataOutput" );
  addOutputConnector( text , "Diagnostics" ) ;
};

// Function simply takes input frames, provides serialization to output
// memory area and sets output event
CDataFrame* ShMemComm::DoProcessing( const CDataFrame* pDataFrame )
{
  double dStart = GetHRTickCount() ;

  //#ifdef _DEBUG
  FXString DiagInfo ;
  //#endif
  CDataChain dc( this );
  FLWDataFrame* df = dc.Serialize( pDataFrame );
  if ( df )
  {
    UINT iSendSize = ( df->uSize + sizeof( MSG_INFO ) ) ;
    int iRes = 0 ;
    switch ( m_GadgetMode )
    {
      case SHMM_Client:
      {
        if ( iSendSize < 1024 * m_iShMemOutSize_KB )
          iRes = m_pShMem->SendRequest( df , df->uSize ) ;
        else
          SEND_GADGET_ERR( "Can't send request data %d (limit is %d)" , iSendSize , 1024 * m_iShMemOutSize_KB ) ;
      }
      break ;
      case SHMM_Server:
      {
        if ( iSendSize < 1024 * m_iShMemInSize_KB )
          iRes = m_pShMem->SendAnswer( df , df->uSize ) ;
        else
          SEND_GADGET_ERR( "Can't send answer data %d (limit is %d)" , iSendSize , 1024 * m_iShMemOutSize_KB ) ;
      }
      break ;
    }
    delete df;
    if ( 0 < iRes )
    {
      TRACE( "\n ShMemComm::DoProcessing - sent %d bytes" ) ;
    }
    else // Not Sent
      SEND_GADGET_ERR( "Error on message send %s" , m_pShMem->GetErrorString( iRes ) ) ;
  }
  return NULL  ;
}

void ShMemComm::ProcessDataFromShMem()
{
  if ( m_bStopWhenGraphStoped )
  {
    if ( !m_pStatus )
      return ;
    switch ( m_pStatus->GetStatus() )
    {
      case CExecutionStatus::STOP:
        switch( m_GadgetMode )
        {
          case SHMM_Client: m_pShMem->MarkAnswerAsRead() ; break ;
          case SHMM_Server: m_pShMem->MarkRequestAsRead() ; break ;

        }
        return ;
      case CExecutionStatus::PAUSE:
      {
        HANDLE pEvents[] = { m_evExit , m_pStatus->GetPauseHandle() , m_pStatus->GetStpFwdHandle() };
        DWORD cEvents = sizeof( pEvents ) / sizeof( HANDLE );
        DWORD retVal = ::WaitForMultipleObjects( cEvents , pEvents , FALSE , INFINITE );
        if ( retVal != 2 )
          return ;
      }
    }
  }

  int iRes = 0 ;
  switch ( m_GadgetMode )
  {
    case SHMM_Client:
    {
      int iLen = m_pShMem->GetAnswerLength() ;
      if ( iLen > 0 )
      {
        int iBufLen = iLen + 20 ;
        BYTE * pBuf = new BYTE[ iBufLen ] ;
        iRes = m_pShMem->ReceiveAnswer( pBuf , iBufLen ) ;
        if ( iRes > 0 )
        {
          CDataChain dc( this );
          CDataFrame* df = dc.Restore( ( FLWDataFrame* ) pBuf );
          if ( df )
            PutFrame( GetOutputConnector( 0 ) , df ) ;
        }
        delete[] pBuf;
      }
      else
        SEND_GADGET_ERR( "ShMemComm::ProcessDataFromShMem - Error %d on answer data receive" , iRes ) ;
    }
    break ;
    case SHMM_Server:
    {
      int iLen = m_pShMem->GetRequestLength() ;
      if ( iLen > 0 )
      {
        int iBufLen = iLen + 20 ;
        BYTE * pBuf = new BYTE[ iBufLen ] ;
        iRes = m_pShMem->ReceiveRequest( pBuf , iBufLen ) ;
        if ( iRes > 0 )
        {
          CDataChain dc( this );
          CDataFrame* df = dc.Restore( ( FLWDataFrame* ) pBuf );
          if ( df )
            PutFrame( GetOutputConnector( 0 ) , df ) ;
        }
        delete[] pBuf;
      }
      else
        SEND_GADGET_ERR( "ShMemComm::ProcessDataFromShMem - Error %d on request data receive" , iRes ) ;
    }
    break ;
    default: SEND_GADGET_ERR( "ShMemComm::ProcessDataFromShMem - Unknown Gadget Mode %d" , m_GadgetMode ) ;

  }
}

void ShMemComm::ProcessErrorFromShMem( DWORD ErrCode )
{
  SEND_GADGET_ERR( "Error %d on ShMem data waiting" , ErrCode ) ;
}

void ShMemComm::Logger( LPCTSTR pLogString )
{
  PutFrame( GetOutputConnector( 1 ) , CreateTextFrame( pLogString , "LogMsg" ) ) ;
}


// NWSocket.cpp: implementation of the NWSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Network.h"
#include "NWSocket.h"
#include "TCPClient.h"
#include "TCPServer.h"
#include <gadgets\textframe.h>
#include <gadgets\DataChain.h>

#define THIS_MODULENAME "NWSocket"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_RUNTIME_GADGET_EX( NWSocket , CCaptureGadget , LINEAGE_COMM , TVDB400_PLUGIN_NAME )

bool FAR __stdcall OnNWReceive( LPVOID usrData , pDataBurst data )
{
  return ((NWSocket*) usrData)->OnReceive( data );
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

NWSocket::NWSocket() :
  m_Mode( MODE_CLIENT ) ,
  m_Client( NULL ) ,
  m_Server( NULL ) ,
  m_ServerAddress( "127.0.0.1" ) ,
  m_ServicePort( DEFAULT_SERVICE_PORT ) ,
  m_ID( 0 ) ,
  m_StopWhenGraphStoped( true ) ,
  m_bSetRegistered( FALSE ) ,
  m_bNotSHClient( FALSE )
{
  m_pInput = new CInputConnector( transparent );
  m_pOutput = new COutputConnector( transparent );
  Resume();
}

void NWSocket::ShutDown()
{
  CCaptureGadget::ShutDown();
  if ( m_pInput ) delete m_pInput;
  m_pInput = NULL;
  if ( m_pOutput ) delete m_pOutput;
  m_pOutput = NULL;
  switch ( m_Mode )
  {
  case MODE_CLIENT:
    if ( m_Client )
    {
      m_Client->Disconnect();
      Sleep( 50 ) ;
      delete m_Client;
    }
    m_Client = NULL;
    break;
  case MODE_SERVER:
    if ( m_Server )
    {
      m_Server->Stop();
      Sleep( 50 ) ;
      delete m_Server;
    }
    m_Server = NULL;
    break;
  }
}

int NWSocket::DoJob()
{
  CDataFrame* pDataFrame = NULL;
  if ( (m_pInput) && (m_pInput->Get( pDataFrame )) )
  {
    ASSERT( pDataFrame );
    if ( (m_pOutput) && (Tvdb400_IsEOS( pDataFrame )) ) // if EOS - just pass dataframe through without processing for common types gadgets
    {
      if ( !m_pOutput->Put( pDataFrame ) )
        pDataFrame->Release();
    }
    else
    {
      double ts = GetHRTickCount();
      CDataFrame* pResultFrame = NULL;
      if ( !m_Invalid )
      {
#if (!defined(NO_CATCH_EXCEPTION_IN_DEBUG) || !defined(_DEBUG))
        __try
#else
#pragma message(__FILE__"[" STRING(__LINE__) "] : warning HB0001: The exception catching is disabled!")
#endif
        {
          pResultFrame = DoProcessing( pDataFrame );
          AddCPUUsage( GetHRTickCount() - ts );
          if ( pResultFrame )
          {
            if ( (m_pOutput) && (!m_pOutput->Put( pResultFrame )) )
              pResultFrame->RELEASE( pResultFrame );
          }
        }
#if (!defined(NO_CATCH_EXCEPTION_IN_DEBUG) || !defined(_DEBUG))
        __except ( 1 )
        {
          {
            SENDERR_2( "First-chance exception 0x%x in gadget '%s'" , GetExceptionCode() , m_Name );
            m_Invalid = true;
          }
        }
#endif
      }
      pDataFrame->Release();
    }
  }
  return WR_CONTINUE;
}

int NWSocket::GetInputsCount()
{
  return (m_pInput) ? 1 : 0;
}

CInputConnector* NWSocket::GetInputConnector( int n )
{
  return (!n) ? m_pInput : NULL;
}


bool    NWSocket::ScanSettings( FXString& text )
{
  //,Spin(Port,0,32767)
  switch ( m_Mode )
  {
  case MODE_CLIENT:
    text.Format( "template(ComboBox(Mode(Server(%d),Client(%d))),"
      "EditBox(ServerAddress),Spin(Port,0,65534),"
      "ComboBox(DataType(Text(%d),DataPackets(%d),RAWText(%d),RAWTextSHServer(%d))),"
      "ComboBox(StopWhenGraphStoped(true(true),false(false)))"
      "ComboBox(SetRegistered(No(0),Yes(1))))" ,
      MODE_SERVER , MODE_CLIENT , TYPE_TEXT , TYPE_DATAPACKETS ,
      TYPE_RAWTEXT , TYPE_RAWTEXTSHSERV );
    break;
  case MODE_SERVER:
    text.Format( "template(ComboBox(Mode(Server(%d),Client(%d))),"
      "Spin(Port,0,65534),"
      "ComboBox(StopWhenGraphStoped(true(true),false(false))),"
      "ComboBox(SetRegistered(No(0),Yes(1))),"
      "ComboBox(NotSHClient(No(0),Yes(1))))" ,
      MODE_SERVER , MODE_CLIENT );
    break;
  }
  return true;
}

bool    NWSocket::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  CCaptureGadget::ScanProperties( text , Invalidate );
  FXPropertyKit pk( text );
  int mode = m_Mode;
  pk.GetInt( "Mode" , mode );
  pk.GetInt( "Port" , m_ServicePort );
  pk.GetBool( "StopWhenGraphStoped" ,
    m_StopWhenGraphStoped );
  pk.GetInt( "SetRegistered" , m_bSetRegistered ) ;
  pk.GetInt( "NotSHClient" , m_bNotSHClient ) ;
  if ( mode != m_Mode )
  {
    switch ( m_Mode )
    {
    case MODE_CLIENT:
      if ( m_Client )
        delete m_Client;
      m_Client = NULL;
      break;
    case MODE_SERVER:
      m_Server->Stop();
      if ( m_Server )
        delete m_Server;
      m_Server = NULL;
      break;
    }
    Invalidate = true;
  }
  switch ( mode )
  {
  case MODE_CLIENT:
    {
      if ( !m_Client )
        m_Client = new CTCPClient( OnNWReceive , this );
      pk.GetString( "ServerAddress" , m_ServerAddress );
      int dType = GetDataType();
      pk.GetInt( "DataType" , dType );
      m_Client->SetServerAddress( m_ServerAddress , m_ServicePort , dType );
      m_Client->Connect();
      m_Mode = mode;
      break;
    }
  case MODE_SERVER:
    if ( !m_Server )
      m_Server = new CTCPServer( OnNWReceive , this );
    m_Server->SetNoSHMode( m_bNotSHClient ) ;
    m_Server->SetServicePort( m_ServicePort );
    m_Server->Start();
    m_Mode = mode;
    break;
  }
  return true;
}

bool    NWSocket::PrintProperties( FXString& text )
{
  FXPropertyKit pk;
  CCaptureGadget::PrintProperties( text );
  pk.WriteInt( "Mode" , m_Mode );
  pk.WriteInt( "Port" , m_ServicePort );
  pk.WriteBool( "StopWhenGraphStoped" ,
    m_StopWhenGraphStoped );
  pk.WriteInt( "SetRegistered" , m_bSetRegistered ) ;
  if ( m_Mode == MODE_CLIENT )
  {
    pk.WriteString( "ServerAddress" , m_ServerAddress );
    pk.WriteInt( "DataType" , GetDataType() );
  }
  else
    pk.WriteInt( "NotSHClient" , m_bNotSHClient ) ;
  text += pk;
  return true;
}

__forceinline pDataBurst packData( const CDataFrame* pDataFrame )
{
  pDataBurst retV = NULL;
  CString ForTx;
  const CTextFrame* txtFrame = pDataFrame->GetTextFrame( DEFAULT_LABEL );
  if ( txtFrame )
  {
    CString tmpS = txtFrame->GetString();
    int iLen = tmpS.GetLength() ;
    retV = (pDataBurst) malloc( sizeof( DataBurst ) + iLen );
    retV->dwDataSize = tmpS.GetLength() + 1;
    memcpy( retV->lpData , (LPCTSTR) tmpS , iLen );
    retV->lpData[ iLen ] = 0 ;
  }
  return retV;
}

CDataFrame* NWSocket::DoProcessing( const CDataFrame* pDataFrame )
{
  if ( GetDataType() == TYPE_TEXT )
  {
    pDataBurst pDB = packData( pDataFrame );
    if ( pDB )
    {
      switch ( m_Mode )
      {
      case MODE_CLIENT:
        if ( m_Client )
        {
          if ( !m_Client->Send( pDB ) )
            PrintMessage( m_Client->GetStatusString() );
        }
        break;
      case MODE_SERVER:
        if ( m_Server )
        {
          if ( !m_Server->Send( pDB ) )
            PrintMessage( m_Server->GetStatusString() );
        }
        break;
      }
      free( pDB );
    }
  }
  else if ( (GetDataType() == TYPE_DATAPACKETS) && (pDataFrame) )
  {
    CDataChain dc( this );
    FLWDataFrame* df = dc.Serialize( pDataFrame );
    if ( df )
    {
      pDataBurst pDB = (pDataBurst) malloc( sizeof( DataBurst ) + (UINT) (df->uSize) );
      if ( pDB )
      {
        pDB->dwDataSize = (DWORD) df->uSize;
        memcpy( pDB->lpData , df , (UINT) (df->uSize) );
        delete df;
        switch ( m_Mode )
        {
        case MODE_CLIENT:
          if ( m_Client )
          {
            if ( !m_Client->Send( pDB ) )
              PrintMessage( m_Client->GetStatusString() );
          }
          break;
        case MODE_SERVER:
          if ( m_Server )
          {
            if ( !m_Server->Send( pDB ) )
              PrintMessage( m_Server->GetStatusString() );
          }
          break;
        }
        free( pDB );
      }
    }
  }
  else   if ( (GetDataType() == TYPE_RAWTEXT)
    || (GetDataType() == TYPE_RAWTEXTSHSERV)
    && pDataFrame )
  {
    const CTextFrame * pText = pDataFrame->GetTextFrame() ;
    if ( pText )
    {
      LPCTSTR pDB = (LPCTSTR) pText->GetString() ;
      DWORD dwLen = (DWORD) pText->GetString().GetLength() ;
      if ( pDB )
      {
        switch ( m_Mode )
        {
        case MODE_CLIENT:
          if ( m_Client )
          {
            if ( !((CTCPClient*) m_Client)->SendBuffer( (LPBYTE) pDB , dwLen ) )
              PrintMessage( m_Client->GetStatusString() );
          }
          break;
        case MODE_SERVER:
          if ( m_Server )
          {
            if ( !((CTCPServer*) m_Server)->SendBuffer( (LPBYTE) pDB , dwLen ) )
              PrintMessage( m_Server->GetStatusString() );
          }
          break;
        }
      }
    }

  }

  return NULL;
}

bool NWSocket::OnReceive( pDataBurst data )
{
  if ( m_StopWhenGraphStoped )
  {
    if ( !m_pStatus )
      return true ;
    switch ( m_pStatus->GetStatus() )
    {
    case CExecutionStatus::STOP:
      return true;
    case CExecutionStatus::PAUSE:
      {
        HANDLE pEvents[] = { m_evExit , m_pStatus->GetPauseHandle() , m_pStatus->GetStpFwdHandle() };
        DWORD cEvents = sizeof( pEvents ) / sizeof( HANDLE );
        DWORD retVal = ::WaitForMultipleObjects( cEvents , pEvents , FALSE , INFINITE );
        if ( retVal != 2 )
          return true;
      }
    }
  }
  if ( data->dwDataSize )
  {
    if ( GetDataType() == TYPE_TEXT
      || GetDataType() == TYPE_RAWTEXT
      || GetDataType() == TYPE_RAWTEXTSHSERV )
    {
      CTextFrame* tf = CTextFrame::Create( (LPCTSTR) (data->lpData) );
      tf->SetTime( GetGraphTime() * 1.e-3 );
      tf->ChangeId( m_ID ); m_ID++;
      if ( m_bSetRegistered )
        tf->SetRegistered() ;
      if ( (!m_pOutput) || (!m_pOutput->Put( tf )) )
        tf->RELEASE( tf );
    }
    else if ( GetDataType() == TYPE_DATAPACKETS )
    {
      CDataChain dc( this );
      CDataFrame* df = dc.Restore( (FLWDataFrame*) data->lpData );
      if ( (!m_pOutput) || (!m_pOutput->Put( df )) )
      {
        df->RELEASE( df );
      }
    }
  }
  return true;
}

void NWSocket::PrintMessage( LPCTSTR mes )
{
  if ( m_LastShownMessage != mes )
  {
    m_LastShownMessage = mes;
    SENDERR_0( mes );
  }
}

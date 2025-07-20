// TCPClient.cpp: implementation of the CTCPClient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Network.h"
#include "TCPClient.h"
#include <networks\WSAErr2Mes.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#pragma comment(lib,"Ws2_32.lib")

UINT ClientSocketThread( LPVOID lpData )
{
  CTCPClient* hst = ( CTCPClient* )lpData;
  if ( !hst->m_ShutDown )
  {
    ASSERT( hst->m_Sock == INVALID_SOCKET );
    hst->m_Sock = socket( AF_INET , SOCK_STREAM , 0 );
    if ( INVALID_SOCKET == hst->m_Sock )
    {
      hst->m_Status.Format( "CTCPClient socket : socket error: \"%s\"" , LastWSAErr2Mes( WSAGetLastError() ) );
      TRACE( hst->m_Status );
      goto shutdown;
    }
    while ( !hst->m_ShutDown )
    {
      if ( connect( hst->m_Sock , ( PSOCKADDR )&hst->m_SDest , sizeof( hst->m_SDest ) ) == SOCKET_ERROR )
      {
        Sleep( 100 );
        hst->m_Status.Format( "Can't connect to server, error: \"%s\"" , LastWSAErr2Mes( WSAGetLastError() ) );
        if ( WSAGetLastError() == WSAEISCONN )
        {
          shutdown( hst->m_Sock , SD_BOTH );
          closesocket( hst->m_Sock );
          hst->m_Sock = INVALID_SOCKET;
          hst->m_Sock = socket( AF_INET , SOCK_STREAM , 0 );
          //hst->m_ShutDown=true;
          //hst->m_SocketThread=NULL;
          continue;
        }
        continue;
      }
      if ( hst->m_DataType != TYPE_RAWTEXT )
      {
        if ( send( hst->m_Sock , hst->m_ClientName , hst->m_ClientName.GetLength() + 1 , 0 ) == SOCKET_ERROR )
        {
          Sleep( 10 );
          hst->m_Status.Format( "CTCPClient send: socket error: \"%s\"" , LastWSAErr2Mes( WSAGetLastError() ) );
          TRACE( hst->m_Status );
          Sleep( 10 );
          break;
        }
        int iRet;
        char answer[ 124 ];
        iRet = recv( hst->m_Sock , answer , 124 , 0 );
        if ( ( iRet == 0 ) || ( iRet == SOCKET_ERROR ) )
        {
          hst->m_Status.Format( "Can't connect to server, error: \"%s\"" , LastWSAErr2Mes( WSAGetLastError() ) );
          TRACE( hst->m_Status );
          break;
        }
      }

      hst->m_Connected = true;
      hst->m_Status = "Client connected";
      hst->ReceiveData();
    }
  }
shutdown:
  closesocket( hst->m_Sock ); hst->m_Sock = INVALID_SOCKET;
  hst->m_Status.Format( "Connection closed\n" );
  TRACE( hst->m_Status );
  return 0;
}

bool CTCPClient::ReceiveData()
{
  DWORD iRet;
  while ( !m_ShutDown )
  {
    if ( m_DataType != TYPE_RAWTEXT )
    {
      DataBurst *db = NULL;
      DWORD      ds;
      iRet = reciveall( m_Sock , ( char* )&ds , sizeof( ds ) , 0 );
      if ( iRet == SOCKET_ERROR )
      {
        m_Status.Format( "Can't connect to client, error: \"%s\"" , LastWSAErr2Mes( WSAGetLastError() ) );
        TRACE( m_Status );
        return true;
      }
      else if ( iRet != sizeof( DWORD ) )
      {
        m_Status = "Connection terminated suddenly";
        TRACE( m_Status );
        return true;
      }
      else
      {
        if ( ds != 0 )
        {
          db = ( DataBurst* )malloc( ds + sizeof( DataBurst ) );
          if ( !db )
            continue;
          db->dwDataSize = ds;
          iRet = reciveall( m_Sock , ( char* )db->lpData , db->dwDataSize , 0 );
          if ( db->dwDataSize != iRet )
          {
            free( db ); db = NULL;
            continue;
          }
        }
        OnReceive( db );
      }
    }
    else
    {
      DataBurst *db = NULL;
      BYTE      bBuffer[ 4000 ] ;
      iRet = recv( m_Sock , ( char* )bBuffer , sizeof( bBuffer ) , 0 );
      if ( iRet == SOCKET_ERROR )
      {
        m_Status.Format( "Can't connect to client, error: \"%s\"" , LastWSAErr2Mes( WSAGetLastError() ) );
        TRACE( m_Status );
        return true;
      }
      db = ( DataBurst* ) new BYTE[ sizeof( db->dwDataSize ) + iRet + 1] ;
      db->dwDataSize = iRet ;
      memcpy( db->lpData , bBuffer , iRet ) ;
      db->lpData[ iRet ] = 0 ;
      OnReceive( db );
    }
  }
  return true;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTCPClient::CTCPClient( NWDataCallback pDataCallback , LPVOID usrData ) :
CClient( pDataCallback , usrData ) ,
m_ShutDown( false ) ,
m_Sock( INVALID_SOCKET ) ,
m_SocketThread( NULL ) ,
m_Connected( false )
{
  m_ClientName = CONNECTION_REQUEST_TEXT;
  m_Status = "Connection closed";
  INT iRetVal;
  iRetVal = WSAStartup( MAKEWORD( 2 , 0 ) , &m_wsaData );
  if ( 0 != iRetVal )
  {
    m_Status.Format( "WSAStartup error: %d\n" , iRetVal );
    TRACE( m_Status );
  }
}

CTCPClient::~CTCPClient()
{
  Disconnect();
  WSACleanup();
}

void CTCPClient::Connect()
{
  m_ShutDown = false;
  if ( !m_SocketThread )
  {
    m_SocketThread = AfxBeginThread( ClientSocketThread ,
      this , THREAD_PRIORITY_TIME_CRITICAL , 0 , CREATE_SUSPENDED , NULL );
    if ( m_SocketThread )
    {
      m_SocketThread->m_bAutoDelete = TRUE;
      m_SocketThread->ResumeThread();
    }
  }
}

void CTCPClient::Disconnect()
{
  m_ShutDown = true;
  shutdown( m_Sock , SD_BOTH );
  if ( ( m_SocketThread ) && ( m_Sock != INVALID_SOCKET ) )
    ::WaitForSingleObject( m_SocketThread->m_hThread , INFINITE );
  m_Sock = INVALID_SOCKET;
  m_SocketThread = NULL;
}

bool CTCPClient::IsConnected()
{
  return m_SocketThread != NULL;;
}

bool CTCPClient::SetServerAddress( LPCTSTR server , int Port , int DataType )
{
  m_DataType = DataType;
  switch ( m_DataType )
  {
    case TYPE_TEXT: m_ClientName = CONNECTION_REQUEST_TEXT ; break ;
    case TYPE_DATAPACKETS: m_ClientName = CONNECTION_REQUEST_DATAPACKETS ; break ;
    case TYPE_RAWTEXT: m_ClientName = CONNECTION_REQUEST_RAWTEXT ; break ;
  }
  hostent * adr = gethostbyname( server );

  if ( adr == NULL ) return false;

  memcpy( ( char FAR * )&( m_SDest.sin_addr ) ,
    adr->h_addr ,
    adr->h_length );
  m_SDest.sin_family = AF_INET;
  m_SDest.sin_port = htons( Port );
  if ( m_SocketThread )
  {
    Disconnect();
    Sleep( 10 );
    Connect();
  }
  return true;
}

bool CTCPClient::Send( pDataBurst pDB )
{
  if ( m_Sock == INVALID_SOCKET ) return false;
  if ( pDB->dwDataSize )
  {
    if ( send( m_Sock , ( char* )pDB , pDB->dwDataSize + sizeof( DWORD ) , 0 ) == SOCKET_ERROR )
    {
      m_Status.Format( "CTCPClient send: socket error: \"%s\"" , LastWSAErr2Mes( WSAGetLastError() ) );
      TRACE( "%s\n" , m_Status );
      return false;
    }
  }
  return true;
}
bool CTCPClient::SendBuffer( LPBYTE pDB , DWORD dwBufLen )
{
  if ( m_Sock == INVALID_SOCKET )
    return false;
  if ( send( m_Sock , ( char* )pDB , dwBufLen , 0 ) == SOCKET_ERROR )
  {
    m_Status.Format( "CTCPClient SendBuffer: socket error: \"%s\"" , LastWSAErr2Mes( WSAGetLastError() ) );
    TRACE( "%s\n" , m_Status );
    return false;
  }
  return true;
}

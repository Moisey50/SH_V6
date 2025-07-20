// TCPServer.cpp: implementation of the CTCPServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Network.h"
#include "TCPServer.h"
#include <networks\WSAErr2Mes.h>
#include "Client.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef THIS_MODULENAME
#undef THIS_MODULENAME
#endif
#define THIS_MODULENAME (__FILE__)
#pragma comment(lib,"Ws2_32.lib")

UINT ReceiveThread(LPVOID lpData)
{
  ((CTCPServer*)lpData)->ListenPortAccept();
  return 0;
}

UINT ListenThread( LPVOID lpData )
{
  CTCPServer* hst=(CTCPServer*)lpData;
  SOCKADDR_IN saLstServ,saAccept;
  int saAcceptLen;

  if (hst->m_ListenSock!=INVALID_SOCKET)
  {
    hst->m_Status.Format("CTCPServer::ListenThread : Listen sock already received"); TRACE("!!! %s\n",hst->m_Status);
    return (UINT)-1;
  }

  hst->m_ListenSock = socket ( AF_INET, SOCK_STREAM, 0 );
  if ( INVALID_SOCKET ==  hst->m_ListenSock)
  {
    hst->m_Status.Format("CTCPServer::ListenThread : socket error: \"%s\"",LastWSAErr2Mes(WSAGetLastError())); TRACE("!!! %s\n",hst->m_Status);
    return (UINT)-1;
  }

  saLstServ.sin_family = AF_INET;
  saLstServ.sin_port = htons (hst->m_ListenPort);  
  saLstServ.sin_addr.s_addr = htonl (INADDR_ANY);

  if ( ::bind(hst->m_ListenSock,(struct sockaddr *) &saLstServ, sizeof (saLstServ)) == SOCKET_ERROR) 
  {
    int ErrNo=WSAGetLastError();
    hst->m_Status.Format("CTCPServer::ListenThread : bind : socket error: \"%s\"",LastWSAErr2Mes(ErrNo)); TRACE("!!! %s\n",hst->m_Status);
    closesocket ( hst->m_ListenSock); hst->m_ListenSock=INVALID_SOCKET;
    if (ErrNo==WSAEADDRINUSE)
    {
      return (UINT)-1;
    }
    return (UINT)-1;
  }
  FxSendLogMsg( 1 , _T( "TCPServer" ) , 0 , 
    _T( "Waiting for connection on port %d" ) , hst->m_ListenPort ) ;

  //    hst->m_Status.Format("Waiting for connection"); TRACE("+++ %s\n", hst->m_Status);
  while (!hst->m_Termreq)
  {
    hst->m_Status.Format("Waiting for connection"); TRACE("+++ %s\n",hst->m_Status);
    if (listen (hst->m_ListenSock, MAX_PENDING_CONNECTS) !=0) 
    {
      hst->m_Status.Format("CTCPServer::ListenThread : listen : socket error: \"%s\"",LastWSAErr2Mes(WSAGetLastError())); TRACE("!!! %s\n",hst->m_Status);
      return (-1);
    }
    else
    {
      saAcceptLen=sizeof(saAccept);
      SOCKET connSock = accept (hst->m_ListenSock,
        (struct sockaddr *) &saAccept, (int *) &saAcceptLen);
      if (hst->m_ConnectedSock != INVALID_SOCKET)
      {
        if (connSock != INVALID_SOCKET)
        {
          shutdown(connSock, SD_BOTH);
          closesocket(connSock);
          hst->m_Status.Format("CTCPServer::ListenThread : accept : already connected");
          TRACE("!!! %s\n", hst->m_Status);
        }
        else
        {
          hst->m_Status.Format(
            "CTCPServer::ListenThread : accept : socket error: \"%s\"",
            LastWSAErr2Mes(WSAGetLastError())); TRACE("!!! %s\n",hst->m_Status);
        }
      }
      else if (connSock !=INVALID_SOCKET)
      {
        hst->m_ConnectedSock=connSock;
        hst->m_ReceiveThread = AfxBeginThread(ReceiveThread, (void*)hst, THREAD_PRIORITY_TIME_CRITICAL, 0, CREATE_SUSPENDED, NULL); 
        if (hst->m_ReceiveThread) 
        {
          hst->m_ReceiveThread->m_bAutoDelete = TRUE;
          hst->m_ReceiveThread->ResumeThread();
        }
      }
      else // error timeSetEvent
      {
        hst->m_Status.Format("CTCPServer::ListenThread : accept : socket error: \"%s\"",LastWSAErr2Mes(WSAGetLastError())); TRACE("!!! %s\n",hst->m_Status);
      }
    }
    Sleep(1);
  }
  closesocket(hst->m_ListenSock);  hst->m_ListenSock=INVALID_SOCKET;
  hst->m_Status.Format("Server turned off"); TRACE("+++ %s\n",hst->m_Status);
  FxSendLogMsg( 1 , _T( "TCPServer" ) , 0 ,
    _T( "Listening on port %d is finished" ) , hst->m_ListenPort ) ;
  return 0;
}

////// private members

void CTCPServer::ListenPortAccept()
{
  if (HandShake())
  {
    while(!m_Termreq)
    {
      if (::WaitForSingleObject(m_CClosed, 0) == WAIT_OBJECT_0)
      {
        break;
      }
      else if ( m_DataType != TYPE_RAWTEXT )
      {
        DWORD ds=0;
        DWORD iRet=reciveall(m_ConnectedSock,(char*)&ds,sizeof(ds),0);
        if (iRet==SOCKET_ERROR) break;
        DataBurst *db=NULL;
        if (ds)
        {
          db=(DataBurst*)malloc(ds+sizeof(DataBurst));
          if (!db) 
            continue;
          db->dwDataSize=ds;
          iRet=reciveall(m_ConnectedSock,(char*)db->lpData,db->dwDataSize,0);
          if (db->dwDataSize!=iRet)
          {
            free(db); db=NULL;
            continue;
          }
        }
        OnReceive(db);
        if (db) 
          free(db); 
        db=NULL;
      }
      else
      {
        DataBurst *db = NULL;
        char bBuffer[ 4000 ] ;
        memset( bBuffer , 0 , sizeof( bBuffer ) );
        int iRet = recv( m_ConnectedSock , bBuffer , sizeof( bBuffer ) , 0 );
        if ( iRet == SOCKET_ERROR )
        {
          m_Status.Format( "Can't receive from client, error: \"%s\"" , LastWSAErr2Mes( WSAGetLastError() ) );
          TRACE( m_Status );
          break ;
        }
        if ( iRet <= 0 )
          break ;
			  db = (DataBurst*) new BYTE[sizeof(db->dwDataSize) + iRet + 1];
			  db->dwDataSize = iRet + 1;
			  memcpy(db->lpData, bBuffer, iRet);
			  db->lpData[iRet] = 0;
			  OnReceive(db);
        if ( db )
          free( db );
        db = NULL;
      }
    }
  }
  closesocket (m_ConnectedSock); 
  m_ConnectedSock=INVALID_SOCKET;
  m_Status="Disconnected";
  TRACE("CTCPServer::ListenPortAccept() : %s\n",m_Status);
  m_ReceiveThread = NULL;
}

bool CTCPServer::HandShake()
{
  TCHAR Name[124];
  sockaddr_in SockAddr;
  int addrlen = sizeof( SockAddr );

  if ( getsockname( m_ConnectedSock , (LPSOCKADDR) &SockAddr , &addrlen ) == SOCKET_ERROR )
  {
    ProcessWSAError() ;
  }
  sprintf_s( Name , "%s:%u" , inet_ntoa( SockAddr.sin_addr ) , (DWORD)SockAddr.sin_port ) ;

  if ( m_bNotSHClient )
  {
    m_DataType = TYPE_RAWTEXT ;
    strcat_s( Name , " TEXT NO HANDSHAKE" ) ;
    FxSendLogMsg( 1 , _T("TCPServer") , 0 , _T("Client connected %s") , Name ) ;
    if ( send( m_ConnectedSock , CONNECTION_ANSWER , 
      (int) strlen( CONNECTION_ANSWER ) + 1 , 0 ) == SOCKET_ERROR )
    {
      m_Status.Format( "Can't connect to client, error: \"%s\"" , LastWSAErr2Mes( WSAGetLastError() ) ); TRACE( "!!! %s\n" , m_Status );
      closesocket( m_ConnectedSock ); m_ConnectedSock = INVALID_SOCKET;
      TRACE( "!!! Error handshake: %s\n" , m_Status );
      return false;
    }
    FxSendLogMsg( 1 , _T( "TCPServer" ) , 0 , _T( "Answer sent %s" ) , "OK" ) ;
  }
  else
  {
    memset(Name,0,124);
    int iRet;
    iRet=recv(m_ConnectedSock,Name,124,0);
    if ((iRet==0) || (iRet==SOCKET_ERROR))
    {
      m_Status.Format("Can't connect to client, error: \"%s\"",LastWSAErr2Mes(WSAGetLastError())); TRACE("!!! %s\n",m_Status);
      closesocket (m_ConnectedSock); 
      m_ConnectedSock=INVALID_SOCKET;
      TRACE("!!! Error handshake: %s\n",m_Status);
      return false;
    }
    Name[100]=0;
    if (strcmp(Name,CONNECTION_REQUEST_TEXT)==0)
    {
      if (send(m_ConnectedSock,CONNECTION_ANSWER,(int)strlen(CONNECTION_ANSWER)+1,0)== SOCKET_ERROR)
      {
        m_Status.Format("Can't connect to client, error: \"%s\"",LastWSAErr2Mes(WSAGetLastError())); TRACE("!!! %s\n",m_Status);
        closesocket (m_ConnectedSock); m_ConnectedSock=INVALID_SOCKET;
        TRACE("!!! Error handshake: %s\n",m_Status);
        return false;
      }
      TRACE("+++ Success handshake. DataType=TYPE_TEXT\n");
      m_DataType=TYPE_TEXT;
      strcat_s( Name , " TEXT" ) ;
      FxSendLogMsg( 1 , _T( "TCPServer" ) , 0 , "Client connected %s" , Name ) ;
    }
    else if (strcmp(Name,CONNECTION_REQUEST_DATAPACKETS)==0)
    {
      if (send(m_ConnectedSock,CONNECTION_ANSWER,(int)strlen(CONNECTION_ANSWER)+1,0)== SOCKET_ERROR)
      {
        m_Status.Format("Can't connect to client, error: \"%s\"",LastWSAErr2Mes(WSAGetLastError())); TRACE("!!! %s\n",m_Status);
        closesocket (m_ConnectedSock); m_ConnectedSock=INVALID_SOCKET;
        TRACE("!!! Error handshake: %s\n",m_Status);
        return false;
      }
      TRACE("+++ Success handshake. DataType=TYPE_DATAPACKETS\n");
      m_DataType=TYPE_DATAPACKETS;
      strcat_s( Name , " DATAPACKETS" ) ;
      FxSendLogMsg( 1 , _T( "TCPServer" ) , 0 , "Client connected %s" , Name ) ;
    }
    else if ( strcmp( Name , CONNECTION_REQUEST_RAWTEXT ) == 0 )
    {
      if ( send( m_ConnectedSock , CONNECTION_ANSWER , (int)strlen( CONNECTION_ANSWER ) + 1 , 0 ) == SOCKET_ERROR )
      {
        m_Status.Format( "Can't connect to client, error: \"%s\"" , LastWSAErr2Mes( WSAGetLastError() ) ); TRACE( "!!! %s\n" , m_Status );
        closesocket( m_ConnectedSock ); m_ConnectedSock = INVALID_SOCKET;
        TRACE( "!!! Error handshake: %s\n" , m_Status );
        return false;
      }
      TRACE( "+++ Success handshake. DataType=RAWTEXT\n" );
      m_DataType = TYPE_RAWTEXT;
      strcat_s( Name , " RAWTEXT" ) ;
      FxSendLogMsg( 1 , _T( "TCPServer" ) , 0 , "Client connected %s" , Name ) ;
    }
    else
    {
      m_Status.Format("Can't connect to client, error: \"Wrong client name\""); TRACE("!!! %s\n",m_Status);
      closesocket (m_ConnectedSock); m_ConnectedSock=INVALID_SOCKET;
      TRACE("!!! Error handshake: %s\n",m_Status);
      FxSendLogMsg( MSG_ERROR_LEVEL , _T( "TCPServer" ) , 0 , "CAN't connect" ) ;

      return false;
    }
  }
  m_Status="Connected"; 
  TRACE("+++ CTCPServer::HandShake() name: '%s'\n",Name);
  return true;
}


void CTCPServer::ProcessWSAError()
{
  int lastErr=WSAGetLastError();
  switch (lastErr)
  {
  case WSAECONNABORTED:
  case WSAECONNRESET:
    m_Status="Connection is terminated";
    TRACE("%s\n",m_Status);
    ::SetEvent(m_CClosed);
    break;
  default:
    {
      TRACE("!!! Unprocessed WSA error: %d\n",lastErr);
      LPVOID lpMsgBuf;
      FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        lastErr ,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL 
        );
      SENDERR("!!! Unprocessed WSA error: %d (%s)" , lastErr , lpMsgBuf ) ;
      // Free the buffer.
      LocalFree( lpMsgBuf );
    }
    break ;
  }
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTCPServer::CTCPServer(NWDataCallback pDataCallback, LPVOID usrData):
  CServer(pDataCallback, usrData),
  m_Termreq(false),
  m_ListenThread(NULL),
  m_ReceiveThread(NULL),
  m_ListenPort(DEFAULT_SERVICE_PORT),
  m_ListenSock(INVALID_SOCKET),
  m_ConnectedSock(INVALID_SOCKET)
{
  m_CClosed=::CreateEvent(NULL, FALSE, FALSE, NULL);
  m_Status="Server turned off";
}

CTCPServer::~CTCPServer()
{
  Stop();
  ::CloseHandle(m_CClosed);
}

bool CTCPServer::Start()
{
  if (IsRunning()) 
    return true;
  ResetEvent(m_CClosed);
  INT iRetVal;
  iRetVal = WSAStartup ( MAKEWORD ( 2,0 ), &m_wsaData );
  if ( 0 != iRetVal)
  {
    m_Status.Format("Server error: WSAStartup error: %d\n",iRetVal); 
    TRACE("!!! %s\n",m_Status);
    return false;
  }
  m_Termreq=FALSE;
  m_ListenThread = AfxBeginThread(ListenThread,this,
    THREAD_PRIORITY_TIME_CRITICAL,0,CREATE_SUSPENDED,NULL); 
  if (m_ListenThread) 
  {
    m_ListenThread ->m_bAutoDelete = TRUE;
    m_ListenThread ->ResumeThread();
  }
  return true;
}

void CTCPServer::Stop()
{
  m_Termreq=TRUE; 
  if (m_ListenThread)
  {
    shutdown(m_ListenSock,SD_BOTH);
    closesocket(m_ListenSock);  
    m_ListenSock=INVALID_SOCKET;
    if (m_ConnectedSock!=INVALID_SOCKET)
    {
      shutdown(m_ConnectedSock,SD_BOTH);
      closesocket(m_ConnectedSock);  
      m_ConnectedSock=INVALID_SOCKET;
    }
    ::SetEvent(m_CClosed);
    if (m_ReceiveThread)
      ::WaitForSingleObject(m_ReceiveThread, INFINITE);
    m_ReceiveThread = NULL;
    if (m_ListenThread)
      ::WaitForSingleObject(m_ListenThread->m_hThread, INFINITE);
    m_ListenThread=NULL;
  }
  WSACleanup ( );
}

bool CTCPServer::IsRunning()
{
  return ((m_ListenThread!=NULL) 
    && (::WaitForSingleObject(m_ListenThread->m_hThread, 0)==WAIT_TIMEOUT));
}

void	 CTCPServer::SetServicePort(int Port)
{
  bool WasRunning=IsRunning();
  if (WasRunning)
    Stop();
  m_ListenPort=Port;
  if (WasRunning)
    Start();
}

bool CTCPServer::Send(pDataBurst pDB)
{
  FXAutolock lock(m_Lock);
  if (m_ConnectedSock==INVALID_SOCKET) 
    return false;
  if (send(m_ConnectedSock,(char*)pDB,sizeof(DWORD)+pDB->dwDataSize,0)== SOCKET_ERROR) 
  {
    ProcessWSAError();
    return false;
  }
  return true;
}
bool CTCPServer::SendBuffer( LPBYTE pDB , DWORD dwBufLen )
{
  FXAutolock lock(m_Lock);
  if (m_ConnectedSock==INVALID_SOCKET)
    return false;
  if (send(m_ConnectedSock,(char*)pDB,dwBufLen,0)== SOCKET_ERROR) 
  {
    ProcessWSAError();
    return false;
  }
  return true;
}

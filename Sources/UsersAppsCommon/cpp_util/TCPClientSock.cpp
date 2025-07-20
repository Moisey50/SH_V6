// TCPClientSock.cpp: implementation of the CTCPClientSock class.
//
//////////////////////////////////////////////////////////////////////
#include <Winsock2.h>
#include "TCPClientSock.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CTCPClientSock::CTCPClientSock()
{
	m_socket = INVALID_SOCKET;
	m_iID = 0; //for server reference
}

CTCPClientSock::~CTCPClientSock()
{
	Close();
}

bool CTCPClientSock::InitWinSock2()
{
	//Initialize Windows Socket2 library
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD( 2, 2 ); 

	return ( WSAStartup( wVersionRequested, &wsaData ) == 0 );
}

bool CTCPClientSock::Init( char* szAddr, int iPort )
{
	sockaddr_in  sa; 
  ZeroMemory( (char*)&sa, sizeof(sa) );
  sa.sin_family = AF_INET;

	//If no address is provided bind to whatever address
	u_long ulAddr = INADDR_ANY;
	if ( szAddr )
	{
		ulAddr = inet_addr( szAddr );
		if ( ulAddr == INADDR_NONE )
			return false;
	}

	sa.sin_addr.S_un.S_addr = ulAddr;
  sa.sin_port = (u_short)htons( iPort );

	//Create the socket
	m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( m_socket == INVALID_SOCKET )
		return false;

    //Bind to the provided address
	int iRet = bind( m_socket, (sockaddr*)&sa, sizeof( sockaddr_in ) );
	if ( iRet == SOCKET_ERROR )
		return false;
  	m_saBoundAddr = sa;

	int iOptOn = 1;
	if ( setsockopt( m_socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&iOptOn, sizeof(iOptOn) ) == SOCKET_ERROR  )
		return false;

	//Make socket non blocking
	u_long ulVal = 1;
	if ( ioctlsocket( m_socket, FIONBIO, &ulVal ) == SOCKET_ERROR )
		return false;
	
	SetState( TSS_Init );

	return true;
}

void CTCPClientSock::Close()
{
	if ( m_socket == INVALID_SOCKET )
		return;

	//shutdown( m_socket, SD_BOTH );
	closesocket( m_socket );
	m_socket = INVALID_SOCKET;
	
	SetState( TSS_NotInit );
}

bool CTCPClientSock::Connect(  char* szRemoteAddr, int iRemotePort  )
{
	//Not initialized
	if ( GetState() != TSS_Init )
		return false;
	
	//Get address in network order
	u_long ulAddr = inet_addr( szRemoteAddr );
	if ( ulAddr == INADDR_NONE )
		return false;

	ZeroMemory( &m_saRemoteAddr, sizeof(m_saRemoteAddr) );
	m_saRemoteAddr.sin_family = AF_INET;
	memcpy( &m_saRemoteAddr.sin_addr, &ulAddr, sizeof(ulAddr) );
	m_saRemoteAddr.sin_port = htons( iRemotePort );

	bool bRead, bWrite;
	//Connect the socket to the specified remote address
  int iRet = connect( m_socket, (sockaddr*)&m_saRemoteAddr, sizeof( sockaddr_in ) );
	if ( iRet == SOCKET_ERROR )
	{
		iRet = GetError();
		switch( iRet )
		{
		case WSAEWOULDBLOCK:
			if ( Select( &bWrite, &bRead, 500 ) )
			{
				if ( !bWrite )
				{
					SetState( TSS_Init );
					return false;
				}
			}
			break;
		case WSAEISCONN:
			return false;
		}
	}
	
	SetState( TSS_Connected ); 
	
	return true;
	//Set send and receive buffer sizes for this socket
/*	int iSockBuf = 32*1024;
	setsockopt( m_socket, SOL_SOCKET, SO_SNDBUF, 
				(char*)&iSockBuf, sizeof(iSockBuf) );
	setsockopt( m_socket, SOL_SOCKET, SO_RCVBUF, 
				(char*)&iSockBuf, sizeof(iSockBuf) );
*/
}

bool CTCPClientSock::Send( void* pData, int* pSize, int iOpt )
{
	if ( !IsConnected() )
		return false;

	int iRet = send( m_socket, (LPCSTR)pData, *pSize, iOpt );
	if ( SOCKET_ERROR == iRet )
	{
		//most definatelly means that socket has been disconnected
		int iErr = GetError();
		if ( iErr != WSAEWOULDBLOCK )
			Close();

		return false;
	}

	if ( iRet == 0 ) //gracefull disconnect
	{
		Close();
		return false;
	}
	
	//whole msg is sent
	if ( *pSize == iRet )
		return true;
	
	//part of the message being sent
	*pSize = iRet;

	return false;
}

bool CTCPClientSock::Receive( char* pData, int* piSize, int iOpt )
{
	//The socket is not connected, cant recv anything
	if ( !IsConnected() )
		return false;

	int iRet = recv( m_socket, (char*)pData, *piSize, iOpt );
	if ( SOCKET_ERROR == iRet )
	{
		//most definatelly means that socket has been disconnected
		int iErr = GetError();
		if ( iErr != WSAEWOULDBLOCK )
			Close();

		return false;
	}

	if ( iRet == 0 ) //gracefull disconnect
	{
		Close();
		return false;
	}
	
	return true;
}

bool CTCPClientSock::ReceiveMsg( void* pData, uint* pSize )
{
	if ( *pSize == 0 )
		return true;

	u_long ulDataAvailable = 0;
	int iRet = ioctlsocket( m_socket, FIONREAD, &ulDataAvailable );
	if ( iRet == SOCKET_ERROR )
	{
		Close();
		return false;
	}
	if ( ulDataAvailable >= *pSize )
	{
		return Receive( pData, (int*)pSize );
	}
	else
	{
		//just to track socket status
		int iSize = 0;
		Receive( pData, &iSize );
	}
	return false;
	

	//return Receive( pData, (int*)pSize );
}

//blocking
bool CTCPClientSock::SendMsg( void* pData, uint uiSize )
{
	uint uiSizeWrite = 0;
	char* pByteData = (char*)pData;
	while( IsConnected() && uiSizeWrite < uiSize )
	{
		char* pStart = &pByteData[ uiSizeWrite ];
		int iWritten = uiSize - uiSizeWrite;
		if ( !Send( pStart, &iWritten ) )
		{
			//should not happen
			Sleep( 20 );
		}
		else
		{
			uiSizeWrite += iWritten;
		}
	}
	return ( uiSizeWrite == uiSize );
}

void CTCPClientSock::Attach( SOCKET s, sockaddr_in* psa, int iID )
{
	m_socket = s;
	m_saRemoteAddr = *psa;
	m_iID = iID;
	SetState( TSS_Connected );
	
	int iOptOn = 1;
	int iRet = setsockopt( m_socket, SOL_SOCKET, SO_KEEPALIVE, 
							(char *)&iOptOn, sizeof(iOptOn) );

}

ETCPSockStates CTCPClientSock::GetState()
{
	return m_State;
}

void CTCPClientSock::SetState( ETCPSockStates st )
{
	m_State = st;
}

int CTCPClientSock::GetError()
{
	return WSAGetLastError();
}

bool CTCPClientSock::Select( bool* pWrite, bool* pRead, int iTimeout )
{
	fd_set fdsRead, fdsWrite;
	FD_ZERO( &fdsRead );
	FD_ZERO( &fdsWrite );
	FD_SET( m_socket, &fdsRead );
	FD_SET( m_socket, &fdsWrite );
	*pRead = false;
	*pWrite = false;
	
	timeval tout;
	tout.tv_sec = iTimeout / 1000;
	tout.tv_usec = (iTimeout % 1000) * 1000;

	int iRet = select( 1, &fdsRead, &fdsWrite, 0, &tout );
	if ( iRet == SOCKET_ERROR )
		return false;

	if ( FD_ISSET( m_socket, &fdsRead ) )
		*pRead = true;
	if ( FD_ISSET( m_socket, &fdsWrite ) )
		*pWrite = true;

	return true;
}

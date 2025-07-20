#include "UDPSock.h"

CUDPSock::CUDPSock()
{
	m_socket = 0;
}

CUDPSock::~CUDPSock()
{
	Close();
}

bool CUDPSock::Init( char* szAddr, int iPort )
{
	m_socket = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( m_socket == SOCK_ERROR )
		return false;

	u_long ulOptOn = 1;
	if ( setsockopt( m_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&ulOptOn, sizeof(ulOptOn) ) )
		return false;

	ulOptOn = 1;
#ifdef WIN32 
    if ( SockIOCtrl( m_socket, FIONBIO, &ulOptOn ) == SOCK_ERROR )
		return false;
#else
    if ( SockIOCtrl( m_socket, FIONBIO, (int)ulOptOn ) == SOCK_ERROR )
		return false;
#endif

	u_long ulAddr = 0;
	if ( szAddr )
		ulAddr = inet_addr( szAddr );

	//Bind to the provided address
	sockaddr_in sa;
	memset( &sa, 0, sizeof(sa) );
	sa.sin_family = AF_INET;
	sa.sin_port = htons( iPort );
	sa.sin_addr.s_addr = ulAddr;

#ifdef WIN32 
#else
	sa.sin_len = sizeof( sa );
#endif

	if ( bind( m_socket, (sockaddr*)&sa, sizeof(sockaddr_in) ) == SOCK_ERROR )
		return false;
	
	return true;
}

void CUDPSock::Close()
{
	SockClose( m_socket );
}

bool CUDPSock::SendTo( char* szAddr, int iPort, void* pData, int* piSize )
{
	sockaddr_in sa;
	memset( &sa, 0, sizeof(sa) );
	sa.sin_family = AF_INET;
	sa.sin_port = htons( iPort );
	sa.sin_addr.s_addr = inet_addr( szAddr );
	int iSent = sendto( m_socket, (char*)pData, *piSize, 0, (sockaddr*)&sa, 
						sizeof(sa) );
	bool bRet = ( iSent == *piSize );
	*piSize = iSent;
	return bRet;
}

bool CUDPSock::ReceiveFrom(void* pData, int* piSize, char* szAddr, int* pPort)
{
	sockaddr_in sa;
	memset( &sa, 0, sizeof(sa) );
	sa.sin_family = AF_INET;
	sa.sin_port = htons( *pPort );
	sockaddr_in* pSA = NULL;
	if ( szAddr )
	{
		sa.sin_addr.s_addr = inet_addr( szAddr );
		pSA = &sa;
	}

	int iFromLen = 0;
	int iRecvd = recvfrom( m_socket, (char*)pData, *piSize, 0, (sockaddr*)pSA, 
							&iFromLen );
	if ( iRecvd == SOCK_ERROR )
		return false;
	*pPort = ntohs( pSA->sin_port );
	*piSize = iRecvd;
	return true;
}

int CUDPSock::Receive( void* pData, int iSize )
{
	int iRet = recv( m_socket, (char*)pData, iSize, 0 );
	return ( iRet == iSize );
}

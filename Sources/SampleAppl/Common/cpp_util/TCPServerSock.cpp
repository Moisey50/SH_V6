// TCPServerSock.cpp: implementation of the CTCPServerSock class.
//
//////////////////////////////////////////////////////////////////////
#include "WinSock2.h"
#include "TCPServerSock.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTCPServerSock::CTCPServerSock()
{

}

CTCPServerSock::~CTCPServerSock()
{
	Close();
}

bool CTCPServerSock::Accept( CTCPClientSock* pSock, int iId )
{
	int iRet = listen( m_socket, SOMAXCONN );
	if ( iRet != 0 )
		return false;

	sockaddr saAcc;
	int iAddrLen = sizeof( saAcc );
	SOCKET s = accept( m_socket, &saAcc, &iAddrLen );
	if ( INVALID_SOCKET == s )
		return false;
	
	pSock->Attach( s, (sockaddr_in*)&saAcc, iId );
	return true;
}

bool CTCPServerSock::Init( char* szAddr, int iPort )
{
	if ( !CTCPClientSock::Init( szAddr, iPort ) )
		return false;

/*	int iRet = listen( m_socket, SOMAXCONN );
	if ( iRet == SOCKET_ERROR )
		return false;
*/	
	SetState( TSS_Listening );
	return true;
}

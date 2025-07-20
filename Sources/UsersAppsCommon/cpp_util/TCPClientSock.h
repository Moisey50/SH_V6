// TCPClientSock.h: interface for the CTCPClientSock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TCPCLIENTSOCK_H__07FED0D3_1559_424A_B941_DB4064360011__INCLUDED_)
#define AFX_TCPCLIENTSOCK_H__07FED0D3_1559_424A_B941_DB4064360011__INCLUDED_

#include <WinSock2.h>
#include "TypeDefs.h"


enum ETCPSockStates
{
	TSS_NotInit,
	TSS_Init,
	TSS_Connected,
	TSS_Listening
};

class CTCPClientSock
{
	bool Select( bool* pWrite, bool* pRead, int iTimeout );

public:
	CTCPClientSock();
	virtual ~CTCPClientSock();
	virtual bool Init( char* szAddr=NULL, int iPort=0 );
	virtual void Close();
	bool Connect(  char* szRemoteAddr, int iRemotePort  );
	bool IsConnected() { return GetState() == TSS_Connected; }
	
	bool Send( void* pData, int* pSize, int iOpt=0 );
	bool Receive( void* pData, int* piSize, int iOpt=0 );
	bool ReceiveMsg( void* pData, uint* pSize );
	bool SendMsg( void* pData, uint uiSize );

	void Attach( SOCKET s, sockaddr_in* psa, int iID );

	ETCPSockStates GetState();
	void SetState( ETCPSockStates st );
	int GetError();
	static bool InitWinSock2();

protected:
	int				m_iID; //for server reference
	ETCPSockStates	m_State;
	sockaddr_in m_saBoundAddr;
	sockaddr_in m_saRemoteAddr;
	SOCKET		m_socket;
};

#endif // !defined(AFX_TCPCLIENTSOCK_H__07FED0D3_1559_424A_B941_DB4064360011__INCLUDED_)

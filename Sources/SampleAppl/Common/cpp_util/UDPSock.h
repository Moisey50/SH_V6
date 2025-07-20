#ifndef __UDPSOCK_H__
#define __UDPSOCK_H__

#include "TypeDefs.h"

#ifdef WIN32 
	#include "WinSock2.h"
	typedef SOCKET SOCKTYPE;
	#define SOCK_ERROR	INVALID_SOCKET
	#define SockIOCtrl	ioctlsocket
	#define SockClose	closesocket
#else
	#include <ioLib.h>
	#include <sockLib.h>
	#include <inetLib.h>
	#include <selectLib.h>
	typedef int SOCKTYPE;
	#define SOCK_ERROR	ERROR
	#define SockIOCtrl	ioctl
	#define SockClose	close
#endif


class CUDPSock
{
public:
	CUDPSock();
	virtual ~CUDPSock();
	virtual bool Init( char* szAddr=NULL, int iPort=0 );
	virtual void Close();
	bool SendTo( char* szAddr, int iPort, void* pData, int* piSize );
	bool ReceiveFrom(	void* pData, int* piSize, 
						char* szAddr=NULL, int* pPort=NULL );
	bool Receive(void* pData, int iSize );

protected:
	SOCKTYPE		m_socket;


};

#endif //__UDPSOCK_H__

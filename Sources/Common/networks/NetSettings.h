#ifndef TVDB400_NETWORK_SETTINGS_INCLUDED
#define TVDB400_NETWORK_SETTINGS_INCLUDED

#include <ws2tcpip.h>

#undef DEFAULT_TVDB_PORT
#define DEFAULT_TVDB_PORT	12345

#ifndef DEFAULT_WINSOCK_VERSION
#define DEFAULT_WINSOCK_VERSION	MAKEWORD(2, 2)
#endif

#ifndef LOCAL_ADDRESS
#define LOCAL_ADDRESS	ADDR_ANY
#endif

#ifndef DEFAULT_REMOTE_ADDRESS
#define DEFAULT_REMOTE_ADDRESS	_T("127.0.0.1\0      ")
#endif

#ifndef DEFAULT_TVDB_NETBUFLEN
#define DEFAULT_TVDB_NETBUFLEN	1024
#endif

#ifndef CLIENT_RECONNECT_TIME
#define CLIENT_RECONNECT_TIME	1000	// 1 second
#endif

#ifndef SERVER_PING_TIME
#define SERVER_PING_TIME		10000	// 10 secons
#endif

#define TVDB_PING_MESSAGE		_T("$ping$")

#define TVDB_PING_MSG_SIZE		((DWORD)(_tcslen(TVDB_PING_MESSAGE) + 1) * sizeof(CHAR))

#define CASE_WSAERROR(error)	case(error): return _T(#error)

inline const CHAR* VerboseWSAError(int e)
{
	if (!e)
		return _T("No error");
	switch (e)
	{
		CASE_WSAERROR(WSAEINTR);
		CASE_WSAERROR(WSAEACCES);
		CASE_WSAERROR(WSAEFAULT);
		CASE_WSAERROR(WSAEINVAL);
		CASE_WSAERROR(WSAEMFILE);
		CASE_WSAERROR(WSAEWOULDBLOCK);
		CASE_WSAERROR(WSAEINPROGRESS);
		CASE_WSAERROR(WSAEALREADY);
		CASE_WSAERROR(WSAENOTSOCK);
		CASE_WSAERROR(WSAEDESTADDRREQ);
		CASE_WSAERROR(WSAEMSGSIZE);
		CASE_WSAERROR(WSAEPROTOTYPE);
		CASE_WSAERROR(WSAENOPROTOOPT);
		CASE_WSAERROR(WSAEPROTONOSUPPORT);
		CASE_WSAERROR(WSAESOCKTNOSUPPORT);
		CASE_WSAERROR(WSAEOPNOTSUPP);
		CASE_WSAERROR(WSAEPFNOSUPPORT);
		CASE_WSAERROR(WSAEAFNOSUPPORT);
		CASE_WSAERROR(WSAEADDRINUSE);
		CASE_WSAERROR(WSAEADDRNOTAVAIL);
		CASE_WSAERROR(WSAENETDOWN);
		CASE_WSAERROR(WSAENETUNREACH);
		CASE_WSAERROR(WSAENETRESET);
		CASE_WSAERROR(WSAECONNABORTED);
		CASE_WSAERROR(WSAECONNRESET);
		CASE_WSAERROR(WSAENOBUFS);
		CASE_WSAERROR(WSAEISCONN);
		CASE_WSAERROR(WSAENOTCONN);
		CASE_WSAERROR(WSAESHUTDOWN);
		CASE_WSAERROR(WSAETIMEDOUT);
		CASE_WSAERROR(WSAECONNREFUSED);
		CASE_WSAERROR(WSAEHOSTDOWN);
		CASE_WSAERROR(WSAEHOSTUNREACH);
		CASE_WSAERROR(WSAEPROCLIM);
		CASE_WSAERROR(WSASYSNOTREADY);
		CASE_WSAERROR(WSAVERNOTSUPPORTED);
		CASE_WSAERROR(WSANOTINITIALISED);
		CASE_WSAERROR(WSAEDISCON);
		CASE_WSAERROR(WSATYPE_NOT_FOUND);
		CASE_WSAERROR(WSAHOST_NOT_FOUND);
		CASE_WSAERROR(WSATRY_AGAIN);
		CASE_WSAERROR(WSANO_RECOVERY);
		CASE_WSAERROR(WSANO_DATA);
		CASE_WSAERROR(WSA_INVALID_HANDLE);
		CASE_WSAERROR(WSA_INVALID_PARAMETER);
		CASE_WSAERROR(WSA_IO_INCOMPLETE);
		CASE_WSAERROR(WSA_IO_PENDING);
		CASE_WSAERROR(WSA_NOT_ENOUGH_MEMORY);
		CASE_WSAERROR(WSA_OPERATION_ABORTED);
		//CASE_WSAERROR(WSAINVALIDPROCTABLE);
		//CASE_WSAERROR(WSAINVALIDPROVIDER);
		//CASE_WSAERROR(WSAPROVIDERFAILEDINIT);
	}
	return _T("UNKNOWN WSA ERROR");
}

const enum // my network events
{
	// server events
	NES_START_OK,		// server started; param = NULL
	NES_STARTERR_WSA,	// server not started due to WSA error; param = (int)WSAGetLastError
	NES_STARTERR_SYS,	// server not started due to generic error; param = (DWORD)GetLastError
	NES_SHUTDOWN_OK,	// server shut down; param = NULL
	NES_SHUTDOWN_ERR,	// server stopped with WSA error; param = (int)WSAGetLastError
	NES_ACCEPTERR_WSA,	// server can't accept connection due to WSA error; param = (int)WSAGetLastError
	NES_ACCEPTERR_SYS,	// server can't start I/O thread; param = (DWORD)GetLastError
	NES_CONNECT_OK,		// server accepted connection to a new client; param = (DWORD)client_UID
	NES_DISCONNECT_OK,	// client disconnected softly; param = (DWORD)client_UID
	NES_RECEIVE_ERR,	// connection to client lost when receiving data; param -> { (DWORD)client_UID, (int)WSAGetLastError }
	NES_SEND_ERR,		// connection to client lost when sending data; param -> { (DWORD)client_UID, (int)WSAGetLastError }
	NES_PING,			// a ping message from client; param = (DWORD)client_UID

	// client events
	NEC_START_OK,		// client started; param = NULL
	NEC_STARTERR_WSA,	// client not started due to WSA error; param = (int)WSAGetLastError
	NEC_STARTERR_SYS,	// client not started due to generic error; param = (DWORD)GetLastError
	NEC_SHUTDOWN_OK,	// client stopped; param = NULL
	NEC_SHUTDOWN_ERR,	// client stopped with WSA error; param = (int)WSAGetLastError
	NEC_CONNECTERR_WSA,	// client can't connect due to WSA error; param = (int)WSAGetLastError
	NEC_CONNECTERR_SYS,	// client can't start I/O thread; param = (DWORD)GetLastError
	NEC_CONNECT_OK,		// client connected to server; param = NULL
	NEC_DISCONNECT,		// client disconnected from server; param = NULL
	NEC_SEND_ERR,		// client lost connection while sending data; param = (int)WSAGetLastError
	NEC_RECEIVE_ERR,	// client lost connection while receiving data; param = (int)WSAGetLastError
	NEC_SERVERDOWN,		// client lost connection due to server shut down; param = NULL
	NEC_PONG,			// server replied on client ping; param = NULL
};

#endif

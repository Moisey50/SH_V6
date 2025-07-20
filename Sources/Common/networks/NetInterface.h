#ifndef NETAPI_INTERFACE_INCLUDED
#define NETAPI_INTERFACE_INCLUDED

#ifdef _DEBUG
    #define NETAPI_DLL_NAME  "netapid.dll"
    #define NETAPI_LIB_NAME  "netapid.lib"
#else
 #define NETAPI_DLL_NAME  "netapi.dll"
 #define NETAPI_LIB_NAME  "netapi.lib"
#endif


#ifndef NETAPI_DLL
 #define AFX_EXT_NETAPI __declspec(dllimport)
 #pragma comment(lib, NETAPI_LIB_NAME)
#else
 #define AFX_EXT_NETAPI __declspec(dllexport)
#endif

#include "NetSettings.h"

class INetServer
{
protected:
	INetServer() {};
	virtual ~INetServer() {};
public:
	virtual int IGetLastError() = 0;
	virtual BOOL IShutDown() = 0;
	virtual BOOL IStart(WORD port = DEFAULT_TVDB_PORT, WORD wVersion = DEFAULT_WINSOCK_VERSION) = 0;
};

class INetClient
{
protected:
	INetClient() {};
	virtual ~INetClient() {};
public:
	virtual int IGetLastError() = 0;
	virtual BOOL IIsConnected() = 0;
	virtual BOOL ISend(LPVOID lpData, DWORD cbData) = 0;
	virtual BOOL IShutDown() = 0;
	virtual BOOL IStart(LPTSTR address = (LPTSTR)DEFAULT_REMOTE_ADDRESS, WORD port = DEFAULT_TVDB_PORT, WORD wVersion = DEFAULT_WINSOCK_VERSION) = 0;
};

typedef LPVOID (CALLBACK *NETAPI_ON_SERVER_RECV)(LPCVOID lpData, int& cbData, DWORD idClient, LPVOID pHost);
typedef void (CALLBACK *NETAPI_ON_CLIENT_RECV)(LPCVOID, int cbData, LPVOID pHost);
typedef void (CALLBACK *NETAPI_ON_NET_EVENT)(UINT nEvent, LPVOID pParam, LPVOID pHost);

AFX_EXT_NETAPI INetServer* NetApi_CreateServer(LPVOID pHost, NETAPI_ON_SERVER_RECV fnServerRecv, NETAPI_ON_NET_EVENT fnServerEvent);
AFX_EXT_NETAPI INetClient* NetApi_CreateClient(LPVOID pHost, NETAPI_ON_CLIENT_RECV fnClientRecv, NETAPI_ON_NET_EVENT fnClientEvent);
AFX_EXT_NETAPI void NetApi_DeleteServer(INetServer* Server);
AFX_EXT_NETAPI void NetApi_DeleteClient(INetClient* Client);

#endif

#pragma once
#include "simpleserver.h"
#include <networks\NetInterface.h>

class CNetServer :
	public INetServer,
	public CSimpleServer
{
	NETAPI_ON_SERVER_RECV m_fnServerRecv;
	NETAPI_ON_NET_EVENT m_fnServerEvent;
	LPVOID m_pHost;
public:
	CNetServer(LPVOID pHost, NETAPI_ON_SERVER_RECV fnServerRecv, NETAPI_ON_NET_EVENT fnServerEvent);
	virtual ~CNetServer();

	virtual int IGetLastError() { return GetLastWSAError(); };
	virtual BOOL IShutDown() { return ShutDown(); }
	virtual BOOL IStart(WORD port = DEFAULT_TVDB_PORT, WORD wVersion = DEFAULT_WINSOCK_VERSION) { return Start(port, wVersion); };

private:
	virtual LPVOID OnReceive(LPCVOID lpData, int& cbData, DWORD idClient);
	virtual void OnNetEvent(UINT nEvent, LPVOID pParam); // see events list in NetSettings.h
};

#pragma once
#include "simpleclient.h"
#include <networks\NetInterface.h>

class CNetClient :
	public INetClient,
	public CSimpleClient
{
	NETAPI_ON_CLIENT_RECV m_fnClientRecv;
	NETAPI_ON_NET_EVENT m_fnClientEvent;
	LPVOID m_pHost;
public:
	CNetClient(LPVOID pHost, NETAPI_ON_CLIENT_RECV fnClientRecv, NETAPI_ON_NET_EVENT fnClientEvent);
	virtual ~CNetClient();

	virtual int IGetLastError() { return GetLastWSAError(); };
	virtual BOOL IIsConnected() { return IsConnected(); };
	virtual BOOL ISend(LPVOID lpData, DWORD cbData) { return Send(lpData, cbData); };
	virtual BOOL IShutDown() { return ShutDown(); }
	virtual BOOL IStart(LPTSTR address = (LPTSTR) DEFAULT_REMOTE_ADDRESS, 
    WORD port = DEFAULT_TVDB_PORT, WORD wVersion = DEFAULT_WINSOCK_VERSION) { return Start(address, port, wVersion); };

private:
	virtual void OnReceive(LPCVOID lpData, int cbData);
	virtual void OnNetEvent(UINT nEvent, LPVOID pParam);
};

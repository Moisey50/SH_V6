#include "StdAfx.h"
#include "NetServer.h"

CNetServer::CNetServer(LPVOID pHost, NETAPI_ON_SERVER_RECV fnServerRecv, NETAPI_ON_NET_EVENT fnServerEvent):
m_pHost(pHost),
m_fnServerRecv(fnServerRecv),
m_fnServerEvent(fnServerEvent)
{
}

CNetServer::~CNetServer()
{
	ShutDown();
}

LPVOID CNetServer::OnReceive(LPCVOID lpData, int& cbData, DWORD idClient)
{
	ASSERT(m_pHost && m_fnServerRecv);
	return m_fnServerRecv(lpData, cbData, idClient, m_pHost);
}

void CNetServer::OnNetEvent(UINT nEvent, LPVOID pParam)
{
	if (!m_fnServerEvent)
		return;
	ASSERT(m_pHost);
	m_fnServerEvent(nEvent, pParam, m_pHost);
}

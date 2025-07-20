#include "StdAfx.h"
#include "NetClient.h"

CNetClient::CNetClient(LPVOID pHost, NETAPI_ON_CLIENT_RECV fnClientRecv, NETAPI_ON_NET_EVENT fnClientEvent):
m_pHost(pHost),
m_fnClientRecv(fnClientRecv),
m_fnClientEvent(fnClientEvent)
{
}

CNetClient::~CNetClient()
{
	ShutDown();
}

void CNetClient::OnReceive(LPCVOID lpData, int cbData)
{
	ASSERT(m_pHost && m_fnClientRecv);
	m_fnClientRecv(lpData, cbData, m_pHost);
}

void CNetClient::OnNetEvent(UINT nEvent, LPVOID pParam)
{
	if (!m_fnClientEvent)
		return;
	ASSERT(m_pHost);
	m_fnClientEvent(nEvent, pParam, m_pHost);
}

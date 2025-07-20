#pragma once

#include "NetOps.h"
#include <afxmt.h>
class CSimpleServer
{
  int m_LastWSAError;
	BOOL m_bWSARunning;
	SOCKET m_ListenSocket;
	HANDLE m_evShutDown;
	HANDLE m_ListenThread;
	SOCKET m_AcceptedSocket;
	DWORD m_CurrentID;
	CPtrArray m_IOThreads, m_ConnectedSockets;
	HANDLE m_evIOStarted;
	CCriticalSection m_cs, m_csEvent;
public:
	CSimpleServer();
	virtual ~CSimpleServer();

	BOOL Start(WORD port = DEFAULT_TVDB_PORT, WORD wVersion = DEFAULT_WINSOCK_VERSION);
	BOOL ShutDown();
	int GetLastWSAError() { return m_LastWSAError; };
	void Listen();
	void DoIO();

private:
	void RaiseEvent(UINT nEvent, LPVOID pParam); // safe multithread wrapper for OnNetEvent
	virtual LPVOID OnReceive(LPCVOID lpData, int& cbData, DWORD idClient); // see basic implementation for more info
	virtual void OnNetEvent(UINT nEvent, LPVOID pParam) { };

  static DWORD WINAPI ListenThread( LPVOID pThis )  { ((CSimpleServer*) pThis)->Listen(); return 0; };
  static DWORD WINAPI IOServerThread( LPVOID pThis ) { ((CSimpleServer*) pThis)->DoIO(); return 0; };
};

#pragma once

#include "NetOps.h"
#include <afxmt.h>


class CSimpleClient
{
	int m_LastWSAError;
	BOOL m_bWSARunning;
	SOCKET m_Socket;
	HANDLE m_evShutDown, m_evSend, m_evReady;
	HANDLE m_IOThread;
	LPVOID m_lpSendBuffer;
	DWORD m_cbSendBuffer, m_cbSendBufferMax;
	HANDLE m_ConnectThread;
	ADDRINFOT* m_pAddrResult;
	CCriticalSection m_csEvent;
public:
	CSimpleClient();
	virtual ~CSimpleClient();

	BOOL Start(LPTSTR address = (LPTSTR)DEFAULT_REMOTE_ADDRESS, WORD port = DEFAULT_TVDB_PORT, WORD wVersion = DEFAULT_WINSOCK_VERSION);
	BOOL ShutDown();
	BOOL IsConnected() { return ((m_IOThread != NULL) && (::WaitForSingleObject(m_IOThread, 0) != WAIT_OBJECT_0)); };
	BOOL Send(LPVOID lpData, DWORD cbData);
	int GetLastWSAError() { return m_LastWSAError; };
	void DoIO();
	void Connect();

private:
	void RaiseEvent(UINT nEvent, LPVOID pParam); // safe multithread wrapper for OnNetEvent
	virtual void OnReceive(LPCVOID lpData, int cbData); // see basic implementation for more info
	virtual void OnNetEvent(UINT nEvent, LPVOID pParam) { };
  static DWORD WINAPI IOClientThread( LPVOID pThis ) { ((CSimpleClient*) pThis)->DoIO(); return 0; };
  static DWORD WINAPI ConnectThread( LPVOID pThis ) { ((CSimpleClient*) pThis)->Connect(); return 0; };
};


#include "StdAfx.h"
#include "SimpleClient.h"

CSimpleClient::CSimpleClient():
m_csEvent()
{
	m_LastWSAError = 0;
	m_bWSARunning = FALSE;
	m_Socket = INVALID_SOCKET;
	m_evShutDown = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_evSend = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_evReady = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_IOThread = NULL;
	m_lpSendBuffer = NULL;
	m_cbSendBuffer = 0;
	m_cbSendBufferMax = 0;
	m_ConnectThread = NULL;
	m_pAddrResult = NULL;
}

CSimpleClient::~CSimpleClient(void)
{
	if (m_bWSARunning)
		ShutDown();
	::CloseHandle(m_evReady);
	m_evReady = NULL;
	::CloseHandle(m_evSend);
	m_evSend = NULL;
	::CloseHandle(m_evShutDown);
	m_evShutDown = NULL;
}

BOOL CSimpleClient::Start(LPTSTR address /*= DEFAULT_REMOTE_ADDRESS*/, 
  WORD port /*= DEFAULT_TVDB_PORT*/, WORD wVersion /*= DEFAULT_WINSOCK_VERSION*/)
{
	MYTRACE(">> CSimpleClient::Start()\n");
	if (m_bWSARunning)
	{
		if (!ShutDown())
			return FALSE;
	}
	::ResetEvent(m_evShutDown); // restarting client
	WSADATA WSAData;
	m_LastWSAError = /*::WSAStartup*/StartWSA(wVersion, &WSAData, _T("Client"));
	if (m_LastWSAError)
	{
		RaiseEvent(NEC_STARTERR_WSA, (LPVOID)(size_t)m_LastWSAError);
		::SetEvent(m_evShutDown);
		return FALSE;
	}
	MYTRACE("WSA Initialized by client!\n Version: %d.%d (maximum %d.%d)\n Info:    %s\n Status:  %s\n",
		LOBYTE(WSAData.wVersion), HIBYTE(WSAData.wVersion), LOBYTE(WSAData.wHighVersion), HIBYTE(WSAData.wHighVersion),
		WSAData.szDescription, WSAData.szSystemStatus);
	m_bWSARunning = TRUE;
	ADDRINFOT addrInfo;
	::ZeroMemory(&addrInfo, sizeof(addrInfo));
	addrInfo.ai_family = AF_UNSPEC;
	addrInfo.ai_socktype = SOCK_STREAM;
	addrInfo.ai_protocol = IPPROTO_TCP;
	TCHAR strPort[6];
#pragma warning (disable: 4996)
	_itot((int)port, strPort, 10);
#pragma warning (default: 4996)
	m_LastWSAError = ::GetAddrInfo(address, strPort, &addrInfo, &m_pAddrResult);
	if (m_LastWSAError)
	{
		m_pAddrResult = NULL;
		RaiseEvent(NEC_STARTERR_WSA, (LPVOID) (size_t) m_LastWSAError);
		::SetEvent(m_evShutDown);
		return FALSE;
	}
	m_ConnectThread = ::CreateThread(NULL, 0, ConnectThread, this, CREATE_SUSPENDED, 0);
	if (!m_ConnectThread)
	{
		RaiseEvent(NEC_STARTERR_SYS, (LPVOID) (size_t) GetLastError());
		::SetEvent(m_evShutDown);
		return FALSE;
	}
	MYTRACE("<< CSimpleClient::Start() ok\n");
	RaiseEvent(NEC_START_OK, NULL);
	::ResumeThread(m_ConnectThread);
	return TRUE;
}

BOOL CSimpleClient::ShutDown()
{
	MYTRACE(">> CSimpleClient::ShutDown()\n");
	::SetEvent(m_evShutDown);
	if (m_ConnectThread)
	{
		::WaitForSingleObject(m_ConnectThread, INFINITE);
		::CloseHandle(m_ConnectThread);
		m_ConnectThread = NULL;
	}
	if (m_Socket != INVALID_SOCKET)
	{
		::shutdown(m_Socket, SD_RECEIVE);
		::closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}
	if (m_pAddrResult)
		::FreeAddrInfo(m_pAddrResult);
	m_pAddrResult = NULL;
	MYTRACE("Client disconnected!\n");
	if (m_lpSendBuffer)
		free(m_lpSendBuffer);
	m_lpSendBuffer = NULL;
	m_cbSendBuffer = m_cbSendBufferMax = 0;
	m_LastWSAError = 0;
	if (m_bWSARunning)
	{
		if (/*::WSACleanup()*/StopWSA(_T("Client")) == INVALID_SOCKET)
			m_LastWSAError = ::WSAGetLastError();
	}
	if (m_LastWSAError)
	{
		RaiseEvent(NEC_SHUTDOWN_OK, (LPVOID) (size_t) m_LastWSAError);
		return FALSE;
	}
	m_bWSARunning = FALSE;
	MYTRACE("WSA Terminated by client!\n");
	MYTRACE("<< CSimpleClient::ShutDown() ok\n");
	RaiseEvent(NEC_SHUTDOWN_OK, NULL);
	return TRUE;
}

BOOL CSimpleClient::Send(LPVOID lpData, DWORD cbData)
{
	if (!IsConnected())
		return FALSE;
	MYTRACE(">> CSimpleClient::Send()\n");
	HANDLE hEvents[] = { m_evShutDown, m_evReady };
	DWORD cEvents = sizeof(hEvents) / sizeof(HANDLE);
	switch (::WaitForMultipleObjects(cEvents, hEvents, FALSE, INFINITE))
	{
	case WAIT_OBJECT_0:
		MYTRACE(">> CSimpleClient::Send() (aborted on shutdown)\n");
		return FALSE; // shut down
	case WAIT_OBJECT_0 + 1:
		{
			if (cbData > m_cbSendBufferMax)
			{
				m_lpSendBuffer = (LPBYTE)realloc(m_lpSendBuffer, cbData);
				m_cbSendBufferMax = cbData;
			}
			memcpy(m_lpSendBuffer, lpData, cbData);
			m_cbSendBuffer = cbData;
			::SetEvent(m_evSend);
			break;
		}
	}
	MYTRACE("<< CSimpleClient::Send() ok\n");
	return TRUE;
}

void CSimpleClient::Connect()
{
	while (::WaitForSingleObject(m_evShutDown, CLIENT_RECONNECT_TIME) != WAIT_OBJECT_0)
	{
		m_Socket = ::socket(m_pAddrResult->ai_family, m_pAddrResult->ai_socktype, m_pAddrResult->ai_protocol);
		if (m_Socket == INVALID_SOCKET)
		{
			m_LastWSAError = ::WSAGetLastError();
			RaiseEvent(NEC_STARTERR_WSA, (LPVOID) (size_t) m_LastWSAError);
			::CloseHandle(m_ConnectThread);
			m_ConnectThread = NULL;
			ShutDown();
			return;
		}
		if (::connect(m_Socket, m_pAddrResult->ai_addr, (int)m_pAddrResult->ai_addrlen) == SOCKET_ERROR)
		{
			m_LastWSAError = ::WSAGetLastError();
			RaiseEvent(NEC_CONNECTERR_WSA, (LPVOID) (size_t) m_LastWSAError);
		}
		else
		{
			ASSERT(m_IOThread == NULL);
			m_IOThread = ::CreateThread(NULL, 0, IOClientThread, this, 0, NULL);
			if (!m_IOThread)
			{
				RaiseEvent(NEC_CONNECTERR_SYS, (LPVOID) (size_t) GetLastError());
				::CloseHandle(m_ConnectThread);
				m_ConnectThread = NULL;
				ShutDown();
				return;
			}
			MYTRACE("Client connected!\n");
			RaiseEvent(NEC_CONNECT_OK, NULL);
			::WaitForSingleObject(m_IOThread, INFINITE);
			::CloseHandle(m_IOThread);
			m_IOThread = NULL;
			RaiseEvent(NEC_DISCONNECT, NULL);
			ASSERT(m_Socket != INVALID_SOCKET);
			::closesocket(m_Socket);
			m_Socket = INVALID_SOCKET; // reopen socket if connection breaks
		}
	}
}

void CSimpleClient::DoIO()
{
	MYTRACE("Client enters I/O loop\n");
	::SetEvent(m_evReady);
	HANDLE hEvents[] = { m_evShutDown, m_evSend };
	DWORD cEvents = sizeof(hEvents) / sizeof(HANDLE);
	int cbBufRecv = DEFAULT_TVDB_NETBUFLEN;
	char* lpBufRecv = (char*)malloc(cbBufRecv);
	do
	{
		switch (::WaitForMultipleObjects(cEvents, hEvents, FALSE, SERVER_PING_TIME/*INFINITE*/))
		{
		case WAIT_OBJECT_0:
			// shut down
			free(lpBufRecv);
			MYTRACE("Client exits I/O loop on shut down\n");
			return;
		case WAIT_OBJECT_0 + 1:
			// do I/O
			{
				// send data
				int iResult = WriteToSocket(m_Socket, (char*)m_lpSendBuffer, (int)m_cbSendBuffer);
				if (iResult == SOCKET_ERROR)
				{
					m_LastWSAError = ::WSAGetLastError();
					free(lpBufRecv);
					MYTRACE("Client breaks on send failure\n");
					RaiseEvent(NEC_SEND_ERR, (LPVOID) (size_t) m_LastWSAError);
					return;
				}

				// send buffer ready for new transaction
				::SetEvent(m_evReady);

				// receive server response
				int cbRecv = ReadFromSocket(m_Socket, &lpBufRecv, cbBufRecv);
				if (cbRecv == SOCKET_ERROR)
				{
					m_LastWSAError = ::WSAGetLastError();
					free(lpBufRecv);
					MYTRACE("Client breaks on receive failure\n");
					RaiseEvent(NEC_RECEIVE_ERR, (LPVOID) (size_t) m_LastWSAError);
					return;
				}
				else if (cbRecv == 0)
				{
					free(lpBufRecv);
					MYTRACE("Client breaks on server shutdown\n");
					RaiseEvent(NEC_SERVERDOWN, NULL);
					return;
				}
				if ((cbRecv == TVDB_PING_MSG_SIZE) && (memcmp(lpBufRecv, (LPCVOID)TVDB_PING_MESSAGE, cbRecv) == 0))
				{
					// come round pong message
#ifdef SIGNAL_PING_EVENTS
					RaiseEvent(NEC_PONG, NULL);
#endif
				}
				else
					OnReceive((LPCVOID)lpBufRecv, cbRecv);
				break;
			}
		case WAIT_TIMEOUT:
			// long period of inactivity, check connection
			Send((LPVOID)TVDB_PING_MESSAGE, TVDB_PING_MSG_SIZE);
			break;
		}
	}while (::WaitForSingleObject(m_evShutDown, 0) != WAIT_OBJECT_0);
	free(lpBufRecv);
	MYTRACE("Client exits I/O loop\n");
}

void CSimpleClient::RaiseEvent(UINT nEvent, LPVOID pParam)
{
	CSingleLock lock(&m_csEvent);
	lock.Lock();
	ASSERT(lock.IsLocked());
	OnNetEvent(nEvent, pParam);
	lock.Unlock();
}

void CSimpleClient::OnReceive(LPCVOID lpData, int cbData)
{
/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
* Override this function to analyze server's response  *
* Parameters:                                          *
*  (in) LPCVOID lpData - server's response.            *
*  (in) int     cbData - size of response (in BYTEs).  *
* ATTENTION!!! lpData MUST NOT BE FREED!               *
* * * * * * * * * * * * * * * * * * * * * * * * * * * */

	// just for example
	const int maxLength = MAX_PATH;
	char* strResponse = (char*)lpData;
	if (strlen(strResponse) > maxLength)
		strResponse[maxLength] = 0;
	MYTRACE("Client received \"%s\" from server\n", strResponse);
}

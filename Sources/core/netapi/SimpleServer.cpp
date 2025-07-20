#include "StdAfx.h"
#include "SimpleServer.h"
#include <ws2tcpip.h>

CSimpleServer::CSimpleServer()
{
	m_LastWSAError = 0;
	m_bWSARunning = FALSE;
	m_ListenSocket = INVALID_SOCKET;
	m_evShutDown = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_ListenThread = NULL;
	m_AcceptedSocket = INVALID_SOCKET;
	m_CurrentID = 0;
	m_evIOStarted = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

CSimpleServer::~CSimpleServer()
{
	if (m_bWSARunning)
		ShutDown();
	::CloseHandle(m_evShutDown);
	m_evShutDown = NULL;
	::CloseHandle(m_evIOStarted);
	m_evIOStarted = NULL;
}

BOOL CSimpleServer::Start(WORD port /*= DEFAULT_TVDB_PORT*/, WORD wVersion /*= DEFAULT_WINSOCK_VERSION*/)
{
	MYTRACE(">> CSimpleServer::Start()\n");
	if (m_bWSARunning)
	{
		if (!ShutDown())
			return FALSE;
	}
	::ResetEvent(m_evShutDown); // restarting
	WSADATA WSAData;
	m_LastWSAError = /*::WSAStartup*/StartWSA(wVersion, &WSAData, _T("server"));
	if (m_LastWSAError)
	{
		RaiseEvent(NES_STARTERR_WSA, (LPVOID) (size_t) m_LastWSAError);
		::SetEvent(m_evShutDown);
		return FALSE;
	}
	MYTRACE("WSA Initialized by server!\n Version: %d.%d (maximum %d.%d)\n Info:    %s\n Status:  %s\n",
		LOBYTE(WSAData.wVersion), HIBYTE(WSAData.wVersion), LOBYTE(WSAData.wHighVersion), HIBYTE(WSAData.wHighVersion),
		WSAData.szDescription, WSAData.szSystemStatus);
	m_bWSARunning = TRUE;
	ADDRINFOT addrInfo, *addrResult;
	::ZeroMemory(&addrInfo, sizeof(addrInfo));
	addrInfo.ai_family = AF_INET;
	addrInfo.ai_socktype = SOCK_STREAM;
	addrInfo.ai_protocol = IPPROTO_TCP;
	addrInfo.ai_flags = AI_PASSIVE;
	TCHAR strPort[6];
#pragma warning (disable: 4996)
	_itot((int)port, strPort, 10);
#pragma warning (default: 4996)
	m_LastWSAError = ::GetAddrInfo((PCSTR)LOCAL_ADDRESS, strPort, &addrInfo, &addrResult);
	if (m_LastWSAError)
	{
		RaiseEvent(NES_STARTERR_WSA, (LPVOID) (size_t) m_LastWSAError);
		::SetEvent(m_evShutDown);
		return FALSE;
	}
	m_ListenSocket = ::socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
	if (m_ListenSocket != INVALID_SOCKET)
	{
		if ((::bind(m_ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen) == SOCKET_ERROR) ||
			(::listen(m_ListenSocket, SOMAXCONN) == SOCKET_ERROR))
		{
			m_LastWSAError = ::WSAGetLastError();
			::closesocket(m_ListenSocket);
			m_ListenSocket = INVALID_SOCKET;
		}
	}
	else
		m_LastWSAError = ::WSAGetLastError();
	::FreeAddrInfo(addrResult);
	if (m_ListenSocket == INVALID_SOCKET)
	{
		RaiseEvent(NES_STARTERR_WSA, (LPVOID) (size_t) m_LastWSAError);
		::SetEvent(m_evShutDown);
		return FALSE;
	}
	m_ListenThread = ::CreateThread(NULL, 0, ListenThread, this, 0, NULL);
	if (!m_ListenThread)
	{
		::closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
		RaiseEvent(NES_STARTERR_SYS, (LPVOID) (size_t) GetLastError());
		::SetEvent(m_evShutDown);
		return FALSE;
	}
	MYTRACE("<< CSimpleServer::Start() ok\n");
	RaiseEvent(NES_START_OK, NULL);
	return TRUE;
}

BOOL CSimpleServer::ShutDown()
{
	MYTRACE(">> CSimpleServer::ShutDown()\n");
	if (m_ListenSocket != INVALID_SOCKET)
	{
		::shutdown(m_ListenSocket, SD_BOTH);
		::closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
	}
	if (m_ListenThread)
	{
		::WaitForSingleObject(m_ListenThread, INFINITE);
		::CloseHandle(m_ListenThread);
		m_ListenThread = NULL;
	}
	::SetEvent(m_evShutDown);
	int i;
	for (i = 0; i < m_IOThreads.GetSize(); i++)
	{
		HANDLE hIOThread = (HANDLE)m_IOThreads.GetAt(i);
		SOCKET sConnected = (SOCKET)m_ConnectedSockets.GetAt(i);
		if (sConnected != INVALID_SOCKET)
		{
			::shutdown(sConnected, SD_RECEIVE);
			::closesocket(sConnected);
		}
		MYTRACE("Server is waiting I/O thread 0x%x to exit...", hIOThread);
		::WaitForSingleObject(hIOThread, INFINITE);
		MYTRACE(" ok!\n");
		::CloseHandle(hIOThread);
	}
	m_ConnectedSockets.RemoveAll();
	m_IOThreads.RemoveAll();
	m_CurrentID = 0;
	if (m_AcceptedSocket != INVALID_SOCKET)
	{
		::shutdown(m_AcceptedSocket, SD_BOTH);
		::closesocket(m_AcceptedSocket);
		m_AcceptedSocket = INVALID_SOCKET;
	}
	m_LastWSAError = 0;
	if (m_bWSARunning)
	{
		if (/*::WSACleanup()*/StopWSA(_T("Server")) == SOCKET_ERROR)
			m_LastWSAError = ::WSAGetLastError();
	}
	if (m_LastWSAError)
	{
		RaiseEvent(NES_SHUTDOWN_ERR, (LPVOID) (size_t) m_LastWSAError);
		return FALSE;
	}
	m_bWSARunning = FALSE;
	MYTRACE("WSA Terminated by server!\n");
	MYTRACE("<< CSimpleServer::ShutDown() ok\n");
	RaiseEvent(NES_SHUTDOWN_OK, NULL);
	return TRUE;
}

void CSimpleServer::Listen()
{
	MYTRACE("Server enters listen mode\n");
	while (/*::WaitForSingleObject(m_evShutDown, 0) != WAIT_OBJECT_0*/m_ListenSocket != INVALID_SOCKET)
	{
		ASSERT(m_AcceptedSocket == INVALID_SOCKET);
		m_AcceptedSocket = ::accept(m_ListenSocket, NULL, NULL);
		if (m_AcceptedSocket != INVALID_SOCKET)
		{
			HANDLE hIOThread = ::CreateThread(NULL, 0, IOServerThread, this, CREATE_SUSPENDED, NULL);
			if (!hIOThread)
			{
				::closesocket(m_AcceptedSocket);
				m_AcceptedSocket = INVALID_SOCKET;
				RaiseEvent(NES_ACCEPTERR_SYS, (LPVOID) (size_t) GetLastError());
				continue;
			}
			m_CurrentID++;
			m_IOThreads.Add(hIOThread);
			m_ConnectedSockets.Add((void*)m_AcceptedSocket);
			MYTRACE("Server starts I/O thread 0x%x for client #%d\n", hIOThread, m_CurrentID);
			RaiseEvent(NES_CONNECT_OK, (LPVOID) (size_t) m_CurrentID);
			::ResumeThread(hIOThread);
			::WaitForSingleObject(m_evIOStarted, INFINITE);
		}
		else
		{
			m_LastWSAError = ::WSAGetLastError();
			RaiseEvent(NES_ACCEPTERR_WSA, (LPVOID) (size_t) m_LastWSAError);
			CloseHandle(m_ListenThread);
			m_ListenThread = NULL;
			ShutDown();
			break;
		}
	}
	MYTRACE("Server exits listen mode\n");
}

void CSimpleServer::DoIO()
{
	SOCKET ConnectedSocket = m_AcceptedSocket;
	DWORD idClient = m_CurrentID;
	m_AcceptedSocket = INVALID_SOCKET;
	int cbBufRecv = DEFAULT_TVDB_NETBUFLEN;
	char* lpBufRecv = (char*)malloc(cbBufRecv);
	MYTRACE("Server connected to socket 0x%x!\n", ConnectedSocket);
	::SetEvent(m_evIOStarted);
	while (::WaitForSingleObject(m_evShutDown, 0) != WAIT_OBJECT_0)
	{
		// receive client data
		int cbRecv = ReadFromSocket(ConnectedSocket, &lpBufRecv, cbBufRecv);
		if (cbRecv == SOCKET_ERROR)
		{
			m_LastWSAError = ::WSAGetLastError();
			DWORD params[] = { idClient, (DWORD)m_LastWSAError };
			RaiseEvent(NES_RECEIVE_ERR, (LPVOID)params);
			MYTRACE("Server socket 0x%x breaks on receive failure\n", ConnectedSocket);
			break;
		}
		else if (cbRecv == 0)
		{
			m_LastWSAError = ::WSAGetLastError();
			RaiseEvent(NES_DISCONNECT_OK, (LPVOID) (size_t) idClient);
			MYTRACE("Server socket 0x%x breaks on client shutdown\n", ConnectedSocket);
			break;
		}

		// generate response
		LPVOID lpResponse;
		BOOL bPingMsg = ((cbRecv == TVDB_PING_MSG_SIZE) && (memcmp(lpBufRecv, (LPCVOID)TVDB_PING_MESSAGE, cbRecv) == 0));
		if (bPingMsg)
		{
			lpResponse = lpBufRecv;
#ifdef SIGNAL_PING_EVENTS
			RaiseEvent(NES_PING, (LPVOID) (size_t) idClient);
#endif
		}
		else
		{
			CSingleLock lock(&m_cs);
			lock.Lock();
			lpResponse = OnReceive((LPCVOID)lpBufRecv, cbRecv, idClient);
			lock.Unlock();
		}

		// send response
		int iResult = WriteToSocket(ConnectedSocket, (char*)lpResponse, cbRecv);
		if (!bPingMsg)
			free(lpResponse);
		if (iResult == SOCKET_ERROR)
		{
			m_LastWSAError = ::WSAGetLastError();
			DWORD params[] = { idClient, (DWORD)m_LastWSAError };
			RaiseEvent(NES_SEND_ERR, (LPVOID)params);
			MYTRACE("Server exits I/O loop on send failure\n");
			break;
		}
	}
	free(lpBufRecv);
	lpBufRecv = NULL;
	cbBufRecv = 0;
	m_ConnectedSockets[idClient - 1] = (void*)INVALID_SOCKET;
	::shutdown(ConnectedSocket, SD_BOTH);
	::closesocket(ConnectedSocket);
	MYTRACE("Server disconnected from socket 0x%x!\n", ConnectedSocket);
	ConnectedSocket = INVALID_SOCKET;
}

void CSimpleServer::RaiseEvent(UINT nEvent, LPVOID pParam)
{
	CSingleLock lock(&m_csEvent);
	lock.Lock();
	ASSERT(lock.IsLocked());
	OnNetEvent(nEvent, pParam);
	lock.Unlock();
}

LPVOID CSimpleServer::OnReceive(LPCVOID lpData, int& cbData, DWORD idClient)
{
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* This function must be overridden in order to provide a meaningful response     *
* from the server to a client's request.                                         *
* Parameters:                                                                    *
*  (in)      LPCVOID lpData - a request from client. SHOULD NOT BE FREED!        *
*  (in/out)  int     cbData - on entry, size of lpData in BYTEs;                 *
*                           on exit, size of server's response in BYTEs.         *
*  (in)      DWORD   idClient - a UID for connected client                       *
* Return value:                                                                  *
*  Server's response.                                                            *
* Remarks:                                                                       *
*  Memory for the server's response MUST BE allocated dynamically and            *
*  MUST NOT BE FREED! It is freed inside the working thread of the server.       *
*  The cbData parameter must be changed to contain the correct length (in BYTEs) *
*  of the server's response                                                      *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

	// just for example
	const int maxLength = MAX_PATH;
	char* strMsg = (char*)lpData;
	if (strlen(strMsg) > maxLength)
		strMsg[maxLength] = 0;
	MYTRACE("Server received \"%s\" (total %d bytes) from client #%d\n", strMsg, cbData, idClient);
	const char* strResponse = " - OK!";
	cbData = (int)strlen(strMsg) + (int)strlen(strResponse) + 1;
	char* lpResponse = (char*)malloc(cbData);
	memcpy(lpResponse, strMsg, strlen(strMsg));
	memcpy(lpResponse + strlen(strMsg), strResponse, strlen(strResponse) + 1);
	return (LPVOID)lpResponse;
}

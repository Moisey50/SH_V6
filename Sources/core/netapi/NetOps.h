#ifndef TVDB400_NETWORK_OPERATIONS_INCLUDED
#define TVDB400_NETWORK_OPERATIONS_INCLUDED

#include <networks\netsettings.h>

#define SIGNAL_PING_EVENTS		// undef this to not receive ping notifications in OnNetEvent()

inline int ReadFromSocket(SOCKET s, char** ppBuffer, int& cbBuffer)
{
	int cbSent;
	int cbRecv = ::recv(s, (char*)&cbSent, sizeof(cbSent), 0);
	if (cbRecv > 0)
	{
		if (cbBuffer < cbSent)
		{
			*ppBuffer = (char*)realloc(*ppBuffer, cbSent);
			cbBuffer = cbSent;
		}
		int cbRecvTotal = 0;
		while (cbRecvTotal < cbSent)
		{
			cbRecv = ::recv(s, (*ppBuffer) + cbRecvTotal, cbSent - cbRecvTotal, 0);
			if (cbRecv > 0)
				cbRecvTotal += cbRecv;
			else
				return cbRecv;
		}
		return cbRecvTotal;
	}
	return cbRecv;
}

inline int WriteToSocket(SOCKET s, char* pBuffer, int cbBuffer)
{
	int cbSent = ::send(s, (const char*)&cbBuffer, sizeof(cbBuffer), 0);
	if (cbSent != sizeof(cbBuffer))
		return SOCKET_ERROR;
	int cbSentTotal = 0;
	while (cbSentTotal < cbBuffer)
	{
		cbSent = ::send(s, pBuffer + cbSentTotal, cbBuffer - cbSentTotal, 0);
		if (cbSent == SOCKET_ERROR)
			return SOCKET_ERROR;
		cbSentTotal += cbSent;
	}
	return cbSentTotal;
}

static int cWSAInit = 0;
__forceinline int StartWSA(WORD wVersion, LPWSADATA pData, const TCHAR* caller)
{
	cWSAInit++;
	TRACE("WSAStartup call from %s\n%d WSAStartup unreleased call(s)\n", caller, cWSAInit);
	return ::WSAStartup(wVersion, pData);
}
__forceinline int StopWSA(const TCHAR* caller)
{
	cWSAInit--;
	TRACE("WSACleanup call from %s\n%d WSAStartup unreleased call(s)\n", caller, cWSAInit);
	return ::WSACleanup();
}

#endif

// TCPServer.h: interface for the CTCPServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TCPSERVER_H__965A739E_DFA7_4B00_8580_6A8C4B1ED781__INCLUDED_)
#define AFX_TCPSERVER_H__965A739E_DFA7_4B00_8580_6A8C4B1ED781__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Server.h"

class CTCPServer : public CServer  
{
protected:
	WSADATA			  m_wsaData;
	CWinThread*		m_ListenThread;
	CWinThread*		m_ReceiveThread;
	SOCKET			  m_ListenSock;
	SOCKET			  m_ConnectedSock;
	unsigned short	m_ListenPort;
	bool			m_Termreq;
	HANDLE			m_CClosed;
	FXLockObject		m_Lock;
private:
  void ListenPortAccept();
  void ProcessWSAError();
	bool HandShake();
public:
	CTCPServer(NWDataCallback pDataCallback, LPVOID usrData);
	virtual ~CTCPServer();
	bool	 IsRunning();
	void	 SetServicePort(int Port);
	bool     Send(pDataBurst pDB);
  bool     SendBuffer( LPBYTE pDB , DWORD dwBufLen );
	void     Stop();
	bool     Start();
//
friend UINT ListenThread( LPVOID lpData );
friend UINT ReceiveThread(LPVOID lpData);
};

#endif // !defined(AFX_TCPSERVER_H__965A739E_DFA7_4B00_8580_6A8C4B1ED781__INCLUDED_)

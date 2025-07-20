// TCPClient.h: interface for the CTCPClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TCPCLIENT_H__146BCB70_89A7_468E_A2E0_90C8DC815D1A__INCLUDED_)
#define AFX_TCPCLIENT_H__146BCB70_89A7_468E_A2E0_90C8DC815D1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Client.h"

class CTCPClient : public CClient  
{
protected:
	bool          m_ShutDown;
	SOCKET        m_Sock; 
	SOCKADDR_IN   m_SDest;
	CWinThread*   m_SocketThread;
	WSADATA       m_wsaData;
	CString       m_ClientName;
	bool          m_Connected;
protected:
	bool    ReceiveData();
public:
	CTCPClient(NWDataCallback pDataCallback, LPVOID usrData);
	virtual ~CTCPClient();
	void    Connect();
	void    Disconnect();
	bool    IsConnected();
	bool    SetServerAddress(LPCTSTR server, int Port, int DataType);
	bool    Send(pDataBurst pDB);
  bool    SendBuffer( LPBYTE pDB , DWORD dwBufLen );
/////
friend UINT ClientSocketThread(LPVOID lpData);
};

#endif // !defined(AFX_TCPCLIENT_H__146BCB70_89A7_468E_A2E0_90C8DC815D1A__INCLUDED_)

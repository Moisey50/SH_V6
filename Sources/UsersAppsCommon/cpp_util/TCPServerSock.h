// TCPServerSock.h: interface for the CTCPServerSock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TCPSERVERSOCK_H__3C1D2A01_BD25_44F3_A301_B01DAB669F9D__INCLUDED_)
#define AFX_TCPSERVERSOCK_H__3C1D2A01_BD25_44F3_A301_B01DAB669F9D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TCPClientSock.h"

class CTCPServerSock : public CTCPClientSock
{
public:
	CTCPServerSock();
	virtual ~CTCPServerSock();
	bool Accept( CTCPClientSock* pSock, int iId=0 );
	virtual bool Init( char* szAddr=NULL, int iPort=0 );
};

#endif // !defined(AFX_TCPSERVERSOCK_H__3C1D2A01_BD25_44F3_A301_B01DAB669F9D__INCLUDED_)

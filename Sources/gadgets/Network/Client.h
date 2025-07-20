// Client.h: interface for the CClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIENT_H__11C113EB_8D81_49B2_9E19_201B9E1EDA83__INCLUDED_)
#define AFX_CLIENT_H__11C113EB_8D81_49B2_9E19_201B9E1EDA83__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include  "nwconfig.h"

#define TYPE_TEXT	0
#define TYPE_DATAPACKETS	1
#define TYPE_RAWTEXT      2
#define TYPE_RAWTEXTSHSERV 3

class CClient  
{
protected:
	CString       m_Status;
    NWDataCallback m_pDataCallback;
    LPVOID        m_usrData;
	int		      m_DataType;
  bool          m_bGraphStoped ;
public:
	CClient(NWDataCallback pDataCallback, LPVOID usrData);
	virtual ~CClient();
	LPCTSTR GetStatusString() { return m_Status; };
	int		GetDataType() { return m_DataType; }
  bool  GetGraphStoped() { return m_bGraphStoped ; }
  void  SetGraphStoped( bool bGraphStoped ) { m_bGraphStoped = bGraphStoped ; }
	virtual void    Connect()=0;
	virtual void    Disconnect()=0;
	virtual bool    IsConnected() { return false; }
	virtual bool    SetServerAddress(LPCTSTR server, int Port, int DataType)=0;
	virtual bool    OnReceive(pDataBurst pDB);
	virtual bool    Send(pDataBurst pDB)=0;
};

#endif // !defined(AFX_CLIENT_H__11C113EB_8D81_49B2_9E19_201B9E1EDA83__INCLUDED_)

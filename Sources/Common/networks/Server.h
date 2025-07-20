// Server.h: interface for the CServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVER_H__505933B4_5E3B_4972_BA20_DBD3DF306D9F__INCLUDED_)
#define AFX_SERVER_H__505933B4_5E3B_4972_BA20_DBD3DF306D9F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include  "nwconfig.h"

class CServer  
{
protected:
	CString       m_Status;
    NWDataCallback m_pDataCallback;
    LPVOID        m_usrData;
	int			  m_DataType;
  BOOL          m_bNotSHClient ;
  bool          m_bGraphStoped ;
public:
			 CServer(NWDataCallback pDataCallback, LPVOID usrData);
	virtual ~CServer();
	LPCTSTR		 	 GetStatusString() { return m_Status; };
	int				 GetDataType() { return m_DataType; }
  void     SetNoSHMode( BOOL bSet ) { m_bNotSHClient = bSet ; }
  BOOL     GetNoSHMode() { return m_bNotSHClient ; }
  bool  GetGraphStoped() { return m_bGraphStoped ; }
  void  SetGraphStoped( bool bGraphStoped ) { m_bGraphStoped = bGraphStoped ; }
	virtual bool	 OnReceive(pDataBurst pDB);
	virtual void	 SetServicePort(int Port)=0;
	virtual bool     Send(pDataBurst pDB)=0;
	virtual void     Stop()=0;
	virtual bool     Start()=0;
};

#endif // !defined(AFX_SERVER_H__505933B4_5E3B_4972_BA20_DBD3DF306D9F__INCLUDED_)

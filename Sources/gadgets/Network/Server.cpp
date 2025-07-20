// Server.cpp: implementation of the CServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Network.h"
#include "Server.h"
#include "Client.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServer::CServer(NWDataCallback pDataCallback, LPVOID usrData):
	m_DataType(TYPE_TEXT)
{
    m_pDataCallback=pDataCallback;
    m_usrData=usrData;
}

CServer::~CServer()
{

}

bool   CServer::OnReceive(pDataBurst pDB)
{
	bool res=false;
	if ((m_pDataCallback) && (pDB))
	{
		res=m_pDataCallback(m_usrData,pDB);
	}
	return res;
}

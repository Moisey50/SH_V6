// Client.cpp: implementation of the CClient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Network.h"
#include "Client.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CClient::CClient(NWDataCallback pDataCallback, LPVOID usrData):
	m_DataType(TYPE_TEXT)
{
    m_pDataCallback=pDataCallback;
    m_usrData=usrData;
}

CClient::~CClient()
{

}

bool    CClient::OnReceive(pDataBurst pDB)
{
	bool res=false;
	if ((m_pDataCallback) && (pDB))
	{
		res=m_pDataCallback(m_usrData,pDB);
	}
	free(pDB);
	return res;
}
#include "stdafx.h"
#include "NetBuilder.h"

#define THIS_MODULENAME _T("NetBuilder.cpp")

#ifdef _DEBUG
 #define TODO
#endif

#define TVDB400_DEFAULTTIMEOUT	(10 * 1000)	// 10 seconds to wait for response after query
#define TVDB400_NETMAXTRIES		10			// times to try to send the same query on Send failure
#define TVDB400_WAITAFTERTRY	1000		// 1 second to wait between every try


void CALLBACK fnClientRecv(LPCVOID lpData, int cbData, LPVOID pHost)
{
	((CNetBuilder*)pHost)->OnReceive(lpData, cbData);
}

void CALLBACK fnClientEvent(UINT nEvent, LPVOID pParam, LPVOID pHost)
{
	((CNetBuilder*)pHost)->OnNetEvent(nEvent, pParam);
}

LPCTSTR CNetBuilder::enumgraphs = _T("*");

CNetBuilder::CNetBuilder(LPCTSTR host, WORD port):
m_pNetClient(NULL),
m_pTmpInOut(NULL),
m_nTimeout(TVDB400_DEFAULTTIMEOUT),
m_idNetClient(0),
m_cMsg(0),
m_bRuning(FALSE)
{
	m_pNetClient = NetApi_CreateClient(this, fnClientRecv, fnClientEvent);
	if (m_pNetClient)
		m_pNetClient->IStart((LPTSTR)host, port);
	m_evResponse = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

CNetBuilder::~CNetBuilder()
{
	if (m_pNetClient)
	{
		m_pNetClient->IShutDown();
		NetApi_DeleteClient(m_pNetClient);
		m_pNetClient = NULL;
	}
	if (m_evResponse)
		::CloseHandle(m_evResponse);
	m_evResponse = NULL;
}

DWORD CNetBuilder::MsgUid(DWORD idMsg)
{
	DWORD uid = (DWORD)m_cMsg;
	uid = (uid << 24) & 0xFF000000;
	uid |= (idMsg & 0x00FFFFFF);
	m_cMsg++;
	return uid;
}

BOOL CNetBuilder::Transact(CSHSMessage& msgQuery)
{
	FXAutolock lock(m_Lock);
	if (!m_pNetClient)
	{
		DWORD dwRes = CSHSMessage::ERR_FAILCREATECLIENT;
    CSHSMessage Msg( msgQuery.GetSender() , msgQuery.GetID() , &dwRes , sizeof( dwRes ) );
    msgQuery = Msg ;
		return FALSE;
	}
	int cbData;
	LPVOID lpData = msgQuery.GetMsgToSend(cbData, TRUE);
	m_InOutLock.Lock();
	m_pTmpInOut = &msgQuery;
	::ResetEvent(m_evResponse);
	m_InOutLock.Unlock();
	int i = 0;
	while (!m_pNetClient->ISend(lpData, cbData))
	{
		Sleep(TVDB400_WAITAFTERTRY);
		if (++i == TVDB400_NETMAXTRIES)
		{
			DWORD dwRes = CSHSMessage::ERR_FAILSEND;
      CSHSMessage Msg( msgQuery.GetSender() , msgQuery.GetID() , &dwRes , sizeof( dwRes ) );
      msgQuery = Msg ;
      m_InOutLock.Lock();
			m_pTmpInOut = NULL;
			m_InOutLock.Unlock();
			return FALSE;
		}
	}
	i = 0;
	while (::WaitForSingleObject(m_evResponse, m_nTimeout) != WAIT_OBJECT_0)
	{
		if (++i == TVDB400_NETMAXTRIES)
		{
			m_InOutLock.Lock();
			m_pTmpInOut = NULL;
			m_InOutLock.Unlock();
			DWORD dwRes = CSHSMessage::ERR_RESPONSETIMEOUT;
      CSHSMessage Msg( msgQuery.GetSender() , msgQuery.GetID() , &dwRes , sizeof( dwRes ) );
      msgQuery = Msg ;
      return FALSE;
		}
	}
	return TRUE;
}

void CNetBuilder::Close()
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_CLOSEGRAPH), NULL, 0);
	Transact(msg);
	m_idNetClient = 0;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::NewGraph"

int CNetBuilder::NewGraph()
{
	CSHSMessage msg(0, MsgUid(CSHSMessage::MSG_NEWGRAPH), NULL, 0);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_NEWGRAPH");
			return MSG_ERROR_LEVEL;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return MSG_ERROR_LEVEL;
		}
		if (msg.GetDataSize() < 2 * sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_ENUMGRAPHS");
			return MSG_ERROR_LEVEL;
		}
		m_idNetClient = pData[1];
		SENDINFO_1("Remote builder created with id %d", m_idNetClient);
		return 0; // ok, no comments
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
		return MSG_ERROR_LEVEL;
	}
}

#undef THIS_MODULENAME 
#define THIS_MODULENAME "NetBuilder::EnumGraphs"

void* CNetBuilder::EnumGraphs()
{
	CSHSMessage msg(0, MsgUid(CSHSMessage::MSG_ENUMGRAPHS), NULL, 0);
	if (Transact(msg))
	{
		if (msg.GetTotalSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_ENUMGRAPHS");
			return NULL;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return NULL;
		}
		DWORD cbData = msg.GetDataSize() - sizeof(DWORD);
		if (!cbData)
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_ENUMGRAPHS");
			return NULL;
		}
		void* lpData = malloc(cbData);
		memcpy(lpData, pData + 1, cbData);
		return lpData;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
		return NULL;
	}
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::LoadGraph"

int CNetBuilder::LoadGraph(LPCTSTR fileName, bool bClearOld)
{
	FXPropertyKit pk;
	pk.WriteString(_T("fname"), fileName);
	pk.WriteBool("clear", bClearOld);
	CSHSMessage msg(0, MsgUid(CSHSMessage::MSG_LOADGRAPH), (void*)(LPCTSTR)pk, (DWORD)pk.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_LOADGRAPH");
			return MSG_ERROR_LEVEL;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return MSG_ERROR_LEVEL;
		}
		if (msg.GetDataSize() < 2 * sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_ENUMGRAPHS");
			return MSG_ERROR_LEVEL;
		}
		m_idNetClient = pData[1];
		SENDINFO_1("Remote builder created with id %d", m_idNetClient);
		return 0; // ok, no comments
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
		return MSG_ERROR_LEVEL;
	}
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::LoadScript"

int CNetBuilder::LoadScript(LPCTSTR script, bool bClearOld)
{
	FXPropertyKit pk;
	pk.WriteString("script", script);
	pk.WriteBool("clear", bClearOld);
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_LOADSCRIPT), (void*)LPCTSTR(pk), (int) pk.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_LOADSCRIPT");
			return MSG_ERROR_LEVEL;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return MSG_ERROR_LEVEL;
		}
		if (msg.GetDataSize() < 2 * sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server after ERR_OK in params of MSG_LOADSCRIPT");
			return MSG_ERROR_LEVEL;
		}
		int Result = (int)(*(pData + 1));
		return Result;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
		return MSG_ERROR_LEVEL;
	}
}

void CNetBuilder::OnReceive(LPCVOID lpData, int cbData)
{
	CSHSMessage msg((LPVOID)lpData, cbData);
	m_InOutLock.Lock();
	if (m_pTmpInOut && (m_pTmpInOut->GetID() == msg.GetID()))
	{
		*m_pTmpInOut = msg;
		m_pTmpInOut = NULL;
		::SetEvent(m_evResponse);
	}
	m_InOutLock.Unlock();
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::OnNetEvent"

void CNetBuilder::OnNetEvent(UINT nEvent, LPVOID pParam)
{
  int ErrorCode = (int) (size_t) pParam ;
	switch (nEvent)
	{
	case NEC_START_OK:
		SENDINFO_0(_T("Client started"));
		break;
	case NEC_STARTERR_WSA:
		SENDINFO_1(_T("Client not started due to WSA error %s"), VerboseWSAError(ErrorCode));
		break;
	case NEC_STARTERR_SYS:
		SENDINFO_1(_T("Client not started due to system error %d"), ErrorCode );
		break;
	case NEC_SHUTDOWN_OK:
		SENDINFO_0(_T("Client stopped"));
		break;
	case NEC_SHUTDOWN_ERR:
		SENDINFO_1(_T("Client stopped with WSA error %s"), VerboseWSAError( ErrorCode ));
		break;
	case NEC_CONNECTERR_WSA:
		SENDINFO_1(_T("Client not connected due to WSA error %s"), VerboseWSAError( ErrorCode ));
		break;
	case NEC_CONNECTERR_SYS:
		SENDINFO_1(_T("Client failed to start I/O thread due to system error %d"), ErrorCode );
		break;
	case NEC_CONNECT_OK:
		SENDINFO_0(_T("Client connected"));
		break;
	case NEC_DISCONNECT:
		SENDINFO_0(_T("Client disconnected"));
		break;
	case NEC_SEND_ERR:
		SENDINFO_1(_T("Client lost connection when sending data due to WSA error %s"), VerboseWSAError( ErrorCode ));
		break;
	case NEC_RECEIVE_ERR:
		SENDINFO_1(_T("Client lost connection when receiving data due to WSA error %s"), VerboseWSAError( ErrorCode ) );
		break;
	case NEC_SERVERDOWN:
		SENDINFO_0(_T("Client lost connection due to server shut down"));
		break;
	case NEC_PONG:
		//SENDINFO_0(_T("(pong)"));
		break;
	default:
		SENDINFO_0(_T("UNKNOWN NET EVENT"));
		break;
	}
}

#undef THIS_MODULENAME

void CNetBuilder::ShutDown()
{
	Close();
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::Start"

void CNetBuilder::Start()
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_START), NULL, 0);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_START");
			return;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return;
		}
		m_bRuning = TRUE;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::Stop"

void CNetBuilder::Stop()
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_STOP), NULL, 0);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_STOP");
			return;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return;
		}
		m_bRuning = FALSE;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::Pause"

void CNetBuilder::Pause()
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_PAUSE), NULL, 0);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_PAUSE");
			return;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return;
		}
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::StepFwd"

void CNetBuilder::StepFwd()
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_STEPFWD), NULL, 0);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_STEPFWD");
			return;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return;
		}
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::IsRuning"

BOOL CNetBuilder::IsPaused()
{
    return FALSE;
}

BOOL CNetBuilder::IsRuning()
{
	return m_bRuning;
/*	BOOL bRuning = FALSE;
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_ISRUNING), NULL, 0);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_ISRUNING");
			return bRuning;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return bRuning;
		}
		if (msg.GetDataSize() < 2 * sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_ISRUNING");
			return bRuning;
		}
		bRuning = (BOOL)pData[1];
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return bRuning;*/
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::GetID"

FXString CNetBuilder::GetID()
{
	FXString strId = "";
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_GETID), NULL, 0);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_GETID");
			return strId;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return strId;
		}
		if (msg.GetDataSize() == sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_GETID");
			return strId;
		}
		strId = (LPCTSTR)(pData + 1);
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return strId;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::SetID"

void CNetBuilder::SetID(LPCTSTR newID)
{
	CString uid = (newID ? newID : "");
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_SETID), (void*)LPCTSTR(uid), (int) uid.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_SETID");
			return;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return;
		}
		SENDINFO_2("Builder 0x%p gets id %s", this, uid);
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::SetDirty"

BOOL CNetBuilder::SetDirty(BOOL set)
{
	DWORD data = (DWORD)set;
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_SETDIRTY), &data, sizeof(data));
	BOOL bWasDirty = FALSE;
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_SETDIRTY");
			return bWasDirty;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return bWasDirty;
		}
		if (msg.GetDataSize() < 2 * sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_SETDIRTY");
			return bWasDirty;
		}
		bWasDirty = (BOOL)pData[1];
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return bWasDirty;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::IsDirty"

BOOL CNetBuilder::IsDirty()
{
	BOOL bIsDirty = FALSE;
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_ISDIRTY), NULL, 0);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_ISDIRTY");
			return bIsDirty;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return bIsDirty;
		}
		if (msg.GetDataSize() < 2 * sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_ISDIRTY");
			return bIsDirty;
		}
		bIsDirty = (BOOL)pData[1];
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return bIsDirty;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::Save"

BOOL CNetBuilder::Save(LPCTSTR fileName, CStringArray* BlockUIDs)
{
	FXPropertyKit pk;
	pk.WriteString("fname", fileName);
	if (BlockUIDs)
	{
		int i;
		CString label;
		for (i = 0; i < BlockUIDs->GetSize(); i++)
		{
			label.Format("uid%d", i);
			pk.WriteString(label, BlockUIDs->GetAt(i));
		}
	}
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_SAVE), (void*)LPCTSTR(pk), (int) pk.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_SAVE");
			return FALSE;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return FALSE;
		}
		SENDINFO_1("Graph saved to \"%s\"", fileName);
		return TRUE;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return FALSE;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::Load"

int CNetBuilder::Load(LPCTSTR fileName, LPCTSTR script, bool bClearOld)
{
	if (!fileName && !script)
	{
		// new graph
		if (m_idNetClient)
			Close();
		return NewGraph();
	}
	else if (!fileName && script)
	{
		// load script
		return LoadScript(script, bClearOld);
	}
	else if (!strcmp(fileName, enumgraphs))
	{
		// enum graphs
		return (int)(size_t)EnumGraphs();
	}
	else if (fileName)
	{
		// load graph
		if (m_idNetClient)
			Close();
		return LoadGraph(fileName, bClearOld);
	}
	return MSG_ERROR_LEVEL; // failed to process command
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::GetScriptPath"

BOOL CNetBuilder::GetScriptPath(FXString& uid, FXString& path)
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_GETSCRIPTPATH), (void*)LPCTSTR(uid), (int) (int) uid.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_GETSCRIPTPATH");
			return FALSE;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return FALSE;
		}
		if (msg.GetDataSize() <= 2 * sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server after ERR_OK in params of MSG_GETSCRIPTPATH");
			return FALSE;
		}
		BOOL bResult = (BOOL)(*(pData + 1));
		path = (LPCTSTR)(pData + 2);
		return bResult;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return FALSE;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::GetScript"

BOOL CNetBuilder::GetScript(FXString& script, CStringArray* BlockUIDs)
{
    ASSERT(BlockUIDs!=NULL);
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_GETSCRIPT), NULL, 0);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_GETSCRIPT");
			return FALSE;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return FALSE;
		}
		if (msg.GetDataSize() == sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server after ERR_OK in params of MSG_GETSCRIPT");
			return FALSE;
		}
		script = (LPCTSTR)(pData + 1);
		return TRUE;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return FALSE;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::GetProperties"

BOOL CNetBuilder::GetProperties(FXString& props)
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_GETGRAPHPROPS), NULL, 0);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_GETGRAPHPROPS");
			return FALSE;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return FALSE;
		}
		if (msg.GetDataSize() == sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server after ERR_OK in params of MSG_GETGRAPHPROPS");
			return FALSE;
		}
		props = (LPCTSTR)(pData + 1);
		return TRUE;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return FALSE;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::SetProperties"

void CNetBuilder::SetProperties(LPCTSTR props)
{
	CString Properties = (props ? props : "");
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_SETGRAPHPROPS), (void*)LPCTSTR(Properties), Properties.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_SETGRAPHPROPS");
			return;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return;
		}
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::GetTime"

double CNetBuilder::GetTime()
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_GETTIME), NULL, 0);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_GETTIME");
			return 0;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return 0;
		}
		if (msg.GetDataSize() < 3 * sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server after ERR_OK in params of MSG_GETTIME");
			return 0;
		}
		double dtime = *(double*)(pData + 1);
		return dtime;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return 0;
}

#undef THIS_MODULENAME
//#define THIS_MODULENAME "NetBuilder::GetScript"

void CNetBuilder::SendMsg(int msgLevel, LPCTSTR src, int msgId, LPCTSTR msgText)
{
	if (m_pMsgQueue)
		m_pMsgQueue->AddMsg(msgLevel, src, msgId, msgText);
}

void CNetBuilder::PrintMsg(int msgLevel, LPCTSTR src, int msgId, LPCTSTR msgText)
{
	if (m_pMsgQueue)
		m_pMsgQueue->AddMsg(msgLevel, src, msgId, msgText);
	else
	{
		IGraphMsgQueue* MsgQueue = FxGetGraphMsgQueue();
		if (MsgQueue)
			MsgQueue->AddMsg(msgLevel, src, msgId, msgText);
		TRACE("LOG>> Level=%d, ID=%d, text=\"%s\"\n", msgLevel, msgId, msgText);
	}
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::CreateGadget"

BOOL CNetBuilder::CreateGadget(LPCTSTR GadgetClassName)
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_CREATEGADGET), (void*)GadgetClassName, (int)strlen(GadgetClassName) + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_CREATEGADGET");
			return FALSE;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return FALSE;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::RegisterCurGadget"

UINT CNetBuilder::RegisterCurGadget(FXString& uid, LPCTSTR params)
{
	UINT type = TVDB400_GT_ANY;
	FXPropertyKit pk;
	pk.WriteString("uid", uid);
	pk.WriteString("params", params);
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_REGCURGADGET), (void*)LPCTSTR(pk), (int) pk.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_REGCURGADGET");
			return type;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return type;
		}
		if (msg.GetDataSize() == sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_REGCURGADGET");
			return type;
		}
		pk = (LPCTSTR)(pData + 1);
		if (!pk.GetString("uid", uid))
		{
			SENDERR_0("Wrong data format in params of MSG_REGCURGADGET: no uid");
			return type;
		}
		int t;
		if (!pk.GetInt("type", t))
		{
			SENDERR_0("Wrong data format in params of MSG_REGCURGADGET: no type");
			return type;
		}
		type = (UINT)t;
		return type;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return type;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::UnregisterGadget"

void CNetBuilder::UnregisterGadget(FXString& uid)
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_UNREGGADGET), (void*)LPCTSTR(uid), (int) uid.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_UNREGGADGET");
			return;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return;
		}
		return; // ok, no comments
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::ListGadgetConnectors"

UINT CNetBuilder::ListGadgetConnectors(FXString& uidGadget, CStringArray& inputs, CStringArray& outputs, CStringArray& duplex)
{
	UINT type = TVDB400_GT_ANY;
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_LISTGADGETCONNS), (void*)LPCTSTR(uidGadget),
    (int) uidGadget.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_LISTGADGETCONNS");
			return type;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return type;
		}
		if (msg.GetDataSize() == sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_LISTGADGETCONNS");
			return type;
		}
    FXPropertyKit pk ;
    pk = (LPCTSTR) (pData + 1);
		int t;
		if (!pk.GetInt("type", t))
		{
			SENDERR_0("Wrong data format in params of MSG_LISTGADGETCONNS: no type");
			return type;
		}
		type = (UINT)t;
		int i = 0;
		FXString label, str;
		label.Format("input%d", i++);
		while (pk.GetString(label, str))
		{
			inputs.Add(str);
			label.Format("input%d", i++);
		}
		i = 0;
		label.Format("output%d", i++);
		while (pk.GetString(label, str))
		{
			outputs.Add(str);
			label.Format("output%d", i++);
		}
		i = 0;
		label.Format("duplex%d", i++);
		while (pk.GetString(label, str))
		{
			duplex.Add(str);
			label.Format("duplex%d", i++);
		}
		return type;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return type;
}

#undef THIS_MODULENAME
//#define THIS_MODULENAME "NetBuilder::CreateGadget"

BOOL CNetBuilder::RegisterGadgetClass(CRuntimeGadget* RuntimeGadget)
{
	return FALSE;
}

void CNetBuilder::UnregisterGadgetClass(CRuntimeGadget* RuntimeGadget)
{
}

CGadget* CNetBuilder::GetGadget(LPCTSTR uid)
{
	//TODO;
	return NULL;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::KindOf"

UINT CNetBuilder::KindOf(FXString& uid)
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_KINDOF), (void*)LPCTSTR(uid), (int) uid.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_KINDOF");
			return TVDB400_GT_ANY;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return TVDB400_GT_ANY;
		}
		if (msg.GetDataSize() < 2 * sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server after ERR_OK in params of MSG_KINDOF");
			return TVDB400_GT_ANY;
		}
		UINT type = (UINT)(*(pData + 1));
		return type;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return TVDB400_GT_ANY;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::AggregateBlock"

BOOL CNetBuilder::AggregateBlock(FXString& uid, CStringArray& Gadgets, LPCTSTR loadPath)
{
	FXPropertyKit pk;
	pk.WriteString("uid", uid);
	int i;
	CString label;
	for (i = 0; i < Gadgets.GetSize(); i++)
	{
		label.Format("uid%d", i);
		pk.WriteString(label, Gadgets.GetAt(i));
	}
	if (loadPath)
		pk.WriteString("path", loadPath);
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_AGGRBLOCK), (void*)LPCTSTR(pk), (int) pk.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_AGGRBLOCK");
			return FALSE;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return FALSE;
		}
		if (msg.GetDataSize() == sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_AGGRBLOCK");
			return FALSE;
		}
    FXPropertyKit pk ;
    pk = (LPCTSTR) (pData + 1);
    if ( !pk.GetString( "uid" , uid ) )
		{
			SENDERR_0("Wrong data format in params of MSG_AGGRBLOCK: no uid");
			return FALSE;
		}
		int iResult;
		if (!pk.GetInt("result", iResult))
		{
			SENDERR_0("Wrong data format in params of MSG_AGGRBLOCK: no result");
			return FALSE;
		}
		return (BOOL)iResult;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return FALSE;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::ExtractBlock"

BOOL CNetBuilder::ExtractBlock(FXString& uid, CMapStringToString* renames)
{
	FXPropertyKit pk;
	pk.WriteString("uid", uid);
	if (renames)
		pk.WriteBool("renames", true);
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_EXTRBLOCK), (void*)LPCTSTR(pk), (int) pk.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_EXTRBLOCK");
			return FALSE;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return FALSE;
		}
		if (msg.GetDataSize() == sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_EXTRBLOCK");
			return FALSE;
		}
    FXPropertyKit pk ;
    pk = (LPCTSTR) (pData + 1);
    if ( !pk.GetString( "uid" , uid ) )
		{
			SENDERR_0("Wrong data format in params of MSG_EXTRBLOCK: no uid");
			return FALSE;
		}
		int iResult;
		if (!pk.GetInt("result", iResult))
		{
			SENDERR_0("Wrong data format in params of MSG_EXTRBLOCK: no result");
			return FALSE;
		}
		if (renames)
		{
			int i = 0;
			FXString label, uidFrom, uidTo;
			label.Format("uidFrom%d", i);
			while (pk.GetString(label, uidFrom))
			{
				label.Format("uidTo%d", i++);
				if (!pk.GetString(label, uidTo))
				{
					SENDERR_1("Wrong data format in params of MSG_EXTRBLOCK: no match for renamed %s", uidFrom);
					return FALSE;
				}
				renames->SetAt(uidFrom, uidTo);
				label.Format("uidFrom%d", i);
			}
		}
		return (BOOL)iResult;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return FALSE;
}

#undef THIS_MODULENAME
//#define THIS_MODULENAME "NetBuilder::CreateGadget"

BOOL CNetBuilder::ConnectRendererAndMonitor(LPCTSTR uid, CWnd* pParentWnd, LPCTSTR Monitor, CRenderGadget* &RenderGadget)
{
	//TODO Implement the 'BOOL CNetBuilder::ConnectRendererAndMonitor(LPCTSTR uid, CWnd* pParentWnd, LPCTSTR Monitor, CRenderGadget* &RenderGadget)';
	return FALSE;
}

BOOL CNetBuilder::SetRendererCallback(FXString& uid, RenderCallBack rcb, void* cbData)
{
	//TODO;
	return FALSE;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::GetRenderMonitor"

BOOL CNetBuilder::GetRenderMonitor(FXString& uid, FXString& monitor)
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_GETMONITOR), (void*)LPCTSTR(uid), (int) uid.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_GETMONITOR");
			return FALSE;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return FALSE;
		}
		if (msg.GetDataSize() <= 2 * sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_GETMONITOR");
			return FALSE;
		}
		BOOL bResult = (BOOL)(*(pData + 1));
		monitor = (LPCTSTR)(pData + 2);
		return bResult;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return FALSE;
	return FALSE;
}

#undef THIS_MODULENAME
//#define THIS_MODULENAME "NetBuilder::GetRenderMonitor"

BOOL CNetBuilder::SetOutputCallback(LPCTSTR idPin, OutputCallback ocb, void* pClient)
{
	//TODO;
	return FALSE;
}
BOOL CNetBuilder::SendDataFrame(CDataFrame* pDataFrame, LPCTSTR idPin)
{
	//TODO;
	return FALSE;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::Connect"

BOOL CNetBuilder::Connect(const FXString& uid1, const FXString& uid2)
{
	BOOL bResult = FALSE;
	FXPropertyKit pk;
	pk.WriteString("pin1", uid1);
	pk.WriteString("pin2", uid2);
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_CONNECT), (void*)LPCTSTR(pk), (int) pk.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_CONNECT");
			return FALSE;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return FALSE;
		}
		if (msg.GetDataSize() < 2 * sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server after ERR_OK in params of MSG_CONNECT");
			return FALSE;
		}
		bResult = (BOOL)(*(pData + 1));
		return bResult;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return FALSE;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::Disconnect"

BOOL CNetBuilder::Disconnect(LPCTSTR uidConnector)
{
	FXPropertyKit pk;
	pk.WriteString("pin1", uidConnector);
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_DISCONNECT), (void*)LPCTSTR(pk), (int) pk.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_DISCONNECT");
			return FALSE;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return FALSE;
		}
		if (msg.GetDataSize() < 2 * sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server after ERR_OK in params of MSG_DISCONNECT");
			return FALSE;
		}
		BOOL bResult = (BOOL)(*(pData + 1));
		return bResult;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return FALSE;
}

BOOL CNetBuilder::Disconnect(LPCTSTR pin1, LPCTSTR pin2)
{
	FXPropertyKit pk;
	pk.WriteString("pin1", pin1);
	pk.WriteString("pin2", pin2);
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_DISCONNECT), (void*)LPCTSTR(pk), (int) pk.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_DISCONNECT");
			return FALSE;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return FALSE;
		}
		if (msg.GetDataSize() < 2 * sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server after ERR_OK in params of MSG_DISCONNECT");
			return FALSE;
		}
		BOOL bResult = (BOOL)(*(pData + 1));
		return bResult;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return FALSE;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::IsConnected"

BOOL CNetBuilder::IsConnected(LPCTSTR uidConnector, CStringArray& uidTo)
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_ISCONNECTED), (void*)uidConnector, (int) _tcslen(uidConnector) + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_ISCONNECTED");
			return FALSE;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return FALSE;
		}
		if (msg.GetDataSize() <= 2 * sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_ISCONNECTED");
			return FALSE;
		}
		BOOL bResult = (BOOL)(*(pData + 1));
    FXPropertyKit pk ;
    pk = (LPCTSTR) (pData + 2);
    int i = 0;
		FXString label, pin;
		label.Format("pin%d", i++);
		while (pk.GetString(label, pin))
		{
			uidTo.Add(pin);
			label.Format("pin%d", i++);
		}
		return bResult;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return FALSE;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::IsValid"

BOOL CNetBuilder::IsValid(LPCTSTR UID)
{
    ASSERT(FALSE);
    return TRUE;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::EnumGadgetLinks"

void CNetBuilder::EnumGadgetLinks(FXString& uidGadget, CStringArray& dstGadgets)
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_ENUMGADGETLINKS), (void*)LPCTSTR(uidGadget), 
    (int) uidGadget.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_ENUMGADGETLINKS");
			return;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return;
		}
		if (msg.GetDataSize() == sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_ENUMGADGETLINKS");
			return;
		}
		FXString params = (LPCTSTR)(pData + 1);
		FXPropertyKit pk(params);
		if (!pk.GetString("uid", uidGadget))
		{
			SENDERR_0("Wrong data format in params of MSG_ENUMGADGETLINKS: no uid");
			return;
		}
		int i = 0;
		FXString label, dst;
		label.Format("dst%d", i++);
		while (pk.GetString(label, dst))
		{
			dstGadgets.Add(dst);
			label.Format("dst%d", i++);
		}
		return;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::GetOutputIDs"

BOOL CNetBuilder::GetOutputIDs(const FXString& uidPin, FXString& uidOut, CStringArray* uidInComplementary)
{
	FXPropertyKit pk;
	pk.WriteString("pin", uidPin);
	if (uidInComplementary)
		pk.WriteBool("incomp", true);
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_GETOUTPUTIDS), (void*)LPCTSTR(pk), (int) pk.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_GETOUTPUTIDS");
			return FALSE;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return FALSE;
		}
		if (msg.GetDataSize() == sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_GETOUTPUTIDS");
			return FALSE;
		}
    FXPropertyKit pk ;
    pk = (LPCTSTR) (pData + 1);
    if ( !pk.GetString( "out" , uidOut ) )
		{
			SENDERR_0("Wrong data format in params of MSG_GETOUTPUTIDS: no out");
			return FALSE;
		}
		int iResult;
		if (!pk.GetInt("result", iResult))
		{
			SENDERR_0("Wrong data format in params of MSG_GETOUTPUTIDS: no result");
			return FALSE;
		}
		if (uidInComplementary)
		{
			int i = 0;
			FXString label, uidIn;
			label.Format("uidIn%d", i++);
			while (pk.GetString(label, uidIn))
			{
				uidInComplementary->Add(uidIn);
				label.Format("uidIn%d", i++);
			}
		}
		return (BOOL)iResult;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return FALSE;
}

#undef THIS_MODULENAME
//#define THIS_MODULENAME "NetBuilder::EnumGadgetLinks"

BOOL CNetBuilder::RenderDebugOutput(FXString& uidPin, CWnd* pRenderWnd)
{
	//TODO;
	return FALSE;
}

BOOL CNetBuilder::RenderDebugInput(FXString& uidPin, CWnd* pRenderWnd)
{
	//TODO;
	return FALSE;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::GetElementInfo"

BOOL CNetBuilder::GetElementInfo(FXString& uid, FXString& infostring)
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_GETELEMENTINFO), (void*)LPCTSTR(uid), (int) uid.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_GETELEMENTINFO");
			return FALSE;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return FALSE;
		}
		if (msg.GetDataSize() <= 2 * sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_GETELEMENTINFO");
			return FALSE;
		}
		BOOL bResult = (BOOL)(*(pData + 1));
		infostring = (LPCTSTR)(pData + 2);
		return bResult;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return FALSE;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::RenameGadget"

BOOL CNetBuilder::RenameGadget(FXString& uidOld, FXString& uidNew, CStringArray* uidsToChange, CStringArray* uidsNewNames)
{
	FXPropertyKit pk;
	pk.WriteString("old", uidOld);
	pk.WriteString("new", uidNew);
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_RENAMEGADGET), (void*)LPCTSTR(pk), (int) pk.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_RENAMEGADGET");
			return FALSE;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return FALSE;
		}
		if (msg.GetDataSize() <= 2 * sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server after ERR_OK in params of MSG_RENAMEGADGET");
			return FALSE;
		}
		BOOL bResult = (BOOL)(*(pData + 1));
    FXPropertyKit pk ;
    pk = (LPCTSTR) (pData + 2);
    if ( !pk.GetString( "new" , uidNew ) )
		{
			SENDERR_0("Wrong data format in params of MSG_RENAMEGADGET: no new");
			return FALSE;
		}
		int i = 0;
		FXString label, uid;
		label.Format("chng%d", i++);
		while (pk.GetString(label, uid))
		{
			uidsToChange->Add(uid);
			label.Format("chng%d", i++);
		}
		i = 0;
		label.Format("name%d", i++);
		while (pk.GetString(label, uid))
		{
			uidsNewNames->Add(uid);
			label.Format("name%d", i++);
		}
		return bResult;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return FALSE;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::GetPinLabel"

BOOL CNetBuilder::GetPinLabel(FXString& uidPin, FXString& label)
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_GETPINLABEL), (void*)LPCTSTR(uidPin), (int) uidPin.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_GETPINLABEL");
			return FALSE;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return FALSE;
		}
		if (msg.GetDataSize() <= 2 * sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_GETPINLABEL");
			return FALSE;
		}
		BOOL bResult = (BOOL)(*(pData + 1));
		label = (LPCTSTR)(pData + 2);
		return bResult;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return FALSE;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::SetPinLabel"

BOOL CNetBuilder::SetPinLabel(FXString& uidPin, LPCTSTR label)
{
	FXPropertyKit pk;
	pk.WriteString("pin", uidPin);
	pk.WriteString("label", label);
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_SETPINLABEL), (void*)LPCTSTR(pk), (int) pk.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_SETPINLABEL");
			return FALSE;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return FALSE;
		}
		if (msg.GetDataSize() < 2 * sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server after ERR_OK in params of MSG_SETPINLABEL");
			return FALSE;
		}
		BOOL bResult = (BOOL)(*(pData + 1));
		return bResult;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return FALSE;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::IsGadgetSetupOn"

BOOL CNetBuilder::IsGadgetSetupOn(FXString& uid)
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_ISGADGETSETUPON), (void*)LPCTSTR(uid), (int) uid.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_ISGADGETSETUPON");
			return FALSE;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return FALSE;
		}
		if (msg.GetDataSize() < 2 * sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_ISGADGETSETUPON");
			return FALSE;
		}
		BOOL bResult = (BOOL)(*(pData + 1));
		return bResult;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return FALSE;
}

#undef THIS_MODULENAME

BOOL CNetBuilder::GetGadgetMode(FXString& uid, int& mode)
{
	//TODO;
	return FALSE;
}

BOOL CNetBuilder::SetGadgetMode(FXString& uid, int mode)
{
	//TODO;
	return FALSE;
}

BOOL CNetBuilder::GetGadgetThreadsNumber(FXString& uid, int& n)
{
	// TODO;
	return FALSE;
}

BOOL CNetBuilder::GetOutputMode(FXString& uid, CFilterGadget::OutputMode& mode)
{
	// TODO;
	return FALSE;
}

BOOL CNetBuilder::SetOutputMode(FXString& uid, CFilterGadget::OutputMode mode)
{
	// TODO;
	return FALSE;
}

BOOL CNetBuilder::SetGadgetThreadsNumber(FXString& uid, int n)
{
	// TODO;
	return FALSE;
}

BOOL CNetBuilder::SetGadgetStatus(FXString& uid, LPCTSTR statusname, bool status)
{
    return FALSE;
}
BOOL CNetBuilder::GetGadgetStatus(FXString& uid, LPCTSTR statusname, bool& status)
{
return TRUE ;
}
BOOL CNetBuilder::GetGadgetIsMultyCoreAllowed(FXString& uid)
{
	// TODO;
	return FALSE;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::GetGadgetClassAndLineage"

UINT CNetBuilder::GetGadgetClassAndLineage(FXString& uidGadget, FXString& Class, FXString& Lineage)
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_GETGADGETCLSLNG), (void*)LPCTSTR(uidGadget), (int) uidGadget.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_GETGADGETCLSLNG");
			return TVDB400_GT_ANY;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return TVDB400_GT_ANY;
		}
		if (msg.GetDataSize() <= 2 * sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server after ERR_OK in params of MSG_GETGADGETCLSLNG");
			return TVDB400_GT_ANY;
		}
		UINT Result = (UINT)(*(pData + 1));
    FXPropertyKit pk ;
    pk = (LPCTSTR) (pData + 2);
    if ( !pk.GetString( "class" , Class ) )
		{
			SENDERR_0("Wrong data format in params of MSG_GETGADGETCLSLNG: no class");
			return TVDB400_GT_ANY;
		}
		if (!pk.GetString("lineage", Lineage))
		{
			SENDERR_0("Wrong data format in params of MSG_GETGADGETCLSLNG: no lineage");
			return TVDB400_GT_ANY;
		}
		return Result;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return 0;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::EnumGadgets"

void CNetBuilder::EnumGadgets(CStringArray& srcGadgets, CStringArray& dstGadgets)
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_ENUMGADGETS), NULL, 0);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_ENUMGADGETS");
			return;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return;
		}
		if (msg.GetDataSize() == sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_ENUMGADGETS");
			return;
		}
		FXString params = (LPCTSTR)(pData + 1);
		FXPropertyKit pk(params); 
		int i = 0;
		FXString label, uid;
		label.Format("src%d", i++);
		while (pk.GetString(label, uid))
		{
			srcGadgets.Add(uid);
			label.Format("src%d", i++);
		}
		i = 0;
		label.Format("dst%d", i++);
		while (pk.GetString(label, uid))
		{
			dstGadgets.Add(uid);
			label.Format("dst%d", i++);
		}
		return;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::EnumGadgetClassesAndLineages"

void CNetBuilder::EnumGadgetClassesAndLineages(CUIntArray& Types, CStringArray& Classes, CStringArray& Lineages)
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_ENUMCLSLNG), NULL, 0);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_ENUMCLSLNG");
			return;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return;
		}
		if (msg.GetDataSize() == sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_ENUMCLSLNG");
			return;
		}
		FXString params = (LPCTSTR)(pData + 1);
		FXPropertyKit pk(params);
		int i = 0, type;
		FXString label, cls, lng;
		label.Format("type%d", i);
		while (pk.GetInt(label, type))
		{
			label.Format("class%d", i);
			if (!pk.GetString(label, cls))
			{
				SENDWARN_0("Not enough classes listed");
				break;
			}
			label.Format("lineage%d", i);
			if (!pk.GetString(label, lng))
			{
				SENDWARN_0("Not enough lineages listed");
				break;
			}
			Types.Add((UINT)type);
			Classes.Add(cls);
			Lineages.Add(lng);
			i++;
			label.Format("type%d", i);
		}
		return;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::EnumGadgetClassesAndPlugins"

void CNetBuilder::EnumGadgetClassesAndPlugins(CStringArray& Classes, CStringArray& Plugins)
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_ENUMCLSPLG), NULL, 0);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_ENUMCLSPLG");
			return;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return;
		}
		if (msg.GetDataSize() == sizeof(DWORD))
		{
			SENDERR_0("No data from server after ERR_OK in params of MSG_ENUMCLSPLG");
			return;
		}
		FXString params = (LPCTSTR)(pData + 1);
		FXPropertyKit pk(params);
		int i = 0;
		FXString label, cls, plg;
		label.Format("class%d", i);
		while (pk.GetString(label, cls))
		{
			label.Format("plugin%d", i);
			if (!pk.GetString(label, plg))
			{
				SENDWARN_0("Not enough plugins listed");
				break;
			}
			Classes.Add(cls);
			Plugins.Add(plg);
			i++;
			label.Format("class%d", i);
		}
		return;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
}

#undef THIS_MODULENAME
//#define THIS_MODULENAME "NetBuilder::EnumGadgetClassesAndPlugins"
BOOL CNetBuilder::IsComplexGadget(FXString& uid)
{
	//TODO;
    return FALSE;
}

BOOL CNetBuilder::IsLibraryComplexGadget(FXString& uid)
{
	//TODO;
    return FALSE;
}

void CNetBuilder::SetLocalComplexGadget(FXString& uid)
{
	//TODO;
}

IGraphbuilder* CNetBuilder::GetSubBuilder(FXString& uid)
{
	//TODO;
	return NULL;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::EnumModifiedGadgets"

BOOL CNetBuilder::EnumModifiedGadgets(CStringArray& Gadgets)
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_ENUMMODGADGETS), NULL, 0);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_ENUMMODGADGETS");
			return FALSE;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return FALSE;
		}
		if (msg.GetDataSize() <= 2 * sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server after ERR_OK in params of MSG_ENUMMODGADGETS");
			return FALSE;
		}
		BOOL bResult = (BOOL)(*(pData + 1));
    FXPropertyKit pk ;
    pk = (LPCTSTR) (pData + 2);
    FXString label , gadget;
		int i = 0;
		label.Format("gadget%d", i++);
		while (pk.GetString(label, gadget))
		{
			Gadgets.Add(gadget);
			label.Format("gadget%d", i++);
		}
		return bResult;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return FALSE;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::GetGraphInputsCount"

int CNetBuilder::GetGraphInputsCount()
{
	CString param = "in";
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_GETGRAPHPINSCOUNT), (void*)LPCTSTR(param), param.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_GETGRAPHPINSCOUNT (\"in\")");
			return 0;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return 0;
		}
		if (msg.GetDataSize() < 2 * sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server after ERR_OK in params of MSG_GETGRAPHPINSCOUNT (\"in\")");
			return 0;
		}
		int count = (int)(*(pData + 1));
		return count;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return 0;
}

#undef THIS_MODULENAME
//#define THIS_MODULENAME "NetBuilder::EnumGadgetClassesAndPlugins"

CInputConnector* CNetBuilder::GetGraphInput(int n)
{
//	TODO;
	return NULL;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::GetGraphOutputsCount"

int CNetBuilder::GetGraphOutputsCount()
{
	CString param = "out";
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_GETGRAPHPINSCOUNT), (void*)LPCTSTR(param), param.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_GETGRAPHPINSCOUNT (\"out\")");
			return 0;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return 0;
		}
		if (msg.GetDataSize() < 2 * sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server after ERR_OK in params of MSG_GETGRAPHPINSCOUNT (\"out\")");
			return 0;
		}
		int count = (int)(*(pData + 1));
		return count;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return 0;
}

#undef THIS_MODULENAME
//#define THIS_MODULENAME "NetBuilder::GetGraphOutputsCount"

COutputConnector* CNetBuilder::GetGraphOutput(int n)
{
//	TODO;
	return NULL;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::GetGraphDuplexPinsCount"

int CNetBuilder::GetGraphDuplexPinsCount()
{
	CString param = "duplex";
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_GETGRAPHPINSCOUNT), (void*)LPCTSTR(param), param.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_GETGRAPHPINSCOUNT (\"duplex\")");
			return 0;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return 0;
		}
		if (msg.GetDataSize() < 2 * sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server after ERR_OK in params of MSG_GETGRAPHPINSCOUNT (\"duplex\")");
			return 0;
		}
		int count = (int)(*(pData + 1));
		return count;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return 0;
}

#undef THIS_MODULENAME
//#define THIS_MODULENAME "NetBuilder::GetGraphOutputsCount"

CDuplexConnector* CNetBuilder::GetGraphDuplexPin(int n)
{
//	TODO;
	return NULL;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::ScanProperties"

bool CNetBuilder::ScanProperties(LPCTSTR gadgetUID, LPCTSTR text, bool& Invalidate)
{
	FXPropertyKit pk;
	pk.WriteString("uid", gadgetUID);
	pk.WriteString("text", text);
	pk.WriteBool("invalidate", Invalidate);
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_SCANPROPERTIES), (void*)LPCTSTR(pk), (int) pk.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_SCANPROPERTIES");
			return false;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return false;
		}
		if (msg.GetDataSize() == sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server after ERR_OK in params of MSG_SCANPROPERTIES");
			return false;
		}
    FXPropertyKit pk ;
    pk = (LPCTSTR) (pData + 1);
		if (!pk.GetBool("invalidate", Invalidate))
		{
			SENDERR_0("Wrong data format in params of MSG_SCANPROPERTIES: no invalidate");
			return false;
		}
		bool result;
		if (!pk.GetBool("result", result))
		{
			SENDERR_0("Wrong data format in params of MSG_SCANPROPERTIES: no result");
			return false;
		}
		return result;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return false;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::PrintProperties"

bool CNetBuilder::PrintProperties(LPCTSTR gadgetUID, FXString& text)
{
	CString uid = gadgetUID;
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_PRINTPROPERTIES), (void*)LPCTSTR(uid), (int) uid.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_PRINTPROPERTIES");
			return false;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return false;
		}
		if (msg.GetDataSize() == sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server after ERR_OK in params of MSG_PRINTPROPERTIES");
			return false;
		}
    FXPropertyKit pk ;
    pk = (LPCTSTR) (pData + 1);
    if ( !pk.GetString( "text" , text ) )
		{
			SENDERR_0("Wrong data format in params of MSG_PRINTPROPERTIES: no text");
			return false;
		}
		bool result;
		if (!pk.GetBool("result", result))
		{
			SENDERR_0("Wrong data format in params of MSG_PRINTPROPERTIES: no result");
			return false;
		}
		return result;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return false;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::ScanSettings"

bool CNetBuilder::ScanSettings(LPCTSTR gadgetUID, FXString& text)
{
	FXPropertyKit pk;
	pk.WriteString("uid", gadgetUID);
	pk.WriteString("text", text);
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_SCANSETTINGS), (void*)LPCTSTR(pk), (int) pk.GetLength() + 1);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_SCANSETTINGS");
			return false;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return false;
		}
		if (msg.GetDataSize() == sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server after ERR_OK in params of MSG_SCANSETTINGS");
			return false;
		}
    FXPropertyKit pk ;
    pk = (LPCTSTR) (pData + 1);
    if ( !pk.GetString( "text" , text ) )
		{
			SENDERR_0("Wrong data format in params of MSG_SCANSETTINGS: no text");
			return false;
		}
		bool result;
		if (!pk.GetBool("result", result))
		{
			SENDERR_0("Wrong data format in params of MSG_SCANSETTINGS: no result");
			return false;
		}
		return result;
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
	return false;
}

#undef THIS_MODULENAME
//#define THIS_MODULENAME "NetBuilder::ScanProperties"

void CNetBuilder::SetExecutionStatus(CExecutionStatus* Status)
{
	//TODO;
}

IPluginLoader* CNetBuilder::GetPluginLoader()
{
//	TODO;
	return NULL;
}

void CNetBuilder::EnumPluginLoaders(CPtrArray& PluginLoaders)
{
//	TODO;
}

UINT CNetBuilder::RegisterGadget(FXString&, void*)
{
	//TODO;
	return 0;
}

void CNetBuilder::GrantGadgets(IGraphbuilder* pBuilder, CStringArray* uids, CMapStringToString* renames)
{
	//TODO;
}

#undef THIS_MODULENAME
#define THIS_MODULENAME "NetBuilder::Detach"

void CNetBuilder::Detach()
{
	CSHSMessage msg(m_idNetClient, MsgUid(CSHSMessage::MSG_DETACH), NULL, 0);
	if (Transact(msg))
	{
		if (msg.GetDataSize() < sizeof(DWORD))
		{
			SENDERR_0("Not enough data from server (< 4 Bytes) in params of MSG_DETACH");
			return;
		}
		DWORD* pData = (DWORD*)msg.GetData();
		if (*pData != CSHSMessage::ERR_OK)
		{
			SENDERR_1("Server returned error %s", msg.VerboseError(*pData));
			return;
		}
		SENDINFO_1("Builder 0x%p detached", this);
	}
	else
	{
		DWORD* pData = (DWORD*)msg.GetData();
		ASSERT(pData && (msg.GetDataSize() >= sizeof(DWORD)));
		SENDERR_1("Transaction failed with error %s", msg.VerboseError(*pData));
	}
}

int CNetBuilder::EnumAndArrangeGadgets( FXStringArray& GadgetList )
{
  CStringArray SrcGadgets ;
  CStringArray DstGadgets ;
  EnumGadgets( SrcGadgets , DstGadgets ) ;

  for ( int i = 0 ; i < SrcGadgets.GetCount() ; i++ )
    GadgetList.Add( (LPCTSTR) SrcGadgets[ i ] ) ;
  for ( int i = 0 ; i < DstGadgets.GetCount() ; i++ )
    GadgetList.Add( (LPCTSTR) DstGadgets[ i ] ) ;
  qsort( (void*) &GadgetList[ 0 ] , GadgetList.GetCount() ,
    sizeof( FXString* ) , FXStringCompareAscending ) ;

  return 0;
}

#undef THIS_MODULENAME

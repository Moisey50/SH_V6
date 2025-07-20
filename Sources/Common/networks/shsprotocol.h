#pragma once

#define CASE(e)	case(##e): return _T(#e);

class CSHSMessage
{
#pragma warning(disable:4200)
	typedef struct tagSHSMSG
	{
		DWORD idSender;
		DWORD idMsg;
		BYTE lpMsg[1];
	}SHSMSG, *LPSHSMSG;
#pragma warning(default:420)
	LPSHSMSG m_pMsg;
	DWORD m_cbData;
	BOOL m_bCopy;
public:
	CSHSMessage(LPVOID pData, int cbData)
	{
		m_pMsg = (LPSHSMSG)pData;
		m_cbData = (DWORD)cbData - sizeof(SHSMSG);
		m_bCopy = FALSE;
	}
	CSHSMessage(DWORD idSender, DWORD idMsg, void* lpData, DWORD cbData)
	{
		m_cbData = cbData;
		m_pMsg = (LPSHSMSG)malloc(GetTotalSize());
		m_pMsg->idSender = idSender;
		m_pMsg->idMsg = idMsg;
		if (lpData && cbData)
			memcpy(m_pMsg->lpMsg, lpData, cbData);
		m_bCopy = TRUE;
	}
	~CSHSMessage()
	{
		if (m_bCopy)
			free(m_pMsg);
	}
	CSHSMessage& operator = (CSHSMessage& msg)
	{
		if (m_bCopy)
			free(m_pMsg);
		m_pMsg = (LPSHSMSG)malloc(msg.GetTotalSize());
		memcpy(m_pMsg, msg.m_pMsg, msg.GetTotalSize());
		m_cbData = msg.m_cbData;
		m_bCopy = TRUE;
		return *this;
	};
	DWORD GetSender() { return m_pMsg->idSender; };
	DWORD GetID() { return m_pMsg->idMsg; };
	DWORD GetDataSize() { return m_cbData; };
	LPBYTE GetData() { return m_pMsg->lpMsg; };
	DWORD GetTotalSize() { return m_cbData + sizeof(SHSMSG); };
	LPVOID GetMsgToSend(int& cbTotal, BOOL bAutoDelete = FALSE) { cbTotal = (int)GetTotalSize(); m_bCopy = bAutoDelete; return (LPVOID)m_pMsg; };

public:
	enum
	{
		// query & response		// query lpMsg:				// response lpMsg:
		MSG_NEWGRAPH,			// - (idSender=0)			// ERR_OK + DWORD <new idSender for client> / ERR_??? [+ context_info]
		MSG_LOADGRAPH,			// <name,bClear (idSender=0)// ERR_OK + DWORD <new idSender for client> / ERR_??? [+ context_info]
		MSG_LOADSCRIPT,			// <script, bool bClear>	// ERR_OK + <iResult> / ERR_??? [+ context_info]
		MSG_CLOSEGRAPH,			// -						// ERR_OK / ERR_??? [+ context_info]
		MSG_ENUMGRAPHS,			// -						// ERR_OK + <serialized array of graphs' ids and params> / ERR_??? [+ context_info]

		MSG_START,				// -						// ERR_OK / ERR_??? [+ context_info]
		MSG_STOP,				// -						// ERR_OK / ERR_??? [+ context_info]
		MSG_PAUSE,				// -						// ERR_OK / ERR_??? [+ context_info]
		MSG_STEPFWD,			// -						// ERR_OK / ERR_??? [+ context_info]
		MSG_ISRUNING,			// -						// ERR_OK + BOOL <isRuning> / ERR_??? [+ context_info]
		MSG_GETID,				// -						// ERR_OK + <string param> / ERR_??? [+ context_info]
		MSG_SETID,				// <string param>			// ERR_OK / ERR_??? [+ context_info]
		MSG_SETDIRTY,			// <BOOL param>				// ERR_OK + < BOOL was dirty> / ERR_??? [+ context_info]
		MSG_ISDIRTY,			// -						// ERR_OK + < BOOL isDirty> / ERR_??? [+ context_info]
		MSG_DETACH,				// -						// ERR_OK / ERR_??? [+ context_info]
		MSG_SAVE,				// <filename[, uids list]>	// ERR_OK / ERR_??? [+ context_info]
		MSG_GETSCRIPTPATH,		// <uid>					// ERR_OK + <BOOL bResult, path> / ERR_??? [+ context_info]
		MSG_GETSCRIPT,			// -						// ERR_OK + <string script> / ERR_??? [+ context_info]
		MSG_GETGRAPHPROPS,		// -						// ERR_OK + <string properties> / ERR_??? [+ context_info]
		MSG_SETGRAPHPROPS,		// <string properties>		// ERR_OK / ERR_??? [+ context_info]
		MSG_GETTIME,			// -						// ERR_OK + <double time> / ERR_??? [+ context_info]
		MSG_CREATEGADGET,		// <classname>				// ERR_OK / ERR_??? [+ context_info]
		MSG_REGCURGADGET,		// <uid, params list>		// ERR_OK + <uid, type> / ERR_??? [+ context_info]
		MSG_UNREGGADGET,		// <uid>					// ERR_OK / ERR_??? [+ context_info]
		MSG_LISTGADGETCONNS,	// <uid>					// ERR_OK + <type, arrays of inputs, outputs, duplex> / ERR_??? [+ context_info]

		MSG_KINDOF,				// <uid>					// ERR_OK + <type> / ERR_??? [+ context_info]
		MSG_AGGRBLOCK,			// <uid, gadgets, loadpath> // ERR_OK + <BOOL result, uid> / ERR_??? [+ context_info]
		MSG_EXTRBLOCK,			// <uid, bool bRenamesSet>	// ERR_OK + <BOOL result, uid, renames list> / ERR_??? [+ context_info]

		MSG_GETMONITOR,			// <uid>					// ERR_OK + <BOOL result, monitor> / ERR_??? [+ context_info]

		MSG_CONNECT,			// <uid1, uid2>				// ERR_OK + <BOOL result> / ERR_??? [+ context_info]
		MSG_DISCONNECT,			// <uid1 [, uid2]>			// ERR_OK + <BOOL result> / ERR_??? [+ context_info]
		MSG_ISCONNECTED,		// <pin>					// ERR_OK + <BOOL result, array of pins> / ERR_??? [+ context_info]
		MSG_ENUMGADGETLINKS,	// <gadget uid>				// ERR_OK + <array of dst gadgets> / ERR_??? [+ context_info]
		MSG_GETOUTPUTIDS,		// <pin, bool bComplement.>	// ERR_OK + <BOOL result, out, list of compl.> / ERR_??? [+ context_info]

		MSG_GETELEMENTINFO,		// <uid>					// ERR_OK + <BOOL result, string info> / ERR_??? [+ context_info]
		MSG_RENAMEGADGET,		// <uidOld, uidNew>			// ERR_OK + <BOOL result, uidNew, list "ToChange", list "NewNames"> / ERR_??? [+ context_info]
		MSG_GETPINLABEL,		// <pin>					// ERR_OK + <BOOL result, label> / ERR_??? [+ context_info]
		MSG_SETPINLABEL,		// <pin, label>				// ERR_OK + <BOOL result> / ERR_??? [+ context_info]
		MSG_ISGADGETSETUPON,	// <uid>					// ERR_OK + <BOOL result> / ERR_??? [+ context_info]

		MSG_GETGADGETCLSLNG,	// <uid>					// ERR_OK + <type, class, lineage> / ERR_??? [+ context_info]
		MSG_ENUMGADGETS,		// -						// ERR_OK + <arrays of src & dst gadgets> / ERR_??? [+ context_info]
		MSG_ENUMCLSLNG,			// -						// ERR_OK + <arrays of types, classes, linages> / ERR_??? [+ context_info]
		MSG_ENUMCLSPLG,			// -						// ERR_OK + <arrays of classes and plugins> / ERR_??? [+ context_info]

		MSG_ENUMMODGADGETS,		// -						// ERR_OK + <BOOL result, gadgets' list> / ERR_??? [+ context_info]
		MSG_GETGRAPHPINSCOUNT,	// <"in","out",or "duplex">	// ERR_OK + <count> / ERR_??? [+ context_info]

		MSG_SCANPROPERTIES,		// <uid,text,bInvalidate>	// ERR_OK + <bool result, bool bInvalidate> / ERR_??? [+ context_info]
		MSG_PRINTPROPERTIES,	// <uid>					// ERR_OK + <bool result, text> / ERR_??? [+ context_info]
		MSG_SCANSETTINGS,		// <uid, text>				// ERR_OK + <bool result, text> / ERR_??? [+ context_info]
	};
	enum
	{
		// errors & results
		ERR_OK,					// success
		//  server side
		ERR_FAILCREATEBUILDER,	// failed to create a new builder for a remote client
		ERR_QUERYSYNTAX,		// syntax error in query
		ERR_FAILLOADGRAPH,		// failed to load the requested graph
		ERR_NOTFOUND,			// invalid client id: client must be registered to perform requested action
		ERR_NOGRAPHSFOUND,		// no .tvg files in default directory on server
		ERR_FAILSAVEGRAPH,		// remote builder failed to save graph with parameters passed
		ERR_UNKNOWN,			// unknown error, not expected to appear in current version
		ERR_FAILCREATEGADGET,	// remote builder failed to create a gadget of given class
		ERR_UIDNOTSET,			// gadget uid not set while needed to perform the request
		ERR_PINNOTSET,			// pint uid not set while needed to perform the request
		//  client side
		ERR_FAILCREATECLIENT,	// failed to create a new client for remote connection
		ERR_FAILSEND,			// sending of a query failed due to some low-level reason
		ERR_RESPONSETIMEOUT,	// no timely proper response
	};

	LPCTSTR VerboseError(UINT err)
	{
		switch (err)
		{
			CASE(ERR_OK);
			CASE(ERR_FAILCREATEBUILDER);
			CASE(ERR_QUERYSYNTAX);
			CASE(ERR_FAILLOADGRAPH);
			CASE(ERR_NOTFOUND);
			CASE(ERR_NOGRAPHSFOUND);
			CASE(ERR_FAILSAVEGRAPH);
			CASE(ERR_UNKNOWN);
			CASE(ERR_FAILCREATEGADGET);
			CASE(ERR_UIDNOTSET);
			CASE(ERR_PINNOTSET);
			CASE(ERR_FAILCREATECLIENT);
			CASE(ERR_FAILSEND);
			CASE(ERR_RESPONSETIMEOUT);
		default:
			return _T("UNKNOWN ERROR");
		}
	};
};

// dllmain.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxwin.h>
#include <afxdllx.h>
#include "NetServer.h"
#include "NetClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static AFX_EXTENSION_MODULE NetApiDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("NetApi.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(NetApiDLL, hInstance))
			return 0;

		// Insert this DLL into the resource chain
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove this line from DllMain and put it in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.  Otherwise,
		//  the CDynLinkLibrary object will not be attached to the
		//  Regular DLL's resource chain, and serious problems will
		//  result.

		new CDynLinkLibrary(NetApiDLL);

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("NetApi.DLL Terminating!\n");

		// Terminate the library before destructors are called
		AfxTermExtensionModule(NetApiDLL);
	}
	return 1;   // ok
}

INetServer* NetApi_CreateServer(LPVOID pHost, NETAPI_ON_SERVER_RECV fnServerRecv, NETAPI_ON_NET_EVENT fnServerEvent)
{
	INetServer* Server = (INetServer*)new CNetServer(pHost, fnServerRecv, fnServerEvent);
	return Server;
}

INetClient* NetApi_CreateClient(LPVOID pHost, NETAPI_ON_CLIENT_RECV fnClientRecv, NETAPI_ON_NET_EVENT fnClientEvent)
{
	INetClient* Client = (INetClient*)new CNetClient(pHost, fnClientRecv, fnClientEvent);
	return Client;
}

void NetApi_DeleteServer(INetServer* Server)
{
	delete ((CNetServer*)Server);
}

void NetApi_DeleteClient(INetClient* Client)
{
	delete ((CNetClient*)Client);
}

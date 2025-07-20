#ifndef NWCONFIG_INC
#define NWCONFIG_INC

#define DEFAULT_SERVICE_PORT  5050
#define MAX_PENDING_CONNECTS  SOMAXCONN       // Maximum length of the queue 
                                              // of pending connections
#define CONNECTION_REQUEST_TEXT "TEXT"
#define CONNECTION_REQUEST_DATAPACKETS "DATAPACKETS"
#define CONNECTION_REQUEST_RAWTEXT "RAWTEXT"
#define CONNECTION_ANSWER		"OK"

#include <Winsock2.h>
////

typedef struct tagDataBurst
{
    DWORD dwDataSize;
    BYTE  lpData[1];
}DataBurst,*pDataBurst;

typedef bool (FAR __stdcall *NWDataCallback)(LPVOID usrData, pDataBurst data);

__forceinline DWORD reciveall(SOCKET s, char FAR * buf, DWORD len, int flags)
{
    char * pntr=buf;
    DWORD got=0;
    DWORD iRet;
    while (got!=len)
    {
        iRet=recv(s, pntr, len-got, flags);
        if ((iRet==0) || (iRet==SOCKET_ERROR))
            return SOCKET_ERROR;
        pntr+=iRet;
        got+=iRet;
    }
    return got;
}

#endif
// DirectIO.cpp: implementation of the CDirectIO class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tvgeneric.h"
#include <helpers\DirectIO.h>
#include <gadgets\gadbase.h>
#include "conio.h"
#include <Winsvc.h>

#define THIS_MODULENAME "CDirectIO"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define IOCTL_READ_PORT_UCHAR	 -1673519100 //CTL_CODE(40000, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_PORT_UCHAR	 -1673519096 //CTL_CODE(40000, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

extern CDynLinkLibrary* pThisDll;

inline int SystemVersion()
{   
	
   OSVERSIONINFOEX osvi;
   BOOL bOsVersionInfoEx;

   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

   if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
   {
      osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);     
	  if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) )
		  return 0;  
   }
   switch (osvi.dwPlatformId)
   {
   case VER_PLATFORM_WIN32_NT:
       return 2;		//WINNT
       break;
   case VER_PLATFORM_WIN32_WINDOWS:
       return 1;		//WIN9X
       break;
   }   
   return 0; 
}

inline int hwinterface_start(void)
{
    SC_HANDLE  Mgr;
    SC_HANDLE  Ser;

    Mgr = OpenSCManager (NULL, NULL,SC_MANAGER_ALL_ACCESS);
    if (Mgr == NULL)
    {							//No permission to create service
        if (GetLastError() == ERROR_ACCESS_DENIED)
        {
            Mgr = OpenSCManager (NULL, NULL,GENERIC_READ);
            Ser = OpenService(Mgr,"hwinterface",GENERIC_EXECUTE);
            if (Ser)
            {    // we have permission to start the service
                if(!StartService(Ser,0,NULL))
                {
                    CloseServiceHandle (Ser);
                    return 4; // we could open the service but unable to start
                }
            }
        }
    }
    else
    {// Successfuly opened Service Manager with full access
        Ser = OpenService(Mgr,"hwinterface",GENERIC_EXECUTE);
        if (Ser)
        {
            if(!StartService(Ser,0,NULL))
            {
                CloseServiceHandle (Ser);
                return 3; // opened the Service handle with full access permission, but unable to start
            }
            else
            {
                CloseServiceHandle (Ser);
                return 0;
            }
        }
    }
    return 1;
}

int hwinterface_inst()
{
    SC_HANDLE  Mgr;
    SC_HANDLE  Ser;
    char path[MAX_PATH];

    GetModuleFileName(pThisDll->m_hModule,path,MAX_PATH);

    GetSystemDirectory(path , sizeof(path));
    HRSRC hResource = FindResource(pThisDll->m_hResource, MAKEINTRESOURCE(IDR_BIN1), "bin");
    if(hResource)
    {
        HGLOBAL binGlob = LoadResource(pThisDll->m_hResource, hResource);
        if(binGlob)
        {
            void *binData = LockResource(binGlob);
            if(binData)
            {
                HANDLE file;
                strcat(path,"\\Drivers\\hwinterface.sys");
                file = CreateFile(path,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
                if(file!=INVALID_HANDLE_VALUE)
                {
                    DWORD size, written;
                    size = SizeofResource(pThisDll->m_hResource, hResource);
                    WriteFile(file, binData, size, &written, NULL);
                    CloseHandle(file);
                }
            }
            DeleteObject(binGlob);
        }
    }
    Mgr = OpenSCManager (NULL, NULL,SC_MANAGER_ALL_ACCESS);
    if (Mgr == NULL)
    {							//No permission to create service
        if (GetLastError() == ERROR_ACCESS_DENIED) 
        {
            SENDERR_0("error access denied");
            return 5;  // error access denied
        }
    }	
    else
    {
        Ser = CreateService (Mgr,"hwinterface","hwinterface",SERVICE_ALL_ACCESS,SERVICE_KERNEL_DRIVER,SERVICE_SYSTEM_START,SERVICE_ERROR_NORMAL,"System32\\Drivers\\hwinterface.sys",NULL,NULL,NULL,NULL,NULL);
    }
    CloseServiceHandle(Ser);
    CloseServiceHandle(Mgr);
    return 0;
}

inline int Opendriver(HANDLE& hdriver)
{
    hdriver = CreateFile("\\\\.\\hwinterface",GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(hdriver == INVALID_HANDLE_VALUE)
    {
        if(hwinterface_start())
        {
            hwinterface_inst();
            hwinterface_start();
            hdriver = CreateFile("\\\\.\\hwinterface",GENERIC_READ | GENERIC_WRITE,0,NULL, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
        }
        if(hdriver == INVALID_HANDLE_VALUE)
        {
            SENDERR_0("Error access to hwinterface service");
            return 1;
        }
    }
    return 0;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDirectIO::CDirectIO():
    m_hdriver(NULL)
{
    m_Sysver=SystemVersion();
	if(m_Sysver==2)
	{
		Opendriver(m_hdriver);
	}
}

CDirectIO::~CDirectIO()
{
	if(m_hdriver)
	{
		CloseHandle(m_hdriver);
	}

}

void  CDirectIO::Out32(short PortAddress, short data)
{
    switch(m_Sysver)
    {
    case 1:
      SENDERR( "Direct output to HW port is not supported" ) ;

      //_outp( PortAddress,data);
        break;
    case 2:
        unsigned int error;
        DWORD BytesReturned;        
        BYTE Buffer[3];
        unsigned short * pBuffer;
        pBuffer = (unsigned short *)&Buffer[0];
        *pBuffer = LOWORD(PortAddress);
        Buffer[2] = LOBYTE(data);
        error = DeviceIoControl(m_hdriver, IOCTL_WRITE_PORT_UCHAR, &Buffer, 3,NULL,0,&BytesReturned,NULL);
        if (!error)
        {
            LPVOID lpMsgBuf;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),0, (LPTSTR) &lpMsgBuf,0,NULL );
            SENDERR_0((LPCTSTR)lpMsgBuf);
            // Free the buffer.
            LocalFree( lpMsgBuf );
        }
        break;
    }
}

short CDirectIO::Inp32(short PortAddress)
{
//    BYTE retval;
    switch(m_Sysver)
    {
    case 1:
      SENDERR( "Direct input from HW port is not supported" ) ;
//      retval = _inp(PortAddress);
//        return retval;
      return 0 ;
        break;
    case 2:
        unsigned int error;
        DWORD BytesReturned;
        unsigned char Buffer[3];
        unsigned short * pBuffer;
        pBuffer = (unsigned short *)&Buffer;
        *pBuffer = LOWORD(PortAddress);
        Buffer[2] = 0;
        error = DeviceIoControl(m_hdriver,IOCTL_READ_PORT_UCHAR,&Buffer,2,&Buffer,1,&BytesReturned,NULL);
        if (!error)
        {
            LPVOID lpMsgBuf;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),0, (LPTSTR) &lpMsgBuf,0,NULL );
            SENDERR_0((LPCTSTR)lpMsgBuf);
            // Free the buffer.
            LocalFree( lpMsgBuf );
        }
        return((int)Buffer[0]);
        break;
    }
    return 0;
}

// CameraLink.cpp: implementation of the CCameraLink class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CLSer.h"
#include "clserial.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCLSer::CCLSer()
{
	m_hSerialLib = NULL;
	m_pfInit = NULL;
	m_pfRead = NULL;
	m_pfWrite = NULL;
	m_pfClose = NULL;
	m_pInstance = NULL;
	m_CamID = 0xFFFFFFFF;
	m_HandleCOM = NULL;
	m_ulPortNum = 0;
	m_ulTimeout = 50;
	m_bInitialized = false;
}

CCLSer::~CCLSer()
{
	Close();
	if(m_hSerialLib != NULL)
	{
		FreeLibrary(m_hSerialLib);
		m_hSerialLib = NULL;
		m_pfInit = NULL;
		m_pfRead = NULL;
		m_pfWrite = NULL;
		m_pfClose = NULL;
	}
	if(m_HandleCOM != NULL)
	{
		CloseHandle(m_HandleCOM);
		m_HandleCOM = NULL;
	}
}

int CCLSer::Init(LPCTSTR szFileName)
{
	m_bInitialized = false;
	int nRet = 0;
	m_dll = szFileName;
	if(m_hSerialLib != NULL)
	{
		FreeLibrary(m_hSerialLib);
		m_hSerialLib = NULL;
		m_pfInit = NULL;
		m_pfRead = NULL;
		m_pfWrite = NULL;
		m_pfClose = NULL;
	}
	if(m_HandleCOM != NULL)
	{
		CloseHandle(m_HandleCOM);
	}
	if(strncmp(szFileName,"COM",3) == 0)
	{
		COMMCONFIG ComCnf;
		m_HandleCOM = CreateFile( szFileName,
			GENERIC_READ | GENERIC_WRITE,
			0,    // exclusive access 
			NULL, // no security attributes 
			OPEN_EXISTING,
			0,
			NULL
			);
		COMMTIMEOUTS commTimeouts;

		if (m_HandleCOM == INVALID_HANDLE_VALUE) 
		{
			return -51;
		}
		else
		{
			GetCommTimeouts(m_HandleCOM,&commTimeouts);
			commTimeouts.ReadIntervalTimeout = 20;
			commTimeouts.ReadTotalTimeoutConstant = 30;
			commTimeouts.ReadTotalTimeoutMultiplier = 20;
			commTimeouts.WriteTotalTimeoutConstant = 100;
			commTimeouts.WriteTotalTimeoutMultiplier = 100;
			SetCommTimeouts(m_HandleCOM,&commTimeouts);


			DCB dcb;
			GetCommState(m_HandleCOM, &dcb);
			memcpy(&ComCnf.dcb,&dcb,sizeof(dcb));
			if((dcb.BaudRate != CBR_9600)
				||(dcb.ByteSize != 8)
				||(dcb.Parity != NOPARITY)
				||(dcb.fOutxCtsFlow != false)
				||(dcb.fOutxDsrFlow != false)
				||(dcb.fOutX != false)
				||(dcb.fInX != false))
			{
				dcb.BaudRate = CBR_9600;     // set the baud rate
				dcb.ByteSize = 8;             // data size, xmit, and rcv
				dcb.Parity = NOPARITY;        // no parity bit
				dcb.StopBits = ONESTOPBIT;    // one stop bit
				dcb.fOutxCtsFlow = false;
				dcb.fOutxDsrFlow = false;
				memcpy(&ComCnf.dcb,&dcb,sizeof(dcb));
				ComCnf.dcb.fOutX = false;
				ComCnf.dcb.fInX = false; 
			}
			memcpy(&dcb,&ComCnf.dcb,sizeof(dcb));
			SetCommState(m_HandleCOM, &dcb);
		}
	}
	else
	{
		m_hSerialLib = LoadLibrary(szFileName);
		if(m_hSerialLib == NULL)
		{
			int i = GetLastError();
			{
				LPVOID lpMsgBuf;
				FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL, i, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf, 0,NULL);
				MessageBox( NULL, (LPCTSTR)lpMsgBuf, _T("Error"), MB_OK | MB_ICONINFORMATION );
				LocalFree( lpMsgBuf );
			}
			return -51;
		}

		m_pfInit = (ptclSerInitProc)GetProcAddress(m_hSerialLib, "clSerialInit");
		if(m_pfInit == NULL)
			return -52;
		else
			m_pfRead = (ptclSerReadProc)GetProcAddress(m_hSerialLib, "clSerialRead");

		if(m_pfRead == NULL)
			return -53;
		else
			m_pfWrite = (ptclSerWriteProc)GetProcAddress(m_hSerialLib, "clSerialWrite");

		if(m_pfWrite == NULL)
			return -54;
		else
			m_pfClose = (ptclSerCloseProc)GetProcAddress(m_hSerialLib, "clSerialClose");

		if(m_pfClose == NULL)
			nRet = -55;
		else
		{
			nRet = (m_pfInit)(m_ulPortNum, &m_pInstance);
			{
				CString str;
				str.Format(_T("nRet = %d PortNum = %d \n"),nRet,m_ulPortNum);
				OutputDebugString(str);
			}
			if(nRet < 0)
			{
				m_pInstance = NULL;
			}
			else
			{
				m_bInitialized = true;
			}
		}
	}
	return nRet;
}

int CCLSer::Read(char *pcBuffer, unsigned long* pulBufSize)
{
	int ind = 0;
	int nRet = 0;
	ULONG len = 1;
	bool bRet;
	DWORD bytes;
	if(m_HandleCOM != NULL)
	{
			while((ULONG)ind < *pulBufSize)
			{
				bRet = (ReadFile(m_HandleCOM,pcBuffer+ind,len,&bytes,NULL) == TRUE);
				if(bytes == 0 )
				{
					*pulBufSize = ind;
					if(ind > 0)
					{
						nRet = 0;
					}
					else
					{
						nRet = -1;
					}
					break;
				}
				ind++;
			}
	}
	else
	{
		if(m_hSerialLib && m_pfRead && m_bInitialized)
		{
			len = *pulBufSize;
			while (ind < (int)(*pulBufSize))
			{
				len = 1;
				nRet = (m_pfRead)(m_pInstance, (char*)(pcBuffer+ind), &len, m_ulTimeout);
				if(nRet <0 || len == 0)
				{
					if(ind >0)
					{
						*pulBufSize = ind;
						nRet = 0;
					}
					break;
				}
				ind++;
			}
		}
		else
		{
			if(m_hSerialLib == NULL)
				return -510;
			if(m_pfRead == NULL)
				return -530;
			return -555;
		}
	}
	*pulBufSize = ind;
	return nRet;
}

int CCLSer::Write(char *pcBuffer, unsigned long *pulBufSize)
{
	int nRet = 0;
	nRet = -56;
	if(m_HandleCOM != NULL)
	{
		DWORD bytes;
		BOOL bRet = WriteFile(m_HandleCOM,pcBuffer,*pulBufSize,&bytes,NULL);
		nRet = (bRet? 0:-1);
	}
	else
	{
	if(m_hSerialLib && m_pfWrite && m_bInitialized)
	{
		UINT timeOut = m_ulTimeout*(*pulBufSize);
		if(timeOut > 1000)
			timeOut = 1000;
		nRet = (m_pfWrite)(m_pInstance, pcBuffer, pulBufSize, timeOut);
	}
	else
	{
		if(m_hSerialLib == NULL)
		return -510;
		if(m_pfWrite == NULL)
			return -540;
		return -555;
	}
	}
	return nRet;
}

int CCLSer::Close()
{
	int nRet = 0;
	if(m_HandleCOM != NULL)
	{
		CloseHandle(m_HandleCOM);
		m_HandleCOM = NULL;
		return nRet;
	}
	if(m_hSerialLib && m_pfClose && m_bInitialized)
	{
		CString str;
		str.Format(_T("before m_pfClose: m_pInstance = 0x%x\n"),m_pInstance);
		OutputDebugString(str);
		nRet = (m_pfClose)(m_pInstance);
		str.Format(_T("after m_pfClose: m_pInstance = 0x%x -> will be NULL\n"),m_pInstance);
		OutputDebugString(str);
		m_pInstance = NULL;
		m_bInitialized = false;
	}
	else
	{
		if(m_hSerialLib == NULL)
		return -510;
		if(m_pfClose == NULL)
			return -550;
		return -555;
	}
	return nRet;
}

int CCLSer::CheckPort(UINT port)
{
	if(m_HandleCOM != NULL)
	{
		m_ulPortNum = port;
		return 0;
	}
	else
	{
		int nRet = 0;
		UINT oldPort = m_ulPortNum;
		Close();
		CString str;
		str.Format(_T("file = %s ;portN= %d\n"),m_dll,m_ulPortNum);
		OutputDebugString(str);
		if(m_pfInit)
		{
			m_ulPortNum = port;
			nRet = (m_pfInit)(m_ulPortNum, &m_pInstance);
			str.Format(_T("init port %d with ret = %d\n"),m_ulPortNum, nRet);
			OutputDebugString(str);
			if(nRet >= 0)
			{
				m_bInitialized = true;
			}
			if(nRet == 0)
			{
				return port;
			}
			else
			{
				return nRet;
			}
		}
		else
		{
			return -52;
		}
	}
}

CString CCLSer::GetCameraName()
{
	if(m_CamName.IsEmpty())
		CheckConnectedCamera();
	return m_CamName;
}
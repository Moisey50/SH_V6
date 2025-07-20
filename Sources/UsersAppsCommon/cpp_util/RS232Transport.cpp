// RS232Transport.cpp: implementation of the CRS232Transport class.
//
//////////////////////////////////////////////////////////////////////

#include <Windows.h>
#include <stdio.h>
#include "RS232Transport.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRS232Transport::CRS232Transport()
{
	m_hComm = INVALID_HANDLE_VALUE;
}

CRS232Transport::~CRS232Transport()
{
}

bool CRS232Transport::Init( ETransportType Type, char* szAddr, int iPort )
{
	if ( Type != TT_RS232 ) 	
		return false;
	
	if ( !szAddr )
		return false;
	
	m_iPort = iPort;
	if ( !m_iPort )
		return false; //Should be 1 or more

	sprintf( m_szAddress, "%s%d", szAddr, m_iPort );
	
	return ConfigureComm();
}

bool CRS232Transport::ConfigureComm( int iBaud, Parity parity, BYTE DataBits, 
										StopBits stopbits, FlowControl fc )
{
	m_hComm = CreateFile( m_szAddress, GENERIC_READ | GENERIC_WRITE, 0, NULL, 
							OPEN_EXISTING, 0, 0 );

	if ( m_hComm == INVALID_HANDLE_VALUE )
		return false;

	//Get the current state prior to changing it
	DCB dcb;
	if ( !GetCommState(m_hComm, &dcb) )
		return false;

	//Setup the baud rate
	dcb.BaudRate = iBaud; 

	//Setup the Parity
	switch (parity)
	{
	case EvenParity:  dcb.Parity = EVENPARITY;  break;
	case MarkParity:  dcb.Parity = MARKPARITY;  break;
	case NoParity:    dcb.Parity = NOPARITY;    break;
	case OddParity:   dcb.Parity = ODDPARITY;   break;
	case SpaceParity: dcb.Parity = SPACEPARITY; break;
	default:          return false;
	}

	//Setup the data bits
	dcb.ByteSize = DataBits;

	//Setup the stop bits
	switch (stopbits)
	{
	case OneStopBit:           dcb.StopBits = ONESTOPBIT;   break;
	case OnePointFiveStopBits: dcb.StopBits = ONE5STOPBITS; break;
	case TwoStopBits:          dcb.StopBits = TWOSTOPBITS;  break;
	default:                   return false;
	}

	//Setup the flow control 
	dcb.fDsrSensitivity = false;
	switch (fc)
	{
	case NoFlowControl:
	{
	  dcb.fOutxCtsFlow = false;
	  dcb.fOutxDsrFlow = false;
	  dcb.fOutX = false;
	  dcb.fInX = false;
	  break;
	}
	case CtsRtsFlowControl:
	{
	  dcb.fOutxCtsFlow = true;
	  dcb.fOutxDsrFlow = false;
	  dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
	  dcb.fOutX = false;
	  dcb.fInX = false;
	  break;
	}
	case CtsDtrFlowControl:
	{
	  dcb.fOutxCtsFlow = true;
	  dcb.fOutxDsrFlow = false;
	  dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
	  dcb.fOutX = false;
	  dcb.fInX = false;
	  break;
	}
	case DsrRtsFlowControl:
	{
	  dcb.fOutxCtsFlow = false;
	  dcb.fOutxDsrFlow = true;
	  dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
	  dcb.fOutX = false;
	  dcb.fInX = false;
	  break;
	}
	case DsrDtrFlowControl:
	{
	  dcb.fOutxCtsFlow = false;
	  dcb.fOutxDsrFlow = true;
	  dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
	  dcb.fOutX = false;
	  dcb.fInX = false;
	  break;
	}
	case XonXoffFlowControl:
	{
	  dcb.fOutxCtsFlow = false;
	  dcb.fOutxDsrFlow = false;
	  dcb.fOutX = true;
	  dcb.fInX = true;
	  dcb.XonChar = 0x11;
	  dcb.XoffChar = 0x13;
	  dcb.XoffLim = 100;
	  dcb.XonLim = 100;
	  break;
	}
	default: return false;
	}

	//Now that we have all the settings in place, make the changes
  if (!SetCommState(m_hComm, &dcb))
	  return false;

  return true;
}

void CRS232Transport::Close()
{
	CloseHandle(m_hComm);
    m_hComm = INVALID_HANDLE_VALUE;
}

int CRS232Transport::Send( void* pData, uint32 uiSize )
{
	DWORD dwBytesWritten = 0;
	if (!WriteFile(m_hComm, pData, uiSize, &dwBytesWritten, NULL))
		return 0;

	return dwBytesWritten; 	
}

int CRS232Transport::Receive( void* pData, uint32 uiSize )
{
	DWORD dwBytesRead = 0;
	if ( !ReadFile(m_hComm, pData, uiSize, &dwBytesRead, NULL) )
		return 0;			

	return dwBytesRead; 	
}

void CRS232Transport::CancelBlockingIO() 
{
	CancelIo( m_hComm );
}

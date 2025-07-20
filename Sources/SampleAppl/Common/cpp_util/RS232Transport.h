// RS232Transport.h: interface for the CRS232Transport class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RS232TRANSPORT_H__DAA27214_F618_11D6_9F1C_000347F3A2BB__INCLUDED_)
#define AFX_RS232TRANSPORT_H__DAA27214_F618_11D6_9F1C_000347F3A2BB__INCLUDED_

#include "Transport.h"

class CRS232Transport : public CTransport  
{
  enum FlowControl
  {
    NoFlowControl,
    CtsRtsFlowControl,
    CtsDtrFlowControl,
    DsrRtsFlowControl,
    DsrDtrFlowControl,
    XonXoffFlowControl
  };

  enum Parity
  {    
    EvenParity,
    MarkParity,
    NoParity,
    OddParity,
    SpaceParity
  };

  enum StopBits
  {
    OneStopBit,
    OnePointFiveStopBits,
    TwoStopBits
  };

  HANDLE m_hComm;

public:
	CRS232Transport();
	virtual ~CRS232Transport();
	virtual bool Init( ETransportType Type, 
						char* szAddr=NULL, int iPort=0 );
	bool ConfigureComm( int iBaud=9600, Parity parity=NoParity, 
						uint8 DataBits=8, StopBits stopbits=OneStopBit,
						FlowControl fc=NoFlowControl );

	virtual void Close();

	virtual int Send( void* pData, uint32 uiSize );
	
	virtual int Receive( void* pData, uint32 uiSize );
	virtual void CancelBlockingIO();
};

#endif // !defined(AFX_RS232TRANSPORT_H__DAA27214_F618_11D6_9F1C_000347F3A2BB__INCLUDED_)

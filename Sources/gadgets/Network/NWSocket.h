// NWSocket.h: interface for the NWSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NWSOCKET_H__6E7C855C_FCEE_4C20_A065_56DCC9C14EBC__INCLUDED_)
#define AFX_NWSOCKET_H__6E7C855C_FCEE_4C20_A065_56DCC9C14EBC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Gadgets\gadbase.h>
#include "Server.h"
#include "Client.h"

#define MODE_SERVER 0
#define MODE_CLIENT 1

class NWSocket : public CCaptureGadget
{
  friend bool FAR __stdcall OnNWReceive( LPVOID usrData , pDataBurst data );
private:
  CInputConnector*  m_pInput;
  int m_Mode;
  FXString m_ServerAddress;
  int	   	m_ServicePort;
  BOOL      m_bSetRegistered ;
  BOOL    m_bNotSHClient ;
  CClient *m_Client;
  CServer *m_Server;
  DWORD	m_ID;
  FXString    m_LastShownMessage;
  bool        m_StopWhenGraphStoped;
protected:
  bool OnReceive( pDataBurst data );
  int		GetDataType()
  {
    if ( (m_Mode == MODE_CLIENT) && (m_Client) )
      return m_Client->GetDataType();
    if ( (m_Mode == MODE_SERVER) && (m_Server) )
      return m_Server->GetDataType();
    return -1;
  }
public:
  NWSocket();
  virtual void ShutDown();

  virtual CDataFrame* DoProcessing( const CDataFrame* pDataFrame );
  virtual int DoJob();
  virtual int GetInputsCount();
  CInputConnector* GetInputConnector( int n );
  bool    ScanSettings( FXString& text );
  bool    ScanProperties( LPCTSTR text , bool& Invalidate );
  bool    PrintProperties( FXString& text );
  void    PrintMessage( LPCTSTR mes );
  DECLARE_RUNTIME_GADGET( NWSocket );
};

#endif // !defined(AFX_NWSOCKET_H__6E7C855C_FCEE_4C20_A065_56DCC9C14EBC__INCLUDED_)

#pragma once
#include <fxfc\fxfc.h>
#include "helpers\UserBaseGadget.h"
#include "gadgets\videoframe.h"
#include "gadgets\QuantityFrame.h"
#include <gadgets\FigureFrame.h>
#include <helpers\FramesHelper.h>
#include <tchar.h>
#include "helpers/ShMemControl.h"
#include <thread>

enum SHMemMode
{
  SHMM_Server = 0 , // react on In Event and set Out Event
  SHMM_Client ,     // Fill request, set In Event and wait on Out Event
  SHMM_TectoServer ,  // Standard for Tecto
  SHMM_TectoSImulator // Option for interface simulation from graph
};

typedef struct tagDataBurst
{
  DWORD dwDataSize;
  BYTE  lpData[ 1 ];
} DataBurst , *pDataBurst;

// if m_GadgetMode is Client (SHMM_Client)
//   Send request to Shared Memory Out Zone and do set Out Event
//   Wait for In Event and take data from Shared Memory In Zone
// if m_GadgetMode is Server (SHMM_Client)
//   Wait for Out Event and take data from Shared Memory Out Zone
//   Send request to Shared Memory In Zone and do set In Event

class ShMemComm :
  public UserBaseGadget
{
public:
  ShMemComm();
  ~ShMemComm();

  //   void ShutDown();
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );
  void ProcessDataFromShMem() ;
  void ProcessErrorFromShMem( DWORD ErrCode ) ;
  void PropertiesRegistration();
  void ConnectorsRegistration();
  void Logger( LPCTSTR pLogString ) ;
  bool CheckAndDeleteReceiveThread() ;
  // Properties
  SHMemMode m_GadgetMode ;
  UINT      m_iShMemInSize_KB ;
  UINT      m_iShMemOutSize_KB ;
  FXString  m_ShMemAreaName ;
  FXString  m_InEventName ;
  FXString  m_OutEventName ;
  BOOL      m_bStopWhenGraphStoped = TRUE ;

  std::thread * m_pReceiveThread ;
  bool          m_bFinishReceiving ;
//   FXString m_StatusEventName ;

  CShMemControl * m_pShMem ;


  DECLARE_RUNTIME_GADGET( ShMemComm );
};


#pragma once
#include <fxfc\fxfc.h>
#include "helpers\UserBaseGadget.h"
#include "gadgets\videoframe.h"
#include "gadgets\QuantityFrame.h"
#include <gadgets\FigureFrame.h>
#include <helpers\FramesHelper.h>

enum SHMemMode
{
  SHMM_Server = 0 , // react on In Event and set Out Event
  SHMM_Client       // Fill request, set In Event and wait on Out Event
};

class ShMemComm :
  public UserBaseGadget
{
public:
  ShMemComm();
  ~ShMemComm();

  //   void ShutDown();
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );

  void PropertiesRegistration();
  void ConnectorsRegistration();
  void Logger( LPCTSTR pLogString ) ;

  // Properties
  SHMemMode m_GadgetMode ;
  FXString m_ShMemAreaName ;
  FXString m_InEventName ;
  FXString m_OutEventName ;

  DECLARE_RUNTIME_GADGET( ShMemComm );
};


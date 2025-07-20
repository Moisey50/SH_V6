#pragma once
#include "gadgets\gadbase.h"



class CInternalWatcher :
  public FXWorker
{
protected:
  FXStaticQueue<CDataFrame*> m_InputQueue;
  HANDLE    m_evHasData ;
  bool      m_bExit ;
public:
  CInternalWatcher(void);
  virtual ~CInternalWatcher(void);
  virtual int DoJob() ;
  virtual int ProcessData( CDataFrame * pDataFrame ) ;
  virtual int Put( CDataFrame * pFrame ) 
  {
    int ires = m_InputQueue.PutQueueObject( pFrame ) ;
    if ( ires )
      SetEvent( m_evHasData ) ;
    return ires ;
  };
  virtual void ShutDown() 
  {
    m_bExit = true ;
    SetEvent( m_evHasData ) ;
	  Destroy();
  };
};

class CInputListen : public CInternalWatcher
{
protected:
  FN_SENDINPUTDATA m_pFnSendInputData ;
  CGadget        * m_pHostGadget  ;
  double           m_dAccumulatedTime ;
public:
  CInputListen( CGadget * pHost , FN_SENDINPUTDATA pFunc ) ;
  virtual ~CInputListen() ;
  virtual int ProcessData( CDataFrame * pDataFrame ) ;
  double GetAccumulatedSendTime()
  {
    double dOut = m_dAccumulatedTime ;
    m_dAccumulatedTime = 0. ;
    return dOut ;
  }
};

class CInputPinWithQueue: 
  public CInputConnector , public FXWorker
{
protected:
  bool      m_bExit ;
  FN_SENDINPUTDATA m_pFnSendInputData ;
  CGadget        * m_pHostGadget  ;
  double           m_dAccumulatedTime ;
public:
  CInputPinWithQueue( CGadget * pHost , FN_SENDINPUTDATA pFunc , datatype type = transparent) ;
  virtual ~CInputPinWithQueue() ;
  virtual int DoJob() ;
  virtual int ProcessData( CDataFrame * pDataFrame ) ;
  double GetAccumulatedSendTime()
  {
    double dOut = m_dAccumulatedTime ;
    m_dAccumulatedTime = 0. ;
    return dOut ;
  }
};


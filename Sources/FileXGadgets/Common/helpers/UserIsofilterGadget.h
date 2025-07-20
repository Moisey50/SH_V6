#ifndef __INCLUDE__UserIsoFilterGadget_H__
#define __INCLUDE__UserIsoFilterGadget_H__

#include "gadgets\gadbase.h"
#include "helpers\UserBaseGadget.h"

class COutThread :
  public FXWorker
{
protected:
  FXStaticQueue<CDataFrame*> m_InputQueue;
  HANDLE    m_evHasData ;
  bool      m_bExit ;
public:
  COutThread(void)
  {
    m_bExit = false ;
    m_evHasData = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
    Create() ;
  }
  virtual ~COutThread(void)
  {
    CDataFrame * p ;
    while( m_InputQueue.GetQueueObject( p ) )
    {
      p->Release(p) ;
    }
    m_bExit = true ;
    SetEvent( m_evHasData ) ;
    Sleep(1) ;
    CloseHandle( m_evHasData ) ;
  }

  void SetNameAndStart( LPCTSTR Name = NULL )
  {
    if ( Name )
      SetThreadName( Name ) ;
    Resume() ;
  }
  virtual int DoJob() 
  {
    if ( m_bExit )
      return WR_CONTINUE ;

    CDataFrame * pDataFrame = NULL ;
    while ( WaitForSingleObject( m_evHasData , INFINITE) == WAIT_OBJECT_0 )
    {
      if (m_bExit)
        break ;
      if ( m_InputQueue.ItemsInQueue() )
      {
        CDataFrame * pDataFrame = NULL ;
        if ( m_InputQueue.GetQueueObject( pDataFrame ) && pDataFrame )
          ProcessData( pDataFrame ) ;
      }
    }
    m_bExit = false ;
    return WR_CONTINUE ;
  }

  virtual int ProcessData( CDataFrame * pDataFrame )
  {
    if ( pDataFrame )
    {
      pDataFrame->Release( pDataFrame ) ;
      return TRUE ;
    }
    return FALSE ;
  }

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
    SetEvent( m_evExit ) ;
    SetEvent( m_evHasData ) ;
    //WaitForSingleObject( m_hWorkerThread , INFINITE ) ;
    Destroy();
  };

};



class UserIsoFilterGadget : public UserBaseGadget , public COutThread
{
protected:
  int PutForOutput( CDataFrame * pFrame )
  {
    return Put( pFrame ) ;
  }
};

#endif	// #ifndef __INCLUDE__UserIsoFilterGadget_H__


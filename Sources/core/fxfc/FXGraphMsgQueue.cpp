#include "StdAfx.h"
#include <fxfc\fxfc.h>

static bool g_bQueuesAreOpen = false ;

/////////////////////////////////////////////////////////
//
// classes definition
//
//

class IGraphMsgQueueFactory
{
  PrintLogMsgFunc m_DefaultPrintLogFunc;
  FXArray<IGraphMsgQueue*>  m_MsgQueues;
  FXLockObject    m_Lock;
  bool            m_bClosing;
public:
  IGraphMsgQueueFactory();
  virtual ~IGraphMsgQueueFactory();
  IGraphMsgQueue* GetMsgQueue( PrintLogMsgFunc pLF );
  void ReleaseMsgQueue( PrintLogMsgFunc pLF );
  void SetDefaultPrintLogMsgFunc( PrintLogMsgFunc pLF );
  FXSIZE  Lookup( PrintLogMsgFunc pLF );
};

class FXGraphMsgQueue : public FXWorker , public IGraphMsgQueue
{
  class CMsg
  {
  public:
    int      msgLevel , msgId;
    FXString Source;
    FXString msgText;
  public:
    CMsg( int level , LPCTSTR src , int id , LPCTSTR txt ) : msgLevel( level ) , Source( src ) , msgId( id )
    {
      if ( txt ) msgText = txt;
    };
  };

  FXStaticQueue<CMsg*> m_Queue;
  FXLockObject m_Lock;
public:
  FXGraphMsgQueue( PrintLogMsgFunc pLF );
  virtual ~FXGraphMsgQueue();
  void Release();
  void AddMsg( int msgLevel , LPCTSTR src , int msgId , LPCTSTR msgText );
private:
  virtual int DoJob();
  void PublishQueue();
};

/////////////////////////////////////////////////////////
//
// global
//
//

static IGraphMsgQueueFactory* g_pGraphMsgQueueFactory = NULL;

void FxInitMsgQueues( PrintLogMsgFunc pLF )
{
  if ( !g_pGraphMsgQueueFactory )
  {
    g_pGraphMsgQueueFactory = new IGraphMsgQueueFactory;
    g_pGraphMsgQueueFactory->SetDefaultPrintLogMsgFunc( pLF );
  }
}

void FxEnableMsgQueuesLog( bool bEnable )
{
  g_bQueuesAreOpen = bEnable ;
}

void FxExitMsgQueues()
{
  if ( g_pGraphMsgQueueFactory )
    delete g_pGraphMsgQueueFactory;
  g_pGraphMsgQueueFactory = NULL;
}

void FxSetDefaultPrintLogMsgFunc( PrintLogMsgFunc pLF )
{
  if ( g_pGraphMsgQueueFactory )
    g_pGraphMsgQueueFactory->SetDefaultPrintLogMsgFunc( pLF );
}

IGraphMsgQueue* FxGetGraphMsgQueue( PrintLogMsgFunc pLF )
{
  if ( g_pGraphMsgQueueFactory )
    return g_pGraphMsgQueueFactory->GetMsgQueue( pLF );
  return NULL;
}

void FxReleaseGraphMsgQueue( PrintLogMsgFunc pLF )
{
  if ( g_pGraphMsgQueueFactory )
    g_pGraphMsgQueueFactory->ReleaseMsgQueue( pLF );
}

void FxSendLogMsg( int msgLevel , LPCTSTR src , int msgId , LPCTSTR lpszFormat , ... )
{
  va_list argList;
  va_start( argList , lpszFormat );
  FXString tmpS; tmpS.FormatV( lpszFormat , argList );
  IGraphMsgQueue* MsgQueue = FxGetGraphMsgQueue();
  if ( MsgQueue )
    MsgQueue->AddMsg( msgLevel , src , msgId , tmpS );
  va_end( argList );
}

/////////////////////////////////////////////////////////
//
// IGraphMsgQueue messages
//
//

IGraphMsgQueue::IGraphMsgQueue( PrintLogMsgFunc pLF ) :
  m_LogPrint( pLF ) ,
  m_cRefs( 0 )
{
  AddRef();
}

IGraphMsgQueue::~IGraphMsgQueue()
{}

void IGraphMsgQueue::AddRef()
{
  m_cRefs++;
}

/////////////////////////////////////////////////////////
//
// IGraphMsgQueueFactory
//

__forceinline IGraphMsgQueueFactory::IGraphMsgQueueFactory() :
  m_DefaultPrintLogFunc( NULL ) ,
  m_bClosing( false )
{ 
  g_bQueuesAreOpen = true ;
}

__forceinline IGraphMsgQueueFactory::~IGraphMsgQueueFactory()
{
  g_bQueuesAreOpen = !(m_bClosing = true) ;
  while ( m_MsgQueues.GetSize() )
  {
    IGraphMsgQueue* Queue = m_MsgQueues[ m_MsgQueues.GetSize() - 1 ];
    Queue->Release();
  }
}

__forceinline FXSIZE IGraphMsgQueueFactory::Lookup( PrintLogMsgFunc pLF )
{
  FXSIZE retV = -1;
  for ( int i = 0; i < (int)m_MsgQueues.GetSize(); i++ )
  {
    if ( m_MsgQueues[ i ]->GetPrintLogMsgFunc() == pLF )
    {
      retV = i;
      break;
    }
  }
  return retV;
}

__forceinline IGraphMsgQueue* IGraphMsgQueueFactory::GetMsgQueue( PrintLogMsgFunc pLF )
{
  if ( m_bClosing )
    return NULL;
  IGraphMsgQueue* Queue = NULL;
  m_Lock.Lock();
  PrintLogMsgFunc pLogFunc = (pLF) ? pLF : m_DefaultPrintLogFunc;
  FXSIZE i = Lookup( pLogFunc );
  if ( i < 0 )
  {
    Queue = new FXGraphMsgQueue( pLogFunc );
    i = m_MsgQueues.Add( Queue );
  }
  m_Lock.Unlock();
  return m_MsgQueues[ i ];
}

__forceinline void IGraphMsgQueueFactory::ReleaseMsgQueue( PrintLogMsgFunc pLF )
{
  m_Lock.Lock();
  PrintLogMsgFunc pLogFunc = (pLF) ? pLF : m_DefaultPrintLogFunc;
  FXSIZE i = Lookup( pLogFunc );
  if ( i >= 0 )
    m_MsgQueues.RemoveAt( i );
  m_Lock.Unlock();
}

__forceinline void IGraphMsgQueueFactory::SetDefaultPrintLogMsgFunc( PrintLogMsgFunc pLF )
{
  m_DefaultPrintLogFunc = pLF;
}

/////////////////////////////////////////////////////////
//
// FXGraphMsgQueue
//

FXGraphMsgQueue::FXGraphMsgQueue( PrintLogMsgFunc pLF ) :
  IGraphMsgQueue( pLF ) ,
  m_Queue( 1000 )
{
  SetThreadName( "FXGraphMsgQueue" );
  Create();
}

FXGraphMsgQueue::~FXGraphMsgQueue()
{
  Destroy();
  PublishQueue();
}

void FXGraphMsgQueue::Release()
{
  if ( !--m_cRefs )
  {
    FxReleaseGraphMsgQueue( m_LogPrint );
    delete this;
  }
}

void FXGraphMsgQueue::AddMsg( int msgLevel , LPCTSTR src , int msgId , LPCTSTR msgText )
{
  m_Lock.Lock();
  CMsg* pMsg = new CMsg( msgLevel , src , msgId , msgText );
  //m_Queue.Add(pMsg);
  if ( !m_Queue.PutQueueObject( pMsg ) )
  {
    delete pMsg;
  }
  m_Lock.Unlock();
  Resume();
}

//private:
int FXGraphMsgQueue::DoJob()
{
  PublishQueue();
  return WR_CONTINUE;
}
// 
void FXGraphMsgQueue::PublishQueue()
{
  m_Lock.Lock();
  while ( m_Queue.ItemsInQueue() )
  {
    CMsg* pMsg = NULL;
    if ( m_Queue.GetQueueObject( pMsg ) && pMsg )
    {
      if ( m_LogPrint && g_bQueuesAreOpen )
      {
        m_LogPrint( pMsg->msgLevel , pMsg->Source ,
          pMsg->msgId , LPCTSTR( pMsg->msgText ) );
      }
      delete pMsg;
    }
  }
  m_Lock.Unlock();
}


#ifndef IGRAPHMSGQUEUE_INCLUDE
#define IGRAPHMSGQUEUE_INCLUDE
// fxgraphmsgqueue.h : header file
//
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef void (FAR __stdcall  *PrintLogMsgFunc)(int msgLevel , LPCTSTR src , int msgId , LPCTSTR msgText);

class FXFC_EXPORT IGraphMsgQueue
{
protected:
  volatile int    m_cRefs;
  PrintLogMsgFunc	m_LogPrint;
protected:
  IGraphMsgQueue( PrintLogMsgFunc pLF );
  virtual ~IGraphMsgQueue();
public:
  void AddRef();
  virtual void Release() = 0;
  virtual void AddMsg( int msgLevel , LPCTSTR src , int msgId , LPCTSTR msgText ) = 0;
  PrintLogMsgFunc GetPrintLogMsgFunc()
  {
    return m_LogPrint;
  };
};

FXFC_EXPORT void            FxInitMsgQueues( PrintLogMsgFunc pLF = NULL );
FXFC_EXPORT void            FxEnableMsgQueuesLog( bool bEnable ) ;
FXFC_EXPORT void            FxExitMsgQueues();
FXFC_EXPORT void            FxSetDefaultPrintLogMsgFunc( PrintLogMsgFunc pLF );
FXFC_EXPORT IGraphMsgQueue* FxGetGraphMsgQueue( PrintLogMsgFunc pLF = NULL );
FXFC_EXPORT void            FxReleaseGraphMsgQueue( PrintLogMsgFunc pLF = NULL );

#endif //#ifndef IGRAPHMSGQUEUE_INCLUDE
// ProcessThread.cpp : implementation file
//

#include "stdafx.h"
#include "ProcessThread.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ProcessThread

UINT CaptureThread(LPVOID p)
{
  ProcessThread * pMainClass = (ProcessThread*) p ;
  while( !pMainClass->m_bStopFlag && pMainClass->m_pCaptureFunc)
  {
    pMainClass->m_pCaptureFunc(pMainClass->m_pCaptPar);
//			pMainClass->m_bStopFlag = TRUE;
//    pMainClass->m_pCaptureFunc = NULL;
  }
  return 0;
}
UINT ProcessingThread(LPVOID p)
{
  ProcessThread * pMainClass = (ProcessThread*) p ;
  while( !pMainClass->m_bStopFlag && pMainClass->m_pProcessFunc)
  {
    pMainClass->m_pProcessFunc(pMainClass->m_pProcPar) ;
  }
  return 0;
}
UINT CommandThread(LPVOID p)
{
  ProcessThread * pMainClass = (ProcessThread*) p ;
  while( !pMainClass->m_bStopFlag && pMainClass->m_pCommandFunc)
  {
    pMainClass->m_pCommandFunc(pMainClass->m_pCommPar) ;
  }
  return 0;
}
UINT ComFunc(LPVOID p)
{
  SynchData * SyncNum = (SynchData *) p;
  Sleep(SyncNum->iSleepTime) ;
  if(  SyncNum->iCounter
    && SyncNum->iCounter++ >= SyncNum->ilimit )
  {
    SyncNum->iCounter=0;
    SyncNum->SynchDataPtr->iCounter = 1;
  }
   return 0;
}
////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////
ProcessThread::ProcessThread(
     AFX_THREADPROC pCommandFunc, LPVOID pCommPar,
     AFX_THREADPROC pProcessFunc, LPVOID pProcPar ,
     AFX_THREADPROC pCaptureFunc, LPVOID pCaptPar)
{
  m_bStopFlag = 0 ;
  m_pCaptureFunc = pCaptureFunc ;
  m_pProcessFunc = pProcessFunc ;
  m_pCommandFunc = pCommandFunc ;
  m_pCaptPar     = pCaptPar ;
  m_pProcPar     = pProcPar ;
  m_pCommPar     = pCommPar ;
  AfxBeginThread( CaptureThread ,   this);//,  THREAD_PRIORITY_NORMAL );
  AfxBeginThread( ProcessingThread, this );//, THREAD_PRIORITY_LOWEST);
  AfxBeginThread( CommandThread,    this );//, THREAD_PRIORITY_HIGHEST);
}																				


ProcessThread::ProcessThread()
{
  CWinThread * pCaptureThread, * pProcessingThread,* pCommandThread;
  //Par1 Initialization:
  m_Par1.iSleepTime = 100;
  m_Par1.iCounter  =  1;
  m_Par1.ilimit    =  20;
  m_Par1.SynchDataPtr = &m_Par2;
  //Par2 Initialization;
  m_Par2.iSleepTime = 100;
  m_Par2.iCounter  =  0;
  m_Par2.ilimit    =  20;
  m_Par2.SynchDataPtr = &m_Par3;
  //Par3 Initialization:
  m_Par3.iSleepTime = 100;
  m_Par3.iCounter  =  0;
  m_Par3.ilimit    =  20;
  m_Par3.SynchDataPtr = &m_Par1;

  m_pCaptPar = (LPVOID)&m_Par1 ;
  m_pProcPar = (LPVOID)&m_Par2 ;
  m_pCommPar = (LPVOID)&m_Par3 ;
  m_pCaptureFunc = ComFunc;
  m_pProcessFunc = ComFunc;
  m_pCommandFunc = ComFunc;
  m_bStopFlag = 0 ;
  pCaptureThread = AfxBeginThread( CaptureThread , this);
  pProcessingThread = AfxBeginThread( ProcessingThread, this);
  pCommandThread = AfxBeginThread( CommandThread, this);
}
void ProcessThread::Close()
{
  m_bStopFlag = 1;
}
ProcessThread::~ProcessThread()
{
  
}


/////////////////////////////////////////////////////////////////////////////
// ProcessThread message handlers






















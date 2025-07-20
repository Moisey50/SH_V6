#if !defined(_SHMEMCONTROL_H_)
#define _SHMEMCONTROL_H_

#include "ShMemory.h"
//#include <fxfc\fxfc.h>

// This class is exported from the SharedMemory.dll
class CShMemControl: public CShMemoryBase 
{
public:
  CShMemControl( int iSendOffset = 128 , int iReceiveOffset = 1024 , 
    int iSize = 8192 ,
    LPCTSTR pAreaName = (LPCTSTR)"DEFAULT_SHARED_AREA" ,
    LPCTSTR pInEventName = NULL, LPCTSTR pOutEventName = NULL ,
    LPCTSTR pStatusEventName = NULL ) ;
  ~CShMemControl() ;
  virtual int Process( void * pMsg , int iMsgLen ) { return 0 ; } ;
  int SendAnswer( void * pMsg , int iMsgLen ); 
  int SendRequest( void * pMsg , int iMsgLen ) ;
  int GetRequestLength() ;
  int ReceiveRequest( void * pBuf , int& iBufLen );
  BOOL MarkRequestAsRead() ;
  int GetAnswerLength() ;
  int ReceiveAnswer( void * pBuf , int& iBufLen ) ;
  BOOL MarkAnswerAsRead() ;
  int WaitAndReceiveAnswer( void * pBuf , int& iBufLen ,
    int iTimeOut_ms ) ;
  int SendRqWaitAndReceiveAnswer( void * pBuf , int iMsgLen ,
    DWORD dwTimeOut_ms , int& iBufLen) ;
  int IsReceivedMsg() ;
  bool IsSentMessageProcessed() ;
  virtual const char * GetName() { return "CShMemControl" ; }
protected:
  int m_iSendOffset ;
  int m_iReceiveOffset ;
  CRITICAL_SECTION m_CriticalSection ;
//  FXLockObject m_Lock ;  
public:
};


#endif //_SHMEMCONTROL_H_

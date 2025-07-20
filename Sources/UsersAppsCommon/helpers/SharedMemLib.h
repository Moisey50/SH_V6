#if !defined(_SHAREDMEMLIB_H_)
#define _SHAREDMEMLIB_H_

#include "ShMemory.h"

typedef struct  
{
  char Control[12] ;
  char Message[52] ;
} SharedMessage ;

// This class is exported from the SharedMemory.dll
class CSharedMemLib: public CShMemoryBase 
{
public:
  CSharedMemLib( int iSendOffset = 64 , int iReceiveOffset = 128 , 
    int iSize = 4096 ,
    const char * pAreaName = "DEFAULT_SHARED_AREA" , 
    const char * pInEventName = NULL, const char * pOutEventName = NULL ) : 
	    CShMemoryBase( iSize ,pAreaName , pInEventName,pOutEventName) 
    
  {
     m_iSendOffset = iSendOffset ;
     m_iReceiveOffset = iReceiveOffset ;
  };
  int SendMessage( const char * pMsg , int iMsgLen ); 
  int ReceiveMessage( char * pBuf , int iBufLen );
  int IsReceivedMsg() ;
  bool IsSentMessageProcessed() ;
	// TODO: add your methods here.
protected:
  int m_iSendOffset ;
  int m_iReceiveOffset ;
public:
  int PutMessage(const char *  pMsg, int iMsgLen);
};


#endif //_SHAREDMEMLIB_H_

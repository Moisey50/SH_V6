#ifndef __LINKLISTEVT_H__
#define __LINKLISTEVT_H__

#include "synchronizations.h"

#define LIST_FLAG_AT_TAIL	1
#define LIST_FLAG_AT_HEAD	2
#define LIST_FLAG_SORTED	4
#define LIST_FLAG_BEFORE	8
#define LIST_FLAG_AFTER		16
#define LIST_FLAG_EXACT		32

template < class TYPE, class LOCK=CSectionLock >
class CSingleLinkListEvt
{
public:

	CSingleLinkListEvt()
	{
		m_pHead = NULL;
		m_pTail = NULL;
		m_bAutoDestroy = false;
		m_iNodeCount = 0;
    m_hEvent = CreateEvent( NULL , true , false , NULL ) ;
	}

	virtual ~CSingleLinkListEvt()
	{
		CloseHandle( m_hEvent ) ;
    RemoveAll();
	}

	void SetAutoDestroy( bool bAuto=false ) { m_bAutoDestroy = bAuto; }
  HANDLE GetEventHandle() { return m_hEvent ; }

	bool AddHead( TYPE& Item ) { return Add(Item) }
	bool AddTail( TYPE& Item ) { return Add( Item, LIST_FLAG_AT_TAIL ); }
	
	bool Add( TYPE& Item, long lo = LIST_FLAG_AT_HEAD )
	{
		if ( lo & LIST_FLAG_AT_TAIL && lo & LIST_FLAG_AT_HEAD )
			return false; //can't be both

		CGuard guard( &m_lock );
		if ( guard.Lock() )
		{
			TYPE* pLink = new TYPE( Item );
		
			if ( !pLink )
				return false;

			//The list is not empty
			if ( m_pHead )
			{
				if ( lo & LIST_FLAG_AT_TAIL )
				{						
					m_pTail->m_pNext = pLink;
					m_pTail = pLink;
					m_pTail->m_pNext = NULL;
				}
				else if ( lo & LIST_FLAG_AT_HEAD )
				{
					pLink->m_pNext = m_pHead;
					m_pHead = pLink;
				}
			}
			else //List is empty
			{
				m_pHead = pLink;
				m_pTail = m_pHead;
				m_pTail->m_pNext = NULL;
			}

			m_iNodeCount++;
      SetEvent( m_hEvent ) ;
			return true;
		}

		return false;
	}

	bool RemoveAtHead()
	{
		if ( IsEmpty() )
			return false;

		CGuard guard( &m_lock );
		if ( guard.Lock() )
		{
			if ( m_pHead )
			{
				TYPE * pTemp = m_pHead;
				m_pHead = m_pHead->m_pNext;
				delete pTemp ;
        if (--m_iNodeCount <= 0)
          ResetEvent( m_hEvent ) ;
				return true;
			}
		}
		return false;
	}

	void RemoveAll()
	{
		CGuard guard( &m_lock );
		if ( guard.Lock() )
		{
			TYPE * pTemp = m_pHead;
			while ( pTemp )
			{
				TYPE * pCurr = pTemp;
				pTemp = pTemp->m_pNext;
				delete pCurr ;
			}

			m_iNodeCount = 0;
			m_pHead = NULL;
			m_pTail = NULL;
      ResetEvent( m_hEvent ) ;
		}
	}

	TYPE * GetHead() { return m_pHead; }
	TYPE * GetTail() { return m_pTail; } 
	int GetNodeCount() { return m_iNodeCount; }
	bool IsEmpty() { return m_iNodeCount == 0; }

protected:
	LOCK	m_lock;
	TYPE *	m_pHead, * m_pTail;
	int		m_iNodeCount;
	bool	m_bAutoDestroy;
  HANDLE m_hEvent ;
};

typedef struct LINK_MSG
{
  int iMsgLen ;
  char Data[1] ;
} Dummy;

class LinkMsg
{
public:
  LinkMsg( int iMsgLen , const void * pMsg = NULL)
  {
    m_pMsg = (LINK_MSG *)malloc(iMsgLen+sizeof(iMsgLen)) ;
    if (m_pMsg)
    {
      m_pMsg->iMsgLen = iMsgLen ;
      if (pMsg)
        memcpy(m_pMsg , pMsg , iMsgLen) ;
    }
    m_pNext = NULL ;
  }
  LinkMsg( LinkMsg& Msg )
  {
    if ( Msg.m_pMsg )
    {
      m_pMsg = (LINK_MSG *)malloc( Msg.m_pMsg->iMsgLen 
        + sizeof(Msg.m_pMsg->iMsgLen)) ;
      if (m_pMsg)
      {
        m_pMsg->iMsgLen = Msg.m_pMsg->iMsgLen ;
        if ( Msg.GetBuffer() )
          memcpy(m_pMsg->Data ,  Msg.GetBuffer() , Msg.GetMsgLength() ) ;
      }
    }
    else
      m_pMsg = NULL ;
    m_pNext = NULL ;
  }
  ~LinkMsg()
  {
    free(m_pMsg) ;
  }

  LinkMsg & operator =(LinkMsg &Orig)
  {
    if (&Orig != this)
    {
      if (m_pMsg)
        free(m_pMsg) ;
      m_pMsg = (LINK_MSG *)malloc( Orig.m_pMsg->iMsgLen 
        + sizeof(Orig.m_pMsg->iMsgLen)) ;
      if (m_pMsg)
      {
        m_pMsg->iMsgLen = Orig.m_pMsg->iMsgLen ;
        if (Orig.m_pMsg->Data)
          memcpy(m_pMsg->Data , Orig.m_pMsg->Data , m_pMsg->iMsgLen) ;
      }
      m_pNext = NULL ;
    }

    return *this ;
  }
  bool operator==(const LinkMsg& Right)
  {
    if (m_pMsg)
    {
      if (Right.m_pMsg)
      {
        if (m_pMsg->iMsgLen == Right.m_pMsg->iMsgLen)
          return !memcmp(m_pMsg->Data , Right.m_pMsg->Data , Right.m_pMsg->iMsgLen ) ;
      }
      return false ;
    }
    else 
      return (!Right.m_pMsg) ;
  }

  int GetData( void * pMsg , int iLen ) 
  {
    if ( !m_pMsg )
      return NULL ;
    if (iLen < m_pMsg->iMsgLen)
      return -m_pMsg->iMsgLen ;
    memcpy( pMsg , m_pMsg->Data , m_pMsg->iMsgLen ) ;
    return m_pMsg->iMsgLen ;
  }
  void * GetBuffer() { return m_pMsg->Data ; }
  int GetMsgLength() { return m_pMsg->iMsgLen ; } ;

public:
  LinkMsg * m_pNext ;

protected:
  LINK_MSG * m_pMsg ;
};

typedef CSingleLinkListEvt<LinkMsg> MsgListEvt ;


#endif //__LINKLISTEVT_H__


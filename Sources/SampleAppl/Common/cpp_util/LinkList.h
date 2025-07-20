#ifndef __LINKLIST_H__
#define __LINKLIST_H__

#include "synchronizations.h"

#define LIST_FLAG_AT_TAIL	1
#define LIST_FLAG_AT_HEAD	2
#define LIST_FLAG_SORTED	4
#define LIST_FLAG_BEFORE	8
#define LIST_FLAG_AFTER		16
#define LIST_FLAG_EXACT		32

template < class TYPE, class LOCK >
class CSingleLinkList
{
public:
	class CLink
	{
	public:
		CLink()
		{
			m_pNext = NULL;
		}
	
		CLink( TYPE& Data)
		{
			m_Data = Data;
		}
		
		void Destroy()
		{
			//m_pData is presumed to be a pointer
			delete m_Data;
			delete this;
		}
		TYPE	m_Data;
		CLink*	m_pNext;
	};

	CSingleLinkList()
	{
		m_pHead = NULL;
		m_pTail = NULL;
		m_bAutoDestroy = false;
		m_iNodeCount = 0;
	}

	virtual ~CSingleLinkList()
	{
		RemoveAll();
	}

	void SetAutoDestroy( bool bAuto=false )
	{ m_bAutoDestroy = bAuto; }

	bool AddHead( TYPE& Item ) { return Add(Item) }
	bool AddTail( TYPE& Item ) { return Add( Item, LIST_FLAG_AT_TAIL ); }
	
	bool Add( TYPE& Item, long lo = LIST_FLAG_AT_HEAD )
	{
		if ( lo & LIST_FLAG_AT_TAIL && lo & LIST_FLAG_AT_HEAD )
			return false; //can't be both

		CGuard guard( &m_lock );
		if ( guard.Lock() )
		{
			CLink* pLink = new CLink( Item );
		
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
			return true;
		}

		return false;
	}
	
	virtual CLink* Find( TYPE& Item, long lo = LIST_FLAG_EXACT )
	{
		CGuard guard( &m_lock );
		if ( guard.Lock() )
		{
			CLink* pTemp, *pPrev;
			pTemp = m_pHead;
			pPrev = m_pHead;
			while ( pTemp )
			{
				if ( lo & LIST_FLAG_EXACT )
				{
					// == operator must be provided for the TYPE
					if ( pTemp->m_Data == Item )
						return pTemp;
				}
				else if ( lo & LIST_FLAG_BEFORE )
				{
					// < operator must be provided
					if ( pPrev->m_Data < Item && pTemp->m_Data >= Item )
						return pPrev;
				}
				else if ( lo & LIST_FLAG_AFTER )
				{
					// > operator must be provided
					if ( pTemp->m_Data > Item )
						return pTemp;
				}
				
				pPrev = pTemp;
				pTemp = pTemp->m_pNext;
			}			
		}		
		return NULL;
	}
	
	bool Remove( TYPE& Item )
	{
		CLink* pLink = Find( Item, LIST_FLAG_BEFORE );
		CGuard guard( &m_lock );
		if ( guard.Lock() )
		{
			if ( pLink )
			{
				CLink* pTemp = NULL;
				if ( pLink == m_pHead )
				{
					pTemp = m_pHead;
					m_pHead = m_pHead->m_pNext;
				}
				else if ( Item )
				{
					pTemp = pLink->m_pNext;
					//Jump over the item to delete
					pLink = pTemp->m_pNext;
				}

				if ( m_bAutoDestroy )
					pTemp->Destroy();
				else
					delete pTemp;
			}
			else
			{
				return false;
			}
			m_iNodeCount--;
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
				CLink* pTemp = m_pHead;
				m_pHead = m_pHead->m_pNext;
				pTemp->Destroy();
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
			CLink* pTemp = m_pHead;
			while ( pTemp )
			{
				CLink* pCurr = pTemp;
				pTemp = pTemp->m_pNext;
				if ( m_bAutoDestroy )
				{
					//The Data field must be a pointer of some sort
					delete pCurr->m_Data;
				}
				delete pCurr;
			}

			m_iNodeCount = 0;
			m_pHead = NULL;
			m_pTail = NULL;
		}
	}

	CLink* GetNext(CLink* pLink)
	{
		if ( !pLink )
			return NULL;
		return pLink->m_pNext;
	}

	CLink* GetHead() { return m_pHead; }
	CLink* GetTail() { return m_pTail; }
	int GetNodeCount() { return m_iNodeCount; }
	bool IsEmpty() { return m_iNodeCount == 0; }

protected:
	LOCK	m_lock;
	CLink*	m_pHead, *m_pTail;
	int		m_iNodeCount;
	bool	m_bAutoDestroy;
};

#endif //__LINKLIST_H__


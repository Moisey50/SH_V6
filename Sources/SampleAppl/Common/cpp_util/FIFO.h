#ifndef __FIFO_H__
#define __FIFO_H__
// FIFO.h: interface for the CFIFO class.
//
//////////////////////////////////////////////////////////////////////
#include "synchronizations.h"

template < class TYPE, class LOCK >
class CFIFO
{
	LOCK	m_lock;
	TYPE*	m_pBuf;
	int		m_nHead, m_nTail;
	int		m_nSize;
	int		m_nCount;

public:
	CFIFO()
	{
		m_nHead = 0;
		m_nTail = 0;
		m_nSize = 0;
		m_pBuf = NULL;
		m_nCount = 0;
	}

	~CFIFO()
	{
		CGuard guard( &m_lock );
		if ( guard.Lock() )
		{
			delete m_pBuf;
		}
	}

	bool Init( int nElements )
	{
		CGuard guard( &m_lock );

		if ( nElements <= 0 )
			return false;
		
		guard.Lock(); 
		if ( m_pBuf )
			delete m_pBuf;

		m_nSize = nElements;
		m_pBuf = new TYPE[ m_nSize ];
		m_nHead = 0;
		m_nTail = 0;
		m_nCount = 0;
		if ( m_pBuf )
			return true;
		
		return false;
	}

	bool Add( TYPE& T )
	{
		CGuard guard( &m_lock );
		guard.Lock() )
		
		if ( m_nCount >= m_nSize || m_pBuf == NULL )
			return false; //no buffer

		m_nCount++;
		m_pBuf[ m_nTail++ ] = T;

		//Tail shall be set to start of the buffer
		m_nTail %= m_nSize;

		return true;
	}

	//The iSize is a size of pT buffer in T elements. 
	//Returns number of elements added. 
	int AddBulk( TYPE* pT, int iSize )
	{
		CGuard guard( &m_lock );
		guard.Lock(); 
		
		//nothing to add
		if ( iSize == 0)
			return 0;

		if ( m_nCount >= m_nSize || m_pBuf == NULL )
			return 0; //no buffer
		
		int iEl2Add = 0;
		int iEl2Copy = 0;
		if ( m_nHead > m_nTail )
		{
			//if head is greater then tail then the free space 
			//if the distance between them
			iEl2Copy = min( m_nHead - m_nTail, iSize ); 
			memcpy( &m_pBuf[ m_nTail ], pT, sizeof(TYPE)*iEl2Copy );
			m_nCount += iEl2Copy;
			m_nTail += iEl2Copy;
			iEl2Add += iEl2Copy;

			if ( m_nTail == m_nSize )
				m_nTail = 0;
		}
		else if ( m_nTail >= m_nHead )
		{
			//Copy elements to the end of the buffer + elements to the 
			//start of the head
			iEl2Copy = min( m_nSize - m_nTail, riSize );
			memcpy( &m_pBuf[ m_nTail ], pT, sizeof(TYPE)*iEl2Copy );
			m_nTail += iEl2Copy;
			m_nCount += iEl2Copy;
			iEl2Add += iEl2Copy;

			if ( m_nTail >= m_nSize )
				m_nTail = 0;
			
			//If there are still data to copy then it is after the 
			//size of the buffer and before the head
			if ( iSize )
			{
				//start copying from the buffer start till head
				int iEl2Copy2 = min( m_nHead, riSize );
				memcpy( &m_pBuf[ m_nTail ], &pT[ iEl2Copy ], 
						sizeof(TYPE)*iEl2Copy2 );
				m_nTail += iEl2Copy2;
				iEl2Add += iEl2Copy2;
				m_nCount += iEl2Copy2;
			}
		}
		else
		{
			return 0; //should never get here, means we tried to add an 
			//element to a full buffer. This condition should have been 
			//checked already
		}

		if ( m_nTail >= m_nSize )
			m_nTail = 0;
	
		return iEl2Add;
	}

	//Initially iLength is a number of elements to get from buffer.
	//Returns number of elements obtained.
	int GetBulk( TYPE* pT, int iLength, bool bRemove=true )
	{
		CGuard guard( &m_lock );
		guard.Lock() 
		
		if ( iLength == 0 )
			return 0;

		//Nothing to get
		if ( m_nCount == 0 )
			return 0 ;

		int iSize2Get = 0;
		int iEl2Copy = 0;//Elements to copy
		if ( m_nHead >= m_nTail )
		{
			//Get data from head till the end of the buffer
			iEl2Copy = min( m_nSize - m_nHead, iLength );
			memcpy( pT, &m_pBuf[m_nHead], sizeof(TYPE)*iEl2Copy );

			int iNextIdx = (m_nHead + iEl2Copy);	
			if ( iNextIdx == m_nSize )
				iNextIdx = 0;

			iSize2Get += iEl2Copy;
			if ( bRemove )
			{
				m_nHead = iNextIdx;
				m_nCount -= iEl2Copy;
			}
			
			TYPE* pTNext = &pT[ iEl2Copy ];

			int iElLeft = iLength - iSize2Get;
			//Are there still elements to get ?
			if ( iElLeft > 0 )
			{
				iEl2Copy = min( m_nTail - iNextIdx, iElLeft );
				memcpy( pTNext, &m_pBuf[iNextIdx], sizeof(TYPE)*iEl2Copy );
				iSize2Get += iEl2Copy;

				if ( bRemove )
				{
					m_nHead += iEl2Copy;
					m_nCount -= iEl2Copy;
				}
			}			
		}
		else if ( m_nTail > m_nHead )
		{
			iEl2Copy = min( , iLength );
			memcpy( pT, &m_pBuf[m_nHead], sizeof(TYPE)*iEl2Copy );
			iSize2Get += iEl2Copy;
			if ( bRemove )
			{
				m_nHead += iEl2Copy;
				m_nCount -= iEl2Copy;
			}
		}

		return iSize2Get;
	}
	
	//returns number of elements removed
	int RemoveBulk( int iSize )
	{
		CGuard guard( &m_lock );
		guard.Lock();

		if ( m_nCount )
		{
			return 0;
		}
		int i2Remove = min( m_nCount, iSize );
		if ( m_nHead >= m_nTail )
		{
			m_nHead += i2Remove;
			m_nHead %= m_nSize;
			m_nCount -= i2Remove;
		}
		else
		{
			i2Remove = min( m_nTail - m_nHead, iSize );
			m_nHead += i2Remove;
			m_nCount -= i2Remove;
		}
		return i2Remove;
	}

	//Returns number of elements obtained
	int GetContinuousBuf(TYPE*& pBuf)
	{
		CGuard guard( &m_lock );
		guard.Lock();
		
		if ( m_nCount == 0)
		{
			return 0;
		}
		
		pBuf = &m_pBuf[m_nHead];

		if ( m_nTail > m_nHead )
			return m_nTail - m_nHead;
		else
			return m_nSize - m_nHead;
	}

	int Get( TYPE& T, bool bRemove=true )
	{
		CGuard guard( &m_lock );
		if ( guard.Lock() )
		{
			//Nothing to get
			if ( m_nCount == 0 || m_pBuf == NULL )
				return 0;

			//Explicitly copy the element memory
			//memcpy( &T, &m_pBuf[ m_nHead ], sizeof(TYPE) );
			T = m_pBuf[ m_nHead ];

			if ( bRemove == true )
			{
				m_nCount--;
				m_nHead++;

				//Head overlaps, shall be set to start of the buffer
				if ( m_nHead == m_nSize )
					m_nHead = 0;
			}
		}
		return 1;
	}

	TYPE* Pick() 
	{
		CGuard guard( &m_lock );
		if ( guard.Lock() )
		{
			//Nothing to pick at
			if ( m_nCount == 0 || m_pBuf == NULL )
				return NULL;

			return &m_pBuf[ m_nHead ];
		}	
		return NULL;
	}

	int GetCount()
	{
		CGuard guard( &m_lock );
		if ( guard.Lock() )
		{
			return m_nCount;
		}
		
		return -1;
	}

	void Clear()
	{
		CGuard guard( &m_lock );
		if ( guard.Lock() )
		{
			m_nHead = 0;
			m_nTail = 0;
			m_nCount = 0;
		}
	}

	int Remove()
	{
		CGuard guard( &m_lock );
		if ( guard.Lock() )
		{
			//Nothing to remove
			if ( m_nCount == 0 || m_pBuf == NULL )
				return 0;

			m_nCount--;
			m_nHead++;

			//Head overlaps, shall be set to start of the buffer
			if ( m_nHead == m_nSize )
				m_nHead = 0;
		}
		return 1;
	}	

	bool IsEmpty() 
	{ 
		CGuard guard( &m_lock );
		guard.Lock();
		return m_nCount == 0; 
	}
};


#endif //__FIFO_H__
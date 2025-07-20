/*************************************************************************************
 *************************************************************************************
 **																					**
 ** File name:		DynArray.h.														**
 **																					**
 ** Description:	Header file of template class CDynArray.						**
 **																					**
 ** Created on:		February 2002.													**
 **																					**
 ** Author:			Stas Desyatnikov.													**
 **																					**
 ** Copyright (C) 2002. All rights reserved.										**
 **																					**
 *************************************************************************************
 *************************************************************************************/

#ifndef __DYNARRAY_H__
#define __DYNARRAY_H__

#include "synchronizations.h"
//#include <stdlib.h>
//#include <string.h>
//#include <stdio.h>

template < class TYPE, class LOCK >
class CDynArray
{
public:
	enum EConsts {DEFAULT_LENGTH=100};

	///////////////////////////
	CDynArray()
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		m_nLength = 0;
		m_nTail = -1;
		m_pArray = NULL;
	}

	///////////////////////////
	~CDynArray()
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		if( NULL != m_pArray )
			delete[] m_pArray;
	}

	///////////////////////////
	TYPE& operator [] (int nAt)
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		return ((TYPE*)m_pArray)[ nAt ];
	}

	///////////////////////////
	bool Init( int nInitialLength = DEFAULT_LENGTH, int GrowBy = DEFAULT_LENGTH )
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		m_nGrowBy = GrowBy;
		m_nLength = nInitialLength;
		
		//if( m_pArray )
			delete[] m_pArray;

		m_pArray = new TYPE[m_nLength];
		if( NULL == m_pArray )
			return false;

		//memset( m_pArray, 0, m_nLength * sizeof(TYPE) );
		return true;
	}

	///////////////////////////
	bool CopyArray ( const CDynArray& arr )
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		m_nGrowBy = arr.m_nGrowBy;

		if( m_nLength < arr.m_nLength )
		{
			m_nLength = arr.m_nLength;
			
			//if( m_pArray )
				delete[] m_pArray;

			m_pArray = new TYPE [m_nLength];
			if( NULL == m_pArray )
				return false;
		}

		//Copy the content of the source array.
		//memcpy( m_pArray, arr.m_pArray, arr.m_nTail + 1 );
		for(int i = 0; i < m_nLength; i++)
		{
			m_pArray[i] = arr.m_pArray[i];
		}

		m_nTail = arr.m_nTail;

		return true;
	}

	///////////////////////////
	//Counts ALL (even not set) elements from position 0 to position m_nTail.
	//To get index of the element at m_nTail subtract 1 from the value returned by func GetCount.
	int GetCount() const
	{
		return m_nTail + 1;
	}

	///////////////////////////
	//Counts ALL (even not set) elements from position 0 to position m_nLength.
	int GetLength() const
	{
		return m_nLength;
	}

	bool Add( TYPE& Value );

	///////////////////////////
	bool Insert( int nAt, TYPE& Value )	//nAt starts from 0.
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		if( 0 == m_nLength || NULL == m_pArray )
		{
			bool Ret = Init(nAt + 1);	//nAt starts from 0.
			if( Ret == false )
				return false;

			//m_nTail is set below in func SetAt.
			SetAt( nAt, Value );
			return true;
		}

		if( nAt + 1 > m_nLength )	//nAt starts from 0.
		{
			int nFactor = (nAt - m_nLength) / m_nGrowBy + 1;
			TYPE* pTmp = new TYPE[m_nLength + nFactor * m_nGrowBy];
			if( NULL == pTmp )
				return false;

			//memset( pTmp, 0, nAt * sizeof(TYPE) );
			//Copy the existing data to a newly allocated buffer
			//memcpy( pTmp, m_pArray, m_nLength * sizeof(TYPE) );
			for(int i = 0; i < m_nLength; i++)
			{
				pTmp[i] = m_pArray[i];
			}
			delete[] m_pArray;
			m_pArray = pTmp;

			m_nLength = (m_nLength + nFactor * m_nGrowBy);
			//m_nTail is set below in func SetAt.
			SetAt( nAt, Value );
			return true;
		}

		if( nAt > m_nTail )	//nAt + 1 <= m_nLength (see above).
		{
			SetAt( nAt, Value );
			return true;
		}

		if( m_nTail < m_nLength )		//nAt <= m_nTail.
		{
			//There is enough room to move the tail.
			memmove( m_pArray + nAt + 1, m_pArray + nAt, (m_nTail - nAt + 1)*sizeof(TYPE) );
			m_nTail = m_nTail+1;
			SetAt( nAt, Value );
			return true;
		}
		else	//m_nTail == m_nLength.
		{
			TYPE* pTmp = new TYPE [m_nLength + m_nGrowBy];
			if( NULL == pTmp )
				return false;

			//memset( pTmp, 0, (m_nTail+1) * sizeof(TYPE) );
			//memcpy( pTmp, m_pArray, m_nLength * sizeof(TYPE) );
			for(int i = 0; i < m_nLength; i++)
			{
				pTmp[i] = m_pArray[i];
			}
			delete[] m_pArray;
			m_pArray = pTmp;

			m_nLength = m_nLength + m_nGrowBy;
			m_nTail = m_nTail+1;
			SetAt( nAt, Value );
			return true;
		}
	}

	///////////////////////////
	bool AddSorted( TYPE& Value )
	{
		bool Ret;

		for( int i=0; i<=m_nTail; i++ )
		{
			if( Value >= m_pArray[i] )	//">=": Works with an array sorted in increasing order.
				continue;						//Use "<=" to work with an array sorted in decreasing order.

/*			if( 0 == i )
			{
				Ret = Insert( 0, Value );
				if( Ret == false )
					return false;
				return true;
			}
*/
			Ret = Insert( i, Value );
			if( Ret == false )
				return false;
			return true;
		}

		Ret = Add( Value );
		if( Ret == false )
			return false;
		return true;
	}

	///////////////////////////
	static int Compare( const void *arg1, const void *arg2 )
	{
		if( *(TYPE*)arg1 == *(TYPE*)arg2 )
			return 0;
		return *(TYPE*)arg1 > *(TYPE*)arg2 ? 1 : -1;	//">": To sort the array in increasing order.
	}

	///////////////////////////
	void Sort()
	{
		//The array is sorted in increasing order, as defined by the comparison function.
		//To sort an array in decreasing order, reverse the sense of "greater than" and "less than"
		//in the comparison function.
		qsort(m_pArray, m_nLength, sizeof(TYPE), Compare);
	}

	///////////////////////////
	bool SetAt( int nAt, TYPE& Value )	//nAt starts from 0.
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		if( 0 == m_nLength || nAt + 1 > m_nLength )
			return false;

		if( m_nTail < nAt )
			m_nTail = nAt;

		*(m_pArray + nAt) = Value;
		return true;
	}

	///////////////////////////
	bool GetAt( int nAt, TYPE& Value )
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		if( 0 == m_nLength || -1 == m_nTail || nAt + 1 > m_nLength || nAt > m_nTail )
			return false;

		Value = *(m_pArray + nAt);

		return true;
	}

	///////////////////////////
	bool Remove( int nAt )	//nAt starts from 0.
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		if( 0 == m_nLength || -1 == m_nTail || nAt + 1 > m_nLength || nAt > m_nTail )
			return false;

		TYPE* pObj = m_pArray + nAt;
		pObj->~TYPE();
		if( nAt != m_nTail )
		{
			memmove( m_pArray + nAt, m_pArray + nAt + 1, (m_nTail - nAt)*sizeof(TYPE) );
		}

		m_nTail--;

		return true;
	}

	///////////////////////////
	bool RemoveAll()
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		//call destructor for each and every object in the array.
		for(int i = 0; i <= m_nTail; i++)
		{
			TYPE* pObj = m_pArray + i;
			pObj->~TYPE();
		}

		if( 0 == m_nLength || -1 == m_nTail )
			return false;

		m_nTail = -1;

		return true;
	}

private:
	int m_nLength;	//m_nLength * sizeof(TYPE) is total num of bytes in the array.
	int m_nTail;	//Valid m_nTail starts from 0.
	int m_nGrowBy;	//K * (m_nGrowBy * sizeof(TYPE)) is size of grow block (K is an int).
	LOCK m_lock;
	TYPE* m_pArray;
};

///////////////////////////
//Adds an element to the end of the array; grows the array if necessary.
template < class TYPE, class LOCK >
bool CDynArray<TYPE, LOCK>::Add( TYPE& Value )
{
	CGuard guard( &m_lock );
	guard.Lock();	//Blocks execution untill thread-safe lock is acquired.
	
	if( 0 == m_nLength || NULL == m_pArray )
		return false;
	
	if( m_nTail + 1 < m_nLength )
	{
		//There is enough room to add the element.
		SetAt( m_nTail+1, Value );
		return true;
	}
	else	//m_nTail + 1 == m_nLength.
	{
		TYPE* pTmp = new TYPE [m_nLength + m_nGrowBy];
		if( NULL == pTmp )
			return false;
		
		//memset( pTmp, 0, (m_nTail+1) * sizeof(TYPE) );
		//memcpy( pTmp, m_pArray, m_nLength * sizeof(TYPE) );
		for(int i = 0; i < m_nLength; i++)
		{
			pTmp[i] = m_pArray[i];
		}
		delete[] m_pArray;
		m_pArray = pTmp;
		
		m_nLength = m_nLength + m_nGrowBy;
		SetAt( m_nTail+1, Value );
		return true;
	}
}

#endif //#ifndef __DYNARRAY_H__

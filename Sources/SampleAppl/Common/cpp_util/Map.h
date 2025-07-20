/*************************************************************************************
 *************************************************************************************
 **																					**
 ** File name:		Map.h.															**
 **																					**
 ** Description:	Header file of template class CSyncMap.							**
 **																					**
 ** Created on:		February 2002.													**
 **																					**
 ** Author:			Miklovcik Ron.													**
 **																					**
 ** Copyright (C) 2002. All rights reserved.										**
 **																					**
 *************************************************************************************
 *************************************************************************************/

#ifndef __MAP_H__
#define __MAP_H__

#include "synchronizations.h"
//#include <new>

struct CPlex     // warning variable length structure
{
	CPlex* pNext;
	void* data() { return this+1; }

	static CPlex* /*PASCAL*/ Create(CPlex*& pHead, unsigned int nMax, unsigned int cbElement)
			// like 'calloc' but no zero fill
			// may throw memory exceptions
	{
		//ASSERT(nMax > 0 && cbElement > 0);
		CPlex* p = (CPlex*) new char[sizeof(CPlex) + nMax * cbElement];
				// may throw exception
		p->pNext = pHead;
		pHead = p;  // change head (adds in reverse order for simplicity)
		return p;
	}

	void FreeDataChain()       // free this one and links
	{
		CPlex* p = this;
		while (p != NULL)
		{
			char* bytes = (char*) p;
			CPlex* pNext = p->pNext;
			delete[] bytes;
			p = pNext;
		}
	}
};


/////////////////////////////////////////////////////////////////////////////
template<class KEY, class VALUE, class LOCK>
class CSyncMap		//Sync means thread-safe.
{
	LOCK m_lock;
protected:
	// Association
	struct CAssoc
	{
		CAssoc* pNext;
		unsigned int nHashValue;  // needed for efficient iteration
		KEY key;
		VALUE value;
	};

	CAssoc** m_pHashTable;

public:
	// Construction
	CSyncMap(int nBlockSize = 10)
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		//ASSERT(nBlockSize > 0);

		m_pHashTable = NULL;
		m_nHashTableSize = 17;  // default size
		m_nCount = 0;
		m_pFreeList = NULL;
		m_pBlocks = NULL;
		m_nBlockSize = nBlockSize;
	}

	// Attributes
	// number of elements
	int GetCount()
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		return m_nCount;
	}

	bool IsEmpty()
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		return m_nCount == 0;
	}

	// Lookup
	bool Lookup(KEY& key, VALUE& rValue)
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		//ASSERT_VALID(this);

		unsigned int nHash;
		CAssoc* pAssoc = GetAssocAt(key, nHash);
		if (pAssoc == NULL)
			return false;  // not in map

		rValue = pAssoc->value;
		return true;
	}

	// Used to force allocation of a hash table or to override the default
	//   hash table size of (which is fairly small)
	void InitHashTable(unsigned int nHashSize, bool bAllocNow = true)
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		//ASSERT_VALID(this);
		//ASSERT(m_nCount == 0);
		//ASSERT(nHashSize > 0);

		if (m_pHashTable != NULL)
		{
			// free hash table
			delete[] m_pHashTable;
			m_pHashTable = NULL;
		}

		if (bAllocNow)
		{
			m_pHashTable = new CAssoc* [nHashSize];
			memset(m_pHashTable, 0, sizeof(CAssoc*) * nHashSize);
		}
		m_nHashTableSize = nHashSize;
	}

	// Operations
	// Lookup and add if not there
	VALUE& operator[](KEY& key)
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		//ASSERT_VALID(this);

		unsigned int nHash;
		CAssoc* pAssoc;
		if ((pAssoc = GetAssocAt(key, nHash)) == NULL)
		{
			if (m_pHashTable == NULL)
				InitHashTable(m_nHashTableSize);

			// it doesn't exist, add a new Association
			pAssoc = NewAssoc();
			pAssoc->nHashValue = nHash;
			pAssoc->key = key;
			// 'pAssoc->value' is a constructed object, nothing more

			// put into hash table
			pAssoc->pNext = m_pHashTable[nHash];
			m_pHashTable[nHash] = pAssoc;
		}
		return pAssoc->value;  // return new reference
	}

	// add a new (key, value) pair
	void SetAt(KEY& key, VALUE& newValue)
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		unsigned int nHash;
		CAssoc* pAssoc = NULL;
		if ((pAssoc = GetAssocAt(key, nHash)) == NULL)
		{
			if (m_pHashTable == NULL)
				InitHashTable(m_nHashTableSize);

			// it doesn't exist, add a new Association
			pAssoc = NewAssoc();
			pAssoc->nHashValue = nHash;
			pAssoc->key = key;
			// 'pAssoc->value' is a constructed object, nothing more

			// put into hash table
			pAssoc->pNext = m_pHashTable[nHash];
			m_pHashTable[nHash] = pAssoc;
		}
		
		if ( pAssoc )
			pAssoc->value = newValue;
	}

	// removing existing (key, ?) pair
	bool RemoveKey(KEY& key)
	// remove key - return TRUE if removed
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		//ASSERT_VALID(this);

		if (m_pHashTable == NULL)
		{
			return false;  // nothing in the table
		}

		CAssoc** ppAssocPrev;
		ppAssocPrev = &m_pHashTable[HashKey(key) % m_nHashTableSize];

		CAssoc* pAssoc;
		for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext)
		{
			if (CompareElements(&pAssoc->key, &key))
			{
				// remove it
				*ppAssocPrev = pAssoc->pNext;  // remove from list
				FreeAssoc(pAssoc);
				return true;
			}
			ppAssocPrev = &pAssoc->pNext;
		}

		return false;  // not found
	}

	void RemoveAll()
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		//ASSERT_VALID(this);

		if (m_pHashTable != NULL)
		{
			// destroy elements (values and keys)
			for (unsigned int nHash = 0; nHash < m_nHashTableSize; nHash++)
			{
				CAssoc* pAssoc;
				for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL;
				  pAssoc = pAssoc->pNext)
				{
					DestructElements<VALUE>(&pAssoc->value, 1);
					DestructElements<KEY>(&pAssoc->key, 1);
				}
			}
		}

		// free hash table
		delete[] m_pHashTable;
		m_pHashTable = NULL;

		m_nCount = 0;
		m_pFreeList = NULL;
		m_pBlocks->FreeDataChain();
		m_pBlocks = NULL;
	}

	// iterating all (key, value) pairs
	//POSITION GetStartPosition();
	//void GetNextAssoc(POSITION& rNextPosition, KEY& rKey, VALUE& rValue);

	// advanced features for derived classes
	unsigned int GetHashTableSize()
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.
		unsigned int uiTemp = m_nHashTableSize

		return uiTemp;
	}

// Implementation
protected:
	unsigned int m_nHashTableSize;
	int m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CAssoc* NewAssoc()
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		if (m_pFreeList == NULL)
		{
			// add another block
			CPlex* newBlock = CPlex::Create(m_pBlocks, m_nBlockSize, sizeof(CSyncMap::CAssoc));
			// chain them into free list
			CSyncMap::CAssoc* pAssoc = (CSyncMap::CAssoc*) newBlock->data();
			// free in reverse order to make it easier to debug
			pAssoc += m_nBlockSize - 1;
			for (int i = m_nBlockSize-1; i >= 0; i--, pAssoc--)
			{
				pAssoc->pNext = m_pFreeList;
				m_pFreeList = pAssoc;
			}
		}
		//ASSERT(m_pFreeList != NULL);  // we must have something

		CSyncMap::CAssoc* pAssoc = m_pFreeList;
		m_pFreeList = m_pFreeList->pNext;
		m_nCount++;
		//ASSERT(m_nCount > 0);  // make sure we don't overflow
		//ConstructElements<KEY>(&pAssoc->key, 1);
		//memset((void*)&pAssoc->key, 0, sizeof(KEY));
		//KEY TmpK;
		//memcpy( &pAssoc->key, &TmpK, sizeof(KEY) );
		//ConstructElements<VALUE>(&pAssoc->value, 1);   // special construct values
		//memset((void*)&pAssoc->value, 0, sizeof(VALUE));
		//VALUE TmpV;
		//memcpy( &pAssoc->value, &TmpV, sizeof(VALUE) );
		

		return pAssoc;
	}

	void FreeAssoc(CAssoc* pAssoc)
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		DestructElements<VALUE>(&pAssoc->value, 1);
		DestructElements<KEY>(&pAssoc->key, 1);
		pAssoc->pNext = m_pFreeList;
		m_pFreeList = pAssoc;
		m_nCount--;
		//ASSERT(m_nCount >= 0);  // make sure we don't underflow

		// if no more elements, cleanup completely
		if (m_nCount == 0)
			RemoveAll();

	}

	unsigned int HashKey(KEY& k)
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.
		
		// default identity hash - works for most primitive values
		return ((unsigned int)(void*)(unsigned long)k) >> 4;
	}

	CAssoc* GetAssocAt(KEY& key, unsigned int& nHash)
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.
		
		nHash = HashKey(key) % m_nHashTableSize;

		if (m_pHashTable == NULL)
		{
			return NULL;
		}
		// see if it exists
		CAssoc* pAssoc;
		for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext)
		{
			if (CompareElements(&pAssoc->key, &key))
			{
				return pAssoc;
			}
		}

		return NULL;
	}

public:
	~CSyncMap()
	{
		CGuard guard( &m_lock );
		guard.Lock();	//Blocks execution untill thread-safe lock is acquired.

		RemoveAll();
		//ASSERT(m_nCount == 0);
	}

	//void Serialize(CArchive&);
#ifdef _DEBUG
	//void Dump(CDumpContext&);
	void AssertValid();
#endif
};

/////////////////////////////////////////////////////////////////////////////
// CSyncMap<KEY, KEY&, VALUE, VALUE&> inline functions

//template<class KEY, class KEY&, class VALUE, class VALUE&>
//POSITION CSyncMap<KEY, KEY&, VALUE, VALUE&>::GetStartPosition()
//	{ return (m_nCount == 0) ? NULL : BEFORE_START_POSITION; }

/////////////////////////////////////////////////////////////////////////////
// CSyncMap<KEY, KEY&, VALUE, VALUE&> out-of-line functions

#ifdef _DEBUG
template<class KEY, class VALUE>
void CSyncMap<KEY, VALUE, class LOCK>::AssertValid()
{
	//CObject::AssertValid();

	//ASSERT(m_nHashTableSize > 0);
	//ASSERT(m_nCount == 0 || m_pHashTable != NULL);
		// non-empty map should have hash table
}
#endif //_DEBUG

/*
template<class TYPE>
void ConstructElements(TYPE* pElements, int nCount)
{
	//ASSERT(nCount == 0 ||
	//	AfxIsValidAddress(pElements, nCount * sizeof(TYPE)));

	// first do bit-wise zero initialization
	memset((void*)pElements, 0, nCount * sizeof(TYPE));

	// then call the constructor(s)
	for (; nCount--; pElements++)
		::new (pElements) TYPE;
}
*/

template<class TYPE>
void DestructElements(TYPE* pElements, int nCount)
{
	//ASSERT(nCount == 0 ||
	//	AfxIsValidAddress(pElements, nCount * sizeof(TYPE)));

	// call the destructor(s)
	//for (; nCount--; pElements++)
	//	pElements->~TYPE();
}

template<class TYPE>
void CopyElements(TYPE* pDest, const TYPE* pSrc, int nCount)
{
	//ASSERT(nCount == 0 ||
	//	AfxIsValidAddress(pDest, nCount * sizeof(TYPE)));
	//ASSERT(nCount == 0 ||
	//	AfxIsValidAddress(pSrc, nCount * sizeof(TYPE)));

	// default is element-copy using assignment
	while (nCount--)
		*pDest++ = *pSrc++;
}

template<class TYPE>
bool CompareElements(const TYPE* pElement1, const TYPE* pElement2)
{
	//ASSERT(AfxIsValidAddress(pElement1, sizeof(TYPE), FALSE));
	//ASSERT(AfxIsValidAddress(pElement2, sizeof(ARG_TYPE), FALSE));

	return *pElement1 == *pElement2;
}


#endif //#ifndef __MAP_H__

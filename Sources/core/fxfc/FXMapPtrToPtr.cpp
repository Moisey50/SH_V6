#include "StdAfx.h"
#include <fxfc\fxfc.h>

FXMapPtrToPtr::FXMapPtrToPtr(INT_PTR nBlockSize)
{
	ASSERT(nBlockSize > 0);
	if (nBlockSize <= 0)
		nBlockSize = 10;	// default size

	m_pHashTable = NULL;
	m_nHashTableSize = 17;  // default size
	m_nCount = 0;
	m_pFreeList = NULL;
	m_pBlocks = NULL;
	m_nBlockSize = nBlockSize;
}

FXMapPtrToPtr::~FXMapPtrToPtr(void)
{
	RemoveAll();
	ASSERT(m_nCount == 0);
}

void FXMapPtrToPtr::RemoveAll()
{
	if (m_pHashTable != NULL)
	{
		delete[] m_pHashTable;
		m_pHashTable = NULL;
	}
	m_nCount = 0;
	m_pFreeList = NULL;
	m_pBlocks->FreeDataChain();
	m_pBlocks = NULL;
}

INT_PTR FXMapPtrToPtr::GetCount() const
{ 
    return m_nCount; 
}

INT_PTR FXMapPtrToPtr::GetSize() const	
{ 
    return m_nCount; 
}

BOOL FXMapPtrToPtr::IsEmpty() const	
{ 
    return m_nCount == 0; 
}

BOOL FXMapPtrToPtr::Lookup(void* key, void*& rValue) const
{
	UINT nHashBucket, nHashValue;
	_element* pAssoc = GetAssocAt(key, nHashBucket, nHashValue);
	if (pAssoc == NULL)
		return FALSE;
	rValue = pAssoc->value;
	return TRUE;
}

void*& FXMapPtrToPtr::operator[](void* key)
{
	UINT nHashBucket, nHashValue;
	_element* pAssoc;
	if ((pAssoc = GetAssocAt(key, nHashBucket, nHashValue)) == NULL)
	{
		if (m_pHashTable == NULL)
			InitHashTable(m_nHashTableSize);
		pAssoc = NewAssoc();
		pAssoc->key = key;
		pAssoc->pNext = m_pHashTable[nHashBucket];
		m_pHashTable[nHashBucket] = pAssoc;
	}
	return pAssoc->value;
}

void* FXMapPtrToPtr::GetValueAt(void* key) const
{
	if (m_pHashTable == NULL)
		return NULL;
	UINT nHash = HashKey(key) % m_nHashTableSize;
	_element* pAssoc;
	for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
		if (pAssoc->key == key)
			return pAssoc->value;
	}
	return NULL;
}

void FXMapPtrToPtr::SetAt(void* key, void* newValue)
{ 
    (*this)[key] = newValue; 
}

BOOL FXMapPtrToPtr::RemoveKey(void* key)
{
	if (m_pHashTable == NULL)
		return FALSE;  
	_element** ppAssocPrev;
	ppAssocPrev = &m_pHashTable[HashKey(key) % m_nHashTableSize];
	_element* pAssoc;
	for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
		if (pAssoc->key == key)
		{
			*ppAssocPrev = pAssoc->pNext;
			FreeAssoc(pAssoc);
			return TRUE;
		}
		ppAssocPrev = &pAssoc->pNext;
	}
	return FALSE;
}

POSITION FXMapPtrToPtr::GetStartPosition() const
{ 
    return (m_nCount == 0) ? NULL : BEFORE_START_POSITION; 
}

void FXMapPtrToPtr::GetNextAssoc(POSITION& rNextPosition, void*& rKey, void*& rValue) const
{
	_element* pAssocRet = (_element*)rNextPosition;
	if (pAssocRet == NULL)
		return;
	if (pAssocRet == (_element*) BEFORE_START_POSITION)
	{
		for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
        {
			if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
            {
				break;
            }
        }
	}
    _element* pAssocNext;
	if ((pAssocNext = pAssocRet->pNext) == NULL)
	{
		for (UINT nBucket = (HashKey(pAssocRet->key) % m_nHashTableSize) + 1;nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
				break;
	}
	rNextPosition = (POSITION) pAssocNext;
	rKey = pAssocRet->key;
	rValue = pAssocRet->value;
}

UINT FXMapPtrToPtr::GetHashTableSize() const
{ 
    return m_nHashTableSize; 
}

void FXMapPtrToPtr::InitHashTable(UINT nHashSize, BOOL bAllocNow)
{
	if (nHashSize == 0)
		nHashSize = 17;	// default value
	if (m_pHashTable != NULL)
	{
		delete[] m_pHashTable;
		m_pHashTable = NULL;
	}
	if (bAllocNow)
	{
		m_pHashTable = new _element* [nHashSize];
		memset(m_pHashTable, 0, sizeof(_element*) * nHashSize);
	}
	m_nHashTableSize = nHashSize;
}

UINT FXMapPtrToPtr::HashKey(void* key) const
{
	return UINT(DWORD_PTR(key)>>4);
}

/// private:

FXMapPtrToPtr::_element* FXMapPtrToPtr::NewAssoc()
{
	if (m_pFreeList == NULL)
	{
		FXPlex* newBlock = FXPlex::Create(m_pBlocks, m_nBlockSize, sizeof(FXMapPtrToPtr::_element));
		FXMapPtrToPtr::_element* pAssoc = (FXMapPtrToPtr::_element*) newBlock->data();
		pAssoc += m_nBlockSize - 1;
		for (INT_PTR i = m_nBlockSize-1; i >= 0; i--, pAssoc--)
		{
			pAssoc->pNext = m_pFreeList;
			m_pFreeList = pAssoc;
		}
	}
	FXMapPtrToPtr::_element* pAssoc = m_pFreeList;
	m_pFreeList = m_pFreeList->pNext;
	m_nCount++;
	ASSERT(m_nCount > 0);
	pAssoc->key = 0;
	pAssoc->value = 0;
	return pAssoc;
}

void FXMapPtrToPtr::FreeAssoc(FXMapPtrToPtr::_element* pAssoc)
{
	pAssoc->pNext = m_pFreeList;
	m_pFreeList = pAssoc;
	m_nCount--;
	if (m_nCount == 0)
		RemoveAll();
}

FXMapPtrToPtr::_element* FXMapPtrToPtr::GetAssocAt(void* key, UINT& nHashBucket, UINT& nHashValue) const
{
	nHashValue = HashKey(key);
	nHashBucket = nHashValue % m_nHashTableSize;

	if (m_pHashTable == NULL)
		return NULL;
	_element* pAssoc;
	for (pAssoc = m_pHashTable[nHashBucket]; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
		if (pAssoc->key == key)
			return pAssoc;
	}
	return NULL;
}

#ifndef FXMAPPTRTOPTR_INCLUDE
#define FXMAPPTRTOPTR_INCLUDE
// FXMapPtrToPtr.h : header file
//
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFX_H__
    struct __POSITION {};
    typedef __POSITION* POSITION;
    #define BEFORE_START_POSITION ((POSITION)-1L)
#endif

class FXFC_EXPORT FXMapPtrToPtr
{
protected:
	struct _element
	{
		_element* pNext;
		void* key;
		void* value;
	};
protected:
	_element**  m_pHashTable;
	unsigned    m_nHashTableSize;
	int         m_nCount;
	_element* m_pFreeList;
	struct FXPlex* m_pBlocks;
	FXSIZE         m_nBlockSize;
public:
    FXMapPtrToPtr(FXSIZE nBlockSize = 10); //+
    ~FXMapPtrToPtr(void);   //+
	FXSIZE GetCount() const; //+
	FXSIZE GetSize() const; //+
	BOOL IsEmpty() const; //+

	// Lookup
	BOOL Lookup(void* key, void*& rValue) const; //+
	void*& operator[](void* key); //+
    void* GetValueAt(void* key) const; //+

	void SetAt(void* key, void* newValue); //+

    BOOL RemoveKey(void* key); //+
	void RemoveAll(); //+

	POSITION GetStartPosition() const; //+
	void GetNextAssoc(POSITION& rNextPosition, void*& rKey, void*& rValue) const; //+

	UINT GetHashTableSize() const; //+
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE); //+
	UINT HashKey(void* key) const; //+
private:
	_element*   NewAssoc(); //+
	void FreeAssoc(_element*); //+
	_element* GetAssocAt(void*, UINT&, UINT&) const;
};

#endif //#ifndef FXMAPPTRTOPTR_INCLUDE
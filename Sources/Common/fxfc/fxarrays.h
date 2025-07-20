#ifndef FXARRAY_INCLUDE
#define FXARRAY_INCLUDE
// fxarrays.h : header file
//
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

template<class TYPE>
inline void _copyelements( TYPE* pDest , const TYPE* pSrc , INT_PTR nCount )
{
  ASSERT( nCount == 0 || pDest != 0 && pSrc != 0 );
  // default is element-copy using assignment
  while (nCount--)
    *pDest++ = *pSrc++;
}
#pragma push_macro("new")

#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
inline void *__CRTDECL operator new( size_t , void *_Where )
{
  return ( _Where );
}
#if     _MSC_VER >= 1200
inline void __CRTDECL operator delete( void * , void * )
{
  return;
}
#endif
#endif



template<class TYPE , class ARG_TYPE = const TYPE&>
class FXArray
{
  // Implementation
protected:
  TYPE* m_pData;   // the actual array of data
  FXSIZE m_nSize;     // # of elements (upperBound + 1)
  FXSIZE m_nMaxSize;  // max allocated
  FXSIZE m_nGrowBy;   // grow amount
public:
  /// Construction
  FXArray() { m_pData = NULL;	m_nSize = m_nMaxSize = 0 ; m_nGrowBy = 1; }
  ~FXArray()
  {
    if (m_pData != NULL)
    {
      for (FXSIZE i = 0; i < m_nSize; i++)
        ( m_pData + i )->~TYPE();
      delete[]( BYTE* )m_pData;
    }
  }
  /// Members
  FXSIZE  GetSize() const { return m_nSize; }
  FXSIZE  GetCount() const { return m_nSize; }
  FXSIZE  Size() const { return m_nSize; }
  FXSIZE  size() const { return m_nSize; }
  FXSIZE  Count() const { return m_nSize; }
  BOOL    IsEmpty() const { return m_nSize == 0; }
  FXSIZE  GetUpperBound() const { return m_nSize - 1; }
  FXSIZE  ResetLength() // No reallocation
  {
    FXSIZE iOldIndex = m_nSize ;
    m_nSize = 0 ;
    return iOldIndex ;
  }
  void    SetSize( FXSIZE nNewSize , FXSIZE nGrowBy = -1 )
  {
    ASSERT( nNewSize >= 0 );

    if (nNewSize < 0)
      throw 1;

    if (nGrowBy >= 0)
      m_nGrowBy = nGrowBy;
    if (nNewSize == 0)
    {
      if (m_pData != NULL)
      {
        for (FXSIZE i = 0; i < m_nSize; i++)
          ( m_pData + i )->~TYPE();
        delete[]( BYTE* )m_pData;
        m_pData = NULL;
      }
      m_nSize = m_nMaxSize = 0;
    }
    else if (m_pData == NULL)
    {
      FXSIZE nAllocSize = __max( nNewSize , m_nGrowBy );
      m_pData = ( TYPE* ) new BYTE[ nAllocSize * sizeof( TYPE ) ];
      memset( ( void* ) m_pData , 0 , nAllocSize * sizeof( TYPE ) );
      for (FXSIZE i = 0; i < nNewSize; i++)
#pragma push_macro("new")
#undef new
        ::new( ( void* ) ( m_pData + i ) ) TYPE;
#pragma pop_macro("new")
      m_nSize = nNewSize;
      m_nMaxSize = nAllocSize;
    }
    else if (nNewSize <= m_nMaxSize)
    {
      if (nNewSize > m_nSize)
      {
        memset( ( void* ) ( m_pData + m_nSize ) , 0 , ( FXSIZE ) ( nNewSize - m_nSize ) * sizeof( TYPE ) );
        for (FXSIZE i = 0; i < nNewSize - m_nSize; i++)
#pragma push_macro("new")
#undef new
          ::new( ( void* ) ( m_pData + m_nSize + i ) ) TYPE;
#pragma pop_macro("new")
      }
      else if (m_nSize > nNewSize)
      {
        for (FXSIZE i = 0; i < m_nSize - nNewSize; i++)
          ( m_pData + nNewSize + i )->~TYPE();
      }
      m_nSize = nNewSize;
    }
    else // bufer already exists
    {
      nGrowBy = m_nGrowBy;
      if (nGrowBy == 0)
      {
        nGrowBy = m_nSize / 8;
        nGrowBy = ( nGrowBy < 4 ) ? 4 : ( ( nGrowBy > 1024 ) ? 1024 : nGrowBy );
      }
      FXSIZE nNewMax;
      if (nNewSize < m_nMaxSize + nGrowBy)
        nNewMax = m_nMaxSize + nGrowBy;
      else
        nNewMax = nNewSize;
      ASSERT( nNewMax >= m_nMaxSize );
      if (nNewMax < m_nMaxSize)
        throw 1;

      TYPE* pNewData = ( TYPE* ) new BYTE[ nNewMax * sizeof( TYPE ) ];

      memcpy_s( pNewData , ( size_t ) nNewMax * sizeof( TYPE ) ,
        m_pData , ( size_t ) m_nSize * sizeof( TYPE ) );

      ASSERT( nNewSize > m_nSize );
      memset( ( void* ) ( pNewData + m_nSize ) , 0 , ( size_t ) ( nNewSize - m_nSize ) * sizeof( TYPE ) );
      for (FXSIZE i = 0; i < nNewSize - m_nSize; i++)
#pragma push_macro("new")
#undef new
        ::new( ( void* ) ( pNewData + m_nSize + i ) ) TYPE;
#pragma pop_macro("new")

      delete[]( BYTE* )m_pData;
      m_pData = pNewData;
      m_nSize = nNewSize;
      m_nMaxSize = nNewMax;
    }
  }
  void    FreeExtra()
  {
    if (m_nSize != m_nMaxSize)
    {
      TYPE* pNewData = NULL;
      if (m_nSize != 0)
      {
        pNewData = ( TYPE* ) new BYTE[ m_nSize * sizeof( TYPE ) ];
        memcpy_s( pNewData , m_nSize * sizeof( TYPE ) ,
          m_pData , m_nSize * sizeof( TYPE ) );
      }
      delete[]( BYTE* )m_pData;
      m_pData = pNewData;
      m_nMaxSize = m_nSize;
    }
  }
  void    RemoveAll() { SetSize( 0 , -1 ); }
  void    clear() { SetSize( 0 , -1 ); }
  const TYPE& GetAt( FXSIZE nIndex ) const
  {
    ASSERT( nIndex >= 0 && nIndex < m_nSize );
    if (nIndex >= 0 && nIndex < m_nSize)
      return m_pData[ nIndex ];
    throw 1;
  }
  TYPE&   GetAt( FXSIZE nIndex )
  {
    ASSERT( nIndex >= 0 && nIndex < m_nSize );
    if (nIndex >= 0 && nIndex < m_nSize)
      return m_pData[ nIndex ];
    throw 1;
  }
  TYPE& front() { return GetAt( 0 ) ; }
  void    SetAt( FXSIZE nIndex , ARG_TYPE newElement )
  {
    ASSERT( nIndex >= 0 && nIndex < m_nSize );
    if (nIndex >= 0 && nIndex < m_nSize)
      m_pData[ nIndex ] = newElement;
    else
      throw 1;
  }
  const TYPE& ElementAt( FXSIZE nIndex ) const
  {
    ASSERT( nIndex >= 0 && nIndex < m_nSize );
    if (nIndex >= 0 && nIndex < m_nSize)
      return m_pData[ nIndex ];
    throw 1;
  }
  TYPE&   ElementAt( FXSIZE nIndex )
  {
    ASSERT( nIndex >= 0 && nIndex < m_nSize );
    if (nIndex >= 0 && nIndex < m_nSize)
      return m_pData[ nIndex ];
    throw 1;
  }
  const TYPE* GetData() const { return ( const TYPE* ) m_pData; }
  TYPE*   GetData() { return ( TYPE* ) m_pData; }
  void    SetAtGrow( FXSIZE nIndex , const ARG_TYPE newElement )
  {
    ASSERT( nIndex >= 0 );
    if (nIndex < 0)
      throw 1;

    if (nIndex >= m_nSize)
    {
      if (m_nGrowBy <= 0)
        m_nGrowBy = 1 ;

      //      SetSize( nIndex + m_nGrowBy , -1 ) ; // was in FXArray (Error?)
      SetSize( nIndex + 1 , -1 ) ;
    }
    m_pData[ nIndex ] = newElement;
  }
  FXSIZE Add( ARG_TYPE newElement )
  {
    FXSIZE nIndex = m_nSize;
    SetAtGrow( nIndex , newElement );
    return nIndex;
  }
  FXSIZE Append( const FXArray& src )
  {
    ASSERT( this != &src );
    if (this == &src)
      throw 1;
    FXSIZE nOldSize = m_nSize;
    SetSize( m_nSize + src.m_nSize );
    _copyelements<TYPE>( m_pData + nOldSize , src.m_pData , src.m_nSize );
    return nOldSize;
  }
  void    Copy( const FXArray& src )
  {
    ASSERT( this != &src );
    if (this != &src)
    {
      SetSize( src.m_nSize );
      _copyelements<TYPE>( m_pData , src.m_pData , src.m_nSize );
    }
  }
  const TYPE& operator[]( size_t nIndex ) const { return GetAt( nIndex ); }
  TYPE& operator[]( FXSIZE nIndex ) { return ElementAt( nIndex ); }
  void    RemoveAt( FXSIZE nIndex , FXSIZE nCount = 1 )
  {
    ASSERT( nIndex >= 0 );
    ASSERT( nCount >= 0 );
    FXSIZE nUpperBound = nIndex + nCount;
    ASSERT( nUpperBound <= m_nSize && nUpperBound >= nIndex && nUpperBound >= nCount );

    if (nIndex < 0 || nCount < 0 || ( nUpperBound > m_nSize ) || ( nUpperBound < nIndex ) || ( nUpperBound < nCount ))
      throw 1;

    // just remove a range
    FXSIZE nMoveCount = m_nSize - ( nUpperBound );
    for (FXSIZE i = 0; i < nCount; i++)
      ( m_pData + nIndex + i )->~TYPE();
    if (nMoveCount)
    {
      memmove_s( m_pData + nIndex , nMoveCount * sizeof( TYPE ) ,
        m_pData + nUpperBound , nMoveCount * sizeof( TYPE ) );
    }
    m_nSize -= nCount;
  }
  FXSIZE    RemoveLast()
  {
    ASSERT( m_nSize );

    if (!m_nSize)
      throw 1;

    // just remove a last
    ( m_pData + m_nSize - 1 )->~TYPE();
    return --m_nSize ;
  }

  void    InsertAt( FXSIZE nIndex , ARG_TYPE newElement , FXSIZE nCount = 1 )
  {
    ASSERT( nIndex >= 0 );
    ASSERT( nCount > 0 );
    if (nIndex < 0 || nCount <= 0)
      throw 1;
    if (nIndex >= m_nSize)
      SetSize( nIndex + nCount , -1 );
    else
    {
      FXSIZE nOldSize = m_nSize;
      SetSize( m_nSize + nCount , -1 );
      for (FXSIZE i = 0; i < nCount; i++)
        ( m_pData + nOldSize + i )->~TYPE();
      memmove_s( m_pData + nIndex + nCount , ( nOldSize - nIndex ) * sizeof( TYPE ) ,
        m_pData + nIndex , ( nOldSize - nIndex ) * sizeof( TYPE ) );
      memset( ( void* ) ( m_pData + nIndex ) , 0 , nCount * sizeof( TYPE ) );
      for (FXSIZE i = 0; i < nCount; i++)
#pragma push_macro("new")
#undef new
        ::new( ( void* ) ( m_pData + nIndex + i ) ) TYPE;
#pragma pop_macro("new")
    }
    ASSERT( nIndex + nCount <= m_nSize );
    while (nCount--)
      m_pData[ nIndex++ ] = newElement;
  }
  void    InsertAt( FXSIZE nStartIndex , const FXArray* pNewArray )
  {
    ASSERT( pNewArray != NULL );
    ASSERT( nStartIndex >= 0 );

    if (pNewArray == NULL || nStartIndex < 0)
      throw 1;

    if (pNewArray->GetSize() > 0)
    {
      FXSIZE nCount = pNewArray->GetSize();
      if (nStartIndex >= m_nSize)
        SetSize( nStartIndex + nCount , -1 );
      else
      {
        FXSIZE nOldSize = m_nSize;
        SetSize( m_nSize + nCount , -1 );
        for (FXSIZE i = 0; i < nCount; i++)
          ( m_pData + nOldSize + i )->~TYPE();
        memmove_s( m_pData + nStartIndex + nCount , ( nOldSize - nStartIndex ) * sizeof( TYPE ) ,
          m_pData + nStartIndex , ( nOldSize - nStartIndex ) * sizeof( TYPE ) );
        memset( ( void* ) ( m_pData + nStartIndex ) , 0 , nCount * sizeof( TYPE ) );
        for (FXSIZE i = 0; i < nCount; i++)
#pragma push_macro("new")
#undef new
          ::new( ( void* ) ( m_pData + nStartIndex + i ) ) TYPE;
#pragma pop_macro("new")
      }
      for (FXSIZE i = 0; i < pNewArray->GetSize(); i++)
        SetAt( nStartIndex + i , pNewArray->GetAt( i ) );
    }
  }

  ///// Additional members, not defined in CArray
  size_t     Find( TYPE& value )
  {
    for (FXSIZE i = 0; i < GetSize(); i++)
    {
      if (ElementAt( i ) == value) return i;
    }
    return -1;
  }
  const TYPE& Last() const
  {
    ASSERT( m_nSize );
    if (m_nSize)
      return m_pData[ m_nSize - 1 ];
    throw 1 ;
  }
  TYPE& Last()
  {
    ASSERT( m_nSize );
    if (m_nSize)
      return m_pData[ m_nSize - 1 ];
    throw 1 ;
  }

  const TYPE& GetLast() const { return Last() ; }
  TYPE& GetLast() { return Last() ; }
  void Reverse()
  {
    TYPE Tmp ;
    for (int i = 0 ; i < Count() / 2 ; i++)
    {
      Tmp = m_pData[ i ] ;
      m_pData[ i ] = m_pData[ GetUpperBound() - i ] ;
      m_pData[ GetUpperBound() - i ] = Tmp ;
    }
  }
};

#pragma pop_macro("new")

class FXStringArray : public FXArray<FXString>
{
};

class FXPtrArray : public FXArray<void*>
{
};

typedef FXArray<double> CDblArray ;
typedef FXArray<int> CIntArray ;

class FXDblArray : public FXArray<double>
{};
class FXIntArray : public FXArray<int>
{};
class FXInt64Array : public FXArray<__int64>
{};
class FXUIntArray : public FXArray<UINT>
{};

#endif //#ifndef FXARRAY_INCLUDE
#pragma once

#include <io.h>
#include <math/Intf_sup.h>
#include <fxfc\fxext.h>
#include <classes\drect.h>
#include <classes\dpoint.h>
#include <helpers\FXParser2.h>


static LPCTSTR pOpenBrackets = _T( "({[" ) ;
static LPCTSTR pCloseBrackets = _T( ")}]" ) ;
static LPCTSTR pDelimiters = _T( "; (\t{[<)}]>\r\n" ) ;
static LPCTSTR pNumberElements = _T( "0123456789.+-" ) ;
static LPCTSTR pInternalSeparators = _T( ", \t" ) ;

inline bool IsOpenBracket( TCHAR ch )
{
  return (_tcschr( pOpenBrackets , ch ) != NULL) ;
}

inline bool IsCloseBracket( TCHAR ch )
{
  return (_tcschr( pCloseBrackets , ch ) != NULL) ;
}

inline bool IsNumberElement( TCHAR ch )
{
  return (_tcschr( pNumberElements , ch ) != NULL) ;
}

inline LPCTSTR IsNameWithIndex( LPCTSTR pName , int * pIndex = NULL )
{
  LPCTSTR pOpenBracket = _tcschr( pName , _T( '[' ) ) ;
  if ( pOpenBracket && _tcschr( pOpenBracket + 1 , _T( ']' ) ) )
  {
    if ( pIndex )
      *pIndex = atoi( pOpenBracket + 1 ) ;
    return pOpenBracket ;
  }
  return NULL ;
}
inline FXString GetNameNoBrackets( LPCTSTR pWithBrackets , int iBracketPos )
{
  FXString NoBracket( pWithBrackets ) ;
  return NoBracket.Left( iBracketPos ) ;
}


inline int GetArray( const FXString& AsString , TCHAR cType , int iMaxNItems , void * pArray )
{
  TCHAR c = AsString[ 0 ] ;
  FXSIZE iPos = (c == _T( '(' )) || (c == _T( '[' )) || (c == _T( '{' )) ;
  int iItemCnt = 0 ;
  FXString Token ;
  switch ( cType )
  {
    case 'd':
    case 'i':
    case 'u':
    case 'x':
    {
      FXSIZE * pInt = (FXSIZE*) pArray ;
      int iVal ;
      do
      {
        Token = AsString.Tokenize( _T( " \t\n\r,;:" ) , iPos ) ;
        if ( Token.IsEmpty() )
          return iItemCnt ;
        int iRes = 0 ;
        switch ( cType )
        {
          case 'd':
          case 'i':
            iRes = sscanf_s( Token , "%d" , &iVal ) ;
            if ( iRes && (iItemCnt < iMaxNItems) )
              pInt[ iItemCnt ] = iVal ;
            break ;
          case 'u':
            iRes = sscanf_s( Token , "%u" , &iVal ) ;
            if ( iRes && (iItemCnt < iMaxNItems) )
              pInt[ iItemCnt ] = iVal ;
            break ;
          case 'x':
            iRes = sscanf_s( Token , "%x" , &iVal ) ;
            if ( iRes && (iItemCnt < iMaxNItems) )
              pInt[ iItemCnt ] = iVal ;
            break ;
        }
        iItemCnt++ ;
      } while ( iItemCnt < iMaxNItems ) ;
    }
    break ;
    case 'f':
    case 'e':
    case 'g':
    {
      double * pDbl = (double*) pArray ;
      double dVal ;
      do
      {
        Token = AsString.Tokenize( _T( " \t\n\r,;:" ) , iPos ) ;
        if ( Token.IsEmpty() )
          return iItemCnt ;
        int iRes = 0 ;
        switch ( cType )
        {
          case 'f':
            iRes = sscanf_s( Token , "%lf" , &dVal ) ;
            if ( iRes && (iItemCnt < iMaxNItems) )
              pDbl[ iItemCnt ] = dVal ;
            break ;
          case 'g':
            iRes = sscanf_s( Token , "%lg" , &dVal ) ;
            if ( iRes && (iItemCnt < iMaxNItems) )
              pDbl[ iItemCnt ] = dVal ;
            break ;
          case 'e':
            iRes = sscanf_s( Token , "%le" , &dVal ) ;
            if ( iRes && (iItemCnt < iMaxNItems) )
              pDbl[ iItemCnt ] = dVal ;
            break ;
        }
        iItemCnt++ ;
      } while ( iItemCnt < iMaxNItems ) ;
    }
    break ;
    default:
      TRACE1( _T( "GetArray: Unknown data type %c" ) , cType ) ;
      return FALSE ;
  }
  return iItemCnt ;
}

inline int GetArray( const FXPropertyKit& pk , LPCTSTR PropName ,
  TCHAR cType , int iNItems , void * pArray )
{
  FXString AsString ;
  if ( !pk.GetString( PropName , AsString ) )
    return FALSE ;
  return GetArray( AsString , cType , iNItems , pArray ) ;
}

inline int GetArray( const FXString& AsString , TCHAR cType , void * pArray )
{
  TCHAR c = AsString[ 0 ] ;
  FXSIZE iPos = (c == _T( '(' )) || (c == _T( '[' )) || (c == _T( '{' )) ;
  int iItemCnt = 0 ;
  FXString Token ;
  switch ( cType )
  {
    case 'd':
    case 'i':
    case 'u':
    case 'x':
    {
      CIntArray * pInt = (CIntArray*) pArray ;
      pInt->RemoveAll() ;
      int iVal ;
      while ( 1 )
      {
        Token = AsString.Tokenize( _T( " \t\n\r,;:" ) , iPos ) ;
        if ( Token.IsEmpty() )
          return (int) pInt->GetCount() ;
        int iRes = 0 ;
        switch ( cType )
        {
          case 'd':
          case 'i':
            iRes = sscanf_s( Token , "%d" , &iVal ) ;
            if ( iRes )
              pInt->Add( iVal ) ;
            break ;
          case 'u':
            iRes = sscanf_s( Token , "%u" , &iVal ) ;
            if ( iRes )
              pInt->Add( iVal ) ;
            break ;
          case 'x':
            iRes = sscanf_s( Token , "%x" , &iVal ) ;
            if ( iRes )
              pInt->Add( iVal ) ;
            break ;
        }
        iItemCnt++ ;
      }
    }
    break ;
    case 'f':
    case 'e':
    case 'g':
    {
      CDblArray * pDbl = (CDblArray*) pArray ;
      pDbl->RemoveAll() ;
      double dVal ;
      while ( 1 )
      {
        Token = AsString.Tokenize( _T( " \t\n\r,;:" ) , iPos ) ;
        if ( Token.IsEmpty() )
          return (int) pDbl->GetCount() ;
        int iRes = 0 ;
        switch ( cType )
        {
          case 'f':
            iRes = sscanf_s( Token , "%lf" , &dVal ) ;
            if ( iRes )
              pDbl->Add( dVal ) ;
            break ;
          case 'g':
            iRes = sscanf_s( Token , "%lg" , &dVal ) ;
            if ( iRes )
              pDbl->Add( dVal ) ;
            break ;
          case 'e':
            iRes = sscanf_s( Token , "%le" , &dVal ) ;
            if ( iRes )
              pDbl->Add( dVal ) ;
            break ;
        }
        iItemCnt++ ;
      }
    }
    break ;
    default:
      TRACE1( _T( "GetArray: Unknown data type %c" ) , cType ) ;
      return FALSE ;
  }
  return iItemCnt ;
}

inline int GetArray( const FXPropertyKit& pk , LPCTSTR PropName ,
  TCHAR cType , void * pArray )
{
  FXString AsString ;
  if ( !pk.GetString( PropName , AsString ) )
    return FALSE ;
  return GetArray( AsString , cType , pArray ) ;
}

inline BOOL WriteArray( FXString& AsString ,
  TCHAR cType , int iNItems , void * pArray )
{
  int iItemCnt = 0 ;
  for ( ; iItemCnt < iNItems ; iItemCnt++ )
  {
    TCHAR Token[ 50 ] ;
    switch ( cType )
    {
      case 'd': sprintf_s( Token , "%d" , *(((int*) pArray) + iItemCnt) ) ; break ;
      case 'u': sprintf_s( Token , "%u" , *(((UINT*) pArray) + iItemCnt) ) ; break ;
      case 'f': sprintf_s( Token , "%lf" , *(((double*) pArray) + iItemCnt) ) ; break ;
      case 'g': sprintf_s( Token , "%lg" , *(((double*) pArray) + iItemCnt) ) ; break ;
      default:
      {
        TRACE1( _T( "WriteetArray: Unknown data type %c" ) , cType ) ;
        return FALSE ;
      }
    }
    AsString += Token ;
    if ( iItemCnt < iNItems - 1 )
      AsString += _T( ',' ) ;
  }  ;
  return (iItemCnt == iNItems) ;
}

inline BOOL WriteArray( FXPropertyKit& pk , LPCTSTR PropName ,
  TCHAR cType , int iNItems , void * pArray )
{
  FXString AsString ;
  if ( WriteArray( AsString , cType , iNItems , pArray ) )
  {
    pk.WriteString( PropName , AsString ) ;
    return TRUE ;
  }
  return FALSE ;
}

inline BOOL WriteArray( FXString& AsString ,
  TCHAR cType , LPCTSTR pFormat , int iNItems , void * pArray )
{
  int iItemCnt = 0 ;
  for ( ; iItemCnt < iNItems ; iItemCnt++ )
  {
    TCHAR Token[ 50 ] ;
    switch ( cType )
    {
      case 'd': sprintf_s( Token , pFormat , *(((int*) pArray) + iItemCnt) ) ; break ;
      case 'u': sprintf_s( Token , pFormat , *(((UINT*) pArray) + iItemCnt) ) ; break ;
      case 'f': sprintf_s( Token , pFormat , *(((double*) pArray) + iItemCnt) ) ; break ;
      case 'g': sprintf_s( Token , pFormat , *(((double*) pArray) + iItemCnt) ) ; break ;
      default:
      {
        TRACE1( _T( "WriteetArray: Unknown data type %c" ) , cType ) ;
        return FALSE ;
      }
    }
    AsString += Token ;
    if ( iItemCnt < iNItems - 1 )
      AsString += _T( ',' ) ;
  }  ;
  return (iItemCnt == iNItems) ;
}

inline BOOL WriteArray( FXPropertyKit& pk , LPCTSTR PropName ,
  TCHAR cType , LPCTSTR pFormat , int iNItems , void * pArray )
{
  FXString AsString ;
  if ( WriteArray( AsString , cType , pFormat , iNItems , pArray ) )
  {
    pk.WriteString( PropName , AsString ) ;
    return TRUE ;
  }
  return FALSE ;
}


inline BOOL WriteArray( FXString& AsString , TCHAR cType ,
  void * pArray , LPCTSTR pFormat = NULL )
{
  int iItemCnt = 0 ;
  CIntArray * pInt = (CIntArray*) pArray ;
  CDblArray * pDbl = (CDblArray*) pArray ;
  int iNItems = (cType == 'd' || cType == 'u'
    || cType == 'i' || cType == 'x') ? (int) pInt->GetCount()
    : (cType == 'f' || cType == 'g' || cType == 'e') ? (int) pDbl->GetCount() : 0 ;
  if ( iNItems == 0 )
  {
    {
      TRACE1( _T( "WriteArray: Unknown data type %c" ) , cType ) ;
      return FALSE ;
    }
  }

  for ( ; iItemCnt < iNItems ; iItemCnt++ )
  {
    TCHAR Token[ 50 ] ;
    switch ( cType )
    {
      case 'd':
        _stprintf_s( Token , pFormat ? pFormat : _T( "%d" ) ,
          pInt->GetAt( iItemCnt ) ) ;
        break ;
      case 'i':
        _stprintf_s( Token , pFormat ? pFormat : _T( "%i" ) ,
          pInt->GetAt( iItemCnt ) ) ; break ;
      case 'u':
        _stprintf_s( Token , pFormat ? pFormat : _T( "%u" ) ,
          pInt->GetAt( iItemCnt ) ) ; break ;
      case 'x':
        _stprintf_s( Token , pFormat ? pFormat : _T( "%x" ) ,
          pInt->GetAt( iItemCnt ) ) ; break ;
      case 'f':
        _stprintf_s( Token , pFormat ? pFormat : _T( "%lf" ) ,
          pDbl->GetAt( iItemCnt ) ) ; break ;
      case 'g':
        _stprintf_s( Token , pFormat ? pFormat : _T( "%lg" ) ,
          pDbl->GetAt( iItemCnt ) ) ; break ;
      case 'e':
        _stprintf_s( Token , pFormat ? pFormat : _T( "%le" ) ,
          pDbl->GetAt( iItemCnt ) ) ; break ;
      default:
      {
        TRACE1( _T( "WriteArray: Unknown data type %c" ) , cType ) ;
        return FALSE ;
      }
    }
    AsString += Token ;
    if ( iItemCnt < iNItems - 1 )
      AsString += _T( ',' ) ;
  }  ;
  return (iItemCnt == iNItems) ;
}

inline BOOL WriteArray( FXPropertyKit& pk , LPCTSTR PropName ,
  TCHAR cType , void * pArray )
{
  FXString AsString ;
  if ( WriteArray( AsString , cType , pArray ) )
  {
    pk.WriteString( PropName , AsString ) ;
    return TRUE ;
  }
  return FALSE ;
}

inline BOOL GetRect( FXPropertyKit& pk , LPCTSTR PropName , CRect& Rect )
{
  return GetArray( pk , PropName , _T( 'd' ) , 4 , &Rect ) == 4 ;
}
inline void WriteRect( FXPropertyKit& pk , LPCTSTR PropName , CRect& Rect )
{
  WriteArray( pk , PropName , _T( 'd' ) , 4 , &Rect ) ;
}
inline BOOL GetRect( FXPropertyKit& pk , LPCTSTR PropName , CDRect& Rect )
{
  return GetArray( pk , PropName , _T( 'f' ) , 4 , &Rect ) == 4 ;
}
inline void WriteRect( FXPropertyKit& pk , LPCTSTR PropName , CDRect& Rect )
{
  WriteArray( pk , PropName , _T( 'f' ) , 4 , &Rect ) ;
}
inline BOOL GetPoint( FXPropertyKit& pk , LPCTSTR PropName , CPoint& Pt )
{
  return GetArray( pk , PropName , _T( 'd' ) , 2 , &Pt ) == 2 ;
}
inline void WritePoint( FXPropertyKit& pk , LPCTSTR PropName , CPoint& Pt )
{
  WriteArray( pk , PropName , _T( 'd' ) , 2 , &Pt ) ;
}
inline BOOL GetPoint( FXPropertyKit& pk , LPCTSTR PropName , CDPoint& Pt )
{
  return GetArray( pk , PropName , _T( 'f' ) , 2 , &Pt ) == 2 ;
}
inline void WritePoint( FXPropertyKit& pk , LPCTSTR PropName , CDPoint& Pt )
{
  WriteArray( pk , PropName , _T( 'f' ) , 2 , &Pt ) ;
}
inline BOOL GetPoint( FXPropertyKit& pk , LPCTSTR PropName , cmplx& Pt )
{
  return GetArray( pk , PropName , _T( 'f' ) , 2 , &Pt ) == 2 ;
}
inline void WritePoint( FXPropertyKit& pk , LPCTSTR PropName , cmplx& Pt )
{
  WriteArray( pk , PropName , _T( 'f' ) , 2 , &Pt ) ;
}
inline bool FillFXStringFromFile( LPCTSTR pFileName , FXString& Dest )
{
  CFile fr ;
  if ( fr.Open( pFileName , CFile::modeRead ) )
  {
    int iLen = (int) fr.GetLength() ;
    LPTSTR pBuf = Dest.GetBufferSetLength( iLen + 1 ) ;
    if ( pBuf )
    {
      int iRead = fr.Read( pBuf , iLen ) ;
      fr.Close() ;
      if ( iRead == iLen )
      {
        pBuf[ iLen ] = 0 ;
        Dest.ReleaseBuffer() ;
        return TRUE ;
      }
    }
    fr.Close() ;
  }
  return FALSE ;
}

inline FXSIZE WriteToFile( char * pData , FXSIZE iDataLen , FILE * pSaveFile )
{
  if ( !pSaveFile || !iDataLen || !pData )
    return 0 ;
  void * pVoidData = (void*) pData ;
  FXSIZE iWritten = fwrite( pVoidData , 1 , iDataLen , pSaveFile ) ;
  return iWritten ;
}

inline FXSIZE WriteToFile( FXString Data , FILE * pSaveFile )
{
  if ( !pSaveFile || Data.IsEmpty() )
    return 0 ;
  FXSIZE iWritten = fwrite( (LPCTSTR) Data , sizeof( TCHAR ) , Data.GetLength() , pSaveFile ) ;
  return iWritten ;
}


class FXPropKit2 : public FXPropertyKit
{
public:
  FXSIZE m_iLastItemBegin ;
  FXSIZE m_iLastItemEnd ;
  FXPropKit2()
  {
    m_iLastItemBegin = m_iLastItemEnd = 0 ;
  } ;
  FXPropKit2( const FXString& Src )
  {
    SetString( Src );
  } ;
  FXPropKit2& operator=( LPCTSTR pStr )
  {
    *(FXString*) this = pStr ;
    m_iLastItemBegin = m_iLastItemEnd = 0 ;
    return *this ;
  }
  inline int GetStringToChars( LPCTSTR pName , TCHAR * pChars , int iCharsLen )
  {
    FXSIZE iNamePos = Find( pName ) ;
    if ( iNamePos < 0 )
      return 0 ;
    if ( iNamePos > 0 )
    {
      if ( isalpha( GetAt( iNamePos - 1 ) ) )
        return 0 ;
    }
    FXSIZE iPosAfter = iNamePos + _tcslen( pName ) ;
    while ( GetAt( iPosAfter ) == _T( ' ' ) )
      iPosAfter++ ;
    if ( GetAt( iPosAfter ) != _T( '=' ) )
      return 0 ;
    FXSIZE iSemicolonPos = Find( ';' , ++iPosAfter ) ;
    FXSIZE iCopyLenChars = ( iSemicolonPos > 0 ) ?
      ( iSemicolonPos - iPosAfter ) : GetLength() - iPosAfter ;
    if ( iCopyLenChars > iCharsLen - 1 )
      iCopyLenChars = iCharsLen - 1 ;
    memcpy( pChars , ( ( LPCTSTR ) ( *this ) ) + iPosAfter , iCopyLenChars * sizeof( TCHAR ) ) ;
    pChars[ iCopyLenChars ] = 0 ;
    return (int)iCopyLenChars ;
  }
  inline int GetIndexForName( LPCTSTR pName , int& iEquPos )
  {
    FXSIZE iNamePos = Find( pName ) ;
    if ( iNamePos < 0 )
      return -2 ;
    FXSIZE iPosAfter = iNamePos + _tcslen( pName ) ;
    if ( GetAt( iPosAfter ) != _T( '[' ) )
      return -1 ;
    int iIndex = atoi( ((LPCTSTR) (*this)) + iPosAfter + 1 ) ;
    iEquPos = (int) Find( _T( '=' ) , iPosAfter ) ;
    return iIndex ;
  }

  int GetAllPropertyValuesAsString( LPCTSTR pName , FXString& Result )
  {
    int iIndex = -1 ;
    LPCTSTR pBracket = IsNameWithIndex( pName , &iIndex ) ;
    if ( !pBracket || iIndex < 0 )
      return -1 ;// name is without index
    int iBracketPos = (int) (pBracket - pName) ;
    FXString NoBracket = GetNameNoBrackets( pName , iBracketPos ) ;
    FXSIZE iNamePos = Find( NoBracket ) ;
    if ( iNamePos < 0 )
      return -3 ; // No such name in content
    FXSIZE iValPos = Find( _T( '=' ) , iBracketPos ) ;
    if ( iValPos < 0 )
      return -4 ; // Name exists, but is not name of property
    FXSIZE iEndPos = m_iLastItemEnd = Find( _T( ';' ) , iValPos ) ;
    Result = Mid( iValPos , iEndPos - iValPos ) ;
    return iIndex ;
  }
  int GetIndexedValueAsString( LPCTSTR pName , FXString& Result )
  {
    int iIndex = GetAllPropertyValuesAsString( pName , Result ) ;
    if ( iIndex < 0 )
      return iIndex ;
    FXSIZE iNextTokenPos = 0 ;
    int iTokenCnt = 0 ;
    while ( iTokenCnt <= iIndex )
    {
      FXString Token = Tokenize( _T( " ,\t" ) , iNextTokenPos ) ;
      if ( iNextTokenPos >= 0
        && iTokenCnt == iIndex )
      {
        Result = Token ;
        return iIndex ;
      }
      iTokenCnt++ ;
    }
    return -5 ; // index is too high
  }
  inline bool GetCmplx( LPCTSTR lpszEntry , cmplx& cmplxVal )
  {
    FXParser2 Str ;
    if ( FXPropertyKit::GetString( lpszEntry , Str ) )
    {
      return Str.StrToCmplx( cmplxVal ) ;
    }
    return false ;
  }
  inline bool WriteCmplx( LPCTSTR lpszEntry , const cmplx& cmplxVal ,
    LPCTSTR pFormat = NULL )
  {
    FXString Tmp ;
    Tmp.Format( pFormat ? pFormat : _T( "(%lg,%lg)" ) ,
      cmplxVal.real() , cmplxVal.imag() ) ;
    return WriteString( lpszEntry , Tmp ) ;
  }
  inline bool WriteCmplxArray( LPCTSTR lpszEntry , 
    const cmplx * pCmplxData , FXSIZE Len ,
    LPCTSTR pFormat = NULL )
  {
    FXString Tmp , All ;
    const cmplx * pcEnd = pCmplxData + Len ;
    while ( pCmplxData < pcEnd )
    {
      Tmp.Format( pFormat ? pFormat : _T( "(%lg,%lg)" ) ,
        (*pCmplxData).real() , (*pCmplxData).imag() ) ;
      pCmplxData++ ;
      All += Tmp ;
      if ( pCmplxData < pcEnd )
        All += ',' ;
    }
    if ( lpszEntry && *lpszEntry )
      return WriteString( lpszEntry , All ) ;
    else
    {
      *this += All + ';' ;
    }
    return true ;
  }
  inline bool GetCoord3( LPCTSTR lpszEntry , CCoord3& Coord3 )
  {
    FXParser2 Str ;
    if ( GetString( lpszEntry , Str ) )
    {
      return Str.StrToCoord3( Coord3 ) ;
    }
    return false ;
  }
  inline void WriteCoord3( LPCTSTR lpszEntry , CCoord3& Coord3 ,
    LPCTSTR pFormat = NULL )
  {
    FXString Tmp ;
    Tmp.Format( pFormat ? pFormat : _T( "(%lg,%lg,%lg)" ) ,
      Coord3.m_x , Coord3.m_y , Coord3.m_z ) ;
    WriteString( lpszEntry , Tmp ) ;
  }
  inline bool IsOutOfBrackets( int iStart , int iEnd ) const
  {
    int iNotClosedBrackets[ 3 ] = {0 , 0 , 0} ; // for ( { and [
    for ( int i = iStart ; i <= iEnd ; i++ )
    {
      TCHAR ch = FXString::GetAt( i ) ;
      switch ( ch )
      {
        case _T( '(' ): ++iNotClosedBrackets[ 0 ] ; break ;
        case _T( ')' ): --iNotClosedBrackets[ 0 ] ; break ;
        case _T( '{' ): ++iNotClosedBrackets[ 1 ] ; break ;
        case _T( '}' ): --iNotClosedBrackets[ 1 ] ; break ;
        case _T( '[' ): ++iNotClosedBrackets[ 2 ] ; break ;
        case _T( ']' ): --iNotClosedBrackets[ 2 ] ; break ;
        default: break ;
      }
    }
    return (!iNotClosedBrackets[ 0 ] && !iNotClosedBrackets[ 1 ] && !iNotClosedBrackets[ 2 ]) ;
  }
  inline int FindOpenBracket( int iStart = 0 , int iEnd = -1 ) const
  {
    if ( iEnd == -1 )
      iEnd = (int)GetLength() ;
    for ( int i = iStart ; i < iEnd ; i++ )
    {
      TCHAR ch = FXString::GetAt( i ) ;
      if ( ch == _T( '(' ) || ch == _T( '{' ) || ch == _T( '[' ) )
        return i ; 
    }
    
    return -1 ;
  }
  inline int FindClosingBracket( TCHAR Bracket , int iPos ) const
  {
    int iNotClosedBrackets[ 3 ] = {0 , 0 , 0} ; // for ( { and [
    switch ( Bracket )
    {
      case _T( ')' ): ++iNotClosedBrackets[ 0 ] ; break ;
      case _T( '}' ): ++iNotClosedBrackets[ 1 ] ; break ;
      case _T( ']' ): ++iNotClosedBrackets[ 2 ] ; break ;
      default: ASSERT( 0 ) ;  break ;
    }
    FXSIZE iLen = GetLength() ;
    for ( FXSIZE i = iPos ; i < iLen ; i++ )
    {
      TCHAR ch = FXString::GetAt( i ) ;
      switch ( ch )
      {
        case _T( '(' ): ++iNotClosedBrackets[ 0 ] ; break ;
        case _T( ')' ): --iNotClosedBrackets[ 0 ] ; break ;
        case _T( '{' ): ++iNotClosedBrackets[ 1 ] ; break ;
        case _T( '}' ): --iNotClosedBrackets[ 1 ] ; break ;
        case _T( '[' ): ++iNotClosedBrackets[ 2 ] ; break ;
        case _T( ']' ): --iNotClosedBrackets[ 2 ] ; break ;
        default: break ;
      }
      if ( !iNotClosedBrackets[ 0 ] && !iNotClosedBrackets[ 1 ] && !iNotClosedBrackets[ 2 ] )
        return (int)i ;
    }
    return -1 ;
  }
  inline int FindClosingBracket( FXSIZE iStart = 0 , FXSIZE iEnd = -1 ) const
  {
    int iNotClosedBrackets[ 3 ] = { 0 , 0 , 0 } ; // for ( { and [
    switch ( GetAt( iStart ) )
    {
    case _T( '(' ): ++iNotClosedBrackets[ 0 ] ; break ;
    case _T( '{' ): ++iNotClosedBrackets[ 1 ] ; break ;
    case _T( '[' ): ++iNotClosedBrackets[ 2 ] ; break ;
    default: ASSERT( 0 ) ;  break ;
    }
    if ( iEnd == -1 )
      iEnd = GetLength() ;
    for ( FXSIZE i = iStart + 1 ; i < iEnd ; i++ )
    {
      TCHAR ch = FXString::GetAt( i ) ;
      switch ( ch )
      {
      case _T( '(' ): ++iNotClosedBrackets[ 0 ] ; break ;
      case _T( ')' ): --iNotClosedBrackets[ 0 ] ; break ;
      case _T( '{' ): ++iNotClosedBrackets[ 1 ] ; break ;
      case _T( '}' ): --iNotClosedBrackets[ 1 ] ; break ;
      case _T( '[' ): ++iNotClosedBrackets[ 2 ] ; break ;
      case _T( ']' ): --iNotClosedBrackets[ 2 ] ; break ;
      default: break ;
      }
      if ( !iNotClosedBrackets[ 0 ] && !iNotClosedBrackets[ 1 ] && !iNotClosedBrackets[ 2 ] )
        return (int) i ;
    }
    return -1 ;
  }


  inline FXSIZE FindKeyWidthBrackets( LPCTSTR lpszEntry , FXSIZE iPos ) const
  {
    FXSIZE pos = FXString::Find( lpszEntry , iPos );
    if ( pos <= 0 )
      return pos;
    if ( _tcschr( pDelimiters , GetAt( pos - 1 ) ) )
    {
      if ( IsOutOfBrackets( (int)iPos , (int)pos ) )
        return pos;
    }
    return -1;
  }
  inline FXSIZE FindKeyWidthBrackets( LPCTSTR lpszEntry ) const
  {
    return FindKeyWidthBrackets( lpszEntry , 0 ) ;
  }

  inline bool GetStringWithBrackets( LPCTSTR lpszEntry , FXString& sValue , FXSIZE iPos = 0 ) const
  {
    FXAutolock lock( ((FXPropertyKit*) this)->m_Lock );
    FXString tS( lpszEntry ); tS += '=';
    FXSIZE pos = FindKeyWidthBrackets( tS , iPos );
    if ( pos >= 0 )
    {
      pos += tS.GetLength();
      TCHAR cFirst = GetAt( pos ) ;
      if ( IsOpenBracket( cFirst ) )
      {
        int iPos = FindClosingBracket( (cFirst == _T( '(' )) ? _T( ')' ) :
          (cFirst == _T( '{' )) ? _T( '}' ) : _T( ']' ) , (int) pos + 1 ) ;
        if ( iPos > pos + 1 )
        {
          sValue = ::FxUnregularize( Mid( pos + 1 , iPos - pos - 1 ) );
          return true ;
        }
        else
        {
          sValue.Empty() ;
          return false ;
        }
      }

      FXSIZE iEnd = Find( ';' , pos + 1 );
      if ( iEnd >= pos + 1 )
        sValue = ::FxUnregularize( Mid( pos , iEnd - pos - 1 ) );
      return true;
    }
    return false ;
  }

  inline bool GetTillSemicolonWithBrackets( 
    LPCTSTR lpszEntry , FXString& sValue , FXSIZE iPos = 0 ) const
  {
    FXAutolock lock( ((FXPropertyKit*) this)->m_Lock );
    FXString tS( lpszEntry ); tS += '=';
    int pos = (int)FindKeyWidthBrackets( tS , iPos );
    if ( pos >= 0 )
    {
      pos += (int)tS.GetLength();
      int iBegin = pos ;
      int iCloseBracketPos = -1 ;
      do 
      {
        int iFirstSemiPos = (int) Find( _T( ';' ) , pos ) ;
      int iOpenBracketPos = FindOpenBracket( pos , iFirstSemiPos ) ;
      if ( (iOpenBracketPos < 0) || (iOpenBracketPos > iFirstSemiPos) )
      {
          sValue = Mid( iBegin , iFirstSemiPos - iBegin + 1 ) ;
        return true ;
      }
        else
          iCloseBracketPos = FindClosingBracket( iOpenBracketPos ) ;
        pos = iCloseBracketPos + 1 ;
      } while ( iCloseBracketPos > 0 && sValue.IsEmpty() );
      if ( !sValue.IsEmpty() )
      {
        sValue = ::FxUnregularize( sValue );
          return true ;
        }

    }
    return false ;
  }

  inline int FindKeyPositions( LPCTSTR pKey , CUIntArray& Positions )
  {
    FXString WhatToFind( pKey );
    WhatToFind += _T( '=' );

    Positions.RemoveAll() ;
    int iPos = 0 ;
    while ( iPos >= 0 )
    {
      iPos = (int)FindKeyWidthBrackets( WhatToFind , iPos ) ;
      if ( iPos >= 0 )
        Positions.Add( iPos ) ;
      else
        break ;
      iPos += (int) WhatToFind.GetLength() ;
    } ;
    return (int) Positions.GetCount() ;
  }
  bool GetDouble( LPCTSTR pName , double& dVal )
  {
    FXString ResultAsString ;
    if ( GetIndexedValueAsString( pName , ResultAsString ) >= 0 )
    {
      dVal = atof( ResultAsString ) ;
      return true ;
    }
    return FXPropertyKit::GetDouble( pName , dVal ) ;
  }
  bool GetDouble( LPCTSTR pName , double& dVal , double dDefault )
  {
    if ( !FXPropertyKit::GetDouble( pName , dVal ) )
      return FXPropertyKit::WriteDouble( pName , dVal = dDefault ) ;
    return true ;
  }
  bool GetInt( LPCTSTR pName , int& iVal )
  {
    FXString ResultAsString ;
    if ( GetIndexedValueAsString( pName , ResultAsString ) >= 0 )
    {
      iVal = atoi( ResultAsString ) ;
      return true ;
    }
    return FXPropertyKit::GetInt( pName , iVal );
  }
  bool GetInt64( LPCTSTR pName , __int64& i64Val )
  {
    FXString ResultAsString ;
    if (GetIndexedValueAsString( pName , ResultAsString ) >= 0)
    {
      i64Val = _ttoi64( ResultAsString ) ;
      return true ;
    }
    return FXPropertyKit::GetInt64( pName , i64Val );
  }

  //bool GetInt(LPCTSTR pName, int& iVal, int iDefault)
  //{
  //  return FXPropertyKit::GetInt(pName, iVal);
  //}
  bool GetInt( LPCTSTR pName , int& iVal , int iDefault )
  {
    if ( !FXPropertyKit::GetInt( pName , iVal ) )
      return FXPropertyKit::WriteInt( pName , iVal = iDefault );
    return true;
  }
  bool GetUIntOrHex( LPCTSTR pName , UINT& uiVal )
  {
    FXString AsText ;
    if ( GetString( pName , AsText ) )
    {
      FXSIZE fxRes ;
      if ( ConvToBinary( (LPCTSTR)AsText , fxRes ) )
      {
        uiVal = (UINT) fxRes ;
        return true ;
      }
    }
    return false ;
  }
  bool WriteUIntNotDecimal( LPCTSTR pName , UINT uiVal , TCHAR Format = _T( 'x' ) )
  {
    FXString AsText( _T( "0b" ) ) ;
    switch ( Format )
    {
      case _T( 'u' ): AsText.Format( _T( "%u" ) , uiVal ) ; break ;
      case _T( 'b' ):
      {
        bool bWasOne = false ;
        UINT uiShifted = uiVal ;
        for ( size_t i = 0; i < 32; i++ )
        {
          if ( uiShifted & 0x80000000 )
          {
            AsText += _T( '1' ) ;
            bWasOne = true ;
          }
          else if ( bWasOne )
          {
            AsText += _T( '0' ) ;
          }
          uiShifted <<= 1 ;
        }
        break ;
      }
      case _T( 'x' ): AsText.Format( _T( "0x%x" ) , uiVal ) ; break ;
      case _T( 'X' ): AsText.Format( _T( "0x%X" ) , uiVal ) ; break ;
      case _T( 'o' ): AsText.Format( _T( "0%o" ) , uiVal ) ; break ;
      case _T( 'd' ):
      default: AsText.Format( _T( "%d" ) , uiVal ) ; break ;
    }
    return WriteString( pName , AsText ) ;
  }

  bool GetInt( LPCTSTR pName , long& iVal )
  {
    FXString ResultAsString ;
    if ( GetIndexedValueAsString( pName , ResultAsString ) >= 0 )
    {
      iVal = atoi( ResultAsString ) ;
      return true ;
    }
    return FXPropertyKit::GetInt( pName , (int&) iVal ) ;
  }

  bool GetInt( LPCTSTR pName , long& iVal , long iDefault )
  {
    if ( !FXPropertyKit::GetInt( pName , (int&) iVal ) )
      return WriteInt( pName , iVal = iDefault ) ;
    return true ;
  }
  bool WriteInt( LPCTSTR pName , int iVal )
  {
    int iIndex = -1 ;
    LPCTSTR pBracket = IsNameWithIndex( pName , &iIndex ) ;
    if (pBracket && iIndex >= 0)
    {
      FXString NameNoBracket = GetNameNoBrackets( pName , ( int ) ( pBracket - pName ) ) ;
      CIntArray AllData ;
      GetArray( ( FXPropertyKit ) *this , NameNoBracket , 'd' , &AllData ) ;
      int iArrLen = ( int ) AllData.GetCount() ;
      if (iArrLen < iIndex + 1)
      {
        AllData.SetSize( iIndex + 1 ) ;
        // fill omitted data
        for (int i = iArrLen ; i < iIndex ; i++)
          AllData.SetAt( i , 0 ) ;
      }
      AllData.SetAt( iIndex , iVal ) ;
      return WriteArray( *this , ( LPCTSTR ) NameNoBracket , _T( 'd' ) , iArrLen , AllData.GetData() ) ;
    }
    else
      return FXPropertyKit::WriteInt( pName , iVal ) ;
  }
  bool WriteInt64( LPCTSTR pName , __int64 iVal )
  {
//     int iIndex = -1 ;
//     LPCTSTR pBracket = IsNameWithIndex( pName , &iIndex ) ;
//     if (pBracket && iIndex >= 0)
//     {
//       FXString NameNoBracket = GetNameNoBrackets( pName , ( int ) ( pBracket - pName ) ) ;
//       CIntArray AllData ;
//       GetArray( ( FXPropertyKit ) *this , NameNoBracket , 'd' , &AllData ) ;
//       int iArrLen = ( int ) AllData.GetCount() ;
//       if (iArrLen < iIndex + 1)
//       {
//         AllData.SetSize( iIndex + 1 ) ;
//         // fill omitted data
//         for (int i = iArrLen ; i < iIndex ; i++)
//           AllData.SetAt( i , 0 ) ;
//       }
//       AllData.SetAt( iIndex , iVal ) ;
//       return WriteArray( *this , ( LPCTSTR ) NameNoBracket , _T( 'd' ) , iArrLen , AllData.GetData() ) ;
//     }
//     else
    return FXPropertyKit::WriteInt64( pName , iVal ) ;
  }
  bool WriteInt( LPCTSTR pName , long lVal )
  {
    return WriteInt( pName , (int) lVal ) ;
  }
  bool WriteDouble( LPCTSTR pName , double dVal ,
    LPCTSTR pFormat = _T( "%.3f" ) )
  {
    int iIndex = -1 ;
    LPCTSTR pBracket = IsNameWithIndex( pName , &iIndex ) ;
    if ( pBracket && iIndex >= 0 )
    {
      FXString NameNoBracket = GetNameNoBrackets( pName , (int) (pBracket - pName) ) ;
      CDblArray AllData ;
      GetArray( (FXPropertyKit) *this , NameNoBracket , 'f' , &AllData ) ;
      int iArrLen = (int)AllData.GetCount() ;
      if ( iArrLen < iIndex + 1 )
      {
        AllData.SetSize( iIndex + 1 ) ;
        // fill omitted data
        for ( int i = iArrLen ; i < iIndex ; i++ )
          AllData.SetAt( i , 0. ) ;
      }
      AllData.SetAt( iIndex , dVal ) ;
      FXString AsString ;
      WriteArray( AsString , _T('f') , pFormat , (int) AllData.GetCount() , AllData.GetData() ) ;
      return WriteString( (LPCTSTR)NameNoBracket , AsString ) ;
    }
    else
      return FXPropertyKit::WriteDouble( pName , dVal ) ;
  }

  bool GetString( LPCTSTR pName , FXString& sVal , bool bToUnregularlize = true )
  {
    FXString FullItem ;
    int iIndex = GetAllPropertyValuesAsString( pName , FullItem ) ;

    if ( iIndex >= 0 )
    {
      int iTokenCount = 0 ;
      int iSepPos = (int)FullItem.FindOneOf( _T( " \t," ) ) ;
      int iIter = iSepPos + 1 ;
      int iNextSepPos = -1 ;
      int iLen = (int) FullItem.GetLength() ;
      while ( iTokenCount < iIndex )
      {
        while ( iIter < iLen
          && !_tcschr( pInternalSeparators , FullItem[ iIter ] ) )
        {
          iIter++ ;
        }
        iNextSepPos = iIter++ ;
        while ( iIter < iLen
          && _tcschr( pInternalSeparators , FullItem[ iIter++ ] ) ) ;
        iTokenCount++ ;
      }
      if ( iNextSepPos < 0 )
      {
        if ( iSepPos < 0 )
          sVal = FullItem ;
        else
          sVal = FullItem.Mid( 0 , iSepPos ) ;
        return true ;
      }
      else if ( iNextSepPos )
        sVal = FullItem.Mid( iSepPos + 1 , iNextSepPos - iSepPos ) ;
      else
        sVal = FullItem.Mid( iSepPos + 1 ) ;
      return true ;
    }
    return FXPropertyKit::GetString( pName , sVal , bToUnregularlize ) ;
  }
  bool WriteString( LPCTSTR pName , LPCTSTR pValue , bool bToRegularlize = true )
  {
    int iIndex = -1 ;
    if ( IsNameWithIndex( pName , &iIndex ) && iIndex >= 0 )
    {
      FXString FullItem ;
      GetAllPropertyValuesAsString( pName , FullItem ) ;

      int iLen = (int) FullItem.GetLength() ;
      int iTokenCount = 0 ;
      FXSIZE iPos = 0 , iPrevSepPos = 0 ;
      FXString NextToken = FullItem.Tokenize( pInternalSeparators , iPos ) ;
      while ( iTokenCount <= iIndex )
      {
        iTokenCount++ ;
        if ( iPos > 0 )
        {
          iPrevSepPos = iPos ;
          NextToken = FullItem.Tokenize( pInternalSeparators , iPos ) ;
        }
        else
          break ; // was last token
      }

      if ( iPrevSepPos == 0 ) // first item in full string
      {
        if ( iPos < 0 ) // the last item was first item (only one exists)
          *this = pValue;
        else
        {
          Delete( 0 , iPos ) ;
          Insert( 0 , pValue ) ;
        }
      }
      else
      {
        Delete( iPrevSepPos + 1 , iPos - iPrevSepPos - 1 ) ;
        Insert( iPrevSepPos + 1 , pValue ) ;
      }
      return true ;
    }
    else
      return FXPropertyKit::WriteString( pName , pValue , bToRegularlize ) ;
  }
  bool GetString( LPCTSTR pName , FXString& sVal , FXString& sDefault )
  {
    if ( !FXPropertyKit::GetString( pName , sVal ) )
      return FXPropertyKit::WriteString( pName , sVal = sDefault ) ;
    return true ;
  }
  bool Get3d( LPCTSTR pName , void * dValArray , TCHAR cType = 'f' )
  {
    return GetArray( *this , pName , cType , 3 , dValArray ) ;
  }
  bool Write3d( LPCTSTR pName , void * dValArray , TCHAR cType = 'f' , LPCTSTR pFormat = NULL )
  {
    if ( !pFormat )
    return WriteArray( *this , pName , cType , 3 , dValArray ) ;
    else
      return  WriteArray( *this , pName , cType , pFormat , 3 , dValArray ) ;
  }

  inline bool GetCmplx( const FXPropertyKit& pk ,
    LPCTSTR PropName , cmplx& cResult )
  {
    cmplx cRead ;
    if ( GetArray( pk , PropName , _T( 'f' ) , 2 , (double*) &cRead ) == 2 )
    {
      cResult = cRead ;
      return true ;
    };
    return false ;
  }
  inline void WriteCmplx( FXPropertyKit& pk ,
    LPCTSTR PropName , cmplx& cVal , LPCTSTR pFormat = "%.2f" )
  {
    WriteArray( pk , PropName , _T( 'f' ) , pFormat , 2 , (double*) &cVal ) ;
  }
  bool GetFromFile( LPCTSTR pFileName , bool bAppend = false )
  {
    FILE * fr = NULL ;
    errno_t err = _tfopen_s( &fr , pFileName , _T( "rb" ) ) ;
    if ( err == 0 )
    {
      FXSIZE iSize = FxGetFileLength( fr ) ;
      FXSIZE iFullTextLen = iSize + ((bAppend) ? (int) GetLength() : 0) ;
      LPTSTR pBuf = GetBuffer( iFullTextLen + 1 ) ;
      LPBYTE pByteBuf = (LPBYTE) pBuf ;
      FXSIZE iNRead = fread( (LPVOID) (pByteBuf + ((bAppend) ? iSize : 0)) ,
        1 , iSize , fr ) ;
      ASSERT( iNRead == iSize ) ;
      pBuf[ iFullTextLen ] = 0 ; // string terminating zero
      fclose( fr ) ;
      ReleaseBuffer() ;
      return true ;
    }

    return false ;
  }
  bool WriteToFile( LPCTSTR pFileName , bool bRewrite = true )
  {
    FXString Path = FxExtractPath(pFileName);
    FILE * fw = NULL ;
    if ( !FxVerifyCreateDirectory(Path) )
    {
      return false ;
    }
    errno_t err = _tfopen_s( &fw , pFileName , (bRewrite) ? _T( "wb" ) : _T( "ba+" ) ) ;
    if ( err == 0 )
    {
      if ( !bRewrite )
        int res = fseek( fw , 0 , SEEK_END ) ;
      LPCTSTR pString = GetBuffer();
      FXSIZE iNWritten = fwrite( pString , 1 , GetLength() + 1 , fw ) ;
      ReleaseBuffer();
      ASSERT( iNWritten == GetLength() + 1 ) ;
      fclose( fw ) ;
      return true ;
    }

    return false ;
  }
};



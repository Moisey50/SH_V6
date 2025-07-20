// PropKit.cpp: implementation of the FXPropertyKit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <fxfc\fxfc.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

static LPCTSTR pDelimiters = _T( "; (\t{[<)}]>\r\n" ) ;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FXPropertyKit::FXPropertyKit( /*bool bRegularize*/ )
{
  /*m_bRegularize = bRegularize*/ ;
}

FXPropertyKit::FXPropertyKit( const FXString& stringSrc/* , bool bRegularize*/ )
{
  //m_bRegularize = bRegularize ;
  FXAutolock lock( m_Lock );
  FXString::operator=( stringSrc );
}

FXPropertyKit::FXPropertyKit( FXPropertyKit& stringSrc /*, bool bRegularize*/ )
{
  //m_bRegularize = bRegularize ;
  FXAutolock lockd( m_Lock ) , locks( stringSrc.m_Lock );
  FXString::operator=( LPCTSTR( stringSrc ) );
}

/// private :

FXSIZE  FXPropertyKit::FindKey( LPCTSTR lpszEntry ) const
{
  FXSIZE pos = FXString::Find( lpszEntry );
  while ( pos >= 0 )
  {
    if ( pos <= 0 )
      return pos;
    if ( _tcschr( pDelimiters , GetAt( pos - 1 ) ) )
      return pos;
    pos = FXString::Find( lpszEntry , pos + 1 );
  } ;
  
  return -1;
}

bool FXPropertyKit::KeyExist( LPCTSTR lpszEntry ) const
{
  FXString tS( lpszEntry ); tS += '=';
  FXSIZE pos = FXString::Find( tS );
  if ( pos < 0 )
    return false ;
  if ( pos == 0 )
    return true ;
  return ( _tcschr( pDelimiters , GetAt( pos - 1 ) ) != NULL ) ;
}

bool FXPropertyKit::DeleteKey( LPCTSTR lpszEntry )
{
  FXString tS( lpszEntry ); tS += '=';
  FXSIZE pos = FindKey( tS );
  if ( pos >= 0 )
  {
    FXSIZE epos = FXString::Find( ';' , pos );
    if ( epos >= 0 )
    {
      FXString::Delete( pos , epos - pos + 1 );
      return true;
    }
  }
  return false;
}

// public:

bool FXPropertyKit::EnumKeys( FXStringArray& Keys , FXStringArray& Values )
{
  FXAutolock lock( m_Lock );
  FXString copy = LPCTSTR( *this );
  while ( !copy.IsEmpty() )
  {
    FXSIZE pos = copy.Find( '=' );
    if ( pos <= 0 )
      return false;
    FXString key = copy.Left( pos );
    copy = copy.Mid( pos + 1 );
    pos = copy.Find( ';' );
    if ( pos < 0 )
      return false;
    FXString value = copy.Left( pos );
    copy = copy.Mid( pos + 1 );
    Keys.Add( key );
    Values.Add( value );
  }
  return true;
}


FXPropertyKit& FXPropertyKit::operator = ( LPCTSTR stringSrc )
{
  FXAutolock lock( m_Lock );
  FXString::operator=( stringSrc );
  return *this;
}

FXPropertyKit& FXPropertyKit::operator = ( FXString& stringSrc )
{
  FXAutolock lock( m_Lock );
  FXString::operator=( LPCTSTR( stringSrc ) );
  return *this;
}

FXPropertyKit& FXPropertyKit::operator = ( FXPropertyKit& pk )
{
  FXAutolock lockd( m_Lock ) , locks( pk.m_Lock );
  FXString::operator=( LPCTSTR( pk ) );
  return *this;
}

bool FXPropertyKit::WriteInt( LPCTSTR lpszEntry , int nValue )
{
  FXAutolock lock( m_Lock );
  if (KeyExist( lpszEntry ))
    DeleteKey( lpszEntry );
  FXString tmpS;
  tmpS.Format( _T( "%s=%d;" ) , lpszEntry , nValue );
  *this += tmpS;
  return true;
}

bool FXPropertyKit::GetInt( LPCTSTR lpszEntry , int& nValue ) const
{
  FXAutolock lock( ( ( FXPropertyKit* ) this )->m_Lock );
  FXString tS( lpszEntry ); tS += '=';
  FXSIZE pos = FindKey( tS );
  if (pos >= 0)
  {
    pos += tS.GetLength();
    FXString a = this->Mid( pos );
    nValue = _ttoi( a );
    return true;
  }
  return false;
}
bool FXPropertyKit::WriteInt64( LPCTSTR lpszEntry , __int64 nValue )
{
  FXAutolock lock( m_Lock );
  if (KeyExist( lpszEntry ))
    DeleteKey( lpszEntry );
  FXString tmpS;
  tmpS.Format( _T( "%s=%lld;" ) , lpszEntry , nValue );
  *this += tmpS;
  return true;
}

bool FXPropertyKit::GetInt64( LPCTSTR lpszEntry , __int64& nValue ) const
{
  FXAutolock lock( ( ( FXPropertyKit* ) this )->m_Lock );
  FXString tS( lpszEntry ); tS += '=';
  FXSIZE pos = FindKey( tS );
  if (pos >= 0)
  {
    pos += tS.GetLength();
    FXString a = this->Mid( pos );
    nValue = _ttoi64( a );
    return true;
  }
  return false;
}

bool FXPropertyKit::WriteLong( LPCTSTR lpszEntry , long nValue )
{
  FXAutolock lock( m_Lock );
  if ( KeyExist( lpszEntry ) )
    DeleteKey( lpszEntry );
  FXString tmpS;
  tmpS.Format( _T( "%s=%d;" ) , lpszEntry , nValue );
  *this += tmpS;
  return true;
}

bool FXPropertyKit::GetLong( LPCTSTR lpszEntry , long& nValue ) const
{
  FXAutolock lock( ( ( FXPropertyKit* )this )->m_Lock );
  FXString tS( lpszEntry ); tS += '=';
  FXSIZE pos = FindKey( tS );
  if ( pos >= 0 )
  {
    pos += tS.GetLength();
    FXString a = this->Mid( pos );
    nValue = _ttoi( a );
    return true;
  }
  return false;
}


bool FXPropertyKit::WriteDouble( LPCTSTR lpszEntry , double Value , LPCTSTR pFormat )
{
  FXAutolock lock( m_Lock );
  if ( KeyExist( lpszEntry ) )
    DeleteKey( lpszEntry );
  FXString tmpS;
  FXString Format( "%s=" ) ;
  Format += ( pFormat )? pFormat : "%g" ;
  tmpS.Format( Format , lpszEntry , Value );
  *this += tmpS + ';' ;
  return true;
}

bool FXPropertyKit::GetDouble( LPCTSTR lpszEntry , double& Value ) const
{
  FXAutolock lock( ( ( FXPropertyKit* )this )->m_Lock );
  FXString tS( lpszEntry ); tS += '=';
  FXSIZE pos = FindKey( tS );
  if ( pos >= 0 )
  {
    pos += tS.GetLength();
    FXString a = this->Mid( pos );
    Value = atof( a );
    return true;
  }
  return false;
}

bool FXPropertyKit::WriteBool( LPCTSTR lpszEntry , bool bValue )
{
  FXAutolock lock( m_Lock );
  if ( KeyExist( lpszEntry ) )
    DeleteKey( lpszEntry );
  FXString tmpS;
  tmpS.Format( _T( "%s=%s;" ) , lpszEntry , bValue ? _T( "true" ) : _T( "false" ) );
  *this += tmpS;
  return true;
}

bool FXPropertyKit::GetBool( LPCTSTR lpszEntry , bool& bValue ) const
{
  FXAutolock lock( ( ( FXPropertyKit* )this )->m_Lock );
  FXString tS( lpszEntry ); tS += '=';
  FXSIZE pos = FindKey( tS );
  if ( pos >= 0 )
  {
    pos += tS.GetLength();
    FXString a = this->Mid( pos );
    pos = a.Find( ';' );
    if ( pos >= 0 )
    {
      a = a.Left( pos );
      if ( a == _T( "true" ) )
        bValue = true;
      else if ( a == _T( "false" ) )
        bValue = false;
      else
      {
        return false;
      }
      return true;
    }
    return false;
  }
  return false;
}

bool FXPropertyKit::WriteBool( LPCTSTR lpszEntry , BOOL bValue )
{
  FXAutolock lock( m_Lock );
  if ( KeyExist( lpszEntry ) )
    DeleteKey( lpszEntry );
  FXString tmpS;
  tmpS.Format( _T( "%s=%s;" ) , lpszEntry , bValue ? _T( "true" ) : _T( "false" ) );
  *this += tmpS;
  return true;
}

bool FXPropertyKit::GetBool( LPCTSTR lpszEntry , BOOL& bValue ) const
{
  FXAutolock lock( ( ( FXPropertyKit* )this )->m_Lock );
  FXString tS( lpszEntry ); tS += '=';
  FXSIZE pos = FindKey( tS );
  if ( pos >= 0 )
  {
    pos += tS.GetLength();
    FXString a = this->Mid( pos );
    pos = a.Find( ';' );
    if ( pos >= 0 )
    {
      a = a.Left( pos );
      if ( a == _T( "true" ) )
        bValue = true;
      else if ( a == _T( "false" ) )
        bValue = false;
      else
      {
        return false;
      }
      return true;
    }
    return false;
  }
  return false;
}

bool FXPropertyKit::WriteBinary( LPCTSTR lpszEntry , LPBYTE bValue , FXSIZE nBytes )
{
  FXAutolock lock( m_Lock );
  if ( KeyExist( lpszEntry ) )
    DeleteKey( lpszEntry );
  FXString value;
  TCHAR	tmpB[ 4 ]; tmpB[ 2 ] = 0;
  for ( FXSIZE i = 0; i < nBytes; i++ )
  {
    errno_t err = _itot_s( bValue[ i ] , tmpB , 3 , 16 );
    if ( tmpB[ 1 ] == 0 )
    {
      tmpB[ 1 ] = tmpB[ 0 ];
      tmpB[ 0 ] = _T( '0' );
      tmpB[ 2 ] = 0;
    }
    value += tmpB;
  }
  *this += FXString( lpszEntry ) + '=' + value + ';';
  return true;
}

bool FXPropertyKit::GetBinary( LPCTSTR lpszEntry , LPBYTE & bValue , FXSIZE& nBytes ) const
{
  nBytes = 0; bValue = NULL;
  FXAutolock lock( ( ( FXPropertyKit* )this )->m_Lock );
  FXString tS( lpszEntry ); tS += '=';
  FXSIZE pos = FindKey( tS );
  if ( pos >= 0 )
  {
    pos += tS.GetLength();
    FXString a = this->Mid( pos );
    pos = a.Find( ';' );
    if ( pos >= 0 )
    {
      a = a.Left( pos );
      ASSERT( a.GetLength() % 2 == 0 );
      nBytes = a.GetLength() / 2;
      bValue = ( LPBYTE ) malloc( nBytes );
      char tmpB[ 3 ]; tmpB[ 2 ] = 0;
      char *endp;
      for ( FXSIZE i = 0; i < nBytes; i++ )
      {
        tmpB[ 0 ] = a[ i * 2 ];
        tmpB[ 1 ] = a[ i * 2 + 1 ];
        BYTE a = ( BYTE ) strtol( tmpB , &endp , 16 );
        bValue[ i ] = a;
      }
      return true;
    }
  }
  return false;
}

bool FXPropertyKit::WriteString( LPCTSTR lpszEntry , LPCTSTR sValue, bool bToRegularlize)
{
  FXAutolock lock( m_Lock );
  if ( KeyExist( lpszEntry ) )
    DeleteKey( lpszEntry );
  //20181224_yuris FXString scopy = /*( m_bRegularize ) ? */::FxRegularize( sValue ) /*: sValue */;
  FXString scopy = bToRegularlize ? ::FxRegularize( sValue ) : FXString(sValue) ;
  FXString tmpS;
  tmpS.Format( _T( "%s=%s;" ) , lpszEntry , scopy );
  *this += tmpS;
  return true;
}

bool FXPropertyKit::GetString( LPCTSTR lpszEntry , FXString& sValue, bool bToUnregularlize ) const
{
  FXAutolock lock( ( ( FXPropertyKit* )this )->m_Lock );
  FXString tS( lpszEntry ); tS += '=';
  FXSIZE pos = FindKey( tS );
  if ( pos >= 0 )
  {
    pos += tS.GetLength();
    FXString a = this->Mid( pos );
    pos = a.Find( ';' );
    if ( pos >= 0 )
    {
      a = a.Left( pos );
    }
    if ( a.GetLength() == 0 )
    {
      sValue.Empty();
      return true;
    }
    
    //20181224_yuris
    sValue = bToUnregularlize ? ::FxUnregularize( a ) : a;

    return true;
  }
  return false;
}

bool FXPropertyKit::WritePtr( LPCTSTR lpszEntry , void * pPtr )
{
  FXAutolock lock( m_Lock );
  if ( KeyExist( lpszEntry ) )
    DeleteKey( lpszEntry );
  FXString tmpS;
  tmpS.Format( _T( "%s=%p;" ) , lpszEntry , pPtr );
  *this += tmpS;
  return true;
}

#define _CRT_SECURE_NO_WARNINGS
bool FXPropertyKit::GetPtr( LPCTSTR lpszEntry , void * & pValue ) const
{
  FXAutolock lock( ( ( FXPropertyKit* ) this )->m_Lock );
  FXString tS( lpszEntry ); 
  tS += '=';
  FXSIZE pos = FindKey( tS );
  size_t uLen = strlen( lpszEntry ) ;
  if ( pos >= 0 )
  {
    if ( sscanf_s( ( ( LPCTSTR ) ( this ) ) + pos + tS.GetLength() , "%p" , &pValue  ))
      return true ;
  }
  return false;
}

#undef _CRT_SECURE_NO_WARNINGS
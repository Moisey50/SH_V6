// Registry.cpp: implementation of the CRegistry class.
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "Registry.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRegistry::CRegistry()
{
  InitKey( _T("FileX\\Untitled") ) ; 
  m_hkRootKeyForInstance = HKEY_CURRENT_USER ;
}


CRegistry::CRegistry(LPCTSTR szRootKeyName , HKEY hkRootKey)
{
  InitKey( szRootKeyName ) ;
  m_hkRootKeyForInstance = hkRootKey ;
}

CRegistry::~CRegistry()
{

}

HKEY CRegistry::FormKeyName( TCHAR * pBuf , int iBufLenElements , TCHAR * pName )
{
  if ( _tcscpy_s( pBuf , iBufLenElements , _T("Software\\") ) )
    return NULL ;
  if ( _tcscat_s( pBuf , iBufLenElements , m_KeyPrefix ) )
    return NULL;
  if ( _tcscat_s( pBuf , iBufLenElements , _T("\\") ) )
    return NULL;
  if ( _tcscat_s( pBuf , iBufLenElements , pName ) )
    return NULL;
  HKEY Key ;
  int iRes = RegOpenKeyEx( m_hkRootKeyForInstance ,
    pBuf , 0 , KEY_ALL_ACCESS ,
    &Key ) ;
  if ( iRes != ERROR_SUCCESS )
  {                                  
    DWORD dwDisposition = 0 ;  
    TRACE( "\nCan't open Registry key %s" , pBuf)  ;
    iRes = RegCreateKeyEx( m_hkRootKeyForInstance , pBuf , 0 , NULL ,
      REG_OPTION_NON_VOLATILE , KEY_ALL_ACCESS , NULL , &Key , 
      &dwDisposition ) ;
    if ( iRes == ERROR_SUCCESS )
    {
      m_KeyCreated = true ;
      return Key ;
    }
  }
  m_KeyCreated = false ;
  return ( iRes == ERROR_SUCCESS ) ? Key : NULL ;
}

int CRegistry::GetRegiInt(
  LPCTSTR Key, LPCTSTR ValueName, int iDefault)
{
  DWORD iResult = 0xcdcdcdcd ;
  HKEY FileCompanyKey ;
  TCHAR Buf[2000] ;
  if ( (FileCompanyKey = FormKeyName( Buf , 2000 , (TCHAR*) Key)) == NULL )
    return iDefault ;

  DWORD dwType = REG_DWORD ;
  DWORD Len = 4 ;
  int iRes ;
  if ( m_KeyCreated )
  {
    iRes = RegSetValueEx( 
      FileCompanyKey ,
      ValueName , 
      0 , 
      REG_DWORD ,
      ( BYTE * )&iDefault ,
      4 ) ;
    iResult = iDefault ;
  }
  else
  {
    iRes = RegQueryValueEx( 
    FileCompanyKey ,
    ValueName , 
    0 , 
    &dwType ,
    ( unsigned char * )&iResult ,
    &Len ) ;
  if ( iRes != ERROR_SUCCESS  ||  dwType != REG_DWORD )
  {
    if ( dwType != REG_DWORD )
      RegDeleteKey( FileCompanyKey , ValueName ) ;

    iRes = RegSetValueEx( 
      FileCompanyKey ,
      ValueName , 
      0 , 
      REG_DWORD ,
      ( BYTE * )&iDefault ,
      4 ) ;
    if ( iRes != ERROR_SUCCESS )
    {
      TRACE( "\nCan't write default value %s into Registry %s" , ValueName , Buf) ;
    }
    
    iResult = iDefault ;
  }
  }

  RegCloseKey( FileCompanyKey ) ;

  return iResult ;

}

void CRegistry::InitKey(LPCTSTR szNewKeyPrefix , HKEY hkRootKey )
{
  _tcscpy( m_KeyPrefix , szNewKeyPrefix ) ;
  m_hkRootKeyForInstance = hkRootKey ;
}

void CRegistry::WriteRegiInt(
  LPCTSTR Key, LPCTSTR ValueName, int Value)
{
  HKEY FileCompanyKey ;
  TCHAR Buf[2000] ;
  if ( (FileCompanyKey = FormKeyName( Buf , 2000 , (TCHAR*) Key)) == NULL )
    return ;

  int iRes = RegSetValueEx( 
    FileCompanyKey ,
    ValueName , 
    0 , 
    REG_DWORD ,
    ( BYTE * )&Value ,
    4 ) ;

  if ( iRes != ERROR_SUCCESS )
  {
    TRACE( "\nCan't write %s into Registry %s" , ValueName , Buf ) ;
  }
  RegCloseKey( FileCompanyKey ) ;
}


TCHAR *  CRegistry::GetRegiString(
    LPCTSTR Key, LPCTSTR ValueName, LPCTSTR  Default) 
{
  HKEY FileCompanyKey ;
  TCHAR Buf[2000] ;
  _tcscpy( m_TmpBuffer , Default ) ;
  if ( (FileCompanyKey = FormKeyName( Buf , 2000 , (TCHAR*) Key)) == NULL )
  {
    return  (TCHAR*)&m_TmpBuffer ;
  }
  DWORD dwType = REG_SZ ;
  DWORD Len = sizeof( m_TmpBuffer ) ;

  if ( m_KeyCreated )
  {
    int     iRes = RegSetValueEx( 
      FileCompanyKey ,
      ValueName , 
      0 , 
      REG_SZ ,
      ( BYTE * )Default ,
      (DWORD) (_tcslen( Default ) + 1) * sizeof(TCHAR) ) ;
  }
  else
  {
  int iRes = RegQueryValueEx( 
    FileCompanyKey ,
    ValueName , 
    0 , 
    &dwType ,
    ( BYTE * )m_TmpBuffer ,
    &Len ) ;

  if ( iRes == ERROR_MORE_DATA )
  {
    BYTE * buf1 = (BYTE*)malloc( Len + 2 );
		
		iRes = RegQueryValueEx( 
      FileCompanyKey ,
      ValueName , 
      0 , 
      &dwType ,
      buf1 ,
      &Len ) ;

    memcpy( m_TmpBuffer , buf1 , sizeof(m_TmpBuffer) - 1 ) ;
		m_TmpBuffer[ 2047 ] = 0 ;

    free( buf1 ) ;
  }
  else if ( (iRes != ERROR_SUCCESS  ||  dwType != REG_SZ )
      &&  Default )
  {
      if ( iRes == ERROR_SUCCESS  &&  dwType != REG_SZ )
    {
      //TRACE( "Value of %s in REGISTRY is not string" , ValueName ) ;
      RegDeleteKey( FileCompanyKey , ValueName ) ;
    }

    iRes = RegSetValueEx( 
      FileCompanyKey ,
      ValueName , 
      0 , 
      REG_SZ ,
      ( BYTE * )Default ,
      (DWORD) (_tcslen( Default ) + 1) * sizeof(TCHAR) ) ;

    if ( iRes != ERROR_SUCCESS )
    {
      TRACE( "\nCan't write default value %s into Registry %s" , ValueName , Buf) ;
    }
    
    _tcscpy( m_TmpBuffer , Default ) ;
  }
	else
		m_TmpBuffer[ Len ] = 0 ;
  }
      
  RegCloseKey( FileCompanyKey ) ;

  return m_TmpBuffer ;
}

double  CRegistry::GetRegiDouble(
      LPCTSTR Key, LPCTSTR ValueName, double dDefault) 
{
  TCHAR szDef[ 100 ] ;
  double dResult = 1e30 ;
  _stprintf( szDef , _T("%g") , dDefault ) ;

  _stscanf( GetRegiString( Key , ValueName , szDef ) , _T("%lg") , &dResult ) ;

  return dResult ;
};

__int64  CRegistry::GetRegiInt64(
   LPCTSTR Key, LPCTSTR ValueName, __int64 i64Default) 
{
  TCHAR szDef[ 100 ] ;
  __int64 i64Result = 111111111111 ;
  _stprintf( szDef , _T("%I64d") , i64Default ) ;

  _stscanf( GetRegiString( Key , ValueName , szDef ) , _T("%I64d") , &i64Result ) ;

  return i64Result ;
};

void    CRegistry::WriteRegiString(
  LPCTSTR Key, LPCTSTR ValueName, LPCTSTR  Value) 
{
  HKEY FileCompanyKey ;
  TCHAR Buf[2000] ;
  if ( (FileCompanyKey = FormKeyName( Buf , 2000 , (TCHAR*) Key)) == NULL )
    return ;

  int iRes = RegSetValueEx( 
      FileCompanyKey ,
      ValueName , 
      0 , 
      REG_SZ ,
      ( const BYTE * )Value ,
       (DWORD) (_tcslen( Value ) + 1) * sizeof(TCHAR) ) ;

  if ( iRes != ERROR_SUCCESS )
  {
    TRACE( "\nCan't write value %s into Registry %s" , ValueName , Buf) ;
  }
  RegCloseKey( FileCompanyKey ) ;
};

void    CRegistry::WriteRegiDouble(
  LPCTSTR Key, LPCTSTR ValueName, double dValue )
{
  TCHAR szDef[ 100 ] ;
  _stprintf( szDef , _T("%g") , dValue ) ;

  WriteRegiString( Key , ValueName , szDef ) ;
};

void    CRegistry::WriteRegiInt64(
                                   LPCTSTR Key, LPCTSTR ValueName, __int64 i64Val )
{
  TCHAR szDef[ 100 ] ;
  _stprintf( szDef , _T("%I64d") , i64Val ) ;

  WriteRegiString( Key , ValueName , szDef ) ;
};

void    CRegistry::AddToMultiString(
  LPCTSTR Key, LPCTSTR ValueName, LPCTSTR  Value )
{
  HKEY FileCompanyKey ;
  DWORD dwType ;
  BYTE buf[ 2048 ] ;
  DWORD iLen = 2048 ;
  TCHAR Buf[2000] ;
  if ( (FileCompanyKey = FormKeyName( Buf , 2000 , (TCHAR*) Key)) == NULL )
    return ;

	int iRes = RegQueryValueEx( 
    FileCompanyKey ,
    ValueName , 
    0 , 
    &dwType ,
    buf ,
    &iLen ) ;

  if ( iRes != ERROR_SUCCESS )
  {
    iRes = RegSetValueEx( 
      FileCompanyKey ,
      ValueName , 
      0 , 
      REG_MULTI_SZ ,
      ( const BYTE * )Value ,
       (DWORD) (_tcslen( Value ) + 1) * sizeof(TCHAR) ) ;
  }
  else if ( dwType == REG_MULTI_SZ )
  {
    int iPtr = 0 ;
    int iLastLen ;
    int ValueLen = (int) _tcslen( Value ) ;
    TCHAR Upper[200] ;
    _tcscpy( Upper , Value ) ;
    _tcsupr( Upper ) ;
    while ( iLastLen = (int) _tcslen( (LPCTSTR)(buf + iPtr) ) )
    {
      if ( iLastLen == ValueLen )
      {
        TCHAR Up1[200] ;
        _tcscpy( Up1 , (LPCTSTR)(buf + iPtr) ) ;
        _tcsupr( Up1 ) ;

        if ( !_tcscmp( Up1 , Upper ) ) // Name Already exists
        {
          iPtr = -1 ;
          break ;
        }
      }
      iPtr += iLastLen + 1 ; // plus terminating zero
    }
    if ( iPtr >= 0 )
    {
      _tcscpy( (TCHAR *)(buf + iPtr) , Value ) ;
      buf[ iPtr + ValueLen + 1 ] = 0 ;
      iRes = RegSetValueEx( 
        FileCompanyKey ,
        ValueName , 
        0 , 
        REG_MULTI_SZ ,
        ( const BYTE * )buf ,
        (iPtr + ValueLen + 2) * sizeof(TCHAR) ) ;
    }

  }

  RegCloseKey( FileCompanyKey ) ;
};


  // Gets string from registry and parse into array of integers
  // returns amount of converted numbers
  // 
int CRegistry::GetRegiIntSerie( 
  LPCTSTR  Key , LPCTSTR  ValueName , 
  int * iResultArray, 
  int iArrayLength // if not equal to returned value - not enogh space in array
                           // In this case this variable will be a length of necessary
                           // array
  )
{
  TCHAR * p = GetRegiString( Key , ValueName , _T("") ) ;

  if ( !p  ||  !(*p) )
    return 0 ;

  TCHAR * tok = _tcstok( ( TCHAR * )p , _T(" ,") );

  int iConverted = 0 ;

  while ( tok )
  {
    
    DWORD dwNewNumber = 0xffffffff ;
    int iRes ;
    if ( (tok[0] == _T('0'))  &&  (_totlower(tok[1]) == _T('x')) )
      iRes = _stscanf( tok + 2 , _T("%x") , &dwNewNumber ) ;
    else 
      iRes = _stscanf( tok , _T("%d") , &dwNewNumber ) ;

    if ( iRes )
    {
      if ( iConverted < iArrayLength )
        iResultArray[ iConverted ] = dwNewNumber ;

      iConverted++ ;

      tok = _tcstok( NULL , _T(" ,") ) ;
    }
    else
      break ;
  }

  return iConverted;
};  


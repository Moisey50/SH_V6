// Registry.h: interface for the CRegistry class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGISTRY_H__FD331F2F_4D4A_46EF_A7C8_06289C1B7E3C__INCLUDED_)
#define AFX_REGISTRY_H__FD331F2F_4D4A_46EF_A7C8_06289C1B7E3C__INCLUDED_

#pragma warning(disable:4996)

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CRegistry  
{
public:
  CRegistry( LPCTSTR szRootKeyName , HKEY hkRootKey = HKEY_CURRENT_USER );
  CRegistry();
  virtual ~CRegistry();
  void    InitKey( LPCTSTR szNewKeyPrefix , HKEY hkRootKey = HKEY_CURRENT_USER );
  HKEY FormKeyName( TCHAR * pBuf , int iBufLenElements , TCHAR * pName ) ;

  int     GetRegiInt( 
    LPCTSTR Key , LPCTSTR ValueName , int iDefault);
  __int64 GetRegiInt64(
    LPCTSTR Key , LPCTSTR ValueName , __int64 iDefault);
  TCHAR *  GetRegiString(
    LPCTSTR Key, LPCTSTR ValueName, LPCTSTR  Default) ;
  double  GetRegiDouble(
    LPCTSTR Key, LPCTSTR ValueName, double dDefault) ;
  int     GetRegiIntSerie( 
    LPCTSTR  Key , LPCTSTR  ValueName , 
    int * iResultArray, int iArrayLength );  

  void    WriteRegiInt(
    LPCTSTR  Key , LPCTSTR  ValueName , int Value ) ;
  void    WriteRegiInt64(
    LPCTSTR  Key , LPCTSTR  ValueName , __int64 Value ) ;
  void    WriteRegiString(
    LPCTSTR Key, LPCTSTR ValueName, LPCTSTR  Value) ;
  void    WriteRegiDouble(
    LPCTSTR Key, LPCTSTR ValueName, double dValue ) ;
  void    AddToMultiString(
    LPCTSTR Key, LPCTSTR ValueName, LPCTSTR  Value ) ;

private:
  bool     m_KeyCreated ;
  TCHAR    m_KeyPrefix[ 255 ] ;
  TCHAR    m_TmpBuffer[ 2048 ];
  HKEY     m_hkRootKeyForInstance ;
};

#endif // !defined(AFX_REGISTRY_H__FD331F2F_4D4A_46EF_A7C8_06289C1B7E3C__INCLUDED_)

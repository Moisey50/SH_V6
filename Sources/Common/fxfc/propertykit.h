// propertykit.h: interface for the FXPropertyKit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROPERTYKIT_H__BC444EBF_490D_406D_BB4F_0D36867151B3__INCLUDED_)
#define AFX_PROPERTYKIT_H__BC444EBF_490D_406D_BB4F_0D36867151B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define SEPARATORS " \t\n\r"
#define BRAKETS    "()"
#define TERMINATORS ",;()"
#define NEWLINE     "\r\n"

class FXFC_EXPORT FXPropertyKit : public FXString
{
private:
public:
  FXLockObject m_Lock;
  FXPropertyKit();
  FXPropertyKit( const FXString& stringSrc );
  FXPropertyKit( FXPropertyKit& stringSrc );
  FXPropertyKit& operator = ( LPCTSTR stringSrc );
  FXPropertyKit& operator = ( FXString& stringSrc );
  FXPropertyKit& operator = ( FXPropertyKit& pk );
  bool WriteInt( LPCTSTR lpszEntry , int nValue );
  bool GetInt( LPCTSTR lpszEntry , int& nValue ) const; /// No default value!!!
  bool WriteInt64( LPCTSTR lpszEntry , __int64 nValue );
  bool GetInt64( LPCTSTR lpszEntry , __int64& nValue ) const; /// No default value!!!
  bool WriteLong( LPCTSTR lpszEntry , long nValue );
  bool GetLong( LPCTSTR lpszEntry , long& nValue ) const; /// No default value!!!
  bool WriteDouble( LPCTSTR lpszEntry , double Value , LPCTSTR pFormat = NULL );
  bool GetDouble( LPCTSTR lpszEntry , double& Value ) const;
  bool WriteBool( LPCTSTR lpszEntry , bool bValue );
  bool GetBool( LPCTSTR lpszEntry , bool& bValue ) const;
  bool WriteBool( LPCTSTR lpszEntry , BOOL bValue );
  bool GetBool( LPCTSTR lpszEntry , BOOL& bValue ) const;
  bool WriteBinary( LPCTSTR lpszEntry , LPBYTE bValue , FXSIZE nBytes );
  bool GetBinary( LPCTSTR lpszEntry , LPBYTE & bValue , FXSIZE& nBytes ) const;
  bool WriteString( LPCTSTR lpszEntry , LPCTSTR sValue, bool bToRegularlize=true);
  bool GetString( LPCTSTR lpszEntry , FXString& sValue, bool bToUnregularlize=true) const;
  bool WritePtr( LPCTSTR lpszEntry , void * pPtr ) ;
  bool GetPtr( LPCTSTR lpszEntry , void* & Ptr ) const ;

  bool EnumKeys( FXStringArray& Keys , FXStringArray& Values );
  bool DeleteKey( LPCTSTR lpszEntry );
  bool KeyExist( LPCTSTR lpszEntry ) const;
private:
  FXSIZE  FindKey( LPCTSTR lpszEntry ) const;
};

#endif // !defined(AFX_PROPERTYKIT_H__BC444EBF_490D_406D_BB4F_0D36867151B3__INCLUDED_)

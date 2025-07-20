#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "GadgetLicense.cpp"
#include <gadgets/gadbase.h>
#include <fxfc/FXRegistry.h>
#include <helpers/FXParser2.h>
#include <time.h>

bool Tvdb400_CheckSetLicense( LPCTSTR pAppName , LPCTSTR pDirName )
{
  FXString RegName( "TheFileX\\" ) ;
  RegName += pAppName ;
  FXRegistry Reg( RegName );
  BYTE Buf[ MAX_PATH ] ; // in reality, max computer name length is 15 bytes
  DWORD dwSize = MAX_PATH - 1 ;
  FXString License = Reg.GetRegiString( pDirName ,
    "SystemID" , "No ID" ) ;
  if ( License.GetLength() == 8 )  // initial registration
  {
    FXString Today = GetTimeStamp().Left( 8 ) ; // Year (4 digits), month (2) , day(2)
    if ( License == Today ) // condition for registration
    {
      GetComputerName( ( LPSTR ) Buf , &dwSize ) ;
      DWORD dwNameLen = dwSize ;
      double dTime = GetHRTickCount() ;
      UINT iSeed = ( UINT ) ( ( *( ( __int64 * ) ( &dTime ) ) >> 16 ) & 0xffffffff ) ;
      srand( iSeed ) ;
      for ( UINT i = dwNameLen ; i < dwNameLen * 2 ; i++ )
        Buf[ i ] = ( UCHAR ) rand() ;

      for ( UINT i = dwNameLen ; i < dwNameLen * 2 ; i++ )
      {
        Buf[ i - dwNameLen ] ^= Buf[ i ] ^ Buf[ dwNameLen + ( ( i + 5 ) % dwNameLen ) ] ;
      }
      FXString Hex ;
      BYTE CRC = ConvBytesToHexSaveCRC( Buf , dwNameLen * 2 , Hex ) ;
      Reg.WriteRegiString( "ProcessingParams" , "SystemID" , Hex ) ;
      return true ;
    }
  }
  else // not first licensing, Check computer name
  {
    BYTE CRC = ConvHexToBytes( License , ( DWORD ) License.GetLength() , Buf , dwSize ) ;
    int iNameLen = ( dwSize - 1 ) / 2 ; // minus CRC
    for ( int i = 0 ; i < iNameLen ; i++ )
    {
      Buf[ i ] ^= Buf[ i + iNameLen ] ^ Buf[ ( ( i + 5 ) % iNameLen ) + iNameLen ] ;
    }
    Buf[ iNameLen ] = 0 ;
    FXString LicensedTo = ( char* ) Buf ;
    FXString ComputerName ;
    dwSize = sizeof( Buf ) - 1 ;

    GetComputerName( ( LPSTR ) Buf , &dwSize ) ;
    ComputerName = Buf ;
    return ( ComputerName == LicensedTo ) ;
  }
  return false ;
}


class LicenseData
{
public:
  LicenseData()
  {
    memset( this , 0 , sizeof( *this ) ) ;
    char Buf[ 64 ] ;
    DWORD dwSize = 64 ;
    GetComputerName( ( LPSTR ) Buf , &dwSize ) ;
    if ( dwSize > ( sizeof( m_ComputerName ) - 1 ) )
      dwSize = sizeof( m_ComputerName ) - 1 ;
    memcpy( m_ComputerName , Buf , dwSize ) ;
    m_ComputerName[ dwSize ] = 0 ;
  }

  DWORD m_dwShift ; // Offset in _Random Array
  BYTE  m_Random[ 256 ] ;
  DWORD m_dwCRC ;
  BYTE  m_ComputerName[ 32 ] ;
  BYTE  m_Data[ 128 ] ;
} ;

FXString CreateLicenseRecord( LPCTSTR pLicenseName , int iLicenseTimeDaysOrRemove ,
  LicenseData& Result )
{
  FXString AsHex ;
  if ( iLicenseTimeDaysOrRemove >= 0 )
  {
    __time64_t Now ;
    time( &Now ) ;
    struct tm NowS ;
    _gmtime64_s( &NowS , &Now ) ;

    double dTime = GetHRTickCount() ;
    UINT iSeed = ( UINT ) ( ( *( ( __int64 * ) ( &dTime ) ) >> 16 ) & 0xffffffff ) ;
    srand( iSeed ) ;
    for ( UINT i = 0 ; i < 256 ; i++ )
      Result.m_Random[ i ] = ( BYTE ) rand() ;

    Result.m_dwShift = rand() % 256 ;
    Result.m_dwCRC = 0 ;

    FXString LicenseText ;
    LicenseText.Format( "%s=%04d%02d%02d,%d;" , pLicenseName ,
      NowS.tm_year + 1900 , NowS.tm_mon + 1 , NowS.tm_mday , iLicenseTimeDaysOrRemove ) ;

    if ( LicenseText.GetLength() < sizeof( Result.m_Data ) - 1 )
    {
      strcpy_s( ( char* ) Result.m_Data , sizeof( Result.m_Data ) , ( LPCTSTR ) LicenseText ) ;
      // Computer Name and Data encoding
      BYTE * p = Result.m_ComputerName ;
      BYTE * pEnd = p + sizeof( Result.m_ComputerName ) + sizeof( Result.m_Data ) ;
      DWORD dwIndex = Result.m_dwShift ;
      while ( p < pEnd )
      {
        Result.m_dwCRC += *p ;
        *( p++ ) ^= Result.m_Random[ dwIndex++ ] ;
        if ( dwIndex >= 256 )
          dwIndex = 0 ;
      }
      Result.m_dwShift ^= *( ( DWORD* ) &Result.m_Random[ 252 ] ) ;
      Result.m_dwCRC ^= *( ( DWORD* ) &Result.m_Random[ 248 ] ) ;

      BYTE CRC = ConvBytesToHexSaveCRC( ( LPBYTE ) &Result , sizeof( Result ) , AsHex ) ;
    }
  }
  return AsHex ;
}

bool DecodeLicenseRecord( FXString& LicenseHexString ,
  FXString& LicenseName , int& iLicenseTimeDays ,
  __time64_t& InitDate , LicenseData& Result )
{
  DWORD Converted = sizeof( Result ) ;
  ConvHexToBytes( ( LPCTSTR ) LicenseHexString , ( int ) LicenseHexString.GetLength() ,
    ( LPBYTE ) &Result , Converted ) ;

  Result.m_dwShift ^= *( ( DWORD* ) &Result.m_Random[ 252 ] ) ;
  Result.m_dwCRC ^= *( ( DWORD* ) &Result.m_Random[ 248 ] ) ;

  BYTE * p = Result.m_ComputerName ;
  BYTE * pEnd = p + sizeof( Result.m_ComputerName ) + sizeof( Result.m_Data ) ;
  DWORD dwIndex = Result.m_dwShift ;
  DWORD dwCRC = 0 ;
  while ( p < pEnd )
  {
    *p ^= Result.m_Random[ dwIndex++ ] ;
    dwCRC += *( p++ ) ;

    if ( dwIndex >= 256 )
      dwIndex = 0 ;
  }

  if ( dwCRC != Result.m_dwCRC )
    return false ;

  FXString LicenseText( ( LPCTSTR ) Result.m_Data ) ;
  FXSIZE iEquPos = LicenseText.Find( '=' ) ;
  LicenseName = LicenseText.Left( iEquPos ) ;
  FXSIZE iCommaPos = LicenseText.Find( ',' ) ;
  iLicenseTimeDays = atoi( ( LPCTSTR ) LicenseText + iCommaPos + 1 ) ;
  FXString Date = LicenseText.Mid( iEquPos + 1 , iCommaPos - iEquPos - 1 ) ;
  tm LicenseDate ;
  memset( &LicenseDate , 0 , sizeof( LicenseDate ) ) ;

  sscanf( ( LPCSTR ) Date , "%4d%2d%2d" , &LicenseDate.tm_year ,
    &LicenseDate.tm_mon , &LicenseDate.tm_mday ) ;
  LicenseDate.tm_year -= 1900 ;
  LicenseDate.tm_mon-- ;
  InitDate = mktime( &LicenseDate ) ;

  return true ;
}

// iLicenseTimeDaysOrRemove: == 0 => infinite license
//                            > 0  => Number of days from licensing
//                            < 0  => Remove license

bool Tvdb400_AddRemoveLicense( LPCTSTR pGadgetName , int iLicenseTimeDaysOrRemove )
{
  FXString RegName( "TheFileX\\" ) ;
  FXRegistry Reg( RegName );

  __time64_t Now ;
  time( &Now ) ;
  struct tm NowS ;
  _gmtime64_s( &NowS , &Now ) ;

  FXString FolderOfLicenses( RegName + "Licensing" ) ;
  FXStringArray Folders ;
  FXStringArray LicensesAsHex ;
  FXString NewRecordAsHex ;
  LicenseData NewLicense ;
  FXString ComputerName = NewLicense.m_ComputerName ;
  vector< LicenseData > AllExistingLicenses ;

  if ( iLicenseTimeDaysOrRemove >= 0 ) // Create license record (if necessary)
  {
    NewRecordAsHex = CreateLicenseRecord( pGadgetName , iLicenseTimeDaysOrRemove ,
      NewLicense ) ;
  }
  bool bLicenseExists = false ;
  FXRegistry LicensesFolder( FolderOfLicenses ) ;
  if ( LicensesFolder.EnumerateFolderForSubfolders( "" , Folders ) )
  {  // there are existing licenses
    for ( int i = 0 ; i < Folders.GetCount() ; i++ )
    {
      FXString FolderName = Folders[ i ] ;
      FXString NextLicense = LicensesFolder.GetRegiString( FolderName , "LicenseData" , "" ) ;
      LicensesFolder.DeleteKey( FolderName ) ;
      if ( !NextLicense.IsEmpty() )
      {
        LicenseData Decoded ;
        FXString LicenseName ;
        int iLicenseDurationDays = 0 ;
        __time64_t LicenseBegin = 0L ;
        if ( DecodeLicenseRecord( NextLicense , LicenseName , iLicenseDurationDays , LicenseBegin , Decoded ) )
        {
          if ( LicenseName == pGadgetName )
          {
            bLicenseExists = true ;
            if ( iLicenseDurationDays < 0 ) // Is necessary to delete?
              continue ; // don't add license to arrays
            else // replace license record
            {
              LicensesAsHex.Add( NewRecordAsHex ) ;
              AllExistingLicenses.push_back( NewLicense ) ;
            }
          }
          else
          {
            if ( ComputerName == Decoded.m_ComputerName )
            {
              AllExistingLicenses.push_back( Decoded ) ;
              LicensesAsHex.Add( NextLicense ) ;
            }
          }
        }
      }
    }

  }
  if ( !bLicenseExists && !NewRecordAsHex.IsEmpty() )
  {
    AllExistingLicenses.push_back( NewLicense ) ;
    LicensesAsHex.Add( NewRecordAsHex ) ;
  }

  if ( LicensesAsHex.GetCount() )
  {
    for ( int i = 0 ; i < LicensesAsHex.GetCount() ; i++ )
    {
      FXString LicenseFolderName ;
      LicenseFolderName.Format( "License%d" , i + 1 ) ;
      LicensesFolder.WriteRegiString( LicenseFolderName , "LicenseData" , LicensesAsHex[ i ] ) ;
    }
  }
  return ( LicensesAsHex.GetCount() > 0 ) ;
}

int Tvdb400_CheckLicense( LPCTSTR pGadgetName )
{
  FXString RegName( "TheFileX\\" ) ;
  FXRegistry Reg( RegName );

  __time64_t Now ;
  time( &Now ) ;
  struct tm NowS ;
  _gmtime64_s( &NowS , &Now ) ;

  FXString FolderOfLicenses( RegName + "Licensing" ) ;
  FXStringArray Folders ;

  bool bLicenseExists = false ;
  FXRegistry LicensesFolder( FolderOfLicenses ) ;
  if ( LicensesFolder.EnumerateFolderForSubfolders( "" , Folders ) )
  {  // there are existing licenses
    for ( int i = 0 ; i < Folders.GetCount() ; i++ )
    {
      FXString FolderName = Folders[ i ] ;
      FXString NextLicense = LicensesFolder.GetRegiString( FolderName , "LicenseData" , "" ) ;
      if ( !NextLicense.IsEmpty() )
      {
        LicenseData Decoded ;
        FXString ComputerName = Decoded.m_ComputerName ;
        FXString LicenseName ;
        int iLicenseDurationDays = 0 ;
        __time64_t LicenseBegin = 0L ;
        if ( DecodeLicenseRecord( NextLicense , LicenseName , iLicenseDurationDays , LicenseBegin , Decoded ) )
        {
          if ( LicenseName == pGadgetName )
          {
            if ( iLicenseDurationDays != 0 )
            {
              double dTimeDiff_seconds = difftime( Now , LicenseBegin ) ;
#define SecondsPerDay (3600 * 24)
              int iRestDays = iLicenseDurationDays - ( int ) ( dTimeDiff_seconds / SecondsPerDay ) ;
              return ( iRestDays > 0 ) ? iRestDays : 0 ;
            }
            else
              return 10000000 ; // the rest in days
          }
        }
      }
    }

  }
  return -1 ; // No license
}

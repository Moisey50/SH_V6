#include "stdafx.h"
#include "basesecurity.h"
using namespace std ;
#include <string>
#include <algorithm>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <helpers/FXParser2.h>
#include <helpers/propertykitEx.h>


unsigned char secseq[]={0xD6,0xD7,0xD4,0xEF,0x6B,0xDA,0x77,0x15,0xFC,0x98,0x07,0x6C,0x11,0x36,0x03,0x65,
  0x9D,0x9B,0xFC,0x87,0xA6,0xC1,0xE4,0x3D,0x9F,0x5A,0x44,0x98,0xA3,0xA9,0x87,0x57,
  0xA7,0x0C,0x69,0x9C,0xD8,0xDF,0xA3,0x3E,0xFB,0x4D,0x19,0xF5,0x10,0x8D,0x15,0x47,
  0xCB,0x57,0xC8,0x3E,0xAE,0xA3,0x34,0xA0,0x43,0x96,0xEC,0x3C,0x1A,0xD1,0xB5,0x30};

unsigned secseq_size=sizeof(secseq);

SystemInfo g_SystemInfo ;

DWORD GetWindowsVersionInfo()
{
  OSVERSIONINFO osver = { sizeof(osver) };
  GetVersionEx(&osver);
  return osver.dwMajorVersion;
}

BOOL IsUserInAdminGroup()
{
#if (_MSC_VER<1300)
  return TRUE;
#else
  BOOL fInAdminGroup = FALSE;
  DWORD dwError = ERROR_SUCCESS;
  HANDLE hToken = NULL;
  HANDLE hTokenToCheck = NULL;
  DWORD cbSize = 0;

  // Open the primary access token of the process for query and duplicate.
  if (!OpenProcessToken(GetCurrentProcess(), 
    TOKEN_QUERY | TOKEN_DUPLICATE, &hToken))
  {
    dwError = GetLastError();
    goto Cleanup;
  }

  // Determine whether system is running Windows Vista or later operating 
  // systems (major version >= 6) because they support linked tokens, but 
  // previous versions (major version < 6) do not.

  if (GetWindowsVersionInfo()>= 6)
  {
    // Running Windows Vista or later (major version >= 6). 
    // Determine token type: limited, elevated, or default. 
    TOKEN_ELEVATION_TYPE elevType;
    if (!GetTokenInformation(hToken, TokenElevationType, &elevType, 
      sizeof(elevType), &cbSize))
    {
      dwError = GetLastError();
      goto Cleanup;
    }

    // If limited, get the linked elevated token for further check.
    if (TokenElevationTypeLimited == elevType)
    {
      if (!GetTokenInformation(hToken, TokenLinkedToken, &hTokenToCheck, 
        sizeof(hTokenToCheck), &cbSize))
      {
        dwError = GetLastError();
        goto Cleanup;
      }
    }
  }

  // CheckTokenMembership requires an impersonation token. If we just got a 
  // linked token, it already is an impersonation token.  If we did not get 
  // a linked token, duplicate the original into an impersonation token for 
  // CheckTokenMembership.
  if (!hTokenToCheck)
  {
    if (!DuplicateToken(hToken, SecurityIdentification, &hTokenToCheck))
    {
      dwError = GetLastError();
      goto Cleanup;
    }
  }

  // Create the SID corresponding to the Administrators group.
  BYTE adminSID[SECURITY_MAX_SID_SIZE];
  cbSize = sizeof(adminSID);
  if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, &adminSID,  
    &cbSize))
  {
    dwError = GetLastError();
    goto Cleanup;
  }

  // Check if the token to be checked contains admin SID.
  // http://msdn.microsoft.com/en-us/library/aa379596(VS.85).aspx:
  // To determine whether a SID is enabled in a token, that is, whether it 
  // has the SE_GROUP_ENABLED attribute, call CheckTokenMembership.
  if (!CheckTokenMembership(hTokenToCheck, &adminSID, &fInAdminGroup)) 
  {
    dwError = GetLastError();
    goto Cleanup;
  }

Cleanup:
  // Centralized cleanup for all allocated resources.
  if (hToken)
  {
    CloseHandle(hToken);
    hToken = NULL;
  }
  if (hTokenToCheck)
  {
    CloseHandle(hTokenToCheck);
    hTokenToCheck = NULL;
  }

  // Throw the error if something failed in the function.
  if (ERROR_SUCCESS != dwError)
  {
    throw dwError;
  }

  return fInAdminGroup;
#endif
}

BOOL IsRunAsAdmin()
{
#if (_MSC_VER<1300)
  return TRUE;
#else
  BOOL fIsRunAsAdmin = FALSE;
  DWORD dwError = ERROR_SUCCESS;
  PSID pAdministratorsGroup = NULL;

  // Allocate and initialize a SID of the administrators group.
  SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
  if (!AllocateAndInitializeSid(
    &NtAuthority, 
    2, 
    SECURITY_BUILTIN_DOMAIN_RID, 
    DOMAIN_ALIAS_RID_ADMINS, 
    0, 0, 0, 0, 0, 0, 
    &pAdministratorsGroup))
  {
    dwError = GetLastError();
    goto Cleanup;
  }

  // Determine whether the SID of administrators group is enabled in 
  // the primary access token of the process.
  if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
  {
    dwError = GetLastError();
    goto Cleanup;
  }

Cleanup:
  // Centralized cleanup for all allocated resources.
  if (pAdministratorsGroup)
  {
    FreeSid(pAdministratorsGroup);
    pAdministratorsGroup = NULL;
  }

  // Throw the error if something failed in the function.
  if (ERROR_SUCCESS != dwError)
  {
    throw dwError;
  }

  return fIsRunAsAdmin;
#endif
}

bool ElevatePrivilege(LPCTSTR companyname)
{
  BOOL res=IsRunAsAdmin();
  if (res) return true;
  TCHAR szPath[MAX_PATH];
  TCHAR param[REGISTREDTO_MAXLEN];
  if (GetModuleFileName(NULL, szPath, MAX_PATH))
  {
    // Launch itself as administrator.
    SHELLEXECUTEINFO sei = { sizeof(sei) };
    sei.lpVerb = _T("runas");
    sei.lpFile = szPath;
    sei.hwnd = NULL;
    sei.nShow = SW_NORMAL;
    if (!companyname)
      sei.lpParameters= _T("-i eval");
    else
    {
      _tcscpy_s(param,companyname);
      sei.lpParameters=param;
    }
    sei.fMask=SEE_MASK_NOCLOSEPROCESS;
    if (!ShellExecuteEx(&sei))
    {
      DWORD dwError = GetLastError();
      return false;
    }
    WaitForSingleObject(sei.hProcess,10000);
    return true;
  }
  return false;
}

/// The new one
Rights theRights;

FXSIZE trim( char * pStr , LPCTSTR pSepars = " " )
{
  char * pTarget = pStr ;
  char * pEnd = pStr + _tcslen( pStr ) - 1 ;
  while ( pEnd > pStr )
  {
    LPCTSTR pSep = pSepars ;
    for ( ; *pSep ; pSep++ )
    {
      if ( *pEnd == *pSep )
      {
        pEnd-- ;
        break ;
      }
    }
    if ( !*pSep )
      break ;  // first not separator from end
  }
  if ( pEnd == pStr )
  {
    *pStr = 0 ;
    return 0 ;
  }
  *( ++pEnd ) = 0 ;

  for ( char * p = pStr ; *p ; p++ )
  {
    LPCTSTR pSep = pSepars ;
    for ( ; *pSep ; pSep++ )
    {
      if ( *p == *pSep )
        break ;
    }
    if ( !*pSep ) 
    { // first not separator on begin string side
      _tcscpy_s( pTarget , pEnd - p + 1 , p ) ;
      return ( pEnd - p ) ;
    }
  }
  return 0 ;
}

FXSIZE trim( std::string& Str , LPCTSTR pSepars = " " )
{
  for ( auto It = Str.begin() ; Str.size() ; )
  {
    if ( _tcschr( pSepars , *It ) )
    {
      Str.erase( It ) ;
      It = Str.begin() ;
    }
    else
      break ; // first not separator
  }
  if ( Str.empty() )
    return 0 ;
  auto It = Str.begin() + Str.size() - 1 ;
  for ( ; Str.size() ; )
  {
    if ( _tcschr( pSepars , *It ) )
    {
      Str.erase( It ) ;
      It = Str.begin() + Str.size() - 1 ;
    }
    else
      break ; // first not separator
  }
  return Str.size() ;
}


void to_lower( char * pStr )
{
  while ( *pStr )
    *(pStr++) = tolower( *pStr ) ;
}

int windows_system_hidden( const char *cmd )
{
  PROCESS_INFORMATION p_info;
  STARTUPINFO s_info;

  memset( &s_info , 0 , sizeof( s_info ) );
  memset( &p_info , 0 , sizeof( p_info ) );
  s_info.cb = sizeof( s_info );
  s_info.wShowWindow = SW_HIDE ;

  char cmd_line[ 2000 ] ;
  strcpy_s( cmd_line , "cmd.exe /c " ) ;
  strcat_s( cmd_line , cmd ) ;
  char prog[ 2000 ] ;
  //strcpy_s( prog , cmd ) ;
  _tcscpy_s( prog , "cmd.exe" ) ;
  //strcat_s( cmd_line , " > d:\\Tmp\\Answer.txt" ) ;

  if ( CreateProcess( NULL , cmd_line , NULL , NULL , 0 , CREATE_NO_WINDOW , NULL , NULL , &s_info , &p_info ) )
  {
    WaitForSingleObject( p_info.hProcess , INFINITE );
    CloseHandle( p_info.hProcess );
    CloseHandle( p_info.hThread );
    //WinExec( cmd , SW_HIDE );
    return 0 ; // OK
  }
  return -1 ; // Fault
}

int ReadAndDeleteTextFile( LPCTSTR pFileName , FXStringArray& TextStrings )
{
  FILE * fr = NULL ;
  char Buf[ 16000 ] ;
  int iNStrings = 0 ;
  errno_t err = fopen_s( &fr , pFileName , "r" ) ;
  if ( err == ERROR_SUCCESS )
  {
    int iNRead = (int)fread_s( Buf , sizeof( Buf ) , 1 , sizeof( Buf ) , fr ) ;
    const char * p = Buf ;
    FXString NextString ;
    while ( p - Buf < iNRead )
    {
      char cVal = *( p++ ) ;
      if ( cVal <= 0 )
        continue ;
      NextString += cVal ;
      if ( cVal == '\n' )
      {
        if ( !NextString.IsEmpty() )
        {
          TextStrings.Add( NextString ) ;
          ++iNStrings ;
        }
        NextString.Empty() ;
      }
    }
    fclose( fr ) ;
    DeleteFile( pFileName ) ;
  }
  return iNStrings ;
}

FXSIZE GetDataFromWMIC( LPCTSTR pModule , LPCTSTR pProperty ,
  void * pOutBufOrVectorStringPtr , int iBufLen = 0 )
{
  FILE* wmic_output;
  char * pOutBuf = ( iBufLen == 0 ) ? NULL : ( char* ) pOutBufOrVectorStringPtr ;
  FXStringArray * pVector = ( iBufLen == 0 ) ? ( FXStringArray* ) pOutBufOrVectorStringPtr : NULL ;

  char Command[ 256 ] ;

  char LocalProp[ 256 ] ;
  strcpy_s( LocalProp , pProperty ) ;

  FXSIZE iRestLen = trim( LocalProp , " \r\n" ) ;
  if ( !iRestLen )
    return 0 ;
  if ( pOutBuf )
  {
    *pOutBuf = 0 ;
    iRestLen = iBufLen - 1 ;
  }

  strcpy_s( Command , "wmic " ) ;
//  strcpy_s( Command , "wmic.exe " ) ;
  strcat_s( Command , pModule ) ;
  strcat_s( Command , " get " ) ;
  to_lower( LocalProp ) ;
  strcat_s( Command , LocalProp ) ;

  strcat_s( Command , " > d:\\Tmp\\wmicanswer.txt" ) ;
  windows_system_hidden( Command ) ; // Run the WMIC command to get data

  FXStringArray Answer ;

  if ( ReadAndDeleteTextFile( "d:\\Tmp\\wmicanswer.txt" , Answer ) )
  {  // there are some data

    for ( FXSIZE i = 0 ; i < Answer.size() ; i++ )
    {
      Answer[ i ] = Answer[ i ].MakeLower() ;
      if ( Answer[i].MakeLower().Find( LocalProp ) < 0 ) 
      {
        if ( pOutBuf )
        {
          int iStrLen = (int)Answer[ i ].GetLength() ;
          if ( _tcscat_s( ( char * ) pOutBuf , iRestLen , (LPCTSTR) Answer[ i ] ) )
            break ;
          iRestLen -= iStrLen ;
        }
        else if ( pVector )
          pVector->Add( Answer[ i ] ) ;
      }
    }
    return Answer.size() ;
  }



  wmic_output = _popen( Command , "r" );

  // Read the output of the command
  if ( wmic_output )
  {
    char buffer[ 256 ];
    while ( fgets( buffer , sizeof( buffer ) , wmic_output ) )
    {
      to_lower( buffer ) ;
      iRestLen = trim( buffer , " \r\n" ) ;
      if ( iRestLen && !_tcsstr( buffer , LocalProp ) )
      {
        if ( pOutBuf )
        {
          if ( _tcscat_s( ( char * ) pOutBuf , iBufLen , buffer ) )
            break ;
        }
        else if ( pVector )
          pVector->Add( FXString(buffer) ) ;
      }
    }
    _pclose( wmic_output );



    return ( pOutBuf ) ? _tcslen( pOutBuf ) : pVector ? pVector->size() : 0  ;
  }
  return 0 ;
}

FXSIZE GetDataFromWMIC( LPCTSTR pModule , LPCTSTR pProperty ,
  FXStringArray& Results )
{
  FILE* wmic_output;

  char buffer[ 256 ];
  char Command[ 256 ] ;

  char LocalProp[ 256 ] ;
  strcpy_s( LocalProp , pProperty ) ;

  FXSIZE iRestLen = trim( LocalProp , " \r\n" ) ;
  if ( !iRestLen )
    return 0 ;

  to_lower( LocalProp ) ;
  strcpy_s( Command , "wmic.exe " ) ;
  strcat_s( Command , pModule ) ;
  strcat_s( Command , " get " ) ;
  strcat_s( Command , LocalProp ) ;

  // Run the WMIC command to get the motherboard serial number
  wmic_output = _popen( Command , "r" );

  // Read the output of the command
  if ( wmic_output )
  {
    while ( fgets( buffer , sizeof( buffer ) , wmic_output ) )
    {
      FXString Str( buffer ) ;
      Str.Trim( " \r\n" ) ;
      Str.MakeLower() ;
      if ( Str.Find( LocalProp ) < 0 )
        Results.Add( Str ) ;
    }
    _pclose( wmic_output );
    return Results.size() ;
  }
//    strcat_s( Command , " > wmicanswer.txt" ) ;
//    windows_system_hidden( Command ) ;

//     ReadAndDeleteTextFile( "wmicanswer.txt" , Results ) ;
//     for ( int i = 0 ; i < Results.size() ; i++ )
//     {
//       if ( Results[ i ] == LocalProp )
//         Results.RemoveAt( i ) ;
//     }
   return 0 ;
}

int GetMotherBoardSN( FXString& BoardSN )
{
  TCHAR buffer[ 4000 ];
  if ( GetDataFromWMIC( "baseboard" , "serialnumber" , buffer , sizeof( buffer) ) )
  {
    if ( BoardSN.IsEmpty() || ( BoardSN != buffer ) )
    {
      BoardSN = buffer ;
      BoardSN.Trim( " \r\n" ) ;
    }
    return 1 ;
  }
  return 0 ;
}

int GetMotherBoardProductName( FXString& Name )
{
  char buffer[ 4000 ];
  if ( GetDataFromWMIC( "baseboard" , "product" , buffer , sizeof( buffer ) ) )
  {
    if ( Name.IsEmpty() || ( Name != buffer ) )
    {
      Name = buffer ;
      Name.Trim( " \r\n" ) ;
    }
    return 1 ;
  }
  return 0 ;
}

int GetComputerName( FXString& Name )
{
  char buffer[ 4000 ];
  if ( GetDataFromWMIC( "computersystem" , "name" , buffer , sizeof( buffer ) ) )
  {
    if ( Name.IsEmpty() || ( Name != buffer ) )
    {
      Name = buffer ;
      Name.Trim( " \r\n" ) ;
    }
    return 1 ;
  }
  return 0 ;
}
int GetCPUName( FXString& Name )
{
  char buffer[ 4000 ];
  if ( GetDataFromWMIC( "cpu" , "name" , buffer , sizeof( buffer ) ) )
  {
    if ( Name.IsEmpty() || ( Name != buffer ) )
    {
      Name = buffer ;
      Name.Trim( " \r\n" ) ;
    }
    return 1 ;
  }
  return 0 ;
}

int GetMemoryIDs( FXStringArray& IDs )
{
  IDs.clear() ;
  if ( GetDataFromWMIC( "memorychip" , "serialnumber" , &IDs ) )
  {
    for ( FXSIZE i = 0 ; i < IDs.size() ; i++ )
      IDs[ i ].Trim( " \r\n" ) ;
  }
  return (int)IDs.size() ;
}

FXSIZE GetDisksSNs( FXStringArray& SNs )
{
  FXStringArray Interfaces ;
  if ( !GetDataFromWMIC( "diskdrive" , "interfacetype" , &Interfaces ) )
    return 0 ;
  SNs.clear() ;
  FXStringArray SNtmp ;
  if ( !GetDataFromWMIC( "diskdrive" , "serialnumber" , &SNtmp ) )
    return 0 ;
  for ( int i = 0 ; i < Interfaces.size() ; i++ )
  {
    Interfaces[ i ].Trim( " \r\n" ) ;
    SNtmp[ i ].Trim( " \r\n" ) ;
    if ( ( Interfaces[ i ] == _T("ide") ) || ( Interfaces[ i ] == _T("scsi") ) )
      SNs.Add( SNtmp[ i ] ) ;
  }
  return SNs.size() ;
}

FXSIZE GetLAN_MACs( FXStringArray& MACs )
{
  FXStringArray IsPhysical ;
  if ( !GetDataFromWMIC( "nic" , "physicaladapter" , ( void * ) &IsPhysical ) )
    return 0 ;
  MACs.clear() ;
  FXStringArray MACstmp ;
  if ( !GetDataFromWMIC( "nic" , "macaddress" , &MACstmp ) )
    return 0 ;
  for ( int i = 0 ; i < IsPhysical.size() ; i++ )
  {
    IsPhysical[ i ].Trim( " \r\n" ) ;
    MACstmp[ i ].Trim( " \r\n" ) ;
    if ( ( IsPhysical[ i ] == "true" ) && MACstmp[ i ].size() )
    {
      MACs.Add( MACstmp[ i ] ) ;
    }
  }
  return MACs.size() ;
}

void SystemInfo::Copy( SystemInfo& Orig )
{
  m_ComputerName =    Orig.m_ComputerName ;
  m_ProcessorName =   Orig.m_ProcessorName ;
  m_BoardName =       Orig.m_BoardName ;
  m_BoardSN =         Orig.m_BoardSN ;
  m_SHLicenseStatus = Orig.m_SHLicenseStatus ;
  m_Licensee =        Orig.m_Licensee ;
  m_LicenseDate =     Orig.m_LicenseDate ;
  m_DiskSNs.Copy( Orig.m_DiskSNs ) ;
  m_MemoryIDs.Copy( Orig.m_MemoryIDs ) ;
  m_MACs.Copy( Orig.m_MACs ) ;
  m_bFilled = true ;
}
bool SystemInfo::GetSystemInfo()
{
  if ( !g_SystemInfo.IsFilled() )
  {
    GetMotherBoardSN( g_SystemInfo.m_BoardSN ) ;
    GetMotherBoardProductName( g_SystemInfo.m_BoardName ) ;
    GetCPUName( g_SystemInfo.m_ProcessorName ) ;
    GetComputerName( g_SystemInfo.m_ComputerName ) ;
    GetDisksSNs( g_SystemInfo.m_DiskSNs ) ;
    GetMemoryIDs( g_SystemInfo.m_MemoryIDs ) ;
    GetLAN_MACs( g_SystemInfo.m_MACs ) ;
    g_SystemInfo.SetFilled( true ) ;
  }
  if ( this != &g_SystemInfo )
  {
    Copy( g_SystemInfo ) ;
  }

  return true ;
}


bool SystemIDs::GetThisSystemInfo()
{
  m_Info.GetSystemInfo() ;
  m_bSettled = true ;
  return true ;
}

void CpyToArray( char * Dest , FXSIZE DestLen , const char * Src , FXSIZE SrcLen )
{
  memcpy( Dest , Src ,
    ( SrcLen < ( DestLen - 1 ) ) ? SrcLen + 1 : DestLen - 1 ) ;
  if ( SrcLen >= DestLen - 1 )
    Dest[ LOCATION_ITEM_LEN - 1 ] = 0 ;
}

bool SystemIDs::SaveSystemInfo( bool bToRegistry , FormFilesCode FormViewFiles )
{
  bool retV = false;

  HKEY hk = NULL ;
  if ( bToRegistry )
  {
    hk = openthis( false );
    if ( !hk )
      return false;
  }

  locationdata LData( true ) ; // for data writing to registry

  LData.AddString( m_Info.m_ComputerName , "ComputerName" ) ;
  LData.AddString( m_Info.m_ProcessorName , "ProcessorName" ) ;
  LData.AddString( m_Info.m_BoardName , "BoardName") ;
  LData.AddString( m_Info.m_BoardSN , "BoardSN" ) ;
  LData.AddString( m_Info.m_SHLicenseStatus , "LicenseStatus" ) ;
  LData.AddString( m_Info.m_Licensee , "Licensee" ) ;
  LData.AddString( m_Info.m_LicenseDate , "LicenseDate" ) ;

  LData.AddNumber( ( int ) m_Info.m_DiskSNs.size() , "NDisks" ) ;
  for ( FXSIZE i = 0 ; i < m_Info.m_DiskSNs.size() && i < 8; i++ )
    LData.AddString( m_Info.m_DiskSNs[i] , "Disk" , ( int ) i + 1 ) ;
 
  LData.AddNumber( ( int ) m_Info.m_MemoryIDs.size() , "NMemModules" ) ;
  for ( FXSIZE i = 0 ; i < m_Info.m_MemoryIDs.size() && i < 8; i++ )
    LData.AddString( m_Info.m_MemoryIDs[ i ] , "MemModule" , (int)i + 1 ) ;

  LData.AddNumber( ( int ) m_Info.m_MACs.size() , "NMACs" ) ;
  for ( FXSIZE i = 0 ; i < m_Info.m_MACs.size() && i < 8; i++ )
    LData.AddString( m_Info.m_MACs[ i ] , "MAC" , ( int ) i + 1 ) ;

  LData.DoPadding() ;
  if ( (FormViewFiles == FFC_SimpleViewSysInfo) || (FormViewFiles == FFC_WriteSysInfoAndKey) )
  {
    std::fstream fs( "LicenseInfo.txt" , std::fstream::out | std::fstream::trunc );
    if ( fs.is_open() )
    {
      fs << "Time Stamp: " << GetTimeStamp() << EOL ;
      fs << "Licensee: " << m_Info.m_Licensee << EOL ;
      fs << "License Status: " << m_Info.m_SHLicenseStatus << EOL ;
      fs << "License Date: " << m_Info.m_LicenseDate << EOL ;
      fs << "System name: " << ( LPCTSTR ) m_Info.m_ComputerName << EOL ;
      fs << "Processor: " << ( LPCTSTR ) m_Info.m_ProcessorName << EOL ;
      fs << "Mother Board name: " << ( LPCTSTR ) m_Info.m_BoardName << EOL ;
      fs << "MB SN: " << ( LPCTSTR ) m_Info.m_BoardSN << EOL ;
      fs << "Disks: " << EOL ;
      for ( FXSIZE i = 0 ; i < m_Info.m_DiskSNs.size() && i < 8; i++ )
        fs << "   Disk " << i << ": " << ( LPCTSTR ) m_Info.m_DiskSNs[ i ] << EOL ;

      fs << "Memory Modules: " << EOL ;
      for ( FXSIZE i = 0 ; i < m_Info.m_MemoryIDs.size() && i < 8; i++ )
        fs << "   Module " << i << ": " << ( LPCTSTR ) m_Info.m_MemoryIDs[ i ] << EOL ;

      fs << "MACs: " << EOL ;
      for ( FXSIZE i = 0 ; i < m_Info.m_MACs.size() && i < 8; i++ )
        fs << "   MAC " << i << ": " << ( LPCTSTR ) m_Info.m_MACs[ i ] << EOL ;

      fs.close();
    }
  }

   // encode to hex
  UINT * pData = ( UINT * ) LData.m_AsString ;
  UINT * pEnd = pData + LData.GetLength() / sizeof( UINT ) ;
  
  UINT * pRand = LData.m_Random + LData.m_uiShift ;
  UINT * pRandEnd = LData.m_Random + RandomSize ;

//  m_CRCsOnEncoding.clear() ;
  do
  {
    LData.m_CRC += *pData ;
//    m_CRCsOnEncoding.push_back( LData.m_CRC ) ;
    *pData ^= *( pRand++ ) ;
    if ( pRand >= pRandEnd )
      pRand = LData.m_Random ;
  } while ( ++pData < pEnd ) ;

  LData.m_uiShift ^= LData.m_Random[ RandomSize - 1 ] ; // last random number in array
  DWORD dwLen = sizeof( LData ) - sizeof( LData.m_AsString ) + LData.GetLength() ;
  FXString AsHex ;
  LData.m_iLen ^= LData.m_Random[ RandomSize - 2 ] ; // last random number in array
  BYTE CRC = ConvBytesToHexSaveCRC( ( LPBYTE ) &LData , dwLen , AsHex ) ;

  if ( ( FormViewFiles == FFC_DoInfoForLicenseFile ) || ( FormViewFiles == FFC_WriteSysInfoAndKey ) )
  {
    CString KeyFileName( ( FormViewFiles == FFC_DoInfoForLicenseFile ) ? "InfoForLicense_" : "SysInfoCode_" ) ;
    KeyFileName += m_Info.m_ComputerName + ".txt" ;
    // Form encoded file
    std::fstream Encoded( (LPCTSTR)KeyFileName , 
      std::fstream::out | std::fstream::trunc );
    if ( Encoded.is_open() )
    {
      FXSIZE uLength = AsHex.GetLength() ;
      Encoded.write( ( LPCTSTR ) AsHex , uLength ) ;
      Encoded.close() ;
    }
  }

  SystemIDs Check ;
  string sAsHex( ( LPCTSTR ) AsHex ) ;
  bool bRes = DecodeSystemInfo( sAsHex , Check ) ;
  ASSERT( bRes ) ;
 
  if ( bToRegistry )
  {
    retV = (RegSetValueEx( hk , EVAL_KEY_NAME_CU , 0 , REG_BINARY , 
      (LPBYTE)((LPCTSTR)AsHex) , (DWORD)AsHex.GetLength() + 1 ) == ERROR_SUCCESS) ;
      //  retV = RegSetValueEx( hk , "hinfo" , 0 , REG_SZ , ( LPBYTE ) LData.m_CPUName , 
      //              _tcslen( LData.m_CPUName ) + 1 ) == ERROR_SUCCESS;
    TRACE( "\nResult data writing is %d, Admin=%d, Last Error %d\n" , retV , IsRunAsAdmin() , GetLastError() ) ;

    RegCloseKey( hk );
  }
  else
  {
    CString FileName ;
    if ( m_SaveFileName.IsEmpty() )
      FileName.Format( "Info_%s.dat" , ( LPCTSTR ) m_Info.m_ComputerName ) ;
    else
      FileName = m_SaveFileName ;
    ofstream myfile( ( LPCTSTR ) FileName , std::fstream::out | std::fstream::trunc );
    if ( myfile.is_open() )
    {
      myfile << ( LPCTSTR ) AsHex ;
      myfile.close();
      FXString Msg ;
      Msg.Format( "Success: Information is written into file %s\n" ,
        strerror( GetLastError() ) , ( LPCTSTR ) FileName );
      cerr << ( LPCTSTR ) Msg ;
      retV = true ;
    }
    else
    {
      FXString Msg ;
      Msg.Format( "ERROR %s: Can't write key data file %s\n" ,
        strerror( GetLastError() ) , ( LPCTSTR ) AsHex );
      cerr << ( LPCTSTR ) Msg ;
    }
  };
  return retV;
}

bool SystemIDs::LoadLicenseInfo()
{
  bool retV = false;

  HKEY hk = openthis( false );
  if ( !hk )
      return false;

  char Buffer[ 16000 ] ;
  DWORD dwLen = sizeof( Buffer ) ;
  DWORD dwType = 0 ;
  retV = RegGetValue( hk , NULL , EVAL_KEY_NAME_CU , RRF_RT_REG_BINARY , &dwType ,
    ( LPVOID ) Buffer , &dwLen ) == ERROR_SUCCESS;
  TRACE( "\nResult data reading is %d, InfoLen=%d, Last Error %d\n" , retV , dwLen , GetLastError() ) ;
  RegCloseKey( hk );

  if ( retV )
  {
    string Encoded = Buffer ; // Zero on the end is written to registry
    if ( DecodeSystemInfo( Encoded , *this ) )
      return true ;
    else
      TRACE( "\n!!!!!!!!!!!!!!  Can't decode license\n" ) ;
  }
  return false ;
}

bool SystemIDs::DecodeSystemInfo( string& Encoded , SystemIDs& Decoded )
{
  DWORD dwLen = (DWORD)Encoded.size() ;
  LPTSTR pBuffer = new TCHAR[ dwLen + 10 ] ;
  ConvHexToBytes( Encoded.c_str() , ( int ) Encoded.size() ,
    ( LPBYTE ) pBuffer , dwLen ) ;

  locationdata * pLData = ( locationdata* ) pBuffer ;

  pLData->m_uiShift ^= pLData->m_Random[ RandomSize - 1 ] ;
  pLData->m_iLen ^= pLData->m_Random[ RandomSize - 2 ] ;


  UINT * pData = ( UINT * ) pLData->m_AsString ;
  UINT * pEnd = pData + pLData->GetLength()/sizeof(UINT) ; 

  UINT * pRand = pLData->m_Random + pLData->m_uiShift ;
  UINT * pRandEnd = pLData->m_Random + RandomSize ;

  UINT uiCRC = 0 ;
//  m_CRCsOnDecoding.clear() ;
  do
  {
    UINT uiVal = (*pData ^= *( pRand++ )) ;
    if ( pRand >= pRandEnd )
      pRand = pLData->m_Random ;

    uiCRC += uiVal ;
//    m_CRCsOnDecoding.push_back( uiCRC ) ;
  } while ( ++pData < pEnd ) ;

  m_bSettled = false ;
  if ( uiCRC == pLData->m_CRC )
  {
    pLData->m_AsString[ pLData->m_iLen ] = 0 ;
    FXPropertyKit pk( pLData->m_AsString ) ;
    FXString Tmp ;
    bool bFound = pk.GetString( "ComputerName" , Tmp , false ) ;
    if ( !bFound ) 
      return false ;
    Decoded.m_Info.m_ComputerName = ( LPCTSTR ) Tmp ;
    
    bFound = pk.GetString( "BoardName" , Tmp , false ) ;
    if ( !bFound )
      return false ;
    Decoded.m_Info.m_BoardName = ( LPCTSTR ) Tmp ;

    bFound = pk.GetString( "BoardSN" , Tmp , false ) ;
    if ( !bFound )
      return false ;
    Decoded.m_Info.m_BoardSN = ( LPCTSTR ) Tmp ;

    bFound = pk.GetString( "ProcessorName" , Tmp , false ) ;
    if ( !bFound )
      return false ;
    Decoded.m_Info.m_ProcessorName = ( LPCTSTR ) Tmp ;

    bFound = pk.GetString( "LicenseStatus" , Tmp , false ) ;
    if ( !bFound )
      return false ;
    Decoded.m_Info.m_SHLicenseStatus = ( LPCTSTR ) Tmp ;

    bFound = pk.GetString( "Licensee" , Tmp , false ) ;
    if ( !bFound )
      return false ;
    Decoded.m_Info.m_Licensee = ( LPCTSTR ) Tmp ;

    bFound = pk.GetString( "LicenseDate" , Tmp , false ) ;
    if ( !bFound )
      return false ;
    Decoded.m_Info.m_LicenseDate = ( LPCTSTR ) Tmp ;

    char Name[ 100 ] ;
    int iNDisks = 0 ;
    bFound = pk.GetInt( "NDisks" , iNDisks ) ;
    if ( !bFound )
      return false ;
    for ( int i = 0 ; i < iNDisks ; i++ )
    {
      sprintf_s( Name , "Disk%d" , i + 1 ) ;
      bFound = pk.GetString( Name , Tmp , false ) ;
      if ( !bFound )
        return false ;
      Decoded.m_Info.m_DiskSNs.Add( Tmp );
    }

    int iNMems = 0 ;
    bFound = pk.GetInt( "NMemModules" , iNMems ) ;
    if ( !bFound )
      return false ;
    for ( int i = 0 ; i < iNMems ; i++ )
    {
      sprintf_s( Name , "MemModule%d" , i + 1 ) ;
      bFound = pk.GetString( Name , Tmp , false ) ;
      if ( !bFound )
        return false ;
      ASSERT( pk.GetString( Name , Tmp , false ) ) ;
      Decoded.m_Info.m_MemoryIDs.Add( Tmp );
    }

    int iNMACs = 0 ;
    bFound = pk.GetInt( "NMACs" , iNMACs ) ;
    if ( !bFound )
      return false ;
    for ( int i = 0 ; i < iNMACs ; i++ )
    {
      sprintf_s( Name , "MAC%d" , i + 1 ) ;
      bFound = pk.GetString( Name , Tmp , false ) ;
      if ( !bFound )
        return false ;
      Decoded.m_Info.m_MACs.Add( Tmp );
    }
    m_bSettled = true ;
  }
  delete[] pBuffer ;
  return m_bSettled ;
}

bool SystemInfo::Compare( SystemInfo& Other )
{
  bool bComputerOK = ( m_ComputerName.CompareNoCase( Other.m_ComputerName ) == 0 ) ;

  int iNMatched = 0 ;
  if ( m_ProcessorName.CompareNoCase( Other.m_ProcessorName ) == 0 )
    iNMatched++ ;
  if ( m_BoardName.CompareNoCase( Other.m_BoardName ) == 0 )
    iNMatched++ ;
  if ( m_BoardSN.CompareNoCase( Other.m_BoardSN ) == 0 )
    iNMatched++ ;

  int iNDisksMatched = 0 ;
  for ( FXSIZE i = 0 ; i < m_DiskSNs.size() ; i++ )
  {
    for ( FXSIZE j = 0 ; j < Other.m_DiskSNs.size() ; j++ )
    {
      if ( m_DiskSNs[i].CompareNoCase( Other.m_DiskSNs[j] ) == 0 )
      {
        iNDisksMatched++ ;
        break ;
      }
    }
  }

  int iNMemoriesMatched = 0 ;
  for ( FXSIZE i = 0 ; i < m_MemoryIDs.size() ; i++ )
  {
    for ( FXSIZE j = 0 ; j < Other.m_MemoryIDs.size() ; j++ )
    {
      if ( m_MemoryIDs[ i ].CompareNoCase( Other.m_MemoryIDs[ j ] ) == 0 )
      {
        iNMemoriesMatched++ ;
        break ;
      }
    }
  }

  int iNMACsMatched = 0 ;
  for ( FXSIZE i = 0 ; i < m_MACs.size() ; i++ )
  {
    for ( FXSIZE j = 0 ; j < Other.m_MACs.size() ; j++ )
    {
      if ( m_MACs[ i ].CompareNoCase( Other.m_MACs[ j ] ) == 0 )
      {
        iNMACsMatched++ ;
        break ;
      }
    }
  }

  bool bOK = ( bComputerOK || (iNMatched >= 2) ) && ( iNMemoriesMatched && iNDisksMatched && iNMACsMatched ) ;

  return bOK ;
}

void SystemIDs::ViewLicenseInfoOnCOUT()
{
  cout << "Time Stamp: " << GetTimeStamp() << EOL ;
  cout << "Licensee: " << m_Info.m_Licensee << EOL ;
  cout << "License Status: " << m_Info.m_SHLicenseStatus << EOL ;
  cout << "License Date: " << m_Info.m_LicenseDate << EOL ;
  cout << "System name: " << ( LPCTSTR ) m_Info.m_ComputerName << EOL ;
  cout << "Processor: " << ( LPCTSTR ) m_Info.m_ProcessorName << EOL ;
  cout << "Mother Board name: " << ( LPCTSTR ) m_Info.m_BoardName << EOL ;
  cout << "MB SN: " << ( LPCTSTR ) m_Info.m_BoardSN << EOL ;
  cout << "Disks: " << EOL ;
  for ( FXSIZE i = 0 ; i < m_Info.m_DiskSNs.size() && i < 8; i++ )
    cout << "   Disk " << i << ": " << ( LPCTSTR ) m_Info.m_DiskSNs[ i ] << EOL ;

  cout << "Memory Modules: " << EOL ;
  for ( FXSIZE i = 0 ; i < m_Info.m_MemoryIDs.size() && i < 8; i++ )
    cout << "   Module " << i << ": " << ( LPCTSTR ) m_Info.m_MemoryIDs[ i ] << EOL ;

  cout << "MACs: " << EOL ;
  for ( FXSIZE i = 0 ; i < m_Info.m_MACs.size() && i < 8; i++ )
    cout << "   MAC " << i << ": " << ( LPCTSTR ) m_Info.m_MACs[ i ] << EOL ;
}

locationdata::locationdata( bool bForWriting )
{
  memset( this , 0 , sizeof( *this ) ) ;
  if ( bForWriting )
  {
//     rand_s( ( UINT* ) &m_Seed.LowPart ) ;
//     rand_s( ( UINT* ) &m_Seed.HighPart ) ;
//     srand( ( UINT ) time( NULL ) ) ;
//     for ( FXSIZE i = sizeof( LARGE_INTEGER ) ; i < sizeof( *this ) ; i++ )
//       *( ( ( LPBYTE ) this ) + i ) = ( BYTE ) rand() ;
    rand_s( &m_uiShift ) ;
    m_uiShift %= ARRSZ( m_Random ) ;
    for ( FXSIZE i = 0 ; i < ARRSZ( m_Random ) ; i++ )
      rand_s( &m_Random[ i ] ) ;
  }
  m_iLen = 0 ;
} ;

int locationdata::AddString( 
  LPCTSTR pString , LPCTSTR pName , int iIndex )
{
  int iNPrinted = ( iIndex == 0 ) ? 
    sprintf_s( m_AsString + m_iLen , sizeof( m_AsString ) - m_iLen ,
    "%s%s%s;" , pName , pName ? "=" : NULL , pString ) 
    :
    sprintf_s( m_AsString + m_iLen , sizeof( m_AsString ) - m_iLen ,
      "%s%d%s%s;" , pName , iIndex , ( pName || iIndex ) ? "=" : NULL , pString ) ;

  ASSERT( iNPrinted > 0 ) ;
  m_iLen += iNPrinted ;

  int iLen = (int)_tcslen( m_AsString ) ;
  ASSERT( iLen == m_iLen ) ;
  return m_iLen ;
}

int locationdata::AddNumber( int iNumber , LPCTSTR pName , int iIndex )
{
  int iNPrinted = ( iIndex == 0 ) ?
    sprintf_s( m_AsString + m_iLen , sizeof( m_AsString ) - m_iLen ,
      "%s%s%d;" , pName , pName ? "=" : NULL , iNumber )
    :
    sprintf_s( m_AsString + m_iLen , sizeof( m_AsString ) - m_iLen ,
      "%s%d%s%d;" , pName , iIndex , ( pName || iIndex ) ? "=" : NULL , iNumber ) ;


  ASSERT( iNPrinted > 0 ) ;
  m_iLen += iNPrinted ;
  int iLen = ( int ) _tcslen( m_AsString ) ;
  ASSERT( iLen == m_iLen ) ;
  return m_iLen ;
}

int locationdata::DoPadding()
{
  int iTheRest = m_iLen % 4 ;
  while ( iTheRest != 0 )
  {
    m_AsString[ m_iLen++ ] = ' ' ;
    iTheRest = m_iLen % 4 ;
  }
  m_AsString[ m_iLen ] = 0 ;
  return m_iLen ;
}

void SystemInfo::Reset()
{
  m_ComputerName.Empty() ;
  m_ProcessorName.Empty() ;
  m_BoardName.Empty() ;
  m_BoardSN.Empty() ;
  m_SHLicenseStatus.Empty() ;
  m_Licensee.Empty() ;
  m_LicenseDate.Empty() ;
  m_DiskSNs.clear() ;
  m_MemoryIDs.clear()  ;
  m_MACs.clear() ;
  m_bFilled = false ;
}
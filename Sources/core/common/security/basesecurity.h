#ifndef _BASESECURITY_INC
#define _BASESECURITY_INC

#pragma comment(lib, "IPHLPAPI.lib")
#include <intrin.h>
using namespace std ;
#include <vector>
#include <string>
#include <fxfc/fxfc.h>

extern unsigned char secseq[];
extern unsigned secseq_size;

DWORD GetWindowsVersionInfo();
bool ElevatePrivilege( LPCTSTR companyname = NULL );
BOOL IsRunAsAdmin();

#pragma pack( push, before_tagstpdata )
#pragma pack( 1 )

#define REGISTREDTO_MAXLEN 255
#define LOCATION_ITEM_LEN 64
#define MULTI_VALUES_MAX_LEN 8

enum EvalOrLicense
{
  EOL_NoLicense = 0 ,
  EOL_NoRestriction ,
  EOL_Evaluation ,
  EOL_License
};

enum FormFilesCode
{
  FFC_NoFiles = 0 ,
  FFC_SimpleViewSysInfo ,
  FFC_WriteSysInfoAndKey ,
  FFC_DoInfoForLicenseFile
};

class setupdata
{
public:
  unsigned    _seed;
  time_t      _stp_time;
  char        _eval_or_license;
  unsigned    _crc;
  int         _LicenseDuration_days ;
  char	    	_registredto[ REGISTREDTO_MAXLEN ];
  setupdata() { memset( this , 0 , sizeof( *this ) ) ; }
} ;

#define RandomSize (32)

class locationdata
{
public:
  UINT m_uiShift ;
  UINT m_CRC ;
  UINT m_Random[ RandomSize ] ;
  int m_iLen ;
  char m_AsString[ 16000 ] ;
//   char m_ComputerName[ LOCATION_ITEM_LEN ] ;
//   char m_BoardName[ LOCATION_ITEM_LEN ] ;
//   char m_BoardSN[ LOCATION_ITEM_LEN ] ;
//   char m_CPUName[ LOCATION_ITEM_LEN ] ;
//   char m_DisksSN[ MULTI_VALUES_MAX_LEN ][ LOCATION_ITEM_LEN/2 ] ;
//   char m_MemoryIDs[ MULTI_VALUES_MAX_LEN ][ LOCATION_ITEM_LEN / 2 ] ;
//   char m_MACs[ MULTI_VALUES_MAX_LEN ][ LOCATION_ITEM_LEN / 2 ] ;
  locationdata( bool bForWriting = true ) ;
  int AddString( LPCTSTR String , LPCTSTR pName = NULL , int iIndex = 0 ) ; // returns length after addition
  int AddNumber( int iNumber , LPCTSTR pName = NULL , int iIndex = 0 ) ; // returns length after addition

  int GetLength() { return m_iLen ; }
  int DoPadding() ;
};

#define FullLenDWORDs ( sizeof( locationdata ) / sizeof( DWORD ) ) 
#define RLenDWORDs  (RandomLenItems + 2)  // m_Random + m_uiShift + m_CRC

class SystemInfo
{
  bool m_bFilled = false ;
public:
  FXString m_ComputerName ;
  FXString m_ProcessorName ;
  FXString m_BoardName ;
  FXString m_BoardSN ;
  FXString m_SHLicenseStatus ;
  FXString m_Licensee ;
  FXString m_LicenseDate ;
  FXStringArray m_DiskSNs ;
  FXStringArray m_MemoryIDs ;
  FXStringArray m_MACs ;

  SystemInfo() {} ;
  SystemInfo( SystemInfo& Other ) { Copy( Other ) ; } ;
  void Reset() ;
  bool IsFilled() { return m_bFilled ; }
  void SetFilled( bool bSet ) { m_bFilled = bSet ; }
  bool GetSystemInfo() ;
  void Copy( SystemInfo& Orig ) ;
  bool Compare( SystemInfo& Other ) ;
};

extern SystemInfo g_SystemInfo ;


class SystemIDs
{
  bool m_bSettled = false ;
public:
  DWORD  m_dwShift ;
  BYTE   m_Random[ 128 ] ;
  SystemInfo m_Info ;
  FXString m_SaveFileName ;

public:
  SystemIDs() { Reset() ; }

  void Reset() { 
    memset( this , 0 , sizeof( m_Random ) + sizeof( m_dwShift ) ) ; 
    m_Info.Reset() ;
  }
  bool IsSettled() { return m_bSettled ; }
  bool GetThisSystemInfo() ;
  bool SaveSystemInfo( bool bToReg = true , FormFilesCode FormViewFiles = FFC_NoFiles ) ;
  bool LoadLicenseInfo() ;
  bool DecodeSystemInfo( string& Encoded , SystemIDs& Decoded ) ;
  void  ViewLicenseInfoOnCOUT() ; 
};

#pragma pack( pop, before_tagstpdata )

#define EVAL_PERIOD 30
#define EVAL_KEYNAME		 _T("data")
#define EVAL_KEY_NAME_CU _T("hinfo")
#define APPLICATION_NAME_KEY _T("tvdb400")

__forceinline HKEY openmainKey( bool bLocalMachine = true )
{
  HKEY  HKey;
  CString key = _T( "SOFTWARE\\" );
#if defined(_WIN64)
  if ( bLocalMachine )
    key += _T( "WOW6432Node\\" ) ;
#endif
  key += COMPANY_NAME_SEC;
  if ( RegOpenKey( bLocalMachine ? 
    HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER , key , &HKey ) != ERROR_SUCCESS )
  {
    return NULL;
  }
  return HKey;
}

__forceinline HKEY openhklm_hard_cpu0()
{
  HKEY  HKey;
  CString key = _T( "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0" );
  if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE , key , NULL , KEY_READ , &HKey ) != ERROR_SUCCESS )
  {
    return NULL;
  }
  return HKey;
}

__forceinline HKEY openhklm_hard_bios()
{
  HKEY  HKey;
  CString key = _T( "HARDWARE\\DESCRIPTION\\System\\BIOS" );
  if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE , key , NULL , KEY_READ , &HKey ) != ERROR_SUCCESS )
  {
    return NULL;
  }
  return HKey;
}

__forceinline HKEY openhklm_hard_scsi()
{
  HKEY  HKey;
  CString key = _T( "HARDWARE\\DEVICEMAP\\Scsi" );
  if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE , key , NULL , KEY_READ , &HKey ) != ERROR_SUCCESS )
  {
    return NULL;
  }
  return HKey;
}

__forceinline HKEY openthis( bool bLocalMachine = true )
{
  HKEY  HKey = openmainKey( bLocalMachine );
  if ( !HKey ) 
    return NULL;
  CString name = APPLICATION_NAME_KEY;
  HKEY retV;
  if ( RegOpenKey( HKey , name , &retV ) != ERROR_SUCCESS )
  {
    return NULL;
  }
  RegCloseKey( HKey );
  return retV;
}

__forceinline HKEY createthis( bool bLocalMachine = true )
{
  HKEY hklm_sft , hklm_cmp = NULL , retV = NULL;
  if ( bLocalMachine )
  {
    if ( ( GetWindowsVersionInfo() >= 6 ) && ( !IsRunAsAdmin() ) )
    {
      if ( !ElevatePrivilege() )
      {
        AfxMessageBox( _T( "You must run software as Administrator to set up software!" ) );
        return NULL;
      }
      retV = openthis();
      return retV;
    }
  }
  if ( RegOpenKey( bLocalMachine ?
    HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER , _T( "SOFTWARE" ) , &hklm_sft ) != ERROR_SUCCESS )
  {
    return NULL;
  }

  if ( !hklm_cmp )
  {
    if ( RegCreateKeyEx( hklm_sft , COMPANY_NAME_SEC , 0 , NULL , REG_OPTION_NON_VOLATILE ,
      KEY_ALL_ACCESS , NULL , &hklm_cmp , NULL ) != ERROR_SUCCESS )
    {
      ASSERT( FALSE );
      RegCloseKey( hklm_sft );
      return NULL;
    }
  }
  if ( RegCreateKey( hklm_cmp , APPLICATION_NAME_KEY , &retV ) != ERROR_SUCCESS )
  {
    ASSERT( FALSE );
    RegCloseKey( hklm_cmp );
    RegCloseKey( hklm_sft );
    return NULL;
  }
  RegCloseKey( hklm_cmp );
  RegCloseKey( hklm_sft );
  return retV;
}

// return value is length of evaluation key
__forceinline long is_evalkey_exists( bool * pbLocalMachine = NULL )
{
  DWORD retV = 0;
  HKEY hk = openthis( false ); // open HLCU
  if ( !hk )
  {
    hk = openthis( true ); // open HKLM
    if ( !hk )
      return false;
    DWORD type = REG_SZ;
    if ( RegQueryValueEx( hk , EVAL_KEYNAME , 0 , &type , NULL , &retV ) != ERROR_SUCCESS )
      retV = 0;
    else if ( pbLocalMachine )
      *pbLocalMachine = true ;
  }
  else // check in CURRENT_USER
  {
    DWORD type = REG_SZ;
    if ( RegQueryValueEx( hk , EVAL_KEY_NAME_CU , 0 , &type , NULL , &retV ) != ERROR_SUCCESS )
      retV = 0;
    else if ( pbLocalMachine )
      *pbLocalMachine = false ;
  }

  RegCloseKey( hk );
  return retV;
}

__forceinline int isinstalled() // == 1 => local machine
                                // == -1 => current user
                                // == 0 => doesn't exists
{
  HKEY  HKey = openthis( false );
  bool bLM = false ;
  if ( HKey )
  {
    RegCloseKey( HKey );
    if ( is_evalkey_exists( &bLM ) )
      return -1 ;
  }
  bLM = true ;
  HKey = openthis();
  if ( HKey )
  {
    if ( is_evalkey_exists( &bLM ) )
    {
      RegCloseKey( HKey );
      return 1 ;
    }
  }
  return 0;
}

__forceinline bool _read_eval_key( setupdata * d )
{
  int iRegMode = isinstalled() ;
  if ( !iRegMode ) 
    return false;
  bool bLocalMachine = true ;
  long v = is_evalkey_exists( &bLocalMachine );
  if ( v == 0 ) 
    return false;

  if ( bLocalMachine )
  {
    CString data;
    DWORD retV = v + 1;
    DWORD type = REG_SZ;
    LPBYTE dptr = ( LPBYTE ) data.GetBuffer( v + 1 );
    {
      HKEY hk = openthis();
      if ( RegQueryValueEx( hk , EVAL_KEYNAME , 0 , &type , dptr , &retV ) != ERROR_SUCCESS )
      {
        retV = 0;
      }
      RegCloseKey( hk );
      if ( retV == 0 )
        return false;
    }
    data.ReleaseBuffer();
    TCHAR tmp[ 3 ];
    tmp[ 2 ] = 0;
    LPBYTE db = ( LPBYTE ) d;
    LPBYTE eod = db + sizeof( setupdata ) * 2;
    TCHAR* be;
    while ( data.GetLength() != 0 )
    {
      tmp[ 0 ] = data[ 0 ];
      data = data.Mid( 1 );
      if ( data.GetLength() == 0 )
        return false;
      tmp[ 1 ] = data[ 0 ];
      data = data.Mid( 1 );
      *db = ( BYTE ) _tcstol( tmp , &be , 16 ); // tmp[2]==0 
      db++;
      if ( db >= eod )
        return false;
    }
  }
  else // Current user
  {
    SystemIDs LicenseInfo ;
    if ( LicenseInfo.LoadLicenseInfo() )
    {
      SystemIDs ThisComputerInfo ;
      ThisComputerInfo.GetThisSystemInfo() ;
      if ( ThisComputerInfo.m_Info.Compare( LicenseInfo.m_Info ) )
      {
        FXString Status = LicenseInfo.m_Info.m_SHLicenseStatus ;
        d->_eval_or_license =
          Status.Find( "Evaluation" ) >= 0 ? EOL_Evaluation : 
          LicenseInfo.m_Info.m_SHLicenseStatus.Find( "TimeLimited" ) >= 0 ? EOL_License : 
          LicenseInfo.m_Info.m_SHLicenseStatus.Find( "NoRestrictions" ) >= 0 ? EOL_NoRestriction :
          EOL_NoLicense ;
        strcpy_s( d->_registredto , (LPCTSTR)LicenseInfo.m_Info.m_Licensee ) ;
        g_SystemInfo.m_Licensee = LicenseInfo.m_Info.m_Licensee ;
        g_SystemInfo.m_LicenseDate = LicenseInfo.m_Info.m_LicenseDate ;
        switch ( d->_eval_or_license )
        {
          case EOL_Evaluation:
          case EOL_License:
          {
            FXString LicenseDate = LicenseInfo.m_Info.m_LicenseDate ;
            tm LicenseTM ;
            sscanf( ( LPCTSTR ) LicenseDate , "%4d%2d%2d" , 
              &LicenseTM.tm_year , &LicenseTM.tm_mon , &LicenseTM.tm_mday ) ;
            d->_stp_time = mktime( &LicenseTM ) ;

            time_t Now ;
            gmtime( &Now ) ;

            double dTimeDiffSeconds = difftime( Now , d->_stp_time ) ;
            int iCommaPos = (int)Status.Find( ',' ) ;
            if ( iCommaPos >= 0 )
            {
              d->_LicenseDuration_days = atoi( ( ( LPCTSTR ) Status ) + iCommaPos + 1 ) ;
              int iLicenseDurationSeconds = ( d->_LicenseDuration_days + 1 ) * 24 * 3600 ;
              if ( dTimeDiffSeconds > iLicenseDurationSeconds )
                return false ;
            }
            else
              return false ; // Number of days doesn't exists
          }
          break ; // OK evaluation is not finished
          case EOL_NoRestriction:
            break ;

        }
      }
    };
  }
  return true;
}

__forceinline void _encode( setupdata * d ) // For LocalMachine
{
  srand( GetTickCount() );
  unsigned seed = rand();
  seed = ( seed << 16 ) ^ rand();
  d->_seed = seed;

  d->_crc = 0;
  int i;
  LPBYTE p = ( LPBYTE ) &d->_stp_time;
  for ( i = 0; i < sizeof( time_t ); i++ )
  {
    d->_crc += p[ i ];
  }
  p = ( LPBYTE )&( d->_stp_time );
  int size = sizeof( setupdata ) - sizeof( unsigned );
  LPBYTE seedbytearray = ( LPBYTE )&( d->_seed );
  for ( i = 0; i < size; i++ )
  {
    p[ i ] = p[ i ] ^ seedbytearray[ i % sizeof( unsigned ) ] ^ secseq[ i%secseq_size ];
  }
}

__forceinline bool _decode( setupdata * d ) // For LocalMachine
{
  int i;

  LPBYTE p = ( LPBYTE )&( d->_stp_time );
  int size = sizeof( setupdata ) - sizeof( unsigned );
  LPBYTE seedbytearray = ( LPBYTE )&( d->_seed );
  for ( i = 0; i < size; i++ )
  {
    p[ i ] = p[ i ] ^ seedbytearray[ i % sizeof( unsigned ) ] ^ secseq[ i%secseq_size ];
  }
  p = ( LPBYTE ) &d->_stp_time;
  unsigned    crc = 0;
  for ( i = 0; i < sizeof( time_t ); i++ )
  {
    crc += p[ i ];
  }
 // TRACE(_T("UNIX time and date:\t\t\t%s"), ctime( &d->_stp_time));
  return ( crc == d->_crc );
}

__forceinline bool getowner( CString& owner )
{
  bool bLocalMachine = true ;
  long v = is_evalkey_exists( &bLocalMachine );
  if ( !v )
    return false ;

  setupdata sd;
  if ( !_read_eval_key( &sd ) ) 
    return false;
  if ( bLocalMachine )
  {
    if ( !_decode( &sd ) )
      return false;
  }
  owner = sd._registredto;
  return true;
}

__forceinline char isevaluation( bool& bLocalMachine , int * pRestDays = NULL )
{
  bLocalMachine = true ;
  long v = is_evalkey_exists( &bLocalMachine );
  if ( !v )
    return false ;

  setupdata sd;
  if ( !_read_eval_key( &sd ) )
    return false;
  if ( bLocalMachine )
  {
    if ( !_decode( &sd ) )
      return false;
  }
  if ( pRestDays )
    *pRestDays = sd._LicenseDuration_days ;

  return ( sd._eval_or_license );
}


__forceinline int daysremain()
{
  setupdata sd;
  if ( !_read_eval_key( &sd ) ) 
    return 0;
  if ( (sd._seed != 0) && !_decode( &sd ) ) 
    return 0;
  time_t time_now;
  time( &time_now );
  double dt = difftime( time_now , sd._stp_time ) / 3600;

  dt = ( int ) ( dt / 24 );
  int retV = ( int ) ( sd._LicenseDuration_days - dt );
  retV = ( retV < 0 ) ? 0 : retV;
  return retV;
}

__forceinline bool isexpired()
{
  return ( daysremain() < 0 );
}

__forceinline bool _create_eval_key( setupdata * d )
{
  bool retV = false;
  HKEY hk = openthis();
  if ( !hk ) 
    return false;

  CString regData;

  CString tmpS;
  LPBYTE db = ( LPBYTE ) d;
  for ( int i = 0; i < sizeof( setupdata ); i++ )
  {
    tmpS.Format( _T( "%02x" ) , db[ i ] );
    //regData+=itoa(db[i],buffer,16);
    regData += tmpS;
  }

  retV = RegSetValueEx( hk , EVAL_KEYNAME , 0 , REG_SZ , ( LPBYTE ) ( ( LPCTSTR ) regData ) , regData.GetLength() + 1 ) == ERROR_SUCCESS;

  //retV=RegSetValue(hk,EVAL_KEYNAME,REG_SZ, regData,regData.GetLength())==ERROR_SUCCESS;

  RegCloseKey( hk );
  return retV;
}

__forceinline bool _create_permanent_key( setupdata * d )
{
  bool retV = false;
  HKEY hk = openthis();
  if ( !hk )
    return false;
  RegCloseKey( hk );

  SystemIDs IDs ;
  IDs.m_Info.Copy( g_SystemInfo ) ;
  IDs.m_Info.m_Licensee = d->_registredto ;
  IDs.m_Info.m_SHLicenseStatus = "NoRestrictions" ;
  IDs.m_Info.m_LicenseDate = GetTimeStamp().Left( 8 ) ;
  retV = IDs.SaveSystemInfo( true ) ;

  return retV;
}

__forceinline bool register_eval( EvalOrLicense Mode = EOL_Evaluation )
{
  if ( isinstalled() ) 
    return false;
  bool bLocalMachine = true ;
  if ( is_evalkey_exists( &bLocalMachine ) )
    return false;

  HKEY HKey = createthis( false );
  if ( !HKey )
    return false ;
  bLocalMachine = false ;

  if ( bLocalMachine )
  {
    setupdata data;
    time( &data._stp_time );
    data._eval_or_license = ( char ) Mode ;
    srand( GetTickCount() );
    for ( int i = 0; i < sizeof( data._registredto ); i++ )
    {
      data._registredto[ i ] = ( char ) ( rand() & 0xFF );
    }
#define ARRSZ(x) (sizeof(x)/sizeof(x[0]))

    strcpy_s( data._registredto , ARRSZ( data._registredto ) , /*_T( */"Evaluation"/* )*/ );
    TRACE( _T( "UNIX time and date:\t\t\t%s" ) , ctime( &data._stp_time ) );
    _encode( &data );
    _create_eval_key( &data );
  }
  else
  {
    SystemIDs Info ;
    Info.GetThisSystemInfo() ;
    Info.m_Info.m_SHLicenseStatus = "Evaluation,30" ;
    Info.m_Info.m_LicenseDate = (LPCTSTR)(GetTimeStamp().Left(8)) ;
    Info.SaveSystemInfo( true ) ; // to the registry
  }
  RegCloseKey( HKey );
  return true;
}

__forceinline bool correct_eval( int idays )
{
  HKEY  HKey = openthis();

  setupdata data;
  if ( !_read_eval_key( &data ) )
  {
    printf( _T( "\nERROR: Can't read keys\n" ) ) ;
    return false ;
  }

  if ( ( data._seed != 0 ) && !_decode( &data ) ) 
  {
    printf( _T( "\nERROR: Decode Error\n" ) ) ;
    return false;
  }

  time_t TimeNow ;
  time( &TimeNow );

//   int iDaysBefore = ( idays < EVAL_PERIOD ) ? EVAL_PERIOD - idays : idays ;
  int iDaysBefore = idays - EVAL_PERIOD ;
  time_t CalculatedEvalBegin = TimeNow + iDaysBefore * 24 * 3600 ;
  data._stp_time = CalculatedEvalBegin ;

  data._eval_or_license = (char)((idays > EVAL_PERIOD) ? EOL_License : EOL_Evaluation) ;
  strcpy_s( data._registredto , ( LPCTSTR ) g_SystemInfo.m_Licensee ) ;

  srand( GetTickCount() );

#define ARRSZ(x) (sizeof(x)/sizeof(x[0]))

//   strcpy_s( data._registredto , ARRSZ( data._registredto ) , /*_T( */"Evaluation"/* )*/ );
  TRACE( _T( "UNIX time and date:\t\t\t%s" ) , ctime( &data._stp_time ) );
  _encode( &data );
  _create_eval_key( &data );
  RegCloseKey( HKey );
  printf( _T( "\nBegin was %d days before, the rest time is %d days\n" ) , iDaysBefore , idays) ;

  return true;
}


///// The new one
#define MAX_LICENSE_NMB 1000000
#define KEY_BYTELEN 10

#pragma pack(push)
#pragma pack(1)
typedef struct _tagRights
{
  unsigned short daytoactivate : 6;
  bool           activated : 1;
  unsigned       get();
  void           set( unsigned val );
}Rights;
#pragma pack(pop)

extern Rights theRights;

inline unsigned  Rights::get()
{
  return 0xFFFFFFFF;
}

inline void Rights::set( unsigned val )
{}

__forceinline void codedata( int seed , LPBYTE data , unsigned len )
{
  data[ 0 ] = ( BYTE ) ( seed & 0xFF );
  for ( unsigned i = 1; i < len; i++ )
    data[ i ] = data[ i ] ^ data[ 0 ] ^ secseq[ ( i + data[ 0 ] ) % secseq_size ];
}

__forceinline void decodedata( LPBYTE data , unsigned len )
{
  BYTE seed = data[ 0 ];
  data[ 0 ] = 0;
  for ( unsigned i = 1; i < len; i++ )
    data[ i ] = data[ i ] ^ seed^secseq[ ( i + seed ) % secseq_size ];
}

__forceinline bool  createproductkey( int LicenseID , LPBYTE idb , int idblen = KEY_BYTELEN )
{
  unsigned seed;
  if ( rand_s( &seed ) != S_OK ) return false;

  unsigned id = LicenseID;
  unsigned crc = 0;
  unsigned rights = theRights.get();
  memset( idb , 0 , idblen );
  for ( int i = 1; i < idblen; i++ ) // 0 element used for seed
  {
    if ( ( i % 2 == 0 ) || ( i > 6 ) )
    {
      if ( i != 6 )
      {
        idb[ i ] = ( BYTE ) ( rights & 0xFF );
        crc += idb[ i ];
        rights = ( rights >> 8 );
      }
    }
    else
    {
      idb[ i ] = ( BYTE ) ( id & 0xFF );
      crc += idb[ i ];
      id = ( id >> 8 );
    }
  }
  idb[ 6 ] = ( BYTE ) ( crc & 0xFF );
  codedata( seed , idb , idblen );
  return true;
}

// __forceinline bool  verify_key(LPCTSTR key, int& licenseNmb)
// {
//   BYTE idb[ KEY_BYTELEN + 1 ] = { 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 
//     0xFF , 0xFF , 0xFF , 0xFF , 0xFF }; // one byte more for overwrting by sscanf (reads shots)
//     if (sscanf(key,"%02hX%02hX-%02hX%02hX-%02hX%02hX-%02hX%02hX-%02hX%02hX",&idb[0],&idb[1],&idb[2],&idb[3],&idb[4],&idb[5],&idb[6],&idb[7],&idb[8],&idb[9])!=KEY_BYTELEN) return false;
//     decodedata(idb,KEY_BYTELEN);
//     licenseNmb=0;
//     unsigned rights=0;
//     int crc=0;
//     for (int i=KEY_BYTELEN-1; i>0; i--) // 0 element used for seed
//     {
//         if ((i%2==0) || (i>6))
//         {
//             if (i!=6)
//             {
//                 rights=(rights<<8);
//                 rights|=idb[i];
//                 crc+=idb[i];
//             }
//         }
//         else
//         {
//             licenseNmb=(licenseNmb<<8);
//             licenseNmb|=idb[i];
//             crc+=idb[i];
//         }
//     }
//     return (((crc&0xFF)==idb[6]) && (licenseNmb<MAX_LICENSE_NMB));
// }


#endif
#pragma once

#include <fxfc\fxfc.h>
//#include <gadgets\gadbase.h>

typedef void( *ProcessMessage )( void * pMsg , FXString& Output );


class RTLogMsg
{
public:
  FILETIME m_FileTimeTS;
  char m_Data[ 100 ];
  char   m_Addition[ 50 ];

  RTLogMsg( void * pData = NULL , FXString * pAddition = NULL )
  {
    GetSystemTimeAsFileTime( &m_FileTimeTS );
    if ( pData )
      FXstrcpy_s( m_Data , sizeof( m_Data ) , ( LPCTSTR ) pData );
    else
      m_Data[ 0 ] = 0;
    if ( pAddition )
      FXstrcpy_s( m_Addition , sizeof( m_Addition ) , ( LPCTSTR ) ( *pAddition ) );
    else
      m_Addition[ 0 ] = 0;
  }
  RTLogMsg( LPCTSTR pData , LPCTSTR pAddition = NULL )
  {
    GetSystemTimeAsFileTime( &m_FileTimeTS );
    FXstrcpy_s( m_Data , sizeof( m_Data ) , pData );
    if ( pAddition )
      FXstrcpy_s( m_Addition , sizeof( m_Addition ) , pAddition );
    else
      m_Addition[ 0 ] = 0;
  }
  FXString GetTSAsString( LPCTSTR pPrefix = NULL )
  {
    FXString retV;
    SYSTEMTIME stime;
    //structure to store system time (in usual time format)
    FILETIME ltime;    FileTimeToLocalFileTime( &m_FileTimeTS , &ltime );//convert to local time and store in ltime
    FileTimeToSystemTime( &ltime , &stime );//convert in system time and store in stime

    retV.Format( "%s%d-%d-%d_%d-%d-%d.%d" , pPrefix ,
      stime.wYear , stime.wMonth , stime.wDay ,
      stime.wHour , stime.wMinute , stime.wSecond , stime.wMilliseconds );
    return retV;
  }

  FXString GetMsgForLog()
  {
    FXString ForLog;
    ForLog.Format( _T( "%s %s: %s\n" ) ,
      ( LPCTSTR ) GetTSAsString() , m_Addition , m_Data );
    return ForLog;
  }
} ;

typedef FXArray<RTLogMsg> RTLogDataArray;


class FXFC_EXPORT FXSimpleLogToFile
{
  HANDLE m_hEvNewMessage;
  bool   m_bStopLogging;
  FXString m_Name;
  RTLogDataArray m_Queue;
  FXLockObject m_QueueProtect;
  FXString m_DirectoryName;
  FXString m_FileName;
  FXString m_Prefix ;
  int      m_Indexer;
  HANDLE   m_hThread;
  DWORD    m_ThreadID;

public:
  FXSimpleLogToFile( const char * pDirName = NULL , const char * pLogName = NULL );
  ~FXSimpleLogToFile();
  int AddMsg( LPCTSTR pMsg , LPCTSTR pAddition = NULL );
  int AddFormattedMsg( LPCTSTR pMsgFormat , ... ) ;
  void Destroy();
  void SetLogPrefix( LPCTSTR Prefix )
  {
    m_Prefix = Prefix;
  }
  void SetLogDirectory( LPCTSTR Directory )
  {
    m_DirectoryName = Directory;
  }
  void SetFile( const char * pDirName , const char * pLogName );

private:
  void		    Work();
  static void _workerFunc( void* pLogger );

};

class RTLogMsgEx
{
public:
  double m_dTime;
  void * m_pData;
  char   m_Addition[ 50 ];

  RTLogMsgEx( void * pData = NULL , FXString * pAddition = NULL )
  {
    m_dTime = GetHRTickCount();
    m_pData = pData;
    if ( pAddition )
    {
      int iLen = ( int ) pAddition->GetLength();
      if ( iLen < sizeof( m_Addition ) - 1 )
        strcpy_s( m_Addition , sizeof( m_Addition ) , ( const char* ) ( ( LPCTSTR ) ( *pAddition ) ) );
      else
      {
        memcpy( m_Addition , ( LPCTSTR ) ( *pAddition ) , sizeof( m_Addition ) - 1 );
        m_Addition[ sizeof( m_Addition ) - 1 ] = 0;
      }
    }
  }
};

typedef FXArray<RTLogMsgEx> RTLogExDataArray;

class FXFC_EXPORT FXLoggerToFile
{
  HANDLE m_hEvNewMessage;
  bool   m_bStopLogging;
  FXString m_Name;
  RTLogExDataArray m_Queue;
  FXLockObject m_QueueProtect;
  ProcessMessage m_pConverter;
  FXString m_DirectoryName;
  FXString m_FileName;
  FXString m_Prefix ;
  DWORD    m_Indexer;
  HANDLE   m_hThread;
  DWORD    m_ThreadID;

public:
  FXLoggerToFile( const char * pLogPrefix = NULL , ProcessMessage pConverter = NULL );
  ~FXLoggerToFile();
  int AddMsg( void * pMsg , FXString * pAddition = NULL );
  void Destroy();
  void SetConverter( ProcessMessage pConverter )
  {
    m_pConverter = pConverter;
  }
  void SetLogPrefix( LPCTSTR Prefix )
  {
    m_Prefix = Prefix;
  }
  void SetLogDirectory( LPCTSTR Directory )
  {
    m_DirectoryName = Directory;
  }

private:
  void		    Work();
  static void _workerFunc( FXLoggerToFile* pLogger );

};

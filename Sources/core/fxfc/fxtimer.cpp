// TimerNew.cpp: implementation of the FXTimer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <fxfc\fxfc.h>
//#include <helpers\LastErr2mes.h>
#include <Mmsystem.h>

#pragma comment( lib,"Winmm.lib")

#define THIS_MODULENAME "FXTimer"

//extern void WriteLog(LPCTSTR lpszFormat, ...);
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
double GetHRTickCount()
{
  LARGE_INTEGER pc;
  QueryPerformanceCounter( &pc );
  return ( ( 1000.0*pc.QuadPart ) / pf );
}

int AlarmCntr = 0;

// static  __int64 get_int_time()
// {
//   __asm RDTSC
// #pragma warning (disable: 4035)
// } ;

DWORD WINAPI Alarm_ProcFunc( void* pThis )
{
  return ( ( FXTimer* )pThis )->AlarmLoop();
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FXTimer::FXTimer( LPCTSTR Name )
{
  m_Name = Name;
  m_Alarm = false;
  m_bPause = false;
  m_Interval = 40;
  m_hAlarmThread = NULL;
  m_hExit = CreateEvent( NULL , FALSE , FALSE , NULL );
  /// Init hi-def timer
//   int accurate = 0;
//   DWORD ValType = REG_DWORD , Value , BufSize = 4;
//   HKEY MyKey;
//   long RegOpenResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
//     "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0" ,//\\~MHz",
//     0 , KEY_QUERY_VALUE , &MyKey );
//   if ( RegOpenResult != ERROR_SUCCESS )
//   {
//     char buf[ 300 ];
//     FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM , NULL ,
//       RegOpenResult , NULL , buf , 300 , NULL );
//     printf( "Error: %s -- Unknown processor frequency\n" , buf );
//   }
//   else
//   {
//     long RegQueryResult = RegQueryValueEx( MyKey , "~MHz" , 0 , &ValType , ( unsigned char* )&Value , &BufSize ) ;
//     if ( RegQueryResult == ERROR_SUCCESS )
//     {
//       if ( BufSize == 4 )
//       {
//         accurate = 1;
//         m_BaseFreq = 1000. / ( double )( Value * 1e6 ); // time of  clock ( msec) ( 1e6 == 1000000 )
//       }
//     }
//     else
//     {
//       char buf[ 300 ];
//       FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM , NULL ,
//         RegQueryResult , NULL , buf , 300 , NULL );
//       printf( "Error: %s Processor frequency is unknown\n" , buf );
//     }
//     RegCloseKey( MyKey );
//   }
  LARGE_INTEGER Freq ;
  if ( QueryPerformanceFrequency( &Freq ) )
    m_BaseFreq = 1000. / ( double )Freq.QuadPart;
  else
  {
    ASSERT( 0 ) ;
    m_BaseFreq = 1000. / ( 3.579545e6 ) ;  // subcarrier NTSC
  }
  m_evResetPause = ::CreateEvent( NULL , FALSE , FALSE , NULL );
  m_evDelay = ::CreateEvent( NULL , FALSE , FALSE , NULL );
}

FXTimer::~FXTimer()
{
  AlarmOff();
  ResetPause();
  while ( m_bPause )
    Sleep( 10 );
  CloseHandle( m_evResetPause );
  CloseHandle( m_evDelay );
}

void FXTimer::AlarmOn()
{
  if ( m_Alarm ) return;
  m_Alarm = true;
  m_hAlarmThread = ::CreateThread( NULL , 0 , Alarm_ProcFunc , this , CREATE_SUSPENDED , NULL );
  SetThreadPriority( m_hAlarmThread , THREAD_PRIORITY_HIGHEST );
  m_StartTimeID = get_int_time();
  DWORD res = ResumeThread( m_hAlarmThread );
  if ( res == 0xFFFFFFFF )
  {
    TRACE( "Error: AlarmOn" );
    ASSERT( FALSE );
  }
}

void FXTimer::AlarmOff()
{
  m_Alarm = false;
  if ( m_hAlarmThread )
  {
    SetEvent( m_hExit );
    ::WaitForSingleObject( m_hAlarmThread , INFINITE );
    CloseHandle( m_hAlarmThread );
    m_hAlarmThread = NULL;
  }
}

#ifdef _DEBUG
int		a=0;
DWORD	dw;
#endif

void FXTimer::SetAlarmFreq( double Freq )
{
  bool alarmstate = m_Alarm;
  if ( alarmstate )
    AlarmOff();
  if ( Freq == 0 )
    m_Interval = 40;
  else
    m_Interval = ( DWORD )( 1000.0 / Freq );
  if ( alarmstate )
    AlarmOn();
}
  //' bWaitTimeOut always true
VOID CALLBACK PeriodEllapsed( LPVOID Handle , BOOLEAN bWaitTimeOut )
{
  SetEvent( ( HANDLE )Handle ) ;
}

int FXTimer::AlarmLoop()
{
  HANDLE hE = CreateEvent( NULL , FALSE , FALSE , NULL );
  HANDLE pEvents[] = { hE , m_hExit };
  DWORD cEvents = sizeof( pEvents ) / sizeof( HANDLE );
  BOOL bCreated = CreateTimerQueueTimer(
    &m_hTimer , NULL , PeriodEllapsed , (LPVOID)hE , m_Interval ,
    m_Interval , WT_EXECUTEINTIMERTHREAD );
//   MMRESULT tm = timeSetEvent( m_Interval , 0 , ( LPTIMECALLBACK )hE , NULL , TIME_CALLBACK_EVENT_PULSE | TIME_PERIODIC );
  if ( bCreated )
  {
#ifdef _DEBUG 
    dw=GetTickCount(); 
#endif
    while ( m_Alarm )
    {
      if ( ::WaitForMultipleObjects( cEvents , pEvents , FALSE , 2 * m_Interval ) == WAIT_OBJECT_0 )
        OnAlarm( ( DWORD )( ( get_int_time() - m_StartTimeID )*m_BaseFreq ) );
      //Real time from start, in ms.
    }
//     timeKillEvent( tm );
    HANDLE hEvDeleteTimer = CreateEvent( NULL , FALSE , FALSE , NULL );
    DeleteTimerQueueTimer( NULL , m_hTimer , hEvDeleteTimer ) ;
    if ( WaitForSingleObject( hEvDeleteTimer , m_Interval + 1000 ) != WAIT_OBJECT_0 )
    {
      ASSERT( 0 ) ;
    }
    m_hTimer = NULL ;
  }
  CloseHandle( hE );
  TRACE( "+++ Alarm loop exits\n" );
  return 0;
}

bool FXTimer::Pause( DWORD delay )
{
  m_bPause = true;
  HANDLE pEvents[] = { m_evDelay , m_evResetPause , m_hExit };
  DWORD cEvents = sizeof( pEvents ) / sizeof( HANDLE );
//  MMRESULT tm = timeSetEvent( delay , 0 , ( LPTIMECALLBACK )m_evDelay , NULL , TIME_CALLBACK_EVENT_PULSE | TIME_ONESHOT );
  BOOL bCreated = CreateTimerQueueTimer(
    &m_hTimer , NULL , PeriodEllapsed , ( LPVOID )m_evDelay , delay ,
    NULL , WT_EXECUTEINTIMERTHREAD | WT_EXECUTEONLYONCE  );
  if ( bCreated )
  {
    bool bResult = ( ::WaitForMultipleObjects( cEvents , pEvents , FALSE , 2 + delay ) == WAIT_OBJECT_0 );
//     timeKillEvent( tm );
    HANDLE hEvDeleteTimer = CreateEvent( NULL , FALSE , FALSE , NULL );
    DeleteTimerQueueTimer( NULL , m_hTimer , hEvDeleteTimer ) ;
    if ( WaitForSingleObject( hEvDeleteTimer , m_Interval + 1000 ) != WAIT_OBJECT_0 )
    {
      ASSERT( 0 ) ;
    }
    m_hTimer = NULL ;
    m_bPause = false;
    return bResult;
  }
  m_bPause = false;
  return false;
}

void FXTimer::ResetPause()
{
  if ( m_bPause )
    ::SetEvent( m_evResetPause );
}

void FXTimer::OnAlarm( DWORD TimeID )
{
#ifdef _DEBUG
  a++;
  if ((DWORD)(a*m_Interval)-(GetTickCount()-dw)!=0)
  {
    TRACE("+++ Timer error: %d \n",(DWORD)(a*m_Interval)-(GetTickCount()-dw)); 
  }
#endif
}


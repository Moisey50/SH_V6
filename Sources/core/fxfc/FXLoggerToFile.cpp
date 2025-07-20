#include "stdafx.h"
#include <fxfc/FXRegistry.h>
#include "fxfc/FXLoggerToFile.h"
//#include <gadgets/gadbase.h>


void FXSimpleLogToFile::_workerFunc( void * pData )
{
  FXSimpleLogToFile* pLogger = ( FXSimpleLogToFile* ) pData ;
  pLogger->Work();
};

void FXSimpleLogToFile::SetFile( const char * pDirName , const char * pLogName )
{

  if ( pDirName && *pDirName && pLogName && *pLogName )
  {
    if ( !m_bStopLogging )
      Destroy() ;
    SetLogDirectory( pDirName ) ;
    SetLogPrefix( pLogName ) ;

    m_hEvNewMessage = CreateEvent( NULL , FALSE , FALSE , NULL );
    m_Indexer = -1;

    m_hThread = ::CreateThread( NULL , NULL , ( LPTHREAD_START_ROUTINE ) _workerFunc ,
      this , 0 , &m_ThreadID );
    if ( !m_hThread )
    {
      FxSendLogMsg( MSG_ERROR_LEVEL , _T( "FXSimpleLogToFile" ) , 0 , _T( "Can't create Log thread" ) );
    }
    else
    {
      //     ResumeThread( m_hThread );
      //     ::WaitForSingleObject(m_hEvNewMessage, INFINITE
    }
    TRACE( "++++++++++++++++++++ FXSimpleLogToFile: Thread created 0x%x (id = %d)\n" , m_hThread ,
      m_ThreadID );

  }
  else
    TRACE( "++++++++++++++++++++ FXSimpleLogToFile: Empty Dir name and/or Log name\n" ) ;
}

FXSimpleLogToFile::FXSimpleLogToFile( const char * pDirName , const char * pLogName )
{
  m_bStopLogging = true ;
  SetFile( pDirName , pLogName ) ;
}

int FXSimpleLogToFile::AddMsg(LPCTSTR pMsg , LPCTSTR pAddition )
{
  if ( !m_bStopLogging )
  {
    m_QueueProtect.Lock();
    RTLogMsg TimedMsg( pMsg , pAddition );
    int iLen = ( int ) m_Queue.Add( TimedMsg );
    m_QueueProtect.Unlock();
    SetEvent( m_hEvNewMessage );
    return iLen;
  }
  return 0;
}

int FXSimpleLogToFile::AddFormattedMsg( LPCTSTR pMsgFormat , ... )
{
  if ( !m_bStopLogging )
  {
    FXString Msg ;
    va_list argList;
    va_start( argList , pMsgFormat );
    Msg.FormatV( pMsgFormat , argList );
    va_end( argList );

    RTLogMsg TimedMsg( Msg );
    m_QueueProtect.Lock();
    int iLen = ( int ) m_Queue.Add( TimedMsg );
    m_QueueProtect.Unlock();
    SetEvent( m_hEvNewMessage );
    return iLen;
  }
  return 0;
}

void FXSimpleLogToFile::Destroy()
{
  m_bStopLogging = true;
  SetEvent( m_hEvNewMessage );
  WaitForSingleObject( m_hThread , 500 );
}

FXSimpleLogToFile::~FXSimpleLogToFile()
{
  Destroy();
}

void FXSimpleLogToFile::Work()
{
  //OnThreadStart();

  TRACE( " >>>>>>>>>>>>>>>>>>>>> %s: FXSimpleLogToFile::Work()\n" , ( LPCTSTR ) m_Name );

  SetEvent( m_hEvNewMessage ); // Signal to constructor about thread is working
  m_bStopLogging = false;
  while ( !m_bStopLogging )
  {
    if ( ::WaitForSingleObject( m_hEvNewMessage , INFINITE ) == WAIT_OBJECT_0
      && !m_bStopLogging )
    {
      if ( m_Queue.GetCount() )
      {
        if ( m_FileName.IsEmpty() )
        {
          m_FileName = GetTimeAsString_ms( ( LPCTSTR ) m_Prefix , m_Indexer );
          m_FileName += _T( ".log" );
        }

        RTLogDataArray TmpQueue;
        m_QueueProtect.Lock();
        TmpQueue.Copy( m_Queue );
        m_Queue.RemoveAll();
        m_QueueProtect.Unlock();

        if ( m_DirectoryName.IsEmpty() || FxVerifyCreateDirectory( m_DirectoryName ) )
        {
          FXString FileName = ( !m_DirectoryName.IsEmpty() ) ?
            ( m_DirectoryName + _T( '\\' ) ) + m_FileName : m_FileName;

          CFileException e;
          CFile LogFile;
          if ( !LogFile.Open( FileName ,
            CFile::modeCreate | CFile::modeNoTruncate /*| CFile::typeText*/
            | CFile::modeWrite | CFile::shareDenyWrite | CFile::osWriteThrough , &e ) )
          {
            TCHAR szError[ 1024 ];
            e.GetErrorMessage( szError , 1024 );
            FxSendLogMsg( MSG_ERROR_LEVEL , _T( "FXSimpleLogToFile" ) , 0 , szError );
          }
          else
          {
            if ( LogFile.GetLength() )
              LogFile.SeekToEnd();
            FXString ForLog , Addition , Full;

            for ( int i = 0; i < ( int ) TmpQueue.GetCount(); i++ )
              Full += TmpQueue[i].GetMsgForLog();

            if ( !Full.IsEmpty() )
            {
              int iLen = ( int ) Full.GetLength();
              LogFile.Write( ( LPCTSTR ) Full , iLen );
            }
            LogFile.Close();
          }
        }
      }
    }
  }

  //OnThreadEnd();
  TRACE( " <<<<<<<<<<<<<<<<<<<<<<<<<<<< FXSimpleLogToFile THREAD %s EXITS!\n" , ( LPCTSTR ) m_Name );
}




void FXLoggerToFile::_workerFunc(FXLoggerToFile* pData)
{
  FXLoggerToFile* pLogger = ( FXLoggerToFile* )pData ;
  pLogger->Work();
};


FXLoggerToFile::FXLoggerToFile(LPCTSTR pLogPrefix, ProcessMessage pConverter)
{
  FXRegistry Reg( "TheFileX\\SHStudio" ) ;

  CString PinLogDirectory , PinLogPrefix ;
  PinLogDirectory = ( LPCTSTR ) Reg.GetRegiString( "PinLog" , "PinLogFileDir" , "F:\\SHLogs" );
  PinLogPrefix = ( LPCTSTR ) Reg.GetRegiString( "PinLog" , "PinLogFilePrefix" , "PinLog_" );
  SetLogDirectory( PinLogDirectory ) ;
  SetLogPrefix( PinLogPrefix ) ;

  m_hEvNewMessage = CreateEvent(NULL, FALSE, FALSE, NULL);
  m_Prefix = pLogPrefix;
  m_pConverter = pConverter;
  m_Indexer = -1;

  m_hThread = ::CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)_workerFunc,
    this, 0, &m_ThreadID);
  if (!m_hThread)
  {
    FxSendLogMsg(MSG_ERROR_LEVEL, _T("FXLoggerToFile"), 0, _T("Can't create Log thread"));
  }
  else
  {
    //     ResumeThread( m_hThread );
    //     ::WaitForSingleObject(m_hEvNewMessage, INFINITE
  }
  TRACE("++++++++++++++++++++ FXLoggerToFile: Thread created 0x%x (id = %d)\n", m_hThread,
    m_ThreadID);
}


FXLoggerToFile::~FXLoggerToFile()
{
  Destroy();
}

void FXLoggerToFile::Work()
{
  //OnThreadStart();

  TRACE(" >>>>>>>>>>>>>>>>>>>>> %s: FXLoggerToFile::Work()\n", (LPCTSTR)m_Name);

  SetEvent(m_hEvNewMessage); // Signal to constructor about thread is working
  m_bStopLogging = false;
  while (!m_bStopLogging)
  {
    if (::WaitForSingleObject(m_hEvNewMessage, INFINITE) == WAIT_OBJECT_0
      && !m_bStopLogging)
    {
      if (m_Queue.GetCount())
      {
        if (m_FileName.IsEmpty())
        {
          m_FileName = GetTimeAsString_ms((LPCTSTR)m_Prefix, m_Indexer);
          m_FileName += _T(".log");
        }

        RTLogExDataArray TmpQueue;
        m_QueueProtect.Lock();
        TmpQueue.Copy(m_Queue);
        m_Queue.RemoveAll();
        m_QueueProtect.Unlock();

        if (m_DirectoryName.IsEmpty() || FxVerifyCreateDirectory(m_DirectoryName))
        {
          FXString FileName = (!m_DirectoryName.IsEmpty()) ?
            (m_DirectoryName + _T('\\')) + m_FileName : m_FileName;

          CFileException e;
          CFile LogFile;
          if (!LogFile.Open(FileName,
            CFile::modeCreate | CFile::modeNoTruncate /*| CFile::typeText*/
            | CFile::modeWrite | CFile::shareDenyWrite | CFile::osWriteThrough, &e))
          {
            TCHAR szError[1024];
            e.GetErrorMessage(szError, 1024);
            FxSendLogMsg(MSG_ERROR_LEVEL, _T("FXLoggerToFile"), 0, szError);
          }
          else
          {
            if (LogFile.GetLength())
              LogFile.SeekToEnd();
            FXString ForLog, Addition, Full;

            for (int i = 0; i < (int)TmpQueue.GetCount(); i++)
            {
              if (m_pConverter)
              {
                m_pConverter(TmpQueue[i].m_pData, Addition);
                ForLog.Format(_T("%.3f:%s - %s\n"),
                  TmpQueue[i].m_dTime, TmpQueue[i].m_Addition,
                  (LPCTSTR)Addition);
                Addition.Empty();
              }
              else
              {
                ForLog.Format(_T("%.3f %s\n"),
                  TmpQueue[i].m_dTime, TmpQueue[i].m_Addition,
                  (LPCTSTR)(TmpQueue[i].m_pData));
                delete (LPCTSTR)TmpQueue[i].m_pData;
              }
              Full += ForLog;
            }

            if (!Full.IsEmpty())
            {
              int iLen = (int)Full.GetLength();
              LogFile.Write((LPCTSTR)Full, iLen);
            }
            LogFile.Close();
          }
        }
      }
    }
  }
  m_QueueProtect.Lock();
  if (!m_pConverter)
  {
    for (int i = 0; i < m_Queue.GetCount(); i++)
      delete (LPCTSTR)m_Queue[i].m_pData;
  }
  else
  {

  }
  m_QueueProtect.Unlock();
  //OnThreadEnd();
  TRACE(" <<<<<<<<<<<<<<<<<<<<<<<<<<<< FXLoggerToFile THREAD %s EXITS!\n", (LPCTSTR)m_Name);
}

int FXLoggerToFile::AddMsg(void * pMsg, FXString * pAddition)
{
  if (!m_bStopLogging)
  {
    m_QueueProtect.Lock();
    RTLogMsgEx TimedMsg(pMsg, pAddition);
    int iLen = (int)m_Queue.Add(TimedMsg);
    m_QueueProtect.Unlock();
    SetEvent(m_hEvNewMessage);
    return iLen;
  }
  return 0;
}
void FXLoggerToFile::Destroy()
{
  m_bStopLogging = true;
  SetEvent(m_hEvNewMessage);
  WaitForSingleObject(m_hThread, 500);
}

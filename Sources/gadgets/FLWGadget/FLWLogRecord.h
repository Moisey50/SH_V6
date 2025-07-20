#pragma once

#if !defined(FLW_LOG_RECORD_INCLUDED)
#define FLW_LOG_RECORD_INCLUDED

#include <iostream>
#include <fstream>

#ifndef ARRSZ
#define ARRSZ(x) (sizeof(x)/sizeof(x[0]))
#endif

#define FLW_LOG_SIZE 100000
class FLWFrameLogRecord
{
public:
  DWORD m_FrameID ;
  int   m_FrameType ;
  double m_dFrameArriveTime_ms ;

  FLWFrameLogRecord( const CDataFrame * pFrame = NULL )
  {
    if ( pFrame )
    {
      m_FrameID = pFrame->GetId() ;
      m_FrameType = pFrame->IsContainer() ? -1 : pFrame->GetDataType() ;
      m_dFrameArriveTime_ms = GetHRTickCount() ;
    }
    else
      memset( this , 0 , sizeof( *this ) ) ;
  }
  FXString ToString( double dOriginTime_ms = 0. )
  {
    FXString Out ;
    Out.Format( "%8d %10s %12.3f" , m_FrameID ,
      m_FrameType == -1 ? "Container" : Tvdb400_TypeToStr( m_FrameType ) ,
      m_dFrameArriveTime_ms - dOriginTime_ms ) ;
    return Out ;
  }
  LPCTSTR GetCaption()
  {
    return "    ID      DataType     Time" ;
  }
};

class FLWLog
{
public:
  size_t          m_LogIndex ;
  size_t          m_LogSize ;
  FXString        m_LogFileName ;
  BOOL            m_bDoLog ;
  FLWFrameLogRecord * m_pLog ;

  FLWLog()
  {
    m_LogIndex = 0 ;
    m_LogSize = 0 ;
    m_bDoLog = FALSE ;
    m_pLog = 0 ;
  }
  virtual ~FLWLog() 
  {
    if ( m_pLog )
    {
      delete[] m_pLog ;
      m_pLog = NULL ;
    }
  }

  size_t Reset( LPCTSTR FileName , size_t LogSize , BOOL bDoLog )
  {
    if ( m_LogIndex && !m_LogFileName.IsEmpty() )
      SaveLogAndClear( true , true ) ;

    if ( m_pLog )
    {
      delete[] m_pLog ;
      m_pLog = NULL ;
    }
    m_LogSize = LogSize ;
    if ( LogSize )
    {
      m_LogFileName = FileName ;
      m_pLog = new FLWFrameLogRecord[ m_LogSize ] ;
      if ( m_pLog )
        memset( m_pLog , 0 , sizeof( m_pLog[ 0 ] ) * m_LogSize ) ;
      m_LogIndex = 0 ;
    }
    return m_LogSize ;
  }

  size_t SaveLogAndClear( bool bAsText , bool bAsBinary )
  {
    if ( m_LogSize && m_LogIndex && m_pLog && !m_LogFileName.IsEmpty() )
    {
      FXString FileName = m_LogFileName ;
      FileName += ".log" ;
      if ( bAsText )
      {
        ofstream ostr( ( LPCTSTR ) FileName , ios_base::out ) ;
        if ( ostr.is_open() )
        {
          ostr << m_pLog->GetCaption() << "\n" ;
          if ( m_LogIndex >= m_LogSize - 1 )
            m_LogIndex = m_LogSize ;
          for ( size_t i = 0 ; i < m_LogIndex ; i++ )
            ostr << ( LPCTSTR ) ( ( m_pLog + i )->ToString() ) << "\n" ;
          ostr.close() ;
        }
      }
      FileName += ".bin" ;
      ofstream ostrbin( ( LPCTSTR ) FileName , ios_base::out ) ;
      if ( ostrbin.is_open() )
      {
        if ( m_LogIndex >= m_LogSize - 1 )
          m_LogIndex = m_LogSize ;
        ostrbin.write( ( const char* ) m_pLog , sizeof( FLWFrameLogRecord ) * m_LogIndex ) ;
        ostrbin.close() ;
      }
      m_LogIndex = 0 ;
      return 1 ;
    }
    return 0 ;
  }
  size_t GetLogIndex() { return m_LogIndex; }

  FLWFrameLogRecord& GetLogItem( size_t Index )
  {
    if ( Index < m_LogSize ) 
      return m_pLog[ Index ] ;
    else
      return m_pLog[ m_LogSize - 1 ] ;
  }
  BOOL SetNextLogItem( const CDataFrame * pFrame )
  {
    if ( m_bDoLog )
    {
      FLWFrameLogRecord& NextRec = m_pLog[ m_LogIndex++ ] ;
      NextRec.m_FrameID = pFrame->GetId() ;
      NextRec.m_FrameType = pFrame->GetDataType() ;
      NextRec.m_dFrameArriveTime_ms = GetHRTickCount() ;
      if ( m_LogIndex >= m_LogSize )
      {
        m_LogIndex-- ;
        return FALSE ;
      }
    }
    return TRUE ;
  }
};

#endif  // FLW_LOG_RECORD_INCLUDED
// FLWRenderGadget.cpp: implementation of the FLWRender class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FLWRenderGadget.h"
#include "FlwRenderStp.h"
#include "gadgets/textframe.h"
#include "helpers/FramesHelper.h"
#include <iostream>
#include <fstream>
#include <psapi.h>

#define THIS_MODULENAME "FLWRender"
#define FLW_NOERROR 0
#define FLW_CANTOPENFILE 1

IMPLEMENT_RUNTIME_GADGET_EX( FLWRender , CRenderGadget , "Files.Render" , TVDB400_PLUGIN_NAME );

inline CString generateName( LPCTSTR fmt )
{
  CString retV = fmt;
  CTime t = CTime::GetCurrentTime();
  CString tmpS;
  int pos;
  while ( ( pos = retV.Find( '%' ) ) >= 0 )
  {
    tmpS = retV.Mid( pos , 2 );
    if ( tmpS.GetLength() == 2 )
    {
      retV.Delete( pos , 2 );
      tmpS = t.Format( tmpS );
      // remove forbidden chars
      tmpS.Replace( ':' , '-' );
      tmpS.Replace( '/' , '.' );
      tmpS.Replace( '\\' , ' ' );
      tmpS.Replace( '*' , '+' );
      tmpS.Replace( '|' , 'I' );
      tmpS.Replace( '?' , ' ' );
      tmpS.Replace( '>' , ' ' );
      tmpS.Replace( '<' , ' ' );
      tmpS.Replace( '"' , '\'' );

      retV.Insert( pos , tmpS );
    }
  }
  return retV;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#pragma warning( disable : 4355)
FLWRender::FLWRender() :
  m_LastError( FLW_NOERROR ) ,
  m_Archive( this )
{
  m_FileNameTemplate = "C:\\%x %X.flw";
  SetMonitor( SET_INPLACERENDERERMONITOR );		// Always built-in (not a floating window)
  m_pInput = new CInputConnector( nulltype );
  m_pOutput = new COutputConnector( text ) ;
  m_pInput->SetQueueSize( 100 ) ;
  m_SetupObject = new FLWRenderStp( this , NULL );
  m_bDoLog = false ;
  m_iNBuffers = 0 ;
  m_iBufferSize_KB = 0 ;
  Resume();
}

void FLWRender::ShutDown()
{
  CRenderGadget::ShutDown();
  if ( m_pInput )
  {
    delete m_pInput;
    m_pInput = NULL;
  }
  if ( m_pOutput )
  {
    delete m_pOutput ;
    m_pOutput = NULL ;
  }
}

void FLWRender::Render( const CDataFrame* pDataFrame )
{
  CString fName;
  if ( ( !m_Archive.IsOpen() ) && ( !Tvdb400_IsEOS( pDataFrame ) ) )
  {
    fName = generateName( m_FileNameTemplate );
    if ( m_Archive.OpenWrite( fName ) )
    {
      SENDINFO_1( "Open file '%s' for writing" , fName );
      m_Log.Reset( ( fName + ( LPCTSTR ) GetTimeStamp() ) + ".log" ,
        FLW_LOG_SIZE , m_bDoLog ) ;
      m_iNOmittedFrames = 0 ;
      m_dLastErrorIndicatedTime = 0. ;
    }
    else
      m_Log.Reset( "" , 0 , FALSE ) ;
  }
  if ( !m_Archive.IsOpen() )
  {
    if ( m_LastError != FLW_CANTOPENFILE )
      SENDERR_1( "Can't open file '%s' for rendering" , fName );
    m_LastError = FLW_CANTOPENFILE;
    return;
  }
  HANDLE hProcess = GetCurrentProcess() ;
  PROCESS_MEMORY_COUNTERS pmc;
  
  if ( GetProcessMemoryInfo( hProcess , &pmc , sizeof( pmc ) ) )
  {
    if ( pmc.WorkingSetSize < 1024 * 1024 * 1200 ) // 1.2 GB
    {
      m_LastError = FLW_NOERROR;
      if ( !m_Archive.WriteFrame( pDataFrame ) )
      {
        TRACE( "Can't serialize frame\n" );
      }
      else
      {
        m_Log.SetNextLogItem( pDataFrame ) ;
      }
//       ASSERT( ( m_Archive.GetSerializationQueueLength() < 20 )
//         && ( m_Archive.GetWritingQueueLength() < 20 ) ) ;
      if ( Tvdb400_IsEOS( pDataFrame ) )
      {
        m_Archive.Close();
        FXString FileName = m_Archive.GetFileName() ;
        SENDINFO_1( "File '%s' is closed" , FileName );

        m_Log.SaveLogAndClear( true , true ) ;
      }
      int iNFrames = m_Archive.GetFramesNmb() ;
      if ( m_pOutput->IsConnected() && !( iNFrames % 50 ) && ( iNFrames != m_iLastViewedNFrames ))
      {
//         CTextFrame * pDiagInfo = CreateTextFrameEx( "DiagInfo" ,
//           "%s Nwr=%d Lser=%d Tstor=%u Taq=%.3f Taqmax=%.3f Ts=%.2fms Tsmax=%.2fms M=%u MB" ,
//           GetTimeAsString_ms() ,
//           iNFrames , ( int ) m_Archive.GetSerializationQueueLength() ,
//           ( DWORD ) m_Archive.GetAllocatedForWrite() / 1000000 ,
//           m_Archive.m_dVFrameAquireTime_ms * 1000. ,
//           m_Archive.m_dAquireTimeMax_ms * 1000. ,
//           m_Archive.m_dSerializationTime_ms , m_Archive.m_dSerializationTimeMax_ms ,
//           ( UINT ) pmc.WorkingSetSize / 1000000 ) ;
        CTextFrame * pDiagInfo = CreateTextFrameEx( "DiagInfo" ,
          "%s Nwr=%d Lser=%d Twr=%.1f Nbusy=%d Nrq=%d Nfin=%d Lwr=%lld" ,
          GetTimeAsString_ms() ,
          iNFrames , ( int ) m_Archive.GetSerializationQueueLength() ,
          m_Archive.m_dVFrameSaveTime_ms , m_Archive.GetNBusyBuffers() ,
          m_Archive.m_iNRequestedWrites , m_Archive.m_iNCompletedWrights ,
          m_Archive.m_i64NWritten ) ;
        PutFrame( m_pOutput , pDiagInfo ) ;
        m_iLastViewedNFrames = iNFrames ;
      }
    }
    else if ( GetHRTickCount() - m_dLastErrorIndicatedTime > 1000. )
    {
      SENDERR( "Too much memory allocated (%d MB). %d frames omitted" ,
        pmc.WorkingSetSize / ( 1024 * 1024 ) , ++m_iNOmittedFrames ) ;
      CTextFrame * pDiagInfo = CreateTextFrameEx( "ErrorInfo" ,
        "Too much memory allocated (%d MB). %d frames omitted" ,
        pmc.WorkingSetSize / ( 1024 * 1024 ) , ++m_iNOmittedFrames ) ;
      PutFrame( m_pOutput , pDiagInfo ) ;
      m_dLastErrorIndicatedTime = GetHRTickCount() ;
    }
  }
}

bool FLWRender::IsFileOpen()
{
  return m_Archive.IsOpen();
}

void FLWRender::CloseFile()
{
  if ( IsFileOpen() )
    m_Archive.Close();
}

bool FLWRender::PrintProperties( FXString& text )
{
  FXPropertyKit pk( text );
  pk.WriteString( "filename" , m_FileNameTemplate );
  pk.WriteInt( "DoLog" , m_bDoLog ) ;
  pk.WriteInt( "NBuffers" , m_iNBuffers ) ;
  pk.WriteInt( "BufferSize_KB" , m_iBufferSize_KB ) ;
  text = pk;
  return true;
}

bool FLWRender::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pk( text );
  bool bBuffersChanged = false ;
//     m_FileNameTemplate="C:\\%x %X.flw";
  if ( pk.GetString( "filename" , m_FileNameTemplate ) )
  {
    m_Archive.Close();
    SENDINFO_1( "File '%s' is closed" , m_Archive.GetFileName() );
  };
  if ( pk.GetInt( "DoLog" , m_bDoLog ) )
  {
    SENDINFO_1( "Log is '%s'" , m_bDoLog ? "ON" : "OFF" ) ;
  }
  if ( pk.GetInt( "NBuffers" , m_iNBuffers ) )
  {
    SENDINFO_1( "NBuffers=%d" , m_iNBuffers ) ;
    bBuffersChanged = true ;
  }
  if ( pk.GetInt( "BufferSize_KB" , m_iBufferSize_KB ) )
  {
    SENDINFO_1( "BufferSize is %d KB" , m_iBufferSize_KB ) ;
    bBuffersChanged = true ;
  }
  if ( bBuffersChanged )
  {
    m_Archive.Close();
    m_Archive.SetAsyncWriteMode( m_iBufferSize_KB , m_iNBuffers ) ;
    m_iNOmittedFrames = 0 ;
  }
  return true;
}

bool FLWRender::ScanSettings( FXString& text )
{
  text = "calldialog(true)";
  return true;
}
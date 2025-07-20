// FLWCaptureGadget.cpp: implementation of the FLWCapture class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FLWCaptureGadget.h"
#include "FLWCapStpDialog.h"
#include <gadgets\TextFrame.h>
#include <gadgets\QuantityFrame.h>

#define THIS_MODULENAME "FLWCapture"

IMPLEMENT_RUNTIME_GADGET_EX( FLWCapture , CCaptureGadget , "Files.Capture" , TVDB400_PLUGIN_NAME );
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#pragma warning( disable : 4355)
FLWCapture::FLWCapture() :
m_NextFrame( NEXT_FRAME ) ,
m_Archive( this ) ,
m_bStopping( false ) ,
m_NextTime( 0 ) ,
m_pInputTrigger( NULL ) ,
m_bRewind( false ) ,
m_bDoLog( FALSE )
{
  m_pOutput = new COutputConnector( transparent );
  m_pControl = new CDuplexConnector( this , transparent , transparent );
  m_SetupObject = new CFLWCapStpDialog( this , NULL );
}

void FLWCapture::ShutDown()
{
  
  CCaptureGadget::ShutDown();
  if ( m_pInputTrigger )
    delete m_pInputTrigger;
  m_pInputTrigger = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
  delete m_pControl;
  m_pControl = NULL;
  m_Archive.Close();
}

int FLWCapture::GetInputsCount()
{
  if ( m_pInputTrigger )
    return 1;
  return 0;
}

CInputConnector* FLWCapture::GetInputConnector( int n )
{
  if ( !n )
    return m_pInputTrigger;
  return NULL;
}

void FLWCapture::OnStart()
{
  m_Archive.Rewind();
  CCaptureGadget::OnStart();
}

void FLWCapture::OnStop()
{
  m_bStopping = true;
  m_Archive.SkipDelay();
  CCaptureGadget::OnStop();
}

bool FLWCapture::PrintProperties( FXString& text )
{
  FXPropertyKit pc( text );
  pc.WriteString( "FileName" , m_Archive.GetFileName() );
  pc.WriteInt( "Loop" , ( m_Archive.IsAutoRewindOn() ? 1 : 0 ) );
  pc.WriteInt( "AutoRewind" , ( m_Archive.GetAutoRewindToStart() ? 1 : 0 ) );
  pc.WriteInt( "ExtTrig" , ( m_Archive.IsForceNextFrameOn() ? 1 : 0 ) );
  unsigned FixedDelay;
  pc.WriteInt( "ConstRate" , ( m_Archive.GetFixedDelay( FixedDelay ) ? 1 : 0 ) );
  int rate = ( FixedDelay ? 1000 / ( int )FixedDelay : 25 );
  pc.WriteInt( "FrameRate" , ( int )rate );
  pc.WriteInt( "DoLog" , m_bDoLog );
  pc.WriteInt( "FrameNumMin" , 0 ) ;
  pc.WriteInt( "FrameNumMax" , m_Archive.GetFramesNmb() - 1 ) ;
  pc.WriteDouble( "FrameTimeMin" , m_Archive.GetMinTime() , "%.3f" ) ;
  pc.WriteDouble( "FrameTimeMax" , m_Archive.GetMaxTime() , "%.3f" ) ;
  pc.WriteInt( "PlayNumMin" , m_Archive.m_iPlayNumberMin ) ;
  pc.WriteInt( "PlayNumMax" , m_Archive.m_iPlayNumberMax ) ;
  pc.WriteDouble( "PlayTimeMin" , m_Archive.m_dPlayTimeMin_ms , "%.3f" ) ;
  pc.WriteDouble( "PlayTimeMax" , m_Archive.m_dPlayTimeMax_ms , "%.3f" ) ;
  pc.WriteDouble( "TimeZoom" , m_Archive.m_dTimeZoom , "%.4f" ) ;
  pc.WriteInt( "IterateByFrames" , m_Archive.m_bIterateByFrames ) ;

  text = pc;
  return true;
}

bool FLWCapture::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  int Loop = 1;
  pc.GetInt( "Loop" , Loop );
  m_Archive.SetAutoRewind( Loop == 1 );
  Loop = 1;
  pc.GetInt( "AutoRewind" , Loop );
  m_Archive.SetAutoRewindToStart( Loop == 1 );
  pc.GetInt( "DoLog" , m_bDoLog ) ;
  bool bRecalculate = pc.GetInt( "PlayNumMin" , m_Archive.m_iPlayNumberMin ) ;
  bRecalculate |= pc.GetInt( "PlayNumMax" , m_Archive.m_iPlayNumberMax ) ;
  bRecalculate |= pc.GetDouble( "PlayTimeMin" , m_Archive.m_dPlayTimeMin_ms ) ;
  bRecalculate |= pc.GetDouble( "PlayTimeMax" , m_Archive.m_dPlayTimeMax_ms ) ;
  bRecalculate |= pc.GetDouble( "TimeZoom" , m_Archive.m_dTimeZoom ) ;
  if ( pc.GetInt( "IterateByFrames" , m_Archive.m_bIterateByFrames ) )
  {
    bRecalculate = true ;
    m_Archive.m_bIterateByTime = !m_Archive.m_bIterateByFrames ;
  };

  if ( bRecalculate )
    m_Archive.m_bRecalculate = TRUE ;

  FXString fileName;
  if ( pc.GetString( "FileName" , fileName ) && fileName.Compare( m_Archive.GetFileName() ) )
  {
    if ( !m_Archive.OpenRead( fileName ) )
    {
      SENDERR_1( "Can't open file '%s'" , fileName );
      m_Log.Reset( "" , 0 , FALSE ) ; // close and disable log
    }
    else
    {
      m_Archive.Rewind();
      SENDINFO_1( "File '%s' is open for reading" , fileName );
      m_Log.Reset( (fileName + GetTimeStamp()) + ".log" , FLW_LOG_SIZE , m_bDoLog ) ;
    }
  }
  int bExtTrig;
  if ( pc.GetInt( "ExtTrig" , bExtTrig ) && ( ( bExtTrig == 1 ) != m_Archive.IsForceNextFrameOn() ) )
    ToggleExternalTrigger();
  int FrameRate = 25;
  int bConstRate;
  if ( pc.GetInt( "ConstRate" , bConstRate ) && pc.GetInt( "FrameRate" , FrameRate ) && FrameRate )
  {
    DWORD delay = ( 1000 / ( DWORD )FrameRate );
    unsigned FixedDelay;
    if ( ( ( bConstRate == 1 ) != m_Archive.GetFixedDelay( FixedDelay ) ) || ( delay != ( int )FixedDelay ) )
      m_Archive.SetFixedDelay( ( bConstRate == 1 ) , ( unsigned )delay );
  }

  Status().WriteBool( STATUS_REDRAW , true );
  return true;
}

bool FLWCapture::ScanSettings( FXString& text )
{
  text = "calldialog(true)";
  return true;
}

void FLWCapture::ToggleExternalTrigger()
{
  if ( m_Archive.IsForceNextFrameOn() )
  {
    ASSERT( m_pInputTrigger );
    delete m_pInputTrigger;
    m_pInputTrigger = NULL;
    m_Archive.SetForceNextFrame( false );
    m_Archive.SkipDelay();
  }
  else
  {
    ASSERT( !m_pInputTrigger );
    m_Archive.SetForceNextFrame( true );
    m_pInputTrigger = new CInputConnector( nulltype , _fn_capture_trigger , this );
  }
}

CDataFrame* FLWCapture::GetNextFrame( double* StartTime )
{
  if ( m_bStopping )
  {
    m_bStopping = false;
    return NULL;
  }
  CDataFrame* pFrame = NULL ;
  if ( m_NextFrame != NEXT_FRAME )
  {
    pFrame = m_Archive.ReadFrame( m_NextFrame , m_NextTime );
    m_NextFrame = NEXT_FRAME;
    m_NextTime = 0;
  }
  else
  {
    if ( m_bRewind )
    {
      m_Archive.Rewind() ;
      m_bRewind = false ;
    }
    
    pFrame = m_Archive.ReadFrame( NEXT_FRAME );
  }
  *StartTime = GetHRTickCount() - m_Archive.GetLastOperationTime();
  m_Log.SetNextLogItem( pFrame ) ;
  return pFrame ;
}

void FLWCapture::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  CTextFrame* TextFrame = pParamFrame->GetTextFrame( DEFAULT_LABEL );
  if ( !TextFrame )
  {
    CBooleanFrame * pBool = pParamFrame->GetBooleanFrame() ;
    if ( pBool )
      m_bRewind = pBool->GetValue() ;

    pParamFrame->RELEASE( pParamFrame );
    return;
  }
  FXParser pk = ( LPCTSTR )TextFrame->GetString();
  pParamFrame->RELEASE( pParamFrame );
  FXString cmd , param;
  FXSIZE pos = 0;
  pk.GetWord( pos , cmd );
  if ( cmd.CompareNoCase( "getmax" ) == 0 )
  {
    pk.Format( "%u" , m_Archive.GetFramesNmb() );
  }
  else if ( ( cmd.CompareNoCase( "seek" ) == 0 ) && ( pk.GetWord( pos , cmd ) ) )
  {
    m_NextFrame = atoi( cmd );
    pk = "OK";
  }
  else if ( cmd.CompareNoCase( "getmaxtime" ) == 0 )
  {
    pk.Format( "%f" , m_Archive.GetMaxTime() );
  }
  else if ( ( cmd.CompareNoCase( "set" ) == 0 ) && ( pk.GetWord( pos , cmd ) ) && ( pk.GetParamString( pos , param ) ) )
  {
    if ( cmd.CompareNoCase( "FileName" ) == 0 )
    {
      if ( param.IsEmpty() || !m_Archive.OpenRead( param ) )
        pk = "Error";
      else
      {
        m_Archive.Rewind();
        pk = "OK";
      }
    }
    else if ( cmd.CompareNoCase( "SWTrigger" ) == 0 )
    {
      bool bSoftwareTrigger = ( param.CompareNoCase( "true" ) == 0 );
      if ( bSoftwareTrigger != m_Archive.IsForceNextFrameOn() )
        ToggleExternalTrigger();
      pk = "OK";
    }
    else if ( cmd.CompareNoCase( "LoopFilm" ) == 0 )
    {
      m_Archive.SetAutoRewind( param.CompareNoCase( "true" ) == 0 );
      pk = "OK";
    }
    else if ( cmd.CompareNoCase( "ConstRate" ) == 0 )
    {
      bool bConstRate = ( param.CompareNoCase( "true" ) == 0 );
      m_Archive.SetFixedDelay( bConstRate , 40 );
      pk = "OK";
    }
    else if ( cmd.CompareNoCase( "FrameRate" ) == 0 )
    {
      int rate = atoi( param );
      if ( !rate )
        rate = 25;
      rate = 1000 / rate;
      m_Archive.SetFixedDelay( true , rate );
      pk = "OK";
    }
    else if ( cmd.CompareNoCase( "DoLog" ) == 0 )
    {
      BOOL bDoLog = ( param.CompareNoCase( "true" ) == 0 ) 
                 || ( param.CompareNoCase( "1" ) == 0 );
      m_bDoLog = bDoLog ;
      pk = "OK";
    }
  }
  else if ( ( cmd.CompareNoCase( "get" ) == 0 ) && ( pk.GetWord( pos , cmd ) ) )
  {
    if ( cmd.CompareNoCase( "FileName" ) == 0 )
      pk = m_Archive.GetFileName();
    else if ( cmd.CompareNoCase( "SWTrigger" ) == 0 )
      pk = ( m_Archive.IsForceNextFrameOn() ) ? "true" : "false";
    else if ( cmd.CompareNoCase( "LoopFilm" ) == 0 )
      pk = ( m_Archive.IsAutoRewindOn() ) ? "true" : "false";
    else if ( cmd.CompareNoCase( "ConstRate" ) == 0 )
    {
      unsigned delay;
      pk = ( m_Archive.GetFixedDelay( delay ) ) ? "true" : "false";
    }
    else if ( cmd.CompareNoCase( "FrameRate" ) == 0 )
    {
      unsigned delay;
      if ( m_Archive.GetFixedDelay( delay ) )
        pk.Format( "%d" , ( ( delay ) ? 1000 / ( int )delay : 25 ) );
      else
        pk = "-1";
    }
    else if ( cmd.CompareNoCase( "DoLog" ) == 0 )
    {
      pk = m_bDoLog ? "1" : "0" ;
    }

  }
  else if ( ( cmd.CompareNoCase( "seektime" ) == 0 ) && pk.GetWord( pos , cmd ) )
  {
    m_NextFrame = FRAME_BY_TIME;
    m_NextTime = atof( cmd );
    pk = _T( "OK" );
  }
  else
    pk = "Available commands:\r\ngetmax - return max frame nmb\r\nseek <frnmb>\r\ngetmaxtime - return time for last frame\r\nseektime <time>\r\nget/set FileName\r\nget/set LoopFilm\r\nget/set SWTrigger\r\nget/set ConstRate\r\nget/set FrameRate";
  CTextFrame* retV = CTextFrame::Create( pk );
  retV->ChangeId( NOSYNC_FRAME );
  if ( !m_pControl->Put( retV ) )
    retV->RELEASE( retV );
}

int FLWCapture::GetDuplexCount()
{
  return 1;
}

CDuplexConnector* FLWCapture::GetDuplexConnector( int n )
{
  return ( ( !n ) ? m_pControl : NULL );
}

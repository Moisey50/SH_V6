// CaptureTimer.cpp: implementation of the CaptureTimer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TVGeneric.h"
#include "CaptureTimer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define DEFAULT_CAPTURE_INTERVAL	25

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_RUNTIME_GADGET_EX( CaptureTimer , CCaptureGadget , LINEAGE_GENERIC , TVDB400_PLUGIN_NAME );

CaptureTimer::CaptureTimer() :
  m_FrameRate( DEFAULT_CAPTURE_INTERVAL )
  , m_dFramePeriod( 1000./ DEFAULT_CAPTURE_INTERVAL )
  , m_NSentFrames(0)
  , m_NSendOnStart(0)
{
  m_pOutput = new COutputConnector;
  SetTicksIdle( ROUND(m_dFramePeriod) );
}

void CaptureTimer::ShutDown()
{
  if ( m_pOutput )
    delete m_pOutput;
  m_pOutput = NULL;
  CCaptureGadget::ShutDown();
}

bool CaptureTimer::PrintProperties( FXString& text )
{
  FXPropertyKit pk;
  pk.WriteInt( "FrameRate" , m_FrameRate );
  pk.WriteDouble( "FramePeriod_msec" , m_dFramePeriod);
  pk.WriteInt("NSendOnStart" , m_NSendOnStart);
  text = pk;
  return true;
}

bool CaptureTimer::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  if ( InRunningMode() )
    Suspend();

  FXPropertyKit pk( text );
  pk.GetInt("FrameRate", m_FrameRate);
  if ( pk.GetDouble( "FramePeriod_msec" , m_dFramePeriod) )
  {
    if ( m_dFramePeriod >= 5. )
      SetTicksIdle( ROUND( m_dFramePeriod ) ) ;
    else
      SetTicksIdle( ROUND( 1000. / (double) m_FrameRate ) ) ;
  }
  else if ( pk.GetInt( "FrameRate" , m_FrameRate ) )
    SetTicksIdle( ROUND( m_dFramePeriod = (1000. / (double) m_FrameRate) ) );
  if ( pk.GetInt("NSendOnStart" , m_NSendOnStart) )
  {
    if (m_NSendOnStart)
      m_NSentFrames = 0;
  }
  if ( InRunningMode() )
    Resume();
  return true;
}

bool CaptureTimer::ScanSettings( FXString& text )
{
  text = "template(Spin(FrameRate,1,25)"
    ",EditBox(FramePeriod_msec,0,1000)"
    ",Spin(NSendOnStart,0,1000))";
  return true;
}

CDataFrame* CaptureTimer::GetNextFrame( double* StartTime )
{
  if ( !m_NSendOnStart || (DWORD)m_NSendOnStart > m_NSentFrames )
  {
    CDataFrame* pDataFrame = CDataFrame::Create();
    m_NSentFrames++;
    return pDataFrame;
  }
  return NULL;
}

bool CaptureTimer::InRunningMode()
{
  if ( !m_pStatus ) return false;
  switch ( m_pStatus->GetStatus() )
  {
  case CExecutionStatus::RUN:
  case CExecutionStatus::PAUSE:
    return true;
  default:
    return false;
  }
}

// ExposureControl.cpp: implementation of the ExposureControl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImCNTL.h"
#include "ExposureControl.h"
#include "Registry.h"
#include "MainFrm.h"
#include "Intf_sup.h"
#include "get_time.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ExposureControl::ExposureControl()
{
  m_pOwner = NULL ;
  InitExposureData() ;
  m_Channel = NULL ;
  CString Status = GetStatus() ;
  ActualProgram() ;
}

void ExposureControl::InitExposureData()
{
  m_iExpStart = EXPOSURE_START ;
  m_iExpEnd = EXPOSURE_START + DEFAULT_EXPOSURE ;

  m_iAsynchronousMode = 0 ;
  m_iAsyncExposure_ms = 10 ;
  m_iAsyncPeriod_ms = 130 ;
  if ( !m_pOwner )
    return ;
  if ( m_pOwner->m_iGrabberType == GRABBER_METEOR )
  {
    m_iMaxExposure = 1200 ; //400?
    m_iMinExposure = 1 ;
    m_iDefaultExposure = 140 ;
    m_iMaxAsyncExposure = 360 ;
  }
  if ( m_pOwner->m_iGrabberType == GRABBER_SOLIOS )
  {
    m_iMaxExposure = ROUND( 63000. / m_dScanTime ) ;
    m_iMinExposure = ROUND( 30000. / m_dScanTime ) ;
    m_iDefaultExposure = ROUND( 30000. / m_dScanTime ) ;
  }
  m_iExpEnd = m_iExpStart + m_iDefaultExposure ;
  m_iExposure = m_iDefaultExposure ;
}

ExposureControl::~ExposureControl()
{
  if ( m_Channel )
    delete m_Channel ;
}

int 
ExposureControl::GetExposure()
{
  // relevant for Meteor
//   return (this->m_iAsynchronousMode == 0) ? 
//     ( m_iExpEnd - m_iExpStart ) 
//     :
//     this->m_iAsyncExposure_ms ;

  // using the following for both solios and meteor
  if ( !m_iAsynchronousMode )
    return m_iExposure ;
  return m_iAsyncExposure_ms ;
}

int 
ExposureControl::SetExposure(int NScans_or_ms )
{
  int iGrabberType = m_pOwner->m_iGrabberType ;
  switch ( iGrabberType )
  {
  case GRABBER_METEOR:
    return SetExposure_Meteor(NScans_or_ms ) ;
  case GRABBER_SOLIOS:
    return SetExposure_Solios(NScans_or_ms) ;
  default:
    MessageBox( NULL, "Grabber type not defined", 
      "SetExposure", MB_ERR_INVALID_CHARS ) ;
  }
  return 0 ;
}

int 
ExposureControl::SetExposure_Meteor(int NScans) // NScans is in scans for sync mode and in milliseconds in async mode
{
  if ( !CheckAndTakeChannel() )
    return 0 ;
  double dBeg = get_current_time() ;
  
  GetStatus() ;

  if ( !m_iAsynchronousMode )
  {
    if ( NScans < 0 )
    {
      m_iExposure = m_iDefaultExposure ;
      m_iExpEnd = m_iExpStart + m_iExposure ;
    }
    else
    {
      if ( NScans )
      {
        if ( NScans > m_iMaxExposure )
          m_iExposure = m_iMaxExposure ;
        else
          m_iExposure = NScans ;
        m_iExpEnd = m_iExpStart + m_iExposure ;
      }
    }
    if ( m_iExpEnd > m_iMaxExposure )
    {
      m_iExposure -= ( m_iExpEnd - m_iMaxExposure ) ; // trim the difference
      m_iExpEnd = m_iMaxExposure ;
    }
    //4.11.09 - make current Async exposure be the current exposure:
    m_iAsyncExposure_ms = m_iExposure ;
    ActualProgram() ;
  }
  else
  {
    if ( NScans <= 0 )
      m_iAsyncExposure_ms = m_iDefaultAsyncExposure ;
    else
      m_iAsyncExposure_ms = NScans ;

    if ( m_iAsyncExposure_ms > m_iMaxAsyncExposure )
      m_iAsyncExposure_ms = m_iMaxAsyncExposure ;

    m_iAsyncPeriod_ms = m_iAsyncExposure_ms + 3 ;
    
    if ( m_iAsyncPeriod_ms < 130 )
      m_iAsyncPeriod_ms = 130 ;

    CString Out ;
    Out.Format( "NS%04d %04d %04d\r" ,
      m_iAsyncPeriod_ms , m_iAsyncExposure_ms + 2 , 2 ) ;
    if (m_Channel->GetPort() >= 0) 
      m_Channel->SendData( (LPCTSTR)Out , Out.GetLength() , 10 ) ;
    m_iExposure = m_iAsyncExposure_ms ; //4.11.09 make current sync exposure and async exposure equal
  }
  double dExpLength = get_current_time() - dBeg ;
  CString OutPut;
  OutPut.Format( "Exp=%6d,T=%6.2f", m_iExposure, dExpLength ) ;
  if ( m_pOwner )
    m_pOwner->LogMessage3( (LPCTSTR)OutPut ) ;
  return GetExposure() ;
}

int
ExposureControl::SetExposure_Solios(int iNScans_or_ms )
{
//   int iShutTime = m_pOwner->m_iShutTime ;
  CleanCamera_RS_InputBuffer() ;
  CString Msg ;
  CString Answer ;
  double dBeg = get_current_time() ;

  // verify new exposition value is within limits
  int iNewExposure = iNScans_or_ms ;
  if ( iNewExposure > m_iMaxExposure )
    iNewExposure = m_iMaxExposure ;
  if ( iNewExposure < m_iMinExposure )
    iNewExposure = m_iMinExposure ;

  m_iExposure = iNewExposure ;
  iNewExposure *= ROUND(m_dScanTime) ; //to fit with Solios work mode

  int iFrameRate = m_pOwner->m_nCurrFrameRate ;
  if ( m_iAsynchronousMode )
  {
    double dPossibleFPS = 1000000. /(double)iNewExposure ;
    if ( ROUND(dPossibleFPS) != iFrameRate )
    {
      if ( dPossibleFPS > 30. )
        dPossibleFPS = 30. ;
      double dRatio = dPossibleFPS/(double)iFrameRate ;
      if ( dRatio < 1.0 || dRatio > 1.2 )
      {
        CString FPSCommand ;
        FPSCommand.Format( "ssf %d\r", ROUND(dPossibleFPS) ) ;
        m_Channel->SendData( FPSCommand ) ;
        int iRes = WaitCameraAnswer() ;
      }
    }
    m_iAsyncExposure_ms = m_iExposure ;
  }
  CString ExpCommand ;
  ExpCommand.Format( "set %d\r", iNewExposure ) ;
  m_Channel->SendData( ExpCommand ) ;

  int iRes = WaitCameraAnswer() ;

  double dExpLength = get_current_time() - dBeg ;
  CString OutPut;
  OutPut.Format( "Exp=%6d,T=%6.2f", m_iExposure, dExpLength ) ;
  if ( m_pOwner )
    m_pOwner->LogMessage3( (LPCTSTR)OutPut ) ;

  return GetExposure() ;
}

int 
ExposureControl::ActualProgram() //wrapper function
{
  if ( !CheckAndTakeChannel() )
    return 0 ;
  if ( !m_pOwner )
    return 0 ;
  int iGrabberType = m_pOwner->m_iGrabberType ;
  switch ( iGrabberType )
  {
  case GRABBER_METEOR:
    return ActualProgram_Meteor() ;
  case GRABBER_SOLIOS:
    return ActualProgram_Solios() ;
  default:
    MessageBox( NULL, "Grabber type unknown!", "ActualProgram", MB_OK) ;
  }
  return 0 ;
}
int 
ExposureControl::ActualProgram_Meteor()
{
  CString Out ;
  Out.Format( "AS%04d %04d\r" , m_iExpEnd , m_iExpStart ) ;
  m_Channel->SendData( Out ) ;
  return 1 ;
}

int
ExposureControl::ActualProgram_Solios()
{
  CString Out ;
  Out.Format( "PD%04d\r" , m_iExpEnd ) ;
  m_Channel->SendData( Out ) ;
  Sleep( 20 ) ;
  Out.Format( "ED%04d\r" , m_iExpStart ) ;
  m_Channel->SendData( Out ) ;
  return 1 ;
}

CString 
ExposureControl::GetStatus()
{
  CString Out( "CS\r" ) ;
  if ( !CheckAndTakeChannel() )
    return Out = "No Communication" ;

  char buf[300] ;
  CString In ;
  int InLen = m_Channel->GetInputData( ( BYTE* )buf , 299 ) ; // clean buffer

  m_Channel->SendData( Out ) ;

  Sleep( 70 ) ;

  if ( m_Channel->GetInputLen() )
  {
    int InLen = m_Channel->GetInputData( ( BYTE* )buf , 299 ) ;
    if (InLen >= 0) 
    {
      buf[ InLen ] = 0 ;
      In = buf ;
      In.Trim( "\n \r") ;
    }

    if ( InLen )
    {
      if ( IApp()->m_bTechMode )
        IApp()->LogMessage2( In ) ;
      else
      {
        CString sMsg = IApp()->BuildOperatorStatusMsg( (LPCTSTR)buf );
        IApp()->LogMessage2( (LPCTSTR) sMsg ) ;
      }
    }

    Out = buf ;
  }
  else
    Out.Empty() ;

  return Out ;
}

int ExposureControl::SetVSyncFront(int bFall)
{
  if ( !CheckAndTakeChannel() )
    return 0 ;
  CString Out ;
  Out.Format( "VF%c\r" , (bFall) ? '1' : '0' ) ;
  return m_Channel->SendData( Out ) ;
}

int ExposureControl::ReleaseChannel(void)
{
  if (!m_Channel)
    return 0 ;
  delete m_Channel ;
  m_Channel = NULL ;
  return 1;
}

int ExposureControl::CheckAndTakeChannel(void)
{
  if (m_Channel && m_Channel->GetPort() > 0)
    return 1 ;

  CRegistry Reg("File Company");
  int iPort = Reg.GetRegiInt( "ImCNTL\\CameraComm" , "Port" , 3 ) ;
  int iBaud = Reg.GetRegiInt( "ImCNTL\\CameraComm" , "Baud" , 9600 ) ;

  m_Channel = new CCSerIO( iPort , iBaud ) ;
  if (m_Channel->GetPort() <= 0)
  {
    CString Msg ;
    Msg.Format( "Can't open COM port %d, Err=%d" , iPort , m_Channel->GetPort() ) ;
    ::MessageBox( NULL , (LPCTSTR)Msg , "ImagCNTL with CCB" , MB_OK + MB_SETFOREGROUND ) ;

    delete m_Channel ;
    m_Channel = NULL ;
  }

  int iSL = Reg.GetRegiInt( "ImCNTL\\ExposureParam" , "SeparationLength" , 1456 ) ; // for Jericho; 956 for Carmel
  int iSP = Reg.GetRegiInt( "ImCNTL\\ExposureParam" , "ScanTime_12MHz" , 3563 ) ; // for Jericho; 3039 for Carmel
  if ( !m_pOwner )
    return ( m_Channel->GetPort() ) ;
  if ( m_pOwner->m_iGrabberType == GRABBER_METEOR ) // relevant for meteor
  {
    CString Out ;
    Out.Format( "SL%d\r" , iSL ) ;
    m_Channel->SendData( Out ) ;
    Sleep(20) ;
    Out.Format( "SP%d\r" , iSP ) ;
    m_Channel->SendData( Out ) ;
    Sleep(20) ;
    SetVSyncFront( 0 ) ;
    Out = "SB1\r" ;
    m_Channel->SendData( Out ) ;
  }



  return (m_Channel->GetPort() > 0);
}

int ExposureControl::WaitCameraAnswer(void)
{
  double dBeginWaiting = get_current_time() ;
  int iOldInputLen = m_Channel->GetInputLen() ;
  int iNewInputLen ;
  do 
  {
    iNewInputLen = iOldInputLen ;
    Sleep(1) ;
    iNewInputLen = m_Channel->GetInputLen() ;
    if ( get_current_time() - dBeginWaiting > 1000. )
    {
      if ( m_pOwner )
        m_pOwner->LogMessage3( "Camera Answer Timeout" ) ;
      return -1;
    }
  } while( iNewInputLen == iOldInputLen ); // Wait for answer beginning
  int iCnt = 0 ;   
  do 
  {
    iCnt++ ;
    iOldInputLen = iNewInputLen ;
    Sleep(15) ;
    iNewInputLen = m_Channel->GetInputLen() ;
    if ( get_current_time() - dBeginWaiting > 2000. )
    {            
      if ( m_pOwner )
        m_pOwner->LogMessage3( "Too Long Receive Time" ) ;
      return -2;
    }
  } while( (iNewInputLen != iOldInputLen)  &&  (iNewInputLen <= 4) ); // Wait for answer end

  BYTE buf[2048] ;
  int iLen = m_Channel->GetInputData( buf , sizeof(buf) ) ;
  if (iLen < 2048)
  {
    CString Answer(buf) ;
    CString Ans2 ;
    if (iLen > 2)
      Ans2 = &buf[2] ;   
    Answer.MakeUpper() ;
    if ( Answer.Find("OK>") >= 0)
      return 1 ;
    if (Answer.Find("ERROR") >= 0)
    {
      //strcpy(MyMsg, (LPCSTR)Answer) ;
      if ( m_pOwner )
        m_pOwner->LogMessage3( (LPCSTR)Answer ) ;
      return 0 ;
    }
  }
  //LogMessage3( "Too Long Data in Serial Buffer" ) ;
  return -3 ;
}
void ExposureControl::CleanCamera_RS_InputBuffer(void)
{
  if( !CheckAndTakeChannel() )
    return ;
  while ( m_Channel->GetInputLen() )
  {
    BYTE buf[2048] ;
    int iLen = m_Channel->GetInputData( buf , 2048 ) ;
  }
}

int ExposureControl::SetExposureMode(int iSync)
{
  CleanCamera_RS_InputBuffer() ;

  int iRes ;

  // Meteor
  if ( m_pOwner->m_iGrabberType == GRABBER_METEOR )
  {
    m_iAsynchronousMode = !iSync ;
    //The commented part restores the previous Sync exposure.
    // What we want is for the exposure to remain the same when switching from Async to Sync modes
    //4.11.09 Alex
//    int iSyncExp = m_iExpEnd - m_iExpStart ;
//    int iAsyncExp = m_iAsyncExposure_ms ;
//    iRes = SetExposure( 
//      iSync ? iSyncExp : iAsyncExp ) ;
    iRes = SetExposure( GetExposure()) ;
  }
  
  // Solios
  else if ( m_pOwner->m_iGrabberType == GRABBER_SOLIOS )
  {
    CString FPSCommand ;
    FPSCommand.Format("sem %d\r" , iSync ? 6 : 2 ) ;  // DALSA
    m_Channel->SendData( FPSCommand ) ;
    iRes = WaitCameraAnswer() ;
  }                            
  return iRes;
}


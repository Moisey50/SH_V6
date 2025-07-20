// Basler.cpp: implementation of the Basler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <gadgets\gadbase.h>
#include <gadgets\stdsetup.h>
#include <gadgets\textframe.h>
#include <math\Intf_sup.h>
#include "Basler.h"
#include "BaslerCamera.h"
#include <video\shvideo.h>
#include <helpers\FramesHelper.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "BaslerCamera.cpp"

// #ifdef _DEBUG
//   #pragma comment( lib, "FlyCapture2d.lib" )
// #else
//#pragma comment( lib, "FlyCapture2.lib" )
// #endif

IMPLEMENT_RUNTIME_GADGET_EX( Basler , CDevice , "Video.capture" , TVDB400_PLUGIN_NAME );


CObservedMutex              Basler::m_ConfigMutex/*( "BaslerConfig" )*/;
FXLockObject                Basler::m_ConfigLock;
Device::DeviceInfo          Basler::m_CamInfo[ MAX_DEVICESNMB ];
int                         Basler::m_iCamNum;
int                         Basler::m_iGadgetCount = 0;
FXStringArray               Basler::m_BusyCameras;
bool                        Basler::m_bCamerasEnumerated = false;
bool                        Basler::m_bRescanCameras = true;
BaslerCameraInfo            Basler::m_GigECamerasInfo[ MAX_DEVICESNMB ];
UINT                        Basler::m_uiNGigECameras;
bool                        Basler::m_bSaveFullInfo = true ;
BaslerWatcher *             Basler::m_pBaslerWatcher = NULL ;

BaslerWatcher::BaslerWatcher( Basler * pGadget )
  : FXWorker()
{
  if ( pGadget )
    RegisterGadget( pGadget ) ;
  m_dwTicksIdle = 10 ;
  m_bDidCheck = false ;
}

BOOL BaslerWatcher::Create()
{
  return FXWorker::Create() ;
}

FXString  GetNames( FXStringArray& src )
{
  FXString Result ;
  for ( int i = 0 ; i < src.GetCount() ; i++ )
    Result += src[ i ] + _T( ';' ) ;
  return Result ;
}

int BaslerWatcher::DoJob()
{
  CTlFactory& tlFactory = CTlFactory::GetInstance();

  // Get all attached devices and exit application if no device is found.
  DeviceInfoList_t devices;
  try
  {
    if ( tlFactory.EnumerateDevices( devices ) != 0 )
    {
      FXAutolock al( m_Protect ) ;
      m_CurrentCameras.RemoveAll() ;
      for ( size_t i = 0 ; i < devices.size() ; i++ )
      {
        FXString CameraSN( devices[ i ].GetSerialNumber().c_str() ) ;
        if ( !CameraSN.IsEmpty() )
          m_CurrentCameras.Add( CameraSN ) ;
        else
          ASSERT( 0 ) ;
      }
      if ( m_CurrentCameras.GetCount() )
      {
        if ( m_KnownCameras.GetCount() )
        {
          FXStringArray TmpKnown , TmpCurrent ;
          TmpCurrent.Copy( m_CurrentCameras ) ;
          TmpKnown.Copy( m_KnownCameras ) ;
          for ( int i = 0 ; i < TmpCurrent.GetCount() ; i++ )
          {
            for ( int j = 0 ; j < TmpKnown.GetCount() ; j++ )
            {
              if ( TmpCurrent[ i ] == TmpKnown[ j ] )
              {
                TmpCurrent.RemoveAt( i-- ) ;
                TmpKnown.RemoveAt( j ) ;
                break ;
              }
            }
          }
          if ( TmpCurrent.GetCount() ) // new cameras appeared
          {
            for ( int i = 0 ; i < m_WorkingGadgets.GetCount() ; i++ )
              m_WorkingGadgets[ i ]->NewCameraCallback( TmpCurrent ) ;
          }
          if ( TmpKnown.GetCount() ) // new cameras appeared
          {
            for ( int i = 0 ; i < m_WorkingGadgets.GetCount() ; i++ )
              m_WorkingGadgets[ i ]->CameraDisconnected( TmpKnown ) ;
          }
        }
        else // there is no known cameras, i.e. new camera connected
        {
          for ( int i = 0 ; i < m_WorkingGadgets.GetCount() ; i++ )
            m_WorkingGadgets[ i ]->NewCameraCallback( m_CurrentCameras ) ;
        }
        m_KnownCameras.Copy( m_CurrentCameras ) ;
      }
      else  // there is no connected cameras
      {
        if ( m_KnownCameras.GetCount() != 0 )
        {
          for ( int i = 0 ; i < m_WorkingGadgets.GetCount() ; i++ )
            m_WorkingGadgets[ i ]->CameraDisconnected( m_KnownCameras ) ;

          m_KnownCameras.RemoveAll() ;
        }
      }
      if ( m_WorkingGadgets.GetCount() )
      {
        HANDLE hMutex = m_WorkingGadgets[ 0 ]->m_Devices.GetMutexHandle() ;
        if ( hMutex )
        {
          CAutoLockMutex_H al( hMutex ) ;
          if ( m_KnownCameras.GetCount() )
          {
            int iCamCnt = 0 ;
            for ( iCamCnt = 0 ; iCamCnt < m_KnownCameras.GetCount() ; iCamCnt++ )
            {
              FXString CamName( "_" ) ;
              CamName += m_KnownCameras[ iCamCnt ] ;
              for ( int iGadCnt = 0 ; iGadCnt < m_WorkingGadgets.GetCount() ; iGadCnt++ )
              {
                if ( m_KnownCameras[ iCamCnt ] == m_WorkingGadgets[ iGadCnt ]->m_sSerialNumber )
                {
                  if ( CamName[ 0 ] == _T( '_' ) )
                    CamName.SetAt( 0 , _T( '-' ) ) ;
                }
              }
            }
          }
        }
      }
    }
    else
    {
      if ( m_KnownCameras.GetCount() != 0 )
      {
        for ( int i = 0 ; i < m_WorkingGadgets.GetCount() ; i++ )
          m_WorkingGadgets[ i ]->CameraDisconnected( m_KnownCameras ) ;

        m_KnownCameras.RemoveAll() ;
      }

    }
  }
  catch ( const GenericException& e )
  {
    FXString ErrMsg;
    ErrMsg.Format( _T( "\nBaslerWatcher exception %s" ) ,
      e.GetDescription() );
    TRACE( (LPCTSTR) ErrMsg ) ;
  }
  m_bDidCheck = true ;
  m_dwTicksIdle = 1000 ;
  return WR_CONTINUE ;
}

void BaslerWatcher::RegisterGadget( Basler * pGadget )
{
  FXAutolock al( m_Protect ) ;
  for ( int i = 0 ; i < m_WorkingGadgets.GetCount() ; i++ )
  {
    if ( m_WorkingGadgets[ i ] == pGadget )
      return ;
  }
  m_WorkingGadgets.Add( pGadget ) ;
}

bool BaslerWatcher::UnregisterGadget( Basler * pGadget )
{
  FXAutolock al( m_Protect ) ;
  for ( int i = 0 ; i < m_WorkingGadgets.GetCount() ; i++ )
  {
    if ( m_WorkingGadgets[ i ] == pGadget )
    {
      m_WorkingGadgets.RemoveAt( i ) ;
      break ;
    }
  }
  return (m_WorkingGadgets.GetCount() != 0) ;
}

#ifdef _DEBUG
DWORD Basler::m_dwDefaultTimeout = 60000;
#else
DWORD Basler::m_dwDefaultTimeout = 1000;
#endif
//
//void Basler::NewImageCallBack( Image * pImage , const void * pCallbackData )
//{
//  double dStartTime = GetHRTickCount() ;
//  Basler * pGadget = ( Basler* )pCallbackData ;
//  if ( pGadget->m_iNSkipImages > 0 )
//  {
//    pGadget->m_iNSkipImages-- ;
//    return ;
//  }
//
//  Error error;
//  unsigned int rows , cols , stride;
//  EPixelType format;
//  BayerTileFormat BayerFormat ;
//
//  
//  pImage->GetDimensions( &rows , &cols , &stride , &format , &BayerFormat );
//  DWORD dwDataSize = pImage->GetDataSize() ;
//
//  FlyCapture2::Image * pCopy = new FlyCapture2::Image(
//    rows , cols , stride , pImage->GetData() ,
//    dwDataSize , format , BayerFormat ) ;
//
//
//  if ( pCopy )
//  {
//    if ( pGadget->m_bStopInitialized )
//    {
//      TRACE( "Frame after stop initialized, cam %u\n" , pGadget->m_dwSerialNumber ) ;
//    }
//    pGadget->m_GrabLock.Lock() ;
//    if ( pGadget->m_bStopInitialized )
//    {
//      TRACE( "Lock after stop initialized, cam %u\n" , pGadget->m_dwSerialNumber ) ;
//    }
//    if ( pGadget->m_pNewImage )
//    {
//      if ( pGadget->m_bStopInitialized )
//      {
//        TRACE( "Image delete after stop initialized\n" ) ;
//      }
//      delete pGadget->m_pNewImage ;
//      pGadget->m_pNewImage = NULL ;
//    }
//    pGadget->m_pNewImage = pCopy ;
//    SetEvent( pGadget->m_evFrameReady ) ;
//    pGadget->m_dLastFrameTime = GetHRTickCount() ;
//    pGadget->m_dLastInCallbackTime = pGadget->m_dLastFrameTime - dStartTime ;
//    pGadget->m_bTriedToRestart = false ;
//    pGadget->m_GrabLock.Unlock() ;
//    if ( pGadget->m_bStopInitialized )
//    {
//      TRACE( "Unlock after stop initialized, cam %u\n" , pGadget->m_dwSerialNumber ) ;
//    }
//    if ( pGadget->m_CallbackThreadName.IsEmpty() )
//    {
//      pGadget->GetGadgetName( pGadget->m_CallbackThreadName ) ;
//      pGadget->m_CallbackThreadName += _T( "_CB" ) ;
//      DWORD dwThreadId = ::GetCurrentThreadId() ;
//      ::SetThreadName( ( LPCSTR )( ( LPCTSTR )pGadget->m_CallbackThreadName ) , dwThreadId ) ;
//    }
//  }
//}
//

BaslerPropertyDefine Basler::m_cBProperties[] =
{
  { FORMAT_MODE , _T( "Format" ) , _T( "PixelFormat" ) , (LPCTSTR) NULL } ,
{ SHUTTER_AUTO , _T( "" ) , (LPCTSTR) _T( "ExposureAuto" ) , (LPCTSTR) NULL } ,
{ SHUTTER , "Shutter_us" , _T( "ExposureTime" ) , _T( "ExposureTimeAbs" ) , (LPCTSTR) NULL } ,
{ GAIN_AUTO , _T( "" ) , (LPCTSTR) _T( "GainAuto" ) , (LPCTSTR) NULL } ,
{ GAIN , "Gain_dBx10" , _T( "Gain" ) , _T( "GainRaw" ) , (LPCTSTR) NULL } ,
{ BRIGHTNESS , "Brightness" , _T( "Brightness" ) , _T( "BlackLevel" ) ,
_T( "BlackLevelRaw" ) , (LPCTSTR) NULL } ,
{ GAMMA , "Gamma" , _T( "Gamma" ) , (LPCTSTR) NULL } ,
{ EXPOSURE_MODE , "ExposureMode" , _T( "ExposureMode" ) , (LPCTSTR) NULL } ,
{ TRIGGER_SELECTOR , "Trig_Selector" , _T( "TriggerSelector" ) , (LPCTSTR) NULL } ,
{ TRIGGER_MODE , "Trigger_mode" , _T( "TriggerMode" ) , (LPCTSTR) NULL } ,
{ TRIGGER_SOURCE , "Trigger_source" , _T( "TriggerSource" ) ,
_T( "TriggerSelector" ) , (LPCTSTR) NULL } ,
{ TRIGGER_SOFT_FIRE , _T( "FireSoftTrigger" ) , _T( "TriggerSoftware" ) , NULL } ,
{ TRIGGER_DELAY , "Trigger_delay_us" , _T( "TriggerDelayAbs" ) ,
_T( "TriggerDelay" ) , (LPCTSTR) NULL } ,
{ TRIGGER_ACTIVATE , "Trigger_activate" , _T( "TriggerActivation" ) , (LPCTSTR) NULL } ,
{ BURST_LENGTH , "Burst_Length" , _T( "AcquisitionBurstFrameCount" ) ,
  _T( "AcquisitionFrameCount" ) , (LPCTSTR) NULL } ,
{ FRAME_RATE , "Frame_rate" , _T( "FPS" ) , _T( "AcquisitionFrameRate" ) ,
_T( "AcquisitionFrameRateAbs" ) , (LPCTSTR) NULL } ,
{ FRAME_RATE_ENABLE , "" , _T( "AcquisitionFrameRateEnable" ) , (LPCTSTR) NULL } ,
{ BASLER_PACKETSIZE , _T( "PacketSize" ) , _T( "" ) , (LPCTSTR) NULL } ,
{ BASLER_INTER_PACKET_DELAY , _T( "InterpacketDelay" ) , _T( "" ) , (LPCTSTR) NULL } ,
{ SETROI , "ROI" , _T( "AOI" ) , (LPCTSTR) NULL } ,
{ LINE_SELECTOR , "LineSelect" , _T( "LineSelector" ) , (LPCTSTR) NULL } ,
{ LINE_MODE , "LineMode" , _T( "LineMode" ) , (LPCTSTR) NULL } ,
{ LINE_SOURCE , "LineSource" , _T( "LineSource" ) , (LPCTSTR) NULL } ,
{ LINE_INVERTER , "LineInvert" , _T( "LineInverter" ) , (LPCTSTR) NULL } ,
{ TIMER_SELECT , "TimerSelect" , _T( "TimerSelector" ) , (LPCTSTR) NULL } ,
{ TIMER_DELAY , "TimerDelay_us" ,
_T( "TimerDelay" ) , _T( "TimerDelayAbs" ) , (LPCTSTR) NULL } ,
{ TIMER_DURATION , "TimerDuration_us" ,
_T( "TimerDuration" ) , _T( "TimerDurationAbs" ) , (LPCTSTR) NULL } ,
{ TIMER_TRIGGER_SOURCE , "TimerTrigger" , _T( "TimerTriggerSource" ) , (LPCTSTR) NULL } ,
{ TIMER_TRIGGER_ACTIVATION , "TimerActivation" , _T( "TimerTriggerActivation" ) , (LPCTSTR) NULL } , 
{ OUTPUT_SELECTOR , "OutputSelector" , _T( "UserOutputSelector" ) , (LPCTSTR) NULL } ,
{ OUTPUT_VALUE , "OutputValue" , _T( "UserOutputValue" ) , (LPCTSTR) NULL } ,

//{ TIMER_TRIGGER_POLARITY , "TimerTriggerPolarity" , _T( "TimerTriggerActivation" ) , (LPCTSTR) NULL } , 
{ USER_SET_SELECTOR , "UserSetSelect" , _T( "UserSetSelector" ) , (LPCTSTR) NULL } ,
{ SAVE_SETTINGS , "SaveSettings" , _T( "UserSetSave" ) , (LPCTSTR) NULL } ,
// { EVENT_SELECTOR , "EventSelect" , "EventSelector" , (LPCTSTR) NULL } ,
// { EVENT_ENABLE , "EventEnable" , "EventNotification" , (LPCTSTR) NULL } ,
// { EVENTS_STATUS , "EventsStatus" , NULL , (LPCTSTR) NULL },
{ N_ACTIVE_BUFFERS , "NBuffers" , "" } 
//,{ DONT_SHOW_ERRORS , "DontShowErrors" , "" }

} ;

FXCamPropertyType NotViewedProperties[] = { FRAME_RATE_ENABLE , GAIN_AUTO , SHUTTER_AUTO } ;
bool IsNotViwed( FXCamPropertyType id )
{
  for ( int i = 0 ; i < ARRSZ( NotViewedProperties ) ; i++ )
  {
    if ( NotViewedProperties[ i ] == id )
      return true ;
  }
  return false ;
}

LPCTSTR OffXNames[] = { _T( "OffsetX" ) , _T( "XOffset" ) , (LPCTSTR) NULL } ;
LPCTSTR OffYNames[] = { _T( "OffsetY" ) , _T( "YOffset" ) , (LPCTSTR) NULL } ;

int Basler::_getPropertyCount()
{
  return ARRSZ( m_cBProperties ) ;
}

LPCTSTR Basler::_getPropertyname( int id )
{
  for ( int i = 0; i < _getPropertyCount(); i++ )
  {
    if ( (int) (m_cBProperties[ i ].Id) == id )
      return m_cBProperties[ i ].pUIName;
  }
  return "Unknown property";
}

LPCTSTR * Basler::_getPropertyInCamNames( int id )
{
  for ( int i = 0; i < _getPropertyCount(); i++ )
  {
    if ( (int) (m_cBProperties[ i ].Id) == id )
      return m_cBProperties[ i ].ppNodesNames ;
  }
  return NULL ;
}

// Videoformat vFormats[] =
// {
//   {PixelFormat_Mono8 , "Mono8" } ,
// { PixelFormat_Mono10 , "Mono10" } ,
// { PixelFormat_Mono12 , "Mono12" } ,
// { PixelFormat_Mono12p , "Mono12p" } ,
// { PixelFormat_Mono16 , "Mono16" } ,
// { PixelFormat_YUV411Packed , "YUV411" } ,
// { PixelFormat_YUV422Packed , "YUV422" } ,
// { PixelFormat_BayerGR8 , "RGB24" } ,
// { PixelFormat_RGB8Packed , "RGB24" } ,
// //{ PIXEL_FORMAT_RGB16 , "RGB48" } ,
// //{ PIXEL_FORMAT_RAW8 , "RAW8" },
// //{PIXEL_FORMAT_RAW16,"RAW16"}
// };

string Basler::GetPixelFormatName(DWORD FormatAsDWORD)
{
  for ( size_t i = 0; i < m_AvailableFormats.size() ; i++ )
  {
    if ( FormatAsDWORD == m_AvailableFormats[i].second )
      return m_AvailableFormats[i].first ;
  }
  return string();
}

int Basler::GetGain_dBx10( BaslerCamProperty * pProp )
{
  if ( m_Camera.GainAuto.IsValid() && m_Camera.GainAuto.IsReadable() )
  {
    pProp->m_bAuto = ( m_Camera.GainAuto.GetValue() == GainAuto_Continuous ) ;
    pProp->LastValue.bBool = pProp->m_bAuto ;
  }
  else
    pProp->LastValue.bBool = pProp->m_bAuto = false ;
  if ( m_Camera.Gain.IsValid() )
  {
    pProp->LastValue.iInt = ROUND( 
      ( m_LastEmbedInfo.m_dGain_dB = m_Camera.Gain.GetValue() ) * 10. ) ;
//     if ( m_SetupObject )
//     {
//       CGadgetStdSetup * pSetupDlg = ( CGadgetStdSetup* ) m_SetupObject ;
//       pSetupDlg->SetCellInt( "Gain_dBx10" , pProp->LastValue.iInt ) ;
//     }
  }
  else if ( m_Camera.GainAbs.IsValid() )
  {
    pProp->LastValue.iInt = ROUND( 
      ( m_LastEmbedInfo.m_dGain_dB = m_Camera.GainAbs.GetValue() ) * 10. ) ;
  }
  else if ( m_Camera.GainRaw.IsValid() )
  {
    pProp->LastValue.iInt = ROUND( 
      ( m_LastEmbedInfo.m_dGain_dB = ( double ) m_Camera.GainRaw.GetValue() ) ) ;
  }
  else
    m_LastEmbedInfo.m_dGain_dB = ( double ) ( pProp->LastValue.iInt ) ;

  return true ;
}

int Basler::SetGain_dBx10( BaslerCamProperty * pProp , int iGain_dBx10 , bool bAuto )
{
  pProp->m_bAuto = bAuto ;
  if ( bAuto )
  {
    if ( m_Camera.GainAuto.IsValid() && m_Camera.GainAuto.IsWritable() )
    {
      m_Camera.GainAuto.SetValue( GainAuto_Continuous ) ;
      return 0 ;
    }
    else
    {
      pProp->m_bAuto = false ;
      return GetGain_dBx10( pProp ) ;
    }
  }
  else
  {
    if ( m_Camera.GainAuto.IsValid() && m_Camera.GainAuto.IsWritable() )
    {
//       if ( m_bGainAfterScanSettings )
//       {
//         m_bGainAfterScanSettings = false ;
//         return pProp->LastValue.iInt ;
//       }
//       else
//       {
        if ( m_Camera.GainAuto.GetValue() == GainAuto_Continuous )
          m_Camera.GainAuto.SetValue( GainAuto_Off ) ;
//       }
    }
  }

  if ( iGain_dBx10 < pProp->LastValue.dMin )
    iGain_dBx10 = (int)ceil( pProp->LastValue.dMin ) ;
  else if ( pProp->LastValue.dMax < iGain_dBx10 )
    iGain_dBx10 = ( int ) floor( pProp->LastValue.dMax );

  if ( m_Camera.Gain.IsValid() && m_Camera.Gain.IsWritable() )
  {
    m_Camera.Gain.SetValue( iGain_dBx10 / 10. ) ;
  }
  else if ( m_Camera.GainAbs.IsValid() && m_Camera.GainAbs.IsWritable() )
  {
    m_Camera.GainAbs.SetValue( iGain_dBx10 / 10. ) ;
  }
  else if ( m_Camera.GainRaw.IsValid() && m_Camera.GainRaw.IsWritable() )
  {
    m_Camera.GainRaw.SetValue( iGain_dBx10 ) ;
  }
  m_LastEmbedInfo.m_dGain_dB = pProp->LastValue.dDouble =
    ( double ) ( pProp->LastValue.i64Int = pProp->LastValue.iInt = iGain_dBx10 ) ;

  //         if ( m_SetupObject )
//         {
//           CGadgetStdSetup * pSetupDlg = ( CGadgetStdSetup* ) m_SetupObject ;
//           pSetupDlg->SetCellInt( "Gain_dBx10" , pProp->LastValue.iInt ) ;
//         }
  return true ;
}

int Basler::GetShutter_us( BaslerCamProperty * pProp )
{
  if ( m_Camera.ExposureAuto.IsValid() && m_Camera.ExposureAuto.IsReadable() )
  {
    pProp->m_bAuto = ( m_Camera.ExposureAuto.GetValue() == ExposureAuto_Continuous ) ;
    pProp->LastValue.bBool = pProp->m_bAuto ;
  }
  if ( m_Camera.ExposureTime.IsValid() )
  {
    pProp->LastValue.i64Int = pProp->LastValue.iInt =
      ROUND( pProp->LastValue.dDouble = m_LastEmbedInfo.m_dExposure_us
      = m_Camera.ExposureTime.GetValue() ) ;
  }
  else if ( m_Camera.ExposureTimeAbs.IsValid() )
  {
    pProp->LastValue.i64Int = pProp->LastValue.iInt =
      ROUND( pProp->LastValue.dDouble =
      m_LastEmbedInfo.m_dExposure_us = m_Camera.ExposureTimeAbs.GetValue() );
  }
  else if ( m_Camera.ExposureTimeRaw.IsValid() )
  {
    pProp->LastValue.iInt = ( int ) ( pProp->LastValue.i64Int = m_Camera.ExposureTimeRaw.GetValue() ) ;
    m_LastEmbedInfo.m_dExposure_us = ( double ) pProp->LastValue.i64Int ;
  }
  return pProp->LastValue.iInt ;
}

int Basler::SetShutter_us( BaslerCamProperty * pProp , int iVal_us , bool bAuto )
{
  pProp->m_bAuto = bAuto ;
  if ( bAuto )
  {
    if ( m_Camera.ExposureAuto.IsValid() && m_Camera.ExposureAuto.IsWritable() )
    {
      m_Camera.ExposureAuto.SetValue( ExposureAuto_Continuous ) ;
      return 0 ;
    }
    else
    {
      pProp->m_bAuto = false ;
      return GetShutter_us( pProp ) ;
    }
  }
  else
  {
    if ( m_Camera.ExposureAuto.IsValid() && m_Camera.ExposureAuto.IsWritable() )
    {
//       if ( m_bExpAfterScanSettings )
//       {
//         m_bExpAfterScanSettings = false ;
//         return pProp->LastValue.iInt ;
//       }
//       else
//       {
        if ( m_Camera.ExposureAuto.GetValue() == ExposureAuto_Continuous )
          m_Camera.ExposureAuto.SetValue( ExposureAuto_Off ) ;
//      }
    }
  }
  if ( iVal_us < pProp->LastValue.dMin )
    iVal_us = (int)ceil( pProp->LastValue.dMin ) ;
  else if ( pProp->LastValue.dMax < iVal_us )
    iVal_us = ( int ) floor( pProp->LastValue.dMax );

  if ( m_Camera.ExposureTime.IsValid() && m_Camera.ExposureTime.IsWritable() )
  {
    m_Camera.ExposureTime.SetValue( ( double ) iVal_us ) ;
    pProp->LastValue.dDouble = m_LastEmbedInfo.m_dExposure_us =
      ( double ) ( pProp->LastValue.i64Int = pProp->LastValue.iInt = iVal_us ) ;
  }
  else if ( m_Camera.ExposureTimeAbs.IsValid() && m_Camera.ExposureTimeAbs.IsWritable() )
  {
    m_Camera.ExposureTimeAbs.SetValue( ( double ) iVal_us ) ;
    pProp->LastValue.dDouble = m_LastEmbedInfo.m_dExposure_us =
      ( double ) ( pProp->LastValue.i64Int = pProp->LastValue.iInt = iVal_us ) ;
  }
  else if ( m_Camera.ExposureTimeRaw.IsValid() && m_Camera.ExposureTimeRaw.IsWritable() )
  {
    m_Camera.ExposureTimeRaw.SetValue( iVal_us ) ;
    pProp->LastValue.i64Int = pProp->LastValue.iInt = iVal_us ;
    m_LastEmbedInfo.m_dExposure_us = ( double ) iVal_us ;
  }
  return pProp->LastValue.iInt ;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Basler::Basler()
  : CDevice( SYNC_INPUT | CTRL_INPUT , 32 , 256 , "Basler" )
{
  PylonInitialize();
  m_bTimeOutDisable = false;
  double dStart = GetHRTickCount();
  m_dwSerialNumber = m_dwConnectedSerialNumber = m_dwInitializedSerialNumber = 0;
  m_CurrentROI = CRect( 0 , 0 , 640 , 480 );
  m_pixelFormat = PixelFormat_Mono8;
  m_uiPacketSize = 960;
  m_FormatNotSupportedDispayed = false;
  m_GadgetInfo = "Basler gadget";
  m_pEventsOutput = new COutputConnector( transparent ) ;
  m_WaitEventFrameArray[ 0 ] = m_evFrameReady = CreateEvent( NULL , FALSE , FALSE , NULL );
  m_WaitEventBusChangeArr[ 1 ] = m_WaitEventFrameArray[ 1 ] = m_evExit;
  m_WaitEventBusChangeArr[ 0 ] = m_evBusChange = CreateEvent( NULL , FALSE , FALSE , NULL );
  m_evControlRequestFinished = CreateEvent( NULL , FALSE , FALSE , NULL );
  m_hCamAccessMutex = CreateMutex( NULL , FALSE , NULL );
  m_continueGrabThread = false;
  m_disableEmbeddedTimeStamp = false;
  //m_isSelectingNewCamera = false ;
  m_dLastStartTime = 0.;
  m_pNewFrame = NULL;
  m_pNewImage = NULL;
  m_hGrabThreadHandle = NULL;
  m_BusEvents = 0;
  m_bCamerasEnumerated = false;
  //m_cbArrivalHandle = NULL ;
  //m_cbRemovalHandle = NULL ;
  //m_cbResetHandle = NULL ;
  m_iNoCameraStatus = 0;
  m_bInitialized = false;
  m_bStopInitialized = false;
  m_bIsRunning = false;
  m_bCameraRemoved = false ;
  m_bRescanCameras = true;
  m_iNNoSerNumberErrors = 0;
  //m_IntfType = INTERFACE_UNKNOWN ;
  m_iWBRed = m_iWBlue = 512;
  m_bLocalStopped = FALSE;
  m_TriggerMode = TriggerMode_Off ;
  m_dLastInCallbackTime = 0.;
  m_dwNArrivedEvents = 0;
  m_iFPSx10 = 150; // 15 FPS
  m_iRestartTimeOut_ms = 0;
  m_iSaveSettingsState = -1;
  m_iNSkipImages = 0;
  m_bMaxPacketSize = FALSE;
  m_uiSavedPacketSize = 0;
  m_bEmbedFrameInfo = FALSE;
  m_pConfHandler = NULL ;
  m_iTriggerModeIndex = -1;
  m_bInitialScanDone = false ;
  m_pImageEventHandle = NULL ;
  m_pEventHandler = NULL ;
  m_bBaslerShutDown = false ;
  m_sSerialNumber = _T( "-1" ) ;

  ::memset( &m_BMIH , 0 , sizeof( BITMAPINFOHEADER ) );
  ::memset( &m_RealBMIH , 0 , sizeof( BITMAPINFOHEADER ) );
#ifdef _DEBUG
  m_iMutexTakeCntr = 0;
#endif
  if ( DriverInit() )
  {
    bool bRes = m_ConfigMutex.Lock( 2000
#ifdef _DEBUG
      , "BaslerConstructor"
#endif
    );
    if ( bRes )
    {
      m_iGadgetCount++;
      m_ConfigMutex.Unlock();
    }
    else
    {
      m_iGadgetCount++;
      SENDERR( "Can't take ConfigMutex" );
    }
    if ( !m_pBaslerWatcher )
    {
      m_pBaslerWatcher = new BaslerWatcher( this ) ;
      m_pBaslerWatcher->Create() ;
      m_pBaslerWatcher->Resume() ;
    }
    else
      m_pBaslerWatcher->RegisterGadget( this ) ;
  }

  m_EventCorresp.clear();
  m_EventCorresp.insert( make_pair( "ExposureEnd" , EventSelector_ExposureEnd ) );
  m_EventCorresp.insert( make_pair( "FrameStart" , EventSelector_FrameStart ) );
  m_EventCorresp.insert( make_pair( "FrameBurstStart" , EventSelector_FrameBurstStart ) );
  m_EventCorresp.insert( make_pair( "FrameStartOvertrigger" , EventSelector_FrameStartOvertrigger ) );
  m_EventCorresp.insert( make_pair( "FrameBurstStartOvertrigger" , EventSelector_FrameBurstStartOvertrigger ) );
  m_EventCorresp.insert( make_pair( "CriticalTemperature" , EventSelector_CriticalTemperature ) );
  m_EventCorresp.insert( make_pair( "OverTemperature" , EventSelector_OverTemperature ) );
  m_EventCorresp.insert( make_pair( "FrameStartWait" , EventSelector_FrameStartWait ) );
  m_EventCorresp.insert( make_pair( "FrameBurstStartWait" , EventSelector_FrameBurstStartWait ) );


  double dBusyTime = GetHRTickCount() - dStart;
  TRACE( "Basler::Basler: Start %g , Busy %g\n" , dStart , dBusyTime );
}

Basler::~Basler()
{
  if ( m_pBaslerWatcher )
  {
    m_pBaslerWatcher->Destroy() ;
    delete m_pBaslerWatcher ;
    m_pBaslerWatcher = NULL ;
  }
  if ( m_hGrabThreadHandle )
  {
    DWORD dwRes = WaitForSingleObject( m_hGrabThreadHandle , 1000 );
    ASSERT( dwRes == WAIT_OBJECT_0 );
    dwRes = WaitForSingleObject( m_hCamAccessMutex , 1000 );
    ASSERT( dwRes == WAIT_OBJECT_0 );
    CloseHandle( m_hCamAccessMutex );
  }
  if ( m_pConfHandler )
  {
    delete m_pConfHandler ;
    m_pConfHandler = NULL ;
  }

  bool bRes = m_ConfigMutex.Lock( 2000
#ifdef _DEBUG
    , m_GadgetInfo + "~Basler"
#endif
  );
  if ( bRes )
  {
    m_iGadgetCount--;
    m_ConfigMutex.Unlock();
  }
  else
  {
    m_iGadgetCount--;
    SENDERR( "Can't take ConfigMutex" );
  }
  try
  {
    PylonTerminate();
  }
  catch ( const GenericException& e )
  {
    // Error handling.
    FXString ErrMsg;
    ErrMsg.Format( _T( "Pylon Terminate exception %s - %s" ) ,
      (LPCTSTR) m_sSerialNumber , e.GetDescription() );
    DEVICESENDERR_1( "Fatal error: %s" , (LPCTSTR) ErrMsg );
    TRACE( "\n  %s" , (LPCTSTR) ErrMsg ) ;
  }
}

BaslerCamProperty * Basler::GetThisCamProperty( FXCamPropertyType Type )
{
  for ( int i = 0; i < m_ThisCamProperty.GetSize(); i++ )
  {
    if ( m_ThisCamProperty[ i ].id == Type )
      return &m_ThisCamProperty[ i ] ;
  }
  return NULL ; // no such property
}


FXCamPropertyType Basler::GetThisCamPropertyId( LPCTSTR name )
{
  int i;
  FXCamPropertyType retV = UNSPECIFIED_PROPERTY_TYPE;
  for ( i = 0; i < m_ThisCamProperty.GetSize(); i++ )
  {
    if ( m_ThisCamProperty[ i ].name.CompareNoCase( name ) == 0 )
    {
      retV = m_ThisCamProperty[ i ].id;
      break;
    }
  }
  return retV;
}

int Basler::GetThisCamPropertyIndex( LPCTSTR name )
{
  int i;
  for ( i = 0; i < m_ThisCamProperty.GetSize(); i++ )
  {
    if ( m_ThisCamProperty[ i ].name.CompareNoCase( name ) == 0 )
      return i;
  }
  return -1;
}

int Basler::GetThisCamPropertyIndex( FXCamPropertyType Type )
{
  int i;
  for ( i = 0; i < m_ThisCamProperty.GetSize(); i++ )
  {
    if ( m_ThisCamProperty[ i ].id == Type )
      return i;
  }
  return -1; // bad index
};
LPCTSTR  Basler::GetThisCamInDeviceName( FXCamPropertyType Type )
{
  int i;
  for ( i = 0; i < m_ThisCamProperty.GetSize(); i++ )
  {
    if ( m_ThisCamProperty[ i ].id == Type )
    {
      return m_ThisCamProperty[ i ].m_InDeviceName;
    }
  }
  return NULL; // bad index
};

LPCTSTR Basler::GetThisCamPropertyName( FXCamPropertyType Type )
{
  int iIndex = GetThisCamPropertyIndex( Type );
  if ( iIndex < 0 )
    return _T( "Unknown Name" );
  else
    return (LPCTSTR) m_ThisCamProperty[ iIndex ].name;
  {
  }
};

FXCamPropertyType Basler::GetThisCamPropertyType( int iIndex )
{
  if ( iIndex < 0 || iIndex >= m_ThisCamProperty.GetCount() )
    return UNSPECIFIED_PROPERTY_TYPE;
  return m_ThisCamProperty[ iIndex ].id;
};

LPCTSTR Basler::GetThisCamPropertyName( int iIndex )
{
  if ( iIndex < 0 || iIndex >= m_ThisCamProperty.GetCount() )
    return _T( "Unknown Name" );
  return (LPCTSTR) m_ThisCamProperty[ iIndex ].name;
};

PropertyDataType Basler::GetPropertyDataType( int iIndex )
{
  if ( iIndex < 0 || iIndex >= m_ThisCamProperty.GetCount() )
    return PDTUnknown;
  return m_ThisCamProperty[ iIndex ].m_DataType;
};

PropertyDataType Basler::GetPropertyDataType( FXCamPropertyType Type )
{
  int iIndex = GetThisCamPropertyIndex( Type );
  if ( iIndex < 0 )
    return PDTUnknown;
  return m_ThisCamProperty[ iIndex ].m_DataType;
};

PropertyDataType Basler::GetPropertyDataType( LPCTSTR name )
{
  int iIndex = GetThisCamPropertyIndex( name );
  if ( iIndex < 0 )
    return PDTUnknown;
  return m_ThisCamProperty[ iIndex ].m_DataType;
};

PropertyDataType Basler::GetType( INode * pNode )
{
  if ( CIntegerPtr( pNode ) )
    return PDTInt ;
  if ( CFloatPtr( pNode ) )
    return PDTFloat ;
  if ( CEnumerationPtr( pNode ) )
    return PDTEnum ;
  if ( CCommandPtr( pNode ) )
    return PDTCommand ;
  if ( CStringPtr( pNode ) )
    return PDTString ;
  if ( CBooleanPtr( pNode ) )
    return PDTBool ;
  if ( CCategoryPtr( pNode ) )
    return PDTCategory ;
  return PDTUnknown ;
}


int Basler::WriteToFile( char * pData , int iDataLen , FILE * pSaveFile )
{
  //  char Buf[ 10000 ] ;
  if ( !pSaveFile || !iDataLen /*|| iDataLen >= sizeof( Buf )*/ )
    return 0 ;
  void * pVoidData = (void*) pData ;
  //if ( sizeof(char) == 2 )
  //{
  //  for ( int i = 0; i < iDataLen; i++ )
  //    Buf[ i ] = pData[ i ] & 0xff ;
  //  pVoidData = (void*) Buf ;
  //}
  int iWritten = (int) fwrite( pVoidData , 1 , iDataLen , pSaveFile ) ;
  return iWritten ;
}


bool Basler::DriverInit()
{
  return EnumCameras();
}

void Basler::ShutDown()
{
  m_bBaslerShutDown = true ;
  if ( m_pEventsOutput )
  {
    delete m_pEventsOutput ;
    m_pEventsOutput = NULL ;
  }
  CDevice::ShutDown();
  if ( m_hGrabThreadHandle )
  {
    bool bRes = CamCNTLDoAndWait( BASLER_EVT_SHUTDOWN , 1000 );
    DWORD dwRes = WaitForSingleObject( m_hGrabThreadHandle , 2000 );
    ASSERT( dwRes == WAIT_OBJECT_0 ) ;
  }
  FxReleaseHandle( m_evFrameReady );
  FxReleaseHandle( m_evBusChange );
}

void Basler::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  if ( !pParamFrame )
    return;
  if ( m_Camera.IsOpen() )
  {
    CTextFrame* tf = pParamFrame->GetTextFrame( DEFAULT_LABEL );
    if ( tf )
    {
      bool bNoAnswer = false ;
      LPCTSTR pLabel = tf->GetLabel() ;
      FXParser pk = ( LPCTSTR ) tf->GetString();
      if ( _tcsstr( pLabel , "CameraReset" ) == pLabel )
      {
        if ( m_Camera.DeviceReset.IsValid() )
        {
          try
          {
            DeviceStop();
            Sleep( 50 ) ;
            m_Camera.DeviceReset.Execute() ;
            Sleep( 200 ) ;
            CameraClose() ;
            CameraInit() ;
          }
          catch ( const GenericException& e )
          {
            // Error handling.
            FXString ErrMsg;
            ErrMsg.Format( _T( "Exception on reset of device %s - %s" ) ,
              ( LPCTSTR ) m_sSerialNumber , e.GetDescription() );
            DEVICESENDERR_1( "Fatal error: %s" , ( LPCTSTR ) ErrMsg );
          }
        }
      }
      else if ( _tcsicmp( pLabel , _T( "FireSoftTrigger" ) ) == 0 )
      {
        if ( ( m_Camera.TriggerMode.GetValue() == TriggerMode_On )
          && ( m_Camera.TriggerSource.GetValue() == TriggerSource_Software )
          && m_Camera.TriggerSoftware.IsValid() )
        {
          m_Camera.TriggerSoftware.Execute() ;
        }
  //       if ( m_bSoftwareTriggerMode && m_Camera.TriggerSoftware.IsValid() )
  //       {
  //         m_Camera.TriggerSoftware.Execute() ;
  //         bNoAnswer = true ;
  //       }
  //       else
  //         pk = "Trigger mode is not supported" ;
      }
      else if ( _tcsicmp( pLabel , _T( "GetCameraName" ) ) == 0 )
      {
        if ( m_Camera.DeviceUserID.IsValid() && m_Camera.DeviceUserID.IsReadable() )
        {
          FXString UserId = m_Camera.DeviceUserID.GetValue().c_str() ;
          pk = "CameraName=" ;
          pk += UserId + ';' ;
        }
      }
      else
      {
        FXString cmd , subcmd;
        FXString param;
        FXSIZE pos = 0;
        pk.GetWord( pos , cmd );

        if ( cmd.CompareNoCase( "list" ) == 0 )
        {
          pk.Empty();
          FXAutolock al( m_PropertyListLock , "Basler::AsyncTransaction" );
          for ( FXSIZE i = 0; i < m_ThisCamProperty.GetSize(); i++ )
          {
            //FXParser
            pk += m_ThisCamProperty[ i ].name; pk += "\r\n";
          }
        }
        else if ( ( cmd.CompareNoCase( "get" ) == 0 ) && ( pk.GetWord( pos , subcmd ) ) )
        {
          if ( subcmd == "camlist" )
          {
            if ( !m_bCamerasEnumerated || !m_iCamNum )
              pk = "No cameras";
            else
            {
              FXAutolock al( m_ConfigLock );
              FXString CamList , CamDescription;
              CamList.Format( "%d cameras: " , m_iCamNum );
              bool bCamFound = false;
              for ( int i = 0; i < m_iCamNum; i++ )
              {
                TCHAR cMark = _T( '+' ); // sign, that camera is free
                if ( m_sSerialNumber != m_DevicesInfo[ i ].szSerNum )
                {
                  for ( int j = 0; j < m_BusyCameras.GetCount(); j++ )
                  {
                    if ( m_sSerialNumber != m_BusyCameras[ j ] )
                    {
                      if ( m_BusyCameras[ j ] == m_DevicesInfo[ i ].szSerNum )
                      {
                        cMark = _T( '-' ); // sign, that camera is busy by other gadget
                        break;
                      }
                    }
                  }
                }
                else
                {
                  cMark = _T( '!' );// this gadget camera sign
                  bCamFound = true;
                }
                CamDescription.Format( "%c%s_%s; " , cMark ,
                  ( LPCTSTR ) m_DevicesInfo[ i ].szSerNum ,
                  ( LPCTSTR ) m_DevicesInfo[ i ].model );
                CamList += CamDescription;
              }
              if ( !bCamFound && !m_sSerialNumber.IsEmpty() )
              {
                CamDescription.Format( "?%s;" , m_sSerialNumber );
                CamList += CamDescription;
              }
              pk = CamList;
            }
          }
          else if ( subcmd == "properties" )
          {
            bool bRes = PrintProperties( pk );
            if ( !bRes )
              pk = "error";
          }
          else if ( subcmd == "io_status" )
          {
            if ( m_Camera.IsOpen() && m_Camera.LineStatusAll.IsReadable() )
            {
              int IO_Status = ( int ) m_Camera.LineStatusAll.GetValue() ;
              pk.Format( "io_status=0X%X;" , IO_Status ) ;
            }
          }
          else
          {
            unsigned id = GetPropertyId( subcmd );
            if ( id != WRONG_PROPERTY )
            {
              INT_PTR value;
              bool bauto;
              bool bRes = GetCameraProperty( id , value , bauto );
              if ( bRes )
              {
                if ( IsDigitField( id ) )
                {
                  if ( bauto )
                    pk = "auto";
                  else
                    pk.Format( "%d" , value );
                }
                else
                  pk = ( LPCTSTR ) value;
              }
              else
                pk = "error";
            }
            else
              pk = "error";
          }

        }
        else if ( ( cmd.CompareNoCase( "set" ) == 0 )
          && ( pk.GetWord( pos , subcmd ) )
          && ( pk.GetParamString( pos , param ) ) )
        {
          if ( subcmd == "properties" )
          {
            bool bInvalidate = false;
            bool bRes = ScanProperties( param , bInvalidate );
            pk = ( bRes ) ? "OK" : "error";
          }
          else if ( subcmd == "out_control" )
          {

          }
          else if ( subcmd == "FireSoftTrigger" )
          {
            if ( m_TriggerMode == TriggerMode_On )
              m_Camera.TriggerSoftware.Execute() ;
          }
          else
          {
            bool bRes = false;
            int iIndex = GetPropertyIndex( subcmd );
            if ( iIndex >= 0 )
            {
              INT_PTR value = 0;
              bool bauto = false , Invalidate = false;
              if ( m_ThisCamProperty[ iIndex ].id == FRAME_RATE )
              {
                FXString szNewFrameRate;
                if ( param[ 0 ] == _T( '+' ) || param[ 0 ] == _T( '-' ) )
                {
                  if ( m_bEmbedFrameInfo )
                  {
                    double dAddition = atof( param );
                    double dRatio = dAddition / ( double ) m_dwLastFramePeriod;
                    double dNewFrameRate = ( 1. - dRatio ) * m_fLastRealFrameRate;
                    szNewFrameRate.Format( _T( "%8.2f" ) , dNewFrameRate );
                    value = ( INT_PTR ) ( LPCTSTR ) szNewFrameRate;
                  }
                }
                else
                  value = ( INT_PTR ) ( LPCTSTR ) param;
                if ( value )
                  bRes = SetCameraProperty( iIndex , value , bauto , Invalidate );
              }
              else
              {
                if ( m_ThisCamProperty[ iIndex ].m_DataType == PDTInt )
                {
                  if ( param.Find( "auto" ) >= 0 )
                    bauto = true;
                  else
                    value = atoi( param );
                }
                else
                  value = ( INT_PTR ) ( LPCTSTR ) param;
                bool bWasStopped = m_bWasStopped;
                m_bWasStopped = false;
                bRes = SetCameraProperty( iIndex , value , bauto , Invalidate );
                if ( !bWasStopped  && m_bWasStopped )
                  CamCNTLDoAndWait( BASLER_EVT_INIT | BASLER_EVT_START_GRAB );
                m_bWasStopped = bWasStopped;
              }
              pk = ( bRes ) ? "OK" : "error";
            }
            else
            {
              pk = "error: no such property - ";
              pk += subcmd;
            }
          }
        }
        else if ( subcmd.MakeLower() == "camera" )
        {
          subcmd += "=" ;
          subcmd += param + ';' ; //
          bool bInvalidate = false ;
          ScanProperties( subcmd , bInvalidate ) ;
          if ( bInvalidate && GetSetupObject() )
            ( ( CGadgetStdSetup* ) GetSetupObject() )->Update() ;
        }
        else
        {
          pk = "List of available commands:\r\n"
            "list - return list of properties\r\n"
            "get <item name> - return current value of item\r\n"
            "set <item name>(<value>) - change an item\r\n";
        }
      }
      if ( !bNoAnswer )
      {
        CTextFrame* retV = CTextFrame::Create( pk );
        retV->ChangeId( NOSYNC_FRAME );
    //     FXString Label ;
    //     GetGadgetName( Label ) ;
        retV->SetLabel( m_Name ) ; // m_Name is gadget name in FXWorker class

        PutFrame( m_pControl , retV ) ;
      }
    }
  }
  pParamFrame->Release( pParamFrame );
}

bool Basler::CameraInit()
{
#ifdef _DEBUG
  if ( CamCNTLDoAndWait( BASLER_EVT_INIT , 1500000 ) )
#else
  if ( CamCNTLDoAndWait( BASLER_EVT_INIT , 5000 ) )
#endif
  {
    return true;
    //     if ( m_pCamera )
    //       return BuildPropertyList();
  }
  return false;
}

bool Basler::BaslerCamInit()
{
  BOOL bInLoop = GetGadgetName( m_GadgetInfo );
  DriverInit();

  if ( m_Camera.IsOpen() )
    BaslerCamClose();

  if ( !IsSNLegal( m_sSerialNumber ) )
    return false ;

  //FXAutolock al( m_LocalConfigLock ) ;


  for (int i = 0; i < m_iCamNum; i++)
  {
    if ( m_bSelectByGivenName )
    {
      if ( m_sCameraGivenName == m_CamInfo[ i ].m_sGivenName )
      {
        m_CurrentDevice = i;
        m_bCameraExists = true;
        m_sSerialNumber = m_CamInfo[ i ].szSerNum ;
        break ;
      }
    }
    else if ( m_sSerialNumber == m_CamInfo[ i ].szSerNum ) 
    {
      m_bCameraExists = true ;
      m_sCameraGivenName = m_CamInfo[ i ].m_sGivenName ;
      m_CurrentDevice = i ;
      break;
    }
  }

  if ( m_sSerialNumber.IsEmpty() || !m_iCamNum || !m_bCameraExists
    || ((m_CurrentDevice != 0xffffffff) && (m_CurrentDevice >= (unsigned) m_iCamNum)) )
  {  // nothing to connect
    if ( GetHRTickCount() - m_dLastReportTime > 1000. )
    {
      SEND_GADGET_INFO( "Assigned camera %s is not connected" , ( LPCTSTR ) m_sSerialNumber ) ;
      m_dLastReportTime = GetHRTickCount() ;
    }
    return false;
  }

  CAutoLockMutex alm( m_ConfigMutex , 5000
#ifdef _DEBUG
    , m_GadgetInfo
#endif
  );
  if ( !alm.IsLocked() )
  {
    SEND_GADGET_ERR( "Can't get ConfigMutex" );
    return false;

  }
  CTlFactory& Fact = CTlFactory::GetInstance();
  DeviceInfoList InfoForAll;
  size_t It = 0;
  CDeviceInfo * pDev = NULL;
  try
  {
    Fact.EnumerateDevices( InfoForAll );
    m_NConnectedDevices = (int) InfoForAll.size();
    ASSERT( m_NConnectedDevices < MAX_DEVICESNMB );
    for ( ; It < InfoForAll.size(); ++It )
    {
      pDev = &InfoForAll[ It ];
      if ( m_bSelectByGivenName )
      {
        if ( pDev->GetUserDefinedName().find( m_sCameraGivenName ) != string::npos )
        {
          m_sSerialNumber = pDev->GetSerialNumber().c_str() ;
          int iPos = (int)pDev->GetUserDefinedName().find( _T( '(' ) ) ;
          if ( iPos == string::npos )
            m_sCameraGivenName = pDev->GetUserDefinedName().c_str() ;
          else
            m_sCameraGivenName = 
              pDev->GetUserDefinedName().substr( 0 , iPos ).c_str() ;
          break ;
        }
      }
      else
      {
        if ( m_sSerialNumber == pDev->GetSerialNumber().c_str() )
        {
          int iPos = ( int ) pDev->GetUserDefinedName().find( _T( '(' ) ) ;
          if ( iPos == string::npos )
            m_sCameraGivenName = pDev->GetUserDefinedName().c_str() ;
          else
            m_sCameraGivenName =
            pDev->GetUserDefinedName().substr( 0 , iPos ).c_str() ;
          break;
        }
      }
    }
    if ( It >= InfoForAll.size() )
    {
      if ( ( m_dwSerialNumber != 0) && (m_sSerialNumber != _T("-1")) )
        SEND_GADGET_ERR( "Can't find camera: %s with Name %s" , 
          (LPCTSTR) m_sSerialNumber , ( LPCTSTR ) m_sCameraGivenName ,
          (LPCTSTR) m_sCameraGivenName );
      return false;
    }
  }
  catch ( const GenericException& e )
  {
    // Error handling.
    FXString ErrMsg;
    ErrMsg.Format( _T( "Exception for SN %s, Name %s, ERROR: %s" ) ,
      (LPCTSTR) m_sSerialNumber , e.GetDescription() );
    DEVICESENDERR_1( "Fatal error: %s" , (LPCTSTR) ErrMsg );
    return false;
  }
  try
  {
    string Class = pDev->GetDeviceClass();
    string Model = pDev->GetModelName() ;
    string Interface = pDev->GetInterface() ;
    m_Camera.GrabCameraEvents = true ;

    m_Camera.Attach( Fact.CreateDevice( *pDev ) );
    if ( Class == _T( "BaslerGigE" ) )
    {
      CHeartbeatHelper HBHelper( m_Camera ) ;
#ifdef _DEBUG
#define HB_TIMEOUT 60000
#else
#define HB_TIMEOUT 2000
#endif
      HBHelper.SetValue( HB_TIMEOUT ) ;
    }
    if ( m_pConfHandler )
    {
      delete m_pConfHandler ;
      m_pConfHandler = NULL ;
    }
    m_pConfHandler = new CSampleConfigurationEventHandler( this , CameraRemovedCB ) ;
  }
  catch ( const GenericException& e )
  {
    // Error handling.
    FXString ErrMsg;
    ErrMsg.Format( _T( "Can't create device %s - %s" ) ,
      (LPCTSTR) m_sSerialNumber , e.GetDescription() );
    DEVICESENDERR_1( "Fatal error: %s" , (LPCTSTR) ErrMsg );
    return false;
  }
  m_sModelName = pDev->GetModelName().c_str();

  m_iNNoSerNumberErrors = 0;

  m_Status.Empty();

  if ( m_bSelectByGivenName )
    m_CameraID.Format( "%s(%s)_%s" , ( LPCTSTR ) m_sCameraGivenName , 
    ( LPCTSTR ) m_sSerialNumber , ( LPCTSTR ) m_sModelName );
  else
    m_CameraID.Format( "%s_%s" , (LPCTSTR)m_sSerialNumber , (LPCTSTR)m_sModelName );

  //   RegisterCallbacks() ;
  if ( m_Status.IsEmpty() )
  {
    if ( bInLoop )
      m_Status.Format( "Connected SN=%s Name=%s Model=%s" , 
        (LPCTSTR)m_sSerialNumber , 
        ( LPCTSTR ) m_sCameraGivenName , (LPCTSTR)m_sModelName ) ;
  }
  if ( !m_Status.IsEmpty() )
  {
    DEVICESENDINFO_0( m_Status );
    m_Status.Empty();
  }
  m_bInitialized = true;
  FXAutolock al( m_ConfigLock );
  m_sConnectedSerialNumber = m_sSerialNumber;
  int i = 0;
  for ( ; i < m_BusyCameras.GetCount(); i++ )
  {
    if ( m_BusyCameras[ i ] == m_sSerialNumber )
      break;
  }
  if ( i >= m_BusyCameras.GetCount() )
    m_BusyCameras.Add( m_sSerialNumber );

  try
  {
    m_Camera.Open();
  }
  catch (const GenericException& e)
  {
    // Error handling.
    FXString ErrMsg;
    ErrMsg.Format( _T( "Can't Open Camera %s - %s" ) ,
      ( LPCTSTR ) m_sSerialNumber , e.GetDescription() );
    DEVICESENDERR_1( "Fatal error: %s" , ( LPCTSTR ) ErrMsg );
    return false;
  }

//   if ( m_Camera.EventSelector.IsValid()
//     && m_Camera.EventNotification.IsValid() )
//     //     if ( m_sModelName.Find( _T( "da" ) ) != 0 ) // dart doesn't support events
//   {
//     //m_pImageEventHandle = new CSampleImageEventHandler(this);
//     // Camera event processing must be activated first, the default is off.
//     // 
//     m_pEventHandler = NULL ;
//     try
//     {
//       m_Camera.RegisterConfiguration( new CAcquireContinuousConfiguration() ,
//         RegistrationMode_ReplaceAll , Cleanup_Delete );
// 
//       // The next is commented, because we don't need image handler
//       //m_Camera.RegisterImageEventHandler(m_pImageEventHandle, RegistrationMode_Append, Cleanup_None);
//       
// //      VersionInfo VerInfo = m_Camera.GetSfncVersion() ;
// //      m_Camera.GrabCameraEvents = false ;
//       if (m_Camera.EventSelector.IsWritable())
//       {
//         m_pEventHandler = new CameraEventHandler( this );
// 
// //         if (VerInfo < Sfnc_2_0_0)
// //         {
//           m_Camera.RegisterCameraEventHandler( m_pEventHandler , "EventExposureEndData"  /*"ExposureEnd"*/ ,
//             EventSelector_ExposureEnd , RegistrationMode_ReplaceAll , Cleanup_None );
//           m_Camera.EventSelector.SetValue( EventSelector_ExposureEnd ) ;
//           EventSelectorEnums ReadValSE = m_Camera.EventSelector.GetValue() ;
//           m_Camera.EventNotification.SetValue( EventNotification_On  ) ;
//           EventNotificationEnums ReadValNE = m_Camera.EventNotification.GetValue() ;
//           //int64_t i64Val = m_Camera.EventExposureEndTimestamp.GetValue() ;
//           //         m_Camera.RegisterCameraEventHandler( m_pEventHandler , "FrameStartOvertriggerEventData" ,
//           //           EventSelector_FrameStartOvertrigger , RegistrationMode_Append , Cleanup_None );
//           //         m_Camera.RegisterCameraEventHandler( m_pEventHandler , /*"FrameStart"*/ "FrameStartEventData" ,
//           //           EventSelector_FrameStart , RegistrationMode_Append , Cleanup_None );
//           //         m_Camera.GrabCameraEvents = true;
//           SEND_GADGET_INFO( "Camera %s event handlers registered" , m_sSerialNumber );
//           //SEND_GADGET_INFO( "Camera %s event handlers are not registered" , m_sSerialNumber );
// //         }
// //         else
// //         {
// //           m_Camera.RegisterCameraEventHandler( m_pEventHandler , "EventExposureEndData" ,
// //             EventSelector_ExposureEnd , RegistrationMode_ReplaceAll , Cleanup_None );
// //           //         m_Camera.RegisterCameraEventHandler( m_pEventHandler , "EventFrameStartOvertriggerData" , 
// //           //           EventSelector_FrameStartOvertrigger , RegistrationMode_Append , Cleanup_None );
// //           //         m_Camera.RegisterCameraEventHandler( m_pEventHandler , "EventFrameStart" ,
// //           //           EventSelector_FrameStart , RegistrationMode_Append , Cleanup_None );
// //           SEND_GADGET_INFO( "Camera %s event handlers are not registered" , m_sSerialNumber );
// //         }
//       }
//     }
//     catch ( const GenericException& e )
//     {
//       // Error handling.
//       FXString ErrMsg;
//       ErrMsg.Format( _T( "Can't register events for camera %s - %s" ) ,
//         (LPCTSTR) m_sSerialNumber , e.GetDescription() );
//       DEVICESENDERR_1( "Fatal error: %s" , (LPCTSTR) ErrMsg );
// 
//       if (m_pEventHandler)
//       {
//         m_Camera.RegisterCameraEventHandler( m_pEventHandler , "EventExposureEndData"  /*"ExposureEnd"*/ ,
//           EventSelector_ExposureEnd , RegistrationMode_ReplaceAll , Cleanup_None );
// 
//         delete m_pEventHandler ;
//         m_pEventHandler = NULL ;
//       }
//     }
//   }
//   else
    m_pEventHandler = NULL;

  if ( BaslerBuildPropertyList() )
  {
    // Restore camera parameters from saved in camera in set 1
    TRACE( "Basler::BaslerCamInit - Camera %s initialized\n" ,
      (LPCTSTR) m_sSerialNumber ) ;
    SEND_GADGET_ERR( "Camera Initialized, SN=%s Model=%s" , m_sSerialNumber , m_sModelName );
    m_bCameraRemoved = false ;
    m_dwConnectedSerialNumber = atol( m_sSerialNumber ) ;
  }
  else
    BaslerCamClose();

  return true;
}

void Basler::BaslerDeleteCam()
{
  if ( m_Camera.IsOpen() )
  {
    FXAutolock al( m_ConfigLock );
    bool bOldSFNC = false ;
    try
    {
      bOldSFNC = (m_Camera.GetSfncVersion() < Sfnc_2_0_0);
    }
    catch ( const GenericException &e )
    {
      // Error handling
      FXString CatchData ;
      CatchData.Format( _T( "   Exception catch for on GetSfncVersion: %s\n" ) ,
        e.GetDescription() ) ;
      SEND_GADGET_ERR( (LPCTSTR) CatchData );
      TRACE( "\n   %s" , (LPCTSTR) CatchData ) ;
    }
    try
    {
      m_Camera.DestroyDevice()  ;
    }
    catch ( const GenericException &e )
    {
      // Error handling
      FXString CatchData ;
      CatchData.Format( _T( "   Exception catch for on DestroyDevice: %s\n" ) ,
        e.GetDescription() ) ;
      SEND_GADGET_ERR( (LPCTSTR) CatchData );
      TRACE( "\n   %s" , (LPCTSTR) CatchData ) ;
    }
    m_dwConnectedSerialNumber = 0;
    m_sConnectedSerialNumber.Empty();
    for ( int i = 0; i < m_BusyCameras.GetCount(); i++ )
    {
      if ( m_BusyCameras[ i ] == m_sSerialNumber )
      {
        m_BusyCameras.RemoveAt( i );
      }
    }
    if ( m_pEventHandler )
    {
      try
      {
        if ( bOldSFNC )
        {
          m_Camera.DeregisterCameraEventHandler( m_pEventHandler , "ExposureEndEventData" );
//           m_Camera.DeregisterCameraEventHandler( m_pEventHandler , "FrameStartOvertriggerEventData" );
//           m_Camera.DeregisterCameraEventHandler( m_pEventHandler , /*"FrameStart"*/ "FrameStartEventData" );
        }
        else
        {
          m_Camera.DeregisterCameraEventHandler( m_pEventHandler , "EventExposureEndData" );
//           m_Camera.DeregisterCameraEventHandler( m_pEventHandler , "EventFrameStartOvertriggerData" );
//           m_Camera.DeregisterCameraEventHandler( m_pEventHandler , "EventFrameStart" );
        }
      }
      catch ( const GenericException &e )
      {
        // Error handling
        FXString CatchData ;
        CatchData.Format( _T( "   Exception catch for on Deregister Handlers: %s\n" ) ,
          e.GetDescription() ) ;
        SEND_GADGET_ERR( (LPCTSTR) CatchData );
        TRACE( "\n   %s" , (LPCTSTR) CatchData ) ;
      }
      delete m_pEventHandler;
      m_pEventHandler = NULL;
    }
    if ( m_pImageEventHandle )
    {
      m_Camera.DeregisterImageEventHandler( m_pEventHandler );
      delete m_pImageEventHandle;
      m_pImageEventHandle = NULL;
    }
  }
  m_bInitialized = false;
  TRACE( "Basler::BaslerDeleteCam - Camera %s deleted\n" , (LPCTSTR) m_sSerialNumber );
}


void Basler::BaslerCamClose()
{
  if ( m_Camera.IsOpen() )
  {
    BaslerCameraStop();

    BaslerDeleteCam();
    m_CallbackThreadName.Empty();
    //       m_Status.Format("CameraClose() %s: %s", bMain ? "Main" : "GUI" , m_LastError.GetDescription( ));
    //       DEVICESENDINFO_0(m_Status);
    m_CameraID = "-Basler";
    //m_CurrentCameraGUID = PGRGuid() ;
  }
  TRACE( "Basler::BaslerCamClose - Camera %s closed\n" , (LPCTSTR) m_sSerialNumber );
}
void Basler::BaslerLocalStreamStart()
{
  //if ( m_pCamera )
  //{
  //  UINT uiRead ;
  //  m_LastError = m_Camera.ReadRegister( 0x614 , &uiRead ) ;
  //  if ( m_LastError == PGRERROR_OK )
  //  {
  //    if ( uiRead == 0 ) // stopped
  //    {
  //      m_LastError = m_Camera.WriteRegister( 0x614 , 0x80000000 ) ;
  //      TRACE( "Basler::BaslerLocalStreamStart - Camera %u stream started\n" ,
  //        m_CameraInfo.serialNumber ) ;
  //    }
  //    m_bLocalStopped = false ;
  //    return ;
  //  }
  //}
  //TRACE( "Basler::BaslerLocalStreamStart - Camera %u stream not started\n" ,
  // m_CameraInfo.serialNumber ) ;
}
void Basler::BaslerLocalStreamStop()
{
  //if ( m_pCamera )
  //{
  //  UINT uiRead ;
  //  m_LastError = m_Camera.ReadRegister( 0x614 , &uiRead ) ;
  //  if ( m_LastError == PGRERROR_OK )
  //  {
  //    if ( uiRead != 0 ) // not stopped
  //    {
  //      m_LastError = m_Camera.WriteRegister( 0x614 , 0 ) ;
  //      TRACE( "Basler::BaslerLocalStreamStart - Camera %u stream stopped\n" ,
  //        m_CameraInfo.serialNumber ) ;
  //    }
  //    m_bLocalStopped = true ;
  //    return ;
  //  }
  //}
  //TRACE( "Basler::BaslerLocalStreamStart - Camera %u stream not stopped\n" , 
  //  m_CameraInfo.serialNumber ) ;
}


void Basler::CameraClose()
{
  CamCNTLDoAndWait( BASLER_EVT_RELEASE , 2000 );
}

bool Basler::DriverValid()
{

  return  (m_iCamNum != 0) /*&& (m_pCamera && m_Camera.IsConnected())*/;
}

bool Basler::DeviceStart()
{
  CDevice::DeviceStart();
  m_bRun = TRUE;

  if ( m_ControlThreadName.IsEmpty() && m_continueGrabThread && m_dwGrabThreadId )
  {
    GetGadgetName( m_ControlThreadName );
    m_ControlThreadName += _T( "_CTRL" );
    ::SetThreadName( (LPCSTR) ((LPCTSTR) m_ControlThreadName) , m_dwGrabThreadId );
  }
  bool bRes = CamCNTLDoAndWait( BASLER_EVT_START_GRAB , 1000 );
  if ( bRes  &&  m_TriggerMode >= TriggerOn )
  {
    Sleep( 200 );
    bRes = CamCNTLDoAndWait( BASLER_EVT_STOP_GRAB , 2000 );
    Sleep( 200 );
    bRes = CamCNTLDoAndWait( BASLER_EVT_START_GRAB , 1000 );
  }
  return bRes;
}

void CSampleImageEventHandler::OnImageGrabbed( CInstantCamera& camera ,
  const CGrabResultPtr& ptrGrabResult )
{
  double dStartTime = GetHRTickCount();
  if ( m_pHost->m_iNSkipImages > 0 )
  {
    m_pHost->m_iNSkipImages--;
    return;
  }

  // Is Image grabbed successfully?
  if ( !m_pHost->m_bBaslerShutDown && ptrGrabResult && ptrGrabResult->GrabSucceeded() )
  {

    // Access the image data.
    int32_t cols = ptrGrabResult->GetWidth();
    int32_t rows = ptrGrabResult->GetHeight();
    EPixelType format = ptrGrabResult->GetPixelType();
    LPBYTE data = (LPBYTE) ptrGrabResult->GetBuffer();
    if ( m_pHost->m_bStopInitialized )
    {
      TRACE( "Frame after stop initialized, cam %s\n" , (LPCTSTR) (m_pHost->m_sSerialNumber) );
    }
    m_pHost->m_GrabLock.Lock();
    if ( m_pHost->m_pNewImage )
    {
      if ( m_pHost->m_bStopInitialized )
      {
        TRACE( "Image delete after stop initialized\n" );
      }
      delete m_pHost->m_pNewImage;
      m_pHost->m_pNewImage = NULL;
    }
    pTVFrame pNew = NULL;
    unsigned iSize = rows * cols;
    switch ( format )
    {
    case PixelType_Mono8:
      {

        pNew = makeNewY8Frame( cols , rows );
      }
      break;
    case PixelType_Mono16:
      {
        iSize *= 2;
        pNew = makeNewY16Frame( cols , rows );
      }
      break;
    }
    if ( pNew )
    {
      memcpy( (LPBYTE) (pNew->lpBMIH + 1) , data , iSize );
      m_pHost->m_pNewImage = CVideoFrame::Create( pNew );
      SetEvent( m_pHost->m_evFrameReady );
      m_pHost->m_dLastFrameTime = GetHRTickCount();
      m_pHost->m_dLastInCallbackTime = m_pHost->m_dLastFrameTime - dStartTime;
      m_pHost->m_bTriedToRestart = false;
      if ( m_pHost->m_CallbackThreadName.IsEmpty() )
      {
        m_pHost->GetGadgetName( m_pHost->m_CallbackThreadName );
        m_pHost->m_CallbackThreadName += _T( "_CB" );
        DWORD dwThreadId = ::GetCurrentThreadId();
        ::SetThreadName( (LPCSTR) ((LPCTSTR) m_pHost->m_CallbackThreadName) , dwThreadId );
      }
    }

    m_pHost->m_GrabLock.Unlock();
    if ( m_pHost->m_bStopInitialized )
    {
      TRACE( "Unlock after stop initialized, cam %s\n" , (LPCTSTR) (m_pHost->m_sSerialNumber) );
    }

  }
  else
  {
    cout << "Error: " << ptrGrabResult->GetErrorCode() << " "
      << ptrGrabResult->GetErrorDescription() << endl;
  }
}

void CameraEventHandler::OnCameraEvent(
  CBaslerUniversalInstantCamera& camera ,
  intptr_t userProvidedId , GenApi::INode* pNode )
{
  FXString Description;
  switch ( (EventSelectorEnums) userProvidedId )
  {
  case EventSelector_ExposureEnd: Description = _T( "ExposureEnd" ); break;
  case EventSelector_FrameStart: Description = _T( "FrameStart" ); break;
  case EventSelector_FrameBurstStart: Description = _T( "FrameBurstStart" ); break;
  case EventSelector_CriticalTemperature: Description = _T( "CriticalTemperature" ); break;
  case EventSelector_OverTemperature: Description = _T( "OverTemperature" ); break;
  case EventSelector_FrameStartOvertrigger: Description = _T( "FrameStartOvertrigger" ); break;
  }
  if ( !Description.IsEmpty() && !m_pHost->m_bBaslerShutDown )
  {
    CTextFrame * pEventFrame = CreateTextFrame(
      (LPCTSTR) pNode->GetName() , _T( "CameraEvent" ) );
    COutputConnector * pPin = m_pHost->GetOutputConnector( 1 ) ;
    if ( pPin )
      PutFrame( pPin , pEventFrame );
  }
}


bool Basler::BaslerCameraStart()
{
  if ( !m_sSerialNumber.IsEmpty() )
  {
    if ( !m_Camera.IsOpen() )
    {
      if ( !BaslerCamInit() )
        return false;
    }
    if ( !m_Camera.IsOpen() )
      return false;

    m_bShouldBeReprogrammed = false;

    if ( !m_bIsRunning )
    {
      m_BusEvents &= ~(BASLER_EVT_START_GRAB);
      CamTriggerMode Before = GetTriggerMode();
      if ( m_Camera.IsOpen() )
      {
        FXString GadgetName;
        BOOL bInLoop = GetGadgetName( GadgetName );
        m_GadgetInfo.Format( "%s:%s" , GadgetName , m_CameraID );
        Sleep( 20 );
        try
        {
          m_Camera.StartGrabbing( GrabStrategy_OneByOne , GrabLoop_ProvidedByUser );
          CamTriggerMode After = GetTriggerMode();
          if ( m_Camera.IsGrabbing() )
            m_bIsRunning = true;
          else
            SEND_GADGET_ERR("Can't start grab. " );
        }
        catch (GenICam::GenericException &e)
        {
          SEND_GADGET_ERR("Can't start grab. ERROR: %s", 
            e.GetDescription() );
        }
        if ( m_bIsRunning )
          return true;
      }
      else
      {
        FXString Msg;
        Msg.Format( "Can't open Basler camera" );
        LogError( Msg );
      }
      BaslerCamClose();
      Sleep( 20 );
    }
  }
  return false;
}

void Basler::DeviceStop()
{
  m_bRun = FALSE;
  if ( !m_Camera.IsOpen() || !m_continueGrabThread )
    return;

  CamCNTLDoAndWait( BASLER_EVT_STOP_GRAB , 1000 );
  CDevice::DeviceStop();

  SetEvent( m_evFrameReady );
  TRACE( "+++ Basler::CameraStop() for %s\n " , (LPCTSTR) m_sSerialNumber );
}

void Basler::BaslerCameraStop() // for calling from ControlLoop
{
  if ( !m_Camera.IsOpen() || !m_continueGrabThread )
    return;
  if ( m_bIsRunning )
  {
    int iLoopCnt = 0;
    m_bStopInitialized = true;
    m_Camera.StopGrabbing();
    double dStopStart = GetHRTickCount();
    m_bIsRunning = false;
    m_GrabLock.Lock();
    if ( m_pNewImage )
    {
      delete m_pNewImage;
      m_pNewImage = NULL;
    }
    m_GrabLock.Unlock();

    while ( m_Camera.IsGrabbing() )
    {
      Sleep( 1 );
      iLoopCnt++;
    }
    m_bStopInitialized = false;
    double dStopTime = GetHRTickCount() - dStopStart;
    TRACE( "+++ BaslerCameraStop()for %s in %.1f ms\n " ,
      (LPCTSTR) m_sSerialNumber , dStopTime );
  }
}

CVideoFrame* Basler::DeviceDoGrab( double* StartTime )
{
  if ( !m_Camera.IsOpen() || m_bShouldBeReprogrammed )
  {
    Sleep( 100 );
    *StartTime = GetHRTickCount();
    return NULL;
  }
  if ( !IsRun() )
    return NULL ;

  CVideoFrame * pOut = NULL;
  m_dLastStartTime = ::GetHRTickCount();
  *StartTime = m_dLastStartTime;
  CGrabResultPtr ptrGrabResult;
  try
  {
    if ( m_Camera.RetrieveResult( 100 , ptrGrabResult , TimeoutHandling_Return ) )
    {
      *StartTime = GetHRTickCount();
      // Check for special trigger mode (trigger input pin is connected)
      bool bEnable = !IsTriggerByInputPin() // trigger input is not connected
        || m_bWasTriggerOnPin ; // Was trigger
      if ( bEnable && ptrGrabResult->GrabSucceeded() )
      {
        // Access the image data.
        int32_t cols = m_RealBMIH.biWidth = ptrGrabResult->GetWidth();
        int32_t rows = m_RealBMIH.biHeight = ptrGrabResult->GetHeight();
        m_RealBMIH.biSize = sizeof(m_RealBMIH);
        m_RealBMIH.biSizeImage = (DWORD) ptrGrabResult->GetImageSize() ;
        m_LastEmbedInfo.m_CameraFrameID = ptrGrabResult->GetBlockID();
        m_LastEmbedInfo.m_CameraFrameTime = ptrGrabResult->GetTimeStamp();
        m_LastEmbedInfo.m_dGraphTime = GetHRTickCount();
        TCHAR Label[ 200 ];
        FXString Name;
        GetGadgetName( Name );
        FXSIZE iBackPos = Name.ReverseFind( _T( '.' ) );
        if ( iBackPos > 0 )
          Name = Name.Mid( iBackPos + 1 );

        sprintf_s( Label , _T( "%s" ) , (LPCTSTR) Name );
        m_uiLastFrameInterval = (uint32_t) (m_LastEmbedInfo.m_CameraFrameTime - m_ui64PreviousTimeStamp);
        m_ui64PreviousTimeStamp = m_LastEmbedInfo.m_CameraFrameTime;
        PixelFormatEnums format = m_pixelFormat ;
        EPixelType PType = ((IImage&)(ptrGrabResult)).GetPixelType();
        LPBYTE data = (LPBYTE) ptrGrabResult->GetBuffer();
        if ( m_bStopInitialized )
        {
          TRACE( "Frame after stop initialized, cam %s\n" , (LPCTSTR) m_sSerialNumber );
        }
        pTVFrame pNew = NULL;
        unsigned iSize = rows * cols;
        switch ( /*format*/ PType )
        {
        case PixelFormat_Mono8:
        case PixelType_Mono8:
          pNew = makeNewY8Frame( cols , rows );
          memcpy( (LPBYTE) (pNew->lpBMIH + 1) , data , iSize );
          break;
        case PixelType_RGB8packed:
        case PixelType_BGR8packed:
//         case PixelType_BayerGR8:
//         case PixelType_BayerRG8:
          m_RealBMIH.biBitCount = 24;
          pNew = newTVFrame( rgb24yuv12( &m_RealBMIH , data ) ) ;
          break;
        case PixelFormat_YUV422Packed:
        case PixelFormat_YUV422_YUYV_Packed:
          {
            TVFrame Tmp ;
            Tmp.lpBMIH = &m_RealBMIH ;
            m_RealBMIH.biCompression = BI_UYVY ;
            Tmp.lpData = data ;
            pNew = newTVFrame( _convertUYVY2YUV12( &Tmp ) ) ;
          }
          break;
        case PixelType_YUV422_YUYV_Packed: // Like NV12
        {
          TVFrame Tmp;
          Tmp.lpBMIH = &m_RealBMIH;
          m_RealBMIH.biCompression = BI_YUY2;
          Tmp.lpData = data;
          pNew = newTVFrame(_convertYUY2YUV12(&Tmp));
        }
        break;
        case PixelFormat_Mono16:
          {
            iSize *= 2;
            pNew = makeNewY16Frame( cols , rows );
            memcpy( (LPBYTE) (pNew->lpBMIH + 1) , data , iSize );
          }
          break;
        case PixelType_BayerGR8:
        case PixelType_BayerRG8:
        case PixelType_BayerGB8:
        case PixelType_BayerBG8:
          {
            // First the image format converter class must be created.
            CImageFormatConverter converter;

            // Second the converter must be parameterized. 
            converter.OutputPixelFormat = PixelType_BGR8packed ;
            converter.OutputBitAlignment = OutputBitAlignment_LsbAligned;
            converter.OutputOrientation = OutputOrientation_BottomUp ;

            pNew = makeNewRGBFrame( cols , rows ) ;

            converter.Convert( GetData( pNew ) , pNew->lpBMIH->biSizeImage ,
              data , m_RealBMIH.biSizeImage , PType , cols , rows , 0 , ImageOrientation_TopDown ) ;

            LPBITMAPINFOHEADER pYUV12_BMIH = rgb24yuv12( pNew->lpBMIH , NULL ) ;
            free( pNew->lpBMIH ) ;
            pNew->lpBMIH = pYUV12_BMIH ;
          }

          break ;
        }
        if ( pNew )
        {
          memcpy( ( LPBYTE ) ( pNew->lpBMIH + 1 ) , &m_LastEmbedInfo , // copy embedded info
            sizeof( m_LastEmbedInfo ) );
          pOut = CVideoFrame::Create( pNew );
          if ( pOut )
          {
            pOut->SetLabel( Label );
            pOut->SetTime( GetHRTickCount() );
          }
          m_dLastFrameTime = GetHRTickCount();
          m_bTriedToRestart = false;
        }
        if ( m_bStopInitialized )
        {
          TRACE( "Unlock after stop initialized, cam %s\n" , (LPCTSTR) (m_sSerialNumber) );
        }
      }
      else
      {
        uint32_t ErrorCode = ptrGrabResult->GetErrorCode() ;
        if ( ErrorCode )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , ( LPCTSTR ) m_GadgetInfo , 0 ,
            "Grab Error %s (%d) " , ptrGrabResult->GetErrorDescription().c_str() ,
            ErrorCode );
        }
      }
    }
    else
      *StartTime = GetHRTickCount();
  }
  catch ( const GenericException &e )
  {
    LPCTSTR pDescr = e.GetDescription();
    TRACE( "Basler::DeviceDoGrab exception catch - %s" , pDescr );
  }
//   if ( pOut )
//     PutFrame( GetOutputConnector( 0 ) , pOut ) ;
  return pOut ;
}

bool Basler::BuildPropertyList()
{
  bool bRes = CamCNTLDoAndWait( BASLER_EVT_BUILD_PROP , 5000 );
  //  bool bRes = BaslerBuildPropertyList() ;
  return (bRes && m_ThisCamProperty.GetCount());
}

int64_t Basler::GetIntValue( LPCTSTR pName )
{
  try
  {
    INodeMap* pMap = GetNodeMapWithCatching() ;
    if (!pMap)
      return 0 ;
    INode * pNode = pMap->GetNode( pName ) ;
    if ( pNode )
    {
      CIntegerPtr Parameter( pNode ) ;
      int64_t i64Val = Parameter->GetValue() ;
      return i64Val ;
    }
    return 0 ;
  }
  catch ( GenICam::GenericException &e )
  {
    SEND_GADGET_ERR( "GetIntValue error %s for parameter %s" , e.GetDescription() , pName ) ;
  }
  return 0 ;
}

int64_t Basler::GetIntValue( LPCTSTR * pName )
{
  try
  {
    INodeMap* pMap = GetNodeMapWithCatching() ;
    if (!pMap)
      return 0 ;
    while ( *pName )
    {
      INode * pNode = pMap->GetNode( *pName ) ;
      if ( pNode )
      {
        CIntegerPtr Parameter( pNode ) ;
        int64_t i64Val = Parameter->GetValue() ;
        return i64Val ;
      }
      pName++ ;
    }
    return 0 ;
  }
  catch ( GenICam::GenericException &e )
  {
    SEND_GADGET_ERR( "GetIntValue error %s for parameter %s" , e.GetDescription() , pName ) ;
  }
  return 0 ;
}

void Basler::SetXOffset( int64_t i64Val )
{
  SetIntValue( OffXNames , i64Val ) ;
}

void Basler::SetYOffset( int64_t i64Val )
{
  SetIntValue( OffYNames , i64Val ) ;
}

void Basler::SetIntValue( LPCTSTR * pName , int64_t i64Val )
{
  try
  {
    INodeMap* pMap = GetNodeMapWithCatching() ;
    if (!pMap)
      return ;
    while (*pName)
    {
      INode * pNode = pMap->GetNode( *pName ) ;
      if ( pNode )
      {
        CIntegerPtr Parameter( pNode ) ;
        Parameter->SetValue( i64Val ) ;
        return ;
      }
      pName++ ;
    }
  }
  catch ( GenICam::GenericException &e )
  {
    SEND_GADGET_ERR( "SetIntValue error %s for parameter %s" , e.GetDescription() , pName ) ;
  }
}

void Basler::SetIntValue( LPCTSTR pName , int64_t i64Val )
{
  try
  {
    INodeMap* pMap = GetNodeMapWithCatching() ;
    if (!pMap)
      return ;
    INode * pNode = pMap->GetNode( pName ) ;
    if ( pNode )
    {
      CIntegerPtr Parameter( pNode ) ;
      Parameter->SetValue( i64Val ) ;
    }
  }
  catch ( GenICam::GenericException &e )
  {
    SEND_GADGET_ERR( "SetIntValue error %s for parameter %s" , e.GetDescription() , pName ) ;
  }
}

INode * Basler::GetPropertyNode( LPCTSTR pNodeName )
{
  INodeMap* pMap = GetNodeMapWithCatching() ;
  if (pMap)
  {
    INode * pNode = pMap->GetNode( pNodeName ) ;
    if (pNode && GenApi::IsAvailable( pNode ))
      return pNode ;
  }
  return NULL ;
}


INode * Basler::FindValidCameraPropertyName( BaslerPropertyDefine& Property , LPCTSTR * ppValidName )
{
  LPCTSTR * pInCamPropName = &Property.ppNodesNames[ 0 ] ;
  INode * pNode = NULL ;
  do
  {
    pNode = GetPropertyNode( *(pInCamPropName) ) ;
    if ( pNode )
    {
      *ppValidName = *pInCamPropName ;
      return pNode ;
    }
  } while ( *(++pInCamPropName) ) ;
  *ppValidName = NULL ;
  return NULL ;
}

INode * Basler::FindValidCameraPropertyName( FXCamPropertyType Type , LPCTSTR * ppValidName )
{
  LPCTSTR * pInCamPropName = _getPropertyInCamNames( Type );
  INode * pNode = NULL ;
  do
  {
    pNode = GetPropertyNode( *(pInCamPropName) ) ;
    if ( pNode )
    {
      *ppValidName = *pInCamPropName ;
      return pNode ;
    }
  } while ( *(++pInCamPropName) ) ;
  *ppValidName = NULL ;
  return NULL ;
}


bool Basler::GetEnumeratorsAsText(
  INode * pEnumeratedNode , FXString& Result ,
  BaslerCamProperty& Prop )
{
  if ( GetType( pEnumeratedNode ) == PDTEnum )
  {
    CEnumerationPtr Parameter( pEnumeratedNode );

    NodeList_t entries;
    Parameter->GetEntries( entries );
    if ( GetEnumeratorsAsText( entries , Result , Prop ) )
    {
      _tcscpy_s( Prop.LastValue.szAsString ,
        Parameter->GetCurrentEntry()->GetSymbolic().c_str() ) ;
      Prop.LastValue.iInt = (int)(Prop.LastValue.i64Int = 
        Parameter->GetCurrentEntry()->GetValue()) ;
      return true ;
    }
  }
  return false ;
}

bool Basler::GetEnumeratorsAsText( NodeList_t& entries ,
  FXString& Result , BaslerCamProperty& Prop )
{
  FXString items , Addition ;
  Prop.m_EnumeratorsAsTexts.clear() ;
  Prop.m_EnumeratorsAsValues.clear() ;
  for ( NodeList_t::iterator it = entries.begin(); it != entries.end(); ++it )
  {
    CEnumEntryPtr pEntry = (*it);
    if ( IsAvailable( pEntry ) )
    {
      int64_t i64Val = pEntry->GetValue();
//       switch ( Prop.id )
//       {
//       case EVENT_SELECTOR: i64Val = Prop.m_EnumeratorsAsValues.size(); break;
//       }
      Addition.Format( _T( "%s%s(%lld)" ) ,
        (Prop.m_EnumeratorsAsTexts.size()) ? _T( "," ) : _T( "" ) ,
        pEntry->GetSymbolic().c_str() , i64Val ) ;
      items += Addition ;
      Prop.m_EnumeratorsAsTexts.push_back( pEntry->GetSymbolic().c_str() ) ;
      Prop.m_EnumeratorsAsValues.push_back( pEntry->GetValue() ) ;
    }
  }
  if ( !items.IsEmpty() )
  {
    Result += items ;
    return true ;
  }
  return false ;
}

CamTriggerMode Basler::GetTriggerMode()
{
  if ( !m_Camera.IsOpen() || !m_Camera.TriggerMode.IsValid() )
    return TrigNotSupported;
  BaslerCamProperty& Prop = m_ThisCamProperty.GetAt( m_iTriggerModeIndex );
  try
  {
    m_TriggerMode = m_Camera.TriggerMode.GetValue() ;
    m_LastEmbedInfo.m_iTriggerMode = m_TriggerMode ;
    CamTriggerMode TriggerMode =
      (m_TriggerMode) ? TriggerOn : TriggerOff;
    return TriggerMode ;
  }
  catch ( const GenericException &e )
  {
    // Error handling
    FXString CatchData ;
    CatchData.Format( _T(
      "GetTriggerMode: Exception for Basler::GetTriggerMode : %s\n" ) ,
      (LPCTSTR) Prop.name , e.GetDescription() ) ;
    SEND_GADGET_ERR( (LPCTSTR) CatchData );
  }
  return TriggerModeError;
}
bool Basler::BaslerBuildPropertyList( bool bOutside )
{
  m_ThisCamProperty.RemoveAll();
  BaslerCamProperty P , PLastAuto ;

  if ( bOutside && !m_Camera.IsOpen() && !BaslerCamInit() )
    return false;

  int iWhiteBalanceMode = GetColorMode(); // 0 - no color, 1 - by program, 2 - by camera
  LPCTSTR pInCameraPropName = NULL ;

  INodeMap * pNodeMap = GetNodeMapWithCatching() ;
  if (!pNodeMap)
    return false ;

  INodeMap& nodemap = *pNodeMap;
  for ( int i = 0; i < _getPropertyCount(); i++ )
  {
    bool bExternal = false ;
    INode * pNode = FindValidCameraPropertyName( m_cBProperties[ i ] ,
      &pInCameraPropName ) ;

    if ( !pNode || !pInCameraPropName )
    {
      switch ( m_cBProperties[ i ].Id )
      {
      case EVENTS_STATUS:
      case SETROI: 
        bExternal = true ; 
        break ;
      case BASLER_PACKETSIZE:
        if ( m_Camera.IsGigE() &&  m_Camera.GevSCPSPacketSize.IsValid() )
          pNode = m_Camera.GevSCPSPacketSize.GetNode() ;
        break ;
      case BASLER_INTER_PACKET_DELAY:
        if ( m_Camera.IsGigE() && m_Camera.GevSCPD.IsValid() )
          pNode = m_Camera.GevSCPD.GetNode() ;
        break ;
      }
      if ( !pNode && !bExternal )
      {
        TRACE( "\n  Undefined property:'%s' (Available names are %s %s %s)" ,
          m_cBProperties[ i ].pUIName ,
          m_cBProperties[ i ].ppNodesNames[ 0 ] ? m_cBProperties[ i ].ppNodesNames[ 0 ] : " " ,
          m_cBProperties[ i ].ppNodesNames[ 1 ] ? m_cBProperties[ i ].ppNodesNames[ 1 ] : " " ,
          m_cBProperties[ i ].ppNodesNames[ 2 ] ? m_cBProperties[ i ].ppNodesNames[ 2 ] : " "
        );
        continue ;
      }
    }

    PropertyDataType Type = (!bExternal) ? GetType( pNode ) : PDTExternal ;
    P.Reset();
    P.name = m_cBProperties[ i ].pUIName;
    P.m_InDeviceName = pInCameraPropName ;
    P.id = (FXCamPropertyType) m_cBProperties[ i ].Id;
    P.m_DataType = Type ;
    FXString ForCombo ;
    NodeList_t Entries ;
    // If property type is enumerator, following call will
    // fill correspondent arrays in P and Combo box content
    if ( !bExternal )
    {
      try
      {
//         if ( P.m_InDeviceName == "TriggerSelector"
//           && m_Camera.TriggerSelector.IsWritable() )
//         {
//           m_Camera.TriggerSelector.SetValue( "FrameStart" ) ;
//         }
        GetEnumeratorsAsText( pNode , ForCombo , P ) ;
      }
      catch ( const GenericException &e )
      {
        bool bOpened = m_Camera.IsOpen() ;
        // Error handling
        FXString CatchData ;
        CatchData.Format( _T( "  Error Read %s (Type %s): %s\n" ) ,
          (LPCTSTR) P.name , GetTypeName( P.m_DataType ) , e.GetDescription() ) ;
        SEND_GADGET_ERR( (LPCTSTR) CatchData );
      }
    }

    //if ( *m_cBProperties[ i ].pUIName == 0 ) // property is not for viewing
    //  continue ;

    switch ( m_cBProperties[ i ].Id )
    {
    case FORMAT_MODE:
      {
        if ( m_Camera.PixelFormat.IsValid() && !ForCombo.IsEmpty() )
        {
          P.bSupported = true;
          P.m_GUIFormat.Format( "ComboBox(%s(%s))" , P.name , (LPCTSTR) ForCombo );
          P.m_DataType = PDTEnum ;
          m_AvailableFormats.clear();
          for ( size_t i = 0 ; i < P.m_EnumeratorsAsTexts.size() ; i++ )
          {
            PixelTypeNameAndEnum FormatMode(P.m_EnumeratorsAsTexts[i], 
              (DWORD)(P.m_EnumeratorsAsValues[i]));
            m_AvailableFormats.push_back(FormatMode);
          }
        }
      }
      break;
    case SETROI:
      {
        P.m_GUIFormat.Format( "EditBox(%s)" , P.name );
        P.m_DataType = PDTString;
        P.bSupported = true;
        // Read from camera and correct for 4 pixels step
        m_CurrentROI.left = ((LONG) GetIntValue( OffXNames )) & ~0x3 ; // read from camera
        m_CurrentROI.right = ( ( LONG ) GetIntValue( _T( "Width" ) )) & ~0x03 ;// read from camera
        m_CurrentROI.top = ((LONG) GetIntValue( OffYNames )) & ~0x03; // read from camera
        m_CurrentROI.bottom = ( ( LONG ) GetIntValue( _T( "Height" ) ) ) & ~0x03 ;  // read from camera
        m_SensorSize.cx = (LONG) GetIntValue( _T( "SensorWidth" ) ) ; // read from camera 
        m_SensorSize.cy = (LONG) GetIntValue( _T( "SensorHeight" ) ) ; // read from camera 
        break;
      }
    case SHUTTER_AUTO:
      {
        P.name = "ExposureAuto" ;
        PLastAuto = P ;
        if (m_Camera.ExposureAuto.IsValid() && m_Camera.ExposureAuto.IsReadable())
        {
          ExposureAutoEnums Val = m_Camera.ExposureAuto.GetValue();
          PLastAuto.m_bAuto = (Val != ExposureAuto_Off);
        }
        else
          PLastAuto.m_bAuto = false;
        P.m_bDontShow = true ;
      }
    case GAIN_AUTO:
      {
        P.name = "GainAuto" ;
        PLastAuto = P ;
        if (m_Camera.GainAuto.IsValid() && m_Camera.GainAuto.IsReadable())
        {
          GainAutoEnums Val = m_Camera.GainAuto.GetValue();
          PLastAuto.m_bAuto = (Val != GainAuto_Off);
        }
        else
          PLastAuto.m_bAuto = false;
        P.m_bDontShow = true;
      }
      break ;
    case SHUTTER:
      {
        P.bSupported = true;
        int iMin , iMax = 0 ;
        m_bExpAfterScanSettings = true ;
        try
        {
          if ( m_Camera.ExposureTime.IsValid() )
          {
            iMin = (int) ceil( m_Camera.ExposureTime.GetMin() ) ;
            iMax = (int) floor( m_Camera.ExposureTime.GetMax() ) ;
            P.LastValue.iInt = ROUND( P.LastValue.dDouble =
              m_LastEmbedInfo.m_dExposure_us = m_Camera.ExposureTime.GetValue() ) ;
          }
          else if ( m_Camera.ExposureTimeAbs.IsValid() )
          {
            iMin = (int) ceil( m_Camera.ExposureTimeAbs.GetMin() ) ;
            iMax = (int) floor( m_Camera.ExposureTimeAbs.GetMax() ) ;
            P.LastValue.iInt = ROUND( P.LastValue.dDouble = 
              m_LastEmbedInfo.m_dExposure_us = m_Camera.ExposureTimeAbs.GetValue() );
          }
          else if ( m_Camera.ExposureTimeRaw.IsValid() )
          {
            iMin = (int) m_Camera.ExposureTimeRaw.GetMin() ;
            iMax = (int) m_Camera.ExposureTimeRaw.GetMax() ;
            P.LastValue.iInt = (int) (P.LastValue.i64Int =
              m_Camera.ExposureTimeRaw.GetValue()) ;
            m_LastEmbedInfo.m_dExposure_us = P.LastValue.dDouble = P.LastValue.iInt ;
          }
          if ( iMin != iMax )
          {
            P.LastValue.dMin = ( double ) iMin ;
            P.LastValue.dMax = ( double ) iMax ;
          }
          LPCTSTR pValidName = NULL ;
          INode * pNode = FindValidCameraPropertyName( SHUTTER_AUTO , &pValidName ) ;
          if ( pNode )
          {
            P.m_AutoControlName = pValidName ;
            CEnumParameter ExpoAuto( nodemap , pValidName ) ;
            P.m_bAuto = (ExpoAuto.GetValue() != "Off") ;
          }
          else
          {
            P.m_AutoControlName.Empty() ;
            P.m_bAuto = false ;
          }

          if ( iMax )
          {
            P.m_GUIFormat.Format( "%s(%s,%d,%d)" ,
              (!P.m_AutoControlName.IsEmpty()) ? SETUP_SPINABOOL : SETUP_SPIN ,
              P.name , iMin , iMax );
          }
        }
        catch ( const GenericException &e )
        {
          bool bOpened = m_Camera.IsOpen() ;
          // Error handling
          FXString CatchData ;
          CatchData.Format( _T( "  Error Build Shutter Prop %s (Type %s): %s\n" ) ,
            (LPCTSTR) P.name , GetTypeName( P.m_DataType ) , e.GetDescription() ) ;
          SEND_GADGET_ERR( (LPCTSTR) CatchData );
        }
      }
      break;
    case GAIN:
      {
        P.bSupported = true;
        int iMin , iMax = 0 ;
        m_bGainAfterScanSettings = true ;
        if ( m_Camera.Gain.IsValid() )
        {
          iMin = (int) ceil( m_Camera.Gain.GetMin() * 10. ) ;
          iMax = (int) floor( m_Camera.Gain.GetMax() * 10. ) ;
          P.LastValue.iInt = ROUND( P.LastValue.dDouble =
            (m_LastEmbedInfo.m_dGain_dB = m_Camera.Gain.GetValue()) * 10. ) ;
        }
        else if ( m_Camera.GainAbs.IsValid() )
        {
          iMin = (int) ceil( m_Camera.GainAbs.GetMin() * 10. ) ;
          iMax = (int) floor( m_Camera.GainAbs.GetMax() * 10. ) ;
          P.LastValue.iInt = ROUND( P.LastValue.dDouble = 
            (m_LastEmbedInfo.m_dGain_dB = m_Camera.Gain.GetValue()) * 10. );
        }
        else if ( m_Camera.GainRaw.IsValid() )
        {
          iMin = (int) m_Camera.GainRaw.GetMin() ;
          iMax = (int) m_Camera.GainRaw.GetMax() ;
          P.LastValue.iInt = (int) (P.LastValue.i64Int =
            m_Camera.GainRaw.GetValue() ) ;
          m_LastEmbedInfo.m_dGain_dB = P.LastValue.dDouble = P.LastValue.iInt  ;
        }
        if (iMin != iMax)
        {
          P.LastValue.dMin = ( double ) iMin ;
          P.LastValue.dMax = ( double ) iMax ;
        }
        if ( iMax )
        {
          if ( m_Camera.GainAuto.IsValid() && m_Camera.GainAuto.IsReadable() )
          {
            P.LastValue.bBool = P.m_bAuto = ( m_Camera.GainAuto.GetValue() == GainAuto_Continuous ) ;
            P.m_GUIFormat.Format( "%s(%s,%d,%d)" , SETUP_SPINABOOL ,
              P.name , iMin , iMax );
          }
          else
          {
            P.m_GUIFormat.Format( "%s(%s,%d,%d)" , SETUP_SPIN ,
              P.name , iMin , iMax );
            P.m_AutoControlName.Empty() ;
            P.m_bAuto = false ;
          }
        }
      }
      break;
    case TRIGGER_MODE:
      {
        //         if ( pNode->Is() )
        // //         if ( m_Camera.TriggerMode.IsValid() )
        {
          FXString ForTriggerCombo ;
          GetEnumeratorsAsText( pNode , ForTriggerCombo , P ) ;

          P.bSupported = true;
          P.m_GUIFormat.Format( "ComboBox(%s(%s))" ,
            P.name , (LPCTSTR) ForTriggerCombo ) ;
          //CEnumParameter TModeParam( nodemap , P.m_InDeviceName ) ;

          m_LastEmbedInfo.m_iTriggerMode = m_TriggerMode = m_Camera.TriggerMode.GetValue() ;
          m_iTriggerModeIndex = (int) m_ThisCamProperty.GetCount() ;
          strcpy_s( P.LastValue.szAsString ,
            m_Camera.TriggerMode.ToString().c_str() ) ;
        }
        break ;
      }
    case TRIGGER_SELECTOR:
      {
        if ( /*m_TriggerMode == TriggerMode_On
          && */m_Camera.TriggerSelector.IsValid() )
        {
          P.bSupported = true;
          P.m_GUIFormat.Format( "ComboBox(%s(%s))" ,
            P.name , (LPCTSTR) ForCombo ) ;
          m_LastEmbedInfo.m_iTriggerSelector = 
            (int) (m_TriggerSelector = m_Camera.TriggerSelector.GetValue()) ;
          strcpy_s( P.LastValue.szAsString ,
            m_Camera.TriggerSelector.ToString().c_str() ) ;
        }
        break ;
      }
    case BURST_LENGTH:
      {
        if (( m_TriggerMode == TriggerMode_On ) )
        {
          int iMin = 0 , iMax = 0 ;
          if ( m_Camera.AcquisitionBurstFrameCount.IsValid() )
          {
            iMin = ( int ) ( m_Camera.AcquisitionBurstFrameCount.GetMin() ) ;
            iMax = ( int ) ( m_Camera.AcquisitionBurstFrameCount.GetMax() ) ;
            P.m_GUIFormat.Format( "%s(%s,%d,%d)" ,
              SETUP_SPIN , P.name , iMin , iMax ) ;
            P.LastValue.iInt = ( int ) ( ( P.LastValue.i64Int ) =
              m_Camera.AcquisitionBurstFrameCount.GetValue() ) ;
          }
          else if ( m_Camera.AcquisitionFrameCount.IsValid() )
          {
            iMin = ( int ) ( m_Camera.AcquisitionFrameCount.GetMin() ) ;
            iMax = ( int ) ( m_Camera.AcquisitionFrameCount.GetMax() ) ;
            P.m_GUIFormat.Format( "%s(%s,%d,%d)" ,
              SETUP_SPIN , P.name , iMin , iMax ) ;
            P.LastValue.iInt = ( int ) ( ( P.LastValue.i64Int ) =
              m_Camera.AcquisitionFrameCount.GetValue() ) ;
          }
          if (iMin != iMax)
          {
            P.LastValue.dMin = ( double ) iMin ;
            P.LastValue.dMax = ( double ) iMax ;
          }

        }
        break ;
      }
    case TRIGGER_ACTIVATE:
      {
        if ( m_Camera.TriggerActivation.IsValid() 
          && m_Camera.TriggerActivation.IsReadable() )
        {
          P.m_GUIFormat.Format( "ComboBox(%s(%s))" , P.name , (LPCTSTR) ForCombo ) ;
          P.bSupported = true;
          P.LastValue.iInt = (int) ((P.LastValue.i64Int) =
            m_Camera.TriggerActivation.GetValue()) ;
          strcpy_s( P.LastValue.szAsString ,
            m_Camera.TriggerActivation.ToString().c_str() ) ;
        }
      }
      break;
    case TRIGGER_SOURCE:
      if (  m_Camera.TriggerSource.IsValid() )
      {
        P.m_GUIFormat.Format( "ComboBox(%s(%s))" , P.name , (LPCTSTR) ForCombo ) ;
        P.bSupported = true;
        P.LastValue.iInt = (int) ((P.LastValue.i64Int) =
          m_Camera.TriggerSource.GetValue()) ;
        strcpy_s( P.LastValue.szAsString ,
          m_Camera.TriggerSource.ToString().c_str() ) ;
        m_bSoftwareTriggerMode =
          ( _tcsicmp( P.LastValue.szAsString , _T( "Software" ) ) == 0 ) ;
      }
      break;
    case TRIGGER_DELAY:
      {
//         if ( m_TriggerMode == TriggerMode_On )
        {
          int iMin , iMax = 0 ;
          if ( m_Camera.TriggerDelay.IsValid() )
          {
            iMin = (int) ceil( m_Camera.TriggerDelay.GetMin() ) ;
            iMax = (int) floor( m_Camera.TriggerDelay.GetMax() ) ;
            P.LastValue.dDouble = m_Camera.TriggerDelay.GetValue() ;
            P.LastValue.iInt = (int) (P.LastValue.i64Int = (int64_t) P.LastValue.dDouble) ;
          }
          else if ( m_Camera.TriggerDelayAbs.IsValid() )
          {
            iMin = (int) ceil( m_Camera.TriggerDelayAbs.GetMin() ) ;
            iMax = (int) floor( m_Camera.TriggerDelayAbs.GetMax() ) ;
            P.LastValue.dDouble = m_Camera.TriggerDelayAbs.GetValue() ;
            P.LastValue.iInt = (int) (P.LastValue.i64Int = (int64_t) P.LastValue.dDouble) ;
          }
          if ( iMax )
          {
            P.m_GUIFormat.Format( "%s(%s,%d,%d)" , SETUP_SPIN ,
              P.name , iMin , iMax );
            P.bSupported = true;
          }
          if (iMin != iMax)
          {
            P.LastValue.dMin = ( double ) iMin ;
            P.LastValue.dMax = ( double ) iMax ;
          }
        }
      }
      break;
    case TRIGGER_SOFT_FIRE:
      {
        if ( m_Camera.TriggerSource.IsValid() )
        {
          FXString TriggerSrc = m_Camera.TriggerSource.ToString().c_str() ;
          if ( TriggerSrc == "Software" )
          {
            INode * pFireSoftTrigger = nodemap.GetNode( (LPCTSTR) P.m_InDeviceName ) ;
            if ( pFireSoftTrigger )
            {
              //             P.m_bDontShow = true ;
              P.bSupported = true;
              P.m_GUIFormat.Format( "%s(%s,-1000,1000)" , SETUP_SPIN , P.name ) ;
            }
          }
        }
      }
      break ;

    case EXPOSURE_MODE:
      if ( m_Camera.ExposureMode.IsValid() )
      {
        P.m_GUIFormat.Format( "ComboBox(%s(%s))" , P.name , (LPCTSTR) ForCombo ) ;
        P.bSupported = true;
        P.LastValue.iInt = (int) ((P.LastValue.i64Int) =
          m_Camera.ExposureMode.GetValue()) ;
        strcpy_s( P.LastValue.szAsString ,
          m_Camera.ExposureMode.ToString().c_str() ) ;
      }
      break;
    case FRAME_RATE:
      {
        double dMin = 0. , dMax = 0. ;
        if ( m_Camera.AcquisitionFrameRate.IsValid() )
        {
          dMin = m_Camera.AcquisitionFrameRate.GetMin() ;
          dMax = m_Camera.AcquisitionFrameRate.GetMax() ;
          P.LastValue.dDouble = m_Camera.AcquisitionFrameRate.GetValue() ;
          P.LastValue.iInt = (int) (P.LastValue.i64Int
            = (int64_t) (P.LastValue.dDouble)) ;
          P.m_GUIFormat.Format( "%s(%s)" , SETUP_EDITBOX , P.name );
          P.bSupported = true;
        }
        else if ( m_Camera.AcquisitionFrameRateAbs.IsValid() )
        {
          dMin = m_Camera.AcquisitionFrameRateAbs.GetMin() ;
          dMax = m_Camera.AcquisitionFrameRateAbs.GetMax() ;
          P.LastValue.dDouble = m_Camera.AcquisitionFrameRateAbs.GetValue() ;
          P.LastValue.iInt = (int) (P.LastValue.i64Int
            = (int64_t) (P.LastValue.dDouble)) ;
          P.m_GUIFormat.Format( "%s(%s)" , SETUP_EDITBOX , P.name );
          P.bSupported = true;
        }
        if (dMin != dMax)
        {
          P.LastValue.dMin = dMin ;
          P.LastValue.dMax = dMax ;
        }

      }
      break;
    case GAMMA:
      {
        if ( m_Camera.Gamma.IsValid() )
        {
          double dMin = m_Camera.Gamma.GetMin() ;
          double dMax = m_Camera.Gamma.GetMax() ;
          P.LastValue.dDouble = m_Camera.Gamma.GetValue() ;
          P.LastValue.iInt = (int) (P.LastValue.i64Int
            = (int64_t) (P.LastValue.dDouble)) ;
          P.m_GUIFormat.Format( "%s(%s)" , SETUP_EDITBOX , P.name );
          P.bSupported = true;
          if (dMin != dMax)
          {
            P.LastValue.dMin = dMin ;
            P.LastValue.dMax = dMax ;
          }
        }
      }
      break;
    case FRAME_RATE_ENABLE:
      {
        PLastAuto = P ;
        PLastAuto.m_bAuto = true ;
        P.m_bDontShow = true ;
      }
      break ;
    case BASLER_PACKETSIZE:
      {
        int iMin , iMax = 0 ;
        if ( m_Camera.GevSCPSPacketSize.IsValid() )
        {
          iMin = (int) m_Camera.GevSCPSPacketSize.GetMin() ;
          iMax = (int) m_Camera.GevSCPSPacketSize.GetMax() ;
          P.LastValue.iInt = (int)(P.LastValue.i64Int = m_Camera.GevSCPSPacketSize.GetValue()) ;
        }
        if ( iMax )
        {
          P.m_GUIFormat.Format( "%s(%s,%d,%d)" , SETUP_SPIN ,
            P.name , iMin , iMax );
          P.bSupported = true ;
        }
        if (iMin != iMax)
        {
          P.LastValue.dMin = ( double ) iMin ;
          P.LastValue.dMax = ( double ) iMax ;
        }
      }
      break ;
    case BASLER_INTER_PACKET_DELAY:
      {
        int iMin , iMax = 0 ;
        if ( m_Camera.GevSCPD.IsValid() )
        {
          iMin = (int) m_Camera.GevSCPD.GetMin() ;
          iMax = (int) m_Camera.GevSCPD.GetMax() ;
          P.LastValue.iInt = (int) (P.LastValue.i64Int = m_Camera.GevSCPD.GetValue()) ;
        }
        if ( iMax )
        {
          P.m_GUIFormat.Format( "%s(%s,%d,%d)" , SETUP_SPIN ,
            P.name , iMin , iMax );
          P.bSupported = true ;
          if (iMin != iMax)
          {
            P.LastValue.dMin = ( double ) iMin ;
            P.LastValue.dMax = ( double ) iMax ;
          }
        }
      }
      break ;
    case BRIGHTNESS:
      {
        int iMin , iMax = 0 ;
        if ( m_Camera.BlackLevel.IsValid() )
        {
          iMin = (int) ceil( m_Camera.BlackLevel.GetMin() * 10. ) ;
          iMax = (int) floor( m_Camera.BlackLevel.GetMax() * 10. ) ;
          P.LastValue.iInt = ROUND( P.LastValue.dDouble =
            (m_LastEmbedInfo.m_dBrightness = m_Camera.BlackLevel.GetValue()) * 10. ) ;
        }
        else if ( m_Camera.BlackLevelAbs.IsValid() )
        {
          iMin = (int) ceil( m_Camera.BlackLevelAbs.GetMin() * 10. ) ;
          iMax = (int) floor( m_Camera.BlackLevelAbs.GetMax() * 10. ) ;
          P.LastValue.iInt = ROUND(
            P.LastValue.dDouble = m_LastEmbedInfo.m_dBrightness = m_Camera.BlackLevelAbs.GetValue() );
        }
        else if ( m_Camera.BlackLevelRaw.IsValid() )
        {
          iMin = (int) m_Camera.BlackLevelRaw.GetMin() ;
          iMax = (int) m_Camera.BlackLevelRaw.GetMax() ;
          P.LastValue.i64Int = m_Camera.BlackLevelRaw.GetValue() ;
          m_LastEmbedInfo.m_dBrightness = P.LastValue.dDouble = P.LastValue.iInt = (int) P.LastValue.i64Int ;
        }
        if ( iMax )
        {
          P.m_GUIFormat.Format( "%s(%s,%d,%d)" , SETUP_SPIN ,
            P.name , iMin , iMax );
          P.bSupported = true ;
          if (iMin != iMax)
          {
            P.LastValue.dMin = ( double ) iMin ;
            P.LastValue.dMax = ( double ) iMax ;
          }
        }
      }
      break;
    case LINE_SELECTOR:
      {
        if ( P.m_EnumeratorsAsTexts.size() > 1 )
        {
          P.m_GUIFormat.Format( "ComboBox(%s(%s))" , P.name , (LPCTSTR) ForCombo ) ;
          _tcscpy_s( P.LastValue.szAsString , m_Camera.LineSelector.ToString().c_str() ) ;
          P.bSupported = true;
          m_ComplexIOs.m_Lines.clear() ;
          
        }
      }
      break;
    case LINE_MODE:
      {
        if ( P.m_EnumeratorsAsTexts.size() > 1 )
        {
          P.m_GUIFormat.Format( "ComboBox(%s(%s))" , P.name , (LPCTSTR) ForCombo ) ;
          _tcscpy_s( P.LastValue.szAsString , m_Camera.LineMode.ToString().c_str() ) ;
          P.bSupported = true;
        }
      }
      break;
    case LINE_SOURCE:
      {
        if ( P.m_EnumeratorsAsTexts.size() > 1 )
        {
          P.m_GUIFormat.Format( "ComboBox(%s(%s))" , P.name , (LPCTSTR) ForCombo ) ;
          _tcscpy_s( P.LastValue.szAsString , m_Camera.LineSource.ToString().c_str() ) ;
          P.bSupported = true;
        }
      }
      break;
    case LINE_INVERTER:
      {
        if ( m_Camera.LineInverter.IsValid() )
        {
          P.m_GUIFormat.Format( "ComboBox(%s(%s))" , P.name , "No(0),Yes(1)" ) ;
          P.LastValue.bBool = m_Camera.LineInverter.GetValue() ;
          P.bSupported = true;
        }
      }
      break;
    case TIMER_SELECT:
      {
        if ( m_Camera.TimerSelector.IsValid() )
        {
          P.m_GUIFormat.Format( "ComboBox(%s(%s))" , P.name , (LPCTSTR) ForCombo ) ;
          P.bSupported = true;
          P.LastValue.iInt = (int) ((P.LastValue.i64Int) =
            m_Camera.TimerSelector.GetValue()) ;
          strcpy_s( P.LastValue.szAsString ,
            m_Camera.TimerSelector.ToString().c_str() ) ;
        }
      }
      break;
    case TIMER_TRIGGER_SOURCE:
      {
        try
        {
          if ( m_Camera.TimerTriggerSource.IsValid() )
          {
            P.m_GUIFormat.Format( "ComboBox(%s(%s))" , P.name , (LPCTSTR) ForCombo ) ;
            P.bSupported = true;
            P.LastValue.iInt = (int) ((P.LastValue.i64Int) =
              m_Camera.TimerTriggerSource.GetValue()) ;
            strcpy_s( P.LastValue.szAsString ,
              m_Camera.TimerTriggerSource.ToString().c_str() ) ;
          }
        }
        catch ( const GenericException &e )
        {
          bool bOpened = m_Camera.IsOpen() ;
          // Error handling
          FXString CatchData ;
          CatchData.Format( _T( "  Error Build Prop %s (Type %s): %s\n" ) ,
            (LPCTSTR) P.name , GetTypeName( P.m_DataType ) , e.GetDescription() ) ;
          SEND_GADGET_ERR( (LPCTSTR) CatchData );
        }

      }
      break;
//    case TIMER_TRIGGER_POLARITY:
    case TIMER_TRIGGER_ACTIVATION:
      {
        if ( m_Camera.TimerTriggerActivation.IsValid() )
        {
          P.m_GUIFormat.Format( "ComboBox(%s(%s))" , P.name , (LPCTSTR) ForCombo ) ;
          P.bSupported = true;
          P.LastValue.iInt = (int) ((P.LastValue.i64Int) =
            m_Camera.TimerTriggerActivation.GetValue()) ;
          strcpy_s( P.LastValue.szAsString ,
            m_Camera.TimerTriggerActivation.ToString().c_str() ) ;
        }
      }
      break;
    case TIMER_DELAY:
      {
        int iMin , iMax = 0 ;
        if ( m_Camera.TimerDelay.IsValid() )
        {
          iMin = (int) ceil( m_Camera.TimerDelay.GetMin() ) ;
          iMax = (int) floor( m_Camera.TimerDelay.GetMax() ) ;
          P.LastValue.dDouble = m_Camera.TimerDelay.GetValue() ;
          P.LastValue.iInt = (int) (P.LastValue.i64Int = (int64_t) P.LastValue.dDouble) ;
        }
        else if ( m_Camera.TimerDelayAbs.IsValid() )
        {
          iMin = (int) ceil( m_Camera.TimerDelayAbs.GetMin() ) ;
          iMax = (int) floor( m_Camera.TimerDelayAbs.GetMax() ) ;
          P.LastValue.dDouble = m_Camera.TimerDelayAbs.GetValue() ;
          P.LastValue.iInt = (int) (P.LastValue.i64Int = (int64_t) P.LastValue.dDouble) ;
        }
        else if ( m_Camera.TimerDelayRaw.IsValid() )
        {
          iMin = (int) m_Camera.TimerDelayRaw.GetMin() ;
          iMax = (int) m_Camera.TimerDelayRaw.GetMax() ;
          P.LastValue.dDouble = (double) (P.LastValue.i64Int
            = m_Camera.TimerDelayRaw.GetValue()) ;
          P.LastValue.iInt = (int) P.LastValue.i64Int ;
        }
        if ( iMax )
        {
          P.m_GUIFormat.Format( "%s(%s,%d,%d)" , SETUP_SPIN ,
            P.name , iMin , iMax );
          P.bSupported = true;
          if (iMin != iMax)
          {
            P.LastValue.dMin = ( double ) iMin ;
            P.LastValue.dMax = ( double ) iMax ;
          }
        }
      }
      break;
    case TIMER_DURATION:
      {
        int iMin , iMax = 0 ;
        if ( m_Camera.TimerDuration.IsValid() )
        {
          iMin = (int) ceil( m_Camera.TimerDuration.GetMin() ) ;
          iMax = (int) floor( m_Camera.TimerDuration.GetMax() ) ;
          P.LastValue.dDouble = m_Camera.TimerDuration.GetValue() ;
          P.LastValue.iInt = (int) (P.LastValue.i64Int = (int64_t) P.LastValue.dDouble) ;
        }
        else if ( m_Camera.TimerDurationAbs.IsValid() )
        {
          iMin = (int) ceil( m_Camera.TimerDurationAbs.GetMin() ) ;
          iMax = (int) floor( m_Camera.TimerDurationAbs.GetMax() ) ;
          P.LastValue.dDouble = m_Camera.TimerDelayAbs.GetValue() ;
          P.LastValue.iInt = (int) (P.LastValue.i64Int = (int64_t) P.LastValue.dDouble) ;
        }
        else if ( m_Camera.TimerDurationRaw.IsValid() )
        {
          iMin = (int) m_Camera.TimerDurationRaw.GetMin() ;
          iMax = (int) m_Camera.TimerDurationRaw.GetMax() ;
          P.LastValue.dDouble = (double) (P.LastValue.i64Int
            = m_Camera.TimerDurationRaw.GetValue()) ;
          P.LastValue.iInt = (int) P.LastValue.i64Int ;
        }
        if ( iMax )
        {
          P.m_GUIFormat.Format( "%s(%s,%d,%d)" , SETUP_SPIN ,
            P.name , iMin , iMax );
          P.bSupported = true;
          if (iMin != iMax)
          {
            P.LastValue.dMin = ( double ) iMin ;
            P.LastValue.dMax = ( double ) iMax ;
          }
        }
      }
      break;
    case OUTPUT_SELECTOR:
      {
        try
        {
          if ( m_Camera.UserOutputSelector.IsValid() )
          {
            P.m_GUIFormat.Format( "ComboBox(%s(%s))" , P.name , (LPCTSTR) ForCombo ) ;
            P.bSupported = true;
            P.LastValue.iInt = (int) ((P.LastValue.i64Int) =
              m_Camera.UserOutputSelector.GetValue()) ;
            strcpy_s( P.LastValue.szAsString ,
              m_Camera.UserOutputSelector.ToString().c_str() ) ;
          }
        }
        catch ( const GenericException &e )
        {
          bool bOpened = m_Camera.IsOpen() ;
          // Error handling
          FXString CatchData ;
          CatchData.Format( _T( "  Error Build Prop %s (Type %s): %s\n" ) ,
            (LPCTSTR) P.name , GetTypeName( P.m_DataType ) , e.GetDescription() ) ;
          SEND_GADGET_ERR( (LPCTSTR) CatchData );
        }
      }
      break;
    case OUTPUT_VALUE:
      {
        try
        {
          if ( m_Camera.UserOutputValue.IsValid() )
          {
            P.m_GUIFormat.Format( "ComboBox(%s(Off(0),On(1)))" , P.name , (LPCTSTR) ForCombo ) ;
            P.bSupported = true;
            P.LastValue.bBool = m_Camera.UserOutputValue.GetValue() ;
            P.LastValue.i64Int = P.LastValue.iInt = (int) P.LastValue.bBool ;
            strcpy_s( P.LastValue.szAsString ,
              m_Camera.UserOutputValue.ToString().c_str() ) ;
          }
        }
        catch ( const GenericException &e )
        {
          bool bOpened = m_Camera.IsOpen() ;
          // Error handling
          FXString CatchData ;
          CatchData.Format( _T( "  Error Build Prop %s (Type %s): %s\n" ) ,
            (LPCTSTR) P.name , GetTypeName( P.m_DataType ) , e.GetDescription() ) ;
          SEND_GADGET_ERR( (LPCTSTR) CatchData );
        }
      }
      break;

    case SAVE_SETTINGS:
      {
        INodeMap * pNodeMap = GetNodeMapWithCatching() ;
        if (!pNodeMap)
          return false ;

        LPCTSTR * pInCamNames = _getPropertyInCamNames( SAVE_SETTINGS ) ;
        INode * pSetNode = NULL ;
        while ( !pSetNode && (*pInCamNames[ 0 ]) )
        {
          INode * pSetNode = pNodeMap->GetNode( *pInCamNames ) ;
          if ( pSetNode )
          {
            P.m_GUIFormat.Format( "%s(%s,0,1)" , SETUP_SPIN , P.name );
            P.bSupported = true;
            P.m_bDontShow = false ;
            break ;
          }
        }

      }
      break ;
    case USER_SET_SELECTOR:
      {
        if ( m_Camera.UserSetSelector.IsValid() && m_Camera.UserSetSelector.IsWritable() )
        {
          P.m_GUIFormat.Format( "ComboBox(%s(%s))" , P.name , (LPCTSTR) ForCombo ) ;
          P.LastValue.i64Int = P.LastValue.iInt =
            (int) m_Camera.UserSetSelector.GetValue() ;
          _tcscpy_s( P.LastValue.szAsString ,
            m_Camera.UserSetSelector.ToString() ) ;
          P.m_bDontShow = false ;
          P.bSupported = true;
        }
      }
      break ;
    case EVENT_SELECTOR:
      {
        if ( P.m_EnumeratorsAsTexts.size() > 1 )
        {
          P.m_GUIFormat.Format( "ComboBox(%s(%s))" , P.name , (LPCTSTR) ForCombo ) ;
          P.bSupported = true;
        }
      }
      break ;
    case EVENT_ENABLE:
      {
        if ( P.m_EnumeratorsAsTexts.size() > 1 )
        {
          P.m_GUIFormat.Format( "ComboBox(%s(%s))" , P.name , (LPCTSTR) ForCombo ) ;
          P.bSupported = true;
        }
      }
      break ;
    case EVENTS_STATUS:
      {
        P.m_bDontShow = true;
        break;
      }

    default:
      {
        SEND_DEVICE_ERR( "Undefined property %d(%d):'%s'(%s)" ,
          i , m_cBProperties[ i ].Id ,
          m_cBProperties[ i ].ppNodesNames[ 0 ] ,
          m_cBProperties[ i ].pUIName );
      }
      break;
    }
    if ( !P.m_GUIFormat.IsEmpty() || P.m_bDontShow )
    {
      m_PropertyListLock.Lock( INFINITE , "BaslerBuildPropertyList - Add Property" );
      m_ThisCamProperty.Add( P );
      m_PropertyListLock.Unlock() ;
      if ( m_SavedCamProperty.size() )
      {
        for ( int i = 0 ; i < m_SavedCamProperty.size() ; i++ )
        {
          if ( P.id == m_SavedCamProperty[i].id )
          {
            BaslerCamProperty& Prop = m_SavedCamProperty.GetAt( i ) ;
            SetCamPropertyData Data ;
            Data.m_iIndex = (int)m_ThisCamProperty.GetUpperBound() ;
            Data.m_bAuto = Prop.m_bAuto ;
            Data.m_bBool = Prop.LastValue.bBool ;
            Data.m_int = Prop.LastValue.iInt ;
            Data.m_int64 = Prop.LastValue.i64Int ;
            Data.m_double = Prop.LastValue.dDouble ;
            Data.m_bBool = Prop.LastValue.bBool ;
            Data.m_Type = Prop.id ;
            strcpy_s( Data.m_szString , Prop.LastValue.szAsString ) ;
            BaslerSetCameraProperty( Data.m_iIndex , &Data ) ;
          }
        }
      }
    }
  }

  int iTriggerModeIndex = -1 ;
  for ( int i = 0 ; i < m_SavedCamProperty.size() ; i++ )
  {
    BaslerCamProperty& Prop = m_SavedCamProperty.GetAt( i ) ;
    
    SetCamPropertyData Data ;
    Data.m_iIndex = GetThisCamPropertyIndex( Prop.name ) ;
    if ( Prop.id == TRIGGER_MODE )
    {
      iTriggerModeIndex = Data.m_iIndex ;
    }
    if ( Data.m_iIndex >= 0 )
    {
      Data.m_bAuto = Prop.m_bAuto ;
      Data.m_bBool = Prop.LastValue.bBool ;
      Data.m_int = Prop.LastValue.iInt ;
      Data.m_int64 = Prop.LastValue.i64Int ;
      Data.m_double = Prop.LastValue.dDouble ;
      Data.m_bBool = Prop.LastValue.bBool ;
      Data.m_Type = Prop.id ;
      strcpy_s( Data.m_szString , Prop.LastValue.szAsString ) ;
      BaslerSetCameraProperty( Data.m_iIndex , &Data ) ;
      if ( iTriggerModeIndex > 0 )
      {
        SetCamPropertyData ReadData ;
        BaslerGetCameraProperty( iTriggerModeIndex , &ReadData ) ;
        ASSERT( ReadData.m_int >= 0 ) ;
      }
    }
  }
 
  SetComplexIOs( m_SavedComplexIOs ) ;
  m_SavedCamProperty.RemoveAll() ;
  m_SavedComplexIOs.clear() ;
  if ( iTriggerModeIndex > 0 )
  {
    SetCamPropertyData ReadData ;
    BaslerGetCameraProperty( iTriggerModeIndex , &ReadData ) ;
    ASSERT( ReadData.m_int > 0 ) ;
  }

  return true;
}

bool Basler::GetCameraProperty( FXCamPropertyType Type ,
  INT_PTR &value , bool& bauto )
{
  int iIndex = GetThisCamPropertyIndex( Type );
  if ( iIndex >= 0 )
  {
    if ( Type == RESTART_TIMEOUT )
    {
      value = m_iRestartTimeOut_ms;
      return true;
    }
    return GetCameraProperty( iIndex , value , bauto );
  }
  return false;
}


bool Basler::GetCameraProperty( int iIndex , INT_PTR &value , bool& bauto )
{
  m_PropertyDataReturn.Reset();
  if ( !OtherThreadGetProperty( iIndex , &m_PropertyDataReturn ) )
    return false;
  bauto = m_PropertyDataReturn.m_bAuto;
  FXAutolock al( m_PropertyListLock , "Basler::GetCameraProperty" );
  switch ( m_ThisCamProperty[ iIndex ].m_DataType )
  {
  case PDTCommand:
  case PDTInt:
    value = m_PropertyDataReturn.m_int;
    m_ThisCamProperty[ iIndex ].LastValue.iInt = m_PropertyDataReturn.m_int;
    return true;
  case PDTBool:
    value = (INT_PTR) m_PropertyDataReturn.m_bBool;
    m_ThisCamProperty[ iIndex ].LastValue.iInt = (int) value;
    m_ThisCamProperty[ iIndex ].LastValue.bBool = (value != 0);
    return true;
  case PDTEnum:
    value = (INT_PTR) m_PropertyDataReturn.m_int64 ;
    _tcscpy_s( m_ThisCamProperty[ iIndex ].LastValue.szAsString , m_PropertyDataReturn.m_szString );
    m_ThisCamProperty[ iIndex ].LastValue.i64Int = m_PropertyDataReturn.m_int64 ;
    return true;
  case PDTString:
    value = (INT_PTR) m_PropertyDataReturn.m_szString;
    _tcscpy_s( m_ThisCamProperty[ iIndex ].LastValue.szAsString , m_PropertyDataReturn.m_szString );
    return true;
  case PDTFloat:
    m_ThisCamProperty[ iIndex ].LastValue.dDouble = m_PropertyDataReturn.m_double;
    sprintf_s( m_PropertyDataReturn.m_szString , "%g" , m_PropertyDataReturn.m_double );
    value = (INT_PTR) m_PropertyDataReturn.m_szString;
    return true;
  }
  return false;
}

bool Basler::BaslerGetCameraProperty( int iIndex , SetCamPropertyData * pData )
{
  if ( !pData )
    return false;
  if ( m_Camera.IsCameraDeviceRemoved() || !m_Camera.IsOpen() )
  {
    if ( !BaslerCamInit() )
      return false;
  }
  FXAutolock al( m_PropertyListLock , "Basler::BaslerGetCameraProperty" );
  FXCamPropertyType Type = GetThisCamPropertyType( iIndex );
  if ( Type == UNSPECIFIED_PROPERTY_TYPE )
    return false;

  BaslerCamProperty * pProp = GetThisCamProperty( Type ) ;
  if ( !pProp )
    return false ;
  const GenICam_3_1_Basler_pylon::gcstring PropName( (LPCTSTR) pProp->m_InDeviceName );

  bool bExternal = false ;
  switch ( pProp->id )
  {
  case SETROI: bExternal = true ; break ;
  }

  INodeMap * pNodeMap = GetNodeMapWithCatching() ;
  if (!pNodeMap)
    return false ;

  INodeMap& nodemap = *pNodeMap;

  pData->m_bAuto = false;
  pData->m_bBool = false;
  try
  {
    switch ( Type )
    {
    case FORMAT_MODE:
      {
        pProp->LastValue.i64Int = pData->m_int64
          = m_Camera.PixelFormat.GetIntValue() ;
        pData->m_int = (int)(m_LastEmbedInfo.m_FrameFormat = pData->m_int64)  ;
        _tcscpy_s( pProp->LastValue.szAsString ,
          m_Camera.PixelFormat.ToString().c_str() ) ;
        _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
        return true;
      }
    case SETROI:
      {
        static FXString sROI;
        CRect rc;
        GetROI( rc );
        sprintf_s( pData->m_szString , "%d,%d,%d,%d" , rc.left , rc.top , rc.right , rc.bottom );
        m_LastEmbedInfo.m_ROI = rc;
        return true;
      }
    case TRIGGER_MODE:
      {
        if ( m_Camera.TriggerMode.IsValid() && m_Camera.TriggerMode.IsReadable() )
        {
          TriggerModeEnums NewTriggerMode = m_Camera.TriggerMode.GetValue() ;
          _tcscpy_s( pProp->LastValue.szAsString ,
            m_Camera.TriggerMode.ToString().c_str() ) ;
          _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
          pProp->LastValue.i64Int = pData->m_int64 = pData->m_int = pProp->LastValue.iInt =
            m_LastEmbedInfo.m_iTriggerMode = m_LastEmbedInfo.m_iTriggerMode = (int) (m_TriggerMode = NewTriggerMode) ;
        }
        else
          m_TriggerMode = (TriggerModeEnums) (-1) ;
        return true;
      }
    case TRIGGER_SELECTOR:
      {
        if (m_Camera.TriggerSelector.IsValid())
        {
          pData->m_int64 = m_Camera.TriggerSelector.GetIntValue() ;
          m_LastEmbedInfo.m_iTriggerSelector =
            pData->m_int = pProp->LastValue.iInt =
            ( int ) ( pProp->LastValue.i64Int = pData->m_int64 ) ;
          _tcscpy_s( pProp->LastValue.szAsString ,
            m_Camera.TriggerSelector.ToString().c_str() ) ;
          _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
          return true;
        }
        return false ;
//         if ( m_Camera.TriggerSelector.IsValid() && m_Camera.TriggerSelector.IsReadable() )
//         {
//         }
//         INode * pNode = nodemap.GetNode( (LPCTSTR) (pProp->m_InDeviceName) ) ;
//         if ( pNode )
//         {
//           CEnumParameter TriggerSelector( pNode ) ;
//           if ( TriggerSelector.IsValid() )
//           {
//             FXString sReadVal = TriggerSelector.GetValue().c_str() ;
//             _tcscpy_s( pProp->LastValue.szAsString , sReadVal ) ;
//             _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
//             pData->m_int64 = m_LastEmbedInfo.m_iTriggerSelector = 
//               pData->m_int = (int) TriggerSelector.GetIntValue() ;
//             return true ;
//           }
//         }
      }
    case BURST_LENGTH:
      {
        if ( (m_TriggerMode == TriggerMode_On) )
        {
          if ( m_Camera.AcquisitionBurstFrameCount.IsValid() )
          {
            pData->m_int = ( int ) ( ( pProp->LastValue.i64Int ) =
              m_Camera.AcquisitionBurstFrameCount.GetValue() ) ;
            m_LastEmbedInfo.m_iBurstLength = pData->m_int ;
            return true ;
          }
        }
        else if (m_Camera.AcquisitionFrameCount.IsValid())
        {
          pData->m_int = ( int ) ( ( pProp->LastValue.i64Int ) =
            m_Camera.AcquisitionFrameCount.GetValue() ) ;
          m_LastEmbedInfo.m_iBurstLength = pData->m_int ;
          return true ;
        }
        return false ;
      }
    case TRIGGER_SOURCE:
      {
        INode * pNode = nodemap.GetNode( (LPCTSTR) (pProp->m_InDeviceName) ) ;
        if ( pNode )
        {
          CEnumParameter TriggerSource( pNode ) ;
          if ( TriggerSource.IsValid() )
          {
            FXString sReadVal = TriggerSource.GetValue().c_str() ;
            _tcscpy_s( pProp->LastValue.szAsString , sReadVal ) ;
            _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
            pData->m_int64 = pData->m_int = (int) TriggerSource.GetIntValue() ;
            m_bSoftwareTriggerMode =
              ( _tcsicmp( pProp->LastValue.szAsString , _T( "Software" ) ) == 0 ) ;
            return true ;
          }
        }

        //         if ( m_Camera.TriggerSource.IsValid() )
        //         {
        //           INode * pNode = m_Camera.TriggerSource.GetNode() ;
        //           CEnumerationPtr Param( pNode ) ;
        //           m_LastEmbedInfo.m_iTriggerSelector =
        //             pData->m_int = pProp->LastValue.iInt =
        //             (int) (pProp->LastValue.i64Int =
        //             pData->m_int64 = Param->GetIntValue());
        // 
        //           _tcscpy_s( pProp->LastValue.szAsString ,
        //             Param->GetCurrentEntry()->GetSymbolic().c_str() ) ;
        //           _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
        //           return true;
        //         }
        return false ;
      }

    case TRIGGER_ACTIVATE:
      {
        if ( m_Camera.TriggerActivation.IsValid() )
        {
          INode * pNode = m_Camera.TriggerActivation.GetNode() ;
          CEnumerationPtr Param( pNode ) ;
          m_LastEmbedInfo.m_iTriggerSelector =
            pData->m_int = pProp->LastValue.iInt =
            (int) (pProp->LastValue.i64Int =
            pData->m_int64 = Param->GetIntValue());

          _tcscpy_s( pProp->LastValue.szAsString ,
            Param->GetCurrentEntry()->GetSymbolic().c_str() ) ;
          _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
          return true;
        }
        return false ;
      }
    case TRIGGER_DELAY:
      {
        if ( m_Camera.TriggerDelay.IsValid() )
        {
          m_LastEmbedInfo.m_iTriggerDelay = pData->m_int =
            ROUND( pData->m_double = m_Camera.TriggerDelay.GetValue() ) ;
        }
        else if ( m_Camera.TriggerDelayAbs.IsValid() )
        {
          m_LastEmbedInfo.m_iTriggerDelay = pData->m_int =
            ROUND( pData->m_double = m_Camera.TriggerDelayAbs.GetValue() ) ;
        }
        else
          return false ;
        return true ;
      }
    case TRIGGER_SOFT_FIRE:
      pData->m_int64 = pData->m_int = 0 ;
      return true ;
    case TIMER_SELECT:
      {
        if ( m_Camera.TimerSelector.IsValid() )
        {
          INode * pNode = m_Camera.TimerSelector.GetNode() ;
          CEnumerationPtr Param( pNode ) ;
          pData->m_int = pProp->LastValue.iInt =
            (int) (pProp->LastValue.i64Int =
            pData->m_int64 = Param->GetIntValue());

          _tcscpy_s( pProp->LastValue.szAsString ,
            Param->GetCurrentEntry()->GetSymbolic().c_str() ) ;
          _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
          return true;
        }
        return false ;
      }
    case TIMER_TRIGGER_SOURCE:
      {
        INode * pNode = nodemap.GetNode( (LPCTSTR) (pProp->m_InDeviceName) ) ;
        if ( pNode )
        {
          CEnumParameter TriggerSource( pNode ) ;
          if ( TriggerSource.IsValid() )
          {
            FXString sReadVal = TriggerSource.GetValue().c_str() ;
            _tcscpy_s( pProp->LastValue.szAsString , sReadVal ) ;
            _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
            pData->m_int64 = pData->m_int = (int) TriggerSource.GetIntValue() ;
            return true ;
          }
        }
        //         if ( m_Camera.TimerTriggerSource.IsValid() )
        //         {
        //           INode * pNode = m_Camera.TimerTriggerSource.GetNode() ;
        //           CEnumerationPtr Param( pNode ) ;
        //           pData->m_int = pProp->LastValue.iInt =
        //             (int) (pProp->LastValue.i64Int =
        //             pData->m_int64 = Param->GetIntValue());
        // 
        //           _tcscpy_s( pProp->LastValue.szAsString ,
        //             Param->GetCurrentEntry()->GetSymbolic().c_str() ) ;
        //           _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
        //           return true;
        //         }
        return false ;
      }
    case TIMER_TRIGGER_POLARITY:
    case TIMER_TRIGGER_ACTIVATION:
      {
        if ( m_Camera.TimerTriggerActivation.IsValid() )
        {
          INode * pNode = m_Camera.TimerTriggerActivation.GetNode() ;
          CEnumerationPtr Param( pNode ) ;
          pData->m_int = pProp->LastValue.iInt =
            (int) (pProp->LastValue.i64Int =
            pData->m_int64 = Param->GetIntValue());

          _tcscpy_s( pProp->LastValue.szAsString ,
            Param->GetCurrentEntry()->GetSymbolic().c_str() ) ;
          _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
          return true;
        }
        return false ;
      }
    case TIMER_DELAY:
      {
        if ( m_Camera.TimerDelay.IsValid() )
          pData->m_int64 = pData->m_int =
          ROUND( pData->m_double = m_Camera.TimerDelay.GetValue() ) ;
        else if ( m_Camera.TimerDelayAbs.IsValid() )
          pData->m_int64 = pData->m_int =
          ROUND( pData->m_double = m_Camera.TimerDelayAbs.GetValue() ) ;
        else if ( m_Camera.TimerDelayRaw.IsValid() )
          pData->m_int = (int)
          (pData->m_int64 = m_Camera.TimerDelayRaw.GetValue()) ;
        else
          return false ;
        return true ;
      }
    case TIMER_DURATION:
      {
        if ( m_Camera.TimerDuration.IsValid() )
          pData->m_int64 = pData->m_int =
          ROUND( pData->m_double = m_Camera.TimerDuration.GetValue() ) ;
        else if ( m_Camera.TimerDurationAbs.IsValid() )
          pData->m_int64 = pData->m_int =
          ROUND( pData->m_double = m_Camera.TimerDurationAbs.GetValue() ) ;
        else if ( m_Camera.TimerDurationRaw.IsValid() )
          pData->m_int = (int)
          (pData->m_int64 = m_Camera.TimerDurationRaw.GetValue()) ;
        else
          return false ;
        return true ;
      }
    case EXPOSURE_MODE:
      {
        if ( m_Camera.ExposureMode.IsValid() )
        {
          INode * pNode = m_Camera.ExposureMode.GetNode() ;
          CEnumerationPtr Param( pNode ) ;
          pData->m_int = pProp->LastValue.iInt =
            (int) (pProp->LastValue.i64Int =
            pData->m_int64 = Param->GetIntValue());

          _tcscpy_s( pProp->LastValue.szAsString ,
            Param->GetCurrentEntry()->GetSymbolic().c_str() ) ;
          _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
          return true;
        }
        return false ;
      }
    case SHUTTER_AUTO:
      {
        if ( m_Camera.ExposureAuto.IsValid() )
        {
          ExposureAutoEnums Value = m_Camera.ExposureAuto.GetValue() ;
//           pData->m_int64 = pData->m_int = (int) (Value == ExposureAuto_Continuous) ;
          pData->m_bBool = pData->m_bAuto = (Value == ExposureAuto_Continuous) ;
        }
        else
          pData->m_bBool = pData->m_bAuto = false ;
        if (m_Camera.ExposureTime.IsValid())
        {
          pData->m_int64 = pData->m_int = pProp->LastValue.iInt = 
            ROUND( pData->m_double =
              m_LastEmbedInfo.m_dExposure_us = m_Camera.ExposureTime.GetValue() ) ;
        }
        else if (m_Camera.ExposureTimeAbs.IsValid())
        {
          pData->m_int64 = pData->m_int = pProp->LastValue.iInt =
            ROUND( pData->m_double =
              m_LastEmbedInfo.m_dExposure_us = m_Camera.ExposureTimeAbs.GetValue() );
        }
        else if (m_Camera.ExposureTimeRaw.IsValid())
        {
          pData->m_int64 = pData->m_int = pProp->LastValue.iInt =
            ROUND( pData->m_double =
              m_LastEmbedInfo.m_dExposure_us = (double)m_Camera.ExposureTimeRaw.GetValue() ) ;
        }
        else
          pData->m_int64 = pData->m_int = pProp->LastValue.iInt ;
        return true ;
      }
    case GAIN_AUTO:
      {
        if ( m_Camera.GainAuto.IsValid() )
        {
          GainAutoEnums Value = m_Camera.GainAuto.GetValue() ;
          //pData->m_int64 = pData->m_int = (int) (Value == GainAuto_Continuous) ;
          pData->m_bBool = pData->m_bAuto = (Value == GainAuto_Continuous) ;
        }
        else
          pData->m_bBool = pData->m_bAuto = false ;
      }
      if (m_Camera.Gain.IsValid())
      {
        pData->m_int64 = pData->m_int = pProp->LastValue.iInt = ROUND( pData->m_double
          = ( m_LastEmbedInfo.m_dGain_dB = m_Camera.Gain.GetValue() ) * 10. ) ;
        if (m_SetupObject)
        {
          CGadgetStdSetup * pSetupDlg = ( CGadgetStdSetup* ) m_SetupObject ;
          pSetupDlg->SetCellInt( "Gain_dBx10" , pProp->LastValue.iInt ) ;
        }
      }
      else if (m_Camera.GainAbs.IsValid())
      {
        pData->m_int64 = pData->m_int = pProp->LastValue.iInt = ROUND( pData->m_double
          = ( m_LastEmbedInfo.m_dGain_dB = m_Camera.GainAbs.GetValue() ) * 10. ) ;
      }
      else if (m_Camera.GainRaw.IsValid())
      {
        pData->m_int64 = pData->m_int = pProp->LastValue.iInt = ROUND( pData->m_double
          = ( m_LastEmbedInfo.m_dGain_dB = (double)m_Camera.GainRaw.GetValue() ) ) ;
      }
      else
        m_LastEmbedInfo.m_dGain_dB = ( double ) ( pData->m_int64 = pData->m_int = pProp->LastValue.iInt ) ;
      return true ;
    case SHUTTER:
      {
        if (m_Camera.ExposureTime.IsValid())
        {
          pData->m_int64 = pData->m_int = pProp->LastValue.iInt =
            ROUND( pData->m_double =
              m_LastEmbedInfo.m_dExposure_us = m_Camera.ExposureTime.GetValue() ) ;
        }
        else if (m_Camera.ExposureTimeAbs.IsValid())
        {
          pData->m_int64 = pData->m_int = pProp->LastValue.iInt =
            ROUND( pData->m_double =
              m_LastEmbedInfo.m_dExposure_us = m_Camera.ExposureTimeAbs.GetValue() );
        }
        else if (m_Camera.ExposureTimeRaw.IsValid())
        {
          pData->m_int64 = pData->m_int = pProp->LastValue.iInt =
            ROUND( pData->m_double =
              m_LastEmbedInfo.m_dExposure_us = ( double ) m_Camera.ExposureTimeRaw.GetValue() ) ;
        }
        else
          pData->m_int64 = pData->m_int = pProp->LastValue.iInt ;

        if (m_Camera.ExposureAuto.IsValid())
        {
          ExposureAutoEnums Value = m_Camera.ExposureAuto.GetValue() ;
            //pData->m_int64 = pData->m_int = (int) (Value == ExposureAuto_Continuous) ;
          pData->m_bBool = pData->m_bAuto = (Value == ExposureAuto_Continuous) ;
        }
        else
          pData->m_bBool = pData->m_bAuto = false ;
        return true ;
      }
    case GAIN:
      {
        GetGain_dBx10( pProp ) ;
        pData->m_double = (double)(pData->m_int64 = pData->m_int = pProp->LastValue.iInt) ;
        pData->m_bAuto = pData->m_bBool = pProp->m_bAuto ;
        return true ;
      }
    case BASLER_PACKETSIZE:
      {
        if ( m_Camera.GevSCPSPacketSize.IsValid() )
        {
          pData->m_int = (int) (pData->m_int64 = m_Camera.GevSCPSPacketSize.GetValue()) ;
          return true ;
        }
        return false ;
      }
      break ;
    case BASLER_INTER_PACKET_DELAY:
      {
        if ( m_Camera.GevSCPD.IsValid() )
        {
          pData->m_int = (int) (pData->m_int64 = m_Camera.GevSCPD.GetValue()) ;
          return true ;
        }
        return false ;
      }
      break ;
    case BRIGHTNESS:
      {
        if ( m_Camera.BlackLevel.IsValid() )
          pData->m_int = ROUND( pData->m_double =
            (m_LastEmbedInfo.m_dBrightness = m_Camera.BlackLevel.GetValue()) * 10. ) ;
        else if ( m_Camera.BlackLevelAbs.IsValid() )
          pData->m_int = ROUND( pData->m_double =
            (m_LastEmbedInfo.m_dBrightness = m_Camera.BlackLevel.GetValue()) * 10. ) ;
        else if ( m_Camera.BlackLevelRaw.IsValid() )
          m_LastEmbedInfo.m_dBrightness = pData->m_double = pData->m_int =
          (int) (pData->m_int64 = m_Camera.BlackLevelRaw.GetValue()) ;
        else
          return false ;
        return true ;
      }
    case GAMMA:
      {
        if ( m_Camera.Gamma.IsValid() )
        {
          pData->m_double = m_Camera.Gamma.GetValue() ;
          m_LastEmbedInfo.m_iGamma = ROUND( pData->m_double * 1000. ) ;
          return true;
        }
        return false ;
      }
    case FRAME_RATE:
      {
        if ( m_Camera.AcquisitionFrameRate.IsValid() )
          pData->m_double = m_Camera.AcquisitionFrameRate.GetValue() ;
        else if ( m_Camera.AcquisitionFrameRateAbs.IsValid() )
          pData->m_double = m_Camera.AcquisitionFrameRateAbs.GetValue() ;
        else
          return false ;
        m_fLastRealFrameRate = (float) pData->m_double ;
        return true;
      }
    case FRAME_RATE_ENABLE:
      {
        if ( m_Camera.AcquisitionFrameRateEnable.IsValid() )
        {
          pData->m_bBool = m_Camera.AcquisitionFrameRateEnable.GetValue() ;
          return true ;
        }
        return false ;
      }
    case LINE_SELECTOR:
      {
        if ( m_Camera.LineSelector.IsValid() )
        {
          INode * pNode = m_Camera.LineSelector.GetNode() ;
          CEnumerationPtr Param( pNode ) ;
          pData->m_int = (int) (pData->m_int64 = Param->GetIntValue()) ;
          _tcscpy_s( pProp->LastValue.szAsString ,
            Param->GetCurrentEntry()->GetSymbolic().c_str() ) ;
          _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
          return true;
        }
        return false ;
      }
    case LINE_MODE:
      {
        if ( m_Camera.LineMode.IsValid() )
        {
          INode * pNode = m_Camera.LineMode.GetNode() ;

          CEnumerationPtr Param( pNode ) ;
          pData->m_int = (int) (pData->m_int64 = Param->GetIntValue()) ;
          _tcscpy_s( pProp->LastValue.szAsString ,
            Param->GetCurrentEntry()->GetSymbolic().c_str() ) ;
          _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
          return true;
        }
        return false ;
      }
    case LINE_SOURCE:
      {
        if ( m_Camera.LineSource.IsValid() )
        {
          INode * pNode = m_Camera.LineSource.GetNode() ;
          CEnumerationPtr Param( pNode ) ;
          pData->m_int = (int) (pData->m_int64 = Param->GetIntValue()) ;
          int  iDirect = (int) m_Camera.LineSource.GetIntValue() ;
          FXString AsText = m_Camera.LineSource.ToString().c_str() ;
          _tcscpy_s( pProp->LastValue.szAsString ,
            Param->GetCurrentEntry()->GetSymbolic().c_str() ) ;
          _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
          return true;
        }
        return false ;
      }
    case LINE_INVERTER:
      {
        if ( m_Camera.LineInverter.IsValid() )
        {
          pData->m_bBool = m_Camera.LineInverter.GetValue() ;
          return true ;
        }
        return false ;
      }
      break ;
    case OUTPUT_SELECTOR:
      {
        if ( m_Camera.UserOutputSelector.IsValid() )
        {
          INode * pNode = m_Camera.UserOutputSelector.GetNode() ;
          CEnumerationPtr Param( pNode ) ;
          pData->m_int = (int) (pData->m_int64 = Param->GetIntValue()) ;
          int  iDirect = (int) m_Camera.UserOutputSelector.GetIntValue() ;
          FXString AsText = m_Camera.UserOutputSelector.ToString().c_str() ;
          _tcscpy_s( pProp->LastValue.szAsString ,
            Param->GetCurrentEntry()->GetSymbolic().c_str() ) ;
          _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
          return true;
        }
        return false ;
      }
      break;
    case OUTPUT_VALUE:
      {
        if ( m_Camera.UserOutputValue.IsValid() )
        {
            pData->m_int64 = pData->m_int = (int)( pData->m_bBool = m_Camera.UserOutputValue.GetValue() );
            return true ;
        }
        return false ;
      }
      break;
    case USER_SET_SELECTOR:
      {  // this property is working only in STOP state
        if ( m_Camera.UserSetSelector.IsValid() && m_Camera.UserSetSelector.IsWritable() )
        {
          INode * pNode = m_Camera.UserSetSelector.GetNode() ;

          CEnumerationPtr Param( pNode ) ;
          pProp->LastValue.iInt = pData->m_int =
            (int) (pData->m_int64 = pProp->LastValue.i64Int = Param->GetIntValue()) ;
          _tcscpy_s( pProp->LastValue.szAsString ,
            Param->GetCurrentEntry()->GetSymbolic().c_str() ) ;
          _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;

          return true ;
        }
        return false ;
      }
    case EVENT_SELECTOR:
      {
        if ( m_Camera.EventSelector.IsValid() )
        {
          pData->m_int64 = m_Camera.EventSelector.GetValue() ;
          _tcscpy_s( pProp->LastValue.szAsString ,
            m_Camera.EventSelector.ToString() ) ;
          _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
          return true;
        }
        return false ;
      }
      break ;
    case EVENT_ENABLE:
      {
        if ( m_Camera.EventNotification.IsValid() )
        {
          pData->m_int = (int)m_Camera.EventSelector.GetValue() ;
          int64_t i64Val = INT_MAX ;
          switch ( pData->m_int )
          {
            case EventSelector_ExposureEnd: 
              {
                if ( m_Camera.EventExposureEnd.IsValid() )
                {
                  if (m_Camera.EventExposureEnd.IsReadable())
                    i64Val = m_Camera.EventExposureEnd.GetValue() ;
                  else
                    i64Val = 0 ;
                }
              }
              break ;
//             case EventSelector_FrameStart: 
//               i64Val = m_Camera.EventFrameStart.GetValue() ; break ;
//           case EventSelector_FrameBurstStart: 
//             i64Val = m_Camera.EventFrameBurstStart.GetValue() ; break ;
//           case EventSelector_FrameStartOvertrigger:
//             i64Val = m_Camera.EventFrameStartOvertrigger.GetValue() ; break ;
//           case EventSelector_FrameBurstStartOvertrigger:
//             i64Val = m_Camera.EventFrameBurstStartOvertrigger.GetValue() ; break ;
//           case EventSelector_CriticalTemperature:
//             i64Val = m_Camera.EventCriticalTemperature.GetValue() ; break ;
//           case EventSelector_OverTemperature:
//             i64Val = m_Camera.EventOverTemperature.GetValue() ; break ;
//           case EventSelector_FrameStartWait:
//             i64Val = m_Camera.EventFrameStartWait.GetValue() ; break ;
//           case EventSelector_FrameBurstStartWait:
//             i64Val = m_Camera.EventFrameBurstStartWait.GetValue() ; break ;
//           case EventSelector_LineStartOvertrigger:
//           case EventSelector_AcquisitionStartOvertrigger:
//           case EventSelector_FrameTimeout:
//           case EventSelector_AcquisitionStart:
//           case EventSelector_ActionLate:
//           case EventSelector_Line1RisingEdge:
//           case EventSelector_Line2RisingEdge:
//           case EventSelector_Line3RisingEdge:
//           case EventSelector_Line4RisingEdge:
// //           case EventSelector_VirtualLine1RisingEdge:
// //           case EventSelector_VirtualLine2RisingEdge:
// //           case EventSelector_VirtualLine3RisingEdge:
// //           case EventSelector_VirtualLine4RisingEdge:
//           case EventSelector_FrameWait:
//           case EventSelector_AcquisitionWait:
//           case EventSelector_AcquisitionStartWait:
//           case EventSelector_EventOverrun:
// // 
          }
//           _tcscpy_s( pProp->LastValue.szAsString ,
//             m_Camera.UserSetSelector.ToString() ) ;
//           _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
//           return true;
//         }
          EventNotificationEnums Val = m_Camera.EventNotification.GetValue() ;
          pData->m_bBool = (Val != EventNotification_Off) ;
          pData->m_int64 = pData->m_int = (int) Val ;
          _tcscpy_s( pProp->LastValue.szAsString ,
            m_Camera.EventNotification.ToString().c_str() ) ;
          _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
          return true;
        }
        return false ;
      }
      break ;
    case SAVE_SETTINGS:
      pData->m_int64 = pData->m_int = 0 ;
      return true ;
    default:
      if ( !pProp->m_bDontShow )
        SENDERR( "Unsupported Property %d" , iIndex );
      break;
    }
  }
  catch ( const GenericException &e )
  {
    bool bOpen = m_Camera.IsOpen() ;
    // Error handling
    FXString CatchData ;
    CatchData.Format( _T( "GetProp: Exception for Node %s (Type %s): %s\n" ) ,
      (LPCTSTR) pProp->name , GetTypeName( pProp->m_DataType ) , e.GetDescription() ) ;
    SEND_GADGET_ERR( (LPCTSTR) CatchData );
  }
  return false;
}

bool Basler::BaslerSetCameraProperty( int iIndex , SetCamPropertyData * pData )
{
  if ( !pData )
    return false;
  if ( m_Camera.IsPylonDeviceAttached() )
  {
    if ( !m_Camera.IsOpen() )
      m_Camera.Open();
  }
  else if ( !BaslerCamInit() )
    return false;

  FXAutolock al( m_PropertyListLock , "Basler::BaslerSetCameraProperty" ) ;
  FXCamPropertyType Type = pData->m_Type ;/*GetThisCamPropertyType( iIndex );*/
  if ( Type == UNSPECIFIED_PROPERTY_TYPE )
    return false;
  SetCamPropertyData Data = *pData;

  BaslerCamProperty * pProp = &m_ThisCamProperty.GetAt( iIndex ) ;/* GetThisCamProperty( Type ) ;*/
  if ( !pProp )
    return false ;
  const GenICam_3_1_Basler_pylon::gcstring PropName( (LPCTSTR) pProp->m_InDeviceName );

  bool bExternal = false ;
  switch ( pProp->id )
  {
  case SETROI: bExternal = true ; break ;
  }
  try
  {
    INodeMap * pNodeMap = GetNodeMapWithCatching() ;
    if (!pNodeMap)
      return false ;

    INodeMap& nodemap = *pNodeMap;

    INode * pNode = !bExternal ? nodemap.GetNode( PropName ) : NULL ;
    if ( !pNode && !bExternal )
      return false ;

    //   if ( pProp->m_DataType == PDTEnum )
    //   {
    //     if ( pData->m_int >= (int)pProp->m_EnumeratorsAsValues.size() )
    //     {
    //       TRACE( "\n  Attention: Property %s enum index %d >= %d; changed to zero" ,
    //         (LPCTSTR)pProp->name , pData->m_int , pProp->m_EnumeratorsAsValues.size() ) ;
    //       pData->m_int = 0 ;
    //     }
    //   }
    switch ( Type )
    {
    case FORMAT_MODE:
      {
        if ( m_Camera.PixelFormat.IsValid() && m_Camera.PixelFormat.IsWritable() )
        {
          try
          {
            m_Camera.PixelFormat.SetIntValue( pData->m_int64 ) ;
          }
          catch ( const GenericException &e )
          {
            LPCTSTR pDescr = e.GetDescription();
            SENDERR( "Error on Set Pixel Format to %u - %s" ,
              pData->m_int , e.GetDescription() );
          }
          m_pixelFormat = m_Camera.PixelFormat.GetValue() ;
          pProp->LastValue.iInt = pData->m_int = (int)(pProp->LastValue.i64Int = m_LastEmbedInfo.m_FrameFormat );
          strcpy_s( pProp->LastValue.szAsString , m_Camera.PixelFormat.ToString().c_str() ) ;
          SetBMIH() ;
          SEND_DEVICE_INFO( "Pixel Format %s" ,
            m_Camera.PixelFormat.ToString().c_str() ) ;
        }
        return true;
      }
    case SETROI:
      {
        CRect rc;
        rc.left = 0;
        TRACE( "Basler::BaslerSetCameraProperty: SetROI %s  \n" , Data.m_szString );
        if ( sscanf( Data.m_szString , "%d,%d,%d,%d" ,
          &rc.left , &rc.top , &rc.right , &rc.bottom ) == 4 )
        {
          SetROI( rc );
          m_LastEmbedInfo.m_ROI = rc;
          pData->m_bInvalidate = true ;
          strcpy_s( pProp->LastValue.szAsString , Data.m_szString ) ;
          return true;
        }
        else if ( rc.left == -1 )
        {
          SetROI( rc ); // max ROI will be settled
          m_LastEmbedInfo.m_ROI = rc;
          return true ;
        }
        return false;
      }
    case TRIGGER_MODE:
      {
        if ( m_Camera.TriggerMode.IsValid() && m_Camera.TriggerMode.IsWritable() )
        {
          string CurrentMode = m_Camera.TriggerMode.ToString() ;
          m_Camera.TriggerMode.SetIntValue( pData->m_int ) ;
          string RealNewMode = m_Camera.TriggerMode.ToString() ;
          _tcscpy_s( pProp->LastValue.szAsString , RealNewMode.c_str() ) ;
          _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
          pProp->LastValue.i64Int = pData->m_int64 = pProp->LastValue.iInt =
            pData->m_int = m_LastEmbedInfo.m_iTriggerMode =
            m_LastEmbedInfo.m_iTriggerMode = m_TriggerMode =  m_Camera.TriggerMode.GetValue() ;
          pData->m_bInvalidate = true ;
          TRACE( " Basler::BaslerSetCameraProperty: Trigger %d(%s) \n" ,
            ( int ) Data.m_int64 , pData->m_szString ) ;
          return true ;
        }
        else
          m_TriggerMode = (TriggerModeEnums) (-1) ;
        return false;
      }
    case TRIGGER_SELECTOR:
      {
        if ( m_Camera.TriggerSelector.IsValid() )
        {
          int64_t iCurrMode = -1L ;
          if (m_Camera.TriggerMode.IsValid() && m_Camera.TriggerMode.IsReadable())
            iCurrMode = m_Camera.TriggerMode.GetIntValue() ;

          m_Camera.TriggerSelector.SetIntValue( pData->m_int64 ) ;
          m_LastEmbedInfo.m_iTriggerSelector = m_LastEmbedInfo.m_iTriggerSelector =
            pData->m_int = pProp->LastValue.iInt =
            (int) m_Camera.TriggerSelector.GetIntValue() ;
          _tcscpy_s( pProp->LastValue.szAsString ,
            m_Camera.TriggerSelector.ToString().c_str() ) ;
          _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;

          if ( (iCurrMode >= 0) && m_Camera.TriggerMode.IsWritable() )
            m_Camera.TriggerMode.SetIntValue( iCurrMode ) ;

          m_Camera.TriggerMode.FromString( "On" ) ;
          string RealNewMode = m_Camera.TriggerMode.ToString() ;
          pData->m_bInvalidate = true ;
          return true;
        }
        return false ;
      }
    case BURST_LENGTH:
      {
        BaslerCheckAndStopCapture() ;
        if ( m_Camera.AcquisitionBurstFrameCount.IsValid() 
          && m_Camera.AcquisitionBurstFrameCount.IsWritable() )
        {
          m_Camera.AcquisitionBurstFrameCount.SetValue( pData->m_int ) ;
          pProp->LastValue.i64Int = pProp->LastValue.iInt = pData->m_int ;
          m_LastEmbedInfo.m_iBurstLength = pData->m_int ;
          return true ;
        }
        else if (m_Camera.AcquisitionFrameCount.IsValid()
          &&     m_Camera.AcquisitionFrameCount.IsWritable() )
        {
          m_Camera.AcquisitionFrameCount.SetValue( pData->m_int ) ;
          pProp->LastValue.i64Int = pProp->LastValue.iInt = pData->m_int ;
          m_LastEmbedInfo.m_iBurstLength = pData->m_int ;
          return true ;
        }
        return false ;
      }
    case TRIGGER_ACTIVATE:
      {
        if ( m_Camera.TriggerActivation.IsValid() )
        {
          m_Camera.TriggerActivation.SetIntValue( pData->m_int64 ) ;
          m_LastEmbedInfo.m_iTriggerSelector =
            pData->m_int = pProp->LastValue.iInt =
            (int) (pProp->LastValue.i64Int = pData->m_int64) ;
          _tcscpy_s( pProp->LastValue.szAsString ,
            m_Camera.TriggerActivation.ToString().c_str() ) ;
          _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
          return true;
        }
        return false ;
      }
    case TRIGGER_SOURCE:
      {
        INode * pNode = nodemap.GetNode( (LPCTSTR) (pProp->m_InDeviceName) ) ;
        if ( pNode )
        {
          CEnumParameter TriggerSource( pNode ) ;
          if ( TriggerSource.IsValid() )
          {
            if ( pData->m_szString[ 0 ] )
              TriggerSource.SetValue( pData->m_szString );
            else
              TriggerSource.SetIntValue( pData->m_int64 ) ;

            FXString sReadVal = TriggerSource.GetValue().c_str() ;
            _tcscpy_s( pProp->LastValue.szAsString , sReadVal ) ;
            m_bSoftwareTriggerMode =
              ( _tcsicmp( pProp->LastValue.szAsString , _T( "Software" ) ) == 0 ) ;
            pProp->LastValue.i64Int = TriggerSource.GetIntValue() ;
            return true ;
          }
        }
        //         if ( m_Camera.TriggerSource.IsValid() )
        //         {
        //           m_Camera.TriggerSource.SetIntValue( pData->m_int64 ) ;
        //           m_LastEmbedInfo.m_iTriggerSource =
        //             pData->m_int = pProp->LastValue.iInt =
        //             (int) (pProp->LastValue.i64Int = pData->m_int64) ;
        //           _tcscpy_s( pProp->LastValue.szAsString ,
        //             m_Camera.TriggerSource.ToString().c_str() ) ;
        //           _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
        //           return true;
        //         }
        return false ;
      }
    case EXPOSURE_MODE:
      {
        if ( m_Camera.ExposureMode.IsValid() )
        {
          m_Camera.ExposureMode.SetIntValue( pData->m_int64 ) ;
          m_LastEmbedInfo.m_iTriggerSource =
            pData->m_int = pProp->LastValue.iInt =
            (int) (pProp->LastValue.i64Int = pData->m_int64) ;
          _tcscpy_s( pProp->LastValue.szAsString ,
            m_Camera.ExposureMode.ToString().c_str() ) ;
          _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
          TRACE( " Basler::BaslerSetCameraProperty: ExposureMode is %d(%s) \n" ,
            (int) pData->m_int64 , pData->m_szString ) ;
          return true;
        }
        return false ;
      }
    case TRIGGER_DELAY:
      {
        if ( m_Camera.TriggerDelay.IsValid() )
        {
          m_Camera.TriggerDelay.SetValue( pData->m_double ) ;
          m_LastEmbedInfo.m_iTriggerDelay = ROUND( pData->m_double ) ;
        }
        else if ( m_Camera.TriggerDelayAbs.IsValid() )
        {
          m_Camera.TriggerDelayAbs.SetValue( pData->m_double ) ;
          m_LastEmbedInfo.m_iTriggerDelay = ROUND( pData->m_double ) ;
        }
        else
          return false ;
        TRACE( " Basler::BaslerSetCameraProperty: Trigger Delay %.1f usec \n" , Data.m_double );
        return true;
      }
    case SHUTTER_AUTO:
      {
        CEnumParameter ExpAuto( nodemap , "ExposureAuto" );
        ExpAuto.SetValue( pData->m_bAuto ?
          "Continuous" : "Off" ) ;
        strcpy_s(pProp->LastValue.szAsString , ExpAuto.GetValue().c_str() ) ;
        pProp->LastValue.iInt = (int) (ExpAuto.GetIntValue()) ;
        //         ExpAuto.SetIntValue( pData->m_bAuto ?
        //           ExposureAuto_Continuous : ExposureAuto_Off ) ;
        return true ;
      }
    case GAIN_AUTO:
      {
        CEnumParameter GainAuto( nodemap , "GainAuto" );
        GainAuto.SetValue( pData->m_bAuto ?
          "Continuous" : "Off" ) ;
        strcpy_s( pProp->LastValue.szAsString , GainAuto.GetValue().c_str() ) ;
        pProp->LastValue.iInt = (int) (GainAuto.GetIntValue()) ;

        //         GainAuto.SetIntValue( pData->m_bAuto ?
        //             GainAuto_Continuous : GainAuto_Off ) ;
        return true ;
      }
      return true ;
    case SHUTTER:
      {
        if (m_Camera.ExposureAuto.IsValid())
        {
          ExposureAutoEnums Value = m_Camera.ExposureAuto.GetValue() ;
          bool bWasExpAuto = ( Value == ExposureAuto_Continuous ) ;
          if ( pData->m_bAuto )
          {
            m_Camera.ExposureAuto.SetValue( ExposureAuto_Continuous ) ;
            return true ;
          }
          else
          {
            if ( bWasExpAuto )
              m_Camera.ExposureAuto.SetValue( ExposureAuto_Off ) ; // switch off auto
          }
          // Take last exposure for view
          if ( bWasExpAuto )
          {
            if (m_Camera.ExposureTime.IsValid())
            {
              pData->m_int64 = pData->m_int = pProp->LastValue.iInt =
                ROUND( pData->m_double =
                  m_LastEmbedInfo.m_dExposure_us = m_Camera.ExposureTime.GetValue() ) ;
            }
            else if (m_Camera.ExposureTimeAbs.IsValid())
            {
              pData->m_int64 = pData->m_int = pProp->LastValue.iInt =
                ROUND( pData->m_double =
                  m_LastEmbedInfo.m_dExposure_us = m_Camera.ExposureTimeAbs.GetValue() );
            }
            else if (m_Camera.ExposureTimeRaw.IsValid())
            {
              pData->m_int64 = pData->m_int = pProp->LastValue.iInt =
                ROUND( pData->m_double =
                  m_LastEmbedInfo.m_dExposure_us = ( double ) m_Camera.ExposureTimeRaw.GetValue() ) ;
            }
//             if (m_SetupObject)
//             {
//               CGadgetStdSetup * pSetupDlg = ( CGadgetStdSetup* ) m_SetupObject ;
//               pSetupDlg->SetCellInt( "Shutter_us" , pData->m_int ) ;
//             }
          }
          else
            pData->m_int64 = pData->m_int = pProp->LastValue.iInt ;
        }
        else
          pData->m_bBool = pData->m_bAuto = false ;

        if ( !pData->m_bAuto )
        {
          if (m_Camera.ExposureTime.IsValid())
          {
            m_Camera.ExposureTime.SetValue( pData->m_double ) ;
            pData->m_int64 = pData->m_int = pProp->LastValue.iInt =
              ROUND( pData->m_double ) ;
          }
          else if (m_Camera.ExposureTimeAbs.IsValid())
          {
            m_Camera.ExposureTimeAbs.SetValue( pData->m_double ) ;
            pData->m_int64 = pData->m_int = pProp->LastValue.iInt =
              ROUND( pData->m_double );
          }
          else if (m_Camera.ExposureTimeRaw.IsValid())
          {
            m_Camera.ExposureTimeRaw.SetValue( ROUND(pData->m_double) ) ;
            pData->m_int64 = pData->m_int = pProp->LastValue.iInt =
              ROUND( pData->m_double );
          }
        }
        else
          pData->m_double = ( double ) (pData->m_int64 = pData->m_int = pProp->LastValue.iInt) ;
        return true ;
      }
    case GAIN:
    {
      if (m_Camera.GainAuto.IsValid())
      {
        GainAutoEnums Value = m_Camera.GainAuto.GetValue() ;
        bool bWasGainAuto = ( Value != GainAuto_Off ) ;
        if (pData->m_bAuto)
        {
          m_Camera.GainAuto.SetValue( GainAuto_Continuous ) ;
          return true ;
        }
        else if (bWasGainAuto)
        {
          m_Camera.GainAuto.SetValue( GainAuto_Off ) ;
          if (m_Camera.Gain.IsValid())
          {
            pData->m_int64 = pData->m_int = pProp->LastValue.iInt = ROUND( pData->m_double
              = ( m_LastEmbedInfo.m_dGain_dB = m_Camera.Gain.GetValue() ) * 10. ) ;
            if (m_SetupObject)
            {
              CGadgetStdSetup * pSetupDlg = ( CGadgetStdSetup* ) m_SetupObject ;
              pSetupDlg->SetCellInt( "Gain_dBx10" , pProp->LastValue.iInt ) ;
            }
          }
          else if (m_Camera.GainAbs.IsValid())
          {
            pData->m_int64 = pData->m_int = pProp->LastValue.iInt = ROUND( pData->m_double
              = ( m_LastEmbedInfo.m_dGain_dB = m_Camera.GainAbs.GetValue() ) * 10. ) ;
          }
          else if (m_Camera.GainRaw.IsValid())
          {
            pData->m_int64 = pData->m_int = pProp->LastValue.iInt = ROUND( pData->m_double
              = ( m_LastEmbedInfo.m_dGain_dB = ( double ) m_Camera.GainRaw.GetValue() ) ) ;
          }
          else
            m_LastEmbedInfo.m_dGain_dB = ( double ) ( pData->m_int64 = pData->m_int = pProp->LastValue.iInt ) ;
        }
      }
      else
        pData->m_bBool = pData->m_bAuto = false ;

      if (!pData->m_bAuto )
      {
        if (m_Camera.Gain.IsValid())
        {
          m_Camera.Gain.SetValue( pData->m_double / 10. ) ;
          pData->m_int64 = pData->m_int = pProp->LastValue.iInt =
            ROUND( m_LastEmbedInfo.m_dGain_dB = pData->m_double ) ;
          if (m_SetupObject)
          {
            CGadgetStdSetup * pSetupDlg = ( CGadgetStdSetup* ) m_SetupObject ;
            pSetupDlg->SetCellInt( "Gain_dBx10" , pProp->LastValue.iInt ) ;
          }
        }
        else if (m_Camera.GainAbs.IsValid())
        {
          m_Camera.GainAbs.SetValue( pData->m_double / 10. ) ;
          pData->m_int64 = pData->m_int = pProp->LastValue.iInt =
            ROUND( m_LastEmbedInfo.m_dGain_dB = pData->m_double ) ;
        }
        else if (m_Camera.GainRaw.IsValid())
        {
          m_Camera.GainRaw.SetValue( ROUND( pData->m_double ) ) ;
          pData->m_int64 = pData->m_int = pProp->LastValue.iInt =
            ROUND( m_LastEmbedInfo.m_dGain_dB = pData->m_double ) ;
        }
      }
    }
      return true ;
    case BASLER_PACKETSIZE:
      {
        if ( m_Camera.GevSCPSPacketSize.IsValid() )
        {
          pProp->LastValue.iInt = pData->m_int = (int) (pProp->LastValue.i64Int 
            = pData->m_int64 = m_Camera.GevSCPSPacketSize.GetValue()) ;
          return true ;
        }
        return false ;
      }
      break ;
    case BASLER_INTER_PACKET_DELAY:
      {
        if ( m_Camera.GevSCPD.IsValid() )
        {
          pProp->LastValue.iInt = pData->m_int = (int) (pProp->LastValue.i64Int
            = pData->m_int64 = m_Camera.GevSCPD.GetValue()) ;
          return true ;
        }
        return false ;
      }
      break ;
    case BRIGHTNESS:
      {
        if ( m_Camera.BlackLevel.IsValid() )
        {
          m_Camera.BlackLevel.SetValue( pData->m_double / 10. ) ;
          pProp->LastValue.dDouble = pData->m_double ;
        }
        else if ( m_Camera.BlackLevelAbs.IsValid() )
        {
          m_Camera.BlackLevelAbs.SetValue( pData->m_double / 10. ) ;
          pProp->LastValue.dDouble = pData->m_double ;
        }
        else if ( m_Camera.BlackLevelRaw.IsValid() )
        {
          m_Camera.BlackLevelRaw.SetValue( pData->m_int ) ;
          pProp->LastValue.iInt = pData->m_int ;
        }
        else
          return false ;
        return true ;
      }
    case GAMMA:
      {
        if ( m_Camera.Gamma.IsValid() )
        {
          m_Camera.Gamma.SetValue( pData->m_double ) ;
          pProp->LastValue.dDouble = pData->m_double ;
          return true;
        }
        return false ;
      }
    case FRAME_RATE:
      {
        if ( m_Camera.AcquisitionFrameRateEnable.IsValid() )
          m_Camera.AcquisitionFrameRateEnable.SetValue( true ) ;
        if ( m_Camera.AcquisitionFrameRate.IsValid() )
          m_Camera.AcquisitionFrameRate.SetValue( pData->m_double ) ;
        else if ( m_Camera.AcquisitionFrameRateAbs.IsValid() )
          m_Camera.AcquisitionFrameRateAbs.SetValue( pData->m_double ) ;
        else
          return false ;
        pProp->LastValue.dDouble = pData->m_double ;
        m_fLastRealFrameRate = (float) pData->m_double ;
        return true ;
      }
    case FRAME_RATE_ENABLE:
      {
        if ( m_Camera.AcquisitionFrameRateEnable.IsValid() )
        {
          m_Camera.AcquisitionFrameRateEnable.SetValue( pData->m_bBool ) ;
          pProp->LastValue.bBool = pData->m_bBool ;
          return true ;
        }
        return false ;
      }
    case USER_SET_SELECTOR:
      {  // this property is working only in STOP state 
        if ( m_Camera.UserSetSelector.IsValid() && m_Camera.UserSetSelector.IsWritable() )
        {
          CEnumerationPtr Param( pNode ) ;
          Param->SetIntValue( pData->m_int64 ) ;
          strcpy_s( pData->m_szString , pProp->m_EnumeratorsAsTexts[ pData->m_int ].c_str() );
          TRACE( " Basler::BaslerSetCameraProperty: %s is %s(%d) \n" ,
            _T( "User Set Selector" ) , pData->m_szString ,
            pProp->LastValue.iInt = (int) (pProp->LastValue.i64Int = Param->GetIntValue()) ) ;
          strcpy_s( pProp->LastValue.szAsString , pData->m_szString ) ;
          return true;
        }
        return false ;
      }
    case LINE_SELECTOR:
      {  // this property is working only in STOP state 
        if ( m_Camera.LineSelector.IsValid() && m_Camera.LineSelector.IsWritable() )
        {
          CEnumerationPtr Param( pNode ) ;
          Param->SetIntValue( pData->m_int64 ) ;
          pProp->LastValue.iInt = (int) (pProp->LastValue.i64Int = Param->GetIntValue()) ;
          strcpy_s( pData->m_szString , Param->ToString().c_str() );
          TRACE( " Basler::BaslerSetCameraProperty: %s is %s(%d) \n" ,
            _T( "Line Selector" ) , pData->m_szString , pProp->LastValue.iInt ) ;
          pData->m_bInvalidate = true ;
          strcpy_s( pProp->LastValue.szAsString , pData->m_szString ) ;
          return true;
        }
        return false ;
      }

    case LINE_MODE:
      {
        CEnumerationPtr Param( pNode ) ;
        Param->SetIntValue( pProp->m_EnumeratorsAsValues[ pData->m_int ] ) ;
        TRACE( " Basler::BaslerSetCameraProperty: Line mode is %s(%d) \n" ,
          pProp->m_EnumeratorsAsTexts[ pData->m_int ].c_str() , pData->m_int ) ;
        strcpy_s( pProp->LastValue.szAsString , pProp->m_EnumeratorsAsTexts[ pData->m_int ].c_str() ) ;

        pData->m_bInvalidate = true;
        return true;
      }
    case LINE_SOURCE:
      {
        CEnumerationPtr Param( pNode ) ;
        //        Param->SetIntValue( pProp->m_EnumeratorsAsValues[ pData->m_int ] ) ;
        Param->SetIntValue( pData->m_int ) ;
        pData->m_bInvalidate = true ;
        int iEnumIndex = GetIndexForEnumProp( pProp , pData->m_int ) ;
#ifdef _DEBUG

        TRACE( " Basler::BaslerSetCameraProperty: Line src is %s(%d) \n" ,
          pProp->m_EnumeratorsAsTexts[ iEnumIndex ].c_str() , pData->m_int ) ;
#endif
        strcpy_s( pProp->LastValue.szAsString , pProp->m_EnumeratorsAsTexts[ iEnumIndex ].c_str() ) ;
        return true;
      }
    case LINE_INVERTER:
      {
        switch ( pProp->m_DataType )
        {
        case PDTBool:
          {
            CBooleanPtr Param( pNode ) ;
            Param->SetValue( pData->m_bBool ) ;
            TRACE( "Basler::BaslerSetCameraProperty: LineInverter is %d. " ,
              Data.m_bBool ? 1 : 0 ) ;
            pProp->LastValue.bBool = pData->m_bBool ;
          }
          break ;
        case PDTEnum:
          {
            CEnumerationPtr Param( pNode ) ;
            Param->SetIntValue( pProp->m_EnumeratorsAsValues[ pData->m_int ] ) ;
            int iEnumIndex = GetIndexForEnumProp( pProp , pData->m_int ) ;
            TRACE( "Basler::BaslerSetCameraProperty: LineInverter is %s. " ,
              pProp->m_EnumeratorsAsTexts[ iEnumIndex ].c_str() ) ;
            strcpy_s( pProp->LastValue.szAsString , pProp->m_EnumeratorsAsTexts[ iEnumIndex ].c_str() ) ;
            pProp->LastValue.iInt = pData->m_int ;
          }
          break ;
        default:
          ASSERT( 0 ) ;
          return false;
        }
        return true;
      }
    case SAVE_SETTINGS:
      {
        // Necessary to set user set to 1 and default set to user set 1
        CCommandPtr Command( pNode ) ;
        Command->Execute() ;
      }
      return true ;
    case TRIGGER_SOFT_FIRE:
      {
        if ( (m_Camera.TriggerMode.GetValue() == TriggerMode_On)
          && (m_Camera.TriggerSource.GetValue() == TriggerSource_Software) )
        {
          m_Camera.ExecuteSoftwareTrigger() ;
        }
     }
      return true ;
    case TIMER_SELECT:
      {
        CEnumerationPtr Param( pNode ) ;
        //        Param->SetIntValue( pProp->m_EnumeratorsAsValues[ pData->m_int ] ) ;
        Param->SetIntValue( pData->m_int ) ;
        pData->m_bInvalidate = true ;
#ifdef _DEBUG
        int iEnumIndex = GetIndexForEnumProp( pProp , pData->m_int ) ;

        TRACE( " Basler::BaslerSetCameraProperty: Timer select is %s(%d) \n" ,
          pProp->m_EnumeratorsAsTexts[ iEnumIndex ].c_str() , pData->m_int ) ;
#endif
        strcpy_s( pProp->LastValue.szAsString , pProp->m_EnumeratorsAsTexts[ pData->m_int ].c_str() ) ;
        pProp->LastValue.iInt = pData->m_int ;
        return true;
      }
      break;
    case TIMER_TRIGGER_SOURCE:
      {
        CEnumParameter Param( pNode ) ;
        int iEnumIndex = -1 ;

        if ( pData->m_szString[ 0 ] )
        {
          Param.SetValue( pData->m_szString ) ;
          iEnumIndex = GetIndexForEnumProp( pProp , pData->m_szString ) ;
        }
        else
        {
          Param.SetIntValue( pData->m_int ) ;
          iEnumIndex = GetIndexForEnumProp( pProp , pData->m_int ) ;
        }
#ifdef _DEBUG
        TRACE( " Basler::BaslerSetCameraProperty: Timer trigger source is %s(%d) \n" ,
          Param.GetValue().c_str() , (int) Param.GetIntValue() ) ;
#endif
        strcpy_s( pProp->LastValue.szAsString , pProp->m_EnumeratorsAsTexts[ iEnumIndex ].c_str() ) ;
        pProp->LastValue.iInt = pData->m_int ;
        return true;
      }
      break;
    case TIMER_TRIGGER_POLARITY:
      {
        CEnumerationPtr Param( pNode ) ;
        //        Param->SetIntValue( pProp->m_EnumeratorsAsValues[ pData->m_int ] ) ;
        Param->SetIntValue( pData->m_int ) ;
#ifdef _DEBUG
        int iEnumIndex = GetIndexForEnumProp( pProp , pData->m_int ) ;

        TRACE( " Basler::BaslerSetCameraProperty: Timer trigger source is %s(%d) \n" ,
          pProp->m_EnumeratorsAsTexts[ iEnumIndex ].c_str() , pData->m_int ) ;
#endif
        strcpy_s( pProp->LastValue.szAsString , pProp->m_EnumeratorsAsTexts[ pData->m_int ].c_str() ) ;
        pProp->LastValue.iInt = pData->m_int ;
        return true;
      }
      break;
    case TIMER_DELAY:
      {
        if ( m_Camera.TimerDelay.IsValid() )
        {
          m_Camera.TimerDelay.SetValue( pData->m_double ) ;
          pProp->LastValue.dDouble = pData->m_double ;
        }
        else if ( m_Camera.TimerDelayAbs.IsValid() )
        {
          m_Camera.TimerDelayAbs.SetValue( pData->m_double ) ;
          pProp->LastValue.dDouble = pData->m_double ;
        }
        else if ( m_Camera.TimerDelayRaw.IsValid() )
        {
          m_Camera.TimerDelayRaw.SetValue( pData->m_int ) ;
          pProp->LastValue.iInt = pData->m_int ;
        }
        else
          return false ;
        TRACE( " Basler::BaslerSetCameraProperty: Timer Delay set to %.1f \n" ,
          Data.m_double );
        return true ;
      }
      return true ;
    case TIMER_DURATION:
      {
        if ( m_Camera.TimerDuration.IsValid() )
        {
          m_Camera.TimerDuration.SetValue( pData->m_double ) ;
          pProp->LastValue.dDouble = pData->m_double ;
        }
        else if ( m_Camera.TimerDurationAbs.IsValid() )
        {
          m_Camera.TimerDurationAbs.SetValue( pData->m_double ) ;
          pProp->LastValue.dDouble = pData->m_double ;
        }
        else if ( m_Camera.TimerDurationRaw.IsValid() )
        {
          m_Camera.TimerDurationRaw.SetValue( pData->m_int ) ;
          pProp->LastValue.iInt = pData->m_int ;
        }
        else
          return false ;
        TRACE( " Basler::BaslerSetCameraProperty: Timer Duration set to %.1f \n" ,
          Data.m_double );
        return true ;
      }
      return true ;
    case OUTPUT_SELECTOR:
      {
        if ( m_Camera.UserOutputSelector.IsValid() )
        {
          m_Camera.UserOutputSelector.SetIntValue( pData->m_int64 ) ;
          INode * pNode = m_Camera.UserOutputSelector.GetNode() ;
          int  iDirect = (int) m_Camera.UserOutputSelector.GetIntValue() ;
          FXString AsText = m_Camera.UserOutputSelector.ToString().c_str() ;
          _tcscpy_s( pProp->LastValue.szAsString , (LPCTSTR)AsText ) ;
          _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
          return true;
        }
        return false ;
      }
      break;
    case OUTPUT_VALUE:
      {
        if ( m_Camera.UserOutputValue.IsValid() )
        {
          m_Camera.UserOutputValue.SetValue( pData->m_bBool ) ;
          pProp->LastValue.bBool = pData->m_bBool ;
          return true ;
        }
        return false ;
      }
      break;

    case EVENT_SELECTOR:
      {
        CEnumerationPtr Param( pNode ) ;
        Param->SetIntValue( pData->m_int ) ;
        pData->m_bInvalidate = true ;
        int iEnumIndex = GetIndexForEnumProp( pProp , pData->m_int ) ;
        _tcscpy_s( pProp->LastValue.szAsString , pProp->m_EnumeratorsAsTexts[ iEnumIndex ].c_str() ) ;
#ifdef _DEBUG

        TRACE( " Basler::BaslerSetCameraProperty: Event Select is %s(%d) \n" ,
          pProp->m_EnumeratorsAsTexts[ iEnumIndex ].c_str() , pData->m_int ) ;
#endif
        return true;
      }
      break ;
    case EVENT_ENABLE:
      {
        if ( m_Camera.EventNotification.IsValid() )
        {
          m_Camera.EventNotification.SetIntValue( pData->m_int ) ;
          _tcscpy_s( pProp->LastValue.szAsString ,
            m_Camera.EventNotification.ToString().c_str() ) ;
          _tcscpy_s( pData->m_szString , pProp->LastValue.szAsString ) ;
#ifdef _DEBUG
          TRACE( " Basler::BaslerSetCameraProperty: Event Enable is %s(%d) \n" ,
            pProp->LastValue.szAsString , pData->m_int ) ;
#endif
          return true;
        }
        return false ;
      }
    }
  }
  catch ( const GenericException &e )
  {
    // Error handling
    FXString CatchData ;
    CatchData.Format( _T( "SetProp: Exception for Node %s (Type %s): %s\n" ) ,
      (LPCTSTR) pProp->name , GetTypeName( pProp->m_DataType ) , e.GetDescription() ) ;
    SEND_GADGET_ERR( (LPCTSTR) CatchData );
  }
  return false;
}

bool Basler::SetCameraProperty( int iIndex , INT_PTR &value , bool& bauto , bool& Invalidate )
{
  bool bRes = SetCameraPropertyEx( iIndex , value , bauto , Invalidate );

  return bRes;

}


bool Basler::SetCameraProperty( FXCamPropertyType Type , INT_PTR &value , bool& bauto , bool& Invalidate )
{
  int iIndex = GetThisCamPropertyIndex( Type );
  if ( iIndex >= 0 )
  {
    if ( Type == RESTART_TIMEOUT )
    {
      m_iRestartTimeOut_ms = (int) value;
      return true;
    }
    return SetCameraProperty( iIndex , value , bauto , Invalidate );
  }
  return false;
}

bool Basler::SetCameraPropertyEx( int iIndex , INT_PTR &value , bool& bauto , bool& Invalidate )
{
  m_PropertyDataSend.Reset();
  m_PropertyDataSend.m_bAuto = bauto;
  m_PropertyListLock.Lock( INFINITE , "Basler::SetCameraPropertyEx" );
  BaslerCamProperty& Prop = m_ThisCamProperty.GetAt( iIndex ) ;
  PropertyDataType Type = Prop.m_DataType;
  m_PropertyListLock.Unlock();
  TRACE( "\nBasler::SetCameraPropertyEx %s " , m_ThisCamProperty[ iIndex ].name );
  switch ( Type )
  {
  case PDTInt:
    if (value < Prop.LastValue.dMin)
      value = ROUND( Prop.LastValue.dMin ) ;
    if ( Prop.LastValue.dMax < value )
      value = ROUND( Prop.LastValue.dMax ) ;
  case PDTEnum:
    m_PropertyDataSend.m_int = (int) (m_PropertyDataSend.m_int64 = value);
    m_PropertyDataSend.m_double = ( double ) value ;
    if ( m_ThisCamProperty[ iIndex ].id != FORMAT_MODE )
      TRACE( "%d auto=%d\n" , value , (int) bauto );
    else
    {

      string Name = GetPixelFormatName( (DWORD)value ) ;
      TRACE( "%s(0x%08X) \n" , (!Name.empty()) ? Name.c_str() : "Unknown" , (DWORD)value );
      CheckAndStopCapture();
    }

    break;
  case PDTBool:
    m_PropertyDataSend.m_bBool = (value != 0);
    m_PropertyDataSend.m_int = (int)(m_PropertyDataSend.m_int64 = value) ;
    TRACE( "%s \n" , (value != 0) ? "true" : "false" );
    break;
  case PDTString:
    strcpy_s( m_PropertyDataSend.m_szString , (LPCTSTR) value );
    TRACE( "%s\n" , (LPCTSTR) value );
    break;
  case PDTFloat:
    m_PropertyDataSend.m_double = atof( (LPCTSTR) value );
    if (m_PropertyDataSend.m_double < Prop.LastValue.dMin)
      m_PropertyDataSend.m_double = ROUND( Prop.LastValue.dMin ) ;
    if (Prop.LastValue.dMax < m_PropertyDataSend.m_double)
      m_PropertyDataSend.m_double = ROUND( Prop.LastValue.dMax ) ;
    TRACE( "%s auto=%d\n" , (LPCTSTR) value , (int) bauto );
    break;
  }

  switch ( Prop.id )
  {
  case SHUTTER:
  {
    if (m_Camera.ExposureAuto.IsValid())
    {
      ExposureAutoEnums Value = m_Camera.ExposureAuto.GetValue() ;
      bool bWasExpAuto = ( Value == ExposureAuto_Continuous ) ;
      if (bauto)
      {
        m_Camera.ExposureAuto.SetValue( ExposureAuto_Continuous ) ;
        return true ;
      }
      else
      {
        if (bWasExpAuto)
        {
          m_Camera.ExposureAuto.SetValue( ExposureAuto_Off ) ; // switch off auto
        }
      }
      // Take last exposure for view
      if (bWasExpAuto )
      {
        if (m_Camera.ExposureTime.IsValid())
        {
          m_Camera.ExposureTime.SetValue( m_PropertyDataSend.m_double ) ;
          Prop.LastValue.i64Int = Prop.LastValue.iInt =
            ROUND( Prop.LastValue.dDouble = m_LastEmbedInfo.m_dExposure_us 
              = m_PropertyDataSend.m_double ) ;
        }
        else if (m_Camera.ExposureTimeAbs.IsValid())
        {
          m_Camera.ExposureTimeAbs.SetValue( m_PropertyDataSend.m_double ) ;
          Prop.LastValue.i64Int = Prop.LastValue.iInt =
            ROUND( Prop.LastValue.dDouble = m_LastEmbedInfo.m_dExposure_us
              = m_PropertyDataSend.m_double ) ;
        }
        else if (m_Camera.ExposureTimeRaw.IsValid())
        {
          m_Camera.ExposureTimeRaw.SetValue( m_PropertyDataSend.m_int ) ;
         Prop.LastValue.dDouble = m_LastEmbedInfo.m_dExposure_us = 
           (double)(Prop.LastValue.i64Int = Prop.LastValue.iInt = m_PropertyDataSend.m_int) ;
        }
        if (m_SetupObject)
        {
          CGadgetStdSetup * pSetupDlg = ( CGadgetStdSetup* ) m_SetupObject ;
          pSetupDlg->SetCellInt( "Shutter_us" , Prop.LastValue.iInt ) ;
        }
      }
    }

    if (!bauto )
    {
      if (m_Camera.ExposureTime.IsValid())
      {
        m_Camera.ExposureTime.SetValue( m_PropertyDataSend.m_double ) ;
        Prop.LastValue.i64Int = Prop.LastValue.iInt =
          ROUND(  m_LastEmbedInfo.m_dExposure_us = Prop.LastValue.dDouble 
            = m_PropertyDataSend.m_double ) ;
      }
      else if (m_Camera.ExposureTimeAbs.IsValid())
      {
        m_Camera.ExposureTimeAbs.SetValue( m_PropertyDataSend.m_double ) ;
        Prop.LastValue.i64Int = Prop.LastValue.iInt =
          ROUND( m_LastEmbedInfo.m_dExposure_us = Prop.LastValue.dDouble 
            = m_PropertyDataSend.m_double ) ;
      }
      else if (m_Camera.ExposureTimeRaw.IsValid())
      {
        m_Camera.ExposureTimeRaw.SetValue( m_PropertyDataSend.m_int ) ;
        Prop.LastValue.dDouble = m_PropertyDataSend.m_double =
          m_LastEmbedInfo.m_dExposure_us = 
          ( double ) (Prop.LastValue.i64Int = Prop.LastValue.iInt ) ;
      }
    }
    return true ;
  }
  case GAIN:
  {
    if (m_Camera.GainAuto.IsValid())
    {
      GainAutoEnums Value = m_Camera.GainAuto.GetValue() ;
      bool bWasGainAuto = ( Value != GainAuto_Off ) ;
      if (bauto)
      {
        m_Camera.GainAuto.SetValue( GainAuto_Continuous ) ;
        return true ;
      }
      else if (bWasGainAuto)
      {
        m_Camera.GainAuto.SetValue( GainAuto_Off ) ;
        if (m_Camera.Gain.IsValid())
        {
          Prop.LastValue.i64Int = Prop.LastValue.iInt =
            ROUND( Prop.LastValue.dDouble = m_LastEmbedInfo.m_dGain_dB = m_Camera.Gain.GetValue()  * 10. ) ;
          if ( m_SetupObject )
          {
            CGadgetStdSetup * pSetupDlg = ( CGadgetStdSetup* ) m_SetupObject ;
            pSetupDlg->SetCellInt( "Gain_dBx10" , Prop.LastValue.iInt ) ;
          }
        }
        else if (m_Camera.GainAbs.IsValid())
        {
          Prop.LastValue.i64Int = Prop.LastValue.iInt =
            ROUND( Prop.LastValue.dDouble = m_LastEmbedInfo.m_dGain_dB 
              = m_Camera.GainAbs.GetValue()  * 10. ) ;
        }
        else if (m_Camera.GainRaw.IsValid())
        {
          Prop.LastValue.i64Int = Prop.LastValue.iInt =
            ROUND( Prop.LastValue.dDouble = m_LastEmbedInfo.m_dGain_dB
              = (double)m_Camera.GainRaw.GetValue()  ) ;
        }
      }
    }

    if (!bauto )
    {
      if (m_Camera.Gain.IsValid())
      {
        m_Camera.Gain.SetValue( m_PropertyDataSend.m_double / 10. ) ;
        Prop.LastValue.i64Int = Prop.LastValue.iInt =
          ROUND( m_LastEmbedInfo.m_dGain_dB = Prop.LastValue.dDouble = m_PropertyDataSend.m_double ) ;
        if (m_SetupObject)
        {
          CGadgetStdSetup * pSetupDlg = ( CGadgetStdSetup* ) m_SetupObject ;
          pSetupDlg->SetCellInt( "Gain_dBx10" , Prop.LastValue.iInt ) ;
        }
      }
      else if (m_Camera.GainAbs.IsValid())
      {
        m_Camera.GainAbs.SetValue( m_PropertyDataSend.m_double / 10. ) ;
        Prop.LastValue.i64Int = Prop.LastValue.iInt =
          ROUND( m_LastEmbedInfo.m_dGain_dB = Prop.LastValue.dDouble = m_PropertyDataSend.m_double ) ;
      }
      else if (m_Camera.GainRaw.IsValid())
      {
        m_Camera.GainRaw.SetValue( ROUND(m_PropertyDataSend.m_double) ) ;
        Prop.LastValue.i64Int = Prop.LastValue.iInt =
          ROUND( m_LastEmbedInfo.m_dGain_dB = Prop.LastValue.dDouble = m_PropertyDataSend.m_double ) ;
      }
    }
  }
  return true ;

  }


  bool bRes = OtherThreadSetProperty( iIndex , &m_PropertyDataSend , &Invalidate );
  if ( bRes )
  {
    Invalidate = m_PropertyDataSend.m_bInvalidate;
    return true;
  }
  return false;
}

bool Basler::SetCameraPropertyEx( FXCamPropertyType Type , INT_PTR &value , bool& bauto , bool& Invalidate )
{
  int iIndex = GetThisCamPropertyIndex( Type );
  if ( iIndex >= 0 )
    return SetCameraPropertyEx( iIndex , value , bauto , Invalidate );
  return false;
}

bool Basler::BaslerCheckAndStopCapture()
{
  bool bWasRunning = IsRunning();
  if ( bWasRunning )
  {
    BaslerCameraStop();
    m_BusEvents &= ~(BASLER_EVT_STOP_GRAB);
    Sleep( 10 );
    m_bWasStopped = true;
  }
  return bWasRunning;
}

bool Basler::BaslerInitAndConditialStart()
{
  if ( BaslerCamInit() )
  {
    if ( m_bWasStopped )
      return BaslerCameraStart();
    return true;
  }
  else
    return false;
}


void Basler::GetROI( CRect& rc )
{
  rc = m_CurrentROI;
}

void Basler::SetROI( CRect& rc )
{
  if ( !m_Camera.IsOpen() )
    return;

  if ( (m_CurrentROI.right == rc.right)
    && (m_CurrentROI.bottom == rc.bottom)
    && (rc.left >= 0)
    && (rc.top >= 0)
    && (rc.left + rc.right <= m_SensorSize.cx)
    && (rc.top + rc.bottom <= m_SensorSize.cy)
    )
  {
    rc.left &= ~0x3; // step 4
    rc.top &= ~0x3;

    // simply do origin shift
    // by writing into IMAGE_ORIGIN register
    // without grab stop
    SetXOffset( rc.left );
    SetYOffset( rc.top );
    m_CurrentROI = rc;
    return;
  }

  BaslerCheckAndStopCapture();

  bool supported = false;

  if ( rc.left >= 0 && rc.top >= 0
    && rc.right >= 4 && rc.bottom >= 4 )
  {
    rc.left &= ~0x3; // step 4
    rc.top &= ~0x3;
    rc.right &= ~0x3; // step 4
    rc.bottom &= ~0x3;
    if ( rc.left < 0 || rc.left > m_SensorSize.cx )
      rc.left = 0;
    if ( rc.left + rc.right > m_SensorSize.cx )
      rc.left = m_SensorSize.cx - rc.right;
    if ( rc.top < 0 || rc.top >m_SensorSize.cy )
      rc.top = 0;
    if ( rc.top + rc.bottom > m_SensorSize.cy )
      rc.top = m_SensorSize.cy - rc.bottom;
  }
  else
  {
    rc.left = rc.top = 0;
    rc.right = m_SensorSize.cx & ~0x03; // Step 4
    rc.bottom = m_SensorSize.cy & ~0x03; // Step 4
  }
  try
  {
    SetXOffset( rc.left );
    SetYOffset( rc.top );
    SetIntValue( _T( "Width" ) , rc.right );
    SetIntValue( _T( "Height" ) , rc.bottom );

    m_LastEmbedInfo.m_ROI = rc ;
  }
  catch ( const GenericException &e )
  {
    LPCTSTR pDescr = e.GetDescription();
    SENDERR( "Set ROI Error %s" , pDescr );
  }

  m_CurrentROI = rc;

}




unsigned int Basler::GetBppFromPixelFormat( EPixelType pixelFormat )
{
  switch ( pixelFormat )
  {
  case PixelType_Mono8:
    return 8;
    break;
    //case PIXEL_FORMAT_411YUV8:
  case PixelType_Mono12:
    // case PIXEL_FORMAT_RAW12:
    return 12;
    break;
  case PixelType_Mono16:
    //case PIXEL_FORMAT_S_MONO16:
    //case PIXEL_FORMAT_422YUV8:
    //case PIXEL_FORMAT_RAW16:
    return 16;
    break;
    //case PIXEL_FORMAT_444YUV8:
    //case PIXEL_FORMAT_RGB:
    //case PIXEL_FORMAT_BGR:
    //  return 24;
    //  break;
    //case PIXEL_FORMAT_BGRU:
    //case PIXEL_FORMAT_RGBU:
    //  return 32;
    //  break;
    //case PIXEL_FORMAT_S_RGB16:
    //case PIXEL_FORMAT_RGB16:
    //case PIXEL_FORMAT_BGR16:
    //  return 48;
    //  break;
  default:
    return 0;
    break;
  }
}

//bool Basler::CheckCameraPower( CInstantCamera * pCam )
//{
//  if ( !pCam )
//    return false ;
//
//  unsigned int powerReg;
//  unsigned int PowerInqReg;
//
//  // Make sure camera supports power control
//  Error error = pCam->ReadRegister( 0x400 , &PowerInqReg );
//
//  // Only proceed if there was no error and power control was supported
//  if ( ( error == PGRERROR_OK ) && ( ( PowerInqReg & 0x00008000 ) != 0 ) )
//  {
//    error = pCam->ReadRegister( 0x610 , &powerReg );
//    if ( error == PGRERROR_OK )
//    {
//      powerReg = powerReg >> 31;
//      return ( powerReg != 0 ) ;
//    }
//  }
//  return false ;
//}

bool Basler::IsGrabThreadRunning()
{
  return m_continueGrabThread;
}
//
//bool Basler::EnableEmbeddedTimeStamp( CInstantCamera* cam )
//{
//  if ( cam != NULL && cam->IsConnected() )
//  {
//    Error error = cam->GetEmbeddedImageInfo( &m_embeddedInfo );
//    if ( error != PGRERROR_OK )
//    {
//      return false;
//    }
//
//    if ( m_embeddedInfo.timestamp.available && !m_embeddedInfo.timestamp.onOff )
//    {
//      m_embeddedInfo.timestamp.onOff = true;
//      error = cam->SetEmbeddedImageInfo( &m_embeddedInfo );
//      if ( error != PGRERROR_OK )
//      {
//        return false;
//      }
//      else
//      {
//        return true;
//      }
//    }
//    else
//    {
//      return false;
//    }
//  }
//  else
//  {
//    return false;
//  }
//}
//
//bool Basler::DisableEmbeddedTimeStamp( CInstantCamera* cam )
//{
//  if ( cam != NULL && cam->IsConnected() )
//  {
//    m_embeddedInfo.timestamp.onOff = false;
//    Error error = cam->SetEmbeddedImageInfo( &m_embeddedInfo );
//    if ( error != PGRERROR_OK )
//    {
//      return false;
//    }
//    else
//    {
//      return true;
//    }
//  }
//  else
//  {
//    return false;
//  }
//}
//
DWORD WINAPI Basler::ControlLoop( LPVOID pParam )
{
  TRACE( "---------Entry to ControlLoop\n" );
  Basler * pGadget = (Basler*) pParam;
  FXString csMessage;
  BOOL isCorruptFrame = FALSE;
  unsigned int cols = 0 , rows = 0 , colsPrev = 0 , rowsPrev = 0;
  DWORD dwWaitRes = 0;
  pGadget->m_continueGrabThread = true;
  pGadget->RegisterCallbacks();
  // Start of main grab loop
  while ( pGadget->m_continueGrabThread )
  {
    dwWaitRes = WaitForMultipleObjects( 2 ,
      pGadget->m_WaitEventBusChangeArr , FALSE ,
      (!(pGadget->m_bInitialized)) ? 500 : 1000 );
    if ( dwWaitRes == WAIT_FAILED )  // gadget deleted
    {
      DWORD dwError = GetLastError();
      break;
    }
    if ( dwWaitRes == WAIT_TIMEOUT && pGadget->m_bInitialized
      && pGadget->m_iRestartTimeOut_ms > 1000 && !pGadget->m_bTriedToRestart )
    {
      FXAutolock al( pGadget->m_GrabLock );
      double dTimeFromLastFrame = GetHRTickCount() - pGadget->m_dLastFrameTime;
      if ( dTimeFromLastFrame > pGadget->m_iRestartTimeOut_ms
        && (pGadget->m_TriggerMode < TriggerOn) )
      {
        pGadget->m_BusEvents |= BASLER_EVT_INIT;
        pGadget->m_bTriedToRestart = true;
      }
    }
    if ( dwWaitRes == WAIT_OBJECT_0 || pGadget->m_bTriedToRestart ) // some bus or command event message
    {
      DWORD InitialBusEvents = pGadget->m_BusEvents;
      int iNInits = 0;
      while ( pGadget->m_BusEvents )
      {
        if ( iNInits > 5 )
        {
          pGadget->m_BusEvents = 0;
          break;
        }
        if ( pGadget->m_sSerialNumber.IsEmpty() )
        {
          if ( !(pGadget->m_BusEvents & BASLER_EVT_SHUTDOWN) )
          {
            pGadget->m_BusEvents = 0;
            break;
          }
        }
        if ( !pGadget->m_bRun )
          pGadget->m_BusEvents &= ~(BASLER_EVT_START_GRAB | BASLER_EVT_LOCAL_START);
        //         FXAutolock al( pGadget->m_LocalConfigLock , "ControlLoop") ;
        if ( pGadget->m_BusEvents & BASLER_EVT_SHUTDOWN )
        {
          if ( pGadget->m_Camera.IsOpen() )
          {
            pGadget->BaslerCameraStop();
            pGadget->BaslerDeleteCam();
          }
          pGadget->m_BusEvents = 0;
          pGadget->m_continueGrabThread = false;
          TRACE( "Camera #%s Shut downed\n" , (LPCTSTR) (pGadget->m_sSerialNumber) );
          Sleep( 20 );
          break;
        }
        if ( pGadget->m_BusEvents & BASLER_EVT_RESTART )
        {
          pGadget->m_BusEvents &= ~(BASLER_EVT_RESTART);
          pGadget->BaslerCamClose();
          TRACE( "Camera #%s closed in ControlLoop\n" , (LPCTSTR) (pGadget->m_sSerialNumber) );
          pGadget->m_BusEvents |= BASLER_EVT_INIT;
          break;
        }
        if ( pGadget->m_BusEvents & (BUS_EVT_REMOVED | BUS_EVT_BUS_RESET) )
        {
          if ( pGadget->m_BusEvents & BUS_EVT_BUS_RESET )
          {
            pGadget->m_BusEvents |= BASLER_EVT_INIT;
            Sleep( 10 );
            TRACE( "Camera #%s Bus Reset\n" , (LPCTSTR) (pGadget->m_sSerialNumber) );
          }
          if ( pGadget->m_BusEvents & BUS_EVT_REMOVED )
          {
            pGadget->BaslerCamClose();
            TRACE( "Camera #%s Removed\n" , (LPCTSTR) (pGadget->m_sSerialNumber) );
          }

          pGadget->m_BusEvents &= ~((BUS_EVT_REMOVED | BUS_EVT_BUS_RESET));
        }
        if ( (pGadget->m_BusEvents & BASLER_EVT_START_GRAB)
          && !pGadget->m_bInitialized )
        {
          pGadget->m_BusEvents |= BASLER_EVT_INIT;
        }
        if ( pGadget->m_BusEvents & BASLER_EVT_INIT )
        {
          if ( !pGadget->m_sSerialNumber.IsEmpty() )
          {
            pGadget->BaslerCamInit();
            if ( pGadget->m_bInitialized  &&  pGadget->m_bRun )
            {
              pGadget->m_BusEvents |= BASLER_EVT_START_GRAB;
              Sleep( 50 );
              pGadget->m_BusEvents &= ~(BASLER_EVT_BUILD_PROP);
            }
            iNInits++;
          }
          pGadget->m_BusEvents &= ~(BASLER_EVT_INIT);
        }
        else if ( pGadget->m_BusEvents &   BASLER_EVT_BUILD_PROP )
        {
          pGadget->BaslerBuildPropertyList();
          pGadget->m_BusEvents &= ~(BASLER_EVT_BUILD_PROP);
        }
        if ( pGadget->m_BusEvents & BASLER_EVT_SET_PROP )
        {
          pGadget->BaslerSetCameraProperty(
            pGadget->m_iPropertyIndex , &pGadget->m_PropertyData );
          pGadget->m_BusEvents &= ~(BASLER_EVT_SET_PROP);
        }
        if ( pGadget->m_BusEvents & BASLER_EVT_GET_PROP )
        {
          pGadget->BaslerGetCameraProperty( pGadget->m_iPropertyIndex ,
            &pGadget->m_PropertyData );
          pGadget->m_BusEvents &= ~(BASLER_EVT_GET_PROP);
        }

        if ( pGadget->m_BusEvents & BASLER_EVT_START_GRAB )
        {
          if ( !pGadget->BaslerCameraStart() )
          {
            if ( !pGadget->m_sSerialNumber.IsEmpty() )
              pGadget->m_BusEvents = BASLER_EVT_INIT | BASLER_EVT_START_GRAB;
          }
          pGadget->m_BusEvents &= ~(BASLER_EVT_START_GRAB);
        }
        if ( pGadget->m_BusEvents & BASLER_EVT_STOP_GRAB )
        {
          pGadget->BaslerCameraStop();
          pGadget->m_BusEvents &= ~(BASLER_EVT_STOP_GRAB);
        }
        if ( pGadget->m_BusEvents & BASLER_EVT_RELEASE )
        {
          pGadget->BaslerCamClose();
          pGadget->m_CurrentDevice = -1;
          pGadget->m_BusEvents &= ~(BASLER_EVT_RELEASE);
          break;
        }
        if ( pGadget->m_BusEvents & BASLER_EVT_LOCAL_STOP )
        {
          pGadget->BaslerLocalStreamStop();
          pGadget->m_BusEvents &= ~(BASLER_EVT_LOCAL_STOP);
          break;
        }
        if ( pGadget->m_BusEvents & BASLER_EVT_LOCAL_START )
        {
          pGadget->BaslerLocalStreamStart();
          pGadget->m_BusEvents &= ~(BASLER_EVT_LOCAL_START);
          break;
        }
        if ( pGadget->m_BusEvents & BUS_EVT_ARRIVED )
        {
          pGadget->BaslerCamInit();
          if ( pGadget->m_bInitialized  &&  pGadget->m_bRun )
          {
            pGadget->m_BusEvents |= BASLER_EVT_START_GRAB;
            Sleep( 50 );
            pGadget->m_BusEvents &= ~(BASLER_EVT_BUILD_PROP);
            FxSendLogMsg( MSG_INFO_LEVEL ,
              pGadget->GetDriverInfo() , 0 ,
              " Camera #%s is initialized after connection\n" ,
              (LPCTSTR) (pGadget->m_sSerialNumber) );
          }
          else if ( pGadget->m_sSerialNumber != _T("-1") )
          {
            FxSendLogMsg( MSG_ERROR_LEVEL ,
              pGadget->GetDriverInfo() , 0 ,
              " Camera #%s is NOT initialized after connection\n" ,
              ( LPCTSTR ) ( pGadget->m_sSerialNumber ) );
          }


          pGadget->m_BusEvents &= ~(BUS_EVT_ARRIVED);
        }
      }
      if ( InitialBusEvents )
        SetEvent( pGadget->m_evControlRequestFinished );
    }

    if ( pGadget->m_bRun  && pGadget->m_continueGrabThread )
    {
      //       int iRes = pGadget->ReceiveImage(); // will read new image and put pointer to pGadget->m_pNewImage
      //       switch ( iRes ) // no camera
      //       {
      //       case -1 :  // no camera
      //         pGadget->m_BusEvents |= BASLER_EVT_INIT ;
      //       case PGRERROR_ISOCH_NOT_STARTED:
      //         pGadget->m_BusEvents |= BASLER_EVT_START_GRAB ;
      //         SetEvent( pGadget->m_evBusChange ) ;
      //         break ;
      //       }
    }
  }
  pGadget->m_continueGrabThread = false;
  pGadget->UnregisterCallbacks();

  if ( pGadget->m_Camera.IsOpen() )
    pGadget->BaslerCamClose();
  pGadget->m_ControlThreadName.Empty();
  pGadget->m_dwGrabThreadId = NULL;
  TRACE( "---PGR Normal Exit from ControlLoop for #%s\n" , (LPCTSTR) (pGadget->m_sSerialNumber) );
  return 0;
}

void Basler::LogError( LPCTSTR Msg )
{
  FXString GadgetName;
  GetGadgetName( GadgetName );
  FxSendLogMsg( MSG_ERROR_LEVEL , GetDriverInfo() , 0 ,
    _T( "%s - %s" ) , (LPCTSTR) GadgetName , Msg );
}

void Basler::LocalStreamStart()
{
  CamCNTLDoAndWait( BASLER_EVT_START_GRAB );
  m_bLocalStopped = false;
}
void Basler::LocalStreamStop()
{
  CamCNTLDoAndWait( BASLER_EVT_STOP_GRAB );
  m_bLocalStopped = true;
}


void
Basler::RegisterCallbacks()
{}

void Basler::UnregisterCallbacks()
{}

bool Basler::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  double dStart = GetHRTickCount();
  FXAutolock al( m_ScanPropLock );
  m_bInScanProperties = true;
  m_bShouldBeReprogrammed = false;
  bool bInternalInvalidate = false ;
  if ( !m_continueGrabThread )
  {
    FXString ThreadName;
    m_hGrabThreadHandle = CreateThread( NULL , 0 ,
      ControlLoop , this , CREATE_SUSPENDED , &m_dwGrabThreadId );
    if ( m_hGrabThreadHandle )
    {
      ResumeThread( m_hGrabThreadHandle );
      Sleep( 50 );
      m_continueGrabThread = true;
    }
    else
    {
      DEVICESENDERR_2( "%s: %s" , (LPCTSTR) ThreadName , _T( "Can't start thread" ) );
      m_bInScanProperties = false;
      return false;
    }
  }

  CSetupObject * pSetupObject = GetSetupObject() ;

  FXString tmpS;
  FXPropKit2 pc( text );
  FXString sSerNum;
  bool bSerNumExists = pc.GetString( "Camera" , sSerNum ) ;
  if ( pc.GetString( "CameraName" , m_sCameraGivenName ) )
  {
    if ( !m_sCameraGivenName.IsEmpty() )
      m_bSelectByGivenName = true ;
  }
  if ( m_bSelectByGivenName )
  {
    FXAutolock al( m_ConfigLock ) ;
    for ( int i = 0 ; i < m_iCamNum ; i++ )
    {
      if ( sSerNum == m_CamInfo[ i ].szSerNum )
      {
        if ( _tcslen( m_CamInfo[ i ].m_sGivenName ) > 0 )
        {
          m_sCameraGivenName = m_CamInfo[ i ].m_sGivenName ;
          break ;
        }
      }
    }
  }

  if ( !m_bInitialOK && bSerNumExists )
  {
//     if ( pc.GetLength() > 20 )
//     {
      m_InitialProperties = text ;
//      m_bInitialOK = true ;
//    }
  }
  unsigned camSN = 0;
  m_bWasStopped = false;
  if ( bSerNumExists )
  {
    FXString GadgetName ;
    BOOL bNameExists = GetGadgetName( GadgetName ) ;
    if ( IsSNLegal( sSerNum ) )
    {
      if ( (m_sSerialNumber != sSerNum) )
      {
        int newCamnmb = SerialToNmb( sSerNum );;
        if ( (newCamnmb < m_iCamNum) && (newCamnmb >= 0) )
        {
          if ( IsSNLegal( m_sSerialNumber ) )
          {
            m_bWasStopped = IsRunning();
            bool bRes = CamCNTLDoAndWait( BASLER_EVT_RELEASE , 2000 );
            m_sConnectedSerialNumber.Empty();
            ASSERT( !m_Camera.IsOpen() );
            m_bCameraExists = true ;
          }
          m_sSerialNumber = sSerNum ;
          m_CurrentDevice = newCamnmb;
          CameraInit();
          m_bShouldBeReprogrammed = m_bWasStopped;
          if ( m_bInitialOK )
          {
            m_InitialProperties.WriteString( _T( "Camera" ) , m_sSerialNumber ) ;
            if ( pSetupObject )
              pSetupObject->SetSavedProperties( m_InitialProperties ) ;
            if ( m_Camera.IsOpen() )
              pc = (LPCTSTR) m_InitialProperties ;
          }
          else
          {
            bool bCamConnected = CheckAndAllocCamera();
            if ( !bCamConnected )
            {
              m_dwSerialNumber = 0;
              m_dwConnectedSerialNumber = 0;
              m_dwInitializedSerialNumber = 0;
              m_bInScanProperties = false;
              return false;
            }
          }
        }
        else
        {
          m_bCameraExists = false ;
          SENDERR( "Assigned camera %s doesn't exists" , sSerNum ) ;
        }
      }
      else
      {
        m_CurrentDevice = -1;
        m_bCameraExists = false ;
      }
      m_sSerialNumber = sSerNum;
    }
    else
    {
      m_bSelectByGivenName = ( sSerNum == _T( "-2" ) );
      if ( IsSNLegal( m_sSerialNumber ) )
      {
        bool bRes = CamCNTLDoAndWait( BASLER_EVT_RELEASE , 2000 );
        m_sSerialNumber = m_bSelectByGivenName ? _T( "-2" ) : _T( "-1" ) ;
        m_dwConnectedSerialNumber = 0;
        m_dwInitializedSerialNumber = 0;
      }
      m_pixelFormat = PixelFormat_Mono8;
      m_bIsRunning = false;
      //       ASSERT( bRes ) ;
    }
    Invalidate = bInternalInvalidate = true; //update setup
  }
  if ( ( !bSerNumExists || (m_CurrentDevice >= 0) || m_dwConnectedSerialNumber )
    && DriverValid() && m_Camera.IsOpen() )
  {
    if ( pc.GetInt( "StreamState" , m_bLocalStopped )
      && IsRunning() )
    {
      if ( m_bLocalStopped )
        LocalStreamStop();
      else
        LocalStreamStart();
    }
    INodeMap * pNodeMap = GetNodeMapWithCatching() ;
    if (!pNodeMap)
      return false ;

    INodeMap& nodemap = *pNodeMap;
    int iNScanned = 0;
    BaslerCamProperty * pTimerSelect = GetThisCamProperty( TIMER_SELECT ) ;
    if ( pTimerSelect )
    {
//       LPCTSTR psSelector = (LPCTSTR) (pTimerSelect->m_InDeviceName) ;
//       CEnumParameter TSelector( nodemap , psSelector );

//       StringList_t AllTimersEnums ;
//       TSelector.GetAllValues( AllTimersEnums ) ;

      size_t i = 0 ;
      for ( ; i < pTimerSelect->m_EnumeratorsAsTexts.size() ; i++ )
      {
        LPCTSTR pTimerName = pTimerSelect->m_EnumeratorsAsTexts[ i ].c_str() ;
        ScanTimersProperties( pTimerName , pc ) ;
      }
    }


    BaslerCamProperty * pLineSelect = GetThisCamProperty( LINE_SELECTOR ) ;
    if ( pLineSelect  )
    {
      LPCTSTR psSelector = (LPCTSTR) (GetThisCamProperty( LINE_SELECTOR )->m_InDeviceName) ;
      CEnumParameter LSelector( nodemap , psSelector );
      StringList_t AllLinesEnums ;
      LSelector.GetAllValues( AllLinesEnums ) ;
      size_t i = 0 ;
      for ( ; i < AllLinesEnums.size() ; i++ )
      {
        LPCTSTR pLineName = AllLinesEnums[ i ].c_str() ;
        if ( AllLinesEnums[ i ] != "Line1" )
          ScanLineProperties( pLineName , pc ) ;
      }
    }

    for ( int i = 0; i < m_ThisCamProperty.GetSize(); i++ )
    {
      if ( !m_bInitialScanDone && Invalidate )
      {
        FXString Settings ;
        ScanSettings( Settings ) ;
        Invalidate = false ;
      }
      m_PropertyListLock.Lock( INFINITE , "Basler::ScanProperties" );
      BaslerCamProperty Prop = m_ThisCamProperty[ i ];
      m_PropertyListLock.Unlock();
      bool bProcessed = false ;
      FXString Val ;
      bool bPropExists = pc.GetString( Prop.name , Val , FXString() ) && !Val.IsEmpty() ;
      if ( !bPropExists )
        continue ;
      Val = Val.Trim() ;
      switch ( Prop.id )
      {
        case GAIN_AUTO:
        {
          bool bAuto = ( Val.Find( "auto" ) > 0 ) ;
          if ( m_Camera.GainAuto.IsValid() && m_Camera.GainAuto.IsWritable() )
          {
            m_Camera.GainAuto.SetValue(
              bAuto ? GainAuto_Continuous : GainAuto_Off ) ;
            Prop.m_bAuto = bAuto ;
            strcpy_s( Prop.LastValue.szAsString , m_Camera.GainAuto.ToString().c_str() ) ;
            bProcessed = true ;
          }
        }
        break ;
        case GAIN:
        {
          bool bAuto = ( Val.Find( "auto" ) >= 0 ) ;
          int iGain_dBx10 = bAuto ? Prop.LastValue.iInt : atoi( ( LPCTSTR ) Val ) ;
          SetGain_dBx10( &Prop , iGain_dBx10 , bAuto ) ;
//           if ( m_SetupObject )
//           {
//             CGadgetStdSetup * pSetupDlg = ( CGadgetStdSetup* ) m_SetupObject ;
//             pSetupDlg->SetCellInt( "Shutter_us" , iExposure_us ) ;
//           }
          bProcessed = true ;
        }
        break ;
        case SHUTTER_AUTO:
        {
          bool bAuto = ( Val.Find( "auto" ) > 0 ) ;
          if ( m_Camera.ExposureAuto.IsValid() && m_Camera.ExposureAuto.IsWritable() )
          {
            m_Camera.ExposureAuto.SetValue(
              bAuto ? ExposureAuto_Continuous : ExposureAuto_Off ) ;
            Prop.m_bAuto = bAuto ;
            strcpy_s( Prop.LastValue.szAsString , m_Camera.ExposureAuto.ToString().c_str() ) ;
            bProcessed = true ;
          }
        }
        break ;
        case SHUTTER:
        {
          bool bAuto = ( Val.Find( "auto" ) >= 0 ) ;
          int iExposure_us = bAuto ? Prop.LastValue.iInt : atoi( ( LPCTSTR ) Val ) ;
          SetShutter_us( &Prop , iExposure_us , bAuto ) ;
//           if ( m_SetupObject )
//           {
//             CGadgetStdSetup * pSetupDlg = ( CGadgetStdSetup* ) m_SetupObject ;
//             pSetupDlg->SetCellInt( "Shutter_us" , iExposure_us ) ;
//           }
          bProcessed = true ;
        }
        break ;
        case EVENT_SELECTOR:
      case EVENT_ENABLE:
      case EVENTS_STATUS:
        if ( ScanCameraEventsProperties( pc ) )
          bInternalInvalidate = true;
        bProcessed = true ;
        break ;
//       case TRIGGER_SELECTOR:
//       case TRIGGER_SOURCE:
//       case TRIGGER_ACTIVATE:
//         {
//           if ( m_Camera.TriggerSelector.IsValid() && m_Camera.TriggerSelector.IsWritable() )
//           {
//             if ( isdigit( Val[ 0 ] ) )
//             {
//               int iSelector = atoi( Val ) ;
//               m_Camera.TriggerSelector.SetIntValue( iSelector ) ;
//               Prop.LastValue.i64Int = Prop.LastValue.iInt = iSelector ;
//               if ( (DWORD) iSelector < Prop.m_EnumeratorsAsTexts.size() )
//                 _tcscpy( Prop.LastValue.szAsString , Prop.m_EnumeratorsAsTexts[ iSelector ].c_str() ) ;
//             }
//             else
//             {
//               m_Camera.TriggerSelector.FromString( (LPCTSTR) Val ) ;
//               _tcscpy( Prop.LastValue.szAsString , Val ) ;
//               for ( size_t i = 0 ; i < Prop.m_EnumeratorsAsTexts.size() ; i++ )
//               {
//                 if ( Prop.m_EnumeratorsAsTexts[ i ] == (LPCTSTR) Val )
//                 {
//                   Prop.LastValue.i64Int = Prop.LastValue.iInt = (int) i ;
//                   break ;
//                 }
//               }
//             }
//           }
//           bProcessed = true ;
//         }
//         break ;
      }
      if ( bProcessed )
        continue ;

      FXString key;
      FXParser param;
      Prop.m_GUIFormat.GetElementNo( 0 , key , param );
      INT_PTR value;
      int iVal ;
      bool bauto = false;
      FXString AsString;
      if ( (key == SETUP_COMBOBOX) || (key == SETUP_SPIN) ) // ints result
      {
        switch ( Prop.m_DataType )
        {
        case PDTEnum:
        case PDTInt:
        case PDTBool:
          if ( pc.GetString( Prop.name , AsString ) )
          {
            AsString.Trim() ;
            if ( isdigit( AsString[ 0 ] ) || (AsString[ 0 ] == '+') || (AsString[ 0 ] == '-') )
            {
              if ( pc.GetInt( Prop.name , iVal ) )
              {
                iNScanned++ ;
                value = iVal ;
                if ( !SetCameraProperty( Prop.id , value , bauto , Invalidate ) )
                {
                  DEVICESENDERR_1( "Can't set property %s" , Prop.name );
                }
                bInternalInvalidate |= Invalidate ;
              }
            }
            else if ( Prop.m_DataType == PDTBool )
            {
              if ( AsString == "true" )
                value = 1 ;
              else if ( AsString == "false" )
                value = 0 ;
              else
                break ;
              iNScanned++ ;
              if ( !SetCameraProperty( Prop.id , value , bauto , Invalidate ) )
              {
                SEND_DEVICE_ERR( "Can't set property %s to %s" ,
                  Prop.name , (LPCTSTR)AsString );
              }
              bInternalInvalidate |= Invalidate ;
            }
            else if ( Prop.m_DataType == PDTEnum ) // case of combo with parameter as text
            {
              if ( Prop.id == TRIGGER_MODE )
              {
                if (AsString.MakeLower() == "toggle")
                {
                  AsString = ( Prop.LastValue.iInt == TriggerMode_Off ) ? "On" : "Off" ;
                }
              }
              for (size_t iIndex = 0 ;
                iIndex < Prop.m_EnumeratorsAsTexts.size() ; iIndex++ )
              {
                if ( AsString.CompareNoCase( Prop.m_EnumeratorsAsTexts[ iIndex ].c_str() ) == 0 )
                {
                  iNScanned++ ;
                  value = (int)Prop.m_EnumeratorsAsValues[ iIndex ] ;
                  if (!SetCameraProperty( Prop.id , value , bauto , Invalidate ))
                  {
                    SENDERR( "Can't set property %s to %s(%d)" , Prop.name , 
                      (LPCTSTR)AsString , iIndex );
                  }
                  bInternalInvalidate |= Invalidate ;
                }
              }
            }
          }
          break;
        case PDTFloat:
          if ( pc.GetString( Prop.name , AsString ) )
          {
            iNScanned++ ;
            value = (INT_PTR) ((LPCTSTR) AsString);
            if ( !SetCameraProperty( Prop.id , value , bauto , Invalidate ) )
            {
              DEVICESENDERR_1( "Can't set property %s" , Prop.name );
            }
            bInternalInvalidate |= Invalidate ;
          }
          break;
        case PDTCommand:
          if ( pc.GetString( Prop.name , AsString ) )
          {
            iNScanned++ ;
            value = 1 ;
            if ( !SetCameraProperty( Prop.id , value , bauto , Invalidate ) )
            {
              DEVICESENDERR_1( "Can't set property %s" , Prop.name );
            }
            bInternalInvalidate |= Invalidate ;
          }
          break;
        }
      }
      else if ( key == SETUP_SPINABOOL )
      {
        if ( pc.GetString( Prop.name , AsString ) )
        {
          iNScanned++ ;
          AsString = AsString.MakeLower();
          bauto = (AsString.Find( "auto" ) >= 0);
          if ( bauto )
          {
            GetCameraProperty( Prop.id , value , bauto );
            bauto = true;
            //value = (int)_T("Unknown") ;
          }
          else
          {
            switch ( Prop.m_DataType )
            {
            case PDTInt: value = atoi( (LPCTSTR) AsString ); break;
            case PDTFloat:
            case PDTString: value = (INT_PTR) ((LPCTSTR) AsString); break;
            }
          }
          if ( !SetCameraProperty( Prop.id , value , bauto , Invalidate ) )
          {
            DEVICESENDERR_1( "Can't set property %s" , Prop.name );
          }
          bInternalInvalidate |= Invalidate ;
        }
      }
      else if ( key == SETUP_EDITBOX )
      {
        FXString svalue;
        INT_PTR value;
        bool bauto = false;
        if ( pc.GetString( Prop.name , svalue ) )
        {
          iNScanned++ ;
          value = (INT_PTR) ((LPCTSTR) svalue);
          if ( !SetCameraProperty( Prop.id , value , bauto , Invalidate ) )
          {
            DEVICESENDERR_2( "Can't set prop %s to %s" ,
              Prop.name , svalue );
          }
          bInternalInvalidate |= Invalidate ;
        }
      }
      else if ( !IsNotViwed( Prop.id ) && !Prop.m_bDontShow )
      {
        DEVICESENDERR_1( "Unsupported key '%s'in scanproperty" , key );
      }
    }
  }
  m_bInitialScanDone = true ;
  if ( m_bShouldBeReprogrammed || m_bWasStopped )
  {
    if ( DeviceStart() )
      m_bShouldBeReprogrammed = m_bWasStopped = false;
    //     else
    //       CamCNTLDoAndWait( BASLER_EVT_INIT , 2000 ) ;
  }
  m_bInScanProperties = false;
  FXString GadgetName;
  GetGadgetName( GadgetName );
  double dBusyTime = GetHRTickCount() - dStart;
  TRACE( "Basler::ScanProperties %s: Start %g , Busy %g\n" , (LPCTSTR) GadgetName ,
    dStart , dBusyTime );
  Invalidate = bInternalInvalidate ;

  return true;
}

// void Basler::OnBusArrival( void* pParam , unsigned int serialNumber )
// {
//   Basler* pGadget = static_cast<Basler*>(pParam);
//   if ( pGadget->m_dwSerialNumber == serialNumber )
//   {
//     bool bRes = pGadget->CamCNTLDoAndWait( BUS_EVT_ARRIVED , 2000 );
//     FxSendLogMsg( MSG_WARNING_LEVEL ,
//       pGadget->GetDriverInfo() , 0 ,
//       " Bus arrival of Camera #%u; Init=%s\n" ,
//       serialNumber , (bRes) ? _T( "OK" ) : _T( "FAULT" ) );
//   }
//   //   else
//   //     FxSendLogMsg(MSG_WARNING_LEVEL , 
//   //     pGadget->GetDriverInfo() , 0 , 
//   //     "  Bus arrival of Camera #%u (Gadget cam SN is %u)\n" , serialNumber , pGadget->m_dwSerialNumber ) ;
// 
//   TRACE( "Bus Arrival for Camera #%u\n " , pGadget->m_dwSerialNumber );
//   pGadget->m_bRescanCameras = true;
//   pGadget->m_dwNArrivedEvents++;
// }
// 
// void Basler::OnBusRemoval( void* pParam , unsigned int serialNumber )
// {
//   Basler* pGadget = static_cast<Basler*>(pParam);
//   if ( serialNumber == pGadget->m_DevicesInfo[ pGadget->m_CurrentDevice ].serialnmb )
//   {
//     FxSendLogMsg( MSG_WARNING_LEVEL ,
//       pGadget->GetDriverInfo() , 0 , "Camera %u disconnected" , serialNumber );
//     TRACE( "Camera #%u Removed\n" , pGadget->m_dwSerialNumber );
//     pGadget->CamCNTLDoAndWait( BUS_EVT_REMOVED , 2000 );
//   }
//   //  SENDINFO_2( "  Bus removal of Camera #%u (Gadget cam SN is %u)\n" , serialNumber , pGadget->m_dwSerialNumber /*, pGadget->m_Bus*/) ;
// 
//   TRACE( "  Bus removal of Camera #%u (Gadget cam SN is %u)\n" , serialNumber , pGadget->m_dwSerialNumber /*, pGadget->m_Bus*/ );
//   pGadget->m_bCamerasEnumerated = false;
//   pGadget->m_bRescanCameras = true;
// }


// void Basler::OnBusReset( void* pParam , unsigned int serialNumber )
// {
//   Basler* pGadget = static_cast<Basler*>(pParam);
//   pGadget->m_bCamerasEnumerated = false;
//   //   bool bRes = pGadget->CamCNTLDoAndWait( BUS_EVT_BUS_RESET | BASLER_EVT_INIT , 2000)  ;
//   pGadget->m_bRescanCameras = true;
//   //  SENDINFO_1( "  Bus Reset for Camera #%u \n" , pGadget->m_dwSerialNumber /*, pGadget->m_Bus*/) ;
//   TRACE( "  Bus Reset for Camera #%u \n" , pGadget->m_dwSerialNumber /*, pGadget->m_Bus*/ );
//   //   ASSERT( bRes ) ;
// }



bool Basler::CheckAndAllocCamera( void )
{
  if ( !m_Camera.IsOpen() )
  {
    if ( m_sSerialNumber.IsEmpty() )
      return false;
    if ( !CameraInit() )
      return false;
  }
  return true;
}


bool Basler::SetBMIH( void )
{
  m_BMIH.biSize = sizeof( BITMAPINFOHEADER );
  m_BMIH.biWidth = m_CurrentROI.Width();
  m_BMIH.biHeight = m_CurrentROI.Height();

  m_BMIH.biPlanes = 1;
  switch ( m_pixelFormat )
  {
  case PixelFormat_Mono8:
    m_BMIH.biCompression = BI_Y8;
    m_BMIH.biBitCount = 8;
    m_BMIH.biSizeImage = m_BMIH.biWidth*m_BMIH.biHeight;
    break;
  case PixelFormat_YUV411Packed:
    m_BMIH.biCompression = BI_YUV411;
    m_BMIH.biBitCount = 12;
    m_BMIH.biSizeImage = 3 * m_BMIH.biWidth*m_BMIH.biHeight / 2;
    break;
  case PixelFormat_YUV422_YUYV_Packed:
  case PixelFormat_YUV422Packed:
    m_BMIH.biCompression = BI_YUV422;
    m_BMIH.biBitCount = 16;
    m_BMIH.biSizeImage = 2 * m_BMIH.biWidth*m_BMIH.biHeight ;
    break;
  case PixelFormat_Mono16:
  case PixelFormat_Mono12:
    m_BMIH.biCompression = BI_Y16;
    m_BMIH.biBitCount = 16;
    m_BMIH.biSizeImage = 2 * m_BMIH.biWidth*m_BMIH.biHeight;
    break;


  default:
    m_BMIH.biSize = 0;
    TRACE( "!!! Unsupported format #%d\n" , m_pixelFormat );
    DEVICESENDERR_1( "!!! Unsupported format #%d" , m_pixelFormat );
    return false;
  }
  m_RealBMIH = m_BMIH ;
  return true;
}


bool Basler::ScanSettings( FXString& text )
{
  EnumCameras();
  bool bAllocated = CheckAndAllocCamera();
  // Prepare cameras list
  FXString camlist( "SelectBySN(-1),SelectByName(-2)" ) , paramlist , tmpS;
  int iCurrentCamera = -1;
  for ( int i = 0; i < m_iCamNum; i++ )
  {
    TCHAR cMark = _T( '+' ); // sign, that camera is free
    if ( m_sSerialNumber.IsEmpty() || m_sSerialNumber != m_CamInfo[ i ].szSerNum )
    {
      for ( int j = 0; j < m_BusyCameras.GetCount(); j++ )
      {
        if ( m_sSerialNumber != m_BusyCameras[ j ] )
        {
          if ( m_BusyCameras[ j ] == m_CamInfo[ i ].szSerNum )
          {
            cMark = _T( '-' ); // sign, that camera is busy by other gadget
            break;
          }
        }
      }
    }
    else
    {
      cMark = _T( '!' );// this gadget camera sign
      iCurrentCamera = m_CurrentDevice = i;
    }
    if ( m_bSelectByGivenName )
    {
      tmpS.Format( "%c%s:%s(%d)" , cMark , m_CamInfo[ i ].m_sGivenName ,
        m_CamInfo[ i ].szSerNum , atoi( m_CamInfo[ i ].szSerNum ) );
    }
    else
    {
      tmpS.Format( "%c%s:%s:%s(%d)" , cMark , m_CamInfo[ i ].szSerNum ,
        m_CamInfo[ i ].model , m_CamInfo[ i ].m_sGivenName , 
        atoi( m_CamInfo[ i ].szSerNum ) );
    }
    if ( !camlist.IsEmpty() )
      camlist += _T( ',' ) ;
    camlist += tmpS;
  }
  if ( iCurrentCamera < 0 && !m_sSerialNumber.IsEmpty() )
  {
    if ( !camlist.IsEmpty() )
      camlist += _T( ',' );
    tmpS.Format( "?%s" , m_sSerialNumber );
    camlist += tmpS;
    iCurrentCamera = m_iCamNum; // first after real cameras
  }
  tmpS.Format( "ComboBox(Camera(%s))" , camlist );
  paramlist += tmpS;
  if ( m_iCamNum && bAllocated && m_Camera.IsOpen() )
  {
    m_sConnectedSerialNumber = m_sSerialNumber;
    if ( !m_sSerialNumber.IsEmpty() )
    {
      paramlist += _T( ",ComboBox(StreamState(Run(0),Idle(1)))," );
      int cnt = 0;
      do
      {
        bool bRes = CamCNTLDoAndWait( BASLER_EVT_BUILD_PROP , 5000 );
        if ( !bRes )
          cnt = 5;
      } while ( cnt++ < 5 && (m_ThisCamProperty.GetCount() == 0) ) ;

      if ( cnt < 5 )
      {
        FXAutolock al( m_PropertyListLock , "Basler::ScanSettings" );
        for ( int i = 0; i < m_ThisCamProperty.GetSize(); i++ )
        {
          if ( !IsNotViwed( m_ThisCamProperty[ i ].id ) )
          {
            paramlist += (FXString) m_ThisCamProperty[ i ].m_GUIFormat;
            if ( i < m_ThisCamProperty.GetUpperBound() )
              paramlist += _T( ',' );
          }
        }
      }
      else
      {
        DEVICESENDERR_0( "Can't build property list" );
        m_BusEvents = 0;
      }
    }
  }
  text.Format( "template(%s)" , paramlist );
  return true;
}

int Basler::PrintCameraEventsProperties( FXPropertyKit& pc )
{
  return 0 ;
  FXPropertyKit CEP_pk ;
  BaslerCamProperty * pSelector = GetThisCamProperty( EVENT_SELECTOR ) ;
  BaslerCamProperty * pEnabled = GetThisCamProperty( EVENT_ENABLE ) ;
  BaslerCamProperty * pEventsStatus = GetThisCamProperty( EVENTS_STATUS );

  FXString CurrentEnum ;
  try
  {
    if ( m_Camera.EventSelector.IsValid() && m_Camera.EventNotification.IsValid() )
    {
      EventSelectorEnums SavedSelector = m_Camera.EventSelector.GetValue();
      EventNotificationEnums SavedEnabled = m_Camera.EventNotification.GetValue();
      size_t i = 0 ;
      for ( ; i < pSelector->m_EnumeratorsAsTexts.size() ; i++ )
      {
        CurrentEnum = pSelector->m_EnumeratorsAsTexts[ i ].c_str() ;
        EventSelectorEnums Result =
          ConvertNameToEnum( CurrentEnum );
        if ( (int) Result >= 0 )
        {
          m_Camera.EventSelector.SetValue( Result );
          EventNotificationEnums EvNotVal = m_Camera.EventNotification.GetValue();
          CEP_pk.WriteInt( pSelector->m_EnumeratorsAsTexts[ i ].c_str() , (int) EvNotVal );
        }
      }

      m_Camera.EventSelector.SetValue( SavedSelector );
      CEP_pk.Insert( 0 , _T( '(' ) ) ;
      pc.WriteString( pEventsStatus->name , CEP_pk += _T( ')' ) , false ) ;
      pc.WriteInt( pSelector->name , SavedSelector );
      pc.WriteInt( pEnabled->name , SavedEnabled );
      return (int) pSelector->m_EnumeratorsAsValues.size() ;
    }
  }
  catch ( const GenericException &e )
  {
    // Error handling
    FXString CatchData ;
    CatchData.Format( _T( "PrintCameraEventsProperties - %sException catch for Node %s: %s\n" ) ,
      (LPCTSTR) m_Tabs , (LPCTSTR) CurrentEnum , e.GetDescription() ) ;
    if ( m_bLoggingON )
    {
      SEND_GADGET_ERR( ( LPCTSTR ) CatchData );
    }
    TRACE( "\n   %s" , (LPCTSTR) CatchData ) ;
  }
  return 0 ;
}

int Basler::PrintCameraTimersProperties( FXPropertyKit& pc , bool bOnlyComplexIOsFill )
{
  if ( !GetThisCamProperty( TIMER_SELECT )
    || !GetThisCamProperty( TIMER_TRIGGER_SOURCE )
    || !GetThisCamProperty( TIMER_DURATION )
    || !GetThisCamProperty( TIMER_DELAY ))
    return 0 ;
  LPCTSTR psSelector = (LPCTSTR) (GetThisCamProperty( TIMER_SELECT )->m_InDeviceName) ;
  FXString psTTriggerSrc = (LPCTSTR) (GetThisCamProperty( TIMER_TRIGGER_SOURCE )->m_InDeviceName) ;
  FXString psTDuration = (LPCTSTR) (GetThisCamProperty( TIMER_DURATION )->m_InDeviceName);
  FXString psTDelay = (LPCTSTR) (GetThisCamProperty( TIMER_DELAY )->m_InDeviceName);
  if ( !psSelector || psTTriggerSrc.IsEmpty() || psTDelay.IsEmpty() || psTTriggerSrc.IsEmpty() )
    return 0 ;

  try
  {
    INodeMap * pNodeMap = GetNodeMapWithCatching() ;
    if (!pNodeMap)
      return false ;

    INodeMap& nodemap = *pNodeMap;

    CEnumParameter TSelector( nodemap , psSelector );

    String_t SavedSelected = TSelector.GetValue() ;

    StringList_t AllTriggersEnums ;
    TSelector.GetAllValues( AllTriggersEnums ) ;
    size_t i = 0 ;
    FXString CurrentEnum , sTriggerActivation ;
    if ( bOnlyComplexIOsFill )
      m_ComplexIOs.m_Timers.clear() ;
    for ( ; i < AllTriggersEnums.size() ; i++ )
    {
      FXPropertyKit CEP_pk ;
      FXString sTriggerSrc , sTriggerActivation , psTriggerActivation ;
      CurrentEnum = AllTriggersEnums[ i ].c_str() ;
      TSelector.SetValue( (LPCTSTR) CurrentEnum );
      if ( m_Camera.TimerTriggerSource.IsValid() && m_Camera.TimerTriggerSource.IsReadable() )
        sTriggerSrc = CEnumParameter( m_Camera.TimerTriggerSource.GetNode() ).GetValue().c_str() ;
      if ( m_Camera.TimerTriggerActivation.IsValid() && m_Camera.TimerTriggerActivation.IsReadable() )
      {
        psTriggerActivation = m_Camera.TimerTriggerActivation.GetNode()->GetName().c_str() ;
        sTriggerActivation = CEnumParameter( m_Camera.TimerTriggerActivation.GetNode() ).GetValue().c_str() ;
      }
      double dDuration = m_Camera.TimerDuration.GetValue();
      double dDelay = m_Camera.TimerDelay.GetValue();
      
      if ( ! bOnlyComplexIOsFill )
      {
        if ( !sTriggerSrc.IsEmpty() )
          CEP_pk.WriteString( psTTriggerSrc + '_' , sTriggerSrc );
        if ( !psTriggerActivation.IsEmpty() )
          CEP_pk.WriteString( psTriggerActivation , sTriggerActivation );
        CEP_pk.WriteInt( psTDuration + '_' , ROUND( dDuration ) ) ;
        CEP_pk.WriteInt( psTDelay + '_' , ROUND( dDelay ) ) ;
        CEP_pk.Insert( 0 , _T( "(" ) ) ;
        CEP_pk += ")" ;
        pc += '\n' ;
        pc.WriteString( CurrentEnum , CEP_pk , false ) ;
      }
      BaslerTimer NewTimer( CurrentEnum , sTriggerSrc , dDuration , dDelay , sTriggerActivation ) ;
      if ( bOnlyComplexIOsFill )
        m_ComplexIOs.m_Timers.push_back( NewTimer ) ;
      else
      {
        BaslerTimer * pFound = m_ComplexIOs.FindTimer( CurrentEnum ) ;
        if ( pFound )
          *pFound = NewTimer ;
        else
          m_ComplexIOs.m_Timers.push_back( NewTimer ) ;
      }
    }
    TSelector.SetValue( (LPCTSTR) SavedSelected ) ;
    return (int) AllTriggersEnums.size() ;
  }
  catch ( const GenericException &e )
  {
    // Error handling
    FXString CatchData ;
    CatchData.Format( _T( "PrintCameraTimersProperties - %sException catch for Print Timers: %s\n" ) ,
      (LPCTSTR) m_Tabs , e.GetDescription() ) ;
    if ( m_bLoggingON )
    {
      SEND_GADGET_ERR( ( LPCTSTR ) CatchData );
    }
    TRACE( "\n   %s" , (LPCTSTR) CatchData ) ;
  }
  return 0 ;
}

int Basler::PrintCameraLinesProperties( FXPropertyKit& pc , bool bOnlyComplexIOsFill )
{
  if ( !GetThisCamProperty( LINE_SELECTOR ) )
    return 0 ;
  LPCTSTR psSelector = (LPCTSTR) (GetThisCamProperty( LINE_SELECTOR )->m_InDeviceName) ;
  if ( !psSelector )
    return 0 ;

  try
  {
    INodeMap * pNodeMap = GetNodeMapWithCatching() ;
    if (!pNodeMap)
      return false ;

    INodeMap& nodemap = *pNodeMap;

    CEnumParameter LSelector( nodemap , psSelector );

    String_t SavedSelected = LSelector.GetValue() ;

    if ( bOnlyComplexIOsFill )
      m_ComplexIOs.clear() ;

    StringList_t AllLinesEnums ;
    LSelector.GetAllValues( AllLinesEnums ) ;
    size_t i = 0 ;
    FXString CurrentEnum ;
    for ( ; i < AllLinesEnums.size() ; i++ )
    {
      FXPropertyKit CEP_pk ;
      FXString sLineMode , sLineSrc ;
      BOOL bLineInvert = false ;
      CurrentEnum = AllLinesEnums[ i ].c_str() ;
      try
      {
        LSelector.SetValue( (LPCTSTR) CurrentEnum );
        if ( m_Camera.LineMode.IsValid() && m_Camera.LineMode.IsReadable() )
        {
          sLineMode = m_Camera.LineMode.ToString().c_str() ;
          if ( !bOnlyComplexIOsFill )
            CEP_pk.WriteString( (LPCTSTR) (m_Camera.LineMode.GetNode()->GetName() + "_") , sLineMode ) ;
        }
        if ( m_Camera.LineSource.IsValid() && m_Camera.LineSource.IsReadable() )
        {
          sLineSrc = m_Camera.LineSource.ToString().c_str() ;
          if ( !bOnlyComplexIOsFill )
            CEP_pk.WriteString( (LPCTSTR) (m_Camera.LineSource.GetNode()->GetName() + "_") , sLineSrc );
        }
        if ( m_Camera.LineInverter.IsValid() )
        {
          BOOL bLineInvert = m_Camera.LineInverter.GetValue() ? TRUE : FALSE ;
          if ( !bOnlyComplexIOsFill )
            CEP_pk.WriteInt( (LPCTSTR) (m_Camera.LineInverter.GetNode()->GetName() + "_") , bLineInvert ) ;
        }
        if ( !CEP_pk.IsEmpty() )
        {
          CEP_pk.Insert( 0 , _T( "(" ) ) ;
          CEP_pk += ")" ;
          pc += '\n' ;
          pc.WriteString( CurrentEnum , CEP_pk , false ) ;
          CEP_pk.Empty() ;
        }
        BaslerIOLine NewLine( CurrentEnum , sLineMode , sLineSrc , bLineInvert ) ;
        if ( bOnlyComplexIOsFill )
          m_ComplexIOs.m_Lines.push_back( NewLine ) ;
        else
        {
          BaslerIOLine * pLine = m_ComplexIOs.FindLine( CurrentEnum ) ;
          if ( pLine )
            *pLine = NewLine ;
          else
            m_ComplexIOs.m_Lines.push_back( NewLine ) ;
        }
      }
      catch ( const GenericException &e )
      {
        FXString CatchData ;
        CatchData.Format( _T( "PrintCameraLinesProperties - %sException catch for line  print: %s\n" ) ,
          (LPCTSTR) CurrentEnum , e.GetDescription() ) ;
        {
          SEND_GADGET_ERR( ( LPCTSTR ) CatchData );
        }
        TRACE( "\n   %s" , (LPCTSTR) CatchData ) ;
      }
    }
    LSelector.SetValue( (LPCTSTR) SavedSelected ) ;
    return (int) AllLinesEnums.size() ;
  }
  catch ( const GenericException &e )
  {
    FXString CatchData ;
    CatchData.Format( _T( "PrintCameraLinesProperties 2 - %sException catch for Print Lines: %s\n" ) ,
      (LPCTSTR) m_Tabs , e.GetDescription() ) ;
    if ( m_bLoggingON )
    {
      SEND_GADGET_ERR( ( LPCTSTR ) CatchData );
    }
    TRACE( "\n   %s" , (LPCTSTR) CatchData ) ;
  }
  return 0 ;
}
int Basler::PrintCameraUserOutputsProperties( FXPropertyKit& pc , bool bOnlyComplexIOsFill )
{
  LPCTSTR psSelector = (LPCTSTR) (GetThisCamProperty( OUTPUT_SELECTOR )->m_InDeviceName) ;
  if ( !psSelector )
    return 0 ;

  try
  {
    INodeMap * pNodeMap = GetNodeMapWithCatching() ;
    if (!pNodeMap)
      return false ;

    INodeMap& nodemap = *pNodeMap;

    CEnumParameter OSelector( nodemap , psSelector );

    String_t SavedSelected = OSelector.GetValue() ;

    if ( bOnlyComplexIOsFill )
      m_ComplexIOs.m_Outs.clear() ;

    StringList_t AllOutputsEnums ;
    OSelector.GetAllValues( AllOutputsEnums ) ;
    size_t i = 0 ;
    FXString CurrentEnum ;
    for ( ; i < AllOutputsEnums.size() ; i++ )
    {
      FXPropertyKit CEP_pk ;
      BOOL bOutputValue = false ;
      CurrentEnum = AllOutputsEnums[ i ].c_str() ;
      OSelector.SetValue( (LPCTSTR) CurrentEnum );
      if ( m_Camera.UserOutputSelector.IsValid() )
      {
        bOutputValue = m_Camera.UserOutputValue.GetValue() ;
        if ( !bOnlyComplexIOsFill )
          CEP_pk.WriteInt( (LPCTSTR) (m_Camera.UserOutputValue.GetNode()->GetName() + "_") , bOutputValue ) ;
      }
      if ( !CEP_pk.IsEmpty() )
      {
        CEP_pk.Insert( 0 , _T( "(" ) ) ;
        CEP_pk += ")" ;
        pc += '\n' ;
        pc.WriteString( CurrentEnum , CEP_pk , false ) ;
        CEP_pk.Empty() ;
      }
      BaslerUserOut NewOut( CurrentEnum , bOutputValue ) ;
      if ( bOnlyComplexIOsFill )
        m_ComplexIOs.m_Outs.push_back( NewOut ) ;
      else
      {
        BaslerUserOut * pOut = m_ComplexIOs.FindOut( CurrentEnum ) ;
        if ( pOut )
          *pOut = NewOut ;
        else
          m_ComplexIOs.m_Outs.push_back( NewOut ) ;
      }
    }
    OSelector.SetValue( (LPCTSTR) SavedSelected ) ;
    return (int) AllOutputsEnums.size() ;
  }
  catch ( const GenericException &e )
  {
    // Error handling
    FXString CatchData ;
    CatchData.Format( _T( "PrintCameraUserOutputsProperties - %sException catch for Print User Output Properties: %s\n" ) ,
      (LPCTSTR) m_Tabs , e.GetDescription() ) ;
    if ( m_bLoggingON )
    {
      SEND_GADGET_ERR( ( LPCTSTR ) CatchData );
    }
    TRACE( "\n   %s" , (LPCTSTR) CatchData ) ;
  }
  return 0 ;
}


EventSelectorEnums Basler::ConvertNameToEnum( LPCTSTR pName )
{
  if ( m_EventCorresp.size() )
  {
    auto result = m_EventCorresp.find( pName );
    if ( result != m_EventCorresp.end() )
      return result->second;
  }
  return (EventSelectorEnums) (-1);
}

int Basler::ScanCameraEventsProperties( FXPropKit2& pc )
{
  return 0 ;
  BaslerCamProperty * pSelector = GetThisCamProperty( EVENT_SELECTOR ) ;
  BaslerCamProperty * pEnabled = GetThisCamProperty( EVENT_ENABLE ) ;
  BaslerCamProperty * pEventsStatus = GetThisCamProperty( EVENTS_STATUS );
  if ( !pSelector || !pEnabled )
    return 0 ;

  bool bWasSomething = false;

  if ( pEventsStatus )
  {
    FXPropertyKit CEP_pk;
    pc.GetStringWithBrackets( pEventsStatus->name , CEP_pk );
    if ( !CEP_pk.IsEmpty() )
    {
      for ( size_t i = 0; i < pSelector->m_EnumeratorsAsTexts.size(); i++ )
      {
        BOOL bEnabled = FALSE;
        if ( CEP_pk.GetInt( pSelector->m_EnumeratorsAsTexts[ i ].c_str() , bEnabled ) )
        {
          EventSelectorEnums Result =
            ConvertNameToEnum( pSelector->m_EnumeratorsAsTexts[ i ].c_str() );
          if ( (int) Result >= 0 )
          {
            m_Camera.EventSelector.SetValue( Result );
            m_Camera.EventNotification.TrySetValue( bEnabled ? EventNotification_On : EventNotification_Off );
            bWasSomething = true;
          }
        }
      }
    }
  }

  int iSelected = -1 ;
  if ( pc.GetInt( pSelector->name , iSelected ) )
  {
    //     for ( size_t i = 0 ; i < pSelector->m_EnumeratorsAsValues.size() ; i++ )
    //     {
    //       if ( iSelected == (int)pSelector->m_EnumeratorsAsValues[i] )
    //       {
    //         if ( i < pSelector->m_EnumeratorsAsTexts.size() )
    //         {
    //           EventSelectorEnums Result =
    //             ConvertNameToEnum( pSelector->m_EnumeratorsAsTexts[i].c_str() );
    //           if ( (int)Result >= 0 )
    //           {
    //             m_Camera.EventSelector.SetValue(Result);
    //             bWasSomething = true;
    //             break;
    //           }
    //         }
    //       }
    //     }
    if ( iSelected < (int) pSelector->m_EnumeratorsAsTexts.size() )
    {
      EventSelectorEnums Result =
        ConvertNameToEnum( pSelector->m_EnumeratorsAsTexts[ iSelected ].c_str() );
      if ( (int) Result >= 0 )
      {
        m_Camera.EventSelector.SetValue( Result );
        bWasSomething = true;
      }
    }
  }
  BOOL bEnabled = FALSE;
  if ( pc.GetInt( pEnabled->name , bEnabled ) )
  {
    m_Camera.EventNotification.TrySetValue(
      bEnabled ? EventNotification_On : EventNotification_Off );
    bWasSomething = true;
  }
  return bWasSomething ? 1 : 0 ;
}

int Basler::ScanTimersProperties( LPCTSTR pTimerName , FXPropKit2& pc )
{
  FXPropertyKit TimerProperty ;
  if ( !m_Camera.IsOpen() ||  !pc.GetStringWithBrackets( pTimerName , TimerProperty ) 
    || !GetThisCamProperty( TIMER_SELECT ) )
    return 0 ;
  LPCTSTR psSelector = (LPCTSTR) (GetThisCamProperty( TIMER_SELECT )->m_InDeviceName) ;
  FXString psTTriggerSrc = (LPCTSTR) (GetThisCamProperty( TIMER_TRIGGER_SOURCE )->m_InDeviceName) ;
  FXString psTDuration = (LPCTSTR) (GetThisCamProperty( TIMER_DURATION )->m_InDeviceName);
  FXString psTDelay = (LPCTSTR) (GetThisCamProperty( TIMER_DELAY )->m_InDeviceName);
  //LPCTSTR psTPolarity = (LPCTSTR) (GetThisCamProperty( TIMER_TRIGGER_POLARITY )->m_InDeviceName);
  if ( !psSelector || !psTDuration || !psTDelay || /*!psTPolarity ||*/ !psTTriggerSrc )
    return 0 ;

  int iNChanged = 0 ;
  try
  {
    TimerSelectorEnums SavedSelected = m_Camera.TimerSelector.GetValue() ;

    m_Camera.TimerSelector.SetValue( pTimerName ) ;
    double dValue = 0. ;
    
    if ( TimerProperty.GetDouble( psTDuration + '_' , dValue ) )
    {
      m_Camera.TimerDuration.SetValue( dValue ) ;
      iNChanged++ ;
    }

    if ( TimerProperty.GetDouble( psTDelay + '_' , dValue ) )
    {
      m_Camera.TimerDelay.SetValue( dValue ) ;
      iNChanged++ ;
    }

    FXString sValue ;
    if ( TimerProperty.GetString( psTTriggerSrc + '_' , sValue ) )
    {
      m_Camera.TimerTriggerSource.SetValue( (LPCTSTR) sValue ) ;
      iNChanged++ ;
    }

    m_Camera.TimerSelector.SetValue( SavedSelected ) ;
    return iNChanged ;
  }
  catch ( const GenericException &e )
  {
    // Error handling
    FXString CatchData ;
    CatchData.Format( _T( "ScanTimersProperties - %sException catch : %s\n" ) ,
      (LPCTSTR) m_Tabs , e.GetDescription() ) ;
    {
      SEND_GADGET_ERR( ( LPCTSTR ) CatchData );
    }
    TRACE( "\n   %s" , (LPCTSTR) CatchData ) ;
  }
  return 0 ;
}
int Basler::ScanLineProperties( LPCTSTR pLineName , FXPropKit2& pc )
{
  if ( !GetThisCamProperty( LINE_SELECTOR ) )
    return 0 ;
  FXPropertyKit LineProperty ;
  if ( !pc.GetStringWithBrackets( pLineName , LineProperty ) )
    return 0 ;

  LPCTSTR psSelector = (LPCTSTR) (GetThisCamProperty( LINE_SELECTOR )->m_InDeviceName) ;
  if ( !psSelector )
    return 0 ;

  try
  {
    INodeMap * pNodeMap = GetNodeMapWithCatching() ;
    if (!pNodeMap)
      return false ;

    INodeMap& nodemap = *pNodeMap;

    CEnumParameter LSelector( nodemap , psSelector );

    String_t SavedSelected = LSelector.GetValue() ;
    LSelector.SetValue( pLineName ) ;


    int iNChanged = 0 ;
    LineProperty.Trim( "()[]{}" ) ;
    FXString Value ;
    FXString Name = m_Camera.LineMode.GetNode()->GetName().c_str() ;
    if ( m_Camera.LineMode.IsValid() && LineProperty.GetString( Name + '_' , Value ) )
    {
      m_Camera.LineMode.SetValue( (LPCTSTR) Value ) ;
      iNChanged++ ;
    }
    Name = m_Camera.LineSource.GetNode()->GetName().c_str() ;
    if ( m_Camera.LineSource.IsValid() && LineProperty.GetString( Name + '_' , Value ) )
    {
      m_Camera.LineSource.SetValue( (LPCTSTR) Value ) ;
      iNChanged++ ;
    }
    Name = m_Camera.LineInverter.GetNode()->GetName().c_str() ;
    BOOL bValue = FALSE ;
    if ( m_Camera.LineInverter.IsValid() && LineProperty.GetInt( Name + '_' , bValue ) )
    {
      bool bNow = m_Camera.LineInverter.GetValue() ;
      m_Camera.LineInverter.SetValue( bValue != FALSE ) ;
      iNChanged++ ;
    }

    LSelector.SetValue( (LPCTSTR) SavedSelected ) ;
    return iNChanged ;
  }
  catch ( const GenericException &e )
  {
    // Error handling
    FXString CatchData ;
    CatchData.Format( _T( "ScanLinesProperties - %sException catch r %s\n" ) ,
      pLineName , e.GetDescription() ) ;
    {
      SEND_GADGET_ERR( ( LPCTSTR ) CatchData );
    }
    TRACE( "\n   %s" , (LPCTSTR) CatchData ) ;
  }
  return 0 ;

}

bool Basler::PrintProperties( FXString& text )
{
  FXPropertyKit pc ;

  FXPropertyKit pc2( text ) , Requested ;
  FXStringArray RequestedProperties ;
  if ( pc2.GetString( "Properties" , Requested ) )
  {
    SplitFXString( Requested , _T( "," ) , RequestedProperties ) ;
  }

  CSetupObject * pSetupObject = GetSetupObject() ;
  if ( DriverValid() && IsSNLegal( m_sSerialNumber ) )
  {
    if ( !RequestedProperties.Size() 
      ||  IsStringInArray( "Camera" , RequestedProperties ) >= 0 )
    {
      pc.WriteString( "Camera" , m_sSerialNumber );
      if ( m_bSelectByGivenName && !m_sCameraGivenName.IsEmpty() )
        pc.WriteString( "CameraName" , m_sCameraGivenName ) ;
    }
    if ( !RequestedProperties.Size()
      || IsStringInArray( "StreamState" , RequestedProperties ) >= 0 )
    {
      pc.WriteInt( "StreamState" , m_bLocalStopped );
    }
    if ( !m_sConnectedSerialNumber.IsEmpty() )
    {
//      ASSERT( m_sConnectedSerialNumber == m_sSerialNumber ) ;
      int iNEvents = 0 , iNTimers = 0 , iNLines = 0 ;

      for ( int i = 0; i < m_ThisCamProperty.GetSize(); i++ )
      {
        if ( RequestedProperties.Size() )
        {
          if ( IsStringInArray( m_ThisCamProperty[ i ].name , RequestedProperties ) < 0 )
            continue ;
        }
        INT_PTR value;
        bool bauto;
        m_PropertyListLock.Lock( INFINITE , "Basler::PrintProperties" ) ;
        BaslerCamProperty Prop = m_ThisCamProperty[ i ];
        m_PropertyListLock.Unlock();
        //For  enumerated lists printing
        switch ( Prop.id )
        {
        case TIMER_SELECT:
          {
            if ( !iNTimers )
              iNTimers = PrintCameraTimersProperties( pc ) ;
          }
        case LINE_SELECTOR:
          {
            if ( !iNLines )
              iNLines = PrintCameraLinesProperties( pc ) ;
          }
        default:
          break;
        }
        switch ( Prop.id )
        {
        case EVENT_SELECTOR:
          {
            iNEvents = PrintCameraEventsProperties( pc ) ;
          }
          break ;
          // Continue from previous for current state show 
        default:
          {
            if ( GetCameraProperty( Prop.id , value , bauto ) )
            {
              FXString key;
              FXParser param;
              Prop.m_GUIFormat.GetElementNo( 0 , key , param );
              if ( (key == SETUP_COMBOBOX) || (key == SETUP_SPIN) ) // ints result
              {
                FXString tmpS;
                switch ( Prop.m_DataType )
                {
                case PDTCommand:
                case PDTEnum:
                case PDTInt: tmpS.Format( "%d" , (int) value ); break;
                case PDTString:
                case PDTFloat: tmpS = (LPCTSTR) value; break;
                case PDTBool: tmpS = (value != 0) ? '1' : '0'; break;
                }
                pc.WriteString( Prop.name , tmpS );
              }
              else if ( key == SETUP_SPINABOOL )
              {
                FXString tmpS;
                
//                 else
//                 {
                switch ( Prop.m_DataType )
                {
                  case PDTEnum:
                  case PDTInt: tmpS.Format( "%d" , ( int ) value ); break;
                  case PDTString:
                  case PDTFloat: tmpS = ( LPCTSTR ) value; break;
                  case PDTBool: tmpS = ( value != 0 ) ? '1' : '0'; break;
                }
//                }

                if ( bauto )
                  tmpS.Insert( 0 , _T( "auto " ) ) ;
                pc.WriteString( Prop.name , tmpS , false );
              }
              else if ( key == SETUP_EDITBOX )
              {
                FXString svalue = (LPCTSTR) value;
                pc.WriteString( Prop.name , svalue );
              }
              else if ( !IsNotViwed( m_ThisCamProperty[ i ].id ) && !Prop.m_bDontShow )
              {
                DEVICESENDERR_1( "Unsupported key '%s' PrintProperty" , key );
              }
            }
          }
          break ;
        }
      }
      if ( !m_bInitialOK )
      {
        m_InitialProperties = pc ;
      }
    }
    else
      pc.WriteString( "Camera" , m_bSelectByGivenName ? _T( "-2" ) : _T( "-1" ) );
  }
  else
  {
//     if ( ( m_InitialProperties.GetLength() > 30) && m_bInitialOK )
//         pc = m_InitialProperties ;
//     else
      pc.WriteString( "Camera" , m_bSelectByGivenName ? _T("-2") : _T( "-1" ) );
  }
  text = pc;
  //TRACE( "\nBasler::PrintProperties - %s\n" , ( LPCTSTR )text ) ;
  return true;
}


int Basler::SetPacketSize( int iPacketSize_Or_FPSx10 , bool bFrameRate )
{
  if ( !m_Camera.IsOpen() )
    return 0;

  return -1;
}
int Basler::SetMaxPacketSize()
{
  if ( !m_Camera.IsOpen() )
    return 0;
  return 0;
}

int Basler::GetMaxPacketSize()
{
  if ( !m_Camera.IsOpen() )
    return 0;

  return 0;
}
bool Basler::SetFrameRate( double dFrameRate )
{

  return false;
}

bool Basler::GetFrameRate( double& dFrameRate )
{
  return false;
}
int Basler::GetColorMode()
{
  switch ( m_pixelFormat )
  {
  case PixelFormat_BayerGR8:
  case PixelFormat_BayerGB8:
  case PixelFormat_BayerRG8:
  case PixelFormat_BayerBG8:
  case PixelFormat_BayerGR10:
  case PixelFormat_BayerGB10:
  case PixelFormat_BayerRG10:
  case PixelFormat_BayerBG10:
  case PixelFormat_BayerGR12:
  case PixelFormat_BayerGB12:
  case PixelFormat_BayerRG12:
  case PixelFormat_BayerBG12:
  case PixelFormat_BayerGR10p:
  case PixelFormat_BayerGB10p:
  case PixelFormat_BayerRG10p:
  case PixelFormat_BayerBG10p:
  case PixelFormat_BayerGR12p:
  case PixelFormat_BayerGB12p:
  case PixelFormat_BayerRG12p:
  case PixelFormat_BayerGR16:
  case PixelFormat_BayerGB16:
  case PixelFormat_BayerRG16:
  case PixelFormat_BayerBG16:
  case PixelFormat_RGB12V1Packed:
    return COLOR_BY_SOFTWARE; // colors by program
  case PixelFormat_YUV411Packed:
  case PixelFormat_YUV422Packed:
  case PixelFormat_YUV444Packed:
    return COLOR_BY_CAMERA; // color by camera

  default:
    return COLOR_BW; // bw mode
  }
}

void Basler::SaveCameraInfo( const CInstantCamera& camera , int iIndex )
{
  FXString ErrorMsg;

  string strModelName = camera.GetDeviceInfo().GetModelName();
  string strName = camera.GetDeviceInfo().GetUserDefinedName();
  string strSerialNumber = camera.GetDeviceInfo().GetSerialNumber();
  string strInterfaceID = camera.GetDeviceInfo().GetDeviceClass();

  if ( strModelName.empty() )
  {
    ErrorMsg.Format( "Could not get camera %d model name. " ,
      iIndex ) ;
    SEND_DEVICE_ERR( (LPCTSTR) ErrorMsg ) ;
    strcpy( m_CamInfo[ iIndex ].model , _T( "Unknown Model" ) ) ;
  }
  else
    strcpy( m_CamInfo[ iIndex ].model , strModelName.c_str() ) ;

  if ( strName.empty() )
  {
    ErrorMsg.Format( "Could not get camera %d name. " ,
      iIndex ) ;
    strcpy( m_CamInfo[ iIndex ].m_sGivenName , _T( "Unknown name" ) ) ;
    SEND_DEVICE_ERR( (LPCTSTR) ErrorMsg ) ;
  }
  else
    strcpy( m_CamInfo[ iIndex ].m_sGivenName , strName.c_str() ) ;

  if ( strSerialNumber.empty() )
  {
    ErrorMsg.Format( "Could not get serial number for camera %d." ,
      iIndex ) ;
    m_CamInfo[ iIndex ].serialnmb = 0 ;
    strcpy( m_CamInfo[ iIndex ].szSerNum , _T( "Unknown SN" ) ) ;
    SEND_DEVICE_ERR( (LPCTSTR) ErrorMsg ) ;
  }
  else
  {  // save to camera info
    strcpy( m_CamInfo[ iIndex ].szSerNum , strSerialNumber.c_str() ) ;
    FXString szSerNum = strSerialNumber.c_str() ;
    int iSNIndex = (int) szSerNum.GetLength() - 1 ;
    LPCTSTR pStr = (LPCTSTR) szSerNum ;
    while ( iSNIndex > 0 && !isdigit( pStr[ iSNIndex ] ) )
      iSNIndex-- ;
    while ( iSNIndex > 0 && isdigit( pStr[ iSNIndex ] ) )
      iSNIndex-- ;
    if ( iSNIndex <= 0 )
      iSNIndex = 0 ;
    else
      iSNIndex++ ;
    m_CamInfo[ iIndex ].serialnmb = atoi( pStr + iSNIndex ) ;
  }

  if ( strInterfaceID.empty() )
  {
    ErrorMsg.Format( "Could not get device class for camera %d." ,
      iIndex ) ;
    SEND_DEVICE_ERR( (LPCTSTR) ErrorMsg ) ;
    strcpy( m_CamInfo[ iIndex ].InterfaceId , _T( "Unknown interface" ) ) ;
  }
  else
    strcpy( m_CamInfo[ iIndex ].InterfaceId , strInterfaceID.c_str() ) ;
}

void Basler::SaveCameraInfo( const CDeviceInfo& DevInfo , int iIndex )
{
  FXString ErrorMsg;

  string strModelName = DevInfo.GetModelName();
  string strName = DevInfo.GetUserDefinedName();
  string strSerialNumber = DevInfo.GetSerialNumber();
  string strInterfaceID = DevInfo.GetDeviceClass();

  if ( strModelName.empty() )
  {
    ErrorMsg.Format( "Could not get camera %d model name. " ,
      iIndex ) ;
    SEND_DEVICE_ERR( (LPCTSTR) ErrorMsg ) ;
    strcpy( m_CamInfo[ iIndex ].model , _T( "Unknown Model" ) ) ;
  }
  else
    strcpy( m_CamInfo[ iIndex ].model , strModelName.c_str() ) ;

  if ( strName.empty() )
  {
    ErrorMsg.Format( "Could not get camera %d name. " ,
      iIndex ) ;
    strcpy( m_CamInfo[ iIndex ].m_sGivenName , _T( "Unknown name" ) ) ;
    SEND_DEVICE_ERR( (LPCTSTR) ErrorMsg ) ;
  }
  else
    strcpy( m_CamInfo[ iIndex ].m_sGivenName , strName.c_str() ) ;

  if ( strSerialNumber.empty() )
  {
    ErrorMsg.Format( "Could not get serial number for camera %d." ,
      iIndex ) ;
    m_CamInfo[ iIndex ].serialnmb = 0 ;
    strcpy( m_CamInfo[ iIndex ].szSerNum , _T( "Unknown SN" ) ) ;
    SEND_DEVICE_ERR( (LPCTSTR) ErrorMsg ) ;
  }
  else
  {  // save to camera info
    strcpy( m_CamInfo[ iIndex ].szSerNum , strSerialNumber.c_str() ) ;
    FXString szSerNum = strSerialNumber.c_str() ;
    int iSNIndex = (int) szSerNum.GetLength() - 1 ;
    LPCTSTR pStr = (LPCTSTR) szSerNum ;
    while ( iSNIndex > 0 && !isdigit( pStr[ iSNIndex ] ) )
      iSNIndex-- ;
    while ( iSNIndex > 0 && isdigit( pStr[ iSNIndex ] ) )
      iSNIndex-- ;
    if ( iSNIndex <= 0 )
      iSNIndex = 0 ;
    else
      iSNIndex++ ;
    m_CamInfo[ iIndex ].serialnmb = atoi( pStr + iSNIndex ) ;
  }

  if ( strInterfaceID.empty() )
  {
    ErrorMsg.Format( "Could not get device class for camera %d." ,
      iIndex ) ;
    SEND_DEVICE_ERR( (LPCTSTR) ErrorMsg ) ;
    strcpy( m_CamInfo[ iIndex ].InterfaceId , _T( "Unknown interface" ) ) ;
  }
  else
    strcpy( m_CamInfo[ iIndex ].InterfaceId , strInterfaceID.c_str() ) ;
}

void Basler::HandleNode( INode * pNode , FILE * pSaveFile )
{
  PropertyDataType Type = GetType( pNode ) ;
  FXString Name = pNode->GetName().c_str() ;

  if ( Type == PDTCategory )
  {
    FXString CategoryNameOut ;
    CategoryNameOut.Format( _T( "%sCategory: %s\n" ) ,
      (LPCTSTR) m_Tabs , (LPCTSTR) Name ) ;
    if ( pSaveFile )
      fwrite( (LPCTSTR) CategoryNameOut , 1 , CategoryNameOut.GetLength() , pSaveFile ) ;

    m_Tabs += _T( "  " ) ;
    NodeList_t nodes;
    NodeList_t::iterator  it;
    pNode->GetChildren( nodes );
    for ( it = nodes.begin() ; it != nodes.end(); it++ )
    {
      INode* pChildNode = (*it);
      HandleNode( pChildNode , pSaveFile ) ;
    }
    if ( m_Tabs.GetLength() > 2 )
      m_Tabs = m_Tabs.Mid( 2 ) ;
    else
      m_Tabs.Empty() ;
  }
  else if ( IsImplemented( pNode ) )
  {
    FXString AsText ;
    FXString DisplayName = pNode->GetDisplayName().c_str() ;
    FXString Description = pNode->GetDescription().c_str() ;
    FXString Tabs ;

    try
    {
      switch ( Type )
      {
      case PDTUnknown:
        break;
      case PDTBool:
        {
          CBooleanPtr Parameter( pNode->GetNodeMap()->GetNode( (LPCTSTR) Name ) ) ;
          if ( Parameter.IsValid() )
          {
            AsText.Format( _T( "%s%s %s(%s)=%s - %s\n" ) ,
              (LPCTSTR) m_Tabs , _T( "Bool" ) , (LPCTSTR) Name ,
              (LPCTSTR) DisplayName ,
              Parameter->GetValue() ? _T( "true" ) : _T( "false" ) ,
              pNode->GetDescription().c_str() );
          }
          else
          {
            AsText.Format( _T( "%sNot valid bool ptr for %s\n" ) ,
              (LPCTSTR) m_Tabs , (LPCTSTR) Name ) ;
          }
          if ( pSaveFile )
            fwrite( (LPCTSTR) AsText , 1 , AsText.GetLength() , pSaveFile ) ;
        }
        break;
      case PDTInt:
        {
          CIntegerPtr Parameter( pNode->GetNodeMap()->GetNode( (LPCTSTR) Name ) );
          if ( Parameter.IsValid() )
          {
            int64_t i64Max = Parameter->GetMax() ;
            int64_t i64Min = Parameter->GetMin() ;
            int64_t i64Val = Parameter->GetValue();
            AsText.Format( _T( "%s%s %s(%s)=%lld[%lld,%lld] - %s\n" ) ,
              (LPCTSTR) m_Tabs , _T( "Int" ) , (LPCTSTR) Name ,
              (LPCTSTR) DisplayName ,
              i64Val , i64Min , i64Max ,
              (LPCTSTR) Description );
          }
          else
          {
            AsText.Format( _T( "%sNot valid int ptr for %s\n" ) ,
              (LPCTSTR) m_Tabs , (LPCTSTR) Name ) ;
          }
          if ( pSaveFile )
            fwrite( (LPCTSTR) AsText , 1 , AsText.GetLength() , pSaveFile ) ;
        }
        break;
      case PDTFloat:
        {
          CFloatPtr Parameter( pNode->GetNodeMap()->GetNode( (LPCTSTR) Name ) );
          if ( Parameter.IsValid() )
          {
            double dMax = Parameter->GetMax() ;
            double dMin = Parameter->GetMin() ;
            double dVal = Parameter->GetValue();
            AsText.Format( _T( "%s%s %s(%s)=%g[%g,%g] - %s\n" ) ,
              (LPCTSTR) m_Tabs , _T( "Float" ) , (LPCTSTR) Name ,
              (LPCTSTR) DisplayName ,
              dVal , dMin , dMax ,
              (LPCTSTR) Description );
          }
          else
          {
            AsText.Format( _T( "%sNot valid float ptr for %s\n" ) ,
              (LPCTSTR) m_Tabs , (LPCTSTR) Name ) ;
          }
          if ( pSaveFile )
            fwrite( (LPCTSTR) AsText , 1 , AsText.GetLength() , pSaveFile ) ;
        }
        break;
      case PDTString:
        {
          CStringPtr Parameter( pNode->GetNodeMap()->GetNode( (LPCTSTR) Name ) );
          if ( Parameter.IsValid() )
          {
            AsText.Format( _T( "%s%s %s(%s)=%s - %s\n" ) ,
              (LPCTSTR) m_Tabs , _T( "String" ) , (LPCTSTR) Name ,
              (LPCTSTR) DisplayName ,
              Parameter->GetValue().c_str() ,
              (LPCTSTR) Description );
          }
          else
          {
            AsText.Format( _T( "%sNot valid string ptr for %s\n" ) ,
              (LPCTSTR) m_Tabs , (LPCTSTR) Name ) ;
          }
          if ( pSaveFile )
            fwrite( (LPCTSTR) AsText , 1 , AsText.GetLength() , pSaveFile ) ;
        }
        break;
      case PDTEnum:
        {
          CEnumerationPtr Parameter( pNode->GetNodeMap()->GetNode( (LPCTSTR) Name ) );
          if ( Parameter.IsValid() )
          {
            int64_t iVal = Parameter->GetIntValue() ;
            IEnumEntry * EE = Parameter->GetCurrentEntry() ;
            GenICam_3_1_Basler_pylon::gcstring Symbolic ;
            if ( EE )
              Symbolic = EE->GetSymbolic() ;

            const char * pSymbolic = Symbolic.c_str() ;
            AsText.Format( _T( "%s%s %s(%s)=%lld(%s) : " ) ,
              (LPCTSTR) m_Tabs , _T( "Enum" ) , (LPCTSTR) Name ,
              (LPCTSTR) DisplayName , iVal , pSymbolic ) ;

            NodeList_t entries;
            Parameter->GetEntries( entries );
            FXString Addition ;
            for ( NodeList_t::iterator it = entries.begin(); it != entries.end(); ++it )
            {
              CEnumEntryPtr pEntry = (*it);
              string EntryName = pEntry->ToString().c_str() ;
              string Symbolic = pEntry->GetSymbolic().c_str() ;
              int64_t i64Value = pEntry->GetValue() ;
              if ( IsAvailable( pEntry ) )
              {
                Addition.Format( _T( "%s%lld(%s)" ) ,
                  (it != entries.begin()) ? _T( ", " ) : _T( "(" ) ,
                  i64Value , Symbolic.c_str() ) ;
                AsText += Addition ;
              }
            }
            Addition.Format( _T( ")\n %s- %s\n" ) ,
              (LPCTSTR) (m_Tabs + _T( "  " )) ,
              pNode->GetDescription().c_str() ) ;
            AsText += Addition ;
          }
          else
          {
            AsText.Format( _T( "(LPCTSTR)m_Tabs , Not valid enum ptr for %s\n" ) ,
              (LPCTSTR) m_Tabs , (LPCTSTR) Name ) ;
          }
          if ( pSaveFile )
            fwrite( (LPCTSTR) AsText , 1 , AsText.GetLength() , pSaveFile ) ;
        }
        break;
      case PDTCommand:
        {
          CCommandPtr Parameter( pNode->GetNodeMap()->GetNode( (LPCTSTR) Name ) );
          if ( Parameter.IsValid() )
          {
            AsText.Format( _T( "%s%s %s(%s) - %s\n" ) ,
              (LPCTSTR) m_Tabs , _T( "Command" ) , (LPCTSTR) Name ,
              (LPCTSTR) DisplayName ,
              (LPCTSTR) Description );
          }
          else
          {
            AsText.Format( _T( "%sNot valid command ptr for %s\n" ) ,
              (LPCTSTR) m_Tabs , (LPCTSTR) Name ) ;
          }
          if ( pSaveFile )
            fwrite( (LPCTSTR) AsText , 1 , AsText.GetLength() , pSaveFile ) ;
        }
        break;
      case PDTCategory:
        HandleNode( pNode , pSaveFile ) ;
        break;
      default:
        break;
      }
    }
    catch ( const GenericException &e )
    {
      // Error handling
      FXString CatchData ;
      CatchData.Format( _T( "HandleNode - %sException catch for Node %s: %s\n" ) ,
        (LPCTSTR) m_Tabs , (LPCTSTR) Name , e.GetDescription() ) ;
      if ( pSaveFile )
        fwrite( (LPCTSTR) CatchData , 1 , CatchData.GetLength() , pSaveFile ) ;
      else if ( m_bLoggingON && m_bInitialScanDone )
      {
        SEND_GADGET_ERR( ( LPCTSTR ) CatchData );
      }
    }
  }
}

bool Basler::EnumCameras()
{
  double dStart = GetHRTickCount() ;

  FXAutolock al( m_ConfigLock ) ;
  if ( m_bCamerasEnumerated && !m_bRescanCameras )
    return true ;

  FILE * pPropertyFile = NULL ;
  CTlFactory& tlFactory = CTlFactory::GetInstance();

  // Get all attached devices and exit application if no device is found.
  DeviceInfoList_t devices;
  try
  {
    if ( tlFactory.EnumerateDevices( devices ) == 0 )
    {
      SEND_DEVICE_ERR( "No camera present." );
    }


    m_iCamNum = (int) devices.size();
    CInstantCameraArray cameras( min( devices.size() , MAX_DEVICESNMB ) );

    CAutoLockMutex_H alm( m_Devices.GetMutexHandle() ) ;

    for ( unsigned i = 0; i < devices.size(); i++ )
    {
      double dCameraStart = GetHRTickCount() ;

      Device::GlobalDeviceInfo DevInfo ;

      if ( devices[ i ].IsSerialNumberAvailable() )
        strcpy_s( DevInfo.szSerialNumber , devices[ i ].GetSerialNumber().c_str() ) ;
      if ( devices[ i ].IsModelNameAvailable() )
        strcpy_s( DevInfo.szModelName , devices[ i ].GetModelName().c_str() ) ;
      if ( devices[ i ].IsUserDefinedNameAvailable() )
        strcpy_s( DevInfo.szCameraName , devices[ i ].GetUserDefinedName().c_str() ) ;
      if ( devices[ i ].IsDeviceClassAvailable() )
        strcpy_s( DevInfo.szInterfaceOrClass , devices[ i ].GetDeviceClass().c_str() ) ;
      
      for ( unsigned iIndex = 0 ; iIndex < i ; iIndex++ )
      {
        if ( _tcscmp( DevInfo.szCameraName , m_CamInfo[ iIndex ].m_sGivenName ) == 0 )
        {
          FXString Msg ;
          Msg.Format( "Cameras #%s and #%s \nhave the same name %s" ,
            m_CamInfo[ iIndex ].szSerNum , DevInfo.szSerialNumber ,
            DevInfo.szCameraName ) ;
          MessageBox( NULL , "Camera Name Doubling" , ( LPCTSTR ) Msg ,
            MB_ICONEXCLAMATION ) ;
        }
      }
//       if ( !_tcsstr( DevInfo.szInterfaceOrClass , "BaslerUsb" ) )
//         continue ;
      m_bLoggingON = false ;
        // Save to global cameras table (shared memory between 
        // all Basler gadgets in computer)
      SaveDevInfo( DevInfo , i ) ;
        // Save to cameras table in memory (m_CamInfo structures for all gadgets
        // in current application memory space)
      SaveCameraInfo( devices[ i ] , i ) ;
      m_bLoggingON = true ;
      double dAfterSave = GetHRTickCount() ;
      TRACE( "\n   Basler EnumCameras: Save %d info time is %g" , i , dAfterSave - dCameraStart ) ;
      // Open the desired camera by its ID
      //     m_LastError = m_System.OpenCameraByID( m_CamInfo[i].Id, VmbAccessModeFull, m_pCamera );
      //     if ( VmbErrorSuccess != m_LastError )
      //     {
      //       string_type ErrText = ErrorCodeToMessage( m_LastError ) ;
      //       SEND_DEVICE_ERR( "Can't open camera %d: Err=%s" , i , ErrText.c_str() ) ;
      //       return false ;
      //     }
      if ( !m_bSaveFullInfo )
        continue ;

      try
      {
        cameras[ i ].Attach( tlFactory.CreateDevice( devices[ i ] ) ) ;
      }
      catch ( const GenericException &e )
      {
        // Error handling
        SEND_GADGET_ERR( "EnumCameras: Camera Access exception occurred - %s" , e.GetDescription() );
        continue ;
      }
      AccessModeSet a( Guru );
      try
      {
        cameras[i].Open();
      }
      catch (const GenericException &e)
      {
        // Error handling
        SEND_GADGET_ERR("EnumCameras: Camera Open exception occurred - %s" , e.GetDescription());
        continue;
      }


      errno_t err = 0 ;
      if ( pPropertyFile == NULL )
      {
        CTime CurrTime = CTime::GetCurrentTime() ;

        FXString FileName ;
//         FileName.Format( _T( "BaslerCamProps%s.txt" ) ,
//           ( LPCTSTR ) CurrTime.Format( "%Y.%m.%d_%H-%M-%S" ) ) ;
        FileName.Format( _T( "BaslerCameras.txt" ) ) ;
        err = fopen_s( &pPropertyFile , (LPCTSTR) FileName , "wb" ) ;
        ASSERT( err == 0 ) ;
      }
      FXString Out ;
      double dAfterFileOpening = GetHRTickCount() ;
      TRACE( "\n   Basler EnumCameras: Open camera %s(%d) time is %g" ,
        (LPCTSTR) m_CamInfo[ i ].model , i , dAfterFileOpening - dCameraStart ) ;
//       Out.Format( "Camera SN%s-%s Interface=%s time=%g\n Properties: \n" ,
//         ( LPCTSTR ) m_CamInfo[ i ].szSerNum , ( LPCTSTR ) m_CamInfo[ i ].model ,
//         ( LPCTSTR ) m_CamInfo[ i ].InterfaceId , dAfterFileOpening - dCameraStart ) ;
      Out.Format( "Camera SN%s-%s Interface=%s \n" ,
        ( LPCTSTR ) m_CamInfo[ i ].szSerNum , ( LPCTSTR ) m_CamInfo[ i ].model ,
        ( LPCTSTR ) m_CamInfo[ i ].InterfaceId ) ;

      dCameraStart = dAfterFileOpening ;
      if ( pPropertyFile )
        fwrite( (LPCTSTR) Out , Out.GetLength() , 1 , pPropertyFile ) ;
      double dAfterFileWrite = GetHRTickCount() ;
      TRACE( "\n   Basler EnumCameras: Write cam %s(%d) info time is %g" ,
        (LPCTSTR) m_CamInfo[ i ].model , i , dAfterFileWrite - dCameraStart ) ;

      INodeMap& NodeMap = cameras[ i ].GetNodeMap() ;
      NodeList_t Nodes ;
      NodeMap.GetNodes( Nodes ) ;
      m_Tabs.Empty() ;
      for ( NodeList_t::iterator it = Nodes.begin() ; it != Nodes.end() ; it++ )
      {
        INode * pNode = *(it) ;
//         HandleNode( pNode , pPropertyFile ) ;
        HandleNode( pNode , NULL ) ; // we don't need save properties, only camera ids
      }
      cameras[ i ].Close() ;
      TRACE( "\n   Basler EnumCameras: Inspect properties of cam %d time is %g" ,
        i , GetHRTickCount() - dCameraStart ) ;
    }
    m_bSaveFullInfo = false ;
    m_bCamerasEnumerated = (devices.size() > 0) ;
    m_bRescanCameras = false ;
    m_NConnectedDevices = (int) devices.size() ;
  }
  catch ( const GenericException &e )
  {
    // Error handling
    SEND_GADGET_ERR( "EnumCameras: An exception occurred - %s" , e.GetDescription() );
  }
  if ( pPropertyFile )
    fclose( pPropertyFile ) ;
  if ( m_dwSerialNumber != -1 && m_dwSerialNumber != 0 )
  {
    m_GadgetInfo.Format( "Basler_%d" , m_dwSerialNumber ) ;
  }
  return true;
}

int Basler::CameraRemovedCB( Basler * pGadget , LPCTSTR pszRemovedCameraSN )
{
  if ( pGadget->m_sSerialNumber == pszRemovedCameraSN )
  {
    pGadget->m_bCameraRemoved = true ;
    pGadget->CamCNTLDoAndWait( BASLER_EVT_SHUTDOWN , 3000 ) ;
    return true ;
  }
  return false ;
}

void Basler::NewCameraCallback( FXStringArray& NewCameras )
{
  m_NewCameras = GetNames( NewCameras ) ;
  m_bCamerasEnumerated = false ;
  m_bRescanCameras = true ;
  if ( !IsSNLegal( m_sSerialNumber ) )
  {
    m_BusEvents |= BUS_EVT_ARRIVED ;
    SetEvent( m_evBusChange ) ;
    SEND_GADGET_INFO( (NewCameras.Count() == 1) ? "The new Camera is %s" : "New Cameras are %s" , m_NewCameras );
  }
  else if ( !m_Camera.IsOpen() && m_bRun )
  {
    for ( int i = 0 ; i < NewCameras.Count() ; i++ )
    {
      if ( NewCameras[i] == m_sSerialNumber )
      {
        if ( !m_continueGrabThread )
        {
          FXString ThreadName;
          m_hGrabThreadHandle = CreateThread( NULL , 0 ,
            ControlLoop , this , CREATE_SUSPENDED , &m_dwGrabThreadId );
          if ( m_hGrabThreadHandle )
          {
            m_BusEvents |= /*BUS_EVT_ARRIVED |*/ BASLER_EVT_INIT ;
            SetEvent( m_evBusChange ) ;
            SEND_GADGET_INFO( "Camera %s will be initialized" , NewCameras[ i ] );
            ResumeThread( m_hGrabThreadHandle );
            Sleep( 50 );
            m_continueGrabThread = true;
          }
          else
          {
            SEND_GADGET_ERR( "%s: %s" , (LPCTSTR) ThreadName , _T( "Can't start thread" ) );
            m_bInScanProperties = false;
          }
          return ;
        }
      }
    }
  }

}
void Basler::CameraDisconnected( FXStringArray& DisconnectedCameras )
{
  m_DisconnectedCameras = GetNames( DisconnectedCameras ) ;

  for ( int i = 0 ; i < DisconnectedCameras.Count() ; i++ )
  {
    if ( DisconnectedCameras[i] == m_sSerialNumber )
    {
      m_SavedCamProperty.Copy( m_ThisCamProperty ) ;
      m_SavedComplexIOs.Copy( m_ComplexIOs ) ;
      CameraRemovedCB( this , (LPCTSTR) m_sSerialNumber ) ;
      m_bCamerasEnumerated = false ;
      SEND_GADGET_INFO( "Camera %s Disconnected" , m_sSerialNumber );
    }
  }
  m_bRescanCameras = true ;
}


int Basler::SetComplexIOs( BaslerComplexIOs& Params )
{

  if ( m_Camera.LineSelector.IsValid() )
  {
    try
    {
      String_t SavedSelected = CEnumParameter(m_Camera.LineSelector.GetNode()).GetValue().c_str() ;
      for ( size_t i = 0 ; i < Params.m_Lines.size() ; i++ )
      {
        BaslerIOLine Line = Params.m_Lines[ i ] ;

        try
        {
          m_Camera.LineSelector.SetValue( Line.m_Name.c_str() ) ;
          if ( m_Camera.LineMode.IsValid() && m_Camera.LineMode.IsWritable() )
            m_Camera.LineMode.SetValue( Line.m_Mode.c_str() ) ;
          if ( m_Camera.LineSource.IsValid() && m_Camera.LineSource.IsWritable() )
            m_Camera.LineSource.SetValue( Line.m_Source.c_str() ) ;
          if ( m_Camera.LineInverter.IsValid() && m_Camera.LineInverter.IsWritable() )
            m_Camera.LineInverter.SetValue( Line.m_bInverter ) ;
        }
        catch ( const GenericException &e )
        {
          FXString CatchData ;
          CatchData.Format( _T( "SetComplexIOs - Exception catch on %s line restore: %s\n" ) ,
            Line.m_Name.c_str() , e.GetDescription() ) ;
          if ( m_bLoggingON )
          {
            SEND_GADGET_ERR( ( LPCTSTR ) CatchData );
          }
          TRACE( "\n   %s" , (LPCTSTR) CatchData ) ;
        }
      }
      m_Camera.LineSelector.SetValue( SavedSelected ) ;
    }
    catch ( const GenericException &e )
    {
      FXString CatchData ;
      CatchData.Format( _T( "SetComplexIOs 2 - Exception catch on IO lines restore\n" ) ,
       e.GetDescription() ) ;
      if ( m_bLoggingON )
      {
        SEND_GADGET_ERR( ( LPCTSTR ) CatchData );
      }
      TRACE( "\n   %s" , (LPCTSTR) CatchData ) ;

    }
  }
  if ( m_Camera.TimerSelector.IsValid() )
  {
    try
    {
      String_t SavedSelected = CEnumParameter( m_Camera.TimerSelector.GetNode() ).GetValue().c_str() ;
      for ( size_t i = 0 ; i < Params.m_Timers.size() ; i++ )
      {
        BaslerTimer Timer = Params.m_Timers[ i ] ;

        try
        {
          m_Camera.TimerSelector.SetValue( Timer.m_Name.c_str() ) ;
          if ( m_Camera.TimerTriggerSource.IsValid() && m_Camera.TimerTriggerSource.IsWritable() )
            m_Camera.TimerTriggerSource.SetValue( Timer.m_Source.c_str() ) ;
          if ( m_Camera.TimerTriggerActivation.IsValid() && m_Camera.TimerTriggerActivation.IsWritable() )
            m_Camera.TimerTriggerActivation.SetValue( Timer.m_Activation.c_str() ) ;
          if ( m_Camera.TimerDuration.IsValid() && m_Camera.TimerDuration.IsWritable() )
            m_Camera.TimerDuration.SetValue( Timer.m_dDuration ) ;
          if ( m_Camera.TimerDelay.IsValid() && m_Camera.TimerDelay.IsWritable() )
            m_Camera.TimerDelay.SetValue( Timer.m_dDelay ) ;
        }
        catch ( const GenericException &e )
        {
          FXString CatchData ;
          CatchData.Format( _T( "Exception catch on %s Timer restore: %s\n" ) ,
            Timer.m_Name.c_str() , e.GetDescription() ) ;
          if ( m_bLoggingON )
          {
            SEND_GADGET_ERR( ( LPCTSTR ) CatchData );
          }
          TRACE( "\n   %s" , (LPCTSTR) CatchData ) ;
        }
      }
      m_Camera.TimerSelector.SetValue( SavedSelected ) ;
    }
    catch ( const GenericException &e )
    {
      FXString CatchData ;
      CatchData.Format( _T( "Exception catch on Timers restore\n" ) ,
        e.GetDescription() ) ;
      if ( m_bLoggingON )
      {
        SEND_GADGET_ERR( ( LPCTSTR ) CatchData );
      }
      TRACE( "\n   %s" , (LPCTSTR) CatchData ) ;

    }
  }
  if ( m_Camera.UserOutputSelector.IsValid() )
  {
    try
    {
      String_t SavedSelected = CEnumParameter( m_Camera.UserOutputSelector.GetNode() ).GetValue().c_str() ;
      for ( size_t i = 0 ; i < Params.m_Outs.size() ; i++ )
      {
        BaslerUserOut Out = Params.m_Outs[ i ] ;

        try
        {
          m_Camera.UserOutputSelector.SetValue( Out.m_Name.c_str() ) ;
          if ( m_Camera.UserOutputValue.IsValid() )
            m_Camera.UserOutputValue.SetValue( Out.m_bValue ) ;
        }
        catch ( const GenericException &e )
        {
          FXString CatchData ;
          CatchData.Format( _T( "Exception catch on %s User IO restore: %s\n" ) ,
            Out.m_Name.c_str() , e.GetDescription() ) ;
          if ( m_bLoggingON )
          {
            SEND_GADGET_ERR( ( LPCTSTR ) CatchData );
          }
          TRACE( "\n   %s" , (LPCTSTR) CatchData ) ;
        }
      }
      m_Camera.UserOutputSelector.SetValue( SavedSelected ) ;
    }
    catch ( const GenericException &e )
    {
      FXString CatchData ;
      CatchData.Format( _T( "Exception catch on User IOs restore\n" ) ,
        e.GetDescription() ) ;
      if ( m_bLoggingON )
      {
        SEND_GADGET_ERR( ( LPCTSTR ) CatchData );
      }
      TRACE( "\n   %s" , (LPCTSTR) CatchData ) ;

    }
  }
  return 0;
}

void Basler::DeviceTriggerPulse( CDataFrame* pDataFrame )
{
  if (Tvdb400_IsEOS( pDataFrame ))
  {
//     SetSoftwareTriggerMode( false ) ;
  }
  else
  {
    if ( m_bSoftwareTriggerMode )
    {
      try
      {
        if ( m_Camera.TriggerSoftware.IsValid() )
          m_Camera.TriggerSoftware.Execute() ;
      }
      catch (const GenericException& e)
      {
        SEND_GADGET_ERR(_T("\nException Basler::DeviceTriggerPulse %s") ,
          (LPCTSTR) e.GetDescription() );
      }
//       if ( IsTriggerByInputPin() )
//         ::SetEvent( m_evSWTriggerPulse );
      m_bWasTriggerOnPin = true ;
    }
    else
      ::SetEvent( m_evSWTriggerPulse );
  }
  pDataFrame->Release();
}

INodeMap* Basler::GetNodeMapWithCatching()
{
  int iNAttempts = 0;
  INodeMap * pNodeMap = NULL;
  while (iNAttempts <= 10)
  {
    try
    {
      pNodeMap = &( m_Camera.GetNodeMap() );
    }
    catch (CException* e)
    {
      if (++iNAttempts < 10)
      {
        char ErrMsg[ 1000 ];
        e->GetErrorMessage( ErrMsg , sizeof( ErrMsg ) - 1 );
        SENDERR( "ERROR on Nodemap: %s" , ErrMsg ) ;
        Sleep( 50 );
        continue;
      }
      else
        return NULL ;
    }
    return pNodeMap ;
  }
  return NULL ;
}

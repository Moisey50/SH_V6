// MatVis.cpp: implementation of the MatVis class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <gadgets\gadbase.h>
#include <gadgets\stdsetup.h>
#include <gadgets\textframe.h>
#include <math\hbmath.h>
#include <math\Intf_sup.h>
#include "MatVis.h"
#include "MatVisCamera.h"
#include <video\shvideo.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "MatVisCamera.cpp"

IMPLEMENT_RUNTIME_GADGET_EX( MatVis , C1394Camera , "Video.capture" , TVDB400_PLUGIN_NAME );

CamProperty mvBlueCOUGAR[] =
{
  { FGP_IMAGEFORMAT , "PixelFormat" , "PixelFormat" , NULL , NULL , true } ,
{ FGP_ACQU_MODE , "AcqMode" , "AcquisitionMode" , NULL , NULL , true } ,
{ FGP_FRAME_RATE , "FrameRate" , "AcquisitionFrameRate" , NULL , NULL , true } ,
{ FGP_EXTSHUTTER , "Shutt_uS" , "ExposureTime" , "ExposureAuto" , NULL , false } ,
{ FGP_GAIN , "Gain_dBx10" , "Gain" , "GainAuto" , NULL , false } ,
{ FGP_ROI , "ROI" , NULL , NULL , NULL , true } ,
{ FGP_GRAB , "Grab" ,
   "Camera/GenICam/AcquisitionControl/AcquisitionFrameCount" , NULL , NULL , false } ,

{ FGP_TRIGGERONOFF , "TriggerMode" , "TriggerMode" , NULL , NULL , false } ,
{ FGP_TRIGGER_SOURCE , "TriggerSource" , "TriggerSource" , NULL , NULL , true } ,
{ FGP_TRIGGERDELAY , "TriggerDelay" , "TriggerDelay" , NULL , NULL , false } ,
{ FGP_TRIGGER_POLARITY , "Tr.Polarity" , "TriggerActivation" , NULL , NULL , true } ,
{ FGP_BINNING , "Binning" ,
  _T( "Camera/GenICam/ImageFormatControl/BinningHorizontal" ) , NULL , NULL , true } ,
{ FGP_DECIMATION , "Decimation" ,
  _T( "Camera/GenICam/ImageFormatControl/DecimationHorizontal" ) , NULL , NULL , true } ,

{ FGP_ADCGAIN , "ADCGain/R" ,
_T( "Camera/GenICam/AnalogControl/mvADCGain" ) , NULL , NULL , true } ,
{ FGP_VRAMP , "VRamp/R" ,
_T( "Camera/GenICam/AnalogControl/mvVRamp" ) , NULL , NULL , true } ,
{ FGP_BLACKLEVEL , "BlackLevel/R" ,
_T( "Camera/GenICam/AnalogControl/BlackLevel" ) , NULL , NULL , true } ,
{ FGP_DIGITALGAINOFFSET , "DigitaGainOffset/R" ,
_T( "Camera/GenICam/AnalogControl/mvDigitalGainOffset" ) , NULL , NULL , true } ,
{ FGP_TEMPERATURE_S , "SensorTemp/R" ,
_T( "Camera/GenICam/AnalogControl/mvDigitalGainOffset" ) , NULL , NULL , true } ,
 { FGP_SAVECALIBDATA , "SaveCalibData/R" , NULL , NULL , NULL , true } ,

//   { FGP_OUT_DELAY , "OutputDelay" , "IntEnaDelayTime" , NULL , NULL , true } ,
//   { FGP_BANDWIDTH , "LAN_Capacity" , "StreamBytesPerSecond" , NULL , NULL , true } ,
//   { FGP_WHITEBALCB , "WhiteBalBlue" , "WhitebalValueBlue" , "WhitebalMode Manual-Auto" , NULL , false } ,
//   { FGP_WHITEBALCR , "WhiteBalRed" , "WhitebalValueRed" , NULL , NULL , false } ,
//   { FGP_WHITEBAL_RATIO , "WB_Ratio" , "BalanceRatioRaw" , "BalanceWhiteAuto" , NULL , false } ,
//   { FGP_WHITEBAL_RATIO , "WB_Ratio" , "BalanceRatioAbs" , "BalanceWhiteAuto" , NULL , false } ,
//   { FGP_WHITEBAL_SELECT , "WB_Base" , "BalanceRatioSelector" , NULL , NULL , false } ,
//   { FGP_IRIS , "Iris" , NULL , NULL , NULL , false } ,
//   { FGP_FOCUS , "Focus" , NULL , NULL , NULL , false } ,
//   { FGP_ZOOM , "Zoom" , NULL , NULL , NULL , false } ,
//   { FGP_PAN , "Pan" , NULL , NULL , NULL , false } ,
//   { FGP_TILT , "Tilt" , NULL , NULL , NULL , false } ,
//   { FGP_LINE_SELECT , "LineSelect" , "LineSelector" , NULL , NULL , false } ,
//   { FGP_LINE_PARAMS , "LineParams" , NULL , NULL , NULL , false } ,
//   { FGP_LINE_SOURCE , "LineSource" , "LineSource" , NULL , NULL , false } ,
//   { FGP_LINE_INVERSE , "LineInverter" , "LineInverter" , NULL , NULL , false } ,
//   //   {FGP_LINE_DEBOUNCE , "Debouncing" , "LineDebounceTime" , NULL , NULL , true  } ,
//   { FGP_LOG , "Log" , NULL , NULL , NULL , false }

};

CamTypeAndProperties CougarTP =
{
  _T( "mvBlueCOUGAR" ) , ARR_SIZE( mvBlueCOUGAR ) , &mvBlueCOUGAR[ 0 ]
};

CamTypeAndProperties * KnownCameras[] =
{ &CougarTP /*, &GuppyProTP , &GuppyTP , &GoldEyeTP , &MantaTP*/ , NULL };

CamProperty CommonProperties[] =
{
  { FGP_IMAGEFORMAT , "PixelFormat" , "PixelFormat" , NULL , NULL , true } ,
{ FGP_ACQU_MODE , "AcqMode" , "AcquisitionMode" , NULL , NULL , true } ,
{ FGP_FRAME_RATE , "FrameRate" , "AcquisitionFrameRate" , NULL , NULL , true } ,
{ FGP_EXTSHUTTER , "Shutter_us" , "ExposureTime" , "ExposureAuto"/*"ExposureMode"*/ , NULL , false } ,
{ FGP_AUTOEXPOSURE , "ExpTarget" , "ExposureAutoTarget" , NULL , NULL , false } ,
{ FGP_GAIN , "Gain_dBx10" , "Gain" , "GainAuto" , NULL , false } ,
//   { FGP_GAIN , "SensorGain" , "SensorGain" , NULL , NULL , true } ,
{ FGP_GAMMA , "Gamma" , "Gamma" , NULL , NULL , false } ,
{ FGP_ROI , "ROI" , NULL , NULL , NULL , true } ,
{ FGP_TRIGGERONOFF , "TriggerMode" , "TriggerMode" , NULL , NULL , true } ,
{ FGP_TRIGGER_SOURCE , "TriggerSource" , "TriggerSource" , NULL , NULL , true } ,
{ FGP_TRIGGERDELAY , "TriggerDelay" , "TriggerDelay" , NULL , NULL , false } ,
{ FGP_TRIGGER_POLARITY , "Tr.Polarity" , "TriggerActivation" , NULL , NULL , false } ,
{ FGP_OUT_DELAY , "OutputDelay" , "IntEnaDelayTime" , NULL , NULL , true } ,
{ FGP_BANDWIDTH , "LAN_Capacity" , "StreamBytesPerSecond" , NULL , NULL , true } ,
{ FGP_WHITEBALCB , "WhiteBalBlue" , "WhitebalValueBlue" , "WhitebalMode Manual-Auto" , NULL , false } ,
{ FGP_WHITEBALCR , "WhiteBalRed" , "WhitebalValueRed" , NULL , NULL , false } ,
{ FGP_WHITEBAL_RATIO , "WB_Ratio" , "BalanceRatioRaw" , "BalanceWhiteAuto" , NULL , false } ,
{ FGP_WHITEBAL_RATIO , "WB_Ratio" , "BalanceRatioAbs" , "BalanceWhiteAuto" , NULL , false } ,
{ FGP_WHITEBAL_SELECT , "WB_Base" , "BalanceRatioSelector" , NULL , NULL , false } ,
{ FGP_IRIS , "Iris" , NULL , NULL , NULL , false } ,
{ FGP_FOCUS , "Focus" , NULL , NULL , NULL , false } ,
{ FGP_ZOOM , "Zoom" , NULL , NULL , NULL , false } ,
{ FGP_PAN , "Pan" , NULL , NULL , NULL , false } ,
{ FGP_TILT , "Tilt" , NULL , NULL , NULL , false } ,
{ FGP_GRAB , "Grab" , "AcquisitionFrameCount" , NULL , NULL , false } ,
{ FGP_LINE_SELECT , "LineSelect" , "LineSelector" , NULL , NULL , false } ,
{ FGP_LINE_PARAMS , "LineParams" , NULL , NULL , NULL , false } ,
{ FGP_LINE_SOURCE , "LineSource" , "LineSource" , NULL , NULL , false } ,
{ FGP_LINE_INVERSE , "LineInverter" , "LineInverter" , NULL , NULL , false } ,
{ FGP_LINE_DEBOUNCE , "Debouncing" , "LineDebounceTime" , NULL , NULL , false } ,
{ FGP_USER_SET_SELECT , "SaveLoad" , "UserSetSave" , "UserSetLoad" , NULL , true } , // UI, SaveCommand, LoadCommand
// the next is not for UI, only info for operations with sets
{ FGP_USER_SET_DEF , "UserSetSelector" , "UserSetMakeDefault" , NULL , NULL , true } , // Name for selector, Command for set default
{ FGP_LOG , "Log" , NULL , NULL , NULL , false }
};

Videoformats vFormats[] =
{
  { ibpfMono8 , "Mono8" , true , 1. , CM_Y8 , BI_Y8 } ,
{ ibpfMono16 , "Mono16" , true , 2.0 , CM_Y16 , BI_Y16 } ,
{ ibpfMono12 , "Mono12" , true , 2.0 , CM_UNKNOWN , BI_Y12 } ,
{ ibpfMono14 , "Mono14" , true , 2.0 , CM_UNKNOWN , BI_Y12 } ,
{ ibpfMono12Packed_V1 , "Mono12Packed" , true , 1.5 , CM_UNKNOWN , BI_YP12 } ,
{ ibpfYUV411_UYYVYY_Packed , "YUV411" , true , 1.5 , CM_YUV411 , BI_YUV9 } ,
{ ibpfYUV422Packed , "YUV422" , false , 2.0 , CM_YUV422 , BI_YUV12 } ,
{ ibpfYUV444Packed , "YUV444" , false , 3.0 , CM_YUV444 , BI_YUV12 } ,
{ ibpfRGBx888Packed , "RGB24" , false , 3.0 , CM_UNKNOWN , BI_YUV12 } ,
{ ibpfRGB161616Packed , "RGB48" , false , 6.0 , CM_UNKNOWN , BI_YUV12 } ,
{ ibpfBGR888Packed , "BGR24" , false , 3.0 , CM_UNKNOWN , BI_YUV12 } ,
//   { VmbPixelFormatRgba8 , "RGBA32" , false , 4.0 , CM_UNKNOWN , BI_YUV12 } ,
//   { VmbPixelFormatBgra8 , "BGRA32" , false , 4.0 , CM_UNKNOWN , BI_YUV12 } ,
//   { VmbPixelFormatMono12Packed , "Mono12Packed" , true , 1.5 , CM_UNKNOWN , BI_Y16 } ,
//   { VmbPixelFormatBayerGR12Packed , "Bayer12Packed" , false , 1.5 , CM_UNKNOWN , BI_Y16 } ,
//   { VmbPixelFormatBayerGR16 , "RAW16" , true , 2.0 , CM_RAW16 , BI_Y16 },
{ ibpfRaw , "RAW8" , true , 1.0 , CM_RAW8 , BI_Y8 }
};



FXLockObject                MatVis::m_ConfigLock;
DeviceManager*              MatVis::m_pDeviceManager = NULL;
int                         MatVis::m_iNGadgets = 0;
MatVisCamInfo               MatVis::m_CamInfo[ MAX_CAMERASNMB ];
int                         MatVis::m_iCamNum = 0;
FXArray<BusyCamera>         MatVis::m_BusyCameras;
DWORD                       MatVis::m_dwInstanceCount = 0;
bool                        MatVis::m_bSaveFullInfo = true;
#ifdef _DEBUG
DWORD MatVis::m_dwDefaultTimeout = 60000;
#else
DWORD MatVis::m_dwDefaultTimeout = 1000;
#endif

int GetPropertyDictionary( const mvIMPACT::acquire::Property& p , FXStringArray& DictAsStrings )
{
  if ( !p.hasDict() )
    return 0;

  mvIMPACT::acquire::TComponentType type = p.type();
  switch ( type )
  {
  case ctPropInt:
    {
      PropertyI pI( p );
      std::vector<std::pair<std::string , int> > dict;
      pI.getTranslationDict( dict );
      for ( auto iter = dict.begin(); iter != dict.end(); iter++ )
        DictAsStrings.Add( iter->first.c_str() );
    }
    break;
  case ctPropInt64:
    {
      PropertyI64 pI64( p );
      std::vector<std::pair<std::string , __int64> > dict;
      pI64.getTranslationDict( dict );
      for ( auto iter = dict.begin(); iter != dict.end(); iter++ )
        DictAsStrings.Add( iter->first.c_str() );
    }
    break;
  case ctPropFloat:
    {
      PropertyF pF( p );
      std::vector<std::pair<std::string , double> > dict;
      pF.getTranslationDict( dict );
      for ( auto iter = dict.begin(); iter != dict.end(); iter++ )
        DictAsStrings.Add( iter->first.c_str() );
    }
    break;
  default: return 0;
  }
  return (int) DictAsStrings.GetCount();
}






struct ThreadParameter
  //-----------------------------------------------------------------------------
{
  MatVis *             pGadget;
  ThreadParameter( MatVis* p ) : pGadget( p ) {}
};

//-----------------------------------------------------------------------------
unsigned int __stdcall GrabThread( void* pData )
//-----------------------------------------------------------------------------
{
  //   ThreadParameter* pThreadParameter = reinterpret_cast< ThreadParameter* >( pData );

  MatVis* pGadget = (MatVis*) pData; // pThreadParameter->pGadget ;
  if ( !pGadget )
  {
    FxSendLogMsg( MSG_ERROR_LEVEL , "MatVis" , 0 ,
      _T( "Zero Gadget Pointer" ) );

    return -2;
  }

  // Set thread name for debugging
  FXString ThreadName;
  pGadget->GetGadgetName( ThreadName );
  ThreadName += "_Grab";
  SetCurrentThreadName( (LPCSTR) ThreadName );

  unsigned int cnt = 0;


  // Send all requests to the capture queue. There can be more than 1 queue for some devices, but for this sample
  // we will work with the default capture queue. If a device supports more than one capture or result
  // queue, this will be stated in the manual. If nothing is mentioned about it, the device supports one
  // queue only. This loop will send all requests currently available to the driver. To modify the number of requests
  // use the property mvIMPACT::acquire::SystemSettings::requestCount at runtime or the property
  // mvIMPACT::acquire::Device::defaultRequestCount BEFORE opening the device.
  TDMR_ERROR result = DMR_NO_ERROR;
  while ( !pGadget->IsRun() && pGadget->m_pDevice )
  {
    Sleep( 10 );
  }
  if ( !pGadget->IsRun() )
  {
    return -1;
  }
  FunctionInterface fi( pGadget->m_pDevice );
  pGadget->m_pFuncInt = &fi ;
  int iReqCnt = 0;

  result = static_cast<TDMR_ERROR>(fi.imageRequestSingle());
  if ( result != DEV_NO_FREE_REQUEST_AVAILABLE
    && result != DMR_NO_ERROR )
  {
    FxSendLogMsg( MSG_ERROR_LEVEL , pGadget->GetGadgetInfo() , 0 ,
      _T( "imageRequestSingle ERROR: %s (%u-0x%X)" ) ,
      ImpactAcquireException::getErrorCodeAsString( result ).c_str() ,
      result , result );
  }

  mvIMPACT::acquire::Request* pRequest = NULL;
  mvIMPACT::acquire::Request* pPreviousRequest = NULL;
  const unsigned int timeout_ms = 500;
  int iNGrabbed = 0;
  int iNShownStartErrors = 0;
  int iNShownRequestErrors = 0;
  int iNOrdered = pGadget->m_iNFramesForGrabbing;
  int requestNr = 0;
  ImageBuffer * pIm = NULL;
  bool bDoStart = true;
  pGadget->m_bIsStopped = false ;
  while ( pGadget->IsRun() && pGadget->m_pDevice )
  {
    if ( bDoStart )
    {
      try
      {
        fi.acquisitionStop() ;
        pGadget->SetNFramesAndAcquisitionMode( iNOrdered );
        if ( pGadget->m_pDevice->acquisitionStartStopBehaviour.read() == mvIMPACT::acquire::assbUser )
        {
          const mvIMPACT::acquire::TDMR_ERROR result =
            static_cast<mvIMPACT::acquire::TDMR_ERROR>(fi.acquisitionStart());

          if ( result != DMR_NO_ERROR )
          {
            FXAutolock al( pGadget->m_GrabLock , "MVGrab_NotStarted" );
            FxSendLogMsg( MSG_ERROR_LEVEL , pGadget->GetGadgetInfo() , 0 ,
              _T( "acquisitionStart ERROR: %s(%u-0x%X)" ) ,
              ImpactAcquireException::getErrorCodeAsString( result ).c_str() ,
              result , result );
            continue;
          }
          else
            bDoStart = false;
        }
      }
      catch ( const ImpactAcquireException& e )
      {
        {
          FXAutolock al( pGadget->m_GrabLock , "MVGrab_StartExcept" );
          // this e.g. might happen if the same device is already opened in another process...
          FxSendLogMsg( MSG_ERROR_LEVEL , pGadget->GetGadgetInfo() , 0 ,
            _T( "acquisitionStart exception: %s" ) , e.getErrorCodeAsString().c_str() );
        }
        Sleep( 100 );
        continue;
      }
    }
    else
    {
      if ( iNOrdered == 0 )
      {
        DWORD dwWaitRes = WaitForSingleObject( pGadget->m_hevGrabEvt , INFINITE );
        if ( dwWaitRes == WAIT_OBJECT_0 )  // NGrab changed
        {
          iNOrdered = pGadget->m_iNFramesForGrabbing;
          if ( iNOrdered != -1 )
          {
            do
            {
              try
              {
                requestNr = fi.imageRequestWaitFor( 1 );
                if ( fi.isRequestNrValid( requestNr ) )
                {
                  pRequest = fi.getRequest( requestNr );
                  pRequest->unlock();
                  fi.imageRequestSingle();
                }
                else
                  break;
              }
              catch ( const ImpactAcquireException& e )
              {
                LPCTSTR pExceptAsString = e.getErrorCodeAsString().c_str();
              }
            } while ( pRequest );
          }
          if ( pGadget->SetNFramesAndAcquisitionMode( iNOrdered ) )
            // N frames is written in SetGrab function                  
          {
            bDoStart = (iNOrdered != 0);
          }
        }
        else
          continue;
      }
      else
      {
        DWORD dwWaitRes = WaitForSingleObject( pGadget->m_hevGrabEvt , 0 );
        if ( dwWaitRes == WAIT_OBJECT_0 )
        {
          fi.acquisitionStop();
          if ( pGadget->SetNFramesAndAcquisitionMode(
            iNOrdered = pGadget->m_iNFramesForGrabbing ) )
            // N frames is written in SetGrab function                  
          {
            bDoStart = (iNOrdered != 0);
          }
          continue;
        }
      }
    }
    // wait for results from the default capture queue
    try
    {
      requestNr = fi.imageRequestWaitFor( timeout_ms );
      pRequest = fi.isRequestNrValid( requestNr ) ?
        fi.getRequest( requestNr ) : 0;
    }
    catch ( const ImpactAcquireException& e )
    {
      {
        FXAutolock al( pGadget->m_GrabLock , "MVGrab_RequestException" );
        FxSendLogMsg( MSG_ERROR_LEVEL , pGadget->GetGadgetInfo() , 0 ,
          _T( "imageRequestWaitFor exception: %s" ) , e.getErrorCodeAsString().c_str() );
      }
      Sleep( 100 );
      continue;
    }
    if ( pRequest )
    {
      if ( pRequest->isOK() )
      {
        bDoStart = false;
        ++cnt;

        try
        {
          const ImageBufferDesc& Descriptor = pRequest->getImageBufferDesc();
          pIm = Descriptor.getBuffer();
        }
        catch ( const ImpactAcquireException& e )
        {
          {
            FXAutolock al( pGadget->m_GrabLock , "MVGrab_Exception" );
            FxSendLogMsg( MSG_ERROR_LEVEL , pGadget->GetGadgetInfo() , 0 ,
              _T( "Image Descriptor handling exception: %s" ) ,
              e.getErrorCodeAsString().c_str() );

          }         Sleep( 100 );
          continue;
        }
        if ( pIm )
        {
          CVideoFrame * pNewFrame = pGadget->ConvertMVtoSHformat( pIm );
          if ( pNewFrame )
          {
            pGadget->m_GrabLock.Lock( INFINITE , "MVGrab_NewFrame" );
            pGadget->m_ReadyFrames.push( pNewFrame );
            pGadget->m_GrabLock.Unlock();
            SetEvent( pGadget->m_evFrameReady );
            ++iNGrabbed;
            if ( pGadget->m_iNFramesForGrabbing > 0 )
            {
              if ( iNGrabbed >= iNOrdered )
              {
                DWORD dwWaitRes = WaitForSingleObject( pGadget->m_hevGrabEvt , INFINITE );
                if ( dwWaitRes == WAIT_OBJECT_0 )  // NGrab changed
                {
                  if ( pGadget->SetNFramesAndAcquisitionMode(
                    iNOrdered = pGadget->m_iNFramesForGrabbing ) )
                    // N frames is written in SetGrab function                  
                  {
                    bDoStart = (iNOrdered != 0);
                  }
                }
              }
            }
            else
            {
              // check, if something is changed
              DWORD dwWaitRes = WaitForSingleObject( pGadget->m_hevGrabEvt , 0 );
              if ( dwWaitRes == WAIT_OBJECT_0 )
              {
                fi.acquisitionStop();
                if ( pGadget->SetNFramesAndAcquisitionMode(
                  iNOrdered = pGadget->m_iNFramesForGrabbing ) )
                  // N frames is written in SetGrab function                  
                {
                  bDoStart = (iNOrdered != 0);
                }
              }
            }
          }
        }
      }
      else
      {
        FXAutolock al( pGadget->m_GrabLock , "MVGrab_Error" );
        FxSendLogMsg( MSG_ERROR_LEVEL , pGadget->GetGadgetInfo() , 0 ,
          _T( "Image Request ERROR: %s" ) , pRequest->requestResult.readS().c_str() );
      }
      if ( pPreviousRequest )
      {
        // this image has been used thus the buffer is no longer needed...
        pPreviousRequest->unlock();
      }
      pPreviousRequest = pRequest;
      // send a new image request into the capture queue
      fi.imageRequestSingle();
    }
    else
    {
      FXAutolock al( pGadget->m_GrabLock , "MVGrab_Failed" );
      // If the error code is -2119(DEV_WAIT_FOR_REQUEST_FAILED), the documentation will provide
      // additional information under TDMR_ERROR in the interface reference
      FxSendLogMsg( MSG_ERROR_LEVEL , pGadget->GetGadgetInfo() , 0 ,
        _T( "imageRequestWaitFor(%d) failed (%s)" ) , requestNr ,
        ImpactAcquireException::getErrorCodeAsString( requestNr ).c_str() );
      fi.acquisitionStop();
      fi.imageRequestReset( 0 , 0 );

      result = static_cast<TDMR_ERROR>(fi.imageRequestSingle());
      if ( result != DEV_NO_FREE_REQUEST_AVAILABLE
        && result != DMR_NO_ERROR )
      {
        FxSendLogMsg( MSG_ERROR_LEVEL , pGadget->GetGadgetInfo() , 0 ,
          _T( "imageRequestSingle(2) ERROR: %s (%u-0x%X)" ) ,
          ImpactAcquireException::getErrorCodeAsString( result ).c_str() ,
          result , result );
      }
      bDoStart = true;
    }
  }
  fi.acquisitionStop();
  pGadget->m_bIsStopped = true ;

  // free the last potentially locked request
  if ( pRequest )
    pRequest->unlock();
  // clear all queues
  fi.imageRequestReset( 0 , 0 );
  pGadget->m_pFuncInt = NULL ;
  pGadget->m_uiGrabThreadId = 0 ;
  pGadget->m_hGrabThreadHandle = NULL ;
  TRACE( "Exit from Grab thread for %s" , pGadget->GetGadgetInfo() ) ;
  return 0;
}

//-----------------------------------------------------------------------------
void populatePropertyMap( StringPropMap& m ,
  ComponentIterator it , const string& currentPath = "" )
  //-----------------------------------------------------------------------------
{
  while ( it.isValid() )
  {
    string fullName( currentPath );
    if ( fullName != "" )
    {
      fullName += "/";
    }
    fullName += it.name();
    if ( it.isList() )
    {
      populatePropertyMap( m , it.firstChild() , fullName );
    }
    else if ( it.isProp() )
    {
      m.insert( make_pair( fullName , Property( it ) ) );
    }
    ++it;
    // method object will be ignored...
  }
}

int MatVis::PopulateDevicePropertyMap( Device * pDev , StringPropMap& map )
{
  // obtain all the settings related properties available for this device
  // Only work with the 'Base' setting. For more information please refer to the manual (working with settings)
  DeviceComponentLocator locator( pDev , dltSetting , "Base" );
  populatePropertyMap( map , ComponentIterator( locator.searchbase_id() ).firstChild() );
  try
  {
    // this category is not supported by every device, thus we can expect an exception if this feature is missing
    locator = DeviceComponentLocator( pDev , dltIOSubSystem );
    populatePropertyMap( map , ComponentIterator( locator.searchbase_id() ).firstChild() );
  }
  catch ( const ImpactAcquireException& ) {}
  locator = DeviceComponentLocator( pDev , dltRequest );
  populatePropertyMap( map , ComponentIterator( locator.searchbase_id() ).firstChild() );
  locator = DeviceComponentLocator( pDev , dltSystemSettings );
  populatePropertyMap( map , ComponentIterator( locator.searchbase_id() ).firstChild() , string( "SystemSettings" ) );
  locator = DeviceComponentLocator( pDev , dltInfo );
  populatePropertyMap( map , ComponentIterator( locator.searchbase_id() ).firstChild() , string( "Info" ) );
  populatePropertyMap( map , ComponentIterator( pDev->hDev() ).firstChild() , string( "Device" ) );
  return (int) m_PropertyMap.size();
}

int MatVis::GetPropertyIndex( LPCTSTR name )
{
  int i;
  unsigned retV = WRONG_PROPERTY;
  for ( i = 0; i < m_PropertiesEx.GetCount(); i++ )
  {
    FXString Prop = m_PropertiesEx[ i ].m_Name;
    if ( _stricmp( m_PropertiesEx[ i ].m_Name , name ) == 0 )
      return i;
  }
  return WRONG_PROPERTY;
}

int MatVis::GetInCameraPropertyIndex( LPCTSTR name )
{
  int i;
  unsigned retV = WRONG_PROPERTY;
  for ( i = 0; i < m_PropertiesEx.GetCount(); i++ )
  {
    FXString Prop = m_PropertiesEx[ i ].m_Name;
    if ( _stricmp( m_PropertiesEx[ i ].m_CameraPropertyName , name ) == 0 )
      return i;
  }
  return WRONG_PROPERTY;
}

FG_PARAMETER MatVis::GetPropertyID( LPCTSTR name )
{
  int i;
  FG_PARAMETER retV = FGP_WRONG;
  for ( i = 0; i < m_PropertiesEx.GetCount(); i++ )
  {
    FXString Prop = m_PropertiesEx[ i ].m_Name;
    if ( _stricmp( m_PropertiesEx[ i ].m_Name , name ) == 0 )
      return m_PropertiesEx[ i ].pr;
  }
  return FGP_WRONG;
}


int MatVis::GetPropertyIndex( FG_PARAMETER id )
{
  int i;
  unsigned retV = WRONG_PROPERTY;
  for ( i = 0; i < m_PropertiesEx.GetCount(); i++ )
  {
    if ( m_PropertiesEx[ i ].pr == id )
      return i;
  }
  return WRONG_PROPERTY;
}

bool MatVis::GetULongFeature( LPCTSTR pFeatureName , DWORD& ulValue )
{
  SetCamPropertyData Data;
  if ( MatVisGetCameraPropertyEx( pFeatureName , &Data ) )
  {
    if ( Data.m_Type == ctPropInt )
    {
      ulValue = Data.m_int;
      return true;
    }
    if ( Data.m_Type == ctPropInt64 )
    {
      ulValue = (ULONG) Data.m_int64;
      return true;
    }
  }
  return false;
}

Component MatVis::FindInList(
  ComponentIterator iter , LPCTSTR pName , const string& path )
{
  while ( iter.isValid() )
  {
    if ( iter.isVisible() )
    {
      if ( iter.isList() )
      {
        Component Found = FindInList( iter.firstChild() , pName , path + iter.name() + "/" );
        if ( Found )
          return Found;
      }
      else if ( iter.isProp() )
      {
        if ( iter.name() == pName )
        {
          return Component( iter );
        }
      }
    }
    ++iter;
  }
  return Component();
}

Property MatVis::GetPropertyByName( LPCTSTR pName )
{
  if ( !m_pDevice )
  {
    SEND_DEVICE_ERR( "No connected device" );
    return Property();
  }
  if ( !m_PropertyMap.size() )
  {
    PopulateDevicePropertyMap( m_pDevice , m_PropertyMap );
    if ( !m_PropertyMap.size() )
    {
      SEND_DEVICE_ERR( "Can't take components list" );
      return Property();
    }
  }
  auto Found = m_PropertyMap.find( std::string( pName ) );
  if ( Found != m_PropertyMap.end() )
    return Found->second;
  else
    return Property();
}

bool MatVis::GetPropertyValue( LPCTSTR Name , SetCamPropertyData& Value )
{
  Property Found = GetPropertyByName( Name );
  if ( !Found.isValid() )
  {
    if ( m_bViewErrorMessagesOnGetSet )
    {
      SEND_DEVICE_ERR( "Property %s is not found" , Name );
    }
    return false;
  }
  Value.m_Type = Found.type();
  switch ( Found.type() )
  {
  case ctPropInt64:
    Value.m_int64 = PropertyI64( Found.hObj() ).read();
    Value.m_int = (int) Value.m_int64;
    return true;
  case ctPropInt:
    Value.m_int64 = Value.m_int = PropertyI( Found.hObj() ).read(); return true;
  case ctPropFloat: Value.m_double = PropertyF( Found.hObj() ).read(); return true;
  case ctPropString: strcpy( Value.m_szString , PropertyS( Found.hObj() ).read().c_str() ); return true;
  default:
    SEND_DEVICE_ERR( "Unsupported property type %d for %s" , Value.m_Type , Name );
    return false;
  }
  return false;
}

bool MatVis::SetPropertyValue( LPCTSTR Name , SetCamPropertyData& Value )
{
  Property Found = GetPropertyByName( Name );
  if ( !Found.isValid() )
  {
    if ( m_bViewErrorMessagesOnGetSet )
    {
      SEND_DEVICE_ERR( "Property %s is not found" , Name );
    }
    return false;
  }
  //   PropertyPtr Prop( Found.hObj() ) ;
  switch ( Found.type() )
  {
  case ctPropInt64: 
    {
      if ( Value.m_szString[0] == 0 )
        PropertyI64( Found.hObj() ).write( Value.m_int64 );
      else
        PropertyI64( Found.hObj() ).writeS( Value.m_szString );

      return true;
    }
  case ctPropInt: PropertyI( Found.hObj() ).write( Value.m_int ); return true;
  case ctPropFloat: PropertyF( Found.hObj() ).write( Value.m_double ); return true;
  case ctPropString: PropertyS( Found.hObj() ).writeS( std::string( Value.m_szString ) ); return true;
  default:
    SEND_DEVICE_ERR( "Unsupported property type %d for %s" , Value.m_Type , Name );
    return false;
  }
  return false;
}

bool MatVis::RunCameraCommand( LPCTSTR pCommand )
{
  SetCamPropertyData DummyData;
  return SetPropertyValue( pCommand , DummyData );
}

bool MatVis::SetULongFeature( LPCTSTR pFeatureName , DWORD ulValue )
{
  SetCamPropertyData Value;
  Value.m_int64 = Value.m_int = (int) ulValue;
  Value.m_Type = ctPropInt;

  return SetPropertyValue( pFeatureName , Value );
}

bool MatVis::GetFeatureIntValue( LPCTSTR pFeatureName , __int64 & value )
{
  SetCamPropertyData Value;
  bool bRes = GetPropertyValue( pFeatureName , Value );
  if ( bRes )
  {
    if ( Value.m_Type == ctPropInt )
    {
      value = Value.m_int;
      return true;
    }
    else if ( Value.m_Type == ctPropInt64 )
    {
      value = Value.m_int64;
      return true;
    }
    else
      SEND_DEVICE_ERR( "Property %s is not int %d " , pFeatureName , Value.m_Type );
  }
  return false;
}
/** write an integer feature from camera.
*/
bool MatVis::SetFeatureIntValue( LPCTSTR pFeatureName , __int64 value )
{
  SetCamPropertyData Value;
  Value.m_int64 = value;
  Value.m_Type = ctPropInt64;

  return SetPropertyValue( pFeatureName , Value );
}

bool MatVis::SetFeatureValueAsEnumeratorString( LPCTSTR pFeatureName , LPCTSTR pValue )
{
  SetCamPropertyData Value;
  strcpy_s( Value.m_szString , pValue );
  Value.m_Type = ctPropInt64;

  return SetPropertyValue( pFeatureName , Value );
}

DWORD MatVis::GetXSize()
{
  ULONG ulVal;
  if ( GetULongFeature( _T( "Camera/GenICam/ImageFormatControl/Width" ) , ulVal ) )
    return ulVal;
  return 0;
}
void MatVis::SetWidth( DWORD Width )
{
  SetULongFeature( _T( "Camera/GenICam/ImageFormatControl/Width" ) , Width );
}

DWORD MatVis::GetYSize()
{
  ULONG ulVal;
  if ( GetULongFeature( _T( "Camera/GenICam/ImageFormatControl/Height" ) , ulVal ) )
    return ulVal;
  return 0;
}
void MatVis::SetHeight( DWORD Height )
{
  SetULongFeature( _T( "Camera/GenICam/ImageFormatControl/Height" ) , Height );
}

DWORD MatVis::GetXOffset()
{
  DWORD lValue;
  if ( GetULongFeature( "Camera/GenICam/ImageFormatControl/OffsetX" , lValue ) )
    return lValue;
  return 0;
}
void MatVis::SetXOffset( DWORD XOffset )
{
  SetULongFeature( "Camera/GenICam/ImageFormatControl/OffsetX" , XOffset );
}

DWORD MatVis::GetYOffset()
{
  DWORD lValue;
  if ( GetULongFeature( "Camera/GenICam/ImageFormatControl/OffsetY" , lValue ) )
    return lValue;
  return 0;
}
void MatVis::SetYOffset( DWORD YOffset )
{
  SetULongFeature( "Camera/GenICam/ImageFormatControl/OffsetY" , YOffset );
}

DWORD MatVis::GetSensorWidth()
{
  DWORD lValue;
  if ( GetULongFeature( "SensorWidth" , lValue ) )
    return lValue;
  return 0;
}

DWORD MatVis::GetSensorHeight()
{
  DWORD lValue;

  if ( GetULongFeature( "SensorHeight" , lValue ) )
    return lValue;
  return 0;
}
DWORD MatVis::GetMaxWidth()
{
  DWORD lValue;
  if ( GetULongFeature( "WidthMax" , lValue ) )
    return lValue;
  return 0;
}

DWORD MatVis::GetMaxHeight()
{
  DWORD lValue;

  if ( GetULongFeature( "HeightMax" , lValue ) )
    return lValue;
  return 0;
}

CVideoFrame * MatVis::ConvertMVtoSHformat( const ImageBuffer * pFrame )
{
  double dStart = GetHRTickCount();
  //   double dGetTime , dToRGBTime , dToYUV9Time ;
  CVideoFrame * pOut = NULL;

  TImageBufferPixelFormat tImFormat = pFrame->pixelFormat;

  DWORD Lx = pFrame->iWidth;
  DWORD Ly = pFrame->iHeight;
  unsigned iSize = Lx * Ly;
  LPBYTE data = (LPBYTE) pFrame->vpData;
  switch ( tImFormat )
  {
  case ibpfMono8:
    {
      LPBITMAPINFOHEADER lpBMIH = (LPBITMAPINFOHEADER) malloc( sizeof( BITMAPINFOHEADER ) + iSize );

      memcpy( lpBMIH , &m_BMIH , sizeof( BITMAPINFOHEADER ) );
      lpBMIH->biWidth = Lx;
      lpBMIH->biHeight = Ly;
      lpBMIH->biSizeImage = iSize;
      lpBMIH->biCompression = BI_Y8;
      memcpy( &lpBMIH[ 1 ] , data , iSize );
      CVideoFrame* vf = CVideoFrame::Create();
      vf->lpBMIH = lpBMIH;
      vf->lpData = NULL;
      vf->SetLabel( m_CameraID );
      vf->SetTime( GetHRTickCount() );
      pOut = vf;
    }
    break;
  case ibpfMono10:
  case ibpfMono12:
  case ibpfMono14:
  case ibpfMono16:
    {
      iSize *= 2;
      LPBITMAPINFOHEADER lpBMIH = (LPBITMAPINFOHEADER) malloc( sizeof( BITMAPINFOHEADER ) + iSize );
      if ( lpBMIH )
      {
        memcpy( lpBMIH , &m_BMIH , sizeof( BITMAPINFOHEADER ) );
        lpBMIH->biWidth = Lx;
        lpBMIH->biHeight = Ly;
        lpBMIH->biSizeImage = iSize;
        lpBMIH->biCompression = BI_Y16;
        memcpy( &lpBMIH[ 1 ] , data , iSize );
        CVideoFrame* vf = CVideoFrame::Create();
        vf->lpBMIH = lpBMIH;
        vf->lpData = NULL;
        vf->SetLabel( m_CameraID );
        vf->SetTime( GetHRTickCount() );
        pOut = vf;
      }
    }
    break;
  default:
    SEND_DEVICE_ERR( "Unknown Image Format 0x%0X" , tImFormat );
    break;
  }
  return pOut;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MatVis::MatVis()
  : m_iNProperties( ARR_SIZE( CommonProperties ) )
{
#ifdef _DEBUG
  m_bDebug = true ;
#else
  m_bDebug = false ;
#endif
  double dStart = GetHRTickCount();
  Sleep( 50 );

  m_pLogOutput = new COutputConnector( text );
  if ( m_pDeviceManager == NULL )
  {
    m_pDeviceManager = new DeviceManager;
  }
  //   m_pImageFormatControl = NULL ;
  //   m_pFuncInt = NULL ;
  //   m_pAcqControl = NULL ;
  m_iNGadgets++;
  //m_CurrentCameraGUID=PGRGuid();
  m_dwSerialNumber = -1;
  m_dwConnectedSerialNumber = 0;
  m_pDevice = NULL;
  m_CurrentROI = CRect( 0 , 0 , 640 , 480 );
  m_pixelFormat = ibpfMono8;
  m_uiPacketSize = 2048;
  m_FormatNotSupportedDispayed = false;
  m_GadgetInfo = "MatVis";
  m_WaitEventFrameArray[ 0 ] = m_evFrameReady = CreateEvent( NULL , FALSE , FALSE , NULL );
  m_WaitEventBusChangeArr[ 1 ] = m_WaitEventFrameArray[ 1 ] = m_evExit;
  m_WaitEventBusChangeArr[ 0 ] = m_evCameraControl = CreateEvent( NULL , FALSE , FALSE , NULL );
  m_WaitEventBusChangeArr[ 2 ] = m_evBusChange = CreateEvent( NULL , FALSE , FALSE , NULL );
  m_WaitEventBusChangeArr[ 3 ] = m_evSWTriggerPulse;
  m_evControlRequestFinished = CreateEvent( NULL , FALSE , FALSE , NULL );
  m_hevGrabEvt = CreateEvent( NULL , FALSE , FALSE , NULL );
  m_hCamAccessMutex = CreateMutex( NULL , FALSE , NULL );
  m_pCameraObserver = NULL;
  m_disableEmbeddedTimeStamp = false;
  //m_isSelectingNewCamera = false ;
  m_dLastStartTime = 0.;
  m_pNewFrame = NULL;
  m_hCameraControlThreadHandle = NULL;
  m_dwCameraControlThreadId = 0;
  m_bContinueCameraControl = false;
  m_BusEvents = 0;
  m_bCamerasEnumerated = false;
  m_CameraStatus = NotInitialized;
  m_bInitialized = false;
  m_bStopInitialized = false;
  m_bRescanCameras = true;
  m_iNNoSerNumberErrors = 0;
  m_iWBRed = m_iWBlue = 512;
  m_bLocalStopped = FALSE;
  m_TriggerMode = TrigNotSupported;
  m_dLastInCallbackTime = 0.;
  m_dwNArrivedEvents = 0;
  m_iFPSx10 = 0;
  m_iSelectedLineNumber = 0;
  m_GadgetInfo = _T( "MatVis" );
  m_dLastBuiltPropertyTime = 0.;
  m_pOrigProperties = CommonProperties;
  m_bViewErrorMessagesOnGetSet = true;
  m_dExtShutter = 0.;
  m_dLogPeriod_ms = 0.;
  m_dLastLogTime_ms = 0.;
  m_iLogCnt = 0;
  m_iNTemperatures = 0;
  m_bSoftwareTriggerMode = false;
  m_GrabMode = GM_Unknown;
  m_iNFramesForGrabbing = -1;
  m_iCurrentBinningOrDecimation = 1 ;
  m_pFuncInt = NULL ;
  m_bInScanSettings = false ;
  memset( m_dTemperatures , 0 , sizeof( m_dTemperatures ) );

  memset( &m_BMIH , 0 , sizeof( BITMAPINFOHEADER ) );
  memset( &m_RealBMIH , 0 , sizeof( BITMAPINFOHEADER ) );
  m_uiGrabThreadId = 0 ;
  m_hGrabThreadHandle = NULL ;
#ifdef _DEBUG
  m_iMutexTakeCntr = 0;
#endif
  //DriverInit() ;
  double dBusyTime = GetHRTickCount() - dStart;
  TRACE( "\nMatVis::MatVis: Start %g , Busy %g" , dStart , dBusyTime );
}

MatVis::~MatVis()
{
  if ( m_evControlRequestFinished )
  {
    CloseHandle( m_evControlRequestFinished );
    m_hCameraControlThreadHandle = NULL;
  }
  if ( m_hCameraControlThreadHandle )
  {
    CloseHandle( m_hCameraControlThreadHandle );
    m_hCameraControlThreadHandle = NULL;
  }

  if ( --m_iNGadgets <= 0 )
  {
    delete m_pDeviceManager;
    m_pDeviceManager = NULL;
  }
}

bool MatVis::DriverInit()
{
  bool bInitResult = OtherThreadDriverInit();
  if ( bInitResult && !m_bContinueCameraControl )
  {
    FXString ThreadName;
    m_hCameraControlThreadHandle = CreateThread( NULL , 0 ,
      CameraControlLoop , this , CREATE_SUSPENDED , &m_dwCameraControlThreadId );
    if ( m_hCameraControlThreadHandle )
    {
      //       FXString ThreadName ;
      //       GetGadgetName( ThreadName ) ;
      //       ThreadName += "_CamCNTRL" ;
      //       ::SetThreadName( (LPCSTR)ThreadName , m_dwCameraControlThreadId ) ;
      m_bContinueCameraControl = true;
      ResumeThread( m_hCameraControlThreadHandle );
      Sleep( 50 );
    }
    else
    {
      C1394_SENDERR_2( "%s: %s" , (LPCTSTR) ThreadName , _T( "Can't start thread" ) );
      m_bInScanProperties = false;
      return false;
    }
  }
  return bInitResult;
}

bool MatVis::MatVisDriverInit()
{
  return EnumCameras();
}

void MatVis::SaveCameraInfo( Device * pCamera , int iIndex )
{
  m_CamInfo[ iIndex ].m_ModelName = pCamera->product.read().c_str();
  m_CamInfo[ iIndex ].m_sSN = pCamera->serial.read();
  FXString AsText = m_CamInfo[ iIndex ].m_sSN.c_str();
  while ( AsText.GetLength() && !isdigit( AsText[ 0 ] ) )
    AsText.Delete( 0 );
  m_CamInfo[ iIndex ].m_dwSN = atoi( (LPCTSTR) AsText );
  m_CamInfo[ iIndex ].m_Id = pCamera->deviceID.read();
}
//bool bSaveFullInfo = false ;
bool MatVis::EnumCameras()
{
  double dStart = GetHRTickCount();

  FXAutolock al( m_ConfigLock );
  if ( m_bCamerasEnumerated && !m_bRescanCameras )
    return true;
  DWORD dwNDevices = m_pDeviceManager->deviceCount();
  if ( !dwNDevices )
  {
    SEND_DEVICE_ERR( "There are no cameras" );
    m_CameraStatus = CantGetCameras;
    return false;
  }

  m_iCamNum = m_CamerasOnBus = dwNDevices;
  FILE * pPropertyFile = NULL;
  //      std::for_each( m_Cameras.begin(), m_Cameras.end(), PrintCameraInfo );
  for ( unsigned i = 0; i < m_CamerasOnBus; i++ )
  {
    double dCameraStart = GetHRTickCount();

    Device * pCam = m_pDeviceManager->getDevice( i );
    std::string InitialLayout = pCam->interfaceLayout.readS();
    if ( pCam->interfaceLayout.isWriteable() )
    {
      pCam->interfaceLayout.writeS( "GenICam" );
    }
    else
    {
      if ( InitialLayout != _T( "GenICam" ) )
        continue;
    }
    m_InterfaceLayout = pCam->interfaceLayout.readS();
    try
    {
      pCam->open();
    }
    catch ( const ImpactAcquireException& e )
    {
      // this e.g. might happen if the same device is already opened in another process...
      FXString Msg;
      Msg.Format( _T( "Open Device %s ERROR %s" ) ,
        pCam->serial.read().c_str() , e.getErrorCodeAsString().c_str() );
      SEND_DEVICE_ERR( Msg );
      continue;
    }

    SaveCameraInfo( pCam , i );
    double dAfterSave = GetHRTickCount();
    TRACE( "\n   MatVis EnumCameras: Save %d info time is %g" , i , dAfterSave - dCameraStart );
    if ( m_bSaveFullInfo )
    {
      ComponentList Features;
      try
      {
        Features = pCam->deviceDriverFeatureList();
      }
      catch ( EInvalidParameterList* e )
      {
        FXString Msg;
        Msg.Format( _T( "Get Param List for Device %s ERROR %s" ) ,
          pCam->serial.read().c_str() , e->getErrorCodeAsString().c_str() );
        SEND_DEVICE_ERR( Msg );
        m_CameraStatus = CantGetFeatures;
        pCam->close() ;
        continue;
      }
      errno_t err = 0;
      if ( pPropertyFile == NULL )
      {
        time_t CurTime;
        time( &CurTime );

        FXString FileName;
        FileName.Format( _T( "MVCamProps%d.txt" ) , (int) CurTime );
        err = fopen_s( &pPropertyFile , (LPCTSTR) FileName , "wb" );
        ASSERT( err == 0 );
      }
      FXString Out;
      double dAfterFileOpening = GetHRTickCount();
      TRACE( "\n   MV EnumCameras: Open camera %s(%d) time is %g" ,
        m_CamInfo[ i ].m_ModelName.c_str() , i , dAfterFileOpening - dCameraStart );
      Out.Format( "Camera SN%s-%s time=%g\n Properties: \n" ,
        m_CamInfo[ i ].m_sSN.c_str() , m_CamInfo[ i ].m_ModelName.c_str() ,
        dAfterFileOpening - dCameraStart );
      dCameraStart = dAfterFileOpening;
      if ( pPropertyFile )
        fwrite( (LPCTSTR) Out , Out.GetLength() , 1 , pPropertyFile );
      double dAfterFileWrite = GetHRTickCount();
      TRACE( "\n   MV EnumCameras: Write cam %s(%d) info time is %g" ,
        m_CamInfo[ i ].m_ModelName.c_str() , i , dAfterFileWrite - dCameraStart );
      StringPropMap PropMap;
      PopulateDevicePropertyMap( pCam , PropMap );
      DeviceComponentLocator locator( pCam , dltSetting , "Base" );
      for ( auto iter = PropMap.begin(); iter != PropMap.end(); iter++ )
      {
        Property& Prop = iter->second;
        if ( !Prop.isValid() )
          continue;

        TComponentType Type = Prop.type();

        std::string Name = iter->first /*Prop.name() */;
        FXString EnumVal( _T( "[" ) );
        CameraAttribute NewAttribute( FGP_LAST , NULL , Name.c_str() );
        bool bRes = FormDialogData( Prop , NewAttribute );
        //       try
        //       {
        //         bRes = locator.bindComponent( Prop , Name ) ;
        //       }
        //       catch ( const EPropertyHandling& e )
        //       {
        //         TRACE( "\nException in comp. %s binding: %s  " , Name.c_str() ,
        //           e.getErrorString().c_str() );
        //         break ;
        //       }

        double dAfterPropertyInfoTaking = GetHRTickCount();
        int iLen = (int)NewAttribute.m_FDescription.GetLength() ;
        if ( iLen == 0 )
          bRes = false ;
        FXString Time;
        Time.Format( " T=%g\n" , dAfterPropertyInfoTaking - dAfterFileWrite );
        NewAttribute.m_FDescription += Time;
        dAfterFileWrite = dAfterPropertyInfoTaking;
        if ( !NewAttribute.m_FDescription.IsEmpty() && pPropertyFile )
        {
          iLen = (int)NewAttribute.m_FDescription.GetLength() ;
          fwrite( (LPCTSTR) NewAttribute.m_FDescription ,
            iLen , 1 , pPropertyFile );
        }
      }
    }
    pCam->close();
    TRACE( "\n   MV EnumCameras: Inspect properties of cam %d time is %g" ,
      i , GetHRTickCount() - dCameraStart );
  }

  if ( pPropertyFile )
    fclose( pPropertyFile );
  m_bSaveFullInfo = false;
  m_bCamerasEnumerated = (m_CamerasOnBus > 0);
  m_bRescanCameras = false;
  m_CameraStatus = DriverInitialized;
  if ( m_dwSerialNumber != -1 && m_dwSerialNumber != 0 )
  {
    m_GadgetInfo.Format( "MatVis_%d" , m_dwSerialNumber );
  }
  return true;
}

void MatVis::ShutDown()
{
  if ( m_hCameraControlThreadHandle )
  {
    OtherThreadCameraShutDown();
    m_bContinueCameraControl = false;
    SetEvent( m_evCameraControl );
    SetEvent( m_evFrameReady );
    Sleep( 50 );
    DWORD dwRes = WaitForSingleObject( m_hCameraControlThreadHandle , 1000 );
    // ASSERT( dwRes == WAIT_OBJECT_0 ) ;
    m_bContinueCameraControl = false;
  }

  DWORD dwRes = WaitForSingleObject( m_hCamAccessMutex , 1000 );
  ASSERT( dwRes == WAIT_OBJECT_0 );
  CloseHandle( m_hCamAccessMutex );
  m_GrabLock.Lock( INFINITE , "MVG_ShutDown" );
  while ( m_ReadyFrames.size() )
  {
    CDataFrame * pFrame = m_ReadyFrames.front();
    m_ReadyFrames.pop();
    pFrame->Release();
  };
  m_GrabLock.Unlock();
  FxReleaseHandle( m_hevGrabEvt );
  FxReleaseHandle( m_evFrameReady );
  FxReleaseHandle( m_evBusChange );
  FxReleaseHandle( m_evCameraControl );
  delete m_pLogOutput;
  m_pLogOutput = NULL;

  C1394Camera::ShutDown();
}

void MatVis::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  if ( !pParamFrame )
    return;


  CTextFrame* tf = pParamFrame->GetTextFrame( DEFAULT_LABEL );
  if ( tf )
  {
    FXParser pk = (LPCTSTR) tf->GetString();
    FXString cmd;
    FXString param;
    FXSIZE pos = 0;
    pk.GetWord( pos , cmd );

    FXAutolock al( m_SettingsLock , "AsyncTransaction" );
    if ( cmd.CompareNoCase( "list" ) == 0 )
    {
      pk.Empty();
      for ( int i = 0; i < m_PropertiesEx.GetCount(); i++ )
      {
        pk += m_PropertiesEx[ i ].m_Name;
        pk += "\r\n";
      }
    }
    else if ( (cmd.CompareNoCase( "get" ) == 0) && (pk.GetWord( pos , cmd )) )
    {
      int iIndex = GetPropertyIndex( cmd );
      if ( iIndex != WRONG_PROPERTY )
      {
        FXSIZE value;
        bool bauto;
        bool bRes = GetCameraProperty( iIndex , value , bauto );
        if ( bRes )
        {
          if ( IsDigitField( m_PropertiesEx[ iIndex ].pr ) )
          {
            if ( bauto )
              pk = "auto";
            else
              pk.Format( "%d" , value );
          }
          else
            pk = (LPCTSTR) value;
        }
        else
          pk = "error";
      }
      else
        pk = "error";
    }
    else if ( cmd.CompareNoCase( "set" ) == 0 )
      if ( pk.GetWord( pos , cmd ) )
        if ( pk.GetParamString( pos , param ) )
        {
          int iIndex = GetPropertyIndex( cmd );
          if ( iIndex != WRONG_PROPERTY )
          {
            FXSIZE value = 0;
            bool bauto = false , Invalidate = false;
            if ( param.CompareNoCase( "auto" ) == 0 )
              bauto = true;
            else if ( m_PropertiesEx[ iIndex ].m_Type == ctPropInt
              || m_PropertiesEx[ iIndex ].m_Type == ctPropInt64
              || m_PropertiesEx[ iIndex ].pr == FGP_EXTSHUTTER )
              value = atoi( param );
            else
              value = (FXSIZE) (LPCTSTR) param;
            bool bWasStopped = m_bWasStopped;
            m_bWasStopped = false;
            bool bRes = SetCameraProperty( iIndex , value , bauto , Invalidate );
            if ( !bWasStopped  && m_bWasStopped )
            {
              /*CamCNTLDoAndWait( MatVis_EVT_INIT | MatVis_EVT_START_GRAB )*/;
            }
            m_bWasStopped = bWasStopped;
            pk = (bRes) ? "OK" : "error";
          }
          else
            pk = "error";
        }
        else
        {
          pk = "List of available commands:\r\n"
            "list - return list of properties\r\n"
            "get <item name> - return current value of item\r\n"
            "set <item name>(<value>) - change an item\r\n";
        }
    CTextFrame* retV = CTextFrame::Create( pk );
    retV->ChangeId( NOSYNC_FRAME );
    if ( !m_pControl->Put( retV ) )
      retV->RELEASE( retV );

  }
  pParamFrame->Release( pParamFrame );
}

bool MatVis::CameraInit()
{
  return OtherThreadCameraInit();
}

FXString MatVis::GetCameraId( DWORD dwSerialNumber , DWORD& dwIndex )
{
  FXString Id;
  for ( DWORD i = 0; i < m_CamerasOnBus; i++ )
  {
    if ( m_CamInfo[ i ].m_dwSN == dwSerialNumber )
    {
      DWORD dwBusyCamIndex = 0;

      FXAutolock al( m_ConfigLock );
      for ( ; dwBusyCamIndex < (DWORD) m_BusyCameras.GetCount(); dwBusyCamIndex++ )
      {
        if ( m_BusyCameras[ dwBusyCamIndex ].m_dwSerialNumber == m_dwSerialNumber )
        {
          SEND_DEVICE_ERR( "\nCamera %u is busy." , m_dwSerialNumber );
          return Id;
        }
      }
      if ( dwBusyCamIndex >= (DWORD) m_BusyCameras.GetCount() )
      {                 // camera found and is not busy
        Id.Format( _T( "%d" ) , m_CamInfo[ i ].m_Id );
        dwIndex = i;
        return Id;
      }
    }
  }
  SEND_DEVICE_ERR( "\nCamera #%u is not connected" , dwSerialNumber );
  dwIndex = 0xffffffff;
  return Id;
}

bool MatVis::MatVisCameraInit()
{
  MatVisDriverInit();
  double dStart = GetHRTickCount();

  if ( m_dwConnectedSerialNumber && (m_dwConnectedSerialNumber == m_dwSerialNumber)
    && !m_bShouldBeReprogrammed )
  {
    if ( m_pDevice && m_pDevice->isOpen() )  // already connected to the same camera
      return true;
  }

  if ( m_pDevice )
    MatVisCameraClose();

  if ( !m_CamerasOnBus )
  {  // nothing to connect
    SEND_DEVICE_ERR( "Fatal error: No MatVis cameras found on a bus." );
    return false;
  }
  if ( (m_dwSerialNumber == 0) || (m_dwSerialNumber == 0xffffffff) )
  {  // nothing to connect
    SEND_DEVICE_ERR( "Fatal error: No selected camera." );
    return false;
  }

  DWORD dwIndex;
  {
    //FXAutolock al( m_ConfigLock ) ;
    FXString Id = GetCameraId( m_dwSerialNumber , dwIndex );
    if ( Id.IsEmpty() )
      return false;
    try
    {
      m_pDevice = m_pDeviceManager->getDevice( dwIndex );
    }
    catch ( EDeviceManager * e )
    {
      SEND_DEVICE_ERR( "Fatal error GetCamera SN=%u for %s: %s" ,
        m_dwSerialNumber , (LPCTSTR) Id , e->getErrorString().c_str() );
      return false;
    }



    m_iNNoSerNumberErrors = 0;

    m_Status.Empty();

    int iLastChannel = -1;
    m_CameraID.Format( "%d_%s" , m_dwSerialNumber , m_CamInfo[ dwIndex ].m_ModelName.c_str() );
    m_GadgetInfo = m_CameraID;
    //FXString GadgetName ;
    BOOL bInLoop = GetGadgetName( m_GadgetInfo );
    //   m_GadgetInfo.Format("%s: %s", GadgetName , m_CameraID );

    //   RegisterCallbacks() ;
    if ( m_Status.IsEmpty() )
    {
      if ( bInLoop )
        m_Status = m_CameraID + " is connected";
    }
    if ( !m_Status.IsEmpty() )
    {
      SEND_DEVICE_INFO( m_Status );
      m_Status.Empty();
    }
    m_dwConnectedSerialNumber = m_dwSerialNumber;
    FXAutolock al( m_ConfigLock );
    int i = 0;
    for ( ; i < m_BusyCameras.GetCount(); i++ )
    {
      if ( m_BusyCameras[ i ].m_dwSerialNumber == m_dwSerialNumber )
        break;
    }
    if ( i == m_BusyCameras.GetCount() )
    {
      BusyCamera NewBusyCamera( m_dwSerialNumber , this );
      m_BusyCameras.Add( NewBusyCamera );
    }
    m_szSerialNumber = (LPCTSTR) m_CamInfo[ dwIndex ].m_sSN.c_str();
  }

  double dConnectionTime = GetHRTickCount();
  TRACE( "\n    MatVisBuildPropertyList: Camera connection time is %g" , dConnectionTime - dStart );
  if ( m_hCameraControlThreadHandle )
  {
    FXString ThreadName;
    GetGadgetName( ThreadName );
    ThreadName += "_CNTRL";
    ::SetThreadName( (LPCSTR) ThreadName , m_dwCameraControlThreadId );
  }
  m_pOrigProperties = CommonProperties;
  m_iNProperties = ARR_SIZE( CommonProperties );

  for ( int i = 0; i < ARR_SIZE( KnownCameras ); i++ )
  {
    CamTypeAndProperties * pCamTP = KnownCameras[ i ];
    if ( !pCamTP )
      break;
    if ( m_CamInfo[ dwIndex ].m_ModelName.find( pCamTP->CamType ) >= 0 )
    {
      {
        m_pOrigProperties = pCamTP->Properties;
        m_iNProperties = pCamTP->iNProperties;
        break;
      }
    }
  };
  try
  {
    bool bInUse = m_pDevice->isInUse() ;
    bool bIsOpen = m_pDevice->isOpen() ;
//     ASSERT( !bInUse ) ;
//     ASSERT( !bIsOpen ) ;
    if ( bIsOpen )
    {
      m_pDevice->close() ;
    }
    m_pDevice->acquisitionStartStopBehaviour.write( assbUser );
    //m_pDevice->userControlledImageProcessingEnable.write( mvIMPACT::acquire::bTrue ) ;
    TDeviceInterfaceLayout Layout = (TDeviceInterfaceLayout) 0;
    std::string InitialLayout = m_pDevice->interfaceLayout.readS();
    m_pDevice->interfaceLayout.writeS( "GenICam" );
    m_InterfaceLayout = m_pDevice->interfaceLayout.readS();
    m_pDevice->open();
    if ( m_pDevice->deviceClass.read() == dcCamera )
    {
      GetGrabConditions( m_GrabMode , m_iNFramesForGrabbing );
      ImageDestination ImDest( m_pDevice );
      string OldFormat = ImDest.pixelFormat.readS();
      ImDest.pixelFormat.writeS( _T( "Mono8" ) );
      string OldFormat2 = ImDest.pixelFormat.readS();
      ImDest.pixelFormat.writeS( _T( "Auto" ) );
    }
    else
    {
      m_pDevice->close();
      return 0;
    }

    // 
    //     m_pFuncInt = new FunctionInterface( m_pDevice ) ;
    //     const std::vector< std::string > Settings = m_pFuncInt->getAvailableSettings() ;
  }
  catch ( const ImpactAcquireException& e )
  {
    LPCTSTR pErrorString = e.getErrorString().c_str() ;
    // this e.g. might happen if the same device is already opened in another process...
    FxSendLogMsg( MSG_ERROR_LEVEL , GetGadgetInfo() , 0 ,
      _T( "Device %s create interface ERROR %s" ) , GetDevice()->serial.read().c_str() ,
      e.getErrorString().c_str() );
    return 0;
  }
  //   try
  //   {
  //     m_pCougarSettings = new CameraSettingsBlueCOUGAR( m_pDevice ) ;
  //     //m_pAcqControl = new GenICam::AcquisitionControl( m_pDevice ) ;
  //   }
  //   catch ( const ImpactAcquireException& e )
  //   {
  //     // this e.g. might happen if the same device is already opened in another process...
  //     FxSendLogMsg( MSG_ERROR_LEVEL , GetGadgetInfo() , 0 ,
  //       _T( "Device %s Get Acquisition Control ERROR %s" ) , GetDevice()->serial.read().c_str() ,
  //       e.getErrorString().c_str() ) ;
  //     return 0;
  //   }
  m_PropertyMap.clear();
  PopulateDevicePropertyMap( m_pDevice , m_PropertyMap );
  auto iter = m_PropertyMap.begin();
  DeviceComponentLocator locator( m_pDevice , dltSetting , "Base" );
  for ( UINT iIndex = 0; iter != m_PropertyMap.end(); iIndex++ , iter++ )
  {
    Property Prop = iter->second;
    if ( !Prop.isValid() )
      continue;

    std::string name = iter->first;
    if ( name.find( _T( "Info/Camera" ) ) >= 0 )
    {
      size_t stSlashPos = name.rfind( _T( '/' ) );
      if ( stSlashPos >= 0 )
      {
        std::string PureName = name.erase( 0 , stSlashPos + 1 );
        if ( PureName == _T( "SensorXRes" ) )
        {
          PropertyI64 Pi64( Prop.hObj() );
          m_SensorSize.cx = (int) Pi64.read();
        }
        if ( PureName == _T( "SensorYRes" ) )
        {
          PropertyI64 Pi64( Prop.hObj() );
          m_SensorSize.cy = (int) Pi64.read();
        }
      }
    }
  }

  if ( !MatVisBuildPropertyList() )
  {
    MatVisCameraClose();
  }
  else
    m_bInitialized = true;


  TRACE( "\nMatVis::MatVisCameraInit - Camera %u initialized in %g ms" ,
    m_CameraInfo.serialnmb , GetHRTickCount() - dStart );
  return true;
}

void MatVis::CameraClose()
{
  OtherThreadCameraClose();
}

void MatVis::MatVisCameraClose()
{
  if ( !m_pDevice || m_dwConnectedSerialNumber == 0 || m_dwSerialNumber == -1 )
  {
    m_CameraStatus = CameraAlreadyClosed;
#ifdef MatVis_DEBUG
    TRACE( "\nCamera already closed SN=%u " , m_dwSerialNumber );
#endif
    return;
  }

  m_pDevice->close();
  m_pDevice = NULL;

  if ( m_dwConnectedSerialNumber != m_dwSerialNumber )
  {
    SEND_DEVICE_ERR( "CameraClose: Connected %d is not equal to Serial %" ,
      m_dwConnectedSerialNumber , m_dwSerialNumber );
  }
  int i = 0;
  FXAutolock al( m_ConfigLock );

  for ( ; i < m_BusyCameras.GetCount(); i++ )
  {
    if ( m_BusyCameras[ i ].m_dwSerialNumber == m_dwConnectedSerialNumber )
    {
      m_BusyCameras.RemoveAt( i );
      break;
    }
  }
  m_PropertyMap.clear();
  //   m_dwSerialNumber = -1 ;
  m_dwConnectedSerialNumber = 0;
  m_CameraStatus = CameraClosed;
  m_bInitialized = false;

#ifdef MatVis_DEBUG
  TRACE( "\nCamera closed SN=%u Index=%d" , m_dwSerialNumber , i );
#endif
}

bool MatVis::DriverValid()
{

  return  (m_CamerasOnBus != 0) /*&& (m_pCamera && m_pCamera->IsConnected())*/;
}

bool MatVis::CameraStart()
{
  if ( m_dwSerialNumber && (m_dwSerialNumber != 0xffffffff) && !m_bInScanProperties )
    return OtherThreadCameraStart();
  return false;
}
bool MatVis::MatVisCameraStart()
{
  if ( !m_pDevice )
  {
    if ( !MatVisCameraInit() )
    {
#ifdef MatVis_DEBUG
      TRACE( "\nMatVisCameraStart: Camera is not initialized SN=%u" , m_dwSerialNumber );
#endif
      return false;
    }
  }

  m_nWidth = GetXSize();
  m_nHeight = GetYSize();


  ULONG ulFrameSize = (ULONG) m_nWidth * (ULONG) m_nHeight;
  if ( ulFrameSize == 0 )
  {
    MatVisCameraClose();
    if ( !MatVisCameraInit() )
    {
      SENDERR( "Area is zero (%d,%d)" , (ULONG) m_nWidth , (ULONG) m_nHeight );
#ifdef MatVis_DEBUG
      TRACE( "\nMatVisCameraStart: Area for SN%u is zero (%d,%d)" ,
        m_dwSerialNumber , (ULONG) m_nWidth , (ULONG) m_nHeight );
#endif
      m_CameraStatus = CantInit;
      return false;
    }
  };
  TRACE( "%f Camera Start Frame Size %u" , GetHRTickCount() , ulFrameSize );
  C1394Camera::CameraStart();
  int iIndex = GetPropertyIndex( FGP_IMAGEFORMAT );
  LPCTSTR pPixelFormat = m_PropertiesEx[ iIndex ].m_enumVal;
  SetBMIH( pPixelFormat );

  if ( !m_hGrabThreadHandle || !m_uiGrabThreadId )
  {
    m_hGrabThreadHandle = (HANDLE) _beginthreadex( 0 , 0 ,
      GrabThread , (LPVOID) this , 0 , &m_uiGrabThreadId );
  }

  //   MatVisCameraClose() ;
  m_CameraStatus = CameraStarted;
  return true;
}
void MatVis::CameraStop()
{
  OtherThreadCameraStop();
}

void MatVis::MatVisCameraStop()
{
  if ( !IsRunning() )
    return;
  m_bRun = FALSE;
  if ( !m_pDevice )
    return;
  if ( m_pFuncInt )
    m_pFuncInt->acquisitionStop() ;
  C1394Camera::CameraStop();
  UINT Res = WaitForSingleObject( m_hGrabThreadHandle , 1000 );
  if ( Res != WAIT_OBJECT_0 )
  {
    SENDERR( "Grab Thread is not stopped (%d)" , Res ) ;
  }
  m_CameraStatus = CameraStopped;
  SetEvent( m_evFrameReady );
  DWORD dwStartWait = GetTickCount() ;
  DWORD dwWaitTime ;
  while ( !m_bIsStopped && ((dwWaitTime = (GetTickCount() - dwStartWait)) < 1000) )
  {
    Sleep( 10 ) ;
  }

  TRACE( "%f MatVis::CameraStop() for #%u\n " , GetHRTickCount() , m_dwSerialNumber );
}

CVideoFrame* MatVis::CameraDoGrab( double* StartTime )
{
  CVideoFrame * pOut = NULL;
  // waiting for frame or exit
  if ( !m_ReadyFrames.size() )
  {
    DWORD dwRes = WaitForMultipleObjects( 2 , m_WaitEventFrameArray , FALSE , /*INFINITE*/ 500 );
    if ( dwRes != WAIT_OBJECT_0 )
      return NULL;
  }
  m_dLastStartTime = ::GetHRTickCount();
  *StartTime = m_dLastStartTime - m_dLastInCallbackTime;
  if ( m_ReadyFrames.size() )
  {
    m_GrabLock.Lock( INFINITE , "MVG_CameraDoGrab" );
    pOut = m_ReadyFrames.front();
    m_ReadyFrames.pop();
    m_GrabLock.Unlock();
  }
  *StartTime -= m_dLastInCallbackTime;
  return pOut;
}

bool MatVis::BuildPropertyList()
{
  FXAutolock al( m_SettingsLock , "BuildPropertyList" );
  return OtherThreadBuildPropertyList();
}

bool MatVis::MatVisBuildPropertyList()
{
  CAMERA1394::Property P;
  if ( !m_pDevice )
  {
    return false;
  }

  if ( GetHRTickCount() - m_dLastBuiltPropertyTime < 1000. )
    return true;

  double dStart = GetHRTickCount();
  m_Properties.RemoveAll();
  m_PropertiesEx.RemoveAll();

  double dOpening = GetHRTickCount() - dStart;
  TRACE( "\n    MatVisBuildPropertyList: Camera Opening time is %g" , dOpening );

  m_SensorSize.cx = GetSensorWidth();
  m_SensorSize.cy = GetSensorHeight();
  m_nWidth = GetXSize();
  m_nHeight = GetYSize();
  bool m_bBinningFound = false;

  DeviceComponentLocator DevCLoc( m_pDevice , dltSetting , "Base" );
  for ( int i = 0; i < m_iNProperties; i++ )
  {
    FXString DlgFormat;
    const CamProperty& Prop = m_pOrigProperties[ i ];

    HOBJ hComponent = INVALID_ID;
    if ( Prop.CamPropertyName )
    {
      string TrigMode = "Camera/GenICam/AcquisitionControl/TriggerMode";
      hComponent = DevCLoc.findComponent( Prop.CamPropertyName );
      if ( hComponent == INVALID_ID )
      {
        //         hComponent = DevCLoc.findComponent( TrigMode ) ;
        continue;
      }
    }

    CameraAttribute NewAttribute( Prop.pr , Prop.name , Prop.CamPropertyName , hComponent );

    //    
    try
    {
      switch ( Prop.pr )
      {
      case FGP_ROI:
        NewAttribute.m_DlgFormat.Format( "EditBox(%s)" , Prop.name );
        break;
      case FGP_GRAB:
        NewAttribute.m_DlgFormat.Format( "Spin(%s,-1,65534)" , Prop.name );
        NewAttribute.SetType( ctPropInt );
        break;
      case FGP_SAVECALIBDATA:
        NewAttribute.m_DlgFormat.Format( "Spin(%s,0,1)" , Prop.name );
        NewAttribute.SetType( ctPropInt );
        break ;
      default:
        {
          mvIMPACT::acquire::Property Comp( hComponent );
          bool bRes = FormDialogData( Comp , NewAttribute );
          if ( bRes )
          {
            switch ( Prop.pr )
            {
            case FGP_EXTSHUTTER: // special case, integer values in usec
              NewAttribute.m_DlgFormat.Format( "Spin&Bool(%s,%d,%d)" ,
                Prop.name , (int) NewAttribute.m_dRange[ 0 ] , (int) NewAttribute.m_dRange[ 1 ] );
              NewAttribute.m_AutoControl = Prop.AutoControl;
              break;
            case FGP_BINNING:
            case FGP_BINNINGV:
            case FGP_DECIMATION:
            case FGP_DECIMATIONV:
              {
                if ( NewAttribute.m_int64Enums.size() == 0 ) // attribute as integer
                {
                  NewAttribute.m_DlgFormat.Format( _T( "ComboBox(%s(" ) ,
                    (LPCTSTR) (!NewAttribute.m_Name.IsEmpty() ? NewAttribute.m_Name : Comp.name().c_str()) );
                  NewAttribute.m_AllEnumeratorsAsString = _T( '[' );
                  for ( __int64 i = NewAttribute.m_i64Range[ 0 ] ; i <= NewAttribute.m_i64Range[ 1 ] ; i *= 2 )
                  {
                    NewAttribute.m_int64Enums.push_back( i ) ;

                    FXString Addition , AsString ;
                    AsString.Format( "%d" , (int) i ) ;
                    NewAttribute.m_StringEnums.push_back( (LPCTSTR)AsString );
                    if ( NewAttribute.m_int64Val == i )
                      NewAttribute.m_enumVal = (LPCTSTR) AsString;
                    Addition.Format( _T( "%s(%I64d)" ), (LPCTSTR) AsString , i );
                    NewAttribute.m_DlgFormat += Addition;
                    NewAttribute.m_DlgFormat +=
                        (i < NewAttribute.m_i64Range[ 1 ]) ? _T( ',' ) : _T( ')' );
                    NewAttribute.m_AllEnumeratorsAsString += Addition;
                    NewAttribute.m_AllEnumeratorsAsString +=
                      (i < NewAttribute.m_i64Range[ 1 ]) ? _T( ',' ) : _T( ']' );
                  };
                  NewAttribute.m_DlgFormat += _T( ')' );
                }
              }
            case FGP_IMAGEFORMAT:
              NewAttribute.m_bIsStopNecessary = true;
              break;
            }
          }
        }
        break;
      }


      //     if ( Prop.CamPropertyName /*&& bOutputLineProcessPass*/ )
      //                 {
      NewAttribute.m_bIsStopNecessary = Prop.m_bStopAcquisition;
    }
    catch ( const ImpactAcquireException& e )
    {
      // this e.g. might happen if the same device is already opened in another process...
      FxSendLogMsg( MSG_ERROR_LEVEL , GetGadgetInfo() , 0 ,
        _T( "Device %s MatVisBuildPropertyList ERROR %s for %s" ) ,
        GetDevice()->serial.read().c_str() ,
        e.getErrorCodeAsString().c_str() , Prop.name );
      continue;
    }
    if ( !NewAttribute.m_DlgFormat.IsEmpty() )
    {
      m_PropertiesEx.Add( NewAttribute );
    }
  }

  m_CameraStatus = PropertyListBuilt;
  m_dLastBuiltPropertyTime = GetHRTickCount();
  TRACE( "\n    MatVisBuildPropertyList: list built time is %g" , m_dLastBuiltPropertyTime - dStart );
  //m_pCamera->Close() ;
  return true;
}
bool MatVis::ReorderDialogData()
{
  //   for ( int i = 0 ; i < m_PropertiesEx.GetCount() ; i++ )
  //   {
  //     CameraAttribute& Prop = m_PropertiesEx.GetAt( i ) ;
  //     switch ( Prop.pr )
  //     {
  //       case FGP_TRIGGERONOFF:
  //       {
  //         if ( Prop.m_StringEnums.GetCount() == 2 )
  //         {
  //           int iTriggerSrcID = GetPropertyIndex( FGP_TRIGGER_SOURCE ) ;
  //           if ( iTriggerSrcID != WRONG_PROPERTY )
  //           {
  //             CameraAttribute& TrigSrcProp = m_PropertiesEx.GetAt( iTriggerSrcID ) ;
  //             Prop.m_StringEnums.RemoveAt( 1 ) ;
  //             Prop.m_StringEnums.Append( TrigSrcProp.m_StringEnums ) ;
  //             TrigSrcProp.m_DlgFormat.Empty() ;
  //           }
  //         }
  //         else
  //           ASSERT( 0 ) ;
  //       }
  //     	break;
  //     }
  //   }
  return true;
}

bool MatVis::GetCameraProperty( unsigned i , FXSIZE &value , bool& bauto )
{
  FXAutolock al( m_GrabLock , "MVG_GetProperty" );
  if ( !CheckAndAllocCamera() )
    return false;
  CameraAttribute& Prop = m_PropertiesEx.GetAt( i );

  switch ( Prop.pr )
  {
    //       case FGP_IMAGEFORMAT:
    //         {
    //           bauto = false;
    //           value = m_pixelFormat;
    //           return true;
    //         }
  case FGP_ROI:
    {
      CRect rc;
      GetROI( rc );
      sprintf_s( m_TempOutOfStack , "%d,%d,%d,%d" ,
        rc.left , rc.top , rc.right , rc.bottom );
      bauto = false;
      value = (FXSIZE) m_TempOutOfStack;
      return true;
    }
  case FGP_SAVECALIBDATA:
    value = 0 ;
    return true ;
  default:
    {
      SetCamPropertyData PropData;
      PropData.m_Pr = Prop.pr;

      if ( OtherThreadGetProperty( i , &PropData ) )
      {
        if ( Prop.m_AutoControl && PropData.m_bAuto )
          bauto = PropData.m_bBool;
        else
          bauto = false;
        if ( Prop.m_StringEnums.size() > 1 )
        {
          for ( int iECnt = 0; iECnt < (int)Prop.m_StringEnums.size(); iECnt++ )
          {
            if ( Prop.m_StringEnums[ iECnt ] == PropData.m_szString )
            {
              Prop.m_enumVal = PropData.m_szString;
              value = iECnt;
              return true;
            }
          }
        }
        //        else
        {
          switch ( Prop.m_Type )
          {
          case ctPropInt64:
            value = Prop.m_intVal = (int) (Prop.m_int64Val = PropData.m_int64);
            return true;
          case ctPropInt:
            Prop.m_int64Val = value = Prop.m_intVal = PropData.m_int;
            return true;
            value = PropData.m_int; return true;
          case ctPropFloat:
            {
              switch ( Prop.pr )
              {
              case FGP_EXTSHUTTER:
                m_dExtShutter = (double) (value = ROUND( Prop.m_dVal = PropData.m_double ));
                break;
              case FGP_GAIN:
                value = ROUND( Prop.m_dVal = (PropData.m_double * 10.) );
                break;

              case FGP_OUT_DELAY:
                {
                  strcpy_s( m_TempOutOfStack , PropData.m_szString );
                  value = (FXSIZE) m_TempOutOfStack;
                  Prop.m_dVal = PropData.m_double;
                }
                break;
              case FGP_FRAME_RATE:
                {
                  sprintf_s( m_TempOutOfStack , "%.2f" , PropData.m_double );
                  value = (FXSIZE) m_TempOutOfStack;
                  Prop.m_dVal = PropData.m_double;
                }
                break;
                //               case FGP_WHITEBAL_RATIO: value = ROUND( PropData.m_double * 100. ) ; break ;
              default:
                sprintf_s( m_TempOutOfStack , "%g" , PropData.m_double );
                value = (FXSIZE) m_TempOutOfStack;
                Prop.m_dVal = PropData.m_double;
                break;
              }
            }
            return true;
          case ctPropString:
            {
              {
                Prop.m_stringVal = m_TmpString = PropData.m_szString;
                value = (FXSIZE) (LPCTSTR) m_TmpString;
                return true;
              }
            }
            break;
          }
        }
      }
    }
  }
  bauto = false;
  return false;
}
bool MatVis::SetCameraProperty( unsigned iIndex , FXSIZE &value , bool& bauto , bool& Invalidate )
{
  //FXAutolock al( m_GrabLock , "MVG_SetProperty" );
  if ( !CheckAndAllocCamera() )
    return false;

  bool bRes = false;
  SetCamPropertyData& Data = m_PropertyData;
  CameraAttribute& pProp = m_PropertiesEx.GetAt( iIndex );
  Data.m_Pr = pProp.pr;
  if ( !pProp.m_AutoControl.IsEmpty() )
  {
    Data.m_bAuto = true;
    Data.m_bBool = bauto;
  }
  else
    Data.m_bAuto = false;

  if ( pProp.m_StringEnums.size() != 0
    && (0 <= value) && (value < (int)pProp.m_StringEnums.size()) )
  {
    strcpy_s( Data.m_szString , (LPCTSTR) pProp.m_StringEnums[ value ].c_str() );
  }
  else
    Data.m_szString[ 0 ] = 0 ;

  switch ( pProp.m_Type )
  {
  case ctPropInt:
  case ctPropInt64:
    {
//       if ( pProp.m_StringEnums.size() != 0 )
//       {
// 
//       }
//       else
//       {
        Data.m_int = (int) (Data.m_int64 = value);
//       }
    }
    break;
  case ctPropFloat:
    {
      double dValue;
      switch ( pProp.pr )
      {
      case FGP_EXTSHUTTER: dValue = (double) value; break;
      case FGP_GAIN: dValue = value * 0.1; break;

//       case FGP_OUT_DELAY:
//       case FGP_FRAME_RATE:
//         dValue = atof( (LPCTSTR) value );
//         break;
        //         case FGP_WHITEBAL_RATIO: dValue = ( double ) value / 100. ; break ;
        //         case FGP_TEMPERATURE_S:
        //           _tcscpy_s( Data.m_szString , ( LPCTSTR ) value ) ;
      default: 
        strcpy_s( Data.m_szString , (LPCTSTR) value ) ;
        dValue = atof( Data.m_szString );
        break;
      }
      Data.m_double = dValue;
      break;
    }
  case ctPropString:
    {
      //       if ( pProp.m_StringEnums.GetCount() > 1 )
      //       {
      //         if ( ( 0 <= value ) && ( ( int ) value < pProp.m_StringEnums.GetCount() ) )
      //           strcpy_s( Data.m_szString , ( LPCTSTR ) pProp.m_StringEnums[ ( int ) value ] ) ;
      //         else
      //         {
      //           SENDERR( "Set Property %s bad enum index %d[0,%d]" , pProp.m_Name ,
      //             value , pProp.m_StringEnums.GetCount() ) ;
      //           return false ;
      //         }
      //         pProp.m_stringVal = Data.m_szString  ;
      //         if ( pProp.pr == FGP_LINE_SELECT )
      //           TRACE( "\nBefore SetPropertyEx %s = %s, Invalidate = %d" ,
      //           ( LPCTSTR ) pProp.m_Name , Data.m_szString , ( int ) Data.m_bInvalidate ) ;
      //       }
      //       else
      {
        strcpy_s( Data.m_szString , (LPCTSTR) value );
        pProp.m_stringVal = Data.m_szString;
      }
    }
    break; ;
  default:
    if ( pProp.pr == FGP_ROI )
    {
      strcpy_s( Data.m_szString , (LPCTSTR) value );
      pProp.m_stringVal = Data.m_szString;
      break;
    }
    else if ( pProp.pr == FGP_LOG )
      break;
    else
    {
      C1394_SENDERR_2( "Undefined property '%s' type %d" , pProp.m_Name , pProp.m_Type );
      return false;
    }
  }
  if ( IsRunning() )
  {
    if ( pProp.m_bIsStopNecessary )
    {
      bRes = OtherThreadCameraStop();
      if ( bRes )
      {
        m_bRun = false ;
        m_bShouldBeReprogrammed = m_bWasStopped = true;
      }
      else
      {
        C1394_SENDERR_1( "Can't stop grab for property '%s' change" , (LPCTSTR) pProp.m_Name );
        return false;
      }
    }
  }
  if ( pProp.pr == FGP_LOG )
  {
    FXSIZE iPos = 0;
    m_LogLock.Lock();
    m_PropertiesForLogAsString = (LPCTSTR) value;
    FXString Token = m_PropertiesForLogAsString.Tokenize( _T( ", \t" ) , iPos );
    if ( Token.IsEmpty() )
    {
      m_dLogPeriod_ms = 0.;
    }
    else
    {
      double dPeriod = atof( Token );
      if ( dPeriod <= 0.0 )
        m_dLogPeriod_ms = 0.;
      else
      {
        m_dLogPeriod_ms = dPeriod;
        m_PropertiesForLog.RemoveAll();
        while ( !(Token = m_PropertiesForLogAsString.Tokenize( _T( ", \t" ) , iPos )).IsEmpty() )
          m_PropertiesForLog.Add( Token );
      }
      m_LogOutString = m_PropertiesForLogAsString + _T( "\n" );
      m_iLogCnt = 0;
      m_BusEvents |= MatVis_EVT_LOG;
      SetEvent( m_evCameraControl );
    }
    m_LogLock.Unlock();
    bRes = true;
  }
  else
//    bRes = OtherThreadSetProperty( iIndex , &Data , &Invalidate );
  {
    bRes = SetCameraPropertyEx( iIndex , &Data , Invalidate ) ;
  }
  if ( bRes )
  {
    if ( pProp.pr == FGP_LINE_SELECT
      || pProp.pr == FGP_LINEIN_SELECT
      || pProp.pr == FGP_LINEOUT_SELECT )
    {
      SetCamPropertyData DataAfter;
      TRACE( "\nAfter SetPropertyEx %s = %s, Invalidate = %d" ,
        (LPCTSTR) pProp.m_Name , Data.m_szString , (int) Data.m_bInvalidate );
      if ( GetPropertyValue( (LPCTSTR) pProp.m_CameraPropertyName , DataAfter ) )
      {
        TRACE( "\nReal Value for %s = %s" , (LPCTSTR) pProp.m_Name , DataAfter.m_szString );
      }
      else
        ASSERT( 0 );
      m_iSelectedLineNumber = (int) value;
      Invalidate = true;
    }
    CameraAttribute& Prop = m_PropertiesEx.GetAt( iIndex );
    if ( Prop.m_StringEnums.size() > 1 )
    {
      Prop.m_enumVal = Data.m_szString;
    }
    else
    {
      switch ( Prop.m_Type )
      {
      case ctPropInt: Prop.m_int64Val = Prop.m_intVal = Data.m_int; break;
      case ctPropInt64: Prop.m_intVal = (int) (Prop.m_int64Val = Data.m_int64); break;
      case ctPropFloat: Prop.m_dVal = Data.m_double; break;
      case ctPropString: Prop.m_stringVal = Data.m_szString; break;
      default:
        if ( Prop.pr == FGP_ROI )
        {
        }
        else
          ASSERT( 0 );
        break;
      }
    }
  }

  return bRes;

}


bool MatVis::MatVisSaveOrLoadSettings( int iMode )  // 0 - nothing to do, 1 - Save, 2 - Load
{
  if ( iMode == 0 )
    return true;
  if ( iMode < 0 || iMode > 2 )
    return false;

  return false;
}

bool MatVis::SetCameraPropertyEx( unsigned iIndex , SetCamPropertyData * pData , bool& Invalidate )
{
  if ( !m_pDevice )
  {
    SENDERR( "\nSetCameraPropertyEx - No ptr to camera for SN%u " , m_dwSerialNumber );
    TRACE( "\nSetCameraPropertyEx - No ptr to camera for SN%u " , m_dwSerialNumber );
    return false;
  }
  if ( iIndex >= (unsigned) m_PropertiesEx.GetCount() )
  {
    SENDERR( "\nSetCameraPropertyEx - No property for index %i " , iIndex );
    TRACE( "\nSetCameraPropertyEx - No property for index %i " , iIndex );
    return false;
  }

  CameraAttribute& Prop = m_PropertiesEx.GetAt( iIndex );
  try
  {
    switch ( Prop.pr )
    {
    case FGP_ROI:
      {
        CRect rc;
        if ( sscanf( (LPCTSTR) pData->m_szString , "%d,%d,%d,%d" ,
          &rc.left , &rc.top , &rc.right , &rc.bottom ) == 4 )
        {
          rc.right += rc.left;
          rc.bottom += rc.top;
          Invalidate = true;
          SetCheckRegionAndBinningOrDecimation(
            rc , NULL , true );

          SetROI( rc );
          return true;
        }
        return false;
      }
    case FGP_SAVECALIBDATA:
      {
        mvIMPACT::acquire::GenICam::AnalogControl AnC( m_pDevice  ) ;
        TDMR_ERROR error = static_cast<TDMR_ERROR>(AnC.mvSaveCalibrationData.call());
        if ( error != DMR_NO_ERROR )
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , GetGadgetInfo() , 0 ,
            _T( "Can't save calibration data (%d): %s (%u-0x%X)" ) ,
            ImpactAcquireException::getErrorCodeAsString( error ).c_str() ,
            error , error );
          return false ;
        }
        return true ;
      }
    case FGP_BINNING:
    case FGP_BINNINGV:
    case FGP_DECIMATION:
    case FGP_DECIMATIONV:
      {
        bool bBinning = ((int) Prop.pr >= (int) FGP_BINNINGV) ;
        __int64 iCoeff = (int) pData->m_int64 ;

        __int64 iReadBH ;

        if ( bBinning )
        {
          bool bHor = GetFeatureIntValue( 
            "Camera/GenICam/ImageFormatControl/BinningHorizontal" , iReadBH ) ;
          if ( (iCoeff != iReadBH) )
            Invalidate = true ;
        }
        else
        {
          bool bHor = GetFeatureIntValue( 
            "Camera/GenICam/ImageFormatControl/DecimationHorizontal" , iReadBH ) ;
          if ( (iCoeff != iReadBH) )
            Invalidate = true ;
        }
        if ( Invalidate ) // something necessary to change
        {
          double dCoeff = log2( (double) iCoeff ) ;
          iCoeff = ROUND( dCoeff ) ;
          iCoeff = ROUND( pow( 2. , (double) iCoeff ) ) ;
          if ( iCoeff < Prop.m_i64Range[ 0 ] )
            iCoeff = Prop.m_i64Range[ 0 ] ;
          if ( iCoeff > Prop.m_i64Range[ 1 ] )
            iCoeff = Prop.m_i64Range[ 1 ] ;

          //Invalidate = false ;
          sprintf( pData->m_szString , "%d" , (int)iCoeff ) ;
          return SetCheckRegionAndBinningOrDecimation(
            m_CurrentROI , pData->m_szString , bBinning );
        }
        else
          return true ;
      }
    case FGP_TEMPERATURE_S:
      {
      }
      return true;
    case FGP_GRAB:
      return SetGrab( (int) pData->m_int );

    default:
      {
        DeviceComponentLocator DevCLoc( m_pDevice , dltSetting , "Base" );
        const CameraAttribute& Prop = m_PropertiesEx[ iIndex ];

        HOBJ hComponent = INVALID_ID;
        if ( Prop.m_CameraPropertyName )
        {
          hComponent = DevCLoc.findComponent( (LPCTSTR) Prop.m_CameraPropertyName );
          if ( hComponent == INVALID_ID )
          {
            SENDERR( "\nSetCameraPropertyEx - Can't locate property %s " ,
              (LPCTSTR) Prop.m_CameraPropertyName );
            TRACE( "\nSetCameraPropertyEx - Can't locate property %s " ,
              (LPCTSTR) Prop.m_CameraPropertyName );
            return false;
          }
        }

        Property Found( hComponent );
        if ( !Found.isValid() )
        {
          SENDERR( "\nSetCameraPropertyEx - property %s is not valid" ,
            (LPCTSTR) Prop.m_CameraPropertyName );
          TRACE( "\nSetCameraPropertyEx - property %s is not valid" ,
            (LPCTSTR) Prop.m_CameraPropertyName );
          return false;
        }
        if ( !Found.isWriteable() )
        {
          SENDERR( "\nSetCameraPropertyEx - property %s is not writable" ,
            (LPCTSTR) Prop.m_CameraPropertyName );
          TRACE( "\nSetCameraPropertyEx - property %s is not writable" ,
            (LPCTSTR) Prop.m_CameraPropertyName );
          return false;
        }
        bool bAutoOn = false;
        if ( !Prop.m_AutoControl.IsEmpty() )
        {
          HOBJ hCompAuto = DevCLoc.findComponent( (LPCTSTR) Prop.m_AutoControl );
          if ( hCompAuto == INVALID_ID || !Property( hCompAuto ).isValid() )
          {
            SENDERR( "\nSetCameraPropertyEx - Can't get auto property '%s' for %s" ,
              (LPCTSTR) Prop.m_AutoControl , (LPCTSTR) Prop.m_Name );
            TRACE( "\nSetCameraPropertyEx - Can't get auto property '%s' for %s" ,
              (LPCTSTR) Prop.m_AutoControl , (LPCTSTR) Prop.m_Name );
            return false;
          }
          Property FoundAuto( hCompAuto );
          std::string AutoValue = FoundAuto.readS();
          if ( !AutoValue.size() )
          {
            return false;
          }
          int iDictSize = FoundAuto.dictSize();
          FXStringArray Enums;
          if ( iDictSize > 1 )
          {
            GetPropertyDictionary( FoundAuto , Enums );
          }
          bAutoOn = (Enums[ 0 ] != AutoValue.c_str());
          //           if ( bAutoOn != pData->m_bBool )
          //           {
          LPCTSTR pValue = (LPCTSTR)
            Enums[ (pData->m_bBool) ? Enums.GetUpperBound() : 0 ];
          bAutoOn = pData->m_bBool;
          FoundAuto.writeS( pValue );
          //           }
        }
        int iDictSize = Found.dictSize();
        switch ( Prop.m_Type )
        {
        case ctPropInt:
          {
            if ( Prop.m_AutoControl.IsEmpty() || !bAutoOn )
            {
              PropertyI PropI( Found.hObj() );
              if ( iDictSize < 2 )
              {
                if ( Prop.m_bHasMinMax )
                {
                  if ( pData->m_int < Prop.m_i64Range[ 0 ] )
                    pData->m_int = (int) Prop.m_i64Range[ 0 ];
                  else if ( pData->m_int > Prop.m_i64Range[ 1 ] )
                    pData->m_int = (int) Prop.m_i64Range[ 1 ];
                }
                PropI.write( pData->m_int );
              }
              else
              {
                PropI.writeS( pData->m_szString );
              }
            }
            return true;
          }
        case ctPropInt64:
          {
            if ( Prop.m_AutoControl.IsEmpty() || !bAutoOn )
            {
              PropertyI64 PropI64( Found.hObj() );
              if ( iDictSize < 2 )
              {
                if ( Prop.m_bHasMinMax )
                {
                  if ( pData->m_int64 < Prop.m_i64Range[ 0 ] )
                    pData->m_int64 = (int) Prop.m_i64Range[ 0 ];
                  else if ( pData->m_int64 > Prop.m_i64Range[ 1 ] )
                    pData->m_int64 = (int) Prop.m_i64Range[ 1 ];
                }
                PropI64.write( pData->m_int64 );
              }
              else
              {
                PropI64.writeS( pData->m_szString );
              }
            }
            return true;
          }

        case ctPropFloat:
          {
            if ( Prop.m_AutoControl.IsEmpty() || !bAutoOn )
            {
              PropertyF PropF( Found.hObj() );
              //             if ( Prop.pr == FGP_OUT_DELAY )
              //             {
              //               SetCamPropertyData Tmp ;
              //               Tmp.m_bBool = ( pData->m_double > 1. ) ;
              //               SetPropertyValue( "IntEnaDelayEnable" , Tmp ) ;
              //               if ( pData->m_double <= 1. )
              //                 break ; // simple disable, not necessary to set delay
              //             }
              //             if ( Prop.pr == FGP_FRAME_RATE  &&  pData->m_double > 1. )
              //             {
              //               double dPeriod = 1.E6 / pData->m_double ;
              //               if ( m_dExtShutter != 0.  &&  dPeriod < m_dExtShutter )
              //                 pData->m_double = 1.E6 / m_dExtShutter ;
              //               m_iFPSx10 = ROUND( 10. * pData->m_double ) ;
              //             }
              //             if ( Prop.pr == FGP_EXTSHUTTER  &&  pData->m_double > 0.1 )
              //             {
              //               int iFPSLimitx10 = ( int ) ( 1.E7 / pData->m_double ) ;
              //               if ( m_iFPSx10 > iFPSLimitx10 )
              //                 m_iFPSx10 = iFPSLimitx10 ;
              //               else
              //               {
              //                 SetCamPropertyData FPSData ;
              //                 GetPropertyValue( m_FPSPropertyName , FPSData ) ;
              //                 m_iFPSx10 = ROUND( FPSData.m_double * 10. ) ;
              //               }
              //             }
              PropF.write( pData->m_double );
              //             if ( Prop.pr == FGP_EXTSHUTTER )
              //             {
              //               SetCamPropertyData FPSData ;
              //               FPSData.m_double = ( double ) m_iFPSx10 / 10. ;
              //               SetPropertyValue( m_FPSPropertyName , FPSData ) ;
              //             }
            }
            return true;
          }
          //case VmbFeatureDataEnum:
        case ctPropString:
          {
            if ( Prop.m_AutoControl.IsEmpty() || !bAutoOn )
            {
              PropertyS PropS( Found.hObj() );
              PropS.write( pData->m_szString );
            }
            return true;
          }
        }
      }
    }
  }
  catch ( const EPropertyHandling& e )
  {
    SEND_DEVICE_ERR( "SetCameraPropertyEx by index: Prop %s set exception: %s  " ,
      Prop.m_Name , e.getErrorString().c_str() );
    return false;
  }
  return false;
}

bool MatVis::MatVisSetCameraPropertyEx( LPCTSTR pName , SetCamPropertyData * pData )
{
  if ( !m_pDevice )
  {
    SENDERR( "\nMatVisSetCameraPropertyEx - No ptr to camera for SN%u " , m_dwSerialNumber );
    TRACE( "\nMatVisSetCameraPropertyEx - No ptr to camera for SN%u " , m_dwSerialNumber );
    return false;
  }
  {
    DeviceComponentLocator DevCLoc( m_pDevice , dltSetting , "Base" );
    Property Found = GetPropertyByName( pName );
    int iIndex = GetPropertyIndex( pName );
    const CameraAttribute& Prop = m_PropertiesEx[ iIndex ];

    if ( !Found.isValid() )
    {
      SENDERR( "\nMatVisSetCameraPropertyEx - Can't get property by name %s" , pName );
      TRACE( "\nMatVisSetCameraPropertyEx - Can't get property by name %s: %s" , pName );
      return false;
    }
    bool bAutoOn = false;
    if ( !Prop.m_AutoControl.IsEmpty() )
    {
      HOBJ hCompAuto = DevCLoc.findComponent( (LPCTSTR) Prop.m_AutoControl );
      if ( hCompAuto == INVALID_ID || !Property( hCompAuto ).isValid() )
      {
        SENDERR( "\nSetCameraPropertyEx - Can't get auto property '%s' for %s" ,
          (LPCTSTR) Prop.m_AutoControl , (LPCTSTR) Prop.m_Name );
        TRACE( "\nSetCameraPropertyEx - Can't get auto property '%s' for %s" ,
          (LPCTSTR) Prop.m_AutoControl , (LPCTSTR) Prop.m_Name );
        return false;
      }
      Property FoundAuto( hCompAuto );
      std::string AutoValue = FoundAuto.readS();
      if ( !AutoValue.size() )
      {
        return false;
      }
      int iDictSize = FoundAuto.dictSize();
      FXStringArray Enums;
      if ( iDictSize > 1 )
      {
        GetPropertyDictionary( FoundAuto , Enums );
      }
      bAutoOn = (Enums[ 0 ] != AutoValue.c_str());
      //           if ( bAutoOn != pData->m_bBool )
      //           {
      LPCTSTR pValue = (LPCTSTR)
        Enums[ (pData->m_bBool) ? Enums.GetUpperBound() : 0 ];
      bAutoOn = pData->m_bBool;
      FoundAuto.writeS( pValue );
      //           }
    }
    TComponentType Type = Found.type();
    int iDictSize = Found.dictSize();
    try
    {
      switch ( Type )
      {
      case ctPropInt:
        {
          if ( Prop.m_AutoControl.IsEmpty() || !bAutoOn )
          {
            PropertyI PropI( Found.hObj() );
            if ( iDictSize < 2 )
            {
              if ( Prop.m_bHasMinMax )
              {
                if ( pData->m_int < Prop.m_i64Range[ 0 ] )
                  pData->m_int = (int) Prop.m_i64Range[ 0 ];
                else if ( pData->m_int > Prop.m_i64Range[ 1 ] )
                  pData->m_int = (int) Prop.m_i64Range[ 1 ];
              }
              PropI.write( pData->m_int );
            }
            else
            {
              PropI.writeS( pData->m_szString );
            }
          }
          return true;
        }
      case ctPropInt64:
        {
          PropertyI64 PropI64( Found.hObj() );
          if ( iDictSize < 2 )
          {
            if ( Prop.m_bHasMinMax )
            {
              if ( pData->m_int64 < Prop.m_i64Range[ 0 ] )
                pData->m_int64 = (int) Prop.m_i64Range[ 0 ];
              else if ( pData->m_int64 > Prop.m_i64Range[ 1 ] )
                pData->m_int64 = (int) Prop.m_i64Range[ 1 ];
            }
            PropI64.write( pData->m_int64 );
          }
          else
          {
            PropI64.writeS( pData->m_szString );
          }
          return true;
        }
      case ctPropFloat:
        {
          PropertyF PropF( Found.hObj() );
          PropF.write( pData->m_double );
          return true;
        }
        //case VmbFeatureDataEnum:
      case ctPropString:
        {
          PropertyS( Found.hObj() ).write( pData->m_szString );
          return true;
        }
        break;
      default: ASSERT( 0 ); break;
      }
    }
    catch ( const EPropertyHandling& e )
    {
      SEND_DEVICE_ERR( "MatVisSetCameraPropertyEx by name: Prop %s set exception: %s  " ,
        Found.name().c_str() , e.getErrorString().c_str() );
    }
  }
  return false;
}

bool MatVis::GetCameraPropertyEx( int iIndex , SetCamPropertyData * pData )
{
  CameraAttribute& Prop = m_PropertiesEx.GetAt( iIndex );
  switch ( Prop.pr )
  {
  case FGP_GRAB:
    {
      int iNFrames;
      if ( GetGrabConditions( m_GrabMode , iNFrames ) )
      {
        pData->m_int64 = pData->m_int = m_iNFramesForGrabbing;
        pData->m_Type = ctPropInt;
        return true;
      }
      return false;
    }
  }
  bool bRes = MatVisGetCameraPropertyEx(
    (LPCTSTR) Prop.m_CameraPropertyName , pData );
  if ( !bRes )
    return false;
  switch ( Prop.pr )
  {
  case FGP_TRIGGERONOFF:
    {
      if ( Prop.m_StringEnums[ 0 ] != pData->m_szString )
      {
        int iTriggerSrcID = GetPropertyIndex( FGP_TRIGGER_SOURCE );
        if ( iTriggerSrcID != WRONG_PROPERTY )
        {
          CameraAttribute& TrigSrcProp = m_PropertiesEx.GetAt( iTriggerSrcID );
          SetCamPropertyData TrigSrcData;
          bRes = MatVisGetCameraPropertyEx(
            (LPCTSTR) TrigSrcProp.m_CameraPropertyName , &TrigSrcData );
          if ( bRes )
            strcpy_s( pData->m_szString , TrigSrcData.m_szString );
        }
      }
    }
    break;
  }

  return bRes;
}

bool MatVis::GetCameraPropertyEx( LPCTSTR pszPropertyName , SetCamPropertyData * pData )
{
  return MatVisGetCameraPropertyEx( pszPropertyName , pData );
}

bool MatVis::MatVisGetCameraPropertyEx( LPCTSTR pszPropertyName ,
  SetCamPropertyData * pData )
{
  if ( !m_pDevice )
    return false;

  DeviceComponentLocator DevCLoc( m_pDevice , dltSetting , "Base" );

  HOBJ hComponent = DevCLoc.findComponent( pszPropertyName );
  if ( hComponent == INVALID_ID )
  {
    if ( m_bViewErrorMessagesOnGetSet )
    {
      SEND_DEVICE_ERR( "Property %s is not found" , pszPropertyName );
    }
    return false;
  }

  Property Found( hComponent );
  return MatVisGetCameraPropertyEx( &Found , pData );
  //   pData->m_Type = Found.type() ;
  //   UINT uiDictSize = ( Found.hasDict() ) ? Found.dictSize() : 1 ;
  // 
  //   int iIndex = (uiDictSize > 1) ? GetInCameraPropertyIndex( pszPropertyName ) : -1  ;
  //   pData->m_szString[ 0 ] = 0 ;
  //   try
  //   {
  //     switch ( pData->m_Type )
  //     {
  //       case ctPropInt64:
  //       {
  //         PropertyI64 i64( Found.hObj() ) ;
  //         pData->m_int = ( int ) ( pData->m_int64 = i64.read() ) ;
  //         if ( uiDictSize > 1 )
  //         {
  //           strcpy_s( pData->m_szString , i64.readS().c_str() ) ;
  // //           const CameraAttribute& Prop = m_PropertiesEx[ iIndex ] ;
  // //           for ( UINT i = 0 ; i < uiDictSize ; i++ )
  // //           {
  // //             if ( Prop.m_StringEnums[i] == pData->m_szString )
  // //             {
  // //               pData->m_int = i ;
  // //               break ;
  // //             }
  // //           }
  //         }
  //         else
  //           pData->m_szString[ 0 ] = 0 ;
  //         return true ;
  //       }
  //       case ctPropInt:
  //       {
  //         PropertyI IProp( Found.hObj() ) ;
  //         pData->m_int = IProp.read() ;
  //         if ( uiDictSize > 1 )
  //         {
  //           strcpy_s( pData->m_szString , IProp.readS().c_str() ) ;
  // //          for ( UINT i = 0 ; i < uiDictSize ; i++ )
  // //           {
  // //             int iVal = IProp.read( i ) ;
  // //             if ( iVal == pData->m_int )
  // //             {
  // //               strcpy_s( pData->m_szString , IProp.readS( i ).c_str() ) ;
  // //               break ;
  // //             }
  // //           }
  //         }
  //         else
  //           pData->m_szString[ 0 ] = 0 ;
  //         return true ;
  //       }
  //       case ctPropFloat:
  //       {
  //         PropertyF FProp( Found.hObj() ) ;
  //         pData->m_double = FProp.read() ;
  //         if ( uiDictSize > 1 )
  //         {
  //           for ( UINT i = 0 ; i < uiDictSize ; i++ )
  //           {
  //             double dVal = FProp.read( i ) ;
  //             if ( dVal == pData->m_double )
  //             {
  //               strcpy_s( pData->m_szString , FProp.readS( i ).c_str() ) ;
  //               break ;
  //             }
  //           }
  //         }
  //         if ( !pData->m_szString[ 0 ] )
  //           sprintf_s( pData->m_szString , _T( "%g" ) , pData->m_double ) ;
  //         return true ;
  //       }
  //       case ctPropString: strcpy( pData->m_szString , 
  //                  PropertyS( Found.hObj() ).read().c_str() ) ; 
  //                  return true ;
  //       default:
  //         SEND_DEVICE_ERR( "Unsupported property type %s for %s" , 
  //                DecodePropertyType( pData->m_Type ) , pszPropertyName ) ;
  //         return false ;
  //     }
  //   }
  //   catch ( const EPropertyHandling& e )
  //   {
  //     SEND_DEVICE_ERR( "MatVisGetCameraPropertyEx by name: Prop %s set exception: %s  " ,
  //       Found.name().c_str() , e.getErrorString().c_str() );
  //   }
  //   return false ;
}

bool MatVis::MatVisGetCameraPropertyEx( Property * pProperty ,
  SetCamPropertyData * pData )
{
  pData->m_Type = pProperty->type();
  if ( !(pData->m_Type & ctProp) )
  {
    SEND_DEVICE_ERR( "MatVisGetCameraPropertyEx - property %s is not simple: %s" ,
      pProperty->name().c_str() , DecodePropertyType( pData->m_Type ) );
    TRACE( "\n MatVisGetCameraPropertyEx - property %s is not simple: %s" ,
      pProperty->name().c_str() , DecodePropertyType( pData->m_Type ) );
    return false;
  };
  if ( pProperty->hasDict() && pProperty->dictSize() && pData->m_szString )
  {
    try
    {
      string sOut = pProperty->readS();
      strcpy_s( pData->m_szString , sOut.c_str() );
    }
    catch ( const EPropertyHandling& e )
    {
      SEND_DEVICE_ERR( "MatVisGetCameraPropertyEx: Prop %s Enum read exception: %s  " ,
        pProperty->name().c_str() , e.getErrorString().c_str() );
      return false;
    }
  }

  try
  {
    switch ( pData->m_Type )
    {
    case ctPropInt64:
      pData->m_int = (int) (pData->m_int64 = PropertyI64( pProperty->hObj() ).read());
      return true;
    case ctPropInt:
      pData->m_int64 = pData->m_int = PropertyI( pProperty->hObj() ).read();
      return true;
    case ctPropFloat:
      pData->m_double = PropertyF( pProperty->hObj() ).read();
      return true;
    case ctPropString:
      strcpy( pData->m_szString , PropertyS( pProperty->hObj() ).read().c_str() );
      return true;
    default:
      SEND_DEVICE_ERR( "Unsupported property type %s for %s" , DecodePropertyType( pData->m_Type ) , pProperty->name().c_str() );
      return false;
    }
  }
  catch ( const EPropertyHandling& e )
  {
    SEND_DEVICE_ERR( "MatVisGetCameraPropertyEx: Prop %s read exception: %s  " ,
      pProperty->name().c_str() , e.getErrorString().c_str() );
  }
  return false;
}

bool MatVis::MatVisSetAndCheckPropertyEx( LPCTSTR pName , SetCamPropertyData * pData )
{
  if ( !m_pDevice )
  {
    SENDERR( "\nMatVisSetCameraPropertyEx - No ptr to camera for SN%u " , m_dwSerialNumber );
    TRACE( "\nMatVisSetCameraPropertyEx - No ptr to camera for SN%u " , m_dwSerialNumber );
    return false;
  }
  DeviceComponentLocator DevCLoc( m_pDevice , dltSetting , "Base" );
  Property Found = GetPropertyByName( pName );

  if ( !Found.isValid() )
  {
    SENDERR( "\nMatVisSetCameraPropertyEx - Can't get property by name %s" , pName );
    TRACE( "\nMatVisSetCameraPropertyEx - Can't get property by name %s: %s" , pName );
    return false;
  }

  TComponentType Type = Found.type();
  int iDictSize = Found.dictSize();
  int iIndex = GetPropertyIndex( pName );
  bool bAutoOn = false;
  if ( iIndex > -1000 )
  {
    const CameraAttribute& Prop = m_PropertiesEx[ iIndex ];
    ASSERT( Type == pData->m_Type );
    if ( !Prop.m_AutoControl.IsEmpty() )
    {
      HOBJ hCompAuto = DevCLoc.findComponent( (LPCTSTR) Prop.m_AutoControl );
      if ( hCompAuto == INVALID_ID || !Property( hCompAuto ).isValid() )
      {
        SENDERR( "\nSetCameraPropertyEx - Can't get auto property '%s' for %s" ,
          (LPCTSTR) Prop.m_AutoControl , (LPCTSTR) Prop.m_Name );
        TRACE( "\nSetCameraPropertyEx - Can't get auto property '%s' for %s" ,
          (LPCTSTR) Prop.m_AutoControl , (LPCTSTR) Prop.m_Name );
        return false;
      }
      Property FoundAuto( hCompAuto );
      std::string AutoValue = FoundAuto.readS();
      if ( !AutoValue.size() )
      {
        return false;
      }
      int iDictSize = FoundAuto.dictSize();
      FXStringArray Enums;
      if ( iDictSize > 1 )
      {
        GetPropertyDictionary( FoundAuto , Enums );
      }
      bAutoOn = (Enums[ 0 ] != AutoValue.c_str());
      LPCTSTR pValue = (LPCTSTR)
        Enums[ (pData->m_bBool) ? Enums.GetUpperBound() : 0 ];
      bAutoOn = pData->m_bBool;
      FoundAuto.writeS( pValue );
      if ( bAutoOn )
        return true;
    }
  }
  if ( Found.isWriteable() )
  {
    SENDERR( "\nMatVisSetCameraPropertyEx - Property %s is not writable" , pName );
    TRACE( "\nMatVisSetCameraPropertyEx - Property %s is not writable" , pName );
    return false;
  }

  try
  {
    switch ( Type )
    {
    case ctPropInt:
      {
        PropertyI PropI( Found.hObj() );
        if ( iDictSize < 2 )
        {
          if ( PropI.hasMinValue() )
          {
            int iMinVal = PropI.getMinValue();
            if ( pData->m_int < iMinVal )
              pData->m_int = iMinVal;
          }
          if ( PropI.hasMaxValue() )
          {
            int iMaxVal = PropI.getMaxValue();
            if ( pData->m_int > iMaxVal )
              pData->m_int = iMaxVal;
          }
          PropI.write( pData->m_int );
          int iRead = PropI.read();
          if ( iRead != pData->m_int )
          {
            SEND_DEVICE_ERR( "MatVisSetCheckPropertyEx: "
              "Prop %s int read after write problem - write=%d read=%d" ,
              Found.name().c_str() , pData->m_int , iRead );
          }
        }
        else
        {
          PropI.writeS( pData->m_szString );
          string ReadValue = PropI.readS();
          if ( ReadValue != pData->m_szString )
          {
            SEND_DEVICE_ERR( "MatVisSetCheckPropertyEx: "
              "Prop %s enumi read after write problem - write=%s read=%s" ,
              Found.name().c_str() , pData->m_szString ,
              ReadValue.c_str() );
          }
        }
        return true;
      }
    case ctPropInt64:
      {
        PropertyI64 PropI( Found.hObj() );
        if ( iDictSize < 2 )
        {
          if ( PropI.hasMinValue() )
          {
            int64_type iMinVal = PropI.getMinValue();
            if ( pData->m_int64 < iMinVal )
              pData->m_int = (int) (pData->m_int64 = iMinVal);
          }
          if ( PropI.hasMaxValue() )
          {
            int64_type iMaxVal = PropI.getMaxValue();
            if ( pData->m_int64 > iMaxVal )
              pData->m_int = (int) (pData->m_int64 = iMaxVal);
          }
          PropI.write( pData->m_int64 );
          int64_type iRead = PropI.read();
          if ( iRead != pData->m_int )
          {
            SEND_DEVICE_ERR( "MatVisSetCheckPropertyEx: "
              "Prop %s int64 read after write problem - write=%lld read=%lld" ,
              Found.name().c_str() , pData->m_int64 , iRead );
          }
        }
        else
        {
          PropI.writeS( pData->m_szString );
          string ReadValue = PropI.readS();
          if ( ReadValue != pData->m_szString )
          {
            SEND_DEVICE_ERR( "MatVisSetCheckPropertyEx: "
              "Prop %s enumi64 read after write problem - write=%s read=%s" ,
              Found.name().c_str() , pData->m_szString ,
              ReadValue.c_str() );
          }
        }
        return true;
      }
    case ctPropFloat:
      {
        PropertyF PropF( Found.hObj() );
        if ( iDictSize < 2 )
        {
          if ( PropF.hasMinValue() )
          {
            double iMinVal = PropF.getMinValue();
            if ( pData->m_double < iMinVal )
              pData->m_double = iMinVal;
          }
          if ( PropF.hasMaxValue() )
          {
            double iMaxVal = PropF.getMaxValue();
            if ( pData->m_double > iMaxVal )
              pData->m_double = iMaxVal;
          }
          PropF.write( pData->m_double );
          double dRead = PropF.read();
          if ( dRead != pData->m_double )
          {
            SEND_DEVICE_ERR( "MatVisSetCheckPropertyEx: "
              "Prop %s double read after write problem - write=%g read=%g  " ,
              Found.name().c_str() , pData->m_double , dRead );
          }
        }
        else
        {
          PropF.writeS( pData->m_szString );
          string ReadValue = PropF.readS();
          if ( ReadValue != pData->m_szString )
          {
            SEND_DEVICE_ERR( "MatVisSetCheckPropertyEx: "
              "Prop %s enumd read after write problem - write=%s read=%s" ,
              Found.name().c_str() , pData->m_szString ,
              ReadValue.c_str() );
          }

        }
        return true;
      }
    case ctPropString:
      {
        PropertyS PropS( Found.hObj() );
        PropS.write( pData->m_szString );
        string ReadValue = PropS.read();
        if ( ReadValue != pData->m_szString )
        {
          SEND_DEVICE_ERR( "MatVisSetCheckPropertyEx: "
            "Prop %s string read after write problem - write=%s read=%s" ,
            Found.name().c_str() , pData->m_szString ,
            ReadValue.c_str() );
        }

        return true;
      }
      break;
    default: ASSERT( 0 ); break;
    }
  }
  catch ( const EPropertyHandling& e )
  {
    SEND_DEVICE_ERR( "MatVisSetAndCheckPropertyEx by name: Prop %s set exception: %s  " ,
      Found.name().c_str() , e.getErrorString().c_str() );
  }
  return false;
}


bool MatVis::GetROI( CRect& rc )
{
  if ( !m_pDevice )
    return false;

  rc.left = GetXOffset();
  rc.top = GetYOffset();
  rc.right = (LONG) GetWidth();
  rc.bottom = (LONG) GetHeight();
  return true;
}

void MatVis::SetROI( CRect& rc )
{
  if ( !m_pDevice )
    return;
  SetCamPropertyData Data;
  Data.m_Type = ctPropInt64;
  CRect CurrROI = m_CurrentROI ;

  Data.m_int64 = Data.m_int = 0 ;
  SetFeatureIntValue( _T( "Camera/GenICam/ImageFormatControl/OffsetX" ) , 0 ) ;
  SetFeatureIntValue( _T( "Camera/GenICam/ImageFormatControl/OffsetY" ) , 0 ) ;

//   MatVisSetAndCheckPropertyEx(
//     _T( "Camera/GenICam/ImageFormatControl/OffsetX" ) , &Data );
//   MatVisSetAndCheckPropertyEx(
//     _T( "Camera/GenICam/ImageFormatControl/OffsetY" ) , &Data );
// 
//   if ( rc.Width() > (int)GetMaxWidth() )
//     rc.right = GetMaxWidth() ;
  Data.m_int64 = Data.m_int = rc.Width();
  SetWidth( rc.Width() ) ;
//   MatVisSetAndCheckPropertyEx(
//     _T( "Camera/GenICam/ImageFormatControl/Width" ) , &Data );

//   if ( rc.left + rc.Width() > (int) GetMaxWidth() )
//     rc.left = 0 ;
  Data.m_int64 = Data.m_int = rc.left;
  SetFeatureIntValue( _T( "Camera/GenICam/ImageFormatControl/OffsetX" ) , Data.m_int64 ) ;
//   MatVisSetAndCheckPropertyEx(
//     _T( "Camera/GenICam/ImageFormatControl/OffsetX" ) , &Data );

//   if ( rc.Height() > (int) GetMaxHeight() )
//     rc.right = GetMaxHeight() ;
  Data.m_int64 = Data.m_int = rc.Height();
  SetHeight( rc.Height() ) ;
//   MatVisSetAndCheckPropertyEx(
//     _T( "Camera/GenICam/ImageFormatControl/Height" ) , &Data );
//   if ( rc.top + rc.Height() > (int) GetMaxHeight() )
//     rc.top = 0 ;
  Data.m_int64 = Data.m_int = rc.top;
  SetFeatureIntValue( _T( "Camera/GenICam/ImageFormatControl/OffsetY" ) , Data.m_int64 ) ;
//   MatVisSetAndCheckPropertyEx(
//     _T( "Camera/GenICam/ImageFormatControl/OffsetY" ) , &Data );

  m_CurrentROI = rc;
  __int64 iReadBH , iReadBV , iReadDH , iReadDV ;
  bool bHor = GetFeatureIntValue( "Camera/GenICam/ImageFormatControl/BinningHorizontal" , iReadBH ) ;
  bool bVert = GetFeatureIntValue( "Camera/GenICam/ImageFormatControl/BinningVertical" , iReadBV ) ;
  bHor = GetFeatureIntValue( "Camera/GenICam/ImageFormatControl/DecimationHorizontal" , iReadDH ) ;
  bVert = GetFeatureIntValue( "Camera/GenICam/ImageFormatControl/DecimationVertical" , iReadDV ) ;

  TRACE( "   SetROI (%d,%d,%d,%d) BH=%d BV=%d DH=%d DV=%d\n" ,
    rc.left , rc.top , rc.Width() , rc.Height() ,
    (int)iReadBH , (int)iReadBV , (int)iReadDH , (int)iReadDV ) ;
}

bool MatVis::SetStrobe( const FXString& StrobeDataAsText , int iIndex )
{
  //       if ( !m_pGUICamera )
  //         return false;
  // 
  //       StrobeControl SControl ;
  //       int iDelay , iDuration ;
  //       if ( sscanf( (LPCTSTR)StrobeDataAsText , _T("%d,%d,%d,%d") ,
  //         &SControl.onOff , &SControl.polarity , 
  //         &iDelay , &iDuration ) == 4 )
  //       {
  //         SControl.delay = (float)((double)iDelay * 1.e-6) ;
  //         SControl.duration = (float)((double)iDuration * 1.e-6) ;
  //         SControl.source = iIndex ;
  //         m_LastError = m_pGUICamera->SetStrobe( &SControl ) ;
  //         if ( m_LastError != VmbErrorSuccess )
  //         {
  //           SEND_DEVICE_ERR("Set Strobe %d error: %s",iIndex,m_LastError.GetDescription( ));
  //           return false ;
  //         }
  //         return true ;
  //       }

  return false;
}

void MatVis::GetStrobe( FXString& StrobeDataAsText , int iIndex )
{
  //       if ( !m_pGUICamera )
  //         return ;
  // 
  //       StrobeControl SControl ;
  //       SControl.source = iIndex ;
  //       m_LastError = m_pGUICamera->GetStrobe( &SControl ) ;
  //       if ( m_LastError != VmbErrorSuccess )
  //       {
  //         SEND_DEVICE_ERR("Get Strobe error: %s",m_LastError.GetDescription( ));
  //       }
  //       else
  //       {
  //         StrobeDataAsText.Format( _T("%d,%d,%d,%d") ,
  //           SControl.onOff , SControl.polarity , 
  //           _ROUND(SControl.delay * 1.e6) , _ROUND( SControl.duration * 1.e6) ) ;
  //       }
}

void MatVis::GetCamResolutionAndPixelFormat(
  unsigned int* rows , unsigned int* cols , TImageBufferPixelFormat* pixelFmt )
{
  //       if ( !m_pGUICamera )
  //         return ;
  // 
  //       // get the current source-image settings
  //       Error error;
  //       VideoMode videoMode;
  //       FrameRate frameRate;
  //       CameraInfo camInfo;
  // 
  //       error = m_pGUICamera->GetCameraInfo(&camInfo);
  // 
  //       if (camInfo.interfaceType == INTERFACE_GIGE)
  //       {
  //         GigECamera* gigeCam = (GigECamera*)(m_pGUICamera);
  //         GigEImageSettings gigeImageSettings;
  //         error = gigeCam->GetGigEImageSettings(&gigeImageSettings);
  //         *cols = gigeImageSettings.width;
  //         *rows = gigeImageSettings.height;
  //         *pixelFmt = gigeImageSettings.pixelFormat;
  //       }
  //       else
  //       {
  //         Camera* pCam = static_cast<Camera*>(m_pGUICamera);
  //         error = pCam->GetVideoModeAndFrameRate(&videoMode, &frameRate);
  // 
  //         bool isStippled = false;
  // 
  //         if (videoMode == VIDEOMODE_FORMAT7)
  //         {
  //           Format7ImageSettings f7ImageSettings;
  //           unsigned int packetSize;
  //           float percentage;
  // 
  //           error = pCam->GetFormat7Configuration(&f7ImageSettings, &packetSize, &percentage);
  // 
  //           *cols = f7ImageSettings.width;
  //           *rows = f7ImageSettings.height;
  //           *pixelFmt = f7ImageSettings.pixelFormat;
  //         }
  //         else
  //         {
  //           // if white balance property is present then stippled is true. This detects
  //           // when camera is in Y8/Y16 and raw bayer output is enabled
  //           PropertyInfo propInfo;
  //           propInfo.type = WHITE_BALANCE;
  // 
  //           m_pGUICamera->GetPropertyInfo(&propInfo);
  // 
  //           if (propInfo.present)
  //           {
  //             isStippled = true;
  //           }
  // 
  //           if (!GetPixelFormatFromVideoMode(videoMode, isStippled, pixelFmt))
  //           {
  //             *pixelFmt = PIXEL_FORMAT_RAW8;
  //           }
  //           GetDimensionsFromVideoMode(videoMode, rows, cols);
  //         }
  //       }
}


unsigned int MatVis::GetBppFromPixelFormat( TImageBufferPixelFormat pixelFormat )
{
  switch ( pixelFormat )
  {
  case ibpfRaw:
  case ibpfMono8:
    return 8;
    break;
  case ibpfMono12Packed_V2:
  case ibpfMono12Packed_V1:
    return 12;
    break;
  case ibpfMono10:
  case ibpfMono12:
  case ibpfMono14:
  case ibpfMono16:
  case ibpfYUV422Packed:
    return 16;
    break;
  case ibpfYUV444Packed:
  case ibpfBGR888Packed:
    //    case VmbPixelFormatBgr8:
  case ibpfRGB888Packed:
    return 24;
    //    case VmbPixelFormatRgba8:
  case ibpfRGBx888Packed:
  case ibpfMono32:
    return 32;
  case ibpfRGB101010Packed:
  case ibpfRGB121212Packed:
  case ibpfRGB141414Packed:
  case ibpfRGB161616Packed:
    return 48;
  default:
    return 0;
    break;
  }
}

void MatVis::LogError( LPCTSTR Msg )
{
  FXString GadgetName;
  GetGadgetName( GadgetName );
  FxSendLogMsg( MSG_ERROR_LEVEL , GetDriverInfo() , 0 ,
    _T( "%s - %s" ) , (LPCTSTR) GadgetName , Msg );
}

void MatVis::LocalStreamStart()
{
  CamCNTLDoAndWait( MatVis_EVT_START_GRAB );
}
void MatVis::LocalStreamStop()
{
  CamCNTLDoAndWait( MatVis_EVT_STOP_GRAB );
}


bool MatVis::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXAutolock al( m_SettingsLock , "ScanProperties" );
  double dStart = GetHRTickCount();
  m_bInScanProperties = true;
  m_bWasStopped = m_bShouldBeReprogrammed = false;

  DriverInit();
  FXString tmpS;
  FXPropertyKit pc( text );
  unsigned camSN = 0;
  if ( pc.GetInt( "Camera" , (int&) camSN ) )
  {
    if ( camSN && (camSN != 0xffffffff) )
    {
      unsigned newCamnmb = SerialToNmb( camSN );;
      if ( newCamnmb < m_CamerasOnBus )
      {
        if ( (m_dwSerialNumber != camSN) || (newCamnmb != m_CurrentCamera) )
        {
          m_bWasStopped = IsRunning();
          if ( m_dwSerialNumber && (m_dwSerialNumber != 0xffffffff) )
          {
            //                 bool bRes =CamCNTLDoAndWait( MatVis_EVT_RELEASE , 2000) ;
            m_dwSerialNumber = 0;
            m_dwConnectedSerialNumber = 0;
          }
          //           ASSERT( !m_pDevice ) ;
          m_dwSerialNumber = camSN;
          m_CurrentCamera = newCamnmb;
          if ( m_dwSerialNumber )
          {
            OtherThreadCameraInit();
            m_bShouldBeReprogrammed = m_bWasStopped;
          }
        }
        else
        {
          bool bCamConnected = CheckAndAllocCamera();
          if ( !bCamConnected )
          {
            m_bInScanProperties = false;
            return false;
          }
        }
      }
      else
      {
        m_dwSerialNumber = camSN;
        m_CurrentCamera = -1;
      }
      Invalidate |= true; //update setup
    }
    else
    {
      if ( m_dwSerialNumber && (m_dwSerialNumber != 0xffffffff) )
      {
        //            bool bRes =CamCNTLDoAndWait( MatVis_EVT_RELEASE , 2000) ;
        CameraStop();
        CameraClose();
        m_dwSerialNumber = 0;
        m_dwConnectedSerialNumber = 0;

        Invalidate = true;
      }
      //       ASSERT( bRes ) ;
    }
  }
  if ( DriverValid() )
  {
    if ( !m_pDevice )
      OtherThreadCameraInit() ;
    //     if ( pc.GetInt( "StreamState" , m_bLocalStopped ) )
    //     {
    //       if ( m_bLocalStopped )
    //         OtherThreadLocalStop() ;
    //       else if ( m_bRun )
    //         OtherThreadLocalStart() ;
    //     };
    if ( m_pDevice )
    {
//       FXString NewROI ;
//       int iNewBinning ;
//       int iNewDecimation ;
//       bool bROIChanged = pc.GetString( "ROI" , NewROI ) ;
//       bool bBinningChanged = pc.GetInt( "Binning" , iNewBinning ) ;
//       bool bDecimationChanged = pc.GetInt( "Decimation" , iNewDecimation ) ;
//       if ( bROIChanged || bBinningChanged || bDecimationChanged )
//       {
// 
//       }
// 
// 
// 
      for ( int i = 0; i < m_PropertiesEx.GetCount(); i++ )
      {
        CameraAttribute& Prop = m_PropertiesEx.GetAt( i );
        FXString key , sValue;
        FXParser param;
        Prop.m_DlgFormat.GetElementNo( 0 , key , param );
        if ( (key == SETUP_COMBOBOX) || (key == SETUP_SPIN) ) // ints result
        {
          FXString sValue;
          FXSIZE value; // what will be passed for property set
          bool bauto = false;
          if ( pc.GetString( Prop.m_Name , sValue ) )
          {
            switch ( Prop.m_Type )
            {
            case ctPropInt:
            case ctPropInt64:
              {
                if ( sValue != _T( "auto" ) )
                  value = atoi( (LPCTSTR) sValue );
                else
                {
                  bauto = true;
                  value = Prop.m_intVal;
                }
              }
              break;
            case ctPropFloat:
              {
                if ( sValue != _T( "auto" ) )
                  value = atoi( (LPCTSTR) sValue );
                else
                {
                  bauto = true;
                  value = ROUND( Prop.m_dVal );
                }
              }
              break;
            case ctPropString:
              {
                value = (FXSIZE) ((LPCTSTR) sValue);
              }
              break;
              //             case VmbFeatureDataBool:
              //               value = atoi( ( LPCTSTR ) sValue ) != 0 ;
              //               break ;
            }

            if ( !SetCameraProperty( i , value , bauto , Invalidate ) )
              //           if (!OtherThreadSetProperty( i , &Data , &Invalidate ))
            {
              SEND_DEVICE_ERR( "Can't set property %s" , (LPCTSTR) Prop.m_Name );
            }
          }
        }
        else if ( key == SETUP_SPINABOOL )
        {
          FXString tmpS;
          if ( pc.GetString( (LPCTSTR) Prop.m_Name , sValue ) )
          {
            FXSIZE value;
            bool bauto = (sValue.CompareNoCase( "auto" ) == 0);
            if ( !bauto )
              value = atoi( sValue );

            if ( !SetCameraProperty( i , value , bauto , Invalidate ) )
              //           if (!OtherThreadSetProperty( i , &Data , &Invalidate ))
            {
              SEND_DEVICE_ERR( "Can't set property %s" , (LPCTSTR) Prop.m_Name );
            }
          }
        }
        else if ( key == SETUP_EDITBOX )
        {
          FXSIZE value;
          bool bauto = false;
          if ( pc.GetString( (LPCTSTR) Prop.m_Name , sValue ) )
          {
            value = (FXSIZE) ((LPCTSTR) sValue);
            if ( !SetCameraProperty( i , value , bauto , Invalidate ) )
            {
              SEND_DEVICE_ERR( "Can't set prop %s to %s" , (LPCTSTR) Prop.m_Name , (LPCTSTR) sValue );
            }
          }
        }
        else
        {
          SEND_DEVICE_ERR( "Unsupported key '%s'in scanproperty" , key );
        }
      }
    }
  }
  if ( Invalidate && m_SetupObject )
  {
    if ( m_bShouldBeReprogrammed )
    {
      m_bShouldBeReprogrammed = false ;
      CamCNTLDoAndWait( MatVis_EVT_INIT , 2000 );
      if ( m_bWasStopped )
      {
        OtherThreadCameraStart();
        m_bWasStopped = false ;
      }
    }
    m_dLastBuiltPropertyTime = 0.;
    OtherThreadBuildPropertyList();
    m_LastPrintedProperties.Empty();
    m_LastPrintedSettings.Empty();
  }

  if ( m_bShouldBeReprogrammed )
  {
    CamCNTLDoAndWait( MatVis_EVT_INIT , 2000 );
    if ( m_bWasStopped )
      OtherThreadCameraStart();
  }
  m_bInScanProperties = false;
  FXString GadgetName;
  GetGadgetName( GadgetName );
  double dBusyTime = GetHRTickCount() - dStart;
  TRACE( "\nMatVis::ScanProperties %s: Start %g , Busy %g" , (LPCTSTR) GadgetName ,
    dStart , dBusyTime );
  return true;
}

void MatVis::OnBusArrival( void* pParam , LPCTSTR szSerNum )
{
  MatVis* pMatVis = static_cast<MatVis*>(pParam);
  if ( pMatVis->m_szSerialNumber == szSerNum )
  {
    pMatVis->m_BusEvents |= BUS_EVT_ARRIVED;
    SetEvent( pMatVis->m_evCameraControl );
    FxSendLogMsg( MSG_WARNING_LEVEL ,
      pMatVis->GetDriverInfo() , 0 , "Camera %u(%s) is connected" ,
      pMatVis->m_dwSerialNumber , szSerNum );
    TRACE( "\nBus Arrival for Camera #%u(%s) " , pMatVis->m_dwSerialNumber , szSerNum );
  }

  pMatVis->m_bRescanCameras = true;
  pMatVis->m_dwNArrivedEvents++;
}

void MatVis::OnBusRemoval( void* pParam , LPCTSTR szSerNum )
{
  MatVis* pMatVis = static_cast<MatVis*>(pParam);
  if ( pMatVis->m_szSerialNumber == szSerNum )
  {
    pMatVis->m_BusEvents |= BUS_EVT_REMOVED;
    SetEvent( pMatVis->m_evCameraControl );
    FxSendLogMsg( MSG_WARNING_LEVEL ,
      pMatVis->GetDriverInfo() , 0 , "Used Camera %u(%s) is disconnected" ,
      pMatVis->m_dwSerialNumber , szSerNum );
    TRACE( "\nCamera #%u(%s) Removed\n" , pMatVis->m_dwSerialNumber , szSerNum );
  }
}


void MatVis::OnBusReset( void* pParam , LPCTSTR szSerNum )
{
  MatVis* pMatVis = static_cast<MatVis*>(pParam);
  if ( pMatVis->m_szSerialNumber == szSerNum )
  {
    //     pMatVis->m_BusEvents |= BUS_EVT_RESET ;
    //     SetEvent( pMatVis->m_evCameraControl ) ;
    FxSendLogMsg( MSG_WARNING_LEVEL ,
      pMatVis->GetDriverInfo() , 0 , "Bus Reset for Camera %u(%s)" ,
      pMatVis->m_dwSerialNumber , szSerNum );
    TRACE( "\nBus Reset for Camera #%u(%s) \n" , pMatVis->m_dwSerialNumber , szSerNum );
  }
  pMatVis->m_bCamerasEnumerated = false;
  //   bool bRes = pMatVis->CamCNTLDoAndWait( BUS_EVT_BUS_RESET | MatVis_EVT_INIT , 2000)  ;
  pMatVis->m_bRescanCameras = true;
  //   ASSERT( bRes ) ;
}



bool MatVis::CheckAndAllocCamera( void )
{
  if ( !m_pDevice )
  {
    if ( !m_dwSerialNumber || m_dwSerialNumber == (-1) )
      return false;
    if ( !OtherThreadCameraInit() )
      return false;
  }
  return true;
}


bool MatVis::SetBMIH( LPCTSTR pPixelFormat )
{
  m_BMIH.biSize = sizeof( BITMAPINFOHEADER );
  m_BMIH.biWidth = m_CurrentROI.Width();
  if ( m_BMIH.biWidth == -1 || m_BMIH.biWidth == 0 )
    m_BMIH.biWidth = m_SensorSize.cx;
  m_BMIH.biHeight = m_CurrentROI.Height();
  if ( m_BMIH.biHeight == -1 || m_BMIH.biHeight == 0 )
    m_BMIH.biHeight = m_SensorSize.cy;

  m_BMIH.biPlanes = 1;
  m_nPixelFormat = GetDecodedPixelFormat( pPixelFormat );
  switch ( m_nPixelFormat )
  {
  case ibpfRaw:
  case ibpfMono8:
    m_BMIH.biCompression = BI_Y8;
    m_BMIH.biBitCount = 8;
    m_BMIH.biSizeImage = m_BMIH.biWidth*m_BMIH.biHeight;
    break;
  case ibpfYUV422Packed:
    m_BMIH.biCompression = BI_YUV9;
    m_BMIH.biBitCount = 12;
    m_BMIH.biSizeImage = 9 * m_BMIH.biWidth*m_BMIH.biHeight / 8;
    break;
  case ibpfMono10:
  case ibpfMono12:
  case ibpfMono14:
  case ibpfMono16:
  case ibpfMono12Packed_V2:
  case ibpfMono12Packed_V1:
    m_BMIH.biCompression = BI_Y16;
    m_BMIH.biBitCount = 16;
    m_BMIH.biSizeImage = 2 * m_BMIH.biWidth*m_BMIH.biHeight;
    break;
  case ibpfRGB888Packed:
  case ibpfRGBx888Packed:
  case ibpfBGR888Packed:   // the same  case VmbPixelFormatRgba8 :
    m_BMIH.biCompression = 0;
    m_BMIH.biBitCount = 24;
    m_BMIH.biSizeImage = 3 * m_BMIH.biWidth*m_BMIH.biHeight;
    break;
    //         PIXEL_FORMAT_422YUV8      /**< YUV 4:2:2. */
    //         PIXEL_FORMAT_444YUV8      /**< YUV 4:4:4. */
    //         PIXEL_FORMAT_RGB8         /**< R = G = B = 8 bits. */
    //         PIXEL_FORMAT_RAW16        /**< 16 bit raw data output of sensor. */
    //         PIXEL_FORMAT_MONO12       /**< 12 bits of mono information. */
    //         PIXEL_FORMAT_RAW12        /**< 12 bit raw data output of sensor. */
    //         PIXEL_FORMAT_BGR          /**< 24 bit BGR. */
    //         PIXEL_FORMAT_BGRU         /**< 32 bit BGRU. */
    //         PIXEL_FORMAT_RGB          _RGB8, /**< 24 bit RGB. */
    //         PIXEL_FORMAT_RGBU         /**< 32 bit RGBU. */
    //         PIXEL_FORMAT_BGR16        /**< R = G = B = 16 bits. */
    //         PIXEL_FORMAT_BGRU16       /**< 64 bit BGRU. */
    //         PIXEL_FORMAT_422YUV8_JPEG /**< JPEG compressed stream. */
  case ibpfRGB161616Packed:
  case ibpfRGB141414Packed:
  case ibpfRGB121212Packed:
  case ibpfRGB101010Packed:
    m_BMIH.biCompression = BI_RGB48;
    m_BMIH.biBitCount = 48;
    m_BMIH.biSizeImage = 6 * m_BMIH.biWidth*m_BMIH.biHeight;
    break;

  default:
    m_BMIH.biSizeImage = 0;
    TRACE( "!!! Unsupported format #%d\n" , m_pixelFormat );
    SEND_DEVICE_ERR( "!!! Unsupported format #%d" , m_pixelFormat );
    return false;
  }
  return true;
}


bool MatVis::ScanSettings( FXString& text )
{
  double dBegin = GetHRTickCount();
  FXAutolock al( m_SettingsLock , "ScanSettings" );
  if ( !m_LastPrintedSettings.IsEmpty()
    && (dBegin - m_dLastSettingsPrintTime < 100.0) )
  {
    text = m_LastPrintedSettings;
    m_dLastSettingsPrintTime = dBegin;
    return true;
  }
  m_dLastBuiltPropertyTime = 0;
  DriverInit();
  CheckAndAllocCamera();
  // Prepare cameras list
  FXString camlist( "Not Selected(-1)," ) , paramlist , tmpS;
  int iCurrentCamera = -1;
  for ( unsigned i = 0; i < m_CamerasOnBus; i++ )
  {
    TCHAR cMark = _T( '+' ); // sign, that camera is free
    if ( m_dwSerialNumber != m_CamInfo[ i ].m_dwSN )
    {
      FXAutolock al( m_ConfigLock );
      for ( int j = 0; j < m_BusyCameras.GetCount(); j++ )
      {
        if ( m_dwSerialNumber != m_BusyCameras[ j ].m_dwSerialNumber )
        {
          if ( m_CamInfo[ i ].m_dwSN == m_BusyCameras[ j ].m_dwSerialNumber )
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
      iCurrentCamera = m_CurrentCamera = i;
    }
    FXString Model = m_CamInfo[ i ].m_ModelName.c_str();
    Model.Replace( _T( '(' ) , _T( '[' ) );
    Model.Replace( _T( ')' ) , _T( ']' ) );
    tmpS.Format( "%c%d:%s(%d)" , cMark , m_CamInfo[ i ].m_dwSN ,
      (LPCTSTR) Model , m_CamInfo[ i ].m_dwSN );
    camlist += tmpS;
    if ( i < m_CamerasOnBus - 1 )
      camlist += _T( ',' );
  }
  if ( iCurrentCamera < 0 && m_dwSerialNumber && (m_dwSerialNumber != 0xffffffff) )
  {
    if ( !camlist.IsEmpty() )
      camlist += _T( ',' );
    tmpS.Format( "?%d(%d)" , m_dwSerialNumber , m_dwSerialNumber );
    camlist += tmpS;
    iCurrentCamera = m_CamerasOnBus; // first after real cameras
  }
  tmpS.Format( "ComboBox(Camera(%s))" , camlist );
  paramlist += tmpS;
  if ( m_dwSerialNumber == m_dwConnectedSerialNumber
    && (m_dwSerialNumber != 0) && (m_dwSerialNumber != -1) )
  {
    paramlist += _T( ',' );
    m_bInScanSettings = true ;
    OtherThreadBuildPropertyList();
    m_bInScanSettings = false ;
    tmpS.Format( "ComboBox(StreamState(Run(0),Idle(1)))" );
    paramlist += tmpS;
    for ( int i = 0; i < m_PropertiesEx.GetCount(); i++ )
    {
      if ( !m_PropertiesEx[ i ].m_DlgFormat.IsEmpty() )
      {
        paramlist += _T( ',' );
        paramlist += m_PropertiesEx[ i ].m_DlgFormat;
      }
    }
  }
  text.Format( "template(%s)" , paramlist );
  m_LastPrintedSettings = text;
  m_dLastSettingsPrintTime = GetHRTickCount();
  double dDutyTime = GetHRTickCount() - dBegin;
  TRACE( "\n----------Settings Printed in %g ms" , dDutyTime );
  return true;
}

bool MatVis::PrintProperties( FXString& text )
{
  double dBegin = GetHRTickCount();
  FXAutolock al( m_SettingsLock , "PrintProperties" );
  if ( !m_LastPrintedProperties.IsEmpty()
    && (dBegin - m_dLastProperiesPrintTime < 100.0) )
  {
    text = m_LastPrintedProperties;
    m_dLastProperiesPrintTime = dBegin;
    return true;
  }
  FXPropertyKit pc;
  if ( DriverValid() && (m_dwSerialNumber != 0)
    && (m_dwSerialNumber != -1) )
  {
    pc.WriteInt( "Camera" , m_dwSerialNumber );
    pc.WriteInt( "StreamState" , m_bLocalStopped );
    if ( m_dwConnectedSerialNumber != 0 )
    {
      for ( int i = 0; i < m_PropertiesEx.GetCount(); i++ )
      {
        FXSIZE value;
        bool bauto;
        if ( GetCameraProperty( i , value , bauto ) )
        {
          FXString key;
          FXParser param;
          CameraAttribute& Prop = m_PropertiesEx.GetAt( i );
          Prop.m_DlgFormat.GetElementNo( 0 , key , param );
          if ( key == SETUP_SPIN ) // ints result
          {
            //             if ( Prop.m_StringEnums.GetCount() > 1 )
            //             {
            //               pc.WriteString( Prop.m_Name , Prop.m_StringEnums[value] ) ;
            //             }
            //             else
            int DictSize = (int) Prop.m_StringEnums.size();
            if ( DictSize <= 0 || DictSize <= value )
              pc.WriteInt( Prop.m_Name , (int) value );
            else  // commented Moisey 23.04.19
              pc.WriteInt( Prop.m_Name , (int) Prop.m_int64Enums[ value ] );
          }
          else if ( key == SETUP_SPINABOOL )
          {
            FXString tmpS;
            if ( bauto )
              pc.WriteString( Prop.m_Name , "auto" );
            else
            {
              int DictSize = (int) Prop.m_StringEnums.size();
              if ( DictSize <= 0 || DictSize <= value )
                pc.WriteInt( Prop.m_Name , (int) value );
              else  // commented Moisey 23.04.19
                pc.WriteInt( Prop.m_Name , (int) Prop.m_int64Enums[ value ] );
            }
          }
          else if ( key == SETUP_COMBOBOX ) // ints result
          {
            //             if ( Prop.m_StringEnums.GetCount() > 1 )
            //             {
            //               pc.WriteString( Prop.m_Name , Prop.m_StringEnums[value] ) ;
            //             }
            //             else
            pc.WriteInt( Prop.m_Name , (int) value );
            //             int DictSize = Prop.m_StringEnums.GetCount() ;
            //             if ( DictSize <= 0  ||  DictSize <= value )
            //             else
            //               pc.WriteString( Prop.m_Name , Prop.m_StringEnums[value] ) ;
          }
          else if ( key == SETUP_EDITBOX )
          {
            FXString svalue = (LPCTSTR) value;
            pc.WriteString( m_PropertiesEx[ i ].m_Name , svalue );
          }
          else
          {
            SEND_DEVICE_ERR( "Unsupported key '%s'in scanproperty" , key );
          }
        }
      }
    }
  }
  else
  {
    pc.WriteInt( "Camera" , -1 );
  }
  CGadget::PrintProperties( pc );
  m_LastPrintedProperties = text = pc;
  m_dLastProperiesPrintTime = GetHRTickCount();
  double dDutyTime = GetHRTickCount() - dBegin;
  TRACE( "\n----------Properties Printed in %g ms" , dDutyTime );
  return true;
}


int MatVis::SetPacketSize( int iPacketSize_Or_FPSx10 , bool bFrameRate )
{
  if ( !m_pDevice )
    return 0;

  return -1;
}
void MatVis::SendValueToLog()
{
  FXString Addition;

  m_LogLock.Lock();
  if ( m_iLogCnt >= 0 && m_PropertiesForLog.GetCount() )
  {
    SetCamPropertyData Value;
    if ( GetPropertyValue( m_PropertiesForLog[ m_iLogCnt ] , Value ) )
    {
      if ( m_iLogCnt == 0 )
      {
        double dT = GetHRTickCount();
        Addition.Format( " %8d.%3d" , (int) (dT / 1000.) , (int) fmod( dT , 1000. ) );
        m_LogOutString += Addition;
      }
      switch ( Value.m_Type )
      {
      case ctPropInt:
        Addition.Format( " %d" , (int) Value.m_int );
        break;
      case ctPropInt64:
        Addition.Format( " %d" , (int) Value.m_int64 );
        break;
      case ctPropFloat:
        Addition.Format( " %g" , Value.m_double );
        break;
      case ctPropString:
        Addition.Format( " %s" , Value.m_szString );
        break;
        //         case VmbFeatureDataBool:
        //           Addition.Format( " %s" , Value.m_bBool ? "true" : "false" ) ;
        //           break ;
      }
      if ( Addition.IsEmpty() )
        Addition.Format( " Can't show property %s" , (LPCTSTR) m_PropertiesForLog[ m_iLogCnt ] );
      m_LogOutString += Addition;
    }
  }

  if ( m_PropertiesForTemp.GetCount() )
  {
    FXString TemperatureView;
    for ( int i = 0; i < m_PropertiesForTemp.GetCount(); i++ )
    {
      SetCamPropertyData Value;
      if ( GetPropertyValue( m_PropertiesForTemp[ m_iLogCnt ] , Value ) )
      {
        switch ( Value.m_Type )
        {
        case ctPropInt:
          Addition.Format( " %d" , (int) Value.m_int );
          break;
        case ctPropInt64:
          Addition.Format( " %d" , (int) Value.m_int64 );
          break;
        case ctPropFloat:
          Addition.Format( " %g" , Value.m_double );
          break;
        case ctPropString:
          Addition.Format( " %s" , Value.m_szString );
          break;
          //         case VmbFeatureDataBool:
          //           Addition.Format( " %s" , Value.m_bBool ? "true" : "false" ) ;
          //           break ;
        }
        if ( Addition.IsEmpty() )
          Addition.Format( " Can't show property %s" , (LPCTSTR) m_PropertiesForLog[ m_iLogCnt ] );
        TemperatureView += Addition;
      }
    }
    if ( !TemperatureView.IsEmpty() )
      m_PropertiesForTempAsString = TemperatureView;
  }
  if ( ++m_iLogCnt >= m_PropertiesForLog.GetCount() )
  {
    m_LogOutString += "\n";
    m_iLogCnt = 0;
    CTextFrame * pOut = CTextFrame::Create( m_LogOutString );
    pOut->SetLabel( "Log" );
    pOut->SetTime( m_dLastLogTime_ms = GetHRTickCount() );
    PutFrame( m_pLogOutput , pOut );
    m_LogOutString.Empty();
    m_BusEvents &= ~MatVis_EVT_LOG;
  };

  m_LogLock.Unlock();
}


DWORD WINAPI MatVis::CameraControlLoop( LPVOID pParam )
{
  TRACE( "---------Entry to CameraControlLoop\n" );
  MatVis * pMatVis = (MatVis*) pParam;

  double dLastTemperatureMeasurementTime = GetHRTickCount();
  int iTemperatureMeasurementIndex = 0;

  FXString csMessage;
  BOOL isCorruptFrame = FALSE;
  unsigned int cols = 0 , rows = 0 , colsPrev = 0 , rowsPrev = 0;
  DWORD dwWaitRes = 0;
  pMatVis->m_BusEvents |= MatVis_EVT_DRIVER_INIT;
  SetEvent( pMatVis->m_evCameraControl );
  //pMatVis->m_hCameraControlThreadHandle = ::GetCurrentThread() ;
  //pMatVis->m_dwCameraControlThreadId = ::GetCurrentThreadId() ;
  //pMatVis->RegisterCallbacks() ;
  // Start of main grab loop
  DWORD dwWaitTimeout = 5000;
  int iNLogRequests = (int) pMatVis->m_PropertiesForLog.GetCount();
  if ( ROUND( pMatVis->m_dLogPeriod_ms ) >= 100 )
  {
    dwWaitTimeout = ROUND( pMatVis->m_dLogPeriod_ms / iNLogRequests );
  }

  while ( pMatVis->m_bContinueCameraControl )
  {
    dwWaitRes = WaitForMultipleObjects( ARR_SIZE( pMatVis->m_WaitEventBusChangeArr ) ,
      pMatVis->m_WaitEventBusChangeArr , FALSE ,
      (!(pMatVis->m_bInitialized)) ? 500 : dwWaitTimeout );
    if ( dwWaitRes == WAIT_FAILED )  // gadget deleted
    {
      DWORD dwError = GetLastError();
      break;
    }
    if ( dwWaitRes == WAIT_OBJECT_0 ) // some control commands for camera (m_evCameraControl event)
    {
      DWORD InitialBusEvents = pMatVis->m_BusEvents;
      int iNInits = 0;
      while ( pMatVis->m_BusEvents )
      {
        FXAutolock al( pMatVis->m_LocalConfigLock , "CameraControlLoop1" );
        if ( !pMatVis->m_dwSerialNumber || (pMatVis->m_dwSerialNumber == 0xffffffff) )
        {
          if ( !(pMatVis->m_BusEvents & (MatVis_EVT_SHUTDOWN | MatVis_EVT_DRIVER_INIT)) )
            break;
        }
        if ( pMatVis->m_BusEvents & MatVis_EVT_SHUTDOWN )
        {
          if ( pMatVis->m_pDevice )
          {
            pMatVis->MatVisCameraStop();
            pMatVis->MatVisCameraClose();
            //pMatVis->m_pCamera = NULL ;
          }
          pMatVis->m_BusEvents = 0;
          pMatVis->m_bContinueCameraControl = false;
          TRACE( "\nCamera #%u Shut downed" , pMatVis->m_dwSerialNumber );
          Sleep( 100 );
          break;
        }
        if ( pMatVis->m_BusEvents & MatVis_EVT_DRIVER_INIT )
        {
          pMatVis->m_BusEvents &= ~(MatVis_EVT_DRIVER_INIT);
          bool bRes = pMatVis->MatVisDriverInit();
          TRACE( "\nDriver %s intialized in CameraControlLoop" , bRes ? "" : "NOT" );
          //           break ;
        }
        if ( pMatVis->m_BusEvents & MatVis_EVT_RESTART )
        {
          pMatVis->m_BusEvents &= ~(MatVis_EVT_RESTART);
          pMatVis->MatVisCameraClose();
          TRACE( "\nCamera #%u closed in CameraControlLoop" , pMatVis->m_dwSerialNumber );
          pMatVis->m_BusEvents |= MatVis_EVT_INIT;
          //           break ;
        }
        if ( pMatVis->m_BusEvents & (BUS_EVT_REMOVED | BUS_EVT_BUS_RESET) )
        {
          pMatVis->m_pDevice->close();
          if ( pMatVis->m_BusEvents & BUS_EVT_BUS_RESET )
          {
            pMatVis->m_BusEvents |= MatVis_EVT_INIT;
            TRACE( "\nCamera #%u Bus Reset" , pMatVis->m_dwSerialNumber );
          }
          if ( pMatVis->m_BusEvents & BUS_EVT_REMOVED )
          {
            TRACE( "\nCamera #%u Removed and closed" , pMatVis->m_dwSerialNumber );
          }

          pMatVis->m_BusEvents &= ~((BUS_EVT_REMOVED | BUS_EVT_BUS_RESET));
        }
        if ( (pMatVis->m_BusEvents & MatVis_EVT_START_GRAB)
          && !pMatVis->m_bInitialized )
        {
          pMatVis->m_BusEvents |= MatVis_EVT_INIT;
        }
        if ( pMatVis->m_BusEvents & (MatVis_EVT_INIT | MatVis_EVT_BUILD_PROP) )
        {
          if ( pMatVis->m_dwSerialNumber &&  pMatVis->m_dwSerialNumber != (-1) )
          {
            pMatVis->m_bInitialized = false ;
            pMatVis->MatVisCameraInit();
            if ( pMatVis->m_bInitialized  )
            {
              pMatVis->m_BusEvents &= ~(MatVis_EVT_BUILD_PROP);
              if ( !pMatVis->m_bInScanProperties && !pMatVis->m_bInScanSettings && pMatVis->m_bRun )
              pMatVis->m_BusEvents |= MatVis_EVT_START_GRAB;
          }
          }
          pMatVis->m_BusEvents &= ~(MatVis_EVT_INIT/* | MatVis_EVT_BUILD_PROP*/);
        }
        if ( pMatVis->m_BusEvents & MatVis_EVT_BUILD_PROP )
        {
          if ( pMatVis->m_dwSerialNumber )
          {
            pMatVis->MatVisBuildPropertyList();
            //             if ( pMatVis->m_bInitialized  &&  pMatVis->m_bRun )
            //               pMatVis->m_BusEvents |= MatVis_EVT_START_GRAB ;
          }
          pMatVis->m_BusEvents &= ~(MatVis_EVT_BUILD_PROP);
        }
        if ( pMatVis->m_BusEvents & MatVis_EVT_START_GRAB )
        {
          if ( pMatVis->m_dwSerialNumber )
          {
            if ( !(pMatVis->m_pDevice) )
            {
              if ( !pMatVis->MatVisCameraInit() )
                break; // will be reinitialized above
            }
            if ( !pMatVis->MatVisCameraStart() )
            {
              //              FXString Msg ;
              //               Msg.Format("Start Failure: %s", error.GetDescription());
              //               pMatVis->LogError( Msg ) ;
              pMatVis->MatVisCameraClose();
              if ( !pMatVis->MatVisCameraInit() )
              {
                pMatVis->m_BusEvents &= ~(MatVis_EVT_START_GRAB);
                break; // will not be started on next loop above
              }
            }
            else
            {
              TRACE( "\n%f MatVis::CameraControlLoop - Camera %u started" ,
                GetHRTickCount() , pMatVis->m_dwSerialNumber );
              pMatVis->m_BusEvents &= ~(MatVis_EVT_START_GRAB);
            }
          }
        }
        if ( pMatVis->m_BusEvents & MatVis_EVT_STOP_GRAB )
        {
          pMatVis->MatVisCameraStop();
          pMatVis->m_BusEvents &= ~(MatVis_EVT_STOP_GRAB);
        }
        if ( pMatVis->m_BusEvents & MatVis_EVT_RELEASE )
        {
          pMatVis->MatVisCameraStop();
          pMatVis->MatVisCameraClose();
          pMatVis->m_CurrentCamera = -1;
          pMatVis->m_BusEvents &= ~(MatVis_EVT_RELEASE);
          //           break ;
        }
        if ( pMatVis->m_BusEvents & MatVis_EVT_LOCAL_STOP )
        {
          pMatVis->MatVisCameraStop();
          pMatVis->m_bLocalStopped = true;

          pMatVis->m_BusEvents &= ~(MatVis_EVT_LOCAL_STOP);
          //          break ;
        }
        if ( pMatVis->m_BusEvents & MatVis_EVT_LOCAL_START )
        {
          pMatVis->MatVisCameraStart();
          pMatVis->m_bLocalStopped = false;
          pMatVis->m_BusEvents &= ~(MatVis_EVT_LOCAL_START);
          //          break ;
        }
        if ( pMatVis->m_BusEvents & BUS_EVT_ARRIVED )
        {
          pMatVis->m_BusEvents |= MatVis_EVT_INIT;
          pMatVis->m_BusEvents &= ~(BUS_EVT_ARRIVED);
        }
        if ( pMatVis->m_BusEvents & MatVis_EVT_SET_PROP )
        {
          double dSetStart = GetHRTickCount();
          //           TRACE("\n%f SetPropertyEx called" , GetHRTickCount() ) ;
          bool bRes = pMatVis->SetCameraPropertyEx( pMatVis->m_iPropertyIndex ,
            &pMatVis->m_PropertyData , pMatVis->m_bInvalidate );
          pMatVis->m_BusEvents &= ~(MatVis_EVT_SET_PROP);
          TRACE( "\n   %g SetPropertyEx %s for %s" , GetHRTickCount() - dSetStart ,
            (bRes) ? "OK" : "FAULT" ,
            pMatVis->m_PropertiesEx[ pMatVis->m_iPropertyIndex ].m_CameraPropertyName );
        }
        if ( pMatVis->m_BusEvents & MatVis_EVT_GET_PROP )
        {
          pMatVis->GetCameraPropertyEx( pMatVis->m_iPropertyIndex ,
            &pMatVis->m_PropertyData );
          pMatVis->m_BusEvents &= ~(MatVis_EVT_GET_PROP);
        }
        if ( pMatVis->m_BusEvents & MatVis_EVT_LOG )
        {
          if ( pMatVis->IsRunning() )
          {
            pMatVis->SendValueToLog();
            if ( pMatVis->m_iLogCnt == 0 )
            {
              int iNLogRequests = (int) pMatVis->m_PropertiesForLog.GetCount();
              if ( ROUND( pMatVis->m_dLogPeriod_ms ) >= 100 )
              {
                double dNextLogTime = pMatVis->m_dLastLogTime_ms + pMatVis->m_dLogPeriod_ms;
                double dTimeToNextLog = dNextLogTime - GetHRTickCount() - 10.;
                dwWaitTimeout = ROUND( dTimeToNextLog > 0. ? dTimeToNextLog : 1. );
              }
              else
                dwWaitTimeout = 5000;
            }
          }
          else
            pMatVis->m_BusEvents &= ~MatVis_EVT_LOG;
        }
        if ( pMatVis->m_BusEvents & ~MatVis_EVT_LOG )
        {
          if ( InitialBusEvents )
            SetEvent( pMatVis->m_evControlRequestFinished );
        }
        if ( pMatVis->m_BusEvents & MatVis_EVT_SET_SOFT_TRIGGER )
        {
          pMatVis->MatVisSetSoftwareTriggerMode( pMatVis->m_PropertyData.m_bBool );
          pMatVis->m_BusEvents &= ~MatVis_EVT_SET_SOFT_TRIGGER;
        }
      }

      if ( InitialBusEvents )
        SetEvent( pMatVis->m_evControlRequestFinished );
    }
    else if ( dwWaitRes == WAIT_OBJECT_0 + 2 ) // Bus changes (camera inserted/removed)
    {

    }
    else if ( dwWaitRes == WAIT_OBJECT_0 + 3 )
    {
      //       AVT::VmbAPI::FeaturePtr pCommandFeature;
      //       if ( VmbErrorSuccess == pMatVis->m_pCamera->GetFeatureByName(
      //         "TriggerSoftware" , pCommandFeature ) )
      //       {
      //         if ( VmbErrorSuccess == pCommandFeature->RunCommand() )
      //         {
      //         }
      //         else
      //         {
      //         }
      //       }
    }
    if ( pMatVis->m_bRun  && pMatVis->m_bContinueCameraControl )
    {
    }
    if ( pMatVis->m_bInitialized  &&  pMatVis->m_pDevice
      && pMatVis->m_dLogPeriod_ms > 0. && pMatVis->IsRunning() )
    {
      if ( GetHRTickCount() - pMatVis->m_dLastLogTime_ms > pMatVis->m_dLogPeriod_ms )
      {
        pMatVis->m_BusEvents |= MatVis_EVT_LOG;
        SetEvent( pMatVis->m_evCameraControl );
      }
      else
      {
        double dNextLogTime = pMatVis->m_dLastLogTime_ms + pMatVis->m_dLogPeriod_ms;
        double dTimeToNextLog = dNextLogTime - GetHRTickCount() - 10.;
        dwWaitTimeout = ROUND( dTimeToNextLog > 0. ? dTimeToNextLog : 1. );
      }
    }
  }

  TRACE( "---MatVis Normal Exit from Camera Control Thread for #%u\n" , pMatVis->m_dwSerialNumber );
  return 0;
}

bool MatVis::SetSoftwareTriggerMode( bool bSet )
{
  return OtherThreadSetSoftwareTrigger( bSet );
}

bool MatVis::MatVisSetSoftwareTriggerMode( bool bSet )
{
  SetCamPropertyData Data;
  bool bInvalidate = false;
  if ( bSet ) //switch to software trigger mode
  {
    if ( MatVisGetCameraPropertyEx( _T( "TriggerMode" ) , &Data ) )
    {
      if ( _tcscmp( Data.m_szString , _T( "On" ) ) == 0 ) // trigger was on
      {
        if ( MatVisGetCameraPropertyEx( _T( "TriggerSource" ) , &Data ) )
        {
          m_bSoftwareTriggerMode = true;
          if ( _tcscmp( Data.m_szString , _T( "Software" ) ) == 0 ) // was software trigger
            m_bHardwareTrigger = false;
          else // Hardware trigger,  system will work by hardware
            m_bHardwareTrigger = true; // trigger and will pass one frame per
          // frame on gadget input pin
          return true;
        }
        return false; // can't read trigger source
      }
      else // trigger was off; it's simplest case
      {
        _tcscpy( Data.m_szString , _T( "On" ) );
        if ( MatVisSetCameraPropertyEx( _T( "TriggerMode" ) , &Data ) )
        {
          _tcscpy( Data.m_szString , _T( "SoftwareTrigger" ) );

          bool bRes = MatVisSetCameraPropertyEx( _T( "TriggerSource" ) , &Data );
          if ( bRes )
          {
            m_bSoftwareTriggerMode = true;
            m_bHardwareTrigger = false;
            return true;
          }
        }
        return false;
      }
    }
  }
  else        // remove software trigger mode
  {

  }

  return true;
}

bool MatVis::OtherThreadGetProperty( int iIndex , SetCamPropertyData * pData ,
  TCHAR * pName )
{
  m_iPropertyIndex = iIndex;
  if ( pName )
    strcpy_s( m_TmpPropertyName , pName );
  else
    m_TmpPropertyName[ 0 ] = 0;

  m_bInvalidate = false;
  bool bRes = CamCNTLDoAndWait( MatVis_EVT_GET_PROP );
  *pData = m_PropertyData;
  return bRes;
}


bool MatVis::FormDialogData( mvIMPACT::acquire::Property& Comp , CameraAttribute& CompValues )
{
  CompValues.m_Type = Comp.type();
  bool bHasStep = Comp.hasStepWidth();
  bool bHasMinMax = Comp.hasMinValue() && Comp.hasMaxValue();
  int iDictionarySize = Comp.dictSize();
  CompValues.m_DisplayName = Comp.displayName().c_str();
  FXString Name = Comp.name().c_str() ;
  std::string Descr = Comp.docString();
  size_t Len = Descr.size();
  if ( Len )
  {
    size_t iPos = 0;
    while ( (iPos = Descr.find( _T( "\n" ) , iPos )) != string::npos )
    {
      if ( iPos < Len - 2 )
        Descr.insert( ++iPos , 1 , '\t' );
      else
        break;
    }
  }
  CompValues.m_Description = Descr.c_str();
  if ( !Comp.isProp() )
    return false ;
  if ( !Comp.isValid() || !Comp.isVisible() )
  {
    CompValues.m_FDescription = CompValues.m_DisplayName + _T(":  ") + Descr.c_str() ;
    CompValues.m_FDescription += !Comp.isValid() ? "Not Valid, " : "Valid, " ;
    CompValues.m_FDescription += !Comp.isVisible() ? "Not Visible" : "Visible" ;
    return false ;
  }
  switch ( CompValues.m_Type )
  {
  case ctPropInt64:
    {

      PropertyI64 PropI64( Comp.hObj() );
      // now the property is ready to use when available:
      if ( PropI64.isValid() /*&& PropI64.isVisible()*/ )
      {

        // find the feature and bind it to the property object
        bool bRes = false;
        CompValues.m_intVal =
          (int) (CompValues.m_int64Val = PropI64.read());
        if ( bHasMinMax )
        {
          CompValues.m_i64Range[ 0 ] = PropI64.getMinValue();
          CompValues.m_i64Range[ 1 ] = PropI64.getMaxValue();
          CompValues.m_bHasMinMax = bHasMinMax;
        }
        if ( iDictionarySize > 1 )
        {
          CompValues.m_DlgFormat.Format( _T( "ComboBox(%s(" ) ,
            (LPCTSTR) (!CompValues.m_Name.IsEmpty()? CompValues.m_Name : Comp.name().c_str() ));
          CompValues.m_AllEnumeratorsAsString = _T( '[' );

          std::vector< std::pair<std::string , __int64 > > dict;
          int i = 0;
          try
          {
            PropI64.getTranslationDict( dict );
            for ( std::pair<std::string , __int64 > iter : dict )
            {
              FXString Addition;
              CompValues.m_StringEnums.push_back( iter.first );
              CompValues.m_int64Enums.push_back( iter.second ); // commented Moisey 23.04.19
              if ( CompValues.m_int64Val == iter.second )
                CompValues.m_enumVal = iter.first.c_str();
              Addition.Format( _T( "%s(%d)" ) , iter.first.c_str() , i );
              CompValues.m_DlgFormat += Addition;
              CompValues.m_DlgFormat +=
                (i < iDictionarySize - 1) ? _T( ',' ) : _T( ')' );
              Addition.Format( _T( "%s(%I64d)" ) ,
                iter.first.c_str() , iter.second );
              CompValues.m_AllEnumeratorsAsString += Addition;
              CompValues.m_AllEnumeratorsAsString +=
                (i++ < iDictionarySize - 1) ? _T( ',' ) : _T( ']' );
            };
            CompValues.m_DlgFormat += _T( ')' );
            CompValues.m_FDescription += CompValues.m_DisplayName + _T( ":  " ) ;
            CompValues.m_FDescription += (Descr + _T( "\n         Enumerators:" )).c_str() ;
            CompValues.m_FDescription += CompValues.m_AllEnumeratorsAsString ;
          }
          catch ( EPropertyHandling& e )
          {
            LPCTSTR pErr = e.getErrorString().c_str();
            TRACE( "\nFormDialogData: Exception in %s Enum %d Read: %s  " ,
              (LPCTSTR) CompValues.m_CameraPropertyName , i , pErr );
            return false;
          }
        }
        else if ( bHasMinMax )
        {
          CompValues.m_DlgFormat.Format( _T( "Spin(%s,%I64i,%I64i)" ) ,
            (LPCTSTR) CompValues.m_Name ,
            CompValues.m_i64Range[ 0 ] , CompValues.m_i64Range[ 1 ] );
          CompValues.m_FDescription.Format(
            "%s(%s)=%I64i Int64 [%I64i,%I64i]\n\tDescription: %s;" ,
            (LPCTSTR) CompValues.m_CameraPropertyName ,
            (LPCTSTR) CompValues.m_DisplayName , CompValues.m_int64Val ,
            CompValues.m_i64Range[ 0 ] ,
            CompValues.m_i64Range[ 1 ] , Descr.c_str() );
        }
        else if ( iDictionarySize <= 1 )
        {
          CompValues.m_DlgFormat.Format( _T( "Spin(%s,%I64i,%I64i)" ) ,
            (LPCTSTR) CompValues.m_Name ,
            CompValues.m_i64Range[ 0 ] = 0 ,
            CompValues.m_i64Range[ 1 ] = 1000 );
          CompValues.m_FDescription.Format(
            "%s(%s)=%I64i Int64 \n\t    Description: %s;" ,
            (LPCTSTR) CompValues.m_CameraPropertyName ,
            (LPCTSTR) CompValues.m_DisplayName ,
            CompValues.m_int64Val , Descr.c_str() );
        }
        return true;
      }
      else
        return false;
    }
    break;
  case ctPropInt:
    {

      PropertyI PropI( Comp.hObj() );
      // now the property is ready to use when available:
      if ( PropI.isValid() /*&& PropI.isVisible()*/ )
      {

        // find the feature and bind it to the property object
        bool bRes = false;
        CompValues.m_int64Val = CompValues.m_intVal = PropI.read();
        if ( bHasMinMax )
        {
          CompValues.m_i64Range[ 0 ] = PropI.getMinValue();
          CompValues.m_i64Range[ 1 ] = PropI.getMaxValue();
          CompValues.m_bHasMinMax = bHasMinMax;
        }
        if ( iDictionarySize > 1 )
        {
          CompValues.m_DlgFormat.Format( _T( "ComboBox(%s(" ) ,
            (LPCTSTR) (!CompValues.m_Name.IsEmpty() ? CompValues.m_Name : Comp.name().c_str()) );   
          CompValues.m_AllEnumeratorsAsString = _T( '[' );
          std::vector< std::pair<std::string , int > > dict;
          int i = 0;
          try
          {
            PropI.getTranslationDict( dict );
            for ( std::pair<std::string , int > iter : dict )
            {
              FXString Addition;
              CompValues.m_StringEnums.push_back( iter.first );
              CompValues.m_intEnums.push_back( iter.second );  // commented Moisey 23.04.19
              if ( CompValues.m_intVal == iter.second )
                CompValues.m_enumVal = iter.first.c_str();
              Addition.Format( _T( "%s(%d)" ) , iter.first.c_str() , i );
              CompValues.m_DlgFormat += Addition;
              CompValues.m_DlgFormat +=
                (i++ < iDictionarySize - 1) ? _T( ',' ) : _T( ')' );
              Addition.Format( _T( "%s(%d)" ) ,
                iter.first.c_str() , iter.second );
              CompValues.m_AllEnumeratorsAsString += Addition;
              CompValues.m_AllEnumeratorsAsString +=
                (i++ < iDictionarySize - 1) ? _T( ',' ) : _T( ']' );
            };
            CompValues.m_DlgFormat += _T( ')' );
            CompValues.m_FDescription += CompValues.m_DisplayName + _T( ":  " ) ;
            CompValues.m_FDescription += _T( "\n     Enumerators:" ) ;
            CompValues.m_FDescription += CompValues.m_AllEnumeratorsAsString ;
          }
          catch ( const EPropertyHandling& e )
          {
            LPCTSTR pErr = e.getErrorString().c_str();
            TRACE( "\nFormDialogData: Exception in %s Enum %d Read: %s  " ,
              (LPCTSTR) CompValues.m_CameraPropertyName , i , pErr );
            return false;
          }
        }
        else if ( bHasMinMax )
        {
          CompValues.m_DlgFormat.Format( _T( "Spin(%s,%I64i,%I64i)" ) ,
            (LPCTSTR) CompValues.m_Name ,
            CompValues.m_i64Range[ 0 ] , CompValues.m_i64Range[ 1 ] );
          CompValues.m_FDescription.Format(
            "%s(%s)=%i Int64 [%I64i,%I64i]\n\t    Description: %s;" ,
            (LPCTSTR) CompValues.m_CameraPropertyName ,
            (LPCTSTR) CompValues.m_DisplayName , CompValues.m_intVal ,
            CompValues.m_i64Range[ 0 ] ,
            CompValues.m_i64Range[ 1 ] , Descr.c_str() );
        }
        else if ( iDictionarySize <= 1 )
        {
          CompValues.m_DlgFormat.Format( _T( "Spin(%s,%I64i,%I64i)" ) ,
            (LPCTSTR) CompValues.m_Name ,
            CompValues.m_i64Range[ 0 ] = 0 ,
            CompValues.m_i64Range[ 1 ] = 1000 );
          CompValues.m_FDescription.Format(
            "%s(%s)=%i Int \n\t    Description: %s;" ,
            (LPCTSTR) CompValues.m_CameraPropertyName ,
            (LPCTSTR) CompValues.m_DisplayName , CompValues.m_intVal ,
            Descr.c_str() );
        }
        return true;
      }
      else
        return false;
    }
    break;
  case ctPropFloat:
    {

      PropertyF PropF( Comp.hObj() );
      // now the property is ready to use when available:
      if ( PropF.isValid() )
      {

        // find the feature and bind it to the property object
        bool bRes = false;
        CompValues.m_dVal = PropF.read();
        CompValues.m_bHasMinMax = bHasMinMax;
        if ( bHasMinMax )
        {
          CompValues.m_dRange[ 0 ] = PropF.getMinValue();
          CompValues.m_dRange[ 1 ] = PropF.getMaxValue();
        }
        CompValues.m_FDescription = CompValues.m_DisplayName + _T( ":  " ) + Descr.c_str() ;
        if ( iDictionarySize > 1 )
        {
          CompValues.m_DlgFormat.Format( _T( "ComboBox(%s(" ) ,
            (LPCTSTR) (!CompValues.m_Name.IsEmpty() ? CompValues.m_Name : Comp.name().c_str()) );
          CompValues.m_AllEnumeratorsAsString = _T( '[' );
          std::vector< std::pair<std::string , double > > dict;
          int i = 0;
          try
          {
            PropF.getTranslationDict( dict );
            for ( std::pair<std::string , double > iter : dict )
            {
              FXString Addition;
              CompValues.m_StringEnums.push_back( iter.first );
              CompValues.m_DblEnums.push_back( iter.second );  // commented Moisey 23.04.19
              if ( CompValues.m_dVal == iter.second )
                CompValues.m_enumVal = iter.first.c_str();
              Addition.Format( _T( "%s(%d)" ) , iter.first.c_str() , i );
              CompValues.m_DlgFormat += Addition;
              CompValues.m_DlgFormat +=
                (i++ < iDictionarySize - 1) ? _T( ',' ) : _T( ')' );
              Addition.Format( _T( "%s(%g)" ) ,
                iter.first.c_str() , iter.second );
              CompValues.m_AllEnumeratorsAsString += Addition;
              CompValues.m_AllEnumeratorsAsString +=
                (i++ < iDictionarySize - 1) ? _T( ',' ) : _T( ']' );
            };
            CompValues.m_DlgFormat += _T( ')' );
            CompValues.m_FDescription += CompValues.m_DisplayName + _T( ":  " ) ;
            CompValues.m_FDescription += _T( "\n     Enumerators:" ) ;
            CompValues.m_FDescription += CompValues.m_AllEnumeratorsAsString ;
          }
          catch ( const EPropertyHandling& e )
          {
            LPCTSTR pErr = e.getErrorString().c_str();
            TRACE( "\nFormDialogData: Exception in %s Enum %d Read: %s  " ,
              (LPCTSTR) CompValues.m_Name , i , pErr );
            return false;
          }
        }
        switch ( CompValues.pr )
        {
        case FGP_EXTSHUTTER:
          CompValues.m_DlgFormat.Format( _T( "Spin(%s,%I64i,%I64i)" ) ,
            (LPCTSTR) CompValues.m_Name ,
            (CompValues.m_i64Range[ 0 ] = (__int64) CompValues.m_dRange[ 0 ]) ,
            (CompValues.m_i64Range[ 1 ] = (__int64) CompValues.m_dRange[ 1 ]) );

          break;
        case FGP_GAIN:
          CompValues.m_DlgFormat.Format( _T( "Spin(%s,%I64i,%I64i)" ) ,
            (LPCTSTR) CompValues.m_Name ,
            (CompValues.m_i64Range[ 0 ] = (__int64) (CompValues.m_dRange[ 0 ] * 10.)) ,
            (CompValues.m_i64Range[ 1 ] = (__int64) (CompValues.m_dRange[ 1 ] * 10.)) );
          break;
        default:
          CompValues.m_DlgFormat.Format( _T( "EditBox(%s)" ) ,
            (LPCTSTR) CompValues.m_Name );
          break;
        }
        if ( bHasMinMax )
        {
          CompValues.m_FDescription.Format( "%s(%s)=%g Float [%g,%g]\n\tDescription: %s;" ,
            (LPCTSTR) CompValues.m_CameraPropertyName ,
            (LPCTSTR) CompValues.m_DisplayName , CompValues.m_dVal ,
            CompValues.m_dRange[ 0 ] ,
            CompValues.m_dRange[ 1 ] , Descr.c_str() );
        }
        else if ( iDictionarySize <= 1 )
        {
          CompValues.m_FDescription.Format( "%s(%s)=%g Float\n\tDescription: %s;" ,
            (LPCTSTR) CompValues.m_CameraPropertyName ,
            (LPCTSTR) CompValues.m_DisplayName ,
            CompValues.m_dVal ,
            Descr.c_str() );
        }
        return true;
      }
      else
        return false;
    }
    break;
  case ctPropString:
    {
      PropertyS PropS( Comp.hObj() );
      // now the property is ready to use when available:
      if ( PropS.isValid() /*&& PropS.isWriteable() */ )
      {

        // find the feature and bind it to the property object
        bool bRes = false;
        CompValues.m_stringVal = PropS.read().c_str();
        CompValues.m_bHasMinMax = false;
        if ( iDictionarySize > 1 )
        {
          TRACE( "\nFormDialogData: String property %s has dictionary %d " ,
            (LPCTSTR) CompValues.m_CameraPropertyName , iDictionarySize );
          ASSERT( 0 );
          return false;
        }
        else
        {

          CompValues.m_DlgFormat.Format( _T( "EditBox(%s)" ) ,
            (LPCTSTR) CompValues.m_Name );
          CompValues.m_FDescription.Format( "%s(%s)=%s ; %s" ,
            (LPCTSTR) CompValues.m_CameraPropertyName ,
            (LPCTSTR) CompValues.m_DisplayName ,
            CompValues.m_stringVal , Descr.c_str() );
        }
        return true;
      }
      else
        return false;
    }
    break;
  case ctPropPtr:
    {
      PropertyPtr PropPtr( Comp.hObj() );
      if ( PropPtr.isValid() )
      {
        CompValues.m_stringVal = PropPtr.representationAsString().c_str();
        CompValues.m_FDescription.Format( "%s(%s)=%s Ptr %s" ,
          (LPCTSTR) CompValues.m_CameraPropertyName ,
          (LPCTSTR) CompValues.m_DisplayName ,
          (LPCTSTR) CompValues.m_stringVal , Descr.c_str() );
        return true;
      }
    }
    break;
  default:
    ASSERT( 0 );
    break;
  }
  return false;
}

FXString MatVis::DecodeEvtMask( DWORD dwEVtMask )
{
  FXString Answer;
  Answer.Format( "0x%X:" , dwEVtMask );
  if ( dwEVtMask & MatVis_EVT_SHUTDOWN )
    Answer += "ShutDown ";
  if ( dwEVtMask & MatVis_EVT_DRIVER_INIT )
    Answer += "DrvInit ";
  if ( dwEVtMask & MatVis_EVT_RELEASE )
    Answer += "Release ";
  if ( dwEVtMask & MatVis_EVT_RESTART )
    Answer += "Restart ";
  if ( dwEVtMask & MatVis_EVT_INIT )
    Answer += "CamInit ";
  if ( dwEVtMask & MatVis_EVT_START_GRAB )
    Answer += "Start ";
  if ( dwEVtMask & MatVis_EVT_STOP_GRAB )
    Answer += "Stop ";
  if ( dwEVtMask & MatVis_EVT_SET_PROP )
    Answer += "SetProp ";
  if ( dwEVtMask & MatVis_EVT_GET_PROP )
    Answer += "GetProp ";
  if ( dwEVtMask & MatVis_EVT_BUILD_PROP )
    Answer += "BuildProp ";
  if ( dwEVtMask & MatVis_EVT_LOCAL_START )
    Answer += "LocStart ";
  if ( dwEVtMask & MatVis_EVT_LOCAL_STOP )
    Answer += "LocStop ";
  return Answer;
}

LPCTSTR MatVis::DecodePropertyType( TComponentType Type )
{
  switch ( Type )
  {
  case ctProp: return _T( "Property" );
  case ctList: return _T( "List" );
  case ctMeth: return _T( "Method" );
  case ctPropInt: return _T( "PropInt" );
  case ctPropFloat: return _T( "PropFloat" );
  case ctPropString: return _T( "PropString" );
  case ctPropPtr: return _T( "PropPtr" );
  case ctPropInt64: return _T( "PropInt64" );
  }
  return _T( "PropUnknown" );
}
TImageBufferPixelFormat MatVis::GetDecodedPixelFormat( LPCTSTR pPixelFormatName )
{
  SetCamPropertyData Data;
  if ( !pPixelFormatName )
  {
    int iIndex = GetPropertyIndex( "PixelFormat" );
    if ( iIndex == WRONG_PROPERTY )
      return ibpfAuto;
    Data.m_Pr = FGP_IMAGEFORMAT;
    if ( OtherThreadGetProperty( iIndex , &Data ) )
      pPixelFormatName = Data.m_szString;
  }
  if ( pPixelFormatName )
  {
    FXString AsString = pPixelFormatName;
    if ( AsString == "Mono8" )
      return ibpfMono8;
    if ( AsString == "Mono10" )
      return ibpfMono10;
    if ( AsString == "Mono12" )
      return ibpfMono12;
    if ( AsString == "Mono14" )
      return ibpfMono14;
    if ( AsString == "Mono16" )
      return ibpfMono16;
    if ( AsString == "Mono12Packed" )
      return ibpfMono12Packed_V1;
    if ( AsString == "Mono12p" )
      return ibpfMono12Packed_V2;
  }
  return ibpfAuto;
}

// This function sets binning or decimation with AOI adjustment when necessary
bool MatVis::SetCheckRegionAndBinningOrDecimation(
  CRect& AOI , char * pValueAsString , bool bBinning ) // if not binning - decimation
{
  int iCoeff = (pValueAsString && (*pValueAsString) ) ? 
    atoi( pValueAsString ) : m_iCurrentBinningOrDecimation ;
  if ( iCoeff <= 0 )
    iCoeff = 1 ;
  SetBinningAndDecimation( bBinning ? iCoeff : 0 , bBinning ? 0 : iCoeff ) ;
  int iSensorWidth = GetSensorWidth();
  int iSensorHeight = GetSensorHeight();
  __int64 iMaxWidth = iSensorWidth / iCoeff ;
  __int64 iMaxHeight = iSensorHeight / iCoeff ;
  GetFeatureIntValue( "Camera/GenICam/ImageFormatControl/WidthMax" , iMaxWidth ) ;
  GetFeatureIntValue( "Camera/GenICam/ImageFormatControl/HeightMax" , iMaxHeight ) ;

  AOI = CRect( 0 , 0 , (int)iMaxWidth , (int)iMaxHeight ) ;
  SetROI( AOI );
//   if ( m_SetupObject )
//   {
//     FXString ROIAsString ;
//     ROIAsString.Format( _T( "%d,%d" ) , m_CurrentROI.Width() , m_CurrentROI.Height() ) ;
//     ((CStdSetupDlg*) m_SetupObject)->SetCellText( _T("ROI") , ROIAsString ) ;
//   }
  return true;
};

bool MatVis::GetGrabConditions(
  GrabMode& Mode , int& iNFrames )
{
  try
  {
    mvIMPACT::acquire::GenICam::AcquisitionControl ac( m_pDevice );
    std::string AcqMode = ac.acquisitionMode.readS();
    if ( ac.acquisitionMode.hasDict() )
    {
      m_AcqModesAsStrings.RemoveAll();
      int iDictSize = ac.acquisitionMode.dictSize();
      std::vector< std::string > Dict;
      ac.acquisitionMode.getTranslationDictStrings( Dict );
      for ( auto iter = Dict.begin(); iter != Dict.end(); iter++ )
      {
        m_AcqModesAsStrings.Add( iter->c_str() );
      }
    }
    if ( AcqMode == (LPCTSTR) m_AcqModesAsStrings[ 0 ] )
    {
      Mode = GM_Continuous;
      iNFrames = -1;
    }
    else
    {
      if ( AcqMode != (LPCTSTR) m_AcqModesAsStrings[ 1 ] )
        ac.acquisitionMode.writeS( (LPCTSTR) m_AcqModesAsStrings[ 1 ] );
      Mode = GM_Multi;
      __int64 i64NFrames = ac.acquisitionFrameCount.read();
      iNFrames = (int) i64NFrames;
    }
    return true;
  }
  catch ( const ImpactAcquireException& e )
  {
    // this e.g. might happen if the same device is already opened in another process...
    FXString Msg;
    Msg.Format( _T( "Can't get acq mode and/or frame count ERROR %s" ) ,
      e.getErrorCodeAsString().c_str() );
    SEND_DEVICE_ERR( Msg );
  }
  return false;
}

bool MatVis::SetGrab( int iNFrames )
{
  try
  {
    m_iNFramesForGrabbing = iNFrames;
    SetEvent( m_hevGrabEvt );
    return true;
  }
  catch ( const ImpactAcquireException& e )
  {
    // this e.g. might happen if the same device is already opened in another process...
    FXString Msg;
    Msg.Format( _T( "Can't set acq mode for frame count %d ERROR %s" ) ,
      iNFrames , e.getErrorCodeAsString().c_str() );
    SEND_DEVICE_ERR( Msg );
  }
  return false;
}

bool MatVis::SetNFramesAndAcquisitionMode( int iNFrames )
{
  FXAutolock al( m_SettingsLock , "SetNFramesAndAcquisitionMode" );

  LPCTSTR pAckModeAsString = NULL;
  try
  {
    mvIMPACT::acquire::GenICam::AcquisitionControl ac( m_pDevice );
    std::string AcqMode = ac.acquisitionMode.readS();

    if ( (iNFrames < -1)
      || (iNFrames > 0xffff) )
      return false;

    if ( iNFrames == -1 )
    {
      if ( m_GrabMode != GM_Continuous )
      {
        pAckModeAsString = (LPCTSTR) m_AcqModesAsStrings[ 0 ] ;
        ASSERT( AcqMode != pAckModeAsString ) ;
        ac.acquisitionMode.writeS( pAckModeAsString );

        m_GrabMode = GM_Continuous;
      }
    }
    else
    {
      if ( (iNFrames <= 1) && (m_GrabMode != GM_Single) )
      {
        pAckModeAsString = (LPCTSTR) m_AcqModesAsStrings[ 2 ] ;
        ASSERT( AcqMode != pAckModeAsString ) ;
        ac.acquisitionMode.writeS( pAckModeAsString ) ;
        m_GrabMode = GM_Single ;
      }
      else
      {
        if ( m_GrabMode != GM_Multi )
        {
          pAckModeAsString = (LPCTSTR) m_AcqModesAsStrings[ 1 ] ;
          ASSERT( AcqMode != pAckModeAsString ) ;
          ac.acquisitionMode.writeS( pAckModeAsString );
          m_GrabMode = GM_Multi;
        }
        if ( iNFrames != 0 )
          ac.acquisitionFrameCount.write( iNFrames );
      }
    }
    return true;
  }
  catch ( const ImpactAcquireException& e )
  {
    if ( e.getErrorCode() != PROPHANDLING_NO_WRITE_RIGHTS )
    {
      FXString Msg;
      Msg.Format( _T( "Can't set acq mode %s for frame count %d ERROR %s" ) ,
        pAckModeAsString , iNFrames , e.getErrorCodeAsString().c_str() );
      SEND_DEVICE_ERR( Msg );
    }
  }
  return false;
}


int MatVis::SetBinningAndDecimation( int iBinning , int iDecimation )
{
  __int64 i64Decim = 1 , i64Bin = 1 ;
  __int64 iReadH = 0 , iReadV = 0 ;
//   SetFeatureIntValue( "Camera/GenICam/ImageFormatControl/DecimationHorizontal" , i64Decim ) ;
//   SetFeatureIntValue( "Camera/GenICam/ImageFormatControl/DecimationVertical" , i64Decim ) ;
//   SetFeatureIntValue( "Camera/GenICam/ImageFormatControl/BinningHorizontal" , i64Bin ) ;
//   SetFeatureIntValue( "Camera/GenICam/ImageFormatControl/BinningVertical" , i64Bin ) ;
  int iSensorWidth = GetSensorWidth();
  int iSensorHeight = GetSensorHeight();
//   CRect MaxROI( 0 , 0 , iSensorWidth , iSensorHeight ) ;
//   SetROI( MaxROI ) ;

  if ( iBinning )
  {
    // Set Binning
    i64Bin = iBinning ;
    iReadH = 0 , iReadV = 0 ;
    SetFeatureIntValue( "Camera/GenICam/ImageFormatControl/BinningHorizontal" , i64Bin ) ;
    SetFeatureIntValue( "Camera/GenICam/ImageFormatControl/BinningVertical" , i64Bin ) ;
    GetFeatureIntValue( "Camera/GenICam/ImageFormatControl/BinningHorizontal" , iReadH ) ;
    GetFeatureIntValue( "Camera/GenICam/ImageFormatControl/BinningVertical" , iReadV ) ;
    
    TRACE( "  Binning=%d   ReadH=%d ReadV=%d " , (int) i64Bin , (int) iReadH , (int) iReadV ) ;
  }
  if ( iDecimation )
  {
    __int64 i64Decim = iDecimation ;
    iReadH = 0 , iReadV = 0 ;
    SetFeatureIntValue( "Camera/GenICam/ImageFormatControl/DecimationHorizontal" , i64Decim ) ;
    SetFeatureIntValue( "Camera/GenICam/ImageFormatControl/DecimationVertical" , i64Decim ) ;
    GetFeatureIntValue( "Camera/GenICam/ImageFormatControl/DecimationHorizontal" , iReadH ) ;
    GetFeatureIntValue( "Camera/GenICam/ImageFormatControl/DecimationVertical" , iReadV ) ;
//     if ( i64Decim > 1 )
//     {
//       SetFeatureValueAsEnumeratorString(
//         "Camera/GenICam/ImageFormatControl/DecimationHorizontalMode" , "Average" ) ;
// //       SetFeatureValueAsEnumeratorString(
// //         "Camera/GenICam/ImageFormatControl/DecimationVerticalMode" , "Average" ) ;
//       GetFeatureIntValue( "Camera/GenICam/ImageFormatControl/DecimationHorizontalMode" , iReadH ) ;
// //       GetFeatureIntValue( "Camera/GenICam/ImageFormatControl/DecimationVerticalMode" , iReadV ) ;
//     }
    TRACE( "  Decim=%d   ReadH=%d ReadV=%d " , iDecimation , (int) iReadH , (int) iReadV ) ;
  }

  return 0;
}

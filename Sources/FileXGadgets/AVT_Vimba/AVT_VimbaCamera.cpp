// AVT_Vimba.cpp: implementation of the AVT_Vimba class.
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
#include "AVT_Vimba.h"
#include "AVT_VimbaCamera.h"
#include <video\shvideo.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "AVT_VimbaCamera.cpp"

IMPLEMENT_RUNTIME_GADGET_EX( AVT_Vimba , C1394Camera , "Video.capture" , TVDB400_PLUGIN_NAME );

CamProperty Marlin[] =
{
  { FGP_IMAGEFORMAT , "PixelFormat" , "PixelFormat" , NULL , NULL , true } ,
  { FGP_ACQU_MODE , "AcqMode" , "AcquisitionMode" , NULL , NULL , true } ,
  { FGP_GAMMA , "Gamma" , "Gamma" , NULL , NULL , false } ,
  { FGP_EXTSHUTTER , "Shutter_us" , "ExposureTime" , "ExposureAuto" , NULL , false } ,
  { FGP_AUTOEXPOSURE , "ExpTarget" , "ExposureAutoTarget" , NULL , NULL , true } ,
  { FGP_GAIN , "Gain_dBx10" , "Gain" , "GainAuto" , NULL , false } ,
  //  { FGP_GAIN , "SensorGain" , "SensorGain" , NULL , NULL , true } ,
    { FGP_ROI , "ROI" , NULL , NULL , NULL , true } ,
  { FGP_TRIGGERONOFF , "TriggerMode" , "TriggerMode" , NULL , NULL , false } ,
  { FGP_TRIGGER_SOURCE , "TriggerSource" , "TriggerSource" , NULL , NULL , false } ,
  { FGP_TRIGGERDELAY , "TriggerDelay" , "TriggerDelay" , NULL , NULL , false } ,
  { FGP_TRIGGER_POLARITY , "Tr.Polarity" , "TriggerActivation" , NULL , NULL , false } ,
  { FGP_OUT_DELAY , "OutputDelay" , "IntEnaDelayTime" , NULL , NULL , false } ,
  { FGP_FRAME_RATE , "FrameRate" , "AcquisitionFrameRate" , NULL , NULL , true } ,
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
    //   {FGP_LINE_DEBOUNCE , "Debouncing" , "LineDebounceTime" , NULL , NULL , true  } ,
    { FGP_LOG , "Log" , NULL , NULL , NULL , false }

} ;

CamProperty GuppyPro[] =
{
  { FGP_IMAGEFORMAT , "PixelFormat" , "PixelFormat" , NULL , NULL , true } ,
  { FGP_ACQU_MODE , "AcqMode" , "AcquisitionMode" , NULL , NULL , true } ,
  { FGP_EXTSHUTTER , "Shutter_us" , "ExposureTime" , "ExposureAuto"/*"ExposureMode"*/ , NULL , false } ,
  { FGP_AUTOEXPOSURE , "ExpTarget" , "ExposureAutoTarget" , NULL , NULL , false } ,
  { FGP_GAIN , "Gain_dBx10" , "Gain" , "GainAuto" , NULL , false } ,
  //   { FGP_GAIN , "SensorGain" , "SensorGain" , NULL , NULL , true } ,
    { FGP_GAMMA , "Gamma" , "Gamma" , NULL , NULL , false } ,
    { FGP_ROI , "ROI" , NULL , NULL , NULL , true } ,
    { FGP_TRIGGERONOFF , "TriggerMode" , "TriggerMode" , NULL , NULL , true } ,
    { FGP_TRIGGER_SOURCE , "TriggerSource" , "TriggerSource" , NULL , NULL , true } ,
    { FGP_TRIGGERDELAY , "TriggerDelay" , "TriggerDelay" , NULL , NULL , false } ,
  { FGP_TRIGGER_POLARITY , "Tr.Polarity" , "TriggerActivation" , NULL , NULL , true } ,
  { FGP_OUT_DELAY , "OutputDelay" , "IntEnaDelayTime" , NULL , NULL , false } ,
  { FGP_FRAME_RATE , "FrameRate" , "AcquisitionFrameRate" , NULL , NULL , true } ,
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
} ;

CamProperty Guppy[] =
{
  { FGP_IMAGEFORMAT , "PixelFormat" , "PixelFormat" , NULL , NULL , true } ,
  { FGP_ACQU_MODE , "AcqMode" , "AcquisitionMode" , NULL , NULL , true } ,
  { FGP_GAMMA , "Gamma" , "Gamma" , NULL , NULL , false } ,
  { FGP_EXTSHUTTER , "Shutter_us" , "ExposureTime" , "ExposureAuto" , NULL , false } ,
  { FGP_AUTOEXPOSURE , "ExpTarget" , "ExposureAutoTarget" , NULL , NULL , false } ,
  { FGP_GAIN , "Gain_dBx10" , "Gain" , "GainAuto" , NULL , false } ,
  //  { FGP_GAIN , "SensorGain" , "SensorGain" , NULL , NULL , true } ,
    { FGP_ROI , "ROI" , NULL , NULL , NULL , true } ,
  { FGP_TRIGGERONOFF , "TriggerMode" , "TriggerMode" , NULL , NULL , false } ,
  { FGP_TRIGGER_SOURCE , "TriggerSource" , "TriggerSource" , NULL , NULL , false } ,
    { FGP_TRIGGERDELAY , "TriggerDelay" , "TriggerDelay" , NULL , NULL , false } ,
    { FGP_TRIGGER_POLARITY , "Tr.Polarity" , "TriggerActivation" , NULL , NULL , false } ,
    { FGP_OUT_DELAY , "OutputDelay" , "IntEnaDelayTime" , NULL , NULL , false } ,
  { FGP_FRAME_RATE , "FrameRate" , "AcquisitionFrameRate" , NULL , NULL , true } ,
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
    { FGP_LOG , "Log" , NULL , NULL , NULL , false }
} ;

CamProperty GoldEye[] =
{
  { FGP_IMAGEFORMAT , "PixelFormat" , "PixelFormat" , NULL , NULL , true } ,
  { FGP_ACQU_MODE , "AcqMode" , "AcquisitionMode" , NULL , NULL , true } ,
  { FGP_EXTSHUTTER , "Shutter_us" , "ExposureTime" , "ExposureAuto"/*"ExposureMode"*/ , NULL , false } ,
  { FGP_AUTOEXPOSURE , "ExpTarget" , "ExposureAutoTarget" , NULL , NULL , false } ,
  //  { FGP_GAIN , "Gain_dBx10" , "Gain" , "GainAuto" , NULL , true } ,
    { FGP_GAIN , "SensorGain" , "SensorGain" , NULL , NULL , false } ,
    { FGP_GAMMA , "Gamma" , "Gamma" , NULL , NULL , false } ,
    { FGP_ROI , "ROI" , NULL , NULL , NULL , true } ,
  { FGP_TRIGGERONOFF , "TriggerMode" , "TriggerMode" , NULL , NULL , false } ,
  { FGP_TRIGGER_SOURCE , "TriggerSource" , "TriggerSource" , NULL , NULL , false } ,
    //  {FGP_TRIG_EVENT , "TrigEvent" , "FrameStartTriggerEvent" , NULL , NULL , true },
    { FGP_TRIGGERDELAY , "TriggerDelay" , "TriggerDelay" , NULL , NULL , false } ,
    { FGP_TRIGGER_POLARITY , "Tr.Polarity" , "TriggerActivation" , NULL , NULL , false } ,
    { FGP_OUT_DELAY , "OutputDelay" , "IntEnaDelayTime" , NULL , NULL , false } ,
  { FGP_FRAME_RATE , "FrameRate" , "AcquisitionFrameRate" , NULL , NULL , true } ,
    { FGP_BANDWIDTH , "LAN_Capacity" , "StreamBytesPerSecond" , NULL , NULL , true } ,
    { FGP_WHITEBALCB , "WhiteBalBlue" , "WhitebalValueBlue" , "WhitebalMode Manual-Auto" , NULL , false } ,
    { FGP_WHITEBALCR , "WhiteBalRed" , "WhitebalValueRed" , NULL , NULL , false } ,
    { FGP_WHITEBAL_RATIO , "WB_Ratio" , "BalanceRatioRaw" , "BalanceWhiteAuto" , NULL , false } ,
    { FGP_WHITEBAL_RATIO , "WB_Ratio" , "BalanceRatioAbs" , "BalanceWhiteAuto" , NULL , false } ,
    { FGP_WHITEBAL_SELECT , "WB_Base" , "BalanceRatioSelector" , NULL , NULL , false } ,
    { FGP_GRAB , "Grab" , "AcquisitionFrameCount" , NULL , NULL , false } ,
    { FGP_LINEIN_SELECT , "LineInSelect" , "LineInSelector" , NULL , NULL , false } ,
    { FGP_LINEOUT_SELECT , "LineOutSelect" , "LineOutSelector" , NULL , NULL , false } ,
    { FGP_LINEOUT_SOURCE , "LineOutSource" , "LineOutSource" , NULL , NULL , false } ,
    { FGP_LINE_INVERSE , "LineOutInverter" , "LineOutPolarity" , NULL , NULL , false } ,
    { FGP_LINE_DEBOUNCE , "Debouncing" , "LineInGlitchFilter" , NULL , NULL , false } ,
    { FGP_USER_SET_SELECT , "SaveLoad" , "UserSetSave" , "UserSetLoad" , NULL , true } , // UI, SaveCommand, LoadCommand
    // the next is not for UI, only info for operations with sets
    // Name for selector, Command for set 
    { FGP_USER_SET_DEF , "UserSetSelector" , "UserSetDefaultSelector" , "Enum" , NULL , true } ,
    { FGP_TEMPERATURE_S , "Temperature" , "DeviceTemperature" , "DeviceTemperatureSelector" , NULL , false } ,
    { FGP_LOG , "Log" , NULL , NULL , NULL , false }

};

CamProperty Manta[] =
{
  { FGP_IMAGEFORMAT , "PixelFormat" , "PixelFormat" , NULL , NULL , true } ,
  { FGP_ACQU_MODE , "AcqMode" , "AcquisitionMode" , NULL , NULL , true } ,
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
  { FGP_FRAME_RATE , "FrameRate" , "AcquisitionFrameRate" , NULL , NULL , true } ,
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

CamTypeAndProperties MarlinTP =
{
  _T( "Marlin" ) , ARR_SIZE( Marlin ) , &Marlin[ 0 ]
} ;
CamTypeAndProperties GuppyProTP =
{
  _T( "Guppy PRO" ) , ARR_SIZE( GuppyPro ) , &GuppyPro[ 0 ]
} ;

CamTypeAndProperties GuppyTP =
{
  _T( "Guppy" ) , ARR_SIZE( Guppy ) , &Guppy[ 0 ]
};

CamTypeAndProperties GoldEyeTP =
{
  _T( "Goldeye" ) , ARR_SIZE( GoldEye ) , &GoldEye[ 0 ]
} ;

CamTypeAndProperties MantaTP =
{
  _T( "Manta" ) , ARR_SIZE( Manta ) , &Manta[ 0 ]
} ;

CamTypeAndProperties * KnownCameras[] =
{&MarlinTP , &GuppyProTP , &GuppyTP , &GoldEyeTP , &MantaTP , NULL} ;

CamProperty CommonProperties[] =
{
  { FGP_IMAGEFORMAT , "PixelFormat" , "PixelFormat" , NULL , NULL , true } ,
  { FGP_ACQU_MODE , "AcqMode" , "AcquisitionMode" , NULL , NULL , true } ,
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
  { FGP_FRAME_RATE , "FrameRate" , "AcquisitionFrameRate" , NULL , NULL , true } ,
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
} ;

Videoformats vFormats[] =
{
  { VmbPixelFormatMono8 , "Mono8" , true , 1. , CM_Y8 , BI_Y8 } ,
  { VmbPixelFormatMono16 , "Mono16" , true , 2.0 , CM_Y16 , BI_Y16 } ,
  { VmbPixelFormatMono12 , "Mono12" , true , 2.0 , CM_UNKNOWN , BI_Y12 } ,
  { VmbPixelFormatMono12Packed , "Mono12Packed" , true , 1.5 , CM_UNKNOWN , BI_YP12 } ,
  { VmbPixelFormatYuv411 , "YUV411" , true , 1.5 , CM_YUV411 , BI_YUV9 } ,
  { VmbPixelFormatYuv422 , "YUV422" , false , 2.0 , CM_YUV422 , BI_YUV12 } ,
  { VmbPixelFormatYuv444 , "YUV444" , false , 3.0 , CM_YUV444 , BI_YUV12 } ,
  { VmbPixelFormatRgb8 , "RGB24" , false , 3.0 , CM_UNKNOWN , BI_YUV12 } ,
  { VmbPixelFormatRgb16 , "RGB48" , false , 6.0 , CM_UNKNOWN , BI_YUV12 } ,
  { VmbPixelFormatBgr8 , "BGR24" , false , 3.0 , CM_UNKNOWN , BI_YUV12 } ,
  { VmbPixelFormatRgba8 , "RGBA32" , false , 4.0 , CM_UNKNOWN , BI_YUV12 } ,
  { VmbPixelFormatBgra8 , "BGRA32" , false , 4.0 , CM_UNKNOWN , BI_YUV12 } ,
  { VmbPixelFormatMono12Packed , "Mono12Packed" , true , 1.5 , CM_UNKNOWN , BI_Y16 } ,
  { VmbPixelFormatBayerGR12Packed , "Bayer12Packed" , false , 1.5 , CM_UNKNOWN , BI_Y16 } ,
  { VmbPixelFormatBayerGR8 , "RAW8" , true , 1.0 , CM_RAW8 , BI_Y8 } ,
  { VmbPixelFormatBayerGR16 , "RAW16" , true , 2.0 , CM_RAW16 , BI_Y16 }
};



FXLockObject                AVT_Vimba::m_ConfigLock ;
VimbaCamInfo                AVT_Vimba::m_CamInfo[ MAX_CAMERASNMB ] ;
int                         AVT_Vimba::m_iCamNum = 0 ;
FXArray<BusyCamera>              AVT_Vimba::m_BusyCameras ;
DWORD                       AVT_Vimba::m_dwInstanceCount = 0 ;
bool                        AVT_Vimba::m_bSaveFullInfo = true ;
#ifdef _DEBUG
DWORD AVT_Vimba::m_dwDefaultTimeout = 60000 ;
#else
DWORD AVT_Vimba::m_dwDefaultTimeout = 1000 ;
#endif

int AVT_Vimba::GetPropertyIndex( LPCTSTR name )
{
  int i;
  unsigned retV = WRONG_PROPERTY ;
  for ( i = 0; i < m_PropertiesEx.GetCount() ; i++ )
  {
    FXString Prop = m_PropertiesEx[ i ].m_Name ;
    if ( _stricmp( m_PropertiesEx[ i ].m_Name , name ) == 0 )
      return i ;
  }
  return WRONG_PROPERTY;
}

int AVT_Vimba::GetPropertyIndex( FG_PARAMETER id )
{
  int i;
  unsigned retV = WRONG_PROPERTY ;
  for ( i = 0; i < m_PropertiesEx.GetCount() ; i++ )
  {
    if ( m_PropertiesEx[ i ].pr == id )
      return i ;
  }
  return WRONG_PROPERTY;
}

bool AVT_Vimba::GetULongFeature( AVT::VmbAPI::CameraPtr& CameraPtr ,
  LPCTSTR pFeatureName , VmbUint32_t& ulValue )
{
  try
  {
  AVT::VmbAPI::FeaturePtr Feature ;
  VmbErrorType Result = CameraPtr->GetFeatureByName( pFeatureName , Feature ) ;
  if ( Result == VmbErrorSuccess )
  {
    VmbFeatureDataType DataType ;
    Result = Feature->GetDataType( DataType ) ;
    if ( Result == VmbErrorSuccess )
    {
      if ( DataType == VmbFeatureDataInt )
      {
        VmbInt64_t Value ;
        Result = Feature->GetValue( Value ) ;
        if ( Result == VmbErrorSuccess )
        {
          ulValue = (VmbUint32_t) Value ;
          return true ;
        }
        else
          SEND_DEVICE_ERR( "Can't read value for %s" , pFeatureName ) ;
      }
      else
        SEND_DEVICE_ERR( "Data type for %s is %d, not Int64" , pFeatureName ) ;
    }
    else
      SEND_DEVICE_ERR( "Can't read data type for %s" , pFeatureName ) ;
  }
  else
    SEND_DEVICE_ERR( "Can't take feature %s" , pFeatureName ) ;

  }
  catch ( CException* e )
  {
    TCHAR Buf[ 1000 ] ;
    BOOL bMsgTaken = e->GetErrorMessage( Buf , 999 ) ;
    SEND_DEVICE_ERR( "AVT_Vimba::GetULongFeature Exception for Property %s - %s" ,
      pFeatureName , bMsgTaken ? Buf : _T( "Reason Unknown" ) ) ;
  }

  return false ;
}
bool AVT_Vimba::GetPropertyValue(
  AVT::VmbAPI::CameraPtr& pCam , LPCTSTR Name , SetCamPropertyData& Value )
{
  try
  {
  AVT::VmbAPI::FeaturePtr PropPtr ;
  VmbErrorType LastError = pCam->GetFeatureByName( Name , PropPtr ) ;
  if ( LastError != VmbErrorSuccess )
  {
    if ( m_bViewErrorMessagesOnGetSet )
    {
      SEND_DEVICE_ERR( "Can't get %s property: %s" , Name ,
        AVT::VmbAPI::Examples::ErrorCodeToMessage( LastError ).c_str() ) ;
    }
    return false ;
  }
  LastError = PropPtr->GetDataType( Value.m_Type ) ;
  ASSERT( LastError == VmbErrorSuccess ) ;
  switch ( Value.m_Type )
  {
    case VmbFeatureDataInt:
      LastError = PropPtr->GetValue( Value.m_int64 ) ;
      return (LastError == VmbErrorSuccess) ;
    case VmbFeatureDataFloat:
    {
      LastError = PropPtr->GetValue( Value.m_double ) ;
      return (LastError == VmbErrorSuccess) ;
    }
    case VmbFeatureDataEnum:
    case VmbFeatureDataString:
    {
      std::string sVal ;
      LastError = PropPtr->GetValue( sVal ) ;
      if ( LastError == VmbErrorSuccess )
      {
        strcpy_s( Value.m_szString , sVal.c_str() ) ;
        return true ;
      }
      return false ;
    }
    case VmbFeatureDataBool:
    {
      LastError = PropPtr->GetValue( Value.m_bBool ) ;
      return (LastError == VmbErrorSuccess) ;
    }
    default:
      SEND_DEVICE_ERR( "Unsupported property type %d for %s" , Value.m_Type , Name ) ;
      return false ;
  }

  }
  catch ( CException* e )
  {
    TCHAR Buf[ 1000 ] ;
    BOOL bMsgTaken = e->GetErrorMessage( Buf , 999 ) ;
    SEND_DEVICE_ERR( "AVT_Vimba::} Exception for Property %s - %s" ,
      Name , bMsgTaken ? Buf : _T( "Reason Unknown" ) ) ;
  }
  return false ;
}

bool AVT_Vimba::SetPropertyValue( AVT::VmbAPI::CameraPtr& pCam ,
  LPCTSTR Name , SetCamPropertyData& Value )
{
  try
  {
  AVT::VmbAPI::FeaturePtr PropPtr ;
  VmbErrorType LastError = pCam->GetFeatureByName( Name , PropPtr ) ;
  if ( LastError != VmbErrorSuccess )
  {
    SEND_DEVICE_ERR( "Can't get %s property for set operation: %s" , Name ,
      AVT::VmbAPI::Examples::ErrorCodeToMessage( LastError ).c_str() ) ;
    return false ;
  }
  VmbFeatureDataType Type ;
  LastError = PropPtr->GetDataType( Type ) ;
  ASSERT( LastError == VmbErrorSuccess ) ;
  switch ( Type )
  {
    case VmbFeatureDataInt:
      LastError = PropPtr->SetValue( (VmbInt64_t) Value.m_int ) ;
      return (LastError == VmbErrorSuccess) ;
    case VmbFeatureDataFloat:
    {
      LastError = PropPtr->SetValue( Value.m_double ) ;
      return (LastError == VmbErrorSuccess) ;
    }
    case VmbFeatureDataEnum:
    case VmbFeatureDataString:
    {
      std::string sVal ;
      LastError = PropPtr->SetValue( Value.m_szString ) ;
      return (LastError == VmbErrorSuccess)  ;
    }
    case VmbFeatureDataBool:
    {
      LastError = PropPtr->SetValue( Value.m_bBool ) ;
      return (LastError == VmbErrorSuccess) ;
    }
    case VmbFeatureDataCommand:
    {
      LastError = PropPtr->RunCommand() ;
      if ( LastError == VmbErrorSuccess )
        return true ;
      else
      {
        SEND_DEVICE_ERR( "Can't run command %s" , Name ) ;
        return false ;
      }
    }
    default:
      SEND_DEVICE_ERR( "Unsupported property type %d for %s" , Value.m_Type , Name ) ;
      return false ;
  }
  }
  catch ( CException* e )
  {
    TCHAR Buf[ 1000 ] ;
    BOOL bMsgTaken = e->GetErrorMessage( Buf , 999 ) ;
    SEND_DEVICE_ERR( "AVT_Vimba::SetPropertyValue Exception for Property %s - %s" ,
      Name , bMsgTaken ? Buf : _T( "Reason Unknown" ) ) ;
  }
  return false ;
}

bool AVT_Vimba::SetPropertyValue( LPCTSTR Name , SetCamPropertyData& Value )
{
  return SetPropertyValue( m_pCamera , Name , Value ) ;
}

bool AVT_Vimba::RunCameraCommand( LPCTSTR pCommand )
{
  SetCamPropertyData DummyData ;
  return SetPropertyValue( pCommand , DummyData ) ;
}

bool AVT_Vimba::SetULongFeature( AVT::VmbAPI::CameraPtr& CameraPtr ,
  LPCTSTR pFeatureName , VmbUint32_t ulValue )
{
  try
  {
  AVT::VmbAPI::FeaturePtr Feature ;
  VmbErrorType Result = CameraPtr->GetFeatureByName( pFeatureName , Feature ) ;
  if ( Result == VmbErrorSuccess )
  {
    VmbFeatureDataType DataType ;
    Result = Feature->GetDataType( DataType ) ;
    if ( Result == VmbErrorSuccess )
    {
      if ( DataType == VmbFeatureDataInt )
      {
        Result = Feature->SetValue( (int) ulValue ) ;
        if ( Result == VmbErrorSuccess )
          return true ;
        else
          SEND_DEVICE_ERR( "Can't set uint32 value for %s" , pFeatureName ) ;
      }
      else
        SEND_DEVICE_ERR( "Data type for %s is %d, not Int64" , pFeatureName ) ;
    }
    else
      SEND_DEVICE_ERR( "Can't read data type for %s" , pFeatureName ) ;
  }
  else
    SEND_DEVICE_ERR( "Can't take feature %s" , pFeatureName ) ;
  }
  catch ( CException* e )
  {
    TCHAR Buf[ 1000 ] ;
    BOOL bMsgTaken = e->GetErrorMessage( Buf , 999 ) ;
    SEND_DEVICE_ERR( "AVT_Vimba::SetULongFeature Exception for Property %s - %s" ,
      pFeatureName , bMsgTaken ? Buf : _T( "Reason Unknown" ) ) ;
  }

  return false ;
}

VmbErrorType AVT_Vimba::GetFeatureIntValue( const AVT::VmbAPI::CameraPtr &camera ,
  LPCTSTR pFeatureName , VmbInt64_t & value )
{
  VmbErrorType    result;
  try
  {
  if ( SP_ISNULL( camera ) )
    return VmbErrorBadParameter;
  AVT::VmbAPI::FeaturePtr      pFeature;
  result = SP_ACCESS( camera )->GetFeatureByName( pFeatureName , pFeature );
  if ( VmbErrorSuccess == result )
    result = SP_ACCESS( pFeature )->GetValue( value );
  else
    SEND_DEVICE_ERR( "Can't get feature %s" , pFeatureName ) ;
  }
  catch ( CException* e )
  {
    TCHAR Buf[ 1000 ] ;
    BOOL bMsgTaken = e->GetErrorMessage( Buf , 999 ) ;
    SEND_DEVICE_ERR( "AVT_Vimba::GetFeatureIntValue Exception for Property %s - %s" ,
      pFeatureName , bMsgTaken ? Buf : _T( "Reason Unknown" ) ) ;
  }

  return result;
}
/** write an integer feature from camera.
*/
VmbErrorType AVT_Vimba::SetFeatureIntValue( const AVT::VmbAPI::CameraPtr &camera ,
  LPCTSTR pFeatureName , VmbInt64_t value )
{
  if ( SP_ISNULL( camera ) )
    return VmbErrorBadParameter;
  AVT::VmbAPI::FeaturePtr      pFeature;
  VmbErrorType    result;
  result = SP_ACCESS( camera )->GetFeatureByName( pFeatureName , pFeature );
  if ( VmbErrorSuccess == result )
    result = SP_ACCESS( pFeature )->SetValue( value );
  else
    SEND_DEVICE_ERR( "Can't get feature %s" , pFeatureName ) ;

  return result;
}

VmbUint32_t AVT_Vimba::GetXSize( AVT::VmbAPI::CameraPtr& CameraPtr )
{
  VmbUint32_t lValue;
  if ( GetULongFeature( CameraPtr , "Width" , lValue ) )
    return lValue;
  return 0;
}
void AVT_Vimba::SetWidth( AVT::VmbAPI::CameraPtr& CameraPtr , VmbUint32_t Width )
{
  SetULongFeature( CameraPtr , "Width" , Width ) ;
}

VmbUint32_t AVT_Vimba::GetYSize( AVT::VmbAPI::CameraPtr& CameraPtr )
{
  VmbUint32_t lValue;
  if ( GetULongFeature( CameraPtr , "Height" , lValue ) )
    return lValue;
  return 0;
}
void AVT_Vimba::SetHeight( AVT::VmbAPI::CameraPtr& CameraPtr , VmbUint32_t Height )
{
  SetULongFeature( CameraPtr , "Height" , Height ) ;
}

VmbUint32_t AVT_Vimba::GetXOffset( AVT::VmbAPI::CameraPtr& CameraPtr )
{
  VmbUint32_t lValue;
  if ( GetULongFeature( CameraPtr , "OffsetX" , lValue ) )
    return lValue;
  return 0;
}
void AVT_Vimba::SetXOffset( AVT::VmbAPI::CameraPtr& CameraPtr , VmbUint32_t XOffset )
{
  SetULongFeature( CameraPtr , "OffsetX" , XOffset ) ;
}

VmbUint32_t AVT_Vimba::GetYOffset( AVT::VmbAPI::CameraPtr& CameraPtr )
{
  VmbUint32_t lValue;
  if ( GetULongFeature( CameraPtr , "OffsetY" , lValue ) )
    return lValue;
  return 0;
}
void AVT_Vimba::SetYOffset( AVT::VmbAPI::CameraPtr& CameraPtr , VmbUint32_t YOffset )
{
  SetULongFeature( CameraPtr , "OffsetY" , YOffset ) ;
}

VmbUint32_t AVT_Vimba::GetMaxWidth( AVT::VmbAPI::CameraPtr& CameraPtr )
{
  VmbUint32_t lValue;
  if ( GetULongFeature( CameraPtr , "SensorWidth" , lValue ) )
    return lValue;
  return 0;
}

VmbUint32_t AVT_Vimba::GetMaxHeight( AVT::VmbAPI::CameraPtr& CameraPtr )
{
  VmbUint32_t lValue;

  if ( GetULongFeature( CameraPtr , "SensorHeight" , lValue ) )
    return lValue;
  return 0;
}

CVideoFrame * AVT_Vimba::ConvertAVTtoSHformat(
  const AVT::VmbAPI::FramePtr pFrame , AVT_Vimba * pGadget )
{
  double dStart = GetHRTickCount() ;
  double dGetTime , dToRGBTime , dToYUV9Time ;
  CVideoFrame * pOut = NULL ;

  VmbPixelFormatType PixFormat ;
  VmbErrorType Res = pFrame->GetPixelFormat( PixFormat ) ;
  if ( Res == VmbErrorSuccess )
  {
    VmbUint32_t Lx , Ly ;
    if ( pFrame->GetWidth( Lx ) == VmbErrorSuccess
      && pFrame->GetHeight( Ly ) == VmbErrorSuccess )
    {
      unsigned iSize = Lx * Ly ;
      LPBYTE data ;
      switch ( PixFormat )
      {
        case VmbPixelFormatMono8:
        {
          LPBITMAPINFOHEADER lpBMIH = (LPBITMAPINFOHEADER) malloc( sizeof( BITMAPINFOHEADER ) + iSize );

          memcpy( lpBMIH , &pGadget->m_BMIH , sizeof( BITMAPINFOHEADER ) );
          lpBMIH->biWidth = Lx ;
          lpBMIH->biHeight = Ly ;
          lpBMIH->biSizeImage = iSize ;
          lpBMIH->biCompression = BI_Y8 ;
          pFrame->GetImage( data ) ;
          memcpy( &lpBMIH[ 1 ] , data , iSize ) ;
          CVideoFrame* vf = CVideoFrame::Create();
          vf->lpBMIH = lpBMIH ;
          vf->lpData = NULL ;
          vf->SetLabel( pGadget->m_CameraID );
          vf->SetTime( GetHRTickCount() ) ;
          pOut = vf ;
        }
        break ;
        case VmbPixelFormatMono10:
        case VmbPixelFormatMono12:
        case VmbPixelFormatMono14:
        case VmbPixelFormatMono16:
        {
          iSize *= 2 ;
          LPBITMAPINFOHEADER lpBMIH = (LPBITMAPINFOHEADER) malloc( sizeof( BITMAPINFOHEADER ) + iSize );
          memcpy( lpBMIH , &pGadget->m_BMIH , sizeof( BITMAPINFOHEADER ) );
          lpBMIH->biWidth = Lx ;
          lpBMIH->biHeight = Ly ;
          lpBMIH->biSizeImage = iSize ;
          lpBMIH->biCompression = BI_Y16 ;
          pFrame->GetImage( data ) ;
          memcpy( &lpBMIH[ 1 ] , data , iSize ) ;
          CVideoFrame* vf = CVideoFrame::Create();
          vf->lpBMIH = lpBMIH ;
          vf->lpData = NULL ;
          vf->SetLabel( pGadget->m_CameraID );
          vf->SetTime( GetHRTickCount() ) ;
          pOut = vf ;
        }
        break ;
        case VmbPixelFormatYuv411:
        {
          iSize = (3 * iSize) / 2 ;
          LPBITMAPINFOHEADER lpBMIH = (LPBITMAPINFOHEADER) malloc( sizeof( BITMAPINFOHEADER ) );
          if ( lpBMIH )
          {
            memcpy( lpBMIH , &pGadget->m_BMIH , sizeof( BITMAPINFOHEADER ) );

            lpBMIH->biWidth = Lx ;
            lpBMIH->biHeight = Ly ;
            lpBMIH->biSizeImage = iSize;
            lpBMIH->biCompression = BI_YUV411 ;
            lpBMIH->biBitCount = 12 ;
            lpBMIH->biPlanes = 1;
            if ( pFrame->GetBuffer( data ) == VmbErrorSuccess && data )
            {
              LPBITMAPINFOHEADER lpYUV9 = yuv411yuv9( lpBMIH , data ) ;
              if ( lpYUV9 )
              {
                CVideoFrame* vf = CVideoFrame::Create();
                vf->lpBMIH = lpYUV9 ;
                vf->lpData = NULL ;
                vf->SetLabel( pGadget->m_CameraID );
                vf->SetTime( GetHRTickCount() ) ;
                pOut = vf ;
              }
            }
            free( lpBMIH ) ;
          }
        }
        break ;
        case VmbPixelFormatBayerGR8:
        case VmbPixelFormatBayerRG8:
        case VmbPixelFormatBayerGB8:
        case VmbPixelFormatBayerBG8:
        {
          VmbImage Src , Dest ;
          Src.Size = Dest.Size = sizeof( Dest ) ;
          VmbErrorType Result = pFrame->GetBuffer( (VmbUchar_t *&) Src.Data ) ;
          if ( Result != VmbErrorSuccess )
          {
            SEND_DEVICE_ERR( "Can't get Data Buffer: %s " , AVT::VmbAPI::Examples::ErrorCodeToMessage( Result ) ) ;
            break ;
          }
          dGetTime = GetHRTickCount() - dStart ;
          VmbError_t Res = VmbSetImageInfoFromPixelFormat( PixFormat , Lx , Ly , &Src ) ;
          if ( Res != VmbErrorSuccess )
          {
            SEND_DEVICE_ERR( "Can't set SRC pixel format to 0x%08X: %s " ,
              PixFormat , AVT::VmbAPI::Examples::ErrorCodeToMessage( Res ) ) ;
            break ;
          }
          Res = VmbSetImageInfoFromPixelFormat( VmbPixelFormatBgr8 , Lx , Ly , &Dest ) ;
          if ( Res != VmbErrorSuccess )
          {
            SEND_DEVICE_ERR( "Can't set DEST pixel format to RGB: %s " ,
              AVT::VmbAPI::Examples::ErrorCodeToMessage( Res ) ) ;
            break ;
          }
          VmbTransformInfo info;
          // set the debayering algorithm to simple 2 by 2
          Res = VmbSetDebayerMode( VmbDebayerMode2x2 , &info );
          if ( Res != VmbErrorSuccess )
          {
            SEND_DEVICE_ERR( "Can't set debayer mode %d : %s " , VmbDebayerMode2x2 ,
              AVT::VmbAPI::Examples::ErrorCodeToMessage( Res ) ) ;
            break ;
          }
          iSize = (3 * iSize) ;
          Dest.Data = malloc( iSize * 2 ) ;
          // perform the transformation
          Res = VmbImageTransform( &Src , &Dest , &info , 1 );
          if ( Res != VmbErrorSuccess )
          {
            SEND_DEVICE_ERR( "Can't do image transformation: %s " ,
              AVT::VmbAPI::Examples::ErrorCodeToMessage( Res ) ) ;
          }
          else
          {
            dToRGBTime = GetHRTickCount() - dStart ;
            LPBITMAPINFOHEADER lpBMIH = (LPBITMAPINFOHEADER) malloc( sizeof( BITMAPINFOHEADER ) );
            if ( lpBMIH )
            {
              LPBYTE pInv = (LPBYTE) Dest.Data + iSize ;
              int iCopySize = Lx * 3 ;
              for ( unsigned i = 0 ; i < Ly ; i++ )
                memcpy( pInv + i * iCopySize , (LPBYTE) Dest.Data + (Ly - i - 1) * iCopySize , iCopySize ) ;
              memcpy( lpBMIH , &pGadget->m_BMIH , sizeof( BITMAPINFOHEADER ) );
              lpBMIH->biWidth = Lx ;
              lpBMIH->biHeight = Ly ;
              lpBMIH->biSizeImage = iSize;
              lpBMIH->biCompression = BI_RGB ;
              lpBMIH->biBitCount = 24 ;
              lpBMIH->biPlanes = 1;
              LPBITMAPINFOHEADER lpYUV9 = rgb24yuv9( lpBMIH , pInv ) ;
              if ( lpYUV9 )
              {
                CVideoFrame* vf = CVideoFrame::Create();
                vf->lpBMIH = lpYUV9 ;
                vf->lpData = NULL ;
                vf->SetLabel( pGadget->m_CameraID );
                vf->SetTime( GetHRTickCount() ) ;
                pOut = vf ;
              }
              dToYUV9Time = GetHRTickCount() - dStart ;
              free( lpBMIH ) ;
            }
          }
          free( Dest.Data ) ;
        }
        break ;
      }
    }
  }
  return pOut ;
}
// Frame completed callback executes on separate driver thread.
// One callback thread per camera. If a frame callback function has not 
// completed, and the next frame returns, the next frame's callback function is queued. 
// This situation is best avoided (camera running faster than host can process frames). 
// Spend as little time in this thread as possible and offload processing
// to other threads or save processing until later.
//
// Note: If a camera is unplugged, this callback will not get called until PvCaptureQueueClear.
// i.e. callback with pFrame->Status = ePvErrUnplugged doesn't happen -- so don't rely
// on this as a test for a missing camera. 
BOOL __stdcall FrameDoneCB( const AVT::VmbAPI::FramePtr pFrame , void * pClient )
{
  double dStart = GetHRTickCount() ;
  VmbFrameStatusType eReceiveStatus;
  AVT_Vimba * pVimba = (AVT_Vimba *) pClient ;
  CVideoFrame * pOut = NULL ;

  if ( VmbErrorSuccess == pFrame->GetReceiveStatus( eReceiveStatus ) )
  {
    if ( eReceiveStatus == VmbFrameStatusComplete )
    {
      CVideoFrame * pOut = pVimba->ConvertAVTtoSHformat( pFrame , pVimba ) ;
      if ( pOut )
      {
        if ( !pVimba->IsTriggerByInputPin() || pVimba->CheckAndResetSoftTriggerReceived() )
        {
          pVimba->m_GrabLock.Lock() ;
          pVimba->m_ReadyFrames.push( pOut ) ;
          SetEvent( pVimba->m_evFrameReady ) ;
          pVimba->m_GrabLock.Unlock() ;
        }
        else
          pOut->Release() ;
      }
    }
  }

  //   ASSERT( pVimba->m_pCamera->QueueFrame( pFrame ) == VmbErrorSuccess ) ;
  pVimba->m_dLastInCallbackTime = GetHRTickCount() - dStart ;
  return (pOut != NULL) ;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AVT_Vimba::AVT_Vimba()
  :
  m_System( AVT::VmbAPI::VimbaSystem::GetInstance() )
  , m_iNProperties( ARR_SIZE( CommonProperties ) )
{
  double dStart = GetHRTickCount() ;
  Sleep( 50 ) ;

  m_pLogOutput = new COutputConnector( text ) ;
  //m_CurrentCameraGUID=PGRGuid();
  m_dwSerialNumber = -1 ;
  m_dwConnectedSerialNumber = 0 ;
  //m_Mode=MODE_0;
  m_CurrentROI = CRect( 0 , 0 , 640 , 480 );
  m_pixelFormat = 0 ; //VmbPixelFormatMono8;
  m_uiPacketSize = 2048 ;
  m_FormatNotSupportedDispayed = false;
  m_GadgetInfo = "AVT_Vimba";
  m_WaitEventFrameArray[ 0 ] = m_evFrameReady = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
  m_WaitEventBusChangeArr[ 1 ] = m_WaitEventFrameArray[ 1 ] = m_evExit ;
  m_WaitEventBusChangeArr[ 0 ] = m_evCameraControl = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
  m_WaitEventBusChangeArr[ 2 ] = m_evBusChange = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
  m_WaitEventBusChangeArr[ 3 ] = m_evSWTriggerPulse ;
  m_evControlRequestFinished = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
  m_hCamAccessMutex = CreateMutex( NULL , FALSE , NULL ) ;
  m_pCameraObserver = NULL ;
  m_disableEmbeddedTimeStamp = false ;
  //m_isSelectingNewCamera = false ;
  m_dLastStartTime = 0. ;
  m_pNewFrame = NULL ;
  m_pNewImage = NULL ;
  m_hCameraControlThreadHandle = NULL ;
  m_dwCameraControlThreadId = 0 ;
  m_bContinueCameraControl = false ;
  m_BusEvents = 0 ;
  m_bCamerasEnumerated = false ;
  m_CameraStatus = NotInitialized ;
  m_bInitialized = false ;
  m_bStopInitialized = false ;
  m_bRescanCameras = true ;
  m_iNNoSerNumberErrors = 0 ;
  memset( &m_IntfType , 0 , sizeof( m_IntfType ) ) ;
  m_iWBRed = m_iWBlue = 512 ;
  m_bLocalStopped = FALSE ;
  m_TriggerMode = TrigNotSupported ;
  m_dLastInCallbackTime = 0. ;
  m_dwNArrivedEvents = 0 ;
  m_iFPSx10 = 0 ;
  m_iSelectedLineNumber = 0 ;
  m_GadgetInfo = _T( "AVT_Vimba" ) ;
  m_dLastBuiltPropertyTime = 0. ;
  m_pOrigProperties = CommonProperties ;
  m_bViewErrorMessagesOnGetSet = true ;
  m_dExtShutter = 0. ;
  m_dLogPeriod_ms = 0. ;
  m_dLastLogTime_ms = 0. ;
  m_iLogCnt = 0 ;
  m_iNTemperatures = 0 ;
  m_bSoftwareTriggerMode = false ;
  m_bVimbaSoftTrigger = false ;
  memset( m_dTemperatures , 0 , sizeof( m_dTemperatures ) ) ;

  memset( &m_BMIH , 0 , sizeof( BITMAPINFOHEADER ) );
  memset( &m_RealBMIH , 0 , sizeof( BITMAPINFOHEADER ) );
#ifdef _DEBUG
  m_iMutexTakeCntr = 0 ;
#endif
  //DriverInit() ;
  double dBusyTime = GetHRTickCount() - dStart ;
  TRACE( "\nAVT_Vimba::AVT_Vimba: Start %g , Busy %g" , dStart , dBusyTime ) ;
  //   }
  //   else
  //     SEND_DEVICE_ERR("Can't start Vimba System") ;
}

AVT_Vimba::~AVT_Vimba()
{}

bool AVT_Vimba::DriverInit()
{
  if ( !m_bContinueCameraControl )
  {
    FXString ThreadName ;
    m_hCameraControlThreadHandle = CreateThread( NULL , 0 ,
      CameraControlLoop , this , CREATE_SUSPENDED , &m_dwCameraControlThreadId ) ;
    if ( m_hCameraControlThreadHandle )
    {
      //       FXString ThreadName ;
      //       GetGadgetName( ThreadName ) ;
      //       ThreadName += "_CamCNTRL" ;
      //       ::SetThreadName( (LPCSTR)ThreadName , m_dwCameraControlThreadId ) ;
      m_bContinueCameraControl = true ;
      ResumeThread( m_hCameraControlThreadHandle ) ;
      Sleep( 50 ) ;
    }
    else
    {
      C1394_SENDERR_2( "%s: %s" , (LPCTSTR) ThreadName , _T( "Can't start thread" ) );
      m_bInScanProperties = false ;
      return false ;
    }
  }
  return OtherThreadDriverInit();
}

bool AVT_Vimba::AVTDriverInit()
{
  return EnumCameras();
}

void AVT_Vimba::SaveCameraInfo( const AVT::VmbAPI::CameraPtr &camera , int iIndex )
{
  std::string strID;
  std::string strName;
  std::string strModelName;
  std::string strSerialNumber;
  std::string strInterfaceID;

  FXString ErrorMsg;

  VmbErrorType err = camera->GetID( strID );
  if ( VmbErrorSuccess != err )
  {
    ErrorMsg.Format( "Could not get camera ID. Error code: %s " ,
      AVT::VmbAPI::Examples::ErrorCodeToMessage( err ) ) ;
    strID = (LPCTSTR) ErrorMsg ;
    SEND_DEVICE_ERR( (LPCTSTR) ErrorMsg ) ;
  }
  else
  {  // save to camera info
    m_CamInfo[ iIndex ].Id = strID.c_str() ;
  }

  err = camera->GetName( strName );
  if ( VmbErrorSuccess != err )
  {
    ErrorMsg.Format( "Could not get camera name. Error code: %s " ,
      AVT::VmbAPI::Examples::ErrorCodeToMessage( err ) ) ;
    strName = (LPCTSTR) ErrorMsg ;
    SEND_DEVICE_ERR( (LPCTSTR) ErrorMsg ) ;
  }
  else
  {  // save to camera info
    m_CamInfo[ iIndex ].name = strName.c_str() ;
  }

  err = camera->GetModel( strModelName );
  if ( VmbErrorSuccess != err )
  {
    ErrorMsg.Format( "Could not get camera model. Error code: %s " ,
      AVT::VmbAPI::Examples::ErrorCodeToMessage( err ) ) ;
    strModelName = (LPCTSTR) ErrorMsg ;
    SEND_DEVICE_ERR( (LPCTSTR) ErrorMsg ) ;
  }
  else
  {  // save to camera info
    m_CamInfo[ iIndex ].model = strModelName.c_str() ;
  }

  err = camera->GetSerialNumber( strSerialNumber );
  if ( VmbErrorSuccess != err )
  {
    ErrorMsg.Format( "Could not get camera serial number. Error code: %s " ,
      AVT::VmbAPI::Examples::ErrorCodeToMessage( err ) ) ;
    strSerialNumber = (LPCTSTR) ErrorMsg ;
    SEND_DEVICE_ERR( (LPCTSTR) ErrorMsg ) ;
    m_CamInfo[ iIndex ].serialnmb = 0 ;
  }
  else
  {  // save to camera info
    FXString szSerNum = m_CamInfo[ iIndex ].szSerNum = strSerialNumber.c_str() ;
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

  err = camera->GetInterfaceID( strInterfaceID );
  if ( VmbErrorSuccess != err )
  {
    ErrorMsg.Format( "Could not get camera interface ID. Error code: %s " ,
      AVT::VmbAPI::Examples::ErrorCodeToMessage( err ) ) ;
    strInterfaceID = (LPCTSTR) ErrorMsg ;
    SEND_DEVICE_ERR( (LPCTSTR) ErrorMsg ) ;
  }
  else
  {  // save to camera info
    m_CamInfo[ iIndex ].InterfaceId = strInterfaceID.c_str() ;
  }
}
//bool bSaveFullInfo = false ;
bool AVT_Vimba::EnumCameras()
{
  double dStart = GetHRTickCount() ;

  FXAutolock al( m_ConfigLock ) ;
  if ( m_bCamerasEnumerated && !m_bRescanCameras )
    return true ;
  m_LastError = m_System.GetCameras( m_Cameras ) ;
  if ( VmbErrorSuccess != m_LastError )
  {
    string_type ErrText = ErrorCodeToMessage( m_LastError ) ;
    SEND_DEVICE_ERR( "Can't get cameras: Err=%s" , ErrText.c_str() ) ;
    m_CameraStatus = CantGetCameras ;
    return false ;
  }

  m_iCamNum = m_CamerasOnBus = (int) m_Cameras.size();
  FILE * pPropertyFile = NULL ;
  //      std::for_each( m_Cameras.begin(), m_Cameras.end(), PrintCameraInfo );
  for ( unsigned i = 0; i < m_CamerasOnBus; i++ )
  {
    double dCameraStart = GetHRTickCount() ;

    const AVT::VmbAPI::CameraPtr Cam = m_Cameras[ i ] ;
    SaveCameraInfo( Cam , i ) ;
    double dAfterSave = GetHRTickCount() ;
    TRACE( "\n   Vimba EnumCameras: Save %d info time is %g" , i , dAfterSave - dCameraStart ) ;
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
    Cam->Open( VmbAccessModeRead ) ;
    if ( VmbErrorSuccess != m_LastError )
    {
      string_type ErrText = ErrorCodeToMessage( m_LastError ) ;
      SEND_DEVICE_ERR( "Can't Open Camera for Read: Err=%s" , ErrText.c_str() ) ;
      m_CameraStatus = CantOpenCameraForRead ;
      return false ;
    }
    m_LastError = Cam->GetFeatures( m_CamInfo[ i ].m_Features ) ;
    if ( VmbErrorSuccess != m_LastError )
    {
      string_type ErrText = ErrorCodeToMessage( m_LastError ) ;
      SEND_DEVICE_ERR( "Can't get Features: Err=%s" , ErrText.c_str() ) ;
      m_CameraStatus = CantGetFeatures ;
      return false ;
    }
    else
    {
      errno_t err = 0 ;
      if ( pPropertyFile == NULL )
      {
        time_t CurTime ;
        time( &CurTime ) ;

        FXString FileName ;
        FileName.Format( _T( "AllCamProps%d.txt" ) , (int) CurTime ) ;
        err = fopen_s( &pPropertyFile , (LPCTSTR) FileName , "wb" ) ;
        ASSERT( err == 0 ) ;
      }
      FXString Out ;
      double dAfterFileOpening = GetHRTickCount() ;
      TRACE( "\n   Vimba EnumCameras: Open camera %s(%d) time is %g" ,
        (LPCTSTR) m_CamInfo[ i ].model , i , dAfterFileOpening - dCameraStart ) ;
      Out.Format( "Camera SN%s-%s Interface=%s time=%g\n Properties: \n" ,
        (LPCTSTR) m_CamInfo[ i ].szSerNum , (LPCTSTR) m_CamInfo[ i ].model ,
        (LPCTSTR) m_CamInfo[ i ].InterfaceId , dAfterFileOpening - dCameraStart ) ;
      dCameraStart = dAfterFileOpening ;
      if ( pPropertyFile )
        fwrite( (LPCTSTR) Out , Out.GetLength() , 1 , pPropertyFile ) ;
      double dAfterFileWrite = GetHRTickCount() ;
      TRACE( "\n   Vimba EnumCameras: Write cam %s(%d) info time is %g" ,
        (LPCTSTR) m_CamInfo[ i ].model , i , dAfterFileWrite - dCameraStart ) ;

      for ( UINT iIndex = 0 ; iIndex < m_CamInfo[ i ].m_Features.size() ; iIndex++ )
      {
        VmbFeatureDataType FType ;
        AVT::VmbAPI::FeaturePtr Prop = m_CamInfo[ i ].m_Features[ iIndex ] ;
        m_LastError = Prop->GetDataType( FType ) ;
        if ( m_LastError == VmbErrorSuccess )
        {
          FXString FDescription ;
          std::string Name , Unit , Category , Descr ;
          m_LastError = Prop->GetName( Name ) ;
          m_LastError = Prop->GetDescription( Descr ) ;
          m_LastError = Prop->GetUnit( Unit ) ;
          m_LastError = Prop->GetCategory( Category ) ;


          CameraAttribute NewAttribute( FGP_LAST , NULL , Name.c_str() ) ;
          switch ( FType )
          {
            case VmbFeatureDataInt:
              Prop->GetRange( NewAttribute.m_i64Range[ 0 ] , NewAttribute.m_i64Range[ 1 ] ) ;
              Prop->GetValue( NewAttribute.m_int64Val ) ;
              FDescription.Format( "%s:%s=%d %s Integer [%d,%d], %s" ,
                Category.c_str() , Name.c_str() ,
                (int) NewAttribute.m_int64Val , Unit.c_str() ,
                (int) NewAttribute.m_i64Range[ 0 ] , (int) NewAttribute.m_i64Range[ 1 ] , Descr.c_str() ) ;
              break;
            case VmbFeatureDataFloat:
              Prop->GetRange( NewAttribute.m_dRange[ 0 ] , NewAttribute.m_dRange[ 1 ] ) ;
              Prop->GetValue( NewAttribute.m_dVal ) ;
              FDescription.Format( "%s:%s=%g %s double [%g,%g], %s" ,
                Category.c_str() , Name.c_str() ,
                NewAttribute.m_dVal , Unit.c_str() ,
                NewAttribute.m_dRange[ 0 ] , NewAttribute.m_dRange[ 1 ] , Descr.c_str() ) ;
              break;
            case VmbFeatureDataEnum:
            {
              std::string Val ;
              Prop->GetValue( Val ) ;
              NewAttribute.m_enumVal = Val.c_str() ;
              FDescription.Format( "%s:%s=%s Enumerator (" ,
                Category.c_str() , Name.c_str() , Val.c_str() ) ;
              AVT::VmbAPI::EnumEntryVector Enumerators ;
              Prop->GetEntries( Enumerators ) ;
              NewAttribute.m_EnumRange.RemoveAll() ;
              for ( UINT j = 0 ; j < Enumerators.size() ; j++ )
              {
                std::string Enum ;
                Enumerators.at( j ).GetName( Enum ) ;
                FXString Addition ;
                Addition.Format( "%s%c" , Enum.c_str() ,
                  (j == Enumerators.size() - 1) ? ')' : ',' ) ;
                FDescription += Addition ;
              }
              //              FDescription += '\n' ;
            }
            break;
            case VmbFeatureDataString:
            {
              std::string Val ;
              Prop->GetValue( Val ) ;
              FDescription.Format( "%s:%s=%s String" ,
                Category.c_str() , Name.c_str() , Val.c_str() ) ;
            }
            break;
            case VmbFeatureDataBool:
              Prop->GetValue( NewAttribute.m_boolVal ) ;
              FDescription.Format( "%s:%s=%s bool" ,
                Category.c_str() , Name.c_str() ,
                NewAttribute.m_boolVal ? "true" : "false" , Descr.c_str() ) ;
              break;
            case VmbFeatureDataUnknown:
              FDescription.Format( "%s:%s Unknown" ,
                Category.c_str() , Name.c_str() , Descr.c_str() ) ;
              break;
            case VmbFeatureDataCommand:
              FDescription.Format( "%s:%s Command" ,
                Category.c_str() , Name.c_str() , Descr.c_str() ) ;
              break;
            case VmbFeatureDataRaw:
              FDescription.Format( "%s:%s RAW" ,
                Category.c_str() , Name.c_str() , Descr.c_str() ) ;
              break;
            case VmbFeatureDataNone:
              FDescription.Format( "%s:%s No Data" ,
                Category.c_str() , Name.c_str() , Descr.c_str() ) ;
              break;
            default:
              ASSERT( 0 ) ;
              break ;
          }
          double dAfterPropertyInfoTaking = GetHRTickCount() ;
          FXString Time ;
          Time.Format( " T=%g\n" , dAfterPropertyInfoTaking - dAfterFileWrite ) ;
          FDescription += Time ;
          dAfterFileWrite = dAfterPropertyInfoTaking ;
          if ( !FDescription.IsEmpty() && pPropertyFile )
            fwrite( (LPCTSTR) FDescription , FDescription.GetLength() , 1 , pPropertyFile ) ;
        }
      }
    }
    Cam->Close() ;
    TRACE( "\n   Vimba EnumCameras: Inspect properties of cam %d time is %g" ,
      i , GetHRTickCount() - dCameraStart ) ;

  }
  if ( pPropertyFile )
    fclose( pPropertyFile ) ;
  m_bSaveFullInfo = false ;
  m_bCamerasEnumerated = (m_CamerasOnBus > 0) ;
  m_bRescanCameras = false ;
  m_CameraStatus = DriverInitialized ;
  if ( m_dwSerialNumber != -1 && m_dwSerialNumber != 0 )
  {
    m_GadgetInfo.Format( "AVT_Vimba_%d" , m_dwSerialNumber ) ;
  }
  return true;
}

void AVT_Vimba::ShutDown()
{
  if ( m_hCameraControlThreadHandle )
  {
    OtherThreadCameraShutDown() ;
    m_bContinueCameraControl = false ;
    SetEvent( m_evCameraControl ) ;
    SetEvent( m_evFrameReady ) ;
    Sleep( 50 ) ;
    DWORD dwRes = WaitForSingleObject( m_hCameraControlThreadHandle , 5000 ) ;
    ASSERT( dwRes == WAIT_OBJECT_0 ) ;
  }

  DWORD dwRes = WaitForSingleObject( m_hCamAccessMutex , 1000 ) ;
  ASSERT( dwRes == WAIT_OBJECT_0 ) ;
  CloseHandle( m_hCamAccessMutex ) ;
  m_GrabLock.Lock() ;
  while ( m_ReadyFrames.size() )
  {
    CDataFrame * pFrame = m_ReadyFrames.front() ;
    m_ReadyFrames.pop() ;
    pFrame->Release() ;
  };
  m_GrabLock.Unlock() ;
  if ( m_pLogOutput )
  {
    delete m_pLogOutput ;
    m_pLogOutput = NULL ;
  }
  FxReleaseHandle( m_evFrameReady ) ;
  FxReleaseHandle( m_evBusChange ) ;
  FxReleaseHandle( m_evCameraControl ) ;
  C1394Camera::ShutDown() ;
}

void AVT_Vimba::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
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

    FXAutolock al( m_SettingsLock , "AsyncTransaction" ) ;
    if ( cmd.CompareNoCase( "list" ) == 0 )
    {
      pk.Empty();
      for ( int i = 0; i < (int) m_PropertiesEx.GetCount() ; i++ )
      {
        pk += m_PropertiesEx[ i ].m_Name ;
        pk += "\r\n" ;
      }
    }
    else if ( (cmd.CompareNoCase( "get" ) == 0) && (pk.GetWord( pos , cmd )) )
    {
      int iIndex = GetPropertyIndex( cmd );
      if ( iIndex != WRONG_PROPERTY )
      {
        FXSIZE value;
        bool bauto;
        bool bRes = GetCameraProperty( iIndex , value , bauto ) ;
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
    else if ( (cmd.CompareNoCase( "set" ) == 0) && (pk.GetWord( pos , cmd )) && (pk.GetParamString( pos , param )) )
    {
      int iIndex = GetPropertyIndex( cmd );
      if ( iIndex != WRONG_PROPERTY )
      {
        FXSIZE value = 0;
        bool bauto = false , Invalidate = false;
        if ( IsDigitField( m_PropertiesEx[ iIndex ].pr ) )
        {
          if ( param.CompareNoCase( "auto" ) == 0 )
            bauto = true;
          else
            value = atoi( param );
        }
        else
          value = (FXSIZE) (LPCTSTR) param;
        bool bWasStopped = m_bWasStopped ;
        m_bWasStopped = false ;
        bool bRes = SetCameraProperty( iIndex , value , bauto , Invalidate ) ;
        if ( !bWasStopped  && m_bWasStopped )
        {
          /*CamCNTLDoAndWait( AVT_EVT_INIT | AVT_EVT_START_GRAB )*/ ;
        }
        m_bWasStopped = bWasStopped ;
        pk = (bRes) ? "OK" : "error" ;
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

bool AVT_Vimba::CameraInit()
{
  return OtherThreadCameraInit() ;
}

FXString AVT_Vimba::GetCameraId( DWORD dwSerialNumber , DWORD& dwIndex )
{
  FXString Id ;
  for ( DWORD i = 0 ; i < m_CamerasOnBus ; i++ )
  {
    if ( m_CamInfo[ i ].serialnmb == dwSerialNumber )
    {
      DWORD dwBusyCamIndex = 0 ;

      FXAutolock al( m_ConfigLock ) ;
      for ( ; dwBusyCamIndex < (DWORD) m_BusyCameras.GetCount() ; dwBusyCamIndex++ )
      {
        if ( m_BusyCameras[ dwBusyCamIndex ].m_dwSerialNumber == m_dwSerialNumber )
        {
          SEND_DEVICE_ERR( "\nCamera %u is busy." , m_dwSerialNumber );
          return Id ;
        }
      }
      if ( dwBusyCamIndex >= (DWORD) m_BusyCameras.GetCount() )
      {                 // camera found and is not busy
        Id = m_CamInfo[ i ].Id ;
        dwIndex = i ;
        return Id ;
      }
    }
  }
  SEND_DEVICE_ERR( "\nCamera #%u is not connected" , dwSerialNumber ) ;
  dwIndex = 0xffffffff ;
  return Id ;
}

bool AVT_Vimba::AVTCameraInit()
{
  AVTDriverInit();
  double dStart = GetHRTickCount() ;

  if ( m_dwConnectedSerialNumber && (m_dwConnectedSerialNumber == m_dwSerialNumber) )
  {
    if ( !SP_ISNULL( m_pCamera ) ) // already connected to the same camera
    {
      if ( m_bShouldBeReprogrammed )
      {
        AVTCameraStop() ;
      }
      return true ;
  }
  }

  if ( !SP_ISNULL( m_pCamera ) )
    AVTCameraClose();

  if ( !m_CamerasOnBus )
  {  // nothing to connect
    SEND_DEVICE_ERR( "Fatal error: No AVT_Vimba cameras found on a bus." );
    return false;
  }
  if ( (m_dwSerialNumber == 0) || (m_dwSerialNumber == 0xffffffff) )
  {  // nothing to connect
    SEND_DEVICE_ERR( "Fatal error: No selected camera." );
    return false;
  }

  DWORD dwIndex ;
  {
    //FXAutolock al( m_ConfigLock ) ;
    FXString Id = GetCameraId( m_dwSerialNumber , dwIndex ) ;
    if ( Id.IsEmpty() )
      return false ;
    m_LastError = m_System.GetCameraByID( (LPCTSTR) Id , m_pCamera );
    if ( m_LastError != VmbErrorSuccess )
    {
      if ( ++m_iNNoSerNumberErrors <= 2 )
        SEND_DEVICE_ERR( "Fatal error GetCameraByID for %s: %s" , (LPCTSTR) Id ,
        ErrorCodeToMessage( m_LastError ).c_str() );
      return false;
    }
    m_iNNoSerNumberErrors = 0 ;

    m_Status.Empty() ;

    int iLastChannel = -1 ;
    m_CameraID.Format( "%d_%s" , m_dwSerialNumber , m_CamInfo[ dwIndex ].model );
    m_GadgetInfo = m_CameraID ;
    //FXString GadgetName ;
    BOOL bInLoop = GetGadgetName( m_GadgetInfo ) ;
    //   m_GadgetInfo.Format("%s: %s", GadgetName , m_CameraID );

    //   RegisterCallbacks() ;
    if ( m_Status.IsEmpty() )
    {
      if ( bInLoop )
        m_Status = m_CameraID + " is connected" ;
    }
    if ( !m_Status.IsEmpty() )
    {
      SEND_DEVICE_INFO( m_Status );
      m_Status.Empty() ;
    }
    m_dwConnectedSerialNumber = m_dwSerialNumber ;
    FXAutolock al( m_ConfigLock ) ;
    int i = 0 ;
    for ( ; i < (int) m_BusyCameras.GetCount() ; i++ )
    {
      if ( m_BusyCameras[ i ].m_dwSerialNumber == m_dwSerialNumber )
        break ;
    }
    if ( i == (int) m_BusyCameras.GetCount() )
    {
      BusyCamera NewBusyCamera( m_dwSerialNumber , this ) ;
      m_BusyCameras.Add( NewBusyCamera ) ;
    }
    m_szSerialNumber = (LPCTSTR) m_CamInfo[ dwIndex ].szSerNum ;
  }
  m_LastError = m_pCamera->Open( VmbAccessModeFull ) ;
  if ( m_LastError != VmbErrorSuccess )
  {
    SEND_DEVICE_ERR( "Camera Open Error for: %s" ,
      ErrorCodeToMessage( m_LastError ).c_str() );
  }
  VmbAccessModeType Mode ;
  m_pCamera->GetPermittedAccess( Mode ) ;

  double dConnectionTime = GetHRTickCount() ;
  TRACE( "\n    AVTBuildPropertyList: Camera connection time is %g" , dConnectionTime - dStart ) ;
  if ( m_hCameraControlThreadHandle )
  {
    FXString ThreadName ;
    GetGadgetName( ThreadName ) ;
    ThreadName += "_CNTRL" ;
    ::SetThreadName( (LPCSTR) ThreadName , m_dwCameraControlThreadId ) ;
  }
  m_pOrigProperties = CommonProperties ;
  m_iNProperties = ARR_SIZE( CommonProperties ) ;

  for ( int i = 0 ; i < ARR_SIZE( KnownCameras ) ; i++ )
  {
    CamTypeAndProperties * pCamTP = KnownCameras[ i ] ;
    if ( !pCamTP )
      break ;
    if ( m_CamInfo[ dwIndex ].model.Find( pCamTP->CamType ) >= 0 )
    {
      if ( m_CamInfo[ dwIndex ].model.Find( "Guppy PRO" ) >= 0 )
      {
        if ( strstr( pCamTP->CamType , "Guppy PRO" ) )
        {
          m_pOrigProperties = pCamTP->Properties ;
          m_iNProperties = pCamTP->iNProperties ;
          break ;
        }
      }
      else  if ( m_CamInfo[ dwIndex ].model.Find( "Guppy" ) >= 0 )
      {
        if ( strstr( pCamTP->CamType , "Guppy" ) )
        {
          m_pOrigProperties = pCamTP->Properties ;
          m_iNProperties = pCamTP->iNProperties ;
          break ;
        }
      }
      else
      {
        m_pOrigProperties = pCamTP->Properties ;
        m_iNProperties = pCamTP->iNProperties ;
        break ;
      }
    }
  } ;

  if ( !AVTBuildPropertyList() )
  {
    AVTCameraClose() ;
  }
  else
    m_bInitialized = true ;


  TRACE( "\nAVT_Vimba::AVTCameraInit - Camera %u initialized in %g ms" ,
    m_CameraInfo.serialnmb , GetHRTickCount() - dStart ) ;
  return true;
}

void AVT_Vimba::CameraClose()
{
  OtherThreadCameraClose() ;
}

void AVT_Vimba::AVTCameraClose()
{
  if ( SP_ISNULL( m_pCamera ) || m_dwConnectedSerialNumber == 0 || m_dwSerialNumber == -1 )
  {
    m_CameraStatus = CameraAlreadyClosed ;
  #ifdef AVT_VIMBA_DEBUG
    TRACE( "\nCamera already closed SN=%u " , m_dwSerialNumber ) ;
  #endif
    return ;
  }

  //   FXAutolock al( m_ConfigLock) ;
  m_pCamera->Close() ;
  if ( m_dwConnectedSerialNumber != m_dwSerialNumber )
    SEND_DEVICE_ERR( "CameraClose: Connected %d is not equal to Serial %" ,
    m_dwConnectedSerialNumber , m_dwSerialNumber ) ;
  int i = 0 ;
  FXAutolock al( m_ConfigLock ) ;

  for ( ; i < m_BusyCameras.GetCount() ; i++ )
  {
    if ( m_BusyCameras[ i ].m_dwSerialNumber == m_dwConnectedSerialNumber )
    {
      m_BusyCameras.RemoveAt( i ) ;
      break ;
    }
  }
  //   m_dwSerialNumber = -1 ;
  m_dwConnectedSerialNumber = 0 ;
  m_CameraStatus = CameraClosed ;
  m_bInitialized = false ;

  SP_RESET( m_pCamera ) ;
#ifdef AVT_VIMBA_DEBUG
  TRACE( "\nCamera closed SN=%u Index=%d" , m_dwSerialNumber , i ) ;
#endif
}

bool AVT_Vimba::DriverValid()
{

  return  (m_CamerasOnBus != 0) /*&& (m_pCamera && m_pCamera->IsConnected())*/;
}

bool AVT_Vimba::CameraStart()
{
  if ( m_dwSerialNumber && (m_dwSerialNumber != 0xffffffff) && !m_bInScanProperties )
    return OtherThreadCameraStart() ;
  return false ;
}
bool AVT_Vimba::AVTCameraStart()
{
  if ( SP_ISNULL( m_pCamera ) )
  {
    if ( !AVTCameraInit() )
    {
    #ifdef AVT_VIMBA_DEBUG
      TRACE( "\nAVTCameraStart: Camera is not initialized SN=%u" , m_dwSerialNumber ) ;
    #endif
      return false ;
    }
  }

  m_nWidth = GetXSize( m_pCamera ) ;
  m_nHeight = GetYSize( m_pCamera ) ;


  ULONG ulFrameSize = (ULONG) m_nWidth * (ULONG) m_nHeight ;
  if ( ulFrameSize == 0 )
  {
    AVTCameraClose() ;
    SP_RESET( m_pCamera ) ;
    if ( !AVTCameraInit() )
    {
      SENDERR( "Area is zero (%d,%d)" , (ULONG) m_nWidth , (ULONG) m_nHeight ) ;
    #ifdef AVT_VIMBA_DEBUG
      TRACE( "\nAVTCameraStart: Area for SN%u is zero (%d,%d)" ,
        m_dwSerialNumber , (ULONG) m_nWidth , (ULONG) m_nHeight ) ;
    #endif
      m_CameraStatus = CantInit ;
      return false ;
    }
  };
  TRACE( "%f Camera Start Frame Size %u" , GetHRTickCount() , ulFrameSize ) ;
  C1394Camera::CameraStart() ;
  //   m_LastError = m_pCamera->StartCapture() ;
  //   if ( m_LastError != VmbErrorSuccess )
  //   {
  //     SENDERR("Can't start stream %s", ErrorCodeToMessage( m_LastError ).c_str() );
  //     return false ;
  //   }
  VmbInterfaceType CamInterface ;
  m_pCamera->GetInterfaceType( CamInterface ) ;
  switch ( CamInterface )
  {
    case VmbInterfaceEthernet:
    {
      AVT::VmbAPI::FeaturePtr pCommandFeature;
      if ( VmbErrorSuccess == m_pCamera->GetFeatureByName(
        "GVSPAdjustPacketSize" , pCommandFeature ) )
      {
        if ( VmbErrorSuccess == pCommandFeature->RunCommand() )
        {
          bool bIsCommandDone = false;
          do
          {
            if ( VmbErrorSuccess != pCommandFeature->IsCommandDone( bIsCommandDone ) )
            {
              break;
            }
          } while ( false == bIsCommandDone );
        }
      }
    }
    break ;
    case VmbInterfaceFirewire:

      break ;
    case VmbInterfaceUsb:

      break ;
  }

  //   m_LastError = SetFeatureIntValue( m_pCamera , "PixelFormat", m_pixelFormat );
  //   if( VmbErrorSuccess != m_LastError )
  //   {
  //     //     // Fall back to Mono
  //     //     res = SetFeatureIntValue( m_pCamera,"PixelFormat", VmbPixelFormatMono8 );
  //     return false ;
  //   }
  m_LastError = GetFeatureIntValue( m_pCamera , "PixelFormat" , m_nPixelFormat );

  if ( VmbErrorSuccess == m_LastError )
  {
    SetBMIH() ;
    // Create a frame observer for this camera (This will be wrapped in a shared_ptr so we don't delete it)
    SP_RESET( m_pFrameObserver ) ;
    if ( SP_ISNULL( m_pFrameObserver ) )
      SP_SET( m_pFrameObserver , new AVT::VmbAPI::FrameObserver( m_pCamera , FrameDoneCB , this ) );
    // Start streaming
    //FXAutolock al( m_ConfigLock ) ;
    SetCamPropertyData FPSData ;
    FPSData.m_double = (double) m_iFPSx10 / 10. ;
    SetPropertyValue( m_pCamera , m_FPSPropertyName , FPSData ) ;
    GetPropertyValue( m_pCamera , m_FPSPropertyName , FPSData ) ;
    m_LastError = SP_ACCESS( m_pCamera )->StartContinuousImageAcquisition( 10 , m_pFrameObserver );
    if ( m_LastError != VmbErrorSuccess )
    {
      SENDERR( "Can't start capture: %s" , ErrorCodeToMessage( m_LastError ).c_str() ) ;
    #ifdef AVT_VIMBA_DEBUG
      TRACE( "\nAVTCameraStart: Camera is not started SN=%u %s" ,
        m_dwSerialNumber , ErrorCodeToMessage( m_LastError ).c_str() ) ;
    #endif
    }
    else
      return true ;

  }
  else
  {
    SENDERR( "Can't get pixel format: %s" , ErrorCodeToMessage( m_LastError ).c_str() ) ;
  #ifdef AVT_VIMBA_DEBUG
    TRACE( "\nAVTCameraStart: Can't get pixel format SN=%u %s" ,
      m_dwSerialNumber , ErrorCodeToMessage( m_LastError ).c_str() ) ;
  #endif
  }

  AVTCameraClose() ;
  SP_RESET( m_pCamera ) ;
  m_CameraStatus = ClosedAfterFaultOnStart ;
  return false ;
}
void AVT_Vimba::CameraStop()
{
  OtherThreadCameraStop() ;
}

void AVT_Vimba::AVTCameraStop()
{
  if ( !IsRunning() )
    return ;
  m_bRun = FALSE ;
  if ( !m_pCamera )
    return;
  m_LastError = SP_ACCESS( m_pCamera )->StopContinuousImageAcquisition();
  if ( m_LastError != VmbErrorSuccess )
  {
    SENDERR( "Can't stop acquisition: %s" , ErrorCodeToMessage( m_LastError ).c_str() ) ;
  #ifdef AVT_VIMBA_DEBUG
    TRACE( "\nAVTCameraStart: Can't stop acquisition SN=%u %s" ,
      m_dwSerialNumber , ErrorCodeToMessage( m_LastError ).c_str() ) ;
  #endif
  }
  SP_RESET( m_pFrameObserver ) ;
  C1394Camera::CameraStop();
  m_CameraStatus = CameraStopped ;
  SetEvent( m_evFrameReady ) ;
  TRACE( "%f AVT_Vimba::CameraStop() for #%u\n " , GetHRTickCount() , m_dwSerialNumber );
}

CVideoFrame* AVT_Vimba::CameraDoGrab( double* StartTime )
{
  CVideoFrame * pOut = NULL ;
  // waiting for frame or exit
  if ( !m_ReadyFrames.size() )
  {
    DWORD dwRes = WaitForMultipleObjects( 2 , m_WaitEventFrameArray , FALSE , /*INFINITE*/ 500 ) ;
    if ( dwRes != WAIT_OBJECT_0 )
      return NULL ;
  }
  m_dLastStartTime = ::GetHRTickCount() ;
  *StartTime = m_dLastStartTime - m_dLastInCallbackTime ;
  if ( m_ReadyFrames.size() )
  {
    m_GrabLock.Lock() ;
    pOut = m_ReadyFrames.front() ;
    m_ReadyFrames.pop() ;
    m_GrabLock.Unlock() ;
  }
  *StartTime -= m_dLastInCallbackTime ;
  return pOut ;
}

bool AVT_Vimba::BuildPropertyList()
{
  FXAutolock al( m_SettingsLock , "BuildPropertyList" ) ;
  return OtherThreadBuildPropertyList() ;
}

bool AVT_Vimba::AVTBuildPropertyList()
{
  TCHAR Msg[ 300 ] ;
  CAMERA1394::Property P;
  if ( !m_pCamera )
    return false ;

  if ( GetHRTickCount() - m_dLastBuiltPropertyTime < 1000. )
    return true ;

  double dStart = GetHRTickCount() ;
  m_Properties.RemoveAll();
  m_PropertiesEx.RemoveAll();
  int iWhiteBalanceMode = GetColorMode() ; // 0 - no color, 1 - by program, 2 - by camera

  VmbAccessModeType Access ;
  VmbErrorType LastError = m_pCamera->GetPermittedAccess( Access ) ;
  if ( LastError == VmbErrorSuccess )
  {
    //     if ( Access != VmbAccessModeFull )
    //     {
    //       m_pCamera->Close() ;
    //       LastError = m_pCamera->Open( VmbAccessModeFull ) ;
    //       if ( LastError != VmbErrorSuccess )
    //       {
    //         _stprintf( Msg ,
    //           "\n    AVTBuildPropertyList: can't open camera #%d for control (Err=%d)" ,
    //           m_dwSerialNumber , (int) LastError ) ;
    //         TRACE( Msg ) ;
    //         SENDERR( Msg ) ;
    //         return false ;
    //       }
    //     }
  }
  else
  {
    _stprintf( Msg ,
      "\n    AVTBuildPropertyList: can't get access for camera #%d (Err=%d)" ,
      m_dwSerialNumber , (int) LastError ) ;
    TRACE( Msg ) ;
    SENDERR( Msg ) ;
    return false ;
  }

  //   SetCamPropertyData Tmp ;
  //   if ( GetPropertyValue( m_pCamera , "LineSelector" , Tmp ) )
  //   {
  //     TRACE("  On Entry BuildPropList: LineSelector=%s set to" , Tmp.m_szString ) ;
  //  
  //     if ( !m_SelectedLine.IsEmpty() )
  //     {
  //       strcpy_s( Tmp.m_szString , (LPCTSTR)m_SelectedLine ) ;
  //       SetPropertyValue( m_pCamera , "LineSelector" , Tmp ) ;
  //       TRACE(" %s\n" , Tmp.m_szString ) ;
  //     }
  //   }

  double dOpening = GetHRTickCount() - dStart ;
  TRACE( "\n    AVTBuildPropertyList: Camera Opening time is %g" , dOpening ) ;
  m_nSensorWidth = m_SensorSize.cx = GetMaxWidth( m_pCamera ) ;
  m_nSensorHeight = m_SensorSize.cy = GetMaxHeight( m_pCamera ) ;
  bool bInputMode = false ;
  bool bOutputMode = false ;
  bool bTriggerMode = false ;
  m_TriggerMode = TrigNotSupported ;

  for ( int i = 0 ; i < m_iNProperties ; i++ )
  {
    FXString DlgFormat ;
    const CamProperty& Prop = m_pOrigProperties[ i ] ;
    CameraAttribute NewAttribute( Prop.pr , Prop.name , Prop.CamPropertyName ) ;
    //     bool bOutputLineProcessPass = !( (m_iSelectedLineNumber < 4) 
    //       && ( (Prop.pr == FGP_LINE_SOURCE) || (Prop.pr == FGP_OUT_DELAY) ) ) ;
    if ( Prop.CamPropertyName /*&& bOutputLineProcessPass*/ )
    {
      NewAttribute.m_bIsStopNecessary = Prop.m_bStopAcquisition ;
      AVT::VmbAPI::FeaturePtr pPropPtr ;
      if ( Prop.pr == FGP_USER_SET_SELECT )
      {
        DlgFormat.Format( "ComboBox(%s(NoOper(0),Save(1),Load(2)))" , Prop.name ) ;
        m_SaveSettingsCommand = Prop.CamPropertyName ;
        m_LoadSettingsCommand = Prop.AutoControl ;
        NewAttribute.m_Type = VmbFeatureDataInt ;
      }
      else if ( Prop.pr == FGP_USER_SET_DEF )
      {
        m_UserSetSelector = Prop.name ;
        m_SetDefaultSettings = Prop.CamPropertyName ;
        m_bSetDefaultSettingsIsEnum = (Prop.AutoControl != NULL) ;
      }
      else
      {
        m_LastError = m_pCamera->GetFeatureByName( Prop.CamPropertyName , pPropPtr ) ;
        if ( m_LastError == VmbErrorSuccess )
        {
          m_LastError = pPropPtr->GetDataType( NewAttribute.m_Type ) ;
          if ( m_LastError == VmbErrorSuccess )
          {
            if ( Prop.AutoControl )
            {
              AVT::VmbAPI::FeaturePtr pAutoPropPtr ;
              NewAttribute.m_AutoControl = Prop.AutoControl ;
              m_LastError = m_pCamera->GetFeatureByName(
                Prop.AutoControl , pAutoPropPtr ) ;
              if ( m_LastError == VmbErrorSuccess )
              {
                AVT::VmbAPI::EnumEntryVector Enumerators ;
                pAutoPropPtr->GetEntries( Enumerators ) ;
                NewAttribute.m_EnumRange.RemoveAll() ;
                for ( UINT j = 0 ; j < Enumerators.size() ; j++ )
                {
                  std::string Name ;
                  Enumerators.at( j ).GetName( Name ) ;
                  NewAttribute.m_EnumRange.Add( Name.c_str() ) ;
                }
                //               Prop.m_EnumerateNames.Copy( NewAttribute.m_EnumRange ) ;
              }
              //               else
              //                 SP_RESET(Prop.pAutoInCamera) ;
            }
            //           Prop.m_DataType = NewAttribute.m_Type ;
            switch ( NewAttribute.m_Type )
            {
              case VmbFeatureDataInt:
                pPropPtr->GetRange( NewAttribute.m_i64Range[ 0 ] , NewAttribute.m_i64Range[ 1 ] ) ;
                pPropPtr->GetValue( NewAttribute.m_int64Val ) ;
                DlgFormat.Format( (Prop.AutoControl ? "Spin&Bool(%s,%i,%i)" : "Spin(%s,%i,%i)") , Prop.name ,
                  (int) NewAttribute.m_i64Range[ 0 ] , (int) NewAttribute.m_i64Range[ 1 ] ) ;
                NewAttribute.m_intVal = (int) NewAttribute.m_int64Val ;
                break;
              case VmbFeatureDataFloat:
                if ( (Prop.pr == FGP_TRIGGERDELAY && !bTriggerMode)
                  || (Prop.pr == FGP_LINE_DEBOUNCE && !bInputMode) )
                  break ;

                pPropPtr->GetRange( NewAttribute.m_dRange[ 0 ] , NewAttribute.m_dRange[ 1 ] ) ;
                pPropPtr->GetValue( NewAttribute.m_dVal ) ;
                switch ( Prop.pr )
                {
                  case FGP_OUT_DELAY:
                    DlgFormat.Format( "Spin(%s,%i,%i)" , Prop.name ,
                      (int) (NewAttribute.m_dRange[ 0 ]) , (int) (NewAttribute.m_dRange[ 1 ]) ) ;
                    NewAttribute.m_intVal = ROUND( NewAttribute.m_dVal ) ;
                    break ;
                  case FGP_EXTSHUTTER:
                    DlgFormat.Format( "Spin&Bool(%s,%i,%i)" , Prop.name ,
                      (int) (NewAttribute.m_dRange[ 0 ]) , (int) (NewAttribute.m_dRange[ 1 ]) ) ;
                    NewAttribute.m_intVal = ROUND( NewAttribute.m_dVal ) ;
                    break ;
                  case FGP_GAIN:
                    DlgFormat.Format( "Spin&Bool(%s,%i,%i)" , Prop.name ,
                      (int) (NewAttribute.m_dRange[ 0 ] * 10.) , (int) (NewAttribute.m_dRange[ 1 ] * 10.) ) ;
                    NewAttribute.m_intVal = ROUND( NewAttribute.m_dVal ) ;
                    break ;
                  case FGP_WHITEBAL_RATIO:
                    DlgFormat.Format( "Spin&Bool(%s,%i,%i)" , Prop.name ,
                      (int) (NewAttribute.m_dRange[ 0 ] * 100.) , (int) (NewAttribute.m_dRange[ 1 ] * 100.) ) ;
                    NewAttribute.m_intVal = ROUND( NewAttribute.m_dVal * 100. ) ;
                    break ;
                  case FGP_FRAME_RATE:
                  {
                    if ( m_TriggerMode == TriggerOff )
                    {
                    DlgFormat.Format( "Spin(%s,%i,%i)" , Prop.name ,
                      (int) (NewAttribute.m_dRange[ 0 ] * 10.) , (int) (NewAttribute.m_dRange[ 1 ] * 10.) ) ;
                    NewAttribute.m_intVal = ROUND( NewAttribute.m_dVal * 10. ) ;
                    m_FPSPropertyName = NewAttribute.m_CameraPropertyName ;
                  }
                  }
                  break ;
                  case FGP_TEMPERATURE_S:
                  {
                    DlgFormat.Format( "EditBox(%s)" , Prop.name ) ;
                    if ( m_pOrigProperties == GoldEye )
                    {
                      //                       CamProperty& Pr = ( CamProperty ) Prop ;
                      //                       Pr.m_DataType = VmbFeatureDataString ;
                      m_PropertiesForTemp.RemoveAll() ;
                      m_PropertiesForTemp.Add( _T( "DeviceTemperature" ) ) ;
                      m_PropertiesForTemp.Add( _T( "SensorCoolingPower" ) );
                      SetCamPropertyData Data ;
                      Data.m_int64 = Data.m_int = 1 ;
                      SetPropertyValue( "SensorTemperatureSetpointActive" , Data ) ;
                      SetPropertyValue( "SensorTemperatureSetpointSelector" , Data ) ;
                      _tcscpy_s( Data.m_szString , _T( "Sensor" ) );
                      SetPropertyValue( _T( "DeviceTemperatureSelector" ) , Data ) ;
                      _tcscpy_s( Data.m_szString , _T( "TemperatureControl" ) );
                      SetPropertyValue( _T( "SensorTemperatureControlMode" ) , Data ) ;
                      RunCameraCommand( _T( "SensorTemperatureSetpointActivate" ) ) ;
                    }
                  }
                  break ;
                  default:
                    DlgFormat.Format( "EditBox(%s)" , Prop.name ) ;
                    break ;
                }
                break;
              case VmbFeatureDataEnum:
              {
                bool bExists = true ;
                switch ( Prop.pr )
                {
                  case FGP_TRIGGER_SOURCE:
                  case FGP_TRIGGERDELAY:
                  case FGP_TRIGGER_POLARITY: bExists = (m_TriggerMode == TriggerOn) ; break ;
                  case FGP_LINE_SOURCE: bExists = bOutputMode ; break ;
                  case FGP_LINE_DEBOUNCE: bExists = bInputMode ; break ;
                  case FGP_LINE_SELECT: bExists = true ; break ;
                }
                if ( !bExists )
                  break ;
                std::string SelectedName ;
                pPropPtr->GetValue( SelectedName ) ;
                NewAttribute.m_enumVal = SelectedName.c_str() ;
                AVT::VmbAPI::EnumEntryVector Enumerators ;
                pPropPtr->GetEntries( Enumerators ) ;
                NewAttribute.m_EnumRange.RemoveAll() ;
                DlgFormat.Format( "ComboBox(%s(" , Prop.name ) ;
                int iNActive = 0 ;
                for ( UINT j = 0 ; j < Enumerators.size() ; j++ )
                {
                  std::string Name ;
                  Enumerators.at( j ).GetName( Name ) ;
                    NewAttribute.m_EnumRange.Add( Name.c_str() ) ;
                    FXString Addition ;
                  Addition.Format( "%s(%d)%c" , Name.c_str() , j ,
                      (j == Enumerators.size() - 1) ? ')' : ',' ) ;
                    DlgFormat += Addition ;
                  }
                DlgFormat += ")" ;
                //                 switch ( Prop.pr )
                //                 {
                //                   case FGP_TRIGGER_SOURCE:
                //                     m_TriggerSourceEnums.Format( _T( "%s(0)," ) , m_TriggerOff ) ;
                //                     DlgFormat += m_TriggerSourceEnums ;
                //                     NewAttribute.m_EnumRange.Add( m_TriggerOff ) ;
                //                     break ;
                //                 }

                if ( Prop.pr == FGP_TRIGGER_SOURCE )
                {
                  int iLast = (int) NewAttribute.m_EnumRange.GetUpperBound() ;
                  for ( int i = 0 ; i < NewAttribute.m_EnumRange.GetCount() ; i++ )
                  {
                    //m_TriggerSourceEnums += NewAttribute.m_EnumRange[i] ;
                    FXString ItemIndex ;
                    ItemIndex.Format( "(%d)%s" , i + 1 , (i < iLast) ? "," : "" ) ;
                    m_TriggerSourceEnums += ItemIndex ;
                  }
                  m_SelectedTriggerSource = NewAttribute.m_enumVal ;
                  //DlgFormat.Empty() ; // this property will be included into FGP_TRIGGERONOFF
                  m_TriggerSourceName = Prop.CamPropertyName ;
                }
                //Prop.m_EnumerateNames.Copy( NewAttribute.m_EnumRange ) ;
                if ( Prop.pr == FGP_LINE_SELECT )
                {
                  pPropPtr->SetValue( NewAttribute.m_enumVal ) ;
                  m_SelectedLine = NewAttribute.m_enumVal ;

                  FXString LineParams ;
                  SetCamPropertyData Data ;
                  m_bViewErrorMessagesOnGetSet = false ;
                  if ( GetPropertyValue( m_pCamera , "LineMode" , Data ) )
                  {
                    LineParams = Data.m_szString ;
                    if ( LineParams == "Input" )
                      bInputMode = true ;
                    else if ( LineParams == "Output" )
                      bOutputMode = true ;
                  }
                  m_bViewErrorMessagesOnGetSet = true ;
                }
                else if ( Prop.pr == FGP_TRIGGERONOFF )
                {
                  m_TriggerOff = NewAttribute.m_EnumRange[ 0 ] ;
                  bTriggerMode = (NewAttribute.m_enumVal != m_TriggerOff) ;// != Off  or  No
                  m_TriggerMode = (bTriggerMode) ? TriggerOn : TriggerOff ;
                  m_TriggerOn = NewAttribute.m_EnumRange[ 1 ] ;
                  m_TriggerModeName = Prop.CamPropertyName ;
                }
              }
              break;
              case VmbFeatureDataString:
              {
                std::string Name ;
                pPropPtr->GetValue( Name ) ;
                NewAttribute.m_stringVal = Name.c_str() ;
                DlgFormat.Format( "EditBox(%s)" , Prop.name ) ;
              }
              break;
              case VmbFeatureDataBool:
                pPropPtr->GetValue( NewAttribute.m_boolVal ) ;
                DlgFormat.Format( "ComboBox(%s(false(0),true(1)))" , Prop.name ) ;
                break;
              default:
                ASSERT( 0 ) ;
                break ;
            }
          }
        }
      }
      //       else
      //         SP_RESET(Prop.pInCamera) ;
    }
    else
    {
      switch ( Prop.pr )
      {
        case FGP_ROI:
          DlgFormat = _T( "EditBox(ROI)" ) ;
          //Prop.m_DataType = VmbFeatureDataString ;
          break;
        case FGP_LOG:
          DlgFormat = _T( "EditBox(Log)" ) ;
          //Prop.m_DataType = VmbFeatureDataString ;
          break;

      }
    }
    if ( !DlgFormat.IsEmpty() )
    {
      //       CAMERA1394::Property NewProperty ;
      //       NewProperty.id = Prop.pr ;
      //       NewProperty.name = Prop.name ;
      //       Prop.m_DlgFormat = DlgFormat ;
      //       NewProperty.property = DlgFormat ;
      NewAttribute.m_DlgFormat = DlgFormat ;
      //       m_Properties.Add( NewProperty ) ;
      m_PropertiesEx.Add( NewAttribute ) ;
      //       Prop.m_bSupported = true ;
    }
    //     else
    //       Prop.m_bSupported = false ;
  }
  m_CameraStatus = PropertyListBuilt ;
  m_dLastBuiltPropertyTime = GetHRTickCount() ;
  TRACE( "\n    AVTBuildPropertyList: list built time is %g" , m_dLastBuiltPropertyTime - dStart ) ;
  //m_pCamera->Close() ;
  return true;
}

bool AVT_Vimba::GetCameraProperty( unsigned i , FXSIZE &value , bool& bauto )
{
  if ( !CheckAndAllocCamera() )
    return false;
  CameraAttribute& Prop = m_PropertiesEx.GetAt( i ) ;

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
      static FXString sROI;
      CRect rc;
      GetROI( rc );
      sROI.Format( "%d,%d,%d,%d" , rc.left , rc.top , rc.right , rc.bottom );
      bauto = false;
      value = (FXSIZE) (LPCTSTR) sROI;
      return true;
    }
    case FGP_LOG:
      value = (FXSIZE) (LPCTSTR) m_PropertiesForLogAsString ;
      return true ;
    case FGP_TEMPERATURE_S:
    {
      SetCamPropertyData PropData ;
      static FXString Tmp ;
      if ( OtherThreadGetProperty( i , &PropData ) )
      {
        if ( m_pOrigProperties == GoldEye )
        {
          Tmp = PropData.m_szString ;
          value = (FXSIZE) (LPCTSTR) Tmp ;
          return true ;
        }
        else
        {
          switch ( PropData.m_Type )
          {
            case VmbFeatureDataFloat:
              Tmp.Format( "%5.2f" , PropData.m_double ) ;
              break ;
            case VmbFeatureDataInt:
              Tmp.Format( "%5ld" , PropData.m_int64 ) ;
              break ;
            default:
              return false ;
          }
          value = (FXSIZE) (LPCTSTR) Tmp ;
        }
        //         static TCHAR szTemp[ 200 ] ;
        //         _stprintf_s( szTemp , "%4.1f" , m_dTemperatures[ 0 ] ) ;
        //         if ( m_dTemperatures[ 0 ] != 0.0 )
        //         {
        //           for ( int i = 1 ; i < m_iNTemperatures ; i++ )
        //           {
        //             int iLen = _tcslen( szTemp ) ;
        //             _stprintf_s( &szTemp[ iLen ] , sizeof( szTemp ) - iLen * sizeof( TCHAR ) ,
        //               " %4.1f" , m_dTemperatures[ i ] ) ;
        //           }
        //           value = ( int )( ( LPCTSTR )szTemp ) ;
        //         }
      }
      return true ;
    }

    default:
    {
      SetCamPropertyData PropData ;
      if ( /*Prop.m_bSupported && */OtherThreadGetProperty( i , &PropData ) )
      {
        if ( Prop.m_AutoControl && PropData.m_bAuto )
          bauto = PropData.m_bBool ;
        else
          bauto = false ;
        switch ( Prop.m_Type )
        {
          case VmbFeatureDataInt:  value = PropData.m_int ; return true ;
          case VmbFeatureDataFloat:
          {
            switch ( Prop.pr )
            {
              case FGP_OUT_DELAY:
              case FGP_EXTSHUTTER: value = (FXSIZE) ROUND( m_dExtShutter = PropData.m_double ) ; break ;
              case FGP_GAIN:       value = (FXSIZE) ROUND( PropData.m_double * 10. ) ; break ;
              case FGP_FRAME_RATE:
                if ( m_iFPSx10 == 0 )
                  value = (FXSIZE) (m_iFPSx10 = ROUND( PropData.m_double * 10. )) ;
                else
                  value = m_iFPSx10 ;
                break ;
              case FGP_WHITEBAL_RATIO: value = (FXSIZE) ROUND( PropData.m_double * 100. ) ; break ;
              default:
                m_TmpString.Format( "%g" , PropData.m_double ) ;
                value = (FXSIZE) (LPCTSTR) m_TmpString ;
                break ;
            }
          }
          return true ;
          case VmbFeatureDataEnum:
          {
            if ( Prop.pr == FGP_LINE_SELECT )
              TRACE( "\nGetProperty %s = %s, Invalidate = %d" ,
              (LPCTSTR) Prop.m_Name , PropData.m_szString , (int) PropData.m_bInvalidate ) ;

            for ( int iEnum = 0 ; iEnum < (int) Prop.m_EnumRange.GetCount() ; iEnum++ )
            {
              if ( Prop.m_EnumRange[ iEnum ] == PropData.m_szString )
              {
                value = iEnum ;
                if ( Prop.pr == FGP_LINE_SELECT )
                  m_iSelectedLineNumber = iEnum ;
                return true ;
              }
            }
            return false ;
          }
          break;
          case VmbFeatureDataString:
          {
            m_TmpString = PropData.m_szString ;
            value = (FXSIZE) (LPCTSTR) m_TmpString ;
            return true ;
          }
          case VmbFeatureDataBool:
          {
            value = (FXSIZE) PropData.m_bBool ;
            return true ;
          }
        }
      }
    }
  }
  bauto = false;
  return false ;
}
bool AVT_Vimba::SetCameraProperty( unsigned iIndex , FXSIZE &value , bool& bauto , bool& Invalidate )
{
  if ( !CheckAndAllocCamera() )
    return false;

  bool bRes = false ;
  SetCamPropertyData& Data = m_PropertyData ;
  CameraAttribute& pProp = m_PropertiesEx.GetAt( iIndex ) ;
  if ( !pProp.m_AutoControl.IsEmpty() )
  {
    Data.m_bAuto = true ;
    Data.m_bBool = bauto ;
  }
  else
    Data.m_bAuto = false ;
  switch ( pProp.m_Type )
  {
    case VmbFeatureDataInt:
    {
      Data.m_int = (int) (Data.m_int64 = value) ;
      //       if ( pProp.AutoControl )
      //       {
      //         Data.m_bAuto = true ;
      //         Data.m_bBool = bauto ;
      //       }
      //       else
      //       {
      //         Data.m_bAuto = false ;
      //         pProp.m_LastValue.iInt = value ;
      //       }
      break ;
    }
    case VmbFeatureDataFloat:
    {
      double dValue ;
      switch ( pProp.pr )
      {
        case FGP_OUT_DELAY:
        case FGP_EXTSHUTTER: dValue = (double) (value) ; break ;
        case FGP_GAIN:       dValue = (double) value / 10. ; break ;
        case FGP_FRAME_RATE:
          dValue = (double) value / 10. ;
          m_iFPSx10 = (int) value ;
          break ;
        case FGP_WHITEBAL_RATIO: dValue = (double) value / 100. ; break ;
        case FGP_TEMPERATURE_S:
          _tcscpy_s( Data.m_szString , (LPCTSTR) (size_t) value ) ;
        default: dValue = atof( (LPCTSTR) value ) ; break ;
      }
      Data.m_double = dValue ;
      break ;
    }
    case VmbFeatureDataEnum:
    {
      if ( (0 <= value) && ((int) value < pProp.m_EnumRange.GetCount()) )
        strcpy_s( Data.m_szString , (LPCTSTR) pProp.m_EnumRange[ value ] ) ;
      else
      {
        SENDERR( "Set Property %s bad enum index %d[0,%d]" , pProp.m_Name ,
          value , pProp.m_EnumRange.GetCount() ) ;
        return false ;
      }
      pProp.m_stringVal = Data.m_szString  ;
      if ( pProp.pr == FGP_LINE_SELECT )
        TRACE( "\nBefore SetPropertyEx %s = %s, Invalidate = %d" ,
        (LPCTSTR) pProp.m_Name , Data.m_szString , (int) Data.m_bInvalidate ) ;
      break ;
    }
    case VmbFeatureDataString:
    {
      strcpy_s( Data.m_szString , (LPCTSTR) value ) ;
      pProp.m_stringVal = Data.m_szString ;
      break ; ;
    }
    break;
    case VmbFeatureDataBool:
    {
      Data.m_bBool = (value != NULL) ;
      pProp.m_boolVal = Data.m_bBool ;
      break ;
    }
    default:
      if ( pProp.pr == FGP_ROI )
      {
        strcpy_s( Data.m_szString , (LPCTSTR) value ) ;
        pProp.m_stringVal = Data.m_szString ;
        break ;
      }
      else if ( pProp.pr == FGP_LOG )
        break ;
      else
      {
        C1394_SENDERR_2( "Undefined property '%s' type %d" , pProp.m_Name , pProp.m_Type ) ;
        return false ;
      }
  }
  if ( IsRunning() )
  {
    if ( pProp.m_bIsStopNecessary )
    {
      bRes = OtherThreadCameraStop() ;
      if ( bRes )
      {
        m_bShouldBeReprogrammed = m_bWasStopped = true ;
      }
      else
      {
        C1394_SENDERR_1( "Can't stop grab for property '%s' change" , (LPCTSTR) pProp.m_Name ) ;
        return false ;
      }
    }
  }
  if ( pProp.pr == FGP_LOG )
  {
    FXSIZE iPos = 0 ;
    m_LogLock.Lock() ;
    m_PropertiesForLogAsString = (LPCTSTR) value ;
    FXString Token = m_PropertiesForLogAsString.Tokenize( _T( ", \t" ) , iPos ) ;
    if ( Token.IsEmpty() )
    {
      m_dLogPeriod_ms = 0. ;
    }
    else
    {
      double dPeriod = atof( Token ) ;
      if ( dPeriod <= 0.0 )
        m_dLogPeriod_ms = 0. ;
      else
      {
        m_dLogPeriod_ms = dPeriod ;
        m_PropertiesForLog.RemoveAll() ;
        while ( !(Token = m_PropertiesForLogAsString.Tokenize( _T( ", \t" ) , iPos )).IsEmpty() )
          m_PropertiesForLog.Add( Token ) ;
      }
      m_LogOutString = m_PropertiesForLogAsString + _T( "\n" ) ;
      m_iLogCnt = 0 ;
      m_BusEvents |= AVT_EVT_LOG ;
      SetEvent( m_evCameraControl ) ;
    }
    m_LogLock.Unlock() ;
    bRes = true ;
  }
  else
    bRes = OtherThreadSetProperty( iIndex , &Data , &Invalidate ) ;
  if ( bRes )
  {
    switch ( pProp.pr )
    {
      case FGP_LINE_SELECT:
      case FGP_LINEIN_SELECT:
      case FGP_LINEOUT_SELECT:
    {
      SetCamPropertyData DataAfter ;
      TRACE( "\nAfter SetPropertyEx %s = %s, Invalidate = %d" ,
        (LPCTSTR) pProp.m_Name , Data.m_szString , (int) Data.m_bInvalidate ) ;
      if ( GetPropertyValue( m_pCamera , (LPCTSTR) pProp.m_CameraPropertyName , DataAfter ) )
      {
        TRACE( "\nReal Value for %s = %s" , (LPCTSTR) pProp.m_Name , DataAfter.m_szString ) ;
      }
      else
        ASSERT( 0 ) ;
      m_iSelectedLineNumber = (int) value ;
      Invalidate = true ;
    }
      break ;
      case FGP_TRIGGERONOFF:
        Invalidate = true ;
        break ;
    }
  }

  return bRes ;

}


bool AVT_Vimba::AVTSaveOrLoadSettings( int iMode )  // 0 - nothing to do, 1 - Save, 2 - Load
{
  if ( iMode == 0 )
    return true ;
  if ( iMode < 0 || iMode > 2 )
    return false ;

  AVT::VmbAPI::FeaturePtr PropPtr ;
  VmbErrorType LastError = m_pCamera->GetFeatureByName( (LPCTSTR) m_UserSetSelector , PropPtr ) ;
  if ( LastError != VmbErrorSuccess )
  {
    SENDERR( "\nSetCameraPropertyEx - Can't get property by name %s: %s" ,
      (LPCTSTR) m_UserSetSelector , ErrorCodeToMessage( LastError ).c_str() ) ;
    TRACE( "\nSetCameraPropertyEx - Can't get property by name %s: %s" ,
      (LPCTSTR) m_UserSetSelector , ErrorCodeToMessage( LastError ).c_str() ) ;
    return false ;
  }
  LastError = PropPtr->SetValue( 1 ) ; //allways set 1
  if ( LastError != VmbErrorSuccess )
  {
    SENDERR( "\nSetCameraPropertyEx - Can't set value %s to 1: %s" ,
      (LPCTSTR) m_UserSetSelector , ErrorCodeToMessage( LastError ).c_str() ) ;
    TRACE( "\nSetCameraPropertyEx - Can't set value %s to 1: %s" ,
      (LPCTSTR) m_UserSetSelector , ErrorCodeToMessage( LastError ).c_str() ) ;
    return false ;
  }

  AVT::VmbAPI::FeaturePtr pCommandFeature;
  FXString Command = (iMode == 1) ? m_SaveSettingsCommand : m_LoadSettingsCommand ;
  LastError = m_pCamera->GetFeatureByName(
    (LPCTSTR) Command , pCommandFeature ) ;
  if ( LastError != VmbErrorSuccess )
  {
    SENDERR( "\nSetCameraPropertyEx - Can't get command by name %s: %s" ,
      (LPCTSTR) Command , ErrorCodeToMessage( LastError ).c_str() ) ;
    TRACE( "\nSetCameraPropertyEx - Can't get command by name %s: %s" ,
      (LPCTSTR) Command , ErrorCodeToMessage( LastError ).c_str() ) ;
    return false ;
  }
  LastError = pCommandFeature->RunCommand() ;
  if ( LastError != VmbErrorSuccess )
  {
    SENDERR( "\nSetCameraPropertyEx - Can't run command %s: %s" ,
      (LPCTSTR) Command , ErrorCodeToMessage( LastError ).c_str() ) ;
    TRACE( "\nSetCameraPropertyEx - Can't run command %s: %s" ,
      (LPCTSTR) Command , ErrorCodeToMessage( LastError ).c_str() ) ;
    return false ;
  }
  double dRunStartTime = GetHRTickCount() ;
  bool bFinished = false ;
  do
  {
    m_LastError = pCommandFeature->IsCommandDone( bFinished ) ;
    if ( LastError != VmbErrorSuccess )
    {
      SENDERR( "\nSetCameraPropertyEx - Can't check finish run %s: %s" ,
        (LPCTSTR) Command , ErrorCodeToMessage( LastError ).c_str() ) ;
      TRACE( "\nSetCameraPropertyEx - Can't check finish run %s: %s" ,
        (LPCTSTR) Command , ErrorCodeToMessage( LastError ).c_str() ) ;
      return false ;
    }
  } while ( !bFinished && (GetHRTickCount() - dRunStartTime < 2000.) ) ;
  if ( !bFinished )
  {
    SENDERR( "\nSetCameraPropertyEx - Command %s is not finshed in 2000 ms" ,
      (LPCTSTR) Command ) ;
    TRACE( "\nSetCameraPropertyEx - Command %s is not finshed in 2000 ms" ,
      (LPCTSTR) Command ) ;
    return false ;
  }

  switch ( iMode )
  {
    case 0: return true ; // no operation
    case 1:// save
    {
      LastError = m_pCamera->GetFeatureByName( (LPCTSTR) m_SetDefaultSettings , PropPtr ) ;
      if ( LastError != VmbErrorSuccess )
      {
        SENDERR( "\nSetCameraPropertyEx - Can't get property by name %s: %s" ,
          (LPCTSTR) m_SetDefaultSettings , ErrorCodeToMessage( LastError ).c_str() ) ;
        TRACE( "\nSetCameraPropertyEx - Can't get property by name %s: %s" ,
          (LPCTSTR) m_SetDefaultSettings , ErrorCodeToMessage( LastError ).c_str() ) ;
        return false ;
      }
      if ( m_bSetDefaultSettingsIsEnum )
      {
        AVT::VmbAPI::EnumEntryVector Enumerators ;
        m_LastError = PropPtr->GetEntries( Enumerators ) ;
        if ( LastError != VmbErrorSuccess )
        {
          SENDERR( "\nSetCameraPropertyEx - Can't get enums for %s: %s" ,
            (LPCTSTR) m_SetDefaultSettings , ErrorCodeToMessage( LastError ).c_str() ) ;
          TRACE( "\nSetCameraPropertyEx - Can't get enums for %s: %s" ,
            (LPCTSTR) m_SetDefaultSettings , ErrorCodeToMessage( LastError ).c_str() ) ;
          return false ;
        }
        if ( Enumerators.size() < 2 )
        {
          SENDERR( "\nSetCameraPropertyEx - Not enough enums for %s(%d): %s" ,
            (LPCTSTR) m_SetDefaultSettings , Enumerators.size() ,
            ErrorCodeToMessage( LastError ).c_str() ) ;
        }
        std::string Enum ;
        Enumerators.at( 1 ).GetName( Enum ) ;
        m_LastError = PropPtr->SetValue( Enum.c_str() ) ;
        if ( LastError != VmbErrorSuccess )
        {
          SENDERR( "\nSetCameraPropertyEx - Can't set %s to %s: %s" ,
            (LPCTSTR) m_SetDefaultSettings , Enum.c_str() ,
            ErrorCodeToMessage( LastError ).c_str() ) ;
          TRACE( "\nSetCameraPropertyEx - Can't set %s to %s: %s" ,
            (LPCTSTR) m_SetDefaultSettings , Enum.c_str() ,
            ErrorCodeToMessage( LastError ).c_str() ) ;
          return false ;
        }
      }
      else  // Set default set is command, not enumerator operation
      {
        LastError = PropPtr->RunCommand() ;
        if ( LastError != VmbErrorSuccess )
        {
          SENDERR( "\nSetCameraPropertyEx - Can't run command %s: %s" ,
            (LPCTSTR) m_SaveSettingsCommand , ErrorCodeToMessage( LastError ).c_str() ) ;
          TRACE( "\nSetCameraPropertyEx - Can't run command %s: %s" ,
            (LPCTSTR) m_SaveSettingsCommand , ErrorCodeToMessage( LastError ).c_str() ) ;
          return false ;
        }
      }
      return true ;
    }
    case 2: // Load, all is done before
      return true ;
  }
  return false ;
}

bool AVT_Vimba::SetCameraPropertyEx( unsigned iIndex , SetCamPropertyData * pData , bool& Invalidate )
{
  VmbErrorType LastError ;
  TCHAR Msg[ 300 ] ;
  if ( !m_pCamera )
  {
    SENDERR( "\nSetCameraPropertyEx - No ptr to camera for SN%u " , m_dwSerialNumber ) ;
    TRACE( "\nSetCameraPropertyEx - No ptr to camera for SN%u " , m_dwSerialNumber ) ;
    return false ;
  }
  if ( iIndex >= (unsigned) m_PropertiesEx.GetCount() )
  {
    SENDERR( "\nSetCameraPropertyEx - No property for index %i " , iIndex ) ;
    TRACE( "\nSetCameraPropertyEx - No property for index %i " , iIndex ) ;
    return false ;
  }

  //   CamProperty& Prop = m_pOrigProperties[iIndex] ;
  CameraAttribute& Prop = m_PropertiesEx.GetAt( iIndex ) ;
  switch ( Prop.pr )
  {
    case FGP_ROI:
    {
      CRect rc;
      if ( sscanf( (LPCTSTR) pData->m_szString , "%d,%d,%d,%d" , &rc.left , &rc.top , &rc.right , &rc.bottom ) == 4 )
      {
        rc.right += rc.left ;
        rc.bottom += rc.top ;
        SetROI( rc );
        return true;
      }
      return false;
    }
    case FGP_LOG:
      strcpy_s( pData->m_szString , sizeof( pData->m_szString ) ,
        (LPCTSTR) m_PropertiesForLogAsString ) ;
      return true ;
    case FGP_USER_SET_SELECT:
    {
      return AVTSaveOrLoadSettings( pData->m_int ) ;
    }
    case FGP_TEMPERATURE_S:
    {
      if ( m_pOrigProperties == GoldEye )
      {
        double dTargetTemp = atof( pData->m_szString ) ;
        SEND_GADGET_INFO( _T( "Target T=%.2f, Src=%s" ) , dTargetTemp , pData->m_szString ) ;
        SetCamPropertyData Data ;
        _tcscpy_s( Data.m_szString , _T( "TemperatureControl" ) );
        SetPropertyValue( _T( "SensorTemperatureControlMode" ) , Data ) ;
        Data.m_double = dTargetTemp ;
        Data.m_int64 = Data.m_int = ROUND( dTargetTemp ) ;
        SetPropertyValue( _T( "SensorTemperatureSetpointValue" ) , Data ) ;
        RunCameraCommand( _T( "SensorTemperatureSetpointActivate" ) ) ;
        return true ;
      }
    }
    default:
    {
      //       LastError = m_pCamera->GetFeatureByName( Prop.CamPropertyName , Prop.pInCamera ) ;
      //       if ( LastError != VmbErrorSuccess )
      //       {
      //         SENDERR("\nSetCameraPropertyEx - Can't get property by name %s: %s", 
      //           Prop.name , ErrorCodeToMessage( LastError).c_str() ) ;
      //         TRACE("\nSetCameraPropertyEx - Can't get property by name %s: %s", 
      //           Prop.name , ErrorCodeToMessage( LastError).c_str() ) ;
      //         return false ;
      //       }
      AVT::VmbAPI::FeaturePtr PropPtr ;
      LastError = m_pCamera->GetFeatureByName( Prop.m_CameraPropertyName , PropPtr ) ;
      if ( LastError != VmbErrorSuccess )
      {
        SENDERR( "\nSetCameraPropertyEx - Can't get property by name %s: %s" ,
          (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( LastError ).c_str() ) ;
        TRACE( "\nSetCameraPropertyEx - Can't get property by name %s: %s" ,
          (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( LastError ).c_str() ) ;
        return false ;
      }
      bool bAutoOn = false ;
      if ( !Prop.m_AutoControl.IsEmpty() )
      {
        AVT::VmbAPI::FeaturePtr AutoPtr ;
        LastError = m_pCamera->GetFeatureByName( Prop.m_AutoControl , AutoPtr ) ;
        if ( LastError != VmbErrorSuccess )
        {
          SENDERR( "\nSetCameraPropertyEx - Can't get auto property '%s' for %s: %s" ,
            (LPCTSTR) Prop.m_AutoControl , (LPCTSTR) Prop.m_Name ,
            ErrorCodeToMessage( LastError ).c_str() ) ;
          TRACE( "\nSetCameraPropertyEx - Can't get auto property '%s' for %s: %s" ,
            (LPCTSTR) Prop.m_AutoControl , (LPCTSTR) Prop.m_Name ,
            ErrorCodeToMessage( LastError ).c_str() ) ;
          return false ;
        }
        std::string AutoValue ;
        LastError = AutoPtr->GetValue( AutoValue ) ;
        if ( LastError != VmbErrorSuccess )
        {
          SENDERR( "\nSetCameraPropertyEx - Can't get auto value %s: %s" ,
            (LPCTSTR) Prop.m_AutoControl , ErrorCodeToMessage( LastError ).c_str() ) ;
          TRACE( "\nSetCameraPropertyEx - Can't get auto value %s: %s" ,
            (LPCTSTR) Prop.m_AutoControl , ErrorCodeToMessage( LastError ).c_str() ) ;
          return false ;
        }
        bAutoOn = (Prop.m_EnumRange[ 0 ] != AutoValue.c_str()) ;
        if ( bAutoOn != pData->m_bBool )
        {
          LPCTSTR pValue = (LPCTSTR)
            Prop.m_EnumRange[ (pData->m_bBool) ? Prop.m_EnumRange.GetUpperBound() : 0 ] ;
          LastError = AutoPtr->SetValue( pValue ) ;
          if ( LastError != VmbErrorSuccess )
          {
            SENDERR( "\nSetCameraPropertyEx - Can't set auto control %s: %s" ,
              (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( LastError ).c_str() ) ;
            TRACE( "\nSetCameraPropertyEx - Can't set auto control %s: %s" ,
              (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( LastError ).c_str() ) ;
          }
          else
            bAutoOn = pData->m_bBool ;
        }
      }
      switch ( Prop.m_Type )
      {
        case VmbFeatureDataInt:
        {
          if ( Prop.m_AutoControl.IsEmpty() || !bAutoOn )
          {
            VmbInt64_t Value64 = pData->m_int ;
            LastError = PropPtr->SetValue( Value64 ) ;
            if ( LastError != VmbErrorSuccess )
            {
              SENDERR( "\nSetCameraPropertyEx - Can't set int data for property %s: %s" ,
                (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( LastError ).c_str() ) ;
              TRACE( "\nSetCameraPropertyEx - Can't set int data for property %s: %s" ,
                (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( LastError ).c_str() ) ;
              return false ;
            }
          }
          return true ;
        }
        case VmbFeatureDataFloat:
        {
          if ( Prop.m_AutoControl.IsEmpty() || !bAutoOn )
          {
            if ( Prop.pr == FGP_OUT_DELAY )
            {
              SetCamPropertyData Tmp ;
              Tmp.m_bBool = (pData->m_double > 1.) ;
              SetPropertyValue( m_pCamera , "IntEnaDelayEnable" , Tmp ) ;
              if ( pData->m_double <= 1. )
                break ; // simple disable, not necessary to set delay
            }
            if ( Prop.pr == FGP_FRAME_RATE && pData->m_double > 1. )
            {
              double dPeriod = 1.E6 / pData->m_double ;
              if ( m_dExtShutter != 0.  &&  dPeriod < m_dExtShutter )
                pData->m_double = 1.E6 / m_dExtShutter ;
              m_iFPSx10 = ROUND( 10. * pData->m_double ) ;
            }
            if ( Prop.pr == FGP_EXTSHUTTER && pData->m_double > 0.1 )
            {
              int iFPSLimitx10 = (int) (1.E7 / pData->m_double) ;
              if ( m_iFPSx10 > iFPSLimitx10 )
                m_iFPSx10 = iFPSLimitx10 ;
              else
              {
                SetCamPropertyData FPSData ;
                GetPropertyValue( m_pCamera , m_FPSPropertyName , FPSData ) ;
                m_iFPSx10 = ROUND( FPSData.m_double * 10. ) ;
              }
            }
            LastError = PropPtr->SetValue( pData->m_double ) ;
            if ( LastError != VmbErrorSuccess )
            {
              if ( LastError == VmbErrorInvalidAccess )
              {
                VmbAccessModeType Access ;
                VmbErrorType LastError = m_pCamera->GetPermittedAccess( Access ) ;
                if ( LastError == VmbErrorSuccess )
                {
                  if ( Access != VmbAccessModeFull )
                  {
                    m_pCamera->Close() ;
                    LastError = m_pCamera->Open( VmbAccessModeFull ) ;
                    if ( LastError != VmbErrorSuccess )
                    {
                      _stprintf( Msg ,
                        "\n    AVTBuildPropertyList: can't open camera #%d for control (Err=%d)" ,
                        m_dwSerialNumber , (int) LastError ) ;
                      TRACE( Msg ) ;
                      SENDERR( Msg ) ;
                      return false ;
                    }
                    VmbAccessModeType Mode ;
                    m_pCamera->GetPermittedAccess( Mode ) ;
                    int a = (int) Mode ;
                  }
                  LastError = PropPtr->SetValue( pData->m_double ) ;
                }
              }
              if ( LastError != VmbErrorSuccess )
              {
                SENDERR( "\nSetCameraPropertyEx - Can't set float data %g for property %s: %s" ,
                  pData->m_double , (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( LastError ).c_str() ) ;
                TRACE( "\nSetCameraPropertyEx - Can't set float data for property %s: %s" ,
                  (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( LastError ).c_str() ) ;
                return false ;
              }
            }
            if ( Prop.pr == FGP_EXTSHUTTER )
            {
              SetCamPropertyData FPSData ;
              FPSData.m_double = (double) m_iFPSx10 / 10. ;
              SetPropertyValue( m_pCamera , m_FPSPropertyName , FPSData ) ;
            }
          }
          return true ;
        }
        case VmbFeatureDataEnum:
        case VmbFeatureDataString:
        {
          if ( Prop.pr == FGP_TRIGGER_SOURCE )
          {
            AVT::VmbAPI::FeaturePtr TriggerModePtr ;
            LastError = m_pCamera->GetFeatureByName( m_TriggerModeName , TriggerModePtr ) ; // retrieve trigger mode
            if ( LastError != VmbErrorSuccess )
            {
              SENDERR( "\nSetCameraPropertyEx - Can't get by name %s: %s" ,
                (LPCTSTR) m_TriggerModeName , ErrorCodeToMessage( LastError ).c_str() ) ;
              TRACE( "\nSetCameraPropertyEx - Can't get by name %s: %s" ,
                (LPCTSTR) m_TriggerModeName , ErrorCodeToMessage( LastError ).c_str() ) ;
              return false ;
            }
            FXString Set( pData->m_szString ) ;
            m_bVimbaSoftTrigger = (Set.Find( "Software" ) >= 0) ;
            bool bSetTrigger = (Set.Find( _T( "Line" ) ) >= 0 || m_bVimbaSoftTrigger);
            LastError = TriggerModePtr->SetValue(
              (LPCTSTR) ((bSetTrigger) ? m_TriggerOn : m_TriggerOff) ) ;  // switch on or off trigger
            if ( LastError != VmbErrorSuccess )
            {
              SENDERR( "\nSetCameraPropertyEx - Can't set %s to %s: %s" ,
                (LPCTSTR) m_TriggerModeName , (LPCTSTR) m_TriggerOn ,
                ErrorCodeToMessage( LastError ).c_str() ) ;
              TRACE( "\nSetCameraPropertyEx - Can't set enum/string data for property %s: %s" ,
                (LPCTSTR) Prop.m_Name , (LPCTSTR) m_TriggerOn ,
                ErrorCodeToMessage( LastError ).c_str() ) ;
              return false ;
            }
            if ( !bSetTrigger )
              return true ;
            m_TriggerMode = (bSetTrigger) ? TriggerOn : TriggerOff ;

            LastError = m_pCamera->GetFeatureByName( m_TriggerSourceName , PropPtr ) ;
            LastError = PropPtr->SetValue( pData->m_szString ) ; // 
            if ( LastError != VmbErrorSuccess )
            {
              SENDERR( "\nSetCameraPropertyEx - Can't Set Property %s to %s: %s" ,
                (LPCTSTR) Prop.m_CameraPropertyName , pData->m_szString ,
                ErrorCodeToMessage( LastError ).c_str() ) ;
              TRACE( "\nSetCameraPropertyEx - Can't Set Property %s to %s: %s" ,
                (LPCTSTR) Prop.m_CameraPropertyName , pData->m_szString ,
                ErrorCodeToMessage( LastError ).c_str() ) ;
              return false ;
            }
            return true ;
          }
          LastError = PropPtr->SetValue( pData->m_szString ) ;
          if ( LastError != VmbErrorSuccess )
          {
            SENDERR( "\nSetCameraPropertyEx - Can't set property %s to %s: %s" ,
              (LPCTSTR) Prop.m_Name , pData->m_szString , ErrorCodeToMessage( LastError ).c_str() ) ;
            TRACE( "\nSetCameraPropertyEx - Can't set property %s to %s: %s" ,
              (LPCTSTR) Prop.m_Name , pData->m_szString , ErrorCodeToMessage( LastError ).c_str() ) ;
            return false ;
          }
          if ( Prop.pr == FGP_LINE_SELECT )
          {
            m_SelectedLine = pData->m_szString ;
            Invalidate = pData->m_bInvalidate = true ;
          #ifdef _DEBUG
            std::string BackRead ;
            LastError = PropPtr->GetValue( BackRead ) ;
            TRACE( "\nSetPropertyEx %s = %s(%s), Invalidate = %d" , Prop.m_Name ,
              pData->m_szString , BackRead.c_str() , (int) pData->m_bInvalidate ) ;
          #endif
          }
          else if ( Prop.pr == FGP_TRIGGER_SOURCE )
            Invalidate = pData->m_bInvalidate = true ;
          return true ;
        }
        break;
        case VmbFeatureDataBool:
        {
          LastError = PropPtr->SetValue( pData->m_bBool ) ;
          if ( LastError != VmbErrorSuccess )
          {
            SENDERR( "\nSetCameraPropertyEx - Can't set bool data for property %s: %s" ,
              (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( LastError ).c_str() ) ;
            TRACE( "\nSetCameraPropertyEx - Can't set bool data for property %s: %s" ,
              (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( LastError ).c_str() ) ;
            return false ;
          }
          return true ;
        }
      }
    }
  }
  return false ;
}

bool AVT_Vimba::AVTSetCameraPropertyEx( LPCTSTR pName , SetCamPropertyData * pData )
{
  VmbErrorType LastError ;
  if ( !m_pCamera )
  {
    SENDERR( "\nAVTSetCameraPropertyEx - No ptr to camera for SN%u " , m_dwSerialNumber ) ;
    TRACE( "\nAVTSetCameraPropertyEx - No ptr to camera for SN%u " , m_dwSerialNumber ) ;
    return false ;
  }
  {
    AVT::VmbAPI::FeaturePtr PropPtr ;
    LastError = m_pCamera->GetFeatureByName( pName , PropPtr ) ;
    if ( LastError != VmbErrorSuccess )
    {
      SENDERR( "\nAVTSetCameraPropertyEx - Can't get property by name %s: %s" ,
        pName , ErrorCodeToMessage( LastError ).c_str() ) ;
      TRACE( "\nAVTSetCameraPropertyEx - Can't get property by name %s: %s" ,
        pName , ErrorCodeToMessage( LastError ).c_str() ) ;
      return false ;
    }
    VmbFeatureDataType Type ;
    LastError = PropPtr->GetDataType( Type ) ;
    if ( LastError != VmbErrorSuccess )
    {
      SENDERR( "\nAVTSetCameraPropertyEx - Can't get property type for %s: %s" ,
        pName , ErrorCodeToMessage( LastError ).c_str() ) ;
      TRACE( "\nAVTSetCameraPropertyEx - Can't get property type for %s: %s" ,
        pName , ErrorCodeToMessage( LastError ).c_str() ) ;
      return false ;
    }
    switch ( Type )
    {
      case VmbFeatureDataInt:
      {
        VmbInt64_t Value64 = pData->m_int ;
        LastError = PropPtr->SetValue( Value64 ) ;
        if ( LastError != VmbErrorSuccess )
        {
          SENDERR( "\nAVTSetCameraPropertyEx - Can't set int data for property %s: %s" ,
            pName , ErrorCodeToMessage( LastError ).c_str() ) ;
          TRACE( "\nAVTSetCameraPropertyEx - Can't set int data for property %s: %s" ,
            pName , ErrorCodeToMessage( LastError ).c_str() ) ;
          return false ;
        }
        return true ;
      }
      case VmbFeatureDataFloat:
      {
        LastError = PropPtr->SetValue( pData->m_double ) ;
        if ( LastError != VmbErrorSuccess )
        {
          if ( LastError != VmbErrorSuccess )
          {
            SENDERR( "\nAVTSetCameraPropertyEx - Can't set float data %g for property %s: %s" ,
              pData->m_double , pName , ErrorCodeToMessage( LastError ).c_str() ) ;
            TRACE( "\nAVTSetCameraPropertyEx - Can't set float data for property %s: %s" ,
              pName , ErrorCodeToMessage( LastError ).c_str() ) ;
            return false ;
          }
        }
        return true ;
      }
      case VmbFeatureDataEnum:
      case VmbFeatureDataString:
      {
        LastError = PropPtr->SetValue( pData->m_szString ) ;
        if ( LastError != VmbErrorSuccess )
        {
          SENDERR( "\nAVTSetCameraPropertyEx - Can't set property %s to %s: %s" ,
            pName , pData->m_szString , ErrorCodeToMessage( LastError ).c_str() ) ;
          TRACE( "\nAVTSetCameraPropertyEx - Can't set property %s to %s: %s" ,
            pName , pData->m_szString , ErrorCodeToMessage( LastError ).c_str() ) ;
          return false ;
        }
        return true ;
      }
      break;
      case VmbFeatureDataBool:
      {
        LastError = PropPtr->SetValue( pData->m_bBool ) ;
        if ( LastError != VmbErrorSuccess )
        {
          SENDERR( "\nAVTSetCameraPropertyEx - Can't set bool data for property %s: %s" ,
            pName , ErrorCodeToMessage( LastError ).c_str() ) ;
          TRACE( "\nAVTSetCameraPropertyEx - Can't set bool data for property %s: %s" ,
            pName , ErrorCodeToMessage( LastError ).c_str() ) ;
          return false ;
        }
        return true ;
      }
    }
  }
  return false ;
}

bool AVT_Vimba::GetCameraPropertyEx( int iIndex , SetCamPropertyData * pData )
{
  if ( m_TmpPropertyName[ 0 ] != 0 )
    return GetCameraPropertyEx( m_TmpPropertyName , pData ) ;

  CameraAttribute& Prop = m_PropertiesEx.GetAt( iIndex ) ;
  m_PropertyData.m_szString[ 0 ] = 0 ;
  switch ( Prop.pr )
  {
    case FGP_ROI:
    {
      CRect rc;
      return GetROI( rc );
    }
    case FGP_LOG:
      strcpy_s( pData->m_szString , sizeof( pData->m_szString ) ,
        (LPCTSTR) m_PropertiesForLogAsString ) ;
      return true ;
    case FGP_USER_SET_SELECT:
    {
      pData->m_int = 0 ;
      return true ;
    }
    case FGP_TEMPERATURE_S:
    {
      if ( m_pOrigProperties == GoldEye )
      {
        SetCamPropertyData Data ;
        if ( AVTGetCameraPropertyEx( _T( "DeviceTemperature" ) , &Data ) )
        {
          double dTemp = Data.m_double ;
          FXString Out ;
          if ( AVTGetCameraPropertyEx( _T( "SensorCoolingPower" ) , &Data ) )
          {
            Out.Format( _T( "%.2f C, %d mW" ) , dTemp , Data.m_int ) ;
          }
          else
            Out.Format( _T( "%.2f C, %d mW" ) , dTemp , Data.m_int ) ;

          _tcscpy_s( pData->m_szString , (LPCTSTR) Out ) ;
          return true ;
        }
      }
      return false ;
    }
    default:
    {
      AVT::VmbAPI::FeaturePtr PropPtr ;
      VmbErrorType LastError = m_pCamera->GetFeatureByName( Prop.m_CameraPropertyName , PropPtr ) ;
      if ( LastError != VmbErrorSuccess )
      {
        SEND_DEVICE_ERR( "\nGetCameraPropertyEx - Can't get property by name %s: %s" ,
          (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( LastError ).c_str() ) ;
        TRACE( "\nGetCameraPropertyEx - Can't get property by name %s: %s" ,
          (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( LastError ).c_str() ) ;
        return false ;
      }
      if ( !Prop.m_AutoControl.IsEmpty() )
      {
        AVT::VmbAPI::FeaturePtr AutoPtr ;
        LastError = m_pCamera->GetFeatureByName( Prop.m_AutoControl , AutoPtr ) ;
        if ( LastError != VmbErrorSuccess )
        {
          SEND_DEVICE_ERR( "\nSetCameraPropertyEx - Can't get auto property by name %s: %s" ,
            (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( LastError ).c_str() ) ;
          TRACE( "\nSetCameraPropertyEx - Can't get auto property by name %s: %s" ,
            (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( LastError ).c_str() ) ;
          return false ;
        }
        std::string AutoValue ;
        LastError = AutoPtr->GetValue( AutoValue ) ;
        if ( LastError != VmbErrorSuccess )
        {
          SEND_DEVICE_ERR( "\nSetCameraPropertyEx - Can't get auto value %s: %s" ,
            (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( LastError ).c_str() ) ;
          TRACE( "\nSetCameraPropertyEx - Can't get auto value %s: %s" ,
            (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( LastError ).c_str() ) ;
          return false ;
        }
        pData->m_bBool = (Prop.m_EnumRange[ 0 ] != AutoValue.c_str()) ;
        pData->m_bAuto = true ;
      }
      switch ( Prop.m_Type )
      {
        case VmbFeatureDataInt:
        {
          VmbInt64_t Value64 ;
          m_LastError = PropPtr->GetValue( Value64 ) ;
          if ( m_LastError != VmbErrorSuccess )
          {
            SEND_DEVICE_ERR( "Can't get int data for property %s: %s" ,
              (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( m_LastError ).c_str() ) ;
            return false ;
          }
          pData->m_int = (int) Value64 ;
          return true ;
        }
        case VmbFeatureDataFloat:
        {
          m_LastError = PropPtr->GetValue( pData->m_double ) ;
          if ( m_LastError != VmbErrorSuccess )
          {
            SEND_DEVICE_ERR( "Can't get double data for property %s: %s" ,
              (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( m_LastError ).c_str() ) ;
            return false ;
          }
          return true ;
        }
        case VmbFeatureDataEnum:
        case VmbFeatureDataString:
        {
          std::string Value ;
          m_LastError = PropPtr->GetValue( Value ) ;
          if ( m_LastError != VmbErrorSuccess )
          {
            SEND_DEVICE_ERR( "Can't get %s data for property %s: %s" ,
              (Prop.m_Type == VmbFeatureDataEnum) ? "Enum" : "String" ,
              (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( m_LastError ).c_str() ) ;
            return false ;
          }
          strcpy_s( pData->m_szString , Value.c_str() ) ;
          if ( Prop.pr == FGP_LINE_SELECT )
          {
            m_SelectedLine = pData->m_szString ;

            pData->m_bInvalidate = true ;
            TRACE( "\n GetPropertyEx %s = %s" , (LPCTSTR) Prop.m_Name , Value.c_str() ) ;
            FXString LineParams ;
            SetCamPropertyData Data ;
            m_bViewErrorMessagesOnGetSet = false ;

            if ( GetPropertyValue( m_pCamera , "LineMode" , Data ) )
            {
              LineParams = Data.m_szString ;
              if ( LineParams == "Input" )
              {
                if ( GetPropertyValue( m_pCamera , "LineRouting" , Data ) )
                {
                  LineParams += ',' ;
                  LineParams += Data.m_szString ;
                }
                if ( GetPropertyValue( m_pCamera , "LineInverter" , Data ) )
                {
                  LineParams += ',' ;
                  LineParams += (Data.m_bBool) ? "Inverted" : "NonInverted" ;
                }
                //                 if ( GetPropertyValue( m_pCamera , "LineDebounceTime" , Data) )
                //                 {
                //                   LineParams += ',' ;
                //                   sprintf_s( Data.m_szString , "%g" , Data.m_double ) ;
                //                   LineParams += Data.m_szString ;
                //                 }
                if ( GetPropertyValue( m_pCamera , "LineStatus" , Data ) )
                {
                  LineParams += ',' ;
                  LineParams += (Data.m_bBool) ? "On" : "Off" ;
                }
                m_SelectedLineParams = LineParams ;
              }
              else if ( LineParams == "Output" )
              {
                if ( GetPropertyValue( m_pCamera , "LineSource" , Data ) )
                {
                  LineParams += ',' ;
                  LineParams += Data.m_szString ;
                  if ( !strcmp( Data.m_szString , "PWM" ) )
                  {
                    if ( GetPropertyValue( m_pCamera , "LineModulationPeriod" , Data ) )
                    {
                      LineParams += ',' ;
                      sprintf_s( Data.m_szString , "%d" , (int) Data.m_int64 ) ;
                      LineParams += Data.m_szString ;
                    }
                    if ( GetPropertyValue( m_pCamera , "LineModulationPulseWidth" , Data ) )
                    {
                      LineParams += ',' ;
                      sprintf_s( Data.m_szString , "%d" , (int) Data.m_int64 ) ;
                      LineParams += Data.m_szString ;
                    }
                  }
                }
                if ( GetPropertyValue( m_pCamera , "LineInverter" , Data ) )
                {
                  LineParams += ',' ;
                  LineParams += (Data.m_bBool) ? "Inverted" : "NonInverted" ;
                }
                if ( GetPropertyValue( m_pCamera , "LineFormat" , Data ) )
                {
                  LineParams += ',' ;
                  LineParams += Data.m_szString ;
                }
                m_SelectedLineParams = LineParams ;
              }
            }
            m_bViewErrorMessagesOnGetSet = true ;

          }
          else if ( Prop.pr == FGP_LINEIN_SELECT )
          {
            m_InSelectedLine = pData->m_szString ;

            pData->m_bInvalidate = true ;
            TRACE( "\n GetPropertyEx %s = %s" , (LPCTSTR) Prop.m_Name , Value.c_str() ) ;
            FXString LineParams ;
            SetCamPropertyData Data ;
            m_bViewErrorMessagesOnGetSet = false ;
            if ( GetPropertyValue( m_pCamera , "LineRouting" , Data ) )
              LineParams += Data.m_szString ;
            if ( GetPropertyValue( m_pCamera , "LineInverter" , Data ) )
            {
              if ( !LineParams.IsEmpty() )
                LineParams += ',' ;
              LineParams += (Data.m_bBool) ? "Inverted" : "NonInverted" ;
            }
            //                 if ( GetPropertyValue( m_pCamera , "LineDebounceTime" , Data) )
            //                 {
            //                   LineParams += ',' ;
            //                   sprintf_s( Data.m_szString , "%g" , Data.m_double ) ;
            //                   LineParams += Data.m_szString ;
            //                 }
            if ( GetPropertyValue( m_pCamera , "LineStatus" , Data ) )
            {
              if ( !LineParams.IsEmpty() )
                LineParams += ',' ;
              LineParams += (Data.m_bBool) ? "On" : "Off" ;
            }
            m_InSelectedLineParams = LineParams ;
            m_bViewErrorMessagesOnGetSet = true ;
          }
          else if ( Prop.pr == FGP_LINEOUT_SELECT )
          {
            FXString LineParams ;
            SetCamPropertyData Data ;
            m_bViewErrorMessagesOnGetSet = false ;
            if ( GetPropertyValue( m_pCamera , "LineSource" , Data ) )
            {
              LineParams += Data.m_szString ;
              if ( !strcmp( Data.m_szString , "PWM" ) )
              {
                if ( GetPropertyValue( m_pCamera , "LineModulationPeriod" , Data ) )
                {
                  if ( !LineParams.IsEmpty() )
                    LineParams += ',' ;
                  sprintf_s( Data.m_szString , "%d" , (int) Data.m_int64 ) ;
                  LineParams += Data.m_szString ;
                }
                if ( GetPropertyValue( m_pCamera , "LineModulationPulseWidth" , Data ) )
                {
                  if ( !LineParams.IsEmpty() )
                    LineParams += ',' ;
                  sprintf_s( Data.m_szString , "%d" , (int) Data.m_int64 ) ;
                  LineParams += Data.m_szString ;
                }
              }
            }
            if ( GetPropertyValue( m_pCamera , "LineInverter" , Data ) )
            {
              if ( !LineParams.IsEmpty() )
                LineParams += ',' ;
              LineParams += (Data.m_bBool) ? "Inverted" : "NonInverted" ;
            }
            else if ( GetPropertyValue( m_pCamera , "LineOutPolarity" , Data ) )
            {
              if ( !LineParams.IsEmpty() )
                LineParams += ',' ;
              LineParams += Data.m_szString ;
            }
            if ( GetPropertyValue( m_pCamera , "LineFormat" , Data ) )
            {
              LineParams += ',' ;
              LineParams += Data.m_szString ;
            }
            m_OutSelectedLineParams = LineParams ;
            m_bViewErrorMessagesOnGetSet = true ;
          }
          else if ( Prop.pr == FGP_TRIGGER_SOURCE )
          {
            AVT::VmbAPI::FeaturePtr pTrigModeProp ;
            SetCamPropertyData ModeData ;
            VmbErrorType LastError = m_pCamera->GetFeatureByName( m_TriggerSourceName , PropPtr ) ;
            if ( LastError != VmbErrorSuccess )
            {
              SEND_DEVICE_ERR( "\nGetCameraPropertyEx - Can't get by name %s: %s" ,
                (LPCTSTR) m_TriggerSourceName , ErrorCodeToMessage( LastError ).c_str() ) ;
              TRACE( "\nGetCameraPropertyEx - Can't get by name %s: %s" ,
                (LPCTSTR) m_TriggerSourceName , ErrorCodeToMessage( LastError ).c_str() ) ;
              return false ;
            }
            m_LastError = PropPtr->GetValue( Value ) ;
            if ( m_LastError != VmbErrorSuccess )
            {
              SEND_DEVICE_ERR( "Can't get data for %s: %s" ,
                (LPCTSTR) m_TriggerSourceName , ErrorCodeToMessage( LastError ).c_str() ) ;
              return false ;
            }
            strcpy_s( pData->m_szString , Value.c_str() ) ;

            if ( GetPropertyValue( m_pCamera , m_TriggerModeName , ModeData ) )
            {
              FXString ModeState( ModeData.m_szString ) ;
              bool bTrigger = (ModeState != "Off") ;

              if ( bTrigger ) // not Off or No
                m_TriggerMode = (bTrigger) ? TriggerOn : TriggerOff ;
            }
            else
              return false ;
          }
          return true ;
        }
        break;
        case VmbFeatureDataBool:
        {
          m_LastError = PropPtr->GetValue( pData->m_bBool ) ;
          if ( m_LastError != VmbErrorSuccess )
          {
            SEND_DEVICE_ERR( "Can't get bool data for property %s: %s" ,
              (LPCTSTR) Prop.m_Name , ErrorCodeToMessage( m_LastError ).c_str() ) ;
            return false ;
          }
          return true ;
        }
      }
    }
  }
  return false ;
}

bool AVT_Vimba::GetCameraPropertyEx( LPCTSTR pszPropertyName , SetCamPropertyData * pData )
{
  return AVTGetCameraPropertyEx( pszPropertyName , pData ) ;
}
bool AVT_Vimba::AVTGetCameraPropertyEx( LPCTSTR pszPropertyName , SetCamPropertyData * pData )
{

  try
  {
  AVT::VmbAPI::FeaturePtr PropPtr ;
  VmbErrorType LastError = m_pCamera->GetFeatureByName( pszPropertyName , PropPtr ) ;
  if ( LastError != VmbErrorSuccess )
  {
    SEND_DEVICE_ERR( "\nGetCameraPropertyEx - Can't get property by name %s: %s" ,
      pszPropertyName , ErrorCodeToMessage( LastError ).c_str() ) ;
    TRACE( "\nGetCameraPropertyEx - Can't get property by name %s: %s" ,
      pszPropertyName , ErrorCodeToMessage( LastError ).c_str() ) ;
    return false ;
  }
  LastError = PropPtr->GetDataType( pData->m_Type ) ;
  if ( LastError != VmbErrorSuccess )
  {
    SEND_DEVICE_ERR( "\nGetCameraPropertyEx - Can't get property type for %s: %s" ,
      pszPropertyName , ErrorCodeToMessage( LastError ).c_str() ) ;
    TRACE( "\nGetCameraPropertyEx - Can't get property type for %s: %s" ,
      pszPropertyName , ErrorCodeToMessage( LastError ).c_str() ) ;
    return false ;
  } ;
  switch ( pData->m_Type )
  {
    case VmbFeatureDataInt:
    {
      VmbInt64_t Value64 ;
      m_LastError = PropPtr->GetValue( Value64 ) ;
      if ( m_LastError != VmbErrorSuccess )
      {
        SEND_DEVICE_ERR( "Can't get int data for property %s: %s" ,
          pszPropertyName , ErrorCodeToMessage( LastError ).c_str() ) ;
        return false ;
      }
      pData->m_int = (int) Value64 ;
      return true ;
    }
    case VmbFeatureDataFloat:
    {
      m_LastError = PropPtr->GetValue( pData->m_double ) ;
      if ( m_LastError != VmbErrorSuccess )
      {
        SEND_DEVICE_ERR( "Can't get double data for property %s: %s" ,
          pszPropertyName , ErrorCodeToMessage( LastError ).c_str() ) ;
        return false ;
      }
      return true ;
    }
    case VmbFeatureDataEnum:
    case VmbFeatureDataString:
    {
      std::string Value ;
      m_LastError = PropPtr->GetValue( Value ) ;
      if ( m_LastError != VmbErrorSuccess )
      {
        SEND_DEVICE_ERR( "Can't get %s data for property %s: %s" ,
          (pData->m_Type == VmbFeatureDataEnum) ? "Enum" : "String" ,
          pszPropertyName , ErrorCodeToMessage( LastError ).c_str() ) ;
        return false ;
      }
      if ( _tcscmp( pszPropertyName , _T( "DeviceTemperature" ) ) == 0 )
      {
        if ( m_pOrigProperties == GoldEye )
        {
          if ( GetPropertyValue( m_pCamera , _T( "SensorCoolingPower" ) , *pData ) )
          {
            TCHAR SensCoolPower[ 30 ] ;
            _stprintf_s( SensCoolPower , ", %d mW" , (int) pData->m_int64 ) ;
            Value += SensCoolPower ;
          }
        }
      }
      strcpy_s( pData->m_szString , Value.c_str() ) ;
      return true ;
    }
    break ;
    case VmbFeatureDataBool:
    {
      m_LastError = PropPtr->GetValue( pData->m_bBool ) ;
      if ( m_LastError != VmbErrorSuccess )
      {
        SEND_DEVICE_ERR( "Can't get bool data for property %s: %s" ,
          pszPropertyName , ErrorCodeToMessage( LastError ).c_str() ) ;
        return false ;
      }
      return true ;
    }
  }
  }
  catch ( CException* e )
  {
    TCHAR Buf[ 1000 ] ;
    BOOL bMsgTaken = e->GetErrorMessage( Buf , 999 ) ;
    SEND_DEVICE_ERR( "AVT_Vimba::AVTGetCameraPropertyEx Exception for Property %s - %s" ,
      pszPropertyName , bMsgTaken ? Buf : _T( "Reason Unknown" ) ) ;
  }
  return false ;
}

bool AVT_Vimba::GetROI( CRect& rc )
{
  if ( IsRunning() )
  {
    rc = m_CurrentROI ;
    return true ;
  }
  int iWidth = GetXSize( m_pCamera ) ;
  int iHeight = GetYSize( m_pCamera ) ;
  if ( iWidth != 0 && iHeight != 0 )
  {
    int iXl = GetXOffset( m_pCamera ) ;
    int iYt = GetYOffset( m_pCamera ) ;

    rc.left = iXl ;
    rc.top = iYt ;
    rc.right = iWidth ;
    rc.bottom = iHeight ;
    return true ;
  }
  return false ;
}

void AVT_Vimba::SetROI( CRect& rc )
{
  if ( !m_pCamera )
    return ;

  if ( rc.left < 0 )
    rc.left = 0 ;
  if ( rc.top < 0 )
    rc.top = 0 ;
  if ( rc.Width() > m_SensorSize.cx )
  {
    rc.left = 0 ;
    rc.right = m_SensorSize.cx ;
  }
  else if ( rc.Width() + rc.left > m_SensorSize.cx )
    rc.left = m_SensorSize.cx - rc.Width() ;

  if ( rc.Height() > m_SensorSize.cy )
  {
    rc.top = 0 ;
    rc.bottom = m_SensorSize.cy ;
  }
  else if ( rc.Height() + rc.top > m_SensorSize.cy )
    rc.top = m_SensorSize.cy - rc.Height() ;

  SetXOffset( m_pCamera , rc.left ) ;
  SetYOffset( m_pCamera , rc.top ) ;
  SetWidth( m_pCamera , rc.Width() ) ;
  SetHeight( m_pCamera , rc.Height() ) ;

  m_CurrentROI = rc;

  return ;
}

bool AVT_Vimba::SetStrobe( const FXString& StrobeDataAsText , int iIndex )
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

  return false ;
}

void AVT_Vimba::GetStrobe( FXString& StrobeDataAsText , int iIndex )
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

void AVT_Vimba::GetCamResolutionAndPixelFormat(
  unsigned int* rows , unsigned int* cols , VmbPixelFormatType* pixelFmt )
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


unsigned int AVT_Vimba::GetBppFromPixelFormat( VmbPixelFormat_t pixelFormat )
{
  switch ( pixelFormat )
  {
    case VmbPixelFormatMono8:
    case VmbPixelFormatBayerGR8:
      return 8;
      break;
    case VmbPixelFormatYuv411:
    case VmbPixelFormatMono12:
    case VmbPixelFormatBayerGR12:
      return 12;
      break;
    case VmbPixelFormatMono16:
    case VmbPixelFormatYuv422:
    case VmbPixelFormatBayerGR16:
      return 16;
      break;
    case VmbPixelFormatYuv444:
    case VmbPixelFormatRgb8:
    case VmbPixelFormatBgr8:
      return 24;
    case VmbPixelFormatRgba8:
    case VmbPixelFormatBgra8:
      return 32;
    case VmbPixelFormatRgb16:
      return 48;
    default:
      return 0;
      break;
  }
}

void AVT_Vimba::LogError( LPCTSTR Msg )
{
  FXString GadgetName ;
  GetGadgetName( GadgetName ) ;
  FxSendLogMsg( MSG_ERROR_LEVEL , GetDriverInfo() , 0 ,
    _T( "%s - %s" ) , (LPCTSTR) GadgetName , Msg );
}

void AVT_Vimba::LocalStreamStart()
{
  CamCNTLDoAndWait( AVT_EVT_START_GRAB ) ;
}
void AVT_Vimba::LocalStreamStop()
{
  CamCNTLDoAndWait( AVT_EVT_STOP_GRAB ) ;
}


bool AVT_Vimba::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXAutolock al( m_SettingsLock , "ScanProperties" ) ;
  double dStart = GetHRTickCount() ;
  m_bInScanProperties = true ;
  m_bWasStopped = m_bShouldBeReprogrammed = false ;

  DriverInit() ;
  FXString tmpS;
  FXPropertyKit pc( text );
  unsigned camSN = 0 ;
  if ( pc.GetInt( "Camera" , (int&) camSN ) )
  {
    if ( camSN && (camSN != 0xffffffff) )
    {
      unsigned newCamnmb = SerialToNmb( camSN );;
      if ( newCamnmb < m_CamerasOnBus )
      {
        if ( (m_dwSerialNumber != camSN) || (newCamnmb != m_CurrentCamera) )
        {
          m_bWasStopped = IsRunning() ;
          if ( m_dwSerialNumber && (m_dwSerialNumber != 0xffffffff) )
          {
            //                 bool bRes =CamCNTLDoAndWait( AVT_EVT_RELEASE , 2000) ;
            m_dwSerialNumber = 0 ;
            m_dwConnectedSerialNumber = 0 ;
          }
          ASSERT( !m_pCamera ) ;
          m_dwSerialNumber = camSN ;
          m_CurrentCamera = newCamnmb;
          if ( m_dwSerialNumber )
          {
            OtherThreadCameraInit();
            m_bShouldBeReprogrammed = m_bWasStopped ;
          }
        }
        else
        {
          bool bCamConnected = CheckAndAllocCamera() ;
          if ( !bCamConnected )
          {
            m_bInScanProperties = false ;
            return false ;
          }
        }
      }
      else
      {
        m_dwSerialNumber = camSN ;
        m_CurrentCamera = -1 ;
      }
      Invalidate |= true; //update setup
    }
    else
    {
      if ( m_dwSerialNumber && (m_dwSerialNumber != 0xffffffff) )
      {
        //            bool bRes =CamCNTLDoAndWait( AVT_EVT_RELEASE , 2000) ;
        CameraStop() ;
        CameraClose() ;
        m_dwSerialNumber = 0 ;
        m_dwConnectedSerialNumber = 0 ;

        Invalidate = true ;
      }
      //       ASSERT( bRes ) ;
    }
  }
  if ( DriverValid() && !SP_ISNULL( m_pCamera ) )
  {
    //     if ( pc.GetInt( "StreamState" , m_bLocalStopped ) )
    //     {
    //       if ( m_bLocalStopped )
    //         OtherThreadLocalStop() ;
    //       else if ( m_bRun )
    //         OtherThreadLocalStart() ;
    //     };

    for ( int i = 0; i < m_PropertiesEx.GetCount() ; i++ )
    {
      CameraAttribute& Prop = m_PropertiesEx.GetAt( i ) ;
      FXString key , sValue ;
      FXParser param;
      Prop.m_DlgFormat.GetElementNo( 0 , key , param );
      if ( (key == SETUP_COMBOBOX) || (key == SETUP_SPIN) ) // ints result
      {
        FXString sValue ;
        FXSIZE value ; // what will be passed for property set
        bool bauto = false ;
        if ( pc.GetString( Prop.m_Name , sValue ) )
        {
          switch ( Prop.m_Type )
          {
            case VmbFeatureDataInt:
            {
              if ( sValue != _T( "auto" ) )
                value = atoi( (LPCTSTR) sValue ) ;
              else
              {
                bauto = true ;
                value = Prop.m_intVal ;
              }
            }
            break ;
            case VmbFeatureDataFloat:
            {
              if ( sValue != _T( "auto" ) )
                value = atoi( (LPCTSTR) sValue ) ;
              else
              {
                bauto = true ;
                value = ROUND( Prop.m_dVal ) ;
              }
            }
            break ;
            case VmbFeatureDataEnum:
            case VmbFeatureDataString:
            {
              value = atoi( (LPCTSTR) sValue ) ;
              if ( Prop.m_EnumRange.GetCount() == 0 || value < 0 )
                value = -1 ;
              if ( value >= Prop.m_EnumRange.GetCount() )
                value = 0 ;
            }
            break ;
            case VmbFeatureDataBool:
              value = atoi( (LPCTSTR) sValue ) != 0 ;
              break ;
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
          bool bauto = (sValue.CompareNoCase( "auto" ) == 0) ;
          if ( !bauto )
            value = atoi( sValue ) ;

          if ( !SetCameraProperty( i , value , bauto , Invalidate ) )
            //           if (!OtherThreadSetProperty( i , &Data , &Invalidate ))
          {
            SEND_DEVICE_ERR( "Can't set property %s" , (LPCTSTR) Prop.m_Name );
          }
        }
      }
      else if ( key == SETUP_EDITBOX )
      {
        FXSIZE value; bool bauto = false;
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
  if ( Invalidate )
  {
    m_dLastBuiltPropertyTime = 0. ;
    OtherThreadBuildPropertyList() ;
    m_LastPrintedProperties.Empty() ;
    m_LastPrintedSettings.Empty() ;
  }

  if ( m_bShouldBeReprogrammed )
  {
    if ( m_bWasStopped )
      OtherThreadCameraStart() ;
    else
      CamCNTLDoAndWait( AVT_EVT_INIT , 2000 ) ;
  }
  m_bInScanProperties = false ;
  FXString GadgetName ;
  GetGadgetName( GadgetName ) ;
  double dBusyTime = GetHRTickCount() - dStart ;
  TRACE( "\nAVT_Vimba::ScanProperties %s: Start %g , Busy %g" , (LPCTSTR) GadgetName ,
    dStart , dBusyTime ) ;
  return true;
}

void AVT_Vimba::OnBusArrival( void* pParam , LPCTSTR szSerNum )
{
  AVT_Vimba* pAVT_Vimba = static_cast<AVT_Vimba*>(pParam);
  if ( pAVT_Vimba->m_szSerialNumber == szSerNum )
  {
    pAVT_Vimba->m_BusEvents |= BUS_EVT_ARRIVED ;
    SetEvent( pAVT_Vimba->m_evCameraControl ) ;
    FxSendLogMsg( MSG_WARNING_LEVEL ,
      pAVT_Vimba->GetDriverInfo() , 0 , "Camera %u(%s) is connected" ,
      pAVT_Vimba->m_dwSerialNumber , szSerNum ) ;
    TRACE( "\nBus Arrival for Camera #%u(%s) " , pAVT_Vimba->m_dwSerialNumber , szSerNum ) ;
  }

  pAVT_Vimba->m_bRescanCameras = true ;
  pAVT_Vimba->m_dwNArrivedEvents++ ;
}

void AVT_Vimba::OnBusRemoval( void* pParam , LPCTSTR szSerNum )
{
  AVT_Vimba* pAVT_Vimba = static_cast<AVT_Vimba*>(pParam);
  if ( pAVT_Vimba->m_szSerialNumber == szSerNum )
  {
    pAVT_Vimba->m_BusEvents |= BUS_EVT_REMOVED ;
    SetEvent( pAVT_Vimba->m_evCameraControl ) ;
    FxSendLogMsg( MSG_WARNING_LEVEL ,
      pAVT_Vimba->GetDriverInfo() , 0 , "Used Camera %u(%s) is disconnected" ,
      pAVT_Vimba->m_dwSerialNumber , szSerNum ) ;
    TRACE( "\nCamera #%u(%s) Removed\n" , pAVT_Vimba->m_dwSerialNumber , szSerNum ) ;
  }
}


void AVT_Vimba::OnBusReset( void* pParam , LPCTSTR szSerNum )
{
  AVT_Vimba* pAVT_Vimba = static_cast<AVT_Vimba*>(pParam);
  if ( pAVT_Vimba->m_szSerialNumber == szSerNum )
  {
    //     pAVT_Vimba->m_BusEvents |= BUS_EVT_RESET ;
    //     SetEvent( pAVT_Vimba->m_evCameraControl ) ;
    FxSendLogMsg( MSG_WARNING_LEVEL ,
      pAVT_Vimba->GetDriverInfo() , 0 , "Bus Reset for Camera %u(%s)" ,
      pAVT_Vimba->m_dwSerialNumber , szSerNum ) ;
    TRACE( "\nBus Reset for Camera #%u(%s) \n" , pAVT_Vimba->m_dwSerialNumber , szSerNum ) ;
  }
  pAVT_Vimba->m_bCamerasEnumerated = false ;
  //   bool bRes = pAVT_Vimba->CamCNTLDoAndWait( BUS_EVT_BUS_RESET | AVT_EVT_INIT , 2000)  ;
  pAVT_Vimba->m_bRescanCameras = true ;
  //   ASSERT( bRes ) ;
}



bool AVT_Vimba::CheckAndAllocCamera( void )
{
  if ( !m_pCamera )
  {
    if ( !m_dwSerialNumber || m_dwSerialNumber == (-1) )
      return false ;
    if ( !OtherThreadCameraInit() )
      return false ;
  }
  return true ;
}


bool AVT_Vimba::SetBMIH( void )
{
  m_BMIH.biSize = sizeof( BITMAPINFOHEADER );
  m_BMIH.biWidth = m_CurrentROI.Width() ;
  if ( m_BMIH.biWidth == -1 || m_BMIH.biWidth == 0 )
    m_BMIH.biWidth = (LONG) m_nSensorWidth ;
  m_BMIH.biHeight = m_CurrentROI.Height();
  if ( m_BMIH.biHeight == -1 || m_BMIH.biHeight == 0 )
    m_BMIH.biHeight = (LONG) m_nSensorHeight ;

  m_BMIH.biPlanes = 1;
  switch ( m_nPixelFormat )
  {
    case VmbPixelFormatBayerRG8:
    case VmbPixelFormatBayerGB8:
    case VmbPixelFormatBayerBG8:
    case VmbPixelFormatBayerGR8:
    case VmbPixelFormatMono8:
      m_BMIH.biCompression = BI_Y8;
      m_BMIH.biBitCount = 8;
      m_BMIH.biSizeImage = m_BMIH.biWidth*m_BMIH.biHeight;
      break;
    case VmbPixelFormatYuv411:
      m_BMIH.biCompression = BI_YUV9 ;
      m_BMIH.biBitCount = 12;
      m_BMIH.biSizeImage = 9 * m_BMIH.biWidth*m_BMIH.biHeight / 8;
      break;
    case VmbPixelFormatMono10:
    case VmbPixelFormatMono10p:
    case VmbPixelFormatMono12:
    case VmbPixelFormatMono12p:
    case VmbPixelFormatMono12Packed:
    case VmbPixelFormatMono14:
    case VmbPixelFormatMono16:
      m_BMIH.biCompression = BI_Y16;
      m_BMIH.biBitCount = 16;
      m_BMIH.biSizeImage = 2 * m_BMIH.biWidth*m_BMIH.biHeight;
      break;
    case VmbPixelFormatRgb8:
    case VmbPixelFormatBgr8:
    case VmbPixelFormatArgb8:   // the same  case VmbPixelFormatRgba8 :
    case VmbPixelFormatBgra8:
      m_BMIH.biCompression = 0 ;
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
    case VmbPixelFormatBayerGR16:
    case VmbPixelFormatBayerRG16:
    case VmbPixelFormatBayerGB16:
    case VmbPixelFormatBayerBG16:
    case VmbPixelFormatBayerGR12Packed:
    case VmbPixelFormatBayerRG12Packed:
    case VmbPixelFormatBayerGB12Packed:
    case VmbPixelFormatBayerBG12Packed:
      m_BMIH.biCompression = BI_RGB48 ;
      m_BMIH.biBitCount = 48;
      m_BMIH.biSizeImage = 6 * m_BMIH.biWidth*m_BMIH.biHeight;
      break ;

    default:
      m_BMIH.biSizeImage = 0;
      TRACE( "!!! Unsupported format #%d\n" , m_pixelFormat );
      SEND_DEVICE_ERR( "!!! Unsupported format #%d" , m_pixelFormat );
      return false;
  }
  return true ;
}


bool AVT_Vimba::ScanSettings( FXString& text )
{
  double dBegin = GetHRTickCount() ;
  FXAutolock al( m_SettingsLock , "ScanSettings" ) ;
  if ( !m_LastPrintedSettings.IsEmpty()
    && (dBegin - m_dLastSettingsPrintTime < 100.0) )
  {
    text = m_LastPrintedSettings ;
    m_dLastSettingsPrintTime = dBegin ;
    return true ;
  }
  m_dLastBuiltPropertyTime = 0 ;
  DriverInit() ;
  CheckAndAllocCamera() ;
  // Prepare cameras list
  FXString camlist( "Not Selected(-1)," ) , paramlist , tmpS;
  int iCurrentCamera = -1 ;
  for ( unsigned i = 0; i < m_CamerasOnBus; i++ )
  {
    TCHAR cMark = _T( '+' ) ; // sign, that camera is free
    if ( m_dwSerialNumber != m_CamInfo[ i ].serialnmb )
    {
      FXAutolock al( m_ConfigLock ) ;
      for ( int j = 0 ; j < m_BusyCameras.GetCount() ; j++ )
      {
        if ( m_dwSerialNumber != m_BusyCameras[ j ].m_dwSerialNumber )
        {
          if ( m_CamInfo[ i ].serialnmb == m_BusyCameras[ j ].m_dwSerialNumber )
          {
            cMark = _T( '-' ) ; // sign, that camera is busy by other gadget
            break ;
          }
        }
      }
    }
    else
    {
      cMark = _T( '!' ) ;// this gadget camera sign
      iCurrentCamera = m_CurrentCamera = i ;
    }
    FXString Model = m_CamInfo[ i ].model ;
    Model.Replace( _T( '(' ) , _T( '[' ) ) ;
    Model.Replace( _T( ')' ) , _T( ']' ) ) ;
    tmpS.Format( "%c%d:%s(%d)" , cMark , m_CamInfo[ i ].serialnmb ,
      (LPCTSTR) Model , m_CamInfo[ i ].serialnmb );
    camlist += tmpS;
    if ( i < m_CamerasOnBus - 1 )
      camlist += _T( ',' ) ;
  }
  if ( iCurrentCamera < 0 && m_dwSerialNumber && (m_dwSerialNumber != 0xffffffff) )
  {
    if ( !camlist.IsEmpty() )
      camlist += _T( ',' ) ;
    tmpS.Format( "?%d(%d)" , m_dwSerialNumber , m_dwSerialNumber ) ;
    camlist += tmpS ;
    iCurrentCamera = m_CamerasOnBus ; // first after real cameras
  }
  tmpS.Format( "ComboBox(Camera(%s))" , camlist );
  paramlist += tmpS;
  if ( m_dwSerialNumber == m_dwConnectedSerialNumber
    && (m_dwSerialNumber != 0) && (m_dwSerialNumber != -1) )
  {
    paramlist += _T( ',' ) ;
    OtherThreadBuildPropertyList() ;
    tmpS.Format( "ComboBox(StreamState(Run(0),Idle(1)))" );
    paramlist += tmpS;
    for ( int i = 0; i < m_PropertiesEx.GetCount() ; i++ )
    {
      paramlist += _T( ',' ) ;
      paramlist += m_PropertiesEx[ i ].m_DlgFormat ;
    }
  }
  text.Format( "template(%s)" , paramlist );
  m_LastPrintedSettings = text ;
  m_dLastSettingsPrintTime = GetHRTickCount() ;
  double dDutyTime = GetHRTickCount() - dBegin ;
  TRACE( "\n----------Settings Printed in %g ms" , dDutyTime ) ;
  return true;
}

bool AVT_Vimba::PrintProperties( FXString& text )
{
  double dBegin = GetHRTickCount() ;
  FXAutolock al( m_SettingsLock , "PrintProperties" ) ;
  if ( !m_LastPrintedProperties.IsEmpty()
    && (dBegin - m_dLastProperiesPrintTime < 100.0) )
  {
    text = m_LastPrintedProperties ;
    m_dLastProperiesPrintTime = dBegin ;
    return true ;
  }
  FXPropertyKit pc;
  if ( DriverValid() && (m_dwSerialNumber != 0)
    && (m_dwSerialNumber != -1) )
  {
    pc.WriteInt( "Camera" , m_dwSerialNumber );
    pc.WriteInt( "StreamState" , m_bLocalStopped ) ;
    if ( m_dwConnectedSerialNumber != 0 )
    {
      for ( int i = 0; i < m_PropertiesEx.GetCount() ; i++ )
      {
        FXSIZE value; 
        bool bauto;
        if ( GetCameraProperty( i , value , bauto ) )
        {
          FXString key;
          FXParser param;
          m_PropertiesEx[ i ].m_DlgFormat.GetElementNo( 0 , key , param );
          if ( (key == SETUP_COMBOBOX) || (key == SETUP_SPIN) ) // ints result
          {
            pc.WriteInt( m_PropertiesEx[ i ].m_Name , (int)value );
          }
          else if ( key == SETUP_SPINABOOL )
          {
            FXString tmpS;
            if ( bauto )
              tmpS = "auto";
            else
              tmpS.Format( "%d" , value );
            pc.WriteString( m_PropertiesEx[ i ].m_Name , tmpS );
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
  CGadget::PrintProperties( pc ) ;
  m_LastPrintedProperties = text = pc;
  m_dLastProperiesPrintTime = GetHRTickCount() ;
  double dDutyTime = GetHRTickCount() - dBegin ;
  TRACE( "\n----------Properties Printed in %g ms" , dDutyTime ) ;
  return true;
}


int AVT_Vimba::SetPacketSize( int iPacketSize_Or_FPSx10 , bool bFrameRate )
{
  if ( !m_pCamera )
    return 0 ;

  return -1 ;
}
void AVT_Vimba::SendValueToLog()
{
  FXString Addition ;

  m_LogLock.Lock() ;
  if ( m_iLogCnt >= 0 && m_PropertiesForLog.GetCount() )
  {
    SetCamPropertyData Value ;
    if ( GetPropertyValue( m_pCamera , m_PropertiesForLog[ m_iLogCnt ] , Value ) )
    {
      if ( m_iLogCnt == 0 )
      {
        double dT = GetHRTickCount() ;
        Addition.Format( " %8d.%3d" , (int) (dT / 1000.) , (int) fmod( dT , 1000. ) ) ;
        m_LogOutString += Addition ;
      }
      switch ( Value.m_Type )
      {
        case VmbFeatureDataInt:
          Addition.Format( " %d" , (int) Value.m_int64 ) ;
          break ;
        case VmbFeatureDataFloat:
          Addition.Format( " %g" , Value.m_double ) ;
          break ;
        case VmbFeatureDataEnum:
        case VmbFeatureDataString:
          Addition.Format( " %s" , Value.m_szString ) ;
          break ;
        case VmbFeatureDataBool:
          Addition.Format( " %s" , Value.m_bBool ? "true" : "false" ) ;
          break ;
      }
      if ( Addition.IsEmpty() )
        Addition.Format( " Can't show property %s" , (LPCTSTR) m_PropertiesForLog[ m_iLogCnt ] ) ;
      m_LogOutString += Addition ;
    }
  }

  if ( m_PropertiesForTemp.GetCount() )
  {
    FXString TemperatureView ;
    for ( int i = 0 ; i < m_PropertiesForTemp.GetCount() ; i++ )
    {
      SetCamPropertyData Value ;
      if ( GetPropertyValue( m_pCamera , m_PropertiesForTemp[ m_iLogCnt ] , Value ) )
      {
        switch ( Value.m_Type )
        {
          case VmbFeatureDataInt:
            Addition.Format( " %d" , (int) Value.m_int64 ) ;
            break ;
          case VmbFeatureDataFloat:
            Addition.Format( " %g" , Value.m_double ) ;
            break ;
          case VmbFeatureDataEnum:
          case VmbFeatureDataString:
            Addition.Format( " %s" , Value.m_szString ) ;
            break ;
          case VmbFeatureDataBool:
            Addition.Format( " %s" , Value.m_bBool ? "true" : "false" ) ;
            break ;
        }
        if ( Addition.IsEmpty() )
          Addition.Format( " Can't show property %s" , (LPCTSTR) m_PropertiesForLog[ m_iLogCnt ] ) ;
        TemperatureView += Addition ;
      }
    }
    if ( !TemperatureView.IsEmpty() )
      m_PropertiesForTempAsString = TemperatureView ;
  }
  if ( ++m_iLogCnt >= m_PropertiesForLog.GetCount() )
  {
    m_LogOutString += "\n" ;
    m_iLogCnt = 0 ;
    CTextFrame * pOut = CTextFrame::Create( m_LogOutString ) ;
    pOut->SetLabel( "Log" ) ;
    pOut->SetTime( m_dLastLogTime_ms = GetHRTickCount() ) ;
    PutFrame( m_pLogOutput , pOut ) ;
    m_LogOutString.Empty() ;
    m_BusEvents &= ~AVT_EVT_LOG ;
  };

  m_LogLock.Unlock() ;
}


DWORD WINAPI AVT_Vimba::CameraControlLoop( LPVOID pParam )
{
  TRACE( "---------Entry to CameraControlLoop\n" ) ;
  AVT_Vimba * pAVT_Vimba = (AVT_Vimba*) pParam ;
  VmbErrorType VError;
  BOOL bLocked = pAVT_Vimba->m_ConfigLock.Lock( 5000 , "CameraControlLoop" ) ;
  ASSERT( bLocked ) ;
  if ( pAVT_Vimba->m_dwInstanceCount++ == 0 )
  {
    VError = pAVT_Vimba->m_System.Startup() ;
    if ( VError != VmbErrorSuccess )
    {
      pAVT_Vimba->m_dwInstanceCount = 0;
      FxSendLogMsg( MSG_CRITICAL_LEVEL , "Vimba" , 0 , "Can't startup Vimba system." );
      pAVT_Vimba->m_ConfigLock.Unlock() ;
      return -1;
    }
  }
  pAVT_Vimba->m_ConfigLock.Unlock() ;
  //pAVT_Vimba->m_pCameraObserver = new CameraObserver() ;
  //pAVT_Vimba->m_pCameraObserver->SetPHost( pAVT_Vimba ) ;
  //VError = pAVT_Vimba->m_System.RegisterCameraListObserver( 
  //  AVT::VmbAPI::ICameraListObserverPtr( pAVT_Vimba->m_pCameraObserver ) ) ;
  //ASSERT( VError == VmbErrorSuccess );

  double dLastTemperatureMeasurementTime = GetHRTickCount() ;
  int iTemperatureMeasurementIndex = 0 ;

  FXString csMessage;
  BOOL isCorruptFrame = FALSE;
  unsigned int cols = 0 , rows = 0 , colsPrev = 0 , rowsPrev = 0;
  DWORD dwWaitRes = 0 ;
  pAVT_Vimba->m_BusEvents |= AVT_EVT_DRIVER_INIT ;
  SetEvent( pAVT_Vimba->m_evCameraControl ) ;

  DWORD dwWaitTimeout = 5000 ;
  int iNLogRequests = (int) pAVT_Vimba->m_PropertiesForLog.GetCount() ;
  if ( ROUND( pAVT_Vimba->m_dLogPeriod_ms ) >= 100 )
  {
    dwWaitTimeout = ROUND( pAVT_Vimba->m_dLogPeriod_ms / iNLogRequests ) ;
  }

  while ( pAVT_Vimba->m_bContinueCameraControl )
  {
    dwWaitRes = WaitForMultipleObjects( ARR_SIZE( pAVT_Vimba->m_WaitEventBusChangeArr ) ,
      pAVT_Vimba->m_WaitEventBusChangeArr , FALSE ,
      (!(pAVT_Vimba->m_bInitialized)) ? 500 : dwWaitTimeout ) ;
    if ( dwWaitRes == WAIT_FAILED )  // gadget deleted
    {
      DWORD dwError = GetLastError() ;
      break ;
    }
    if ( dwWaitRes == WAIT_OBJECT_0 ) // some control commands for camera (m_evCameraControl event)
    {
      DWORD InitialBusEvents = pAVT_Vimba->m_BusEvents ;
      int iNInits = 0 ;
      while ( pAVT_Vimba->m_bContinueCameraControl && pAVT_Vimba->m_BusEvents )
      {
        FXAutolock al( pAVT_Vimba->m_LocalConfigLock , "CameraControlLoop1" ) ;
        if ( !pAVT_Vimba->m_dwSerialNumber || (pAVT_Vimba->m_dwSerialNumber == 0xffffffff) )
        {
          if ( !(pAVT_Vimba->m_BusEvents & (AVT_EVT_SHUTDOWN | AVT_EVT_DRIVER_INIT)) )
            break ;
        }
        if ( pAVT_Vimba->m_BusEvents & AVT_EVT_SHUTDOWN )
        {
          if ( !SP_ISNULL( pAVT_Vimba->m_pCamera ) )
          {
            pAVT_Vimba->AVTCameraStop() ;
            pAVT_Vimba->AVTCameraClose() ;
            //pAVT_Vimba->m_pCamera = NULL ;
          }
          pAVT_Vimba->m_BusEvents = 0 ;
          pAVT_Vimba->m_bContinueCameraControl = false ;
          TRACE( "\nCamera #%u Shut downed" , pAVT_Vimba->m_dwSerialNumber ) ;
          Sleep( 100 ) ;
          break ;
        }
        if ( pAVT_Vimba->m_BusEvents & AVT_EVT_DRIVER_INIT )
        {
          pAVT_Vimba->m_BusEvents &= ~(AVT_EVT_DRIVER_INIT) ;
          bool bRes = pAVT_Vimba->AVTDriverInit() ;
          TRACE( "\nDriver %s intialized in CameraControlLoop" , bRes ? "" : "NOT" ) ;
          //           break ;
        }
        if ( pAVT_Vimba->m_BusEvents & AVT_EVT_RESTART )
        {
          pAVT_Vimba->m_BusEvents &= ~(AVT_EVT_RESTART) ;
          pAVT_Vimba->AVTCameraClose() ;
          TRACE( "\nCamera #%u closed in CameraControlLoop" , pAVT_Vimba->m_dwSerialNumber ) ;
          pAVT_Vimba->m_BusEvents |= AVT_EVT_INIT ;
          //           break ;
        }
        if ( pAVT_Vimba->m_BusEvents & (BUS_EVT_REMOVED | BUS_EVT_BUS_RESET) )
        {
          pAVT_Vimba->m_pCamera->Close() ;
          if ( pAVT_Vimba->m_BusEvents & BUS_EVT_BUS_RESET )
          {
            pAVT_Vimba->m_BusEvents |= AVT_EVT_INIT ;
            TRACE( "\nCamera #%u Bus Reset" , pAVT_Vimba->m_dwSerialNumber ) ;
          }
          if ( pAVT_Vimba->m_BusEvents & BUS_EVT_REMOVED )
          {
            TRACE( "\nCamera #%u Removed and closed" , pAVT_Vimba->m_dwSerialNumber ) ;
          }

          pAVT_Vimba->m_BusEvents &= ~((BUS_EVT_REMOVED | BUS_EVT_BUS_RESET)) ;
        }
        if ( (pAVT_Vimba->m_BusEvents & AVT_EVT_START_GRAB)
          && !pAVT_Vimba->m_bInitialized )
        {
          pAVT_Vimba->m_BusEvents |= AVT_EVT_INIT ;
        }
        if ( pAVT_Vimba->m_BusEvents & (AVT_EVT_INIT /*| AVT_EVT_BUILD_PROP*/) )
        {
          if ( pAVT_Vimba->m_dwSerialNumber &&  pAVT_Vimba->m_dwSerialNumber != (-1) )
          {
            pAVT_Vimba->AVTCameraInit() ;
            if ( pAVT_Vimba->m_bInitialized  &&  pAVT_Vimba->m_bRun )
              pAVT_Vimba->m_BusEvents |= AVT_EVT_START_GRAB ;
          }
          pAVT_Vimba->m_BusEvents &= ~(AVT_EVT_INIT/* | AVT_EVT_BUILD_PROP*/) ;
        }
        if ( pAVT_Vimba->m_BusEvents & AVT_EVT_BUILD_PROP )
        {
          if ( pAVT_Vimba->m_dwSerialNumber )
          {
            pAVT_Vimba->AVTBuildPropertyList() ;
            //             if ( pAVT_Vimba->m_bInitialized  &&  pAVT_Vimba->m_bRun )
            //               pAVT_Vimba->m_BusEvents |= AVT_EVT_START_GRAB ;
          }
          pAVT_Vimba->m_BusEvents &= ~(AVT_EVT_BUILD_PROP) ;
        }
        if ( pAVT_Vimba->m_BusEvents & AVT_EVT_START_GRAB )
        {
          if ( pAVT_Vimba->m_dwSerialNumber )
          {
            if ( SP_ISNULL( pAVT_Vimba->m_pCamera ) )
            {
              if ( !pAVT_Vimba->AVTCameraInit() )
                break ; // will be reinitialized above
            }
            if ( !pAVT_Vimba->AVTCameraStart() )
            {
              //              FXString Msg ;
              //               Msg.Format("Start Failure: %s", error.GetDescription());
              //               pAVT_Vimba->LogError( Msg ) ;
              pAVT_Vimba->AVTCameraClose() ;
              if ( !pAVT_Vimba->AVTCameraInit() )
              {
                pAVT_Vimba->m_BusEvents &= ~(AVT_EVT_START_GRAB) ;
                break ; // will not be started on next loop above
              }
            }
            else
            {
              TRACE( "\n%f AVT_Vimba::CameraControlLoop - Camera %u started" ,
                GetHRTickCount() , pAVT_Vimba->m_dwSerialNumber ) ;
              pAVT_Vimba->m_BusEvents &= ~(AVT_EVT_START_GRAB) ;
            }
          }
        }
        if ( pAVT_Vimba->m_BusEvents & AVT_EVT_STOP_GRAB )
        {
          pAVT_Vimba->AVTCameraStop() ;
          pAVT_Vimba->m_BusEvents &= ~(AVT_EVT_STOP_GRAB) ;
        }
        if ( pAVT_Vimba->m_BusEvents & AVT_EVT_RELEASE )
        {
          pAVT_Vimba->AVTCameraStop() ;
          pAVT_Vimba->AVTCameraClose() ;
          pAVT_Vimba->m_CurrentCamera = -1 ;
          pAVT_Vimba->m_BusEvents &= ~(AVT_EVT_RELEASE) ;
          //           break ;
        }
        if ( pAVT_Vimba->m_BusEvents & AVT_EVT_LOCAL_STOP )
        {
          pAVT_Vimba->AVTCameraStop() ;
          pAVT_Vimba->m_bLocalStopped = true ;

          pAVT_Vimba->m_BusEvents &= ~(AVT_EVT_LOCAL_STOP) ;
          //          break ;
        }
        if ( pAVT_Vimba->m_BusEvents & AVT_EVT_LOCAL_START )
        {
          pAVT_Vimba->AVTCameraStart() ;
          pAVT_Vimba->m_bLocalStopped = false ;
          pAVT_Vimba->m_BusEvents &= ~(AVT_EVT_LOCAL_START) ;
          //          break ;
        }
        if ( pAVT_Vimba->m_BusEvents & BUS_EVT_ARRIVED )
        {
          pAVT_Vimba->m_BusEvents |= AVT_EVT_INIT ;
          pAVT_Vimba->m_BusEvents &= ~(BUS_EVT_ARRIVED) ;
        }
        if ( pAVT_Vimba->m_BusEvents & AVT_EVT_SET_PROP )
        {
          double dSetStart = GetHRTickCount() ;
          //           TRACE("\n%f SetPropertyEx called" , GetHRTickCount() ) ;
          bool bRes = pAVT_Vimba->SetCameraPropertyEx( pAVT_Vimba->m_iPropertyIndex ,
            &pAVT_Vimba->m_PropertyData , pAVT_Vimba->m_bInvalidate ) ;
          pAVT_Vimba->m_BusEvents &= ~(AVT_EVT_SET_PROP) ;
          TRACE( "\n   %g SetPropertyEx %s for %s" , GetHRTickCount() - dSetStart ,
            (bRes) ? "OK" : "FAULT" ,
            pAVT_Vimba->m_PropertiesEx[ pAVT_Vimba->m_iPropertyIndex ].m_CameraPropertyName ) ;
        }
        if ( pAVT_Vimba->m_BusEvents & AVT_EVT_GET_PROP )
        {
          pAVT_Vimba->GetCameraPropertyEx( pAVT_Vimba->m_iPropertyIndex ,
            &pAVT_Vimba->m_PropertyData ) ;
          pAVT_Vimba->m_BusEvents &= ~(AVT_EVT_GET_PROP) ;
        }
        if ( pAVT_Vimba->m_BusEvents & AVT_EVT_LOG )
        {
          if ( pAVT_Vimba->IsRunning() )
          {
            pAVT_Vimba->SendValueToLog() ;
            if ( pAVT_Vimba->m_iLogCnt == 0 )
            {
              int iNLogRequests = (int) pAVT_Vimba->m_PropertiesForLog.GetCount() ;
              if ( ROUND( pAVT_Vimba->m_dLogPeriod_ms ) >= 100 )
              {
                double dNextLogTime = pAVT_Vimba->m_dLastLogTime_ms + pAVT_Vimba->m_dLogPeriod_ms ;
                double dTimeToNextLog = dNextLogTime - GetHRTickCount() - 10. ;
                dwWaitTimeout = ROUND( dTimeToNextLog > 0. ? dTimeToNextLog : 1. ) ;
              }
              else
                dwWaitTimeout = 5000 ;
            }
          }
          else
            pAVT_Vimba->m_BusEvents &= ~AVT_EVT_LOG ;
        }
        if ( pAVT_Vimba->m_BusEvents & ~AVT_EVT_LOG )
        {
          if ( InitialBusEvents )
            SetEvent( pAVT_Vimba->m_evControlRequestFinished ) ;
        }
        if ( pAVT_Vimba->m_BusEvents & AVT_EVT_SET_SOFT_TRIGGER )
        {
          pAVT_Vimba->AVTSetSoftwareTriggerMode( pAVT_Vimba->m_PropertyData.m_bBool ) ;
          pAVT_Vimba->m_BusEvents &= ~AVT_EVT_SET_SOFT_TRIGGER ;
        }
      }

      if ( InitialBusEvents )
        SetEvent( pAVT_Vimba->m_evControlRequestFinished ) ;
    }
    else if ( dwWaitRes == WAIT_OBJECT_0 + 2 ) // Bus changes (camera inserted/removed)
    {

    }
    else if ( dwWaitRes == WAIT_OBJECT_0 + 3 )
    {
      AVT::VmbAPI::FeaturePtr pCommandFeature;
      if ( pAVT_Vimba->m_pCamera &&
        VmbErrorSuccess == pAVT_Vimba->m_pCamera->GetFeatureByName(
        "TriggerSoftware" , pCommandFeature ) )
      {
        if ( VmbErrorSuccess == pCommandFeature->RunCommand() )
        {
        }
        else
        {
        }
      }
    }
    if ( pAVT_Vimba->m_bRun  && pAVT_Vimba->m_bContinueCameraControl )
    {
    }
    if ( pAVT_Vimba->m_bContinueCameraControl &&
      pAVT_Vimba->m_bInitialized  &&  pAVT_Vimba->m_pCamera
      && pAVT_Vimba->m_dLogPeriod_ms > 0. && pAVT_Vimba->IsRunning() )
    {
      if ( GetHRTickCount() - pAVT_Vimba->m_dLastLogTime_ms > pAVT_Vimba->m_dLogPeriod_ms )
      {
        pAVT_Vimba->m_BusEvents |= AVT_EVT_LOG ;
        SetEvent( pAVT_Vimba->m_evCameraControl ) ;
      }
      else
      {
        double dNextLogTime = pAVT_Vimba->m_dLastLogTime_ms + pAVT_Vimba->m_dLogPeriod_ms ;
        double dTimeToNextLog = dNextLogTime - GetHRTickCount() - 10. ;
        dwWaitTimeout = ROUND( dTimeToNextLog > 0. ? dTimeToNextLog : 1. ) ;
      }
    }
  }

  bLocked = pAVT_Vimba->m_ConfigLock.Lock( 5000 , "CameraControlLoop" ) ;
  //ASSERT( bLocked ) ;
  ////pAVT_Vimba->m_pCameraObserver->SetPHost( NULL ) ;
  //pAVT_Vimba->m_System.UnregisterCameraListObserver(
  //  AVT::VmbAPI::ICameraListObserverPtr( pAVT_Vimba->m_pCameraObserver ) ) ;
  //pAVT_Vimba->m_pCameraObserver = NULL ;
  if ( --pAVT_Vimba->m_dwInstanceCount == 0 )
    pAVT_Vimba->m_System.Shutdown() ;
  pAVT_Vimba->m_ConfigLock.Unlock() ;

  TRACE( "---AVT_Vimba Normal Exit from Camera Control Thread for #%u Thread=0x%xh\n" ,
    pAVT_Vimba->m_dwSerialNumber , GetCurrentThread() ) ;
  return 0;
}

CVideoFrame * AVT_Vimba::ConvertVimbaFrame(
  const AVT::VmbAPI::FramePtr pFrame , UINT OutputFormat )
{
  if ( !pFrame )
    return NULL ;
  VmbImage Src , Dest ;
  Src.Size = Dest.Size = sizeof( Dest ) ;
  VmbErrorType Result = pFrame->GetBuffer( (VmbUchar_t *&) Src.Data ) ;
  if ( Result != VmbErrorSuccess )
  {
    SEND_DEVICE_ERR( "AVT_Vimba::ConvertVimbaFrame - Can't get Data Buffer: %s " ,
      AVT::VmbAPI::Examples::ErrorCodeToMessage( Result ) ) ;
    return NULL ;
  }
  VmbPixelFormatType PixFormat ;
  VmbErrorType Res = pFrame->GetPixelFormat( PixFormat ) ;
  VmbUint32_t Lx , Ly ;
  if ( pFrame->GetWidth( Lx ) == VmbErrorSuccess
    && pFrame->GetHeight( Ly ) == VmbErrorSuccess )
  {
    unsigned iSize = Lx * Ly ;
    //LPBYTE data ;
    VmbError_t Res = VmbSetImageInfoFromPixelFormat( PixFormat , Lx , Ly , &Src ) ;
    if ( Res != VmbErrorSuccess )
    {
      SEND_DEVICE_ERR( "AVT_Vimba::ConvertVimbaFrame - Can't set SRC format to 0x%08X: %s " ,
        PixFormat , AVT::VmbAPI::Examples::ErrorCodeToMessage( Res ) ) ;
      return NULL ;
    }
    VmbPixelFormatType OutFormat = VmbPixelFormatRgb8 ;
    iSize *= 3 ; // 8 bits per color
    switch ( PixFormat )
    {
      case VmbPixelFormatBayerGR16:
      case VmbPixelFormatBayerRG16:
      case VmbPixelFormatBayerGB16:
      case VmbPixelFormatBayerBG16:
        OutFormat = VmbPixelFormatRgb16 ;
        iSize *= 2 ; // 16 bits per color
        break ;
    }
    Res = VmbSetImageInfoFromPixelFormat( OutFormat , Lx , Ly , &Dest ) ;
    if ( Res != VmbErrorSuccess )
    {
      SEND_DEVICE_ERR( "AVT_Vimba::ConvertVimbaFrame - Can't set DEST pixel to RGB: %s " ,
        AVT::VmbAPI::Examples::ErrorCodeToMessage( Res ) ) ;
      return NULL ;
    }
    VmbTransformInfo info;
    // set the debayering algorithm to simple 2 by 2
    Res = VmbSetDebayerMode( VmbDebayerMode2x2 , &info );
    if ( Res != VmbErrorSuccess )
    {
      SEND_DEVICE_ERR( "AVT_Vimba::ConvertVimbaFrame - Can't set debayer mode %d : %s " , VmbDebayerMode2x2 ,
        AVT::VmbAPI::Examples::ErrorCodeToMessage( Res ) ) ;
      return NULL ;
    }
    Dest.Data = malloc( iSize ) ;
    CVideoFrame * pOut = NULL ;
    // perform the transformation
    Res = VmbImageTransform( &Src , &Dest , &info , 1 );
    if ( Res != VmbErrorSuccess )
    {
      SEND_DEVICE_ERR( "AVT_Vimba::ConvertVimbaFrame - Can't do image transformation: %s " ,
        AVT::VmbAPI::Examples::ErrorCodeToMessage( Res ) ) ;
    }
    else
    {
      BITMAPINFOHEADER BMIH ;
      memset( &BMIH , 0 , sizeof( BMIH ) ) ;
      LPBYTE pInv = (LPBYTE) Dest.Data + iSize ;
      int iCopySize = Lx * 3 ;

      BMIH.biWidth = Lx ;
      BMIH.biHeight = Ly ;
      BMIH.biSizeImage = iSize;
      BMIH.biPlanes = 1;
      if ( OutFormat == VmbPixelFormatRgb16 )
      {
        iCopySize *= 2 ;
        BMIH.biCompression = BI_RGB48 ;
        BMIH.biBitCount = 48 ;
      }
      else
      {
        BMIH.biCompression = BI_RGB ;
        BMIH.biBitCount = 24 ;
      }
      for ( unsigned i = 0 ; i < Ly ; i++ )
        memcpy( pInv + i * iCopySize , (LPBYTE) Dest.Data + (Ly - i - 1) * iCopySize , iCopySize ) ;

      //       LPBITMAPINFOHEADER lpYUV9 = rgb24yuv9( lpBMIH , pInv ) ;
      //       if ( lpYUV9 )
      //       {
      //         CVideoFrame* vf=CVideoFrame::Create();
      //         vf->lpBMIH = lpYUV9 ;
      //         vf->lpData = NULL ;
      //         vf->SetLabel(pGadget->m_CameraID);
      //         vf->SetTime( GetHRTickCount() ) ;
      //         pOut = vf ;
      //       }
    }
    free( Dest.Data ) ;
    return pOut ;
  }
  return NULL;
}
bool AVT_Vimba::SetSoftwareTriggerMode( bool bSet )
{
  return OtherThreadSetSoftwareTrigger( bSet ) ;
}

bool AVT_Vimba::AVTSetSoftwareTriggerMode( bool bSet )
{
  SetCamPropertyData Data ;
  bool bInvalidate = false ;
  if ( bSet ) //switch to software trigger mode
  {
    if ( AVTGetCameraPropertyEx( _T( "TriggerMode" ) , &Data ) )
    {
      if ( _tcscmp( Data.m_szString , _T( "On" ) ) == 0 ) // trigger was on
      {
        if ( AVTGetCameraPropertyEx( _T( "TriggerSource" ) , &Data ) )
        {
          m_bSoftwareTriggerMode = true ;
          if ( _tcscmp( Data.m_szString , _T( "Software" ) ) == 0 ) // was software trigger
          {
            m_bHardwareTrigger = false ;
            m_bVimbaSoftTrigger = true;
          }
          else // Hardware trigger,  system will work by hardware
          {
            m_bHardwareTrigger = true; // trigger and will pass one frame per
                                        // frame on gadget input pin
            m_bVimbaSoftTrigger = false;
          }
          return true ;
        }
        return false ; // can't read trigger source
      }
      else // trigger was off; it's simplest case
      {
        _tcscpy( Data.m_szString , _T( "On" ) ) ;
        if ( AVTSetCameraPropertyEx( _T( "TriggerMode" ) , &Data ) )
        {
          _tcscpy( Data.m_szString , _T( "SoftwareTrigger" ) ) ;

          bool bRes = AVTSetCameraPropertyEx( _T( "TriggerSource" ) , &Data ) ;
          if ( bRes )
          {
            m_bSoftwareTriggerMode = true ;
            m_bHardwareTrigger = false ;
            return true ;
          }
        }
        return false ;
      }
    }
  }
  else        // remove software trigger mode
  {

  }

  return true ;
}

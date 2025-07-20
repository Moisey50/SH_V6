// Pleora.cpp: implementation of the Pleora class for Pleora controlled cameras management.
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
#include "Pleora.h"
#include "PleoraCamera.h"
#include <video\shvideo.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "PleoraCamera.cpp"

IMPLEMENT_RUNTIME_GADGET_EX( PleoraCamera , C1394Camera , "Video.capture" , TVDB400_PLUGIN_NAME );

CamProperty HiSpec1[] =
{
  { FGP_FRAME_RATE , "FrameRate" , "AcquisitionFrameRateRaw" , NULL , NULL , NULL } ,
  { FGP_EXTSHUTTER , "Shutter_us" , "ExposureTimeRaw" , NULL , NULL , true , NULL } ,
  { FGP_GAIN , "Gain" , "GainRaw" , NULL , NULL , true , NULL } ,
  { FGP_ROI , "ROI" , NULL , NULL , NULL , true , NULL } ,
  { FGP_ACQUISITION_SRC , "AcquisitionSource" , "AcquisitionSource" , NULL , NULL , true , NULL } ,
  { FGP_SYNC_SIGNAL , "SyncSignalMode" , "SyncOutSelect" , NULL , NULL , true , NULL } ,
  { FGP_TRIGGERONOFF , "TriggerMode" , "SyncInEnable" , NULL , NULL , true , NULL } ,
  { FGP_TRIGGER_POLARITY , "Tr.Polarity" , "SyncInPolarity" , NULL , NULL , true , NULL } ,
  { FGP_ARM_POLARITY , "SyncARMPolarity" , "SyncARMPolarity" , NULL , NULL , true , NULL } ,
  { FGP_LOG , "Log" , NULL , NULL , NULL , true , NULL }

} ;

CamTypeAndProperties HiSpec1TP =
{
  _T( "HiSpec 1" ) , ARR_SIZE( HiSpec1 ) , &HiSpec1[ 0 ]
} ;

CamTypeAndProperties * KnownCameras[] =
{ &HiSpec1TP , NULL } ;

CamProperty CommonProperties[] =
{
  { FGP_IMAGEFORMAT , "PixelFormat" , "PixelFormat" , NULL , NULL , true } ,
  { FGP_ACQU_MODE , "AcqMode" , "AcquisitionMode" , NULL , NULL , true } ,
  { FGP_FRAME_RATE , "FrameRate" , "AcquisitionFrameRate" , NULL , NULL } ,
  { FGP_GAMMA , "Gamma" , "Gamma" , NULL , NULL , true } ,
  { FGP_EXTSHUTTER , "Shutter_us" , "ExposureTime" , "ExposureAuto"/*"ExposureMode"*/ , NULL , true } ,
  { FGP_EXTSHUTTER , "Exposure_us" , "ExposureTimeAbs" , "ExposureAuto"/*"ExposureMode"*/ , NULL , true } ,
  { FGP_AUTOEXPOSURE , "ExpTarget" , "ExposureAutoTarget" , NULL , NULL , true } ,
  { FGP_GAIN , "Gain_dBx10" , "Gain" , "GainAuto" , NULL , true } ,
  { FGP_GAIN , "SensorGain" , "SensorGain" , NULL , NULL , true } ,
  { FGP_ROI , "ROI" , NULL , NULL , NULL , true } ,
  { FGP_TRIGGERONOFF , "TriggerMode" , "TriggerMode" , NULL , NULL , true } ,
  { FGP_TRIGGER_SOURCE , "TriggerSource" , "TriggerSource" , NULL , NULL , true } ,
  //  {FGP_TRIG_EVENT , "TrigEvent" , "FrameStartTriggerEvent" , NULL , NULL , true },
  { FGP_TRIGGERDELAY , "TriggerDelay" , "TriggerDelay" , NULL , NULL , true } ,
  { FGP_TRIGGER_POLARITY , "Tr.Polarity" , "TriggerActivation" , NULL , NULL , true } ,
  { FGP_OUT_DELAY , "OutputDelay" , "IntEnaDelayTime" , NULL , NULL , true } ,
  { FGP_BANDWIDTH , "LAN_Capacity" , "StreamBytesPerSecond" , NULL , NULL , true } ,
  { FGP_WHITEBALCB , "WhiteBalBlue" , "WhitebalValueBlue" , "WhitebalMode Manual-Auto" , NULL , true } ,
  { FGP_WHITEBALCR , "WhiteBalRed" , "WhitebalValueRed" , NULL , NULL , true } ,
  { FGP_WHITEBAL_RATIO , "WB_Ratio" , "BalanceRatioRaw" , "BalanceWhiteAuto" , NULL , true } ,
  { FGP_WHITEBAL_RATIO , "WB_Ratio" , "BalanceRatioAbs" , "BalanceWhiteAuto" , NULL , true } ,
  { FGP_WHITEBAL_SELECT , "WB_Base" , "BalanceRatioSelector" , NULL , NULL , true } ,
  { FGP_IRIS , "Iris" , NULL , NULL , NULL , true } ,
  { FGP_FOCUS , "Focus" , NULL , NULL , NULL , true } ,
  { FGP_ZOOM , "Zoom" , NULL , NULL , NULL , true } ,
  { FGP_PAN , "Pan" , NULL , NULL , NULL , true } ,
  { FGP_TILT , "Tilt" , NULL , NULL , NULL , true } ,
  //   {FGP_GRAB , "Grab" , "AcquisitionFrameCount" , NULL  , NULL , true } ,
  { FGP_LINE_SELECT , "LineSelect" , "LineSelector" , NULL , NULL , true } ,
  { FGP_LINE_PARAMS , "LineParams" , NULL , NULL , NULL , true } ,
  { FGP_LINE_SOURCE , "LineSource" , "LineSource" , NULL , NULL , true } ,
  { FGP_LINE_INVERSE , "LineInverter" , "LineInverter" , NULL , NULL , true } ,
  { FGP_LINE_DEBOUNCE , "Debouncing" , "LineDebounceTime" , NULL , NULL , true } ,
  //   {FGP_EXP_AUTO_OUT , "ExposureAuto" , "ExposureAuto" , NULL , NULL , true  } 
  //  ,{FGP_EXP_AUTO_ALG , "EXP_Algorithm" , "ExposureAutoAlg" , NULL , NULL , true  } 
  //  ,{FGP_GAIN_AUTO_TARG , "GAIN_Target" , "GainAutoTarget" , NULL , NULL , true  } 
  //  ,{FGP_GAIN_AUTO_OUT , "GAIN_Outliers" , "GainAutoOutliers" , NULL , NULL , true  } 
  { FGP_USER_SET_SELECT , "SaveLoad" , "UserSetSave" , "UserSetLoad" , NULL , true } , // UI, SaveCommand, LoadCommand
  // the next is not for UI, only info for operations with sets
  { FGP_USER_SET_DEF , "UserSetSelector" , "UserSetDefaultSelector" , "Enum" , NULL , true } , // Name for selector, Command 
  { FGP_LOG , "Log" , NULL , NULL , NULL , true }
};


FG_PARAMETER GrabStopProperties[] = 
{
  FGP_IMAGEFORMAT , FGP_ACQU_MODE , FGP_FRAME_RATE , FGP_ROI ,
  FGP_TRIGGERONOFF , FGP_BANDWIDTH , FGP_ACQUISITION_SRC
} ;

bool IsStopNecessary( FG_PARAMETER Par )
{
  for ( int i = 0 ; i < ARR_SIZE( GrabStopProperties ) ; i++ )
  {
    if ( Par == GrabStopProperties[ i ] )
      return true ;
  }
  return false ;
}
Videoformats vFormats[] =
{
  { PvPixelMono8 , "Mono8" , true , 1. , CM_Y8 , BI_Y8 } ,
  { PvPixelMono16 , "Mono16" , true , 2.0 , CM_Y16 , BI_Y16 } ,
  { PvPixelMono12 , "Mono12" , true , 2.0 , CM_UNKNOWN , BI_Y12 } ,
  { PvPixelMono12Packed , "Mono12Packed" , true , 1.5 , CM_UNKNOWN , BI_YP12 } ,
  { PvPixelYUV411Packed , "YUV411" , true , 1.5 , CM_YUV411 , BI_YUV9 } ,
  { PvPixelYUV422Packed , "YUV422" , false , 2.0 , CM_YUV422 , BI_YUV12 } ,
  { PvPixelYUV444Packed , "YUV444" , false , 3.0 , CM_YUV444 , BI_YUV12 } ,
  { PvPixelRGB8Packed , "RGB24" , false , 3.0 , CM_UNKNOWN , BI_YUV12 } ,
  { PvPixelRGB16Packed , "RGB48" , false , 6.0 , CM_UNKNOWN , BI_YUV12 } ,
  { PvPixelBGR8Packed , "BGR24" , false , 3.0 , CM_UNKNOWN , BI_YUV12 } ,
  { PvPixelRGBA8Packed , "RGBA32" , false , 4.0 , CM_UNKNOWN , BI_YUV12 } ,
  { PvPixelBGRA8Packed , "BGRA32" , false , 4.0 , CM_UNKNOWN , BI_YUV12 } ,
  { PvPixelMono12Packed , "Mono12Packed" , true , 1.5 , CM_UNKNOWN , BI_Y16 } ,
  { PvPixelBayerGR12Packed , "Bayer12Packed" , false , 1.5 , CM_UNKNOWN , BI_Y16 } ,
  { PvPixelBayerGR8 , "RAW8" , true , 1.0 , CM_RAW8 , BI_Y8 } ,
  { PvPixelBayerGR16 , "RAW16" , true , 2.0 , CM_RAW16 , BI_Y16 }
};

FXLockObject                PleoraCamera::m_ConfigLock ;
CAMERA1394::CameraInfo      PleoraCamera::m_CamInfo[ MAX_CAMERASNMB ] ;
int                         PleoraCamera::m_iCamNum = 0 ;
FXArray<BusyCamera>         PleoraCamera::m_BusyCameras ;
DWORD                       PleoraCamera::m_dwInstanceCount = 0 ;
bool                        PleoraCamera::m_bSaveFullInfo = true ;
#ifdef _DEBUG
DWORD PleoraCamera::m_dwDefaultTimeout = 60000 ;
#else
DWORD PleoraCamera::m_dwDefaultTimeout = 1000 ;
#endif

LPCTSTR GetParamTypeName( PvGenType Type )
{
  switch ( Type )
  {
    case PvGenTypeInteger: return "Integer" ;
    case PvGenTypeEnum:    return "Enum" ;
    case PvGenTypeBoolean: return "Boolean" ;
    case PvGenTypeString:  return "String" ;
    case PvGenTypeCommand:   return "Command" ;
    case PvGenTypeFloat:     return "Float" ;
    case PvGenTypeRegister:  return "Register" ;
    case PvGenTypeUndefined: 
    default:             return "Undefined" ;
  }
}

int iDataFrameLen = sizeof( CDataFrame ) ;
int iTextFrameLen = sizeof( CTextFrame ) ;
int iVideoFrameLen = sizeof( CBooleanFrame ) ;
int iQuantityFrameLen = sizeof( CQuantityFrame ) ;

int PleoraCamera::GetPropertyIndex( LPCTSTR name )
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

int PleoraCamera::GetPropertyIndex( FG_PARAMETER id )
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


bool PleoraCamera::GetULongFeature( /*PvGenParameterArray * pParameters ,*/
  LPCTSTR pFeatureName , PvUInt32& ulValue )
{
  PvGenParameterArray * pParameters = m_Camera.GetGenParameters() ;
  PvInt64 Val ;
  PvResult Result = pParameters->GetIntegerValue( PvString( pFeatureName ) , Val ) ;
  if ( Result.IsOK() )
  {
    ulValue = ( PvUInt32 )Val ;
    return true ;
  }
  else
    {
    SEND_DEVICE_ERR( "Can't read value for %s: %s" , pFeatureName ,
      Result.GetDescription().GetAscii() ) ;
  }
  return false ;
}
bool PleoraCamera::GetIntFeature( /*PvGenParameterArray * pParameters ,*/
  LPCTSTR pFeatureName , PvInt32& ulValue )
      {
  PvGenParameterArray * pParameters = m_Camera.GetGenParameters() ;
  PvInt64 Val ;
  PvResult Result = pParameters->GetIntegerValue( PvString( pFeatureName ) , Val ) ;
        if ( Result.IsOK() )
        {
    ulValue = ( PvInt32 ) Val ;
          return true ;
        }
        else
  {
    SEND_DEVICE_ERR( "Can't read value for %s: %s" ,
      pFeatureName , Result.GetDescription().GetAscii() ) ;
  }
  return false ;
}
bool PleoraCamera::GetPropertyValue(
  PvGenParameterArray * pProperties , SetCamPropertyData& Value )
  {
  if ( !pProperties )
  {
    SEND_DEVICE_ERR( "GetPropertyValue - zero property pointer " ) ;
    return false ;
  }
  PvResult Result ;
  PvGenParameter * pParam = pProperties->Get( PvString( Value.m_Name ) ) ;
  if ( !pParam )
  {
    sprintf_s( Value.m_szString , "Can't get GenParam for %s: %s" , Value.m_Name ,
      ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
    return false ;
  }
  Result = pParam->GetType( Value.m_Type ) ;
  if ( !Result.IsOK() )
  {
    sprintf_s( Value.m_szString , "Can't get type for %s: %s" , Value.m_Name ,
      ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
    return false ;
  }

  switch ( Value.m_Type )
  {
    case PvGenTypeInteger:
  {
    if ( GetIntFeature( /*pProperties ,*/ Value.m_Name , Value.m_int ) )
      return true ;
    sprintf_s( Value.m_szString , "Can't get integer value for %s: %s" , Value.m_Name ,
      ( LPCTSTR ) Result.GetDescription().GetAscii() ) ;
    return false ;
  }
    case PvGenTypeFloat:
    {
    Result = pProperties->GetFloatValue( Value.m_Name , Value.m_double ) ;
    if ( Result.IsOK() )
      return true ;
    sprintf_s( Value.m_szString , "Can't get float value for %s: %s" , Value.m_Name ,
      ( LPCTSTR ) Result.GetDescription().GetAscii() ) ;
    return false ;
    }
    case PvGenTypeEnum:
    {
      PvString sVal ;
    Result = pProperties->GetEnumValue( Value.m_Name , sVal ) ;
      if ( Result.IsOK() )
      {
        strcpy_s( Value.m_szString , sVal.GetAscii() ) ;
        return true ;
      }
    sprintf_s( Value.m_szString , "Can't get enum value for %s: %s" , Value.m_Name ,
      ( LPCTSTR ) Result.GetDescription().GetAscii() ) ;
      return false ;
    }
    case PvGenTypeString:
    {
      PvString sVal ;
    Result = pProperties->GetString( Value.m_Name , sVal ) ;
      if ( Result.IsOK() )
      {
        strcpy_s( Value.m_szString , sVal.GetAscii() ) ;
        return true ;
      }
    sprintf_s( Value.m_szString , "Can't get string value for %s: %s" , Value.m_Name ,
        ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
      return false ;
    }
    case PvGenTypeBoolean:
    {
    Result = pProperties->GetBooleanValue( Value.m_Name , Value.m_bBool ) ;
    if ( Result.IsOK() )
      return true ;
    sprintf_s( Value.m_szString , "Can't get boolean value for %s: %s" , Value.m_Name ,
      ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
    return false ;
    }
    case PvGenTypeRegister:
    {
    PvGenRegister* pPropPtr = pProperties->GetRegister( Value.m_Name ) ;
    if ( pPropPtr )
    {
      Result = pPropPtr->GetLength( Value.m_int64 ) ;
      if ( Value.m_int64 > 100 )
      {
        SEND_DEVICE_ERR( "Length of %s register property > 100 (%d)" ,
          Value.m_Name , ( int ) Value.m_int64 ) ;
        return false ;
      }
      Result = pPropPtr->Get( Value.m_Buffer , Value.m_int64 ) ;
      if ( Result.IsOK() )
        return true ;
      sprintf_s( Value.m_szString , "Can't get register value for %s(L=%d): %s" ,
        Value.m_Name , ( int ) Value.m_int64 ,
        ( LPCTSTR ) Result.GetDescription().GetAscii() ) ;
        return false ;
      }
    sprintf_s( Value.m_szString , "Can't Get Register for %s" , Value.m_Name ) ;
    return false ;
    }
    default:
    SEND_DEVICE_ERR( "Unsupported property type %d for %s" , Value.m_Type , Value.m_Name ) ;
      return false ;
  }
  return false ;
}

bool PleoraCamera::GetPropertyValue( SetCamPropertyData& Value )
{
  if ( !m_Camera.IsConnected() )
  {
    SEND_DEVICE_ERR( "GetPropertyValue - Camera is not connected " ) ;
    return false ;
  }
  PvGenParameterArray * pProperties = m_Camera.GetGenParameters() ;
  if ( !pProperties )
  {
    SEND_DEVICE_ERR( "GetPropertyValue - zero property pointer " ) ;
    return false ;
  }
  PvResult Result ;
  PvGenParameter * pParam = pProperties->Get( PvString( Value.m_Name ) ) ;
  if ( !pParam )
  {
    sprintf_s( Value.m_szString , "Can't get GenParam for %s: %s" , Value.m_Name ,
      ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
    return false ;
  }
  Result = pParam->GetType( Value.m_Type ) ;
  if ( !Result.IsOK() )
  {
    sprintf_s( Value.m_szString , "Can't get type for %s: %s" , Value.m_Name ,
      ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
    return false ;
  }

  switch ( Value.m_Type )
  {
  case PvGenTypeInteger:
  {
    //PvGenInteger * pInt = pProperties->GetInteger( PvString( Value.m_Name ) ) ;
    PvGenInteger * pInt = dynamic_cast<PvGenInteger *> (pParam) ;
    if ( pInt )
    {
      PvInt64 Val ;
      Result = pInt->GetValue( Val ) ;
      if ( !Result.IsOK() )
      {
        sprintf_s( Value.m_szString , "Can't get int value for %s: %s" , Value.m_Name ,
          ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
        return false ;
      }
      Value.m_int = ( int )Val ;
      Value.m_int64 = Val ;
      return true ;
    }
    sprintf_s( Value.m_szString , "Can't get GenInteger for %s: %s" , Value.m_Name ,
      ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
    return false ;
  }
  case PvGenTypeFloat:
  {
    Result = pProperties->GetFloatValue( Value.m_Name , Value.m_double ) ;
    if ( Result.IsOK() )
      return true ;
    sprintf_s( Value.m_szString , "Can't get float value for %s: %s" , Value.m_Name ,
      ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
    return false ;
  }
  case PvGenTypeEnum:
  {
    PvString sVal ;
    PvString PropName( Value.m_Name ) ;
    PvGenEnum * pEnum = pProperties->GetEnum( PropName ) ;
    if ( !pEnum )
    {
      sprintf_s( Value.m_szString , "Can't get GenEnum for %s" , Value.m_Name ) ;
      return false ;
    }
    PvInt64 i64Val = 0L ;
    Result = pEnum->GetValue( i64Val ) ;
    Result = pEnum->GetValue( sVal ) ;
    if ( Result.IsOK() )
    {
      strcpy_s( Value.m_szString , sVal.GetAscii() ) ;
      return true ;
    }
    sprintf_s( Value.m_szString , "Can't get enum value for %s: %s" , Value.m_Name ,
      ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
    return false ;
  }
  case PvGenTypeString:
  {
    PvString sVal ;
    Result = pProperties->GetString( Value.m_Name , sVal ) ;
    if ( Result.IsOK() )
    {
      strcpy_s( Value.m_szString , sVal.GetAscii() ) ;
      return true ;
    }
    sprintf_s( Value.m_szString , "Can't get string value for %s: %s" , Value.m_Name ,
      ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
    return false ;
  }
  case PvGenTypeBoolean:
  {
    Result = pProperties->GetBooleanValue( Value.m_Name , Value.m_bBool ) ;
    if ( Result.IsOK() )
      return true ;
    sprintf_s( Value.m_szString , "Can't get boolean value for %s: %s" , Value.m_Name ,
      ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
    return false ;
  }
  case PvGenTypeRegister:
  {
    PvGenRegister* pPropPtr = pProperties->GetRegister( Value.m_Name ) ;
    if ( pPropPtr )
    {
      Result = pPropPtr->GetLength( Value.m_int64 ) ;
      if ( Value.m_int64 > 100 )
      {
        SEND_DEVICE_ERR( "Length of %s register property > 100 (%d)" ,
          Value.m_Name , ( int )Value.m_int64 ) ;
        return false ;
      }
      Result = pPropPtr->Get( Value.m_Buffer , Value.m_int64 ) ;
      if ( Result.IsOK() )
        return true ;
      sprintf_s( Value.m_szString , "Can't get register value for %s(L=%d): %s" ,
        Value.m_Name , ( int )Value.m_int64 ,
        ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
      return false ;
    }
    sprintf_s( Value.m_szString , "Can't Get Register for %s" , Value.m_Name ) ;
    return false ;
  }
  default:
    SEND_DEVICE_ERR( "Unsupported property type %d for %s" , Value.m_Type , Value.m_Name ) ;
    return false ;
  }
  return false ;
}

bool PleoraCamera::GetPropertyValue(
  PvGenParameterArray * pProperties , LPCTSTR Name , SetCamPropertyData& Value )
{
  strcpy_s( Value.m_Name , Name ) ;
  return GetPropertyValue( pProperties , Value ) ; 
  }

bool PleoraCamera::GetPropertyValue(
  LPCTSTR Name , SetCamPropertyData& Value )
{
  strcpy_s( Value.m_Name , Name ) ;
  return GetPropertyValue( Value ) ;
}

bool PleoraCamera::SetPropertyValue(
  PvGenParameterArray*  pPropPtr , SetCamPropertyData& Value )
  {
  if ( !pPropPtr )
  {
    SEND_DEVICE_ERR( "SetPropertyValue Error - zero property array ptr " ) ;
    return false ;
  }
  PvResult Result ;
  switch ( Value.m_Type )
  {
  case PvGenTypeInteger:
    Value.m_int64 = Value.m_int ;
    Result = pPropPtr->SetIntegerValue( Value.m_Name , Value.m_int64 ) ;
    if ( Result.IsOK() )
      return true ;
    sprintf_s( Value.m_szString , "SetPropertyValue Error -Can't set integer value %s to %ld: %s" , 
      Value.m_Name , Value.m_int64 , 
      ( LPCTSTR ) Result.GetDescription().GetAscii() ) ;
    return false ;
  case PvGenTypeFloat:
  {
      Result = pPropPtr->SetFloatValue( Value.m_Name , Value.m_double ) ;
      if ( Result.IsOK() )
        return true ;
      sprintf_s( Value.m_szString , "SetPropertyValue Error -Can't set float value %s to %g : %s" ,
        Value.m_Name , Value.m_double , 
        ( LPCTSTR ) Result.GetDescription().GetAscii() ) ;
    return false ;
  }
  case PvGenTypeEnum:
    {
      PvString sVal ;
    Result = pPropPtr->SetEnumValue( Value.m_Name , Value.m_szString ) ;
    if ( Result.IsOK() )
      return true ;
    sprintf_s( Value.m_szString , "SetPropertyValue Error -Can't set enum value %s to %s : %s" ,
      Value.m_Name , Value.m_szString , 
      ( LPCTSTR ) Result.GetDescription().GetAscii() ) ;
    return false ;
    }
    case PvGenTypeString:
    {
      PvString sVal ;
      Result = ( ( PvGenString* )pPropPtr )->SetValue( Value.m_szString ) ;
    if ( Result.IsOK() )
      return true ;
    sprintf_s( Value.m_szString , "SetPropertyValue Error -Can't set string value %s to %s : %s" ,
      Value.m_Name , Value.m_szString ,
      ( LPCTSTR ) Result.GetDescription().GetAscii() ) ;
    return false ;
    }
    case PvGenTypeBoolean:
    {
      Result = ( ( PvGenBoolean* )pPropPtr )->SetValue( Value.m_bBool ) ;
    if ( Result.IsOK() )
      return true ;
    sprintf_s( Value.m_szString , "SetPropertyValue Error -Can't set bool value %s to %d : %s" ,
      Value.m_Name , Value.m_bBool? 1 : 0 ,
      ( LPCTSTR ) Result.GetDescription().GetAscii() ) ;
  return false ;
}
  case PvGenTypeRegister:
{
    PvGenRegister * pReg = pPropPtr->GetRegister( Value.m_Name ) ;
    if ( !pReg )
  {
      sprintf_s( Value.m_szString , "SetPropertyValue Error -Can't get register %s : %s" ,
        Value.m_Name , ( LPCTSTR ) Result.GetDescription().GetAscii() ) ;
    return false ;
  }
    Result = pReg->GetLength( Value.m_int64 ) ;
    if ( Value.m_int64 > 100 )
  {
      SEND_DEVICE_ERR( "SetPropertyValue Error -Length of %s register property > 100 (%d)" ,
        Value.m_Name , ( int ) Value.m_int64 ) ;
    return false ;
  }
    if ( (int)Value.m_int64 != Value.m_int )
  {
      SEND_DEVICE_ERR( "SetPropertyValue Error -Length of %s register property %d differs from required (%d)" ,
        Value.m_Name , ( int ) Value.m_int64 , Value.m_int ) ;
    return false ;
  }

    Result = pReg->Set( Value.m_Buffer , Value.m_int64 ) ;
  if ( Result.IsOK() )
      return true ;
    sprintf_s( Value.m_szString , "SetPropertyValue Error - Can't set register value for %s: %s" ,
      Value.m_Name , ( LPCTSTR ) Result.GetDescription().GetAscii() ) ;
  }
  default:
    SEND_DEVICE_ERR( "SetPropertyValue Error - Unsupported property type %d for %s" , 
      Value.m_Type , Value.m_Name ) ;
  return false ;
}
    return false ;

}

bool PleoraCamera::SetPropertyValue( PvGenParameterArray * pParameters ,
  LPCTSTR Name , SetCamPropertyData& Value )
  {
  strcpy_s( Value.m_Name , Name ) ;
  return SetPropertyValue( pParameters , Value ) ;
  }

bool PleoraCamera::SetPropertyValue(
  LPCTSTR pName , SetCamPropertyData& Value )
{
  strcpy_s( Value.m_Name , pName ) ;
  return SetPropertyValue( Value ) ;
}

bool PleoraCamera::SetPropertyValue(
  SetCamPropertyData& Value )
{
  if ( !m_Camera.IsConnected() )
  {
    SEND_DEVICE_ERR( "SetPropertyValue Error - Camera is not connected " ) ;
    return false ;
  }
  PvGenParameterArray * pPropPtr = m_Camera.GetGenParameters() ;
  if ( !pPropPtr )
  {
    SEND_DEVICE_ERR( "SetPropertyValue Error - zero property array ptr " ) ;
    return false ;
  }
  
  PvResult Result ;
  PvGenParameter * pParam = pPropPtr->Get( PvString( Value.m_Name ) ) ;
  if ( !pParam )
  {
    sprintf_s( Value.m_szString , "Can't get GenParam for %s: %s" , Value.m_Name ,
      ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
    return false ;
  }
  Result = pParam->GetType( Value.m_Type ) ;
  if ( !Result.IsOK() )
  {
    sprintf_s( Value.m_szString , "Can't get type for %s: %s" , Value.m_Name ,
      ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
    return false ;
  }

  switch ( Value.m_Type )
  {
  case PvGenTypeInteger:
    Value.m_int64 = Value.m_int ;
    Result = pPropPtr->SetIntegerValue( Value.m_Name , Value.m_int64 ) ;
    if ( Result.IsOK() )
      return true ;
    sprintf_s( Value.m_szString , "SetPropertyValue Error -Can't set integer value %s to %ld: %s" ,
      Value.m_Name , Value.m_int64 ,
      ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
    return false ;
  case PvGenTypeFloat:
  {
    Result = pPropPtr->SetFloatValue( Value.m_Name , Value.m_double ) ;
    if ( Result.IsOK() )
      return true ;
    sprintf_s( Value.m_szString , "SetPropertyValue Error -Can't set float value %s to %g : %s" ,
      Value.m_Name , Value.m_double ,
      ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
    return false ;
  }
  case PvGenTypeEnum:
  {
    PvString sVal ;
    Result = pPropPtr->SetEnumValue( Value.m_Name , Value.m_szString ) ;
    if ( Result.IsOK() )
      return true ;
    sprintf_s( Value.m_szString , "SetPropertyValue Error -Can't set enum value %s to %s : %s" ,
      Value.m_Name , Value.m_szString ,
      ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
    return false ;
  }
  case PvGenTypeString:
  {
    PvString sVal ;
    Result = ( ( PvGenString* )pPropPtr )->SetValue( Value.m_szString ) ;
    if ( Result.IsOK() )
      return true ;
    sprintf_s( Value.m_szString , "SetPropertyValue Error -Can't set string value %s to %s : %s" ,
      Value.m_Name , Value.m_szString ,
      ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
    return false ;
  }
  case PvGenTypeBoolean:
  {
    Result = ( ( PvGenBoolean* )pPropPtr )->SetValue( Value.m_bBool ) ;
    if ( Result.IsOK() )
      return true ;
    sprintf_s( Value.m_szString , "SetPropertyValue Error -Can't set bool value %s to %d : %s" ,
      Value.m_Name , Value.m_bBool ? 1 : 0 ,
      ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
    return false ;
  }
  case PvGenTypeRegister:
  {
    PvGenRegister * pReg = pPropPtr->GetRegister( Value.m_Name ) ;
    if ( !pReg )
    {
      sprintf_s( Value.m_szString , "SetPropertyValue Error -Can't get register %s : %s" ,
        Value.m_Name , ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
      return false ;
    }
    Result = pReg->GetLength( Value.m_int64 ) ;
    if ( Value.m_int64 > 100 )
    {
      SEND_DEVICE_ERR( "SetPropertyValue Error -Length of %s register property > 100 (%d)" ,
        Value.m_Name , ( int )Value.m_int64 ) ;
      return false ;
    }
    if ( ( int )Value.m_int64 != Value.m_int )
    {
      SEND_DEVICE_ERR( "SetPropertyValue Error -Length of %s register property %d differs from required (%d)" ,
        Value.m_Name , ( int )Value.m_int64 , Value.m_int ) ;
      return false ;
    }

    Result = pReg->Set( Value.m_Buffer , Value.m_int64 ) ;
    if ( Result.IsOK() )
      return true ;
    sprintf_s( Value.m_szString , "SetPropertyValue Error - Can't set register value for %s: %s" ,
      Value.m_Name , ( LPCTSTR )Result.GetDescription().GetAscii() ) ;
  }
  default:
    SEND_DEVICE_ERR( "SetPropertyValue Error - Unsupported property type %d for %s" ,
      Value.m_Type , Value.m_Name ) ;
    return false ;
  }
  return false ;

}

bool PleoraCamera::SetULongFeature( /*PvGenParameterArray * pParameters ,*/
  LPCTSTR pFeatureName , PvUInt32 ulValue )
{
  SetCamPropertyData Data ;
  Data.m_int64 = ulValue ;
  Data.m_Type = PvGenTypeInteger ;
  strcpy_s( Data.m_Name , pFeatureName ) ;

  return SetPropertyValue( pFeatureName , Data ) ;
}


PvUInt32 PleoraCamera::GetXSize()
{
  PvUInt32 lValue;
  if ( GetULongFeature( "Width" , lValue ) )
    return lValue;
  return 0;
}

void PleoraCamera::SetWidth( PvUInt32 Width )
{
  SetULongFeature( "Width" , Width ) ;
}

PvUInt32 PleoraCamera::GetYSize( )
{
  PvUInt32 lValue;
  if ( GetULongFeature( "Height" , lValue ) )
    return lValue;
  return 0;
}

void PleoraCamera::SetHeight( PvUInt32 Height )
{
  SetULongFeature( "Height" , Height ) ;
}

PvUInt32 PleoraCamera::GetXOffset()
{
  PvUInt32 lValue;
  if ( GetULongFeature( "OffsetX" , lValue ) )
    return lValue;
  return 0;
}
void PleoraCamera::SetXOffset(PvUInt32 XOffset )
{
  SetULongFeature( "OffsetX" , XOffset ) ;
}

PvUInt32 PleoraCamera::GetYOffset()
{
  PvUInt32 lValue;
  if ( GetULongFeature( "OffsetY" , lValue ) )
    return lValue;
  return 0;
}
void PleoraCamera::SetYOffset( PvUInt32 YOffset )
{
  SetULongFeature( "OffsetY" , YOffset ) ;
}

PvUInt32 PleoraCamera::GetMaxWidth()
{
  PvUInt32 lValue;
  if ( GetULongFeature( "SensorWidth" , lValue ) )
    return lValue;
  return 0;
}

PvUInt32 PleoraCamera::GetMaxHeight()
{
  PvUInt32 lValue;

  if ( GetULongFeature( "SensorHeight" , lValue ) )
    return lValue;
  return 0;
}

CVideoFrame * PleoraCamera::ConvertPleoraToSHformat( const PvImage * pFrame )
{
  double dStart = GetHRTickCount() ;
//   double dGetTime , dToRGBTime , dToYUV9Time ;
  CVideoFrame * pOut = NULL ;

  PvPixelType PixFormat = pFrame->GetPixelType() ;
  if ( PixFormat )
  {
    PvUInt32 Lx = pFrame->GetWidth() , Ly = pFrame->GetHeight() ;
    if ( Lx && Ly )
    {
      unsigned iSize = Lx * Ly ;
      const PvUInt8 * data = NULL ;
      switch ( PixFormat )
      {
        case PvPixelMono8:
        {
          LPBITMAPINFOHEADER lpBMIH = ( LPBITMAPINFOHEADER )
            malloc( sizeof( BITMAPINFOHEADER ) + iSize );

          memset( lpBMIH , 0 , sizeof( BITMAPINFOHEADER ) );
          lpBMIH->biSize = sizeof(BITMAPINFOHEADER) ;
          lpBMIH->biWidth = Lx ;
          lpBMIH->biHeight = Ly ;
          lpBMIH->biSizeImage = iSize ;
          lpBMIH->biCompression = BI_Y8 ;
          data = pFrame->GetDataPointer() ;
          memcpy( &lpBMIH[ 1 ] , data , iSize ) ;
          CVideoFrame* vf = CVideoFrame::Create();
          vf->lpBMIH = lpBMIH ;
          vf->lpData = NULL ;
          vf->SetLabel( m_CameraID );
          vf->SetTime( GetHRTickCount() ) ;
          pOut = vf ;
        }
        break ;
        case PvPixelMono10:
        case PvPixelMono12:
        case PvPixelMono14:
        case PvPixelMono16:
        {
          iSize *= 2 ;
          LPBITMAPINFOHEADER lpBMIH = ( LPBITMAPINFOHEADER )malloc( sizeof( BITMAPINFOHEADER ) + iSize );
          memcpy( lpBMIH , &m_BMIH , sizeof( BITMAPINFOHEADER ) );
          lpBMIH->biSize = sizeof(BITMAPINFOHEADER) ;
          lpBMIH->biWidth = Lx ;
          lpBMIH->biHeight = Ly ;
          lpBMIH->biSizeImage = iSize ;
          lpBMIH->biCompression = BI_Y16 ;
          data = pFrame->GetDataPointer() ;
          memcpy( &lpBMIH[ 1 ] , data , iSize ) ;
          CVideoFrame* vf = CVideoFrame::Create();
          vf->lpBMIH = lpBMIH ;
          vf->lpData = NULL ;
          vf->SetLabel( m_CameraID );
          vf->SetTime( GetHRTickCount() ) ;
          pOut = vf ;
        }
        break ;
      }
    }
  }
  return pOut ;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PleoraCamera::PleoraCamera()
  : m_Pipeline( &m_Stream )
{
  //   if ( VmbErrorSuccess == m_System.Startup() )  // Startup will be called in camera control thread
  //   {
  double dStart = GetHRTickCount() ;
  Sleep( 50 ) ;

  m_pLogOutput = new COutputConnector( text ) ;

  m_iNProperties = ARR_SIZE( CommonProperties )  ;
  m_dwSerialNumber = -1 ;
  m_dwConnectedSerialNumber = 0 ;
  
  m_pInterface = NULL ;
  m_pDeviceInfo = NULL ;
  //m_Mode=MODE_0;
  m_CurrentROI = CRect( 0 , 0 , 640 , 480 );
  m_pixelFormat = PvPixelMono8 ; 
  m_uiPacketSize = 2048 ;
  m_FormatNotSupportedDispayed = false;
  m_GadgetInfo = "Pleora";
  m_WaitEventFrameArray[ 0 ] = m_evFrameReady = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
  m_WaitEventBusChangeArr[ 1 ] = m_WaitEventFrameArray[ 1 ] = m_evExit ;
  m_WaitEventBusChangeArr[ 0 ] = m_evBusChange = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
  m_hCamAccessMutex = CreateMutex( NULL , FALSE , NULL ) ;
  m_disableEmbeddedTimeStamp = false ;
  //m_isSelectingNewCamera = false ;
  m_dLastStartTime = 0. ;
  m_pNewFrame = NULL ;
  m_pNewImage = NULL ;
  m_hGrabThreadHandle = NULL ;
  m_dwGrabThreadId = 0 ;
  m_BusEvents = 0 ;
  m_bCamerasEnumerated = false ;
  m_CameraStatus = NotInitialized ;
  m_bInitialized = false ;
  m_bStopInitialized = false ;
  m_bRescanCameras = true ;
  m_iNNoSerNumberErrors = 0 ;
  m_iWBRed = m_iWBlue = 512 ;
  m_bLocalStopped = FALSE ;
  m_TriggerMode = TrigNotSupported ;
  m_dLastInCallbackTime = 0. ;
  m_dwNArrivedEvents = 0 ;
  m_iFPSx10 = 0 ;
  m_iSelectedLineNumber = 0 ;
  m_GadgetInfo = _T( "Pleora" ) ;
  m_dLastBuiltPropertyTime = 0. ;
  m_pOrigProperties = CommonProperties ;
  m_bViewErrorMessagesOnGetSet = true ;
  m_dExtShutter = 0. ;
  m_dLogPeriod_ms = 0. ;
  m_dLastLogTime_ms = 0. ;
  m_iLogCnt = 0 ;
  m_iNTemperatures = 0 ;
  m_iNFramesForAllocation = N_FRAMES_FOR_ALLOCATE ;
  m_iSyncSignalMode = 0 ;
  m_bGrabLoopContinue = false ;
  m_bStreamProgrammed = false ;
  m_pBuffers = NULL ;

  memset( m_dTemperatures , 0 , sizeof( m_dTemperatures ) ) ;

  memset( &m_BMIH , 0 , sizeof( BITMAPINFOHEADER ) );
  memset( &m_RealBMIH , 0 , sizeof( BITMAPINFOHEADER ) );
#ifdef _DEBUG
  m_iMutexTakeCntr = 0 ;
#endif
  double dBusyTime = GetHRTickCount() - dStart ;
  TRACE( "\nPleoraCamera::PleoraCamera: Start %g , Busy %g" , dStart , dBusyTime ) ;
}

PleoraCamera::~PleoraCamera()
{}

bool PleoraCamera::DriverInit()
{
  return EnumCameras();
}

void PleoraCamera::SaveCameraInfo( 
  PvInterface * pInterf , int iIndexOnInterf , 
  int iEnumIndex , FILE * pPropertyFile )
{
  double dCameraStart = GetHRTickCount() ;
  PvDeviceInfo * pInfo = pInterf->GetDeviceInfo( iIndexOnInterf ) ;
  if ( pInfo )
  {
    m_CamInfo[ iEnumIndex ].Id = pInfo->GetID().GetAscii() ;
    m_CamInfo[ iEnumIndex ].name = pInfo->GetUserDefinedName().GetAscii() ;
    m_CamInfo[ iEnumIndex ].model = pInfo->GetModel().GetAscii() ;
    m_CamInfo[ iEnumIndex ].szSerNum = pInfo->GetSerialNumber().GetAscii() ;
    m_CamInfo[ iEnumIndex ].InterfaceId.Format( "MAC=%s;IP=%s;Mask=%s" ,
      pInfo->GetMACAddress().GetAscii() ,
      pInfo->GetIPAddress().GetAscii() ,
      pInfo->GetSubnetMask().GetAscii() );
    TRACE( "Device: %s\n" , ( LPCTSTR )m_CamInfo[ iEnumIndex ].InterfaceId ) ;
    int iSNIndex = m_CamInfo[ iEnumIndex ].szSerNum.GetLength() - 1 ;
    LPCTSTR pStr = ( LPCTSTR )( m_CamInfo[ iEnumIndex ].szSerNum ) ;
    while ( iSNIndex > 0 && !isdigit( pStr[ iSNIndex ] ) )
      iSNIndex-- ;
    while ( iSNIndex > 0 && isdigit( pStr[ iSNIndex ] ) )
      iSNIndex-- ;
    if ( iSNIndex <= 0 )
      iSNIndex = 0 ;
    else
      iSNIndex++ ;

    m_CamInfo[ iEnumIndex ].serialnmb = atoi( pStr + iSNIndex ) ;

    FXString ErrorMsg ;
    PvDevice Camera ;
    PvResult Result = m_Camera.Connect( pInfo , PvAccessReadOnly ) ;
    if ( !Result.IsOK() )
    {
      ErrorMsg.Format( "Can't connect to SN=%s(%s) " ,
        m_CamInfo[ iEnumIndex ].szSerNum , (LPCTSTR)(m_CamInfo[ iEnumIndex ].InterfaceId) ) ;
      SEND_DEVICE_ERR( ( LPCTSTR )ErrorMsg ) ;
      return ;
    }
    PvGenParameterArray * pParameters = m_Camera.GetGenParameters()  ;
    if ( !pParameters )
    {
      ErrorMsg.Format( "Can't get Parameters for SN=%s." ,
        m_CamInfo[ iEnumIndex ].szSerNum ) ;
      SEND_DEVICE_ERR( ( LPCTSTR )ErrorMsg ) ;
      m_Camera.Disconnect() ;
      return ;
    }
    FXString FullDescription ;
    FXString Out ;
    double dAfterFileOpening = GetHRTickCount() ;
    TRACE( "\n  Pleora EnumCameras: Open camera %s(%d) time is %g" ,
      ( LPCTSTR )m_CamInfo[ iEnumIndex ].model , iEnumIndex , dAfterFileOpening - dCameraStart ) ;
    Out.Format( "Camera SN%s-%s Interface=%s time=%g\n Properties: \n" ,
      ( LPCTSTR )m_CamInfo[ iEnumIndex ].szSerNum , ( LPCTSTR )m_CamInfo[ iEnumIndex ].model ,
      ( LPCTSTR )m_CamInfo[ iEnumIndex ].InterfaceId , dAfterFileOpening - dCameraStart ) ;
    dCameraStart = dAfterFileOpening ;
    if ( pPropertyFile )
      fwrite( ( LPCTSTR )Out , Out.GetLength() , 1 , pPropertyFile ) ;
    FullDescription += Out ;
    double dAfterFileWrite = GetHRTickCount() ;
//     TRACE( "\n   Pleora EnumCameras: Write cam %s(%d) info time is %g" ,
//       ( LPCTSTR )m_CamInfo[ iEnumIndex ].model , iEnumIndex , dAfterFileWrite - dCameraStart ) ;

//     UINT uiNPars = pParameters->GetCount() ;
//     m_AllCamProperties.RemoveAll() ;
//     for ( UINT iIndex = 0 ; iIndex < uiNPars ; iIndex++ )
//     {
//       PvGenParameter * pPar = pParameters->Get( iIndex ) ;
//       if ( !pPar )
//       {
//         Out.Format( "Pleora SaveCamera %s_%s: Can't get parameter %\n" ,
//           ( LPCTSTR )m_CamInfo[ iEnumIndex ].model ,
//           ( LPCTSTR )m_CamInfo[ iEnumIndex ].szSerNum , iIndex ) ;
//         if ( pPropertyFile )
//         {
//           fwrite( ( LPCTSTR )Out , Out.GetLength() , 1 , pPropertyFile ) ;
//         }
//         FullDescription += Out ;
//         TRACE( "\n%s" , ( LPCTSTR )Out ) ;
//         continue ;
//       }
//       PvString Name ;
//       PvResult Result = pPar->GetName( Name ) ;
//       TRACE( "\n%d Name=%s    ***********   " , iIndex , ( LPCTSTR )Name ) ;
//       PvString Unit ;
//       PvString Category ;
//       Result = pPar->GetCategory( Category ) ;
//       if ( Result.IsFailure() )
//       {
//         TRACE( "\n Can't get Category %s" , Result.GetDescription().GetAscii() ) ;
//       }
//       PvString Descr ;
//       Result = pPar->GetDescription( Descr ) ;
//       if ( Result.IsFailure() )
//       {
//         TRACE( "\n Can't get Description %s" , Result.GetDescription().GetAscii() ) ;
//       }
//       PvGenType FType ;
//       Result = pPar->GetType( FType ) ;
//       if ( Result.IsFailure() )
//       {
//         TRACE( "\n Can't get Type %s" , Result.GetDescription().GetAscii() ) ;
//       }
//       Out.Format( "%s/%s: %s" , Category.GetAscii() , Name.GetAscii() , GetParamTypeName( FType ) ) ;
//       CameraAttribute NewAttribute( FGP_LAST , NULL , Name.GetAscii() ) ;
//       FXString FDescription ;
//       switch ( FType )
//       {
//       case PvGenTypeInteger:
//       {
//         ( ( PvGenInteger* )pPar )->GetMin( NewAttribute.m_i64Range[ 0 ] ) ;
//         ( ( PvGenInteger* )pPar )->GetMax( NewAttribute.m_i64Range[ 1 ] ) ;
//         NewAttribute.m_dRange[ 0 ] = ( double )NewAttribute.m_i64Range[ 0 ] ;
//         NewAttribute.m_dRange[ 1 ] = ( double )NewAttribute.m_i64Range[ 1 ] ;
//         ( ( PvGenInteger* )pPar )->GetValue( NewAttribute.m_i64Val ) ;
//         FDescription.Format( "%s:%s=%d Integer [%d,%d], %s" ,
//           Category.GetAscii() , Name.GetAscii() , ( int )NewAttribute.m_i64Val ,
//           ( int )NewAttribute.m_i64Range[ 0 ] , ( int )NewAttribute.m_i64Range[ 1 ] , Descr.GetAscii() ) ;
//       }
//       break;
//       case PvGenTypeFloat:
//       {
//         ( ( PvGenFloat* )pPar )->GetUnit( Unit ) ;
//         ( ( PvGenFloat* )pPar )->GetMin( NewAttribute.m_dRange[ 0 ] ) ;
//         ( ( PvGenFloat* )pPar )->GetMax( NewAttribute.m_dRange[ 1 ] ) ;
//         ( ( PvGenFloat* )pPar )->GetValue( NewAttribute.m_dVal ) ;
//         FDescription.Format( "%s:%s=%g %s double [%g,%g], %s" ,
//           Category.GetAscii() , Name.GetAscii() ,
//           NewAttribute.m_dVal , Unit.GetAscii() ,
//           ( int )NewAttribute.m_dRange[ 0 ] , ( int )NewAttribute.m_dRange[ 1 ] , Descr.GetAscii() ) ;
//       }
//       break;
//       case PvGenTypeEnum:
//       {
//         PvString Val ;
//         ( ( PvGenEnum* )pPar )->GetValue( Val ) ;
//         NewAttribute.m_enumVal = Val.GetAscii() ;
//         FDescription.Format( "%s:%s=%s Enumerator (" ,
//           Category.GetAscii() , Name.GetAscii() , Val.GetAscii() ) ;
//         NewAttribute.m_EnumRange.RemoveAll() ;
//         PvInt64 i64EnumCnt ;
//         ( ( PvGenEnum* )pPar )->GetEntriesCount( i64EnumCnt ) ;
//         for ( PvInt64 j = 0 ; j < i64EnumCnt ; j++ )
//         {
//           const PvGenEnumEntry * pEnumEntry ;
//           Result = ( ( PvGenEnum* )pPar )->GetEntryByIndex( j , &pEnumEntry ) ;
//           if ( Result.IsOK() )
//           {
//             PvString Enum ;
//             pEnumEntry->GetName( Enum ) ;
//             FXString Addition ;
//             Addition.Format( "%s%c" , Enum.GetAscii() ,
//               ( j == i64EnumCnt - 1 ) ? ')' : ',' ) ;
//             FDescription += Addition ;
//           }
//           else
//           {
//             ASSERT( 0 ) ;
//           }
//         }
//       }
//       break;
//       case PvGenTypeString:
//       {
//         PvString Val ;
//         ( ( PvGenString* )pPar )->GetValue( Val ) ;
//         FDescription.Format( "%s:%s=%s String %s" ,
//           Category.GetAscii() , Name.GetAscii() , Val.GetAscii() , Descr.GetAscii() ) ;
//       }
//       break;
//       case PvGenTypeBoolean:
//         ( ( PvGenBoolean* )pPar )->GetValue( NewAttribute.m_boolVal ) ;
//         FDescription.Format( "%s:%s=%s bool %s" ,
//           Category.GetAscii() , Name.GetAscii() ,
//           NewAttribute.m_boolVal ? "true" : "false" , Descr.GetAscii() ) ;
//         break;
//       case PvGenTypeUndefined:
//         FDescription.Format( "%s:%s Undefined" ,
//           Category.GetAscii() , Name.GetAscii() , Descr.GetAscii() ) ;
//         break;
//       case PvGenTypeCommand:
//         FDescription.Format( "%s:%s Command %s" ,
//           Category.GetAscii() , Name.GetAscii() , Descr.GetAscii() ) ;
//         break;
//       case PvGenTypeRegister:
//       {
//         PvInt64 i64Len = 0 ;
//         ( ( PvGenRegister* )pPar )->GetLength( i64Len ) ;
//         FDescription.Format( "%s:%s Register Length %d %s" ,
//           Category.GetAscii() , Name.GetAscii() , ( int )i64Len , Descr.GetAscii() ) ;
//       }
//       break;
//       default:
//         ASSERT( 0 ) ;
//         break ;
//       }
//       double dAfterPropertyInfoTaking = GetHRTickCount() ;
//       FXString Time ;
//       Time.Format( " T=%g\n" , dAfterPropertyInfoTaking - dAfterFileWrite ) ;
//       FDescription += Time ;
//       dAfterFileWrite = dAfterPropertyInfoTaking ;
//       NewAttribute.m_Description = FDescription ;
//       m_AllCamProperties.Add( NewAttribute ) ;
//       if ( !FDescription.IsEmpty() )
//       {
//         if ( pPropertyFile )
//           fwrite( ( LPCTSTR )FDescription , FDescription.GetLength() , 1 , pPropertyFile ) ;
//         FullDescription += FDescription ;
//         TRACE( FDescription ) ;
//         FDescription.Empty() ;
//       }
//     }
    m_Camera.Disconnect() ;
  }
}
//bool bSaveFullInfo = false ;
bool PleoraCamera::EnumCameras()
{
  double dStart = GetHRTickCount() ;
  FILE * pPropertyFile = NULL ;
  time_t CurTime ;
  time( &CurTime ) ;
  FXString FileName ;

  FXAutolock al( m_ConfigLock ) ;
  if ( m_bCamerasEnumerated && !m_bRescanCameras )
    return true ;

//  errno_t err ;
//  FileName.Format( _T( "AllCamProps%d.txt" ) , ( int )CurTime ) ;
//  err = fopen_s( &pPropertyFile , ( LPCTSTR )FileName , "wb" ) ;
//  ASSERT( err == 0 ) ;
  m_System.Find() ;
  PvUInt32 uiInterfCnt = m_System.GetInterfaceCount() ;
  m_AvailableInterfaces.RemoveAll() ;
  int iEnumIndex = 0 ;
  for ( PvUInt32 i = 0 ; i < uiInterfCnt ; i++ )
  {
    PvInterface * pInterf = m_System.GetInterface( i ) ;
    if ( pInterf )
    {
      FXString InterfaceInfo ;
      InterfaceInfo.Format( "MAC=%s;IP=%s;Mask=%s" ,
        pInterf->GetMACAddress().GetAscii() ,
        pInterf->GetIPAddress().GetAscii() ,
        pInterf->GetSubnetMask().GetAscii() );
      FXString FileOut ;
      FileOut.Format( "Interface: %s\n" , ( LPCTSTR )InterfaceInfo ) ;
      TRACE( (LPCTSTR)FileOut ) ;
      m_AvailableInterfaces.Add( InterfaceInfo ) ;
      
      PvUInt32 uiCamCnt = pInterf->GetDeviceCount() ;
      for ( PvUInt32 uiDevNum = 0 ; uiDevNum < uiCamCnt ; uiDevNum++ )
      {
        SaveCameraInfo( pInterf , uiDevNum , iEnumIndex , pPropertyFile ) ;
        iEnumIndex++ ;
      }
    }
    else
      ASSERT( 0 ) ;
  }
  m_iCamNum = m_CamerasOnBus = iEnumIndex ;

  TRACE( "\n   Pleora EnumCameras: Inspect properties of cam %d time is %g" ,
    iEnumIndex , GetHRTickCount() - dStart ) ;

  if ( pPropertyFile )
  fclose( pPropertyFile ) ;
  m_bSaveFullInfo = false ;
  m_bCamerasEnumerated = ( m_CamerasOnBus > 0 ) ;
  m_bRescanCameras = false ;
  m_CameraStatus = DriverInitialized ;

  return ( iEnumIndex != 0 ) ;
}

void PleoraCamera::ShutDown()
{
  if ( m_bCameraRunning )
    CameraStop() ;
  if ( m_bStreamProgrammed )
  {
    m_Pipeline.Stop() ;
    m_Pipeline.Reset() ;
    m_Stream.Close() ;
    m_bStreamProgrammed = false ;
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
  FxReleaseHandle( m_evFrameReady ) ;
  FxReleaseHandle( m_evBusChange ) ;
  C1394Camera::ShutDown() ;
}

void PleoraCamera::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  if ( !pParamFrame )
    return;


  CTextFrame* tf = pParamFrame->GetTextFrame( DEFAULT_LABEL );
  if ( tf )
  {
    FXParser pk = ( LPCTSTR )tf->GetString();
    FXString cmd;
    FXString param;
    int pos = 0;
    pk.GetWord( pos , cmd );

    FXAutolock al( m_SettingsLock , "AsyncTransaction" ) ;
    if ( cmd.CompareNoCase( "list" ) == 0 )
    {
      pk.Empty();
      for ( int i = 0; i < m_PropertiesEx.GetCount() ; i++ )
      {
        pk += m_PropertiesEx[ i ].m_Name ;
        pk += "\r\n" ;
      }
    }
    else if ( ( cmd.CompareNoCase( "get" ) == 0 ) && ( pk.GetWord( pos , cmd ) ) )
    {
      int iIndex = GetPropertyIndex( cmd );
      if ( iIndex != WRONG_PROPERTY )
      {
        int value;
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
            pk = ( LPCTSTR )value;
        }
        else
          pk = "error";
      }
      else
        pk = "error";
    }
    else if ( ( cmd.CompareNoCase( "set" ) == 0 ) && ( pk.GetWord( pos , cmd ) ) && ( pk.GetParamString( pos , param ) ) )
    {
      int iIndex = GetPropertyIndex( cmd );
      if ( iIndex != WRONG_PROPERTY )
      {
        int value = 0;
        bool bauto = false , Invalidate = false;
        if ( IsDigitField( m_PropertiesEx[ iIndex ].pr ) )
        {
          if ( param.CompareNoCase( "auto" ) == 0 )
            bauto = true;
          else
            value = atoi( param );
        }
        else
          value = ( int )( LPCTSTR )param;
        bool bWasStopped = m_bWasStopped ;
        m_bWasStopped = false ;
        bool bRes = SetCameraProperty( iIndex , value , bauto , Invalidate ) ;
        if ( !bWasStopped  && m_bWasStopped )
        {
          /*CamCNTLDoAndWait( CAM_EVT_INIT | CAM_EVT_START_GRAB )*/ ;
        }
        m_bWasStopped = bWasStopped ;
        pk = ( bRes ) ? "OK" : "error" ;
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

FXString PleoraCamera::GetCameraId( DWORD dwSerialNumber , DWORD& dwIndex )
{
  FXString Id ;
  for ( DWORD i = 0 ; i < m_CamerasOnBus ; i++ )
  {
    if ( m_CamInfo[ i ].serialnmb == dwSerialNumber )
    {
      DWORD dwBusyCamIndex = 0 ;

      FXAutolock al( m_ConfigLock ) ;
      for ( ; dwBusyCamIndex < ( DWORD )m_BusyCameras.GetCount() ; dwBusyCamIndex++ )
      {
        if ( m_BusyCameras[ dwBusyCamIndex ].m_dwSerialNumber == m_dwSerialNumber )
        {
          SEND_DEVICE_ERR( "\nCamera %u is busy." , m_dwSerialNumber );
          return Id ;
        }
      }
      if ( dwBusyCamIndex >= ( DWORD )m_BusyCameras.GetCount() )
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

bool PleoraCamera::GetCameraInfoBySN( DWORD dwSN , PvDeviceInfo ** pDevInfo )
{
  PvUInt32 uiInterfCnt = m_System.GetInterfaceCount() ;
  m_AvailableInterfaces.RemoveAll() ;
  int iEnumIndex = 0 ;
  for ( PvUInt32 i = 0 ; i < uiInterfCnt ; i++ )
  {
    PvInterface * pInterf = m_System.GetInterface( i ) ;
    if ( pInterf )
    {
      FXString InterfaceInfo ;
      InterfaceInfo.Format( "MAC=%s;IP=%s;Mask=%s" ,
        pInterf->GetMACAddress().GetAscii() ,
        pInterf->GetIPAddress().GetAscii() ,
        pInterf->GetSubnetMask().GetAscii() );
      FXString FileOut ;
      FileOut.Format( "Interface: %s\n" , ( LPCTSTR )InterfaceInfo ) ;
      TRACE( ( LPCTSTR )FileOut ) ;

      PvUInt32 uiCamCnt = pInterf->GetDeviceCount() ;
      for ( PvUInt32 uiDevNum = 0 ; uiDevNum < uiCamCnt ; uiDevNum++ )
      {
        double dCameraStart = GetHRTickCount() ;
        PvDeviceInfo * pInfo = pInterf->GetDeviceInfo( uiDevNum ) ;
        if ( pInfo )
        {
          FXString szSerNum( pInfo->GetSerialNumber().GetAscii() ) ;
          int iSNIndex = szSerNum.GetLength() ;
          LPCTSTR pStr = ( LPCTSTR )( szSerNum ) ;
          while ( iSNIndex > 0 && !isdigit( pStr[ iSNIndex ] ) )
            iSNIndex-- ;
          while ( iSNIndex > 0 && isdigit( pStr[ iSNIndex ] ) )
            iSNIndex-- ;
          if ( iSNIndex <= 0 )
            iSNIndex = 0 ;
          else
            iSNIndex++ ;

          int iSN = atoi( pStr + iSNIndex ) ;
          if ( ( DWORD )iSN == dwSN )
          {
            *pDevInfo = pInfo ;
            FXString CamInterfaceId ;
            CamInterfaceId.Format( "MAC=%s;IP=%s;Mask=%s" ,
              pInfo->GetMACAddress().GetAscii() ,
              pInfo->GetIPAddress().GetAscii() ,
              pInfo->GetSubnetMask().GetAscii() );
            TRACE( "GetCameraInfoBySN: Selected %s\n" , ( LPCTSTR )CamInterfaceId ) ;
            return true ;
          }
        }
      }
    }
  }
  return false ;
}

bool PleoraCamera::CameraInit()
{
  DriverInit();
  double dStart = GetHRTickCount() ;

  if ( m_dwConnectedSerialNumber && ( m_dwConnectedSerialNumber == m_dwSerialNumber )
    && !m_bShouldBeReprogrammed )
  {
    if ( m_Camera.IsConnected() ) // already connected to the same camera
      return true ;
  }

  if ( m_Camera.IsConnected() )
    CameraClose();

  if ( !m_CamerasOnBus )
  {  // nothing to connect
    SEND_DEVICE_ERR( "Fatal error: No Pleora cameras found on a bus." );
    return false;
  }
  if ( !IsSNLegal() )
  {  // nothing to connect
    SEND_DEVICE_ERR( "Fatal error: No selected camera." );
    return false;
  }

  if ( !GetCameraInfoBySN( m_dwSerialNumber , &m_pDeviceInfo ) )
  {
    SEND_DEVICE_ERR( "Fatal error: Can't find camera %d." , m_dwSerialNumber ) ;
    return false;
  }

  PvResult Result = m_Camera.Connect( m_pDeviceInfo , PvAccessControl ) ;
  if ( !Result.IsOK() )
  {
    SEND_DEVICE_ERR( "Can't connect to camera %d: %s" , 
      m_dwSerialNumber , Result.GetDescription().GetAscii() );
    return false;
  }
  FXString ModelName = m_pDeviceInfo->GetModel().GetAscii() ;
  m_CameraID.Format( "%d_%s" , m_dwSerialNumber , ( LPCTSTR )ModelName );
  m_GadgetInfo = m_CameraID ;
  if ( m_Status.IsEmpty() )
    m_Status = m_CameraID + " is connected" ;

  bool lEnabledValue = true;
  PvGenBoolean *lEnabled = m_Camera.GetGenLink()->GetBoolean( "AutoNegotiation" ) ;
  if ( lEnabled != NULL )
  {
    lEnabled->GetValue( lEnabledValue );
  }

  PvInt64 lUserPacketSizeValue = 1476;
  PvGenInteger *lUserPacketSize = m_Camera.GetGenLink()->GetInteger( "DefaultPacketSize" );
  if ( ( lUserPacketSize != NULL ) && lUserPacketSize->IsReadable() )
  {
    lUserPacketSize->GetValue( lUserPacketSizeValue );
  }

  if ( lEnabledValue )
  {
    // Perform automatic packet size negotiation
    Result = m_Camera.NegotiatePacketSize( 0 , 1476 );
  if ( !Result.IsOK() )
  {
      ::Sleep( 3000 );
    }
  }


//   for ( DWORD i = 0 ; i < pCamParams->GetCount() ; i++ )
//   {
//     pCamParams->Get( i )->RegisterEventSink( this ) ;
//   }
// 
//  Result = pCamParams->ExecuteCommand( "DeviceReset" ) ;

  SetCamPropertyData Value ;

  if ( !GetPropertyValue( "SensorWidth" , Value ) )
  {
    SEND_DEVICE_ERR( "Can't get sensor width %d" , m_dwSerialNumber );
    return false;
  }
  m_SensorSize.cx = ( LONG )Value.m_int ;
  Value.Reset() ;
  if ( !GetPropertyValue( "SensorHeight" , Value ) )
  {
    SEND_DEVICE_ERR( "Can't get sensor height %d" , m_dwSerialNumber );
    return false;
  }
  m_SensorSize.cy = ( LONG )Value.m_int ;
  Value.Reset() ;
  if ( !GetPropertyValue( "AcquisitionFrameRateRaw" , Value ) )
  {
//     SEND_DEVICE_ERR( "Can't get sensor height %d" , m_dwSerialNumber );
//     return false;
  }
  Value.m_int = 51 ;
  bool bRes = SetPropertyValue( "AcquisitionFrameRateRaw" , Value ) ;


  m_iNNoSerNumberErrors = 0 ;

  m_Status.Empty() ;

    SEND_DEVICE_INFO( m_Status );
    m_Status.Empty() ;

  m_dwConnectedSerialNumber = m_dwSerialNumber ;
  FXAutolock al( m_ConfigLock ) ;
  int i = 0 ;
  for ( ; i < m_BusyCameras.GetCount() ; i++ )
  {
    if ( m_BusyCameras[ i ].m_dwSerialNumber == m_dwSerialNumber )
      break ;
  }
  if ( i == m_BusyCameras.GetCount() )
  {
    BusyCamera NewBusyCamera( m_dwSerialNumber , this ) ;
    m_BusyCameras.Add( NewBusyCamera ) ;
  }
  m_szSerialNumber = m_pDeviceInfo->GetSerialNumber().GetAscii() ;
  double dConnectionTime = GetHRTickCount() ;
  TRACE( "\n    PleoraInitCamera: Camera connection time is %g" , dConnectionTime - dStart ) ;
  m_pOrigProperties = CommonProperties ;
  m_iNProperties = ARR_SIZE( CommonProperties ) ;

  for ( int i = 0 ; i < ARR_SIZE( KnownCameras ) ; i++ )
  {
    CamTypeAndProperties * pCamTP = KnownCameras[ i ] ;
    if ( !pCamTP )
      break ;
    if ( ModelName.Find( pCamTP->CamType ) == 0 )
      {
        m_pOrigProperties = pCamTP->Properties ;
        m_iNProperties = pCamTP->iNProperties ;
        break ;
      }
  } ;

  if ( !BuildPropertyList() )
  {
    CameraClose() ;
  }
  else
    m_bInitialized = true ;

  m_CurrentROI.left = GetXOffset() ;
  m_CurrentROI.top = GetYOffset() ;
  m_CurrentROI.right = GetXSize() ;
  m_CurrentROI.bottom = GetYSize() ;

//   ReprogramStream() ;
  TRACE( "\nPleoraCamera::CameraInit - Camera %u initialized in %g ms" ,
    m_CameraInfo.serialnmb , GetHRTickCount() - dStart ) ;
  return true;
}

void PleoraCamera::CameraClose()
{
  if ( !m_Camera.IsConnected() || m_dwConnectedSerialNumber == 0 || !IsSNLegal() )
  {
    m_CameraStatus = CameraAlreadyClosed ;
#ifdef Pleora_DEBUG
    TRACE( "\nCamera already closed SN=%u " , m_dwSerialNumber ) ;
#endif
    return ;
  }

  //   FXAutolock al( m_ConfigLock) ;
  m_Camera.Disconnect() ;
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
  m_dwConnectedSerialNumber = 0 ;
  m_CameraStatus = CameraClosed ;
  m_bInitialized = false ;

#ifdef Pleora_DEBUG
  TRACE( "\nCamera closed SN=%u Index=%d" , m_dwSerialNumber , i) ;
#endif
}

bool PleoraCamera::DriverValid()
{
  return  ( m_CamerasOnBus != 0 ) /*&& (m_pCamera && m_Camera.IsConnected())*/;
}

bool PleoraCamera::ReprogramStream()
{
  if ( IsSNLegal() && !m_bInScanProperties )
{
    PvGenParameterArray * pCamParams = m_Camera.GetGenParameters();
    if ( !pCamParams )
  {
      SEND_DEVICE_ERR( "PleoraCamera::ReprogramStream - Can't get camera parameters" );
      TRACE( "\n!!!!!!!PleoraCamera::ReprogramStream - Can't get camera parameters" ) ;
      return false ;
    }
    PvGenParameterArray * pStreamParams = m_Stream.GetParameters();
    if ( !pCamParams )
    {
      SEND_DEVICE_ERR( "PleoraCamera::ReprogramStream - Can't get stream parameters" );
      TRACE( "\n!!!!!!!PleoraCamera::ReprogramStream - Can't get stream parameters" ) ;
      return false ;
    }
    PvResult Result = pStreamParams->ExecuteCommand( "Reset" );
    if ( !Result.IsOK() )
    {
      SEND_DEVICE_ERR( "PleoraCamera::ReprogramStream - "
        "Can't reset Stream\n (%s)" ,
        Result.GetDescription().GetAscii() );
      TRACE( "\n!!!!!!!PleoraCamera::ReprogramStream - "
        "Can't reset Stream\n (%s)" ,
        Result.GetDescription().GetAscii() ) ;
      return false ;
    }

    Result = m_Stream.Open( m_pDeviceInfo->GetIPAddress() ) ;
    if ( !Result.IsOK() )
    {
      SENDERR( "GrabLoop: Can't open stream for %s: %s" ,
        m_pDeviceInfo->GetIPAddress().GetAscii() , Result.GetDescription().GetAscii() ) ;
      return false ;
    }

    bool bRes = m_Camera.IsConnected() ;
    Result = m_Camera.SetStreamDestination(
      m_Stream.GetLocalIPAddress() , m_Stream.GetLocalPort() );
    if ( !Result.IsOK() )
    {
      SENDERR( "PleoraCamera::ReprogramStream - "
        "Can't set stream destination for %s(%d): %s" ,
        m_Stream.GetLocalIPAddress().GetAscii() , m_Stream.GetLocalPort() ,
        Result.GetDescription().GetAscii() ) ;
      TRACE( "\n!!!!!!!PleoraCamera::ReprogramStream - "
        "Can't set stream destination for %s(%d): %s" ,
        m_Stream.GetLocalIPAddress().GetAscii() , m_Stream.GetLocalPort() ,
        Result.GetDescription().GetAscii() ) ;
      return false ;
    }

    TRACE( "%f Camera Start Frame Size %lu" , GetHRTickCount() , m_i64Payload ) ;


    // Try reading payload size from device
    PvInt64 i64PayloadSizeValue = 0;
    Result = PvResult::Code::NOT_INITIALIZED;
    Result = pCamParams->GetIntegerValue( "PayloadSize" , i64PayloadSizeValue );
  if ( !Result.IsOK() )
  {
      SEND_DEVICE_ERR( "PleoraCamera::ReprogramStream - Can't get Payload (%s)" ,
        Result.GetDescription().GetAscii() );
      TRACE( "\n!!!!!!!PleoraCamera::ReprogramStream - Can't get Payload (%s)" ,
        Result.GetDescription().GetAscii() ) ;
    return false ;
  }
    m_i64Payload = i64PayloadSizeValue ;

    // Use min of BUFFER_COUNT and how many buffers can be queued in PvStream


    // If payload size is valid, force buffers re-alloc - better than 
    // adjusting as images are coming in
    if ( m_i64Payload > 0 )
    {
      if ( m_Stream.IsOpen() )
  {
      m_Pipeline.SetBufferSize( static_cast< PvUInt32 >( m_i64Payload ) );

      // Never hurts to start streaming on a fresh pipeline/stream...
      //  Result = m_Pipeline.Reset();
//         if ( !Result.IsOK() )
//         {
//           SEND_DEVICE_ERR( "PleoraCamera::ReprogramStream - "
//             "Can't reset Pipeline\n (%s)" ,
//             Result.GetDescription().GetAscii() );
//           TRACE( "\n!!!!!!!PleoraCamera::ReprogramStream - "
//             "Can't reset Pipeline\n (%s)" ,
//             Result.GetDescription().GetAscii() ) ;
//           return false ;
//         }
        Result = m_Pipeline.SetBufferCount( 16 ) ;
  if ( !Result.IsOK() )
  {
        SEND_DEVICE_ERR( "PleoraCamera::ReprogramStream - "
          "Can't set buffer count for Pipeline\n (%s)" ,
          Result.GetDescription().GetAscii() );
        TRACE( "\n!!!!!!!PleoraCamera::ReprogramStream - "
          "Can't set buffer count for Pipeline\n (%s)" ,
          Result.GetDescription().GetAscii() ) ;
    return false ;
  }

        Result = m_Pipeline.Start() ;
  if ( !Result.IsOK() )
  {
        SEND_DEVICE_ERR( "PleoraCamera::ReprogramStream - "
            "Can't start Pipeline\n (%s)" ,
          Result.GetDescription().GetAscii() );
        TRACE( "\n!!!!!!!PleoraCamera::ReprogramStream - "
            "Can't start Pipeline\n (%s)" ,
          Result.GetDescription().GetAscii() ) ;
    return false ;
  }
  }

      int iCurrentBufferCount = m_Pipeline.GetBufferCount() ;

      // Reset stream statistics
      if ( !m_bGrabLoopContinue )
  {
        FXString ThreadName ;
        m_hGrabThreadHandle = CreateThread( NULL , 0 ,
          CameraGrabLoop , this , CREATE_SUSPENDED , &m_dwGrabThreadId ) ;
        if ( m_hGrabThreadHandle )
        {
          FXString ThreadName ;
          GetGadgetName( ThreadName ) ;
          ThreadName += "_Grabbing" ;
          ::SetThreadName( ( LPCSTR )ThreadName , m_dwGrabThreadId ) ;
          ResumeThread( m_hGrabThreadHandle ) ;
          Sleep( 50 ) ;
  }
        else
  {
          C1394_SENDERR_2( "%s: %s" , ( LPCTSTR )ThreadName , _T( "Can't start grab thread" ) );
          m_dwGrabThreadId = 0 ;
    return false ;
  }
      }
    }
    m_bStreamProgrammed = true ;
  return true ;
}
  return false ;
}

bool PleoraCamera::CameraStart()
{
  if ( !m_Camera.IsConnected() )
  {
    if ( !CameraInit() )
  {
#ifdef Pleora_DEBUG
      TRACE( "\nPleoraCamera::CameraStart: Camera is not initialized SN=%u" , m_dwSerialNumber ) ;
#endif
      return false ;
    }
  }
    if ( !ReprogramStream() )
      return false ;
  
  PvGenParameterArray * pCamParams = m_Camera.GetGenParameters() ;
  PvGenParameterArray * pStreamParams = m_Camera.GetGenParameters() ;
  if ( !pCamParams || !pStreamParams )
    return false ;
  PvResult Result ;
//   Result = pStreamParams->ExecuteCommand( "Reset" ) ;
//   if ( !Result.IsOK() )
//   {
//     SEND_DEVICE_ERR( "PleoraCamera::CameraStart - Can't Reset Stream (%s)" ,
//       Result.GetDescription().GetAscii() );
//     TRACE( "\n!!!!!!!PleoraCamera::CameraStart - Can't Reset Stream (%s)" ,
//       Result.GetDescription().GetAscii() ) ;
//   }

  PvString lStr;
  Result = pCamParams->GetEnumValue( "AcquisitionMode" , lStr );
  if ( !Result.IsOK() )
  {
    SEND_DEVICE_ERR( "PleoraCamera::CameraStart - Can't get AcquisitionMode (%s)" ,
      Result.GetDescription().GetAscii() );
    TRACE( "\n!!!!!!!PleoraCamera::CameraStart - Can't get AcquisitionMode (%s)" ,
      Result.GetDescription().GetAscii() ) ;
    return false;
  }
  FXString lModeStr = lStr.GetAscii();
  if ( lModeStr.Find( _T( "Continuous" ) ) >= 0 )
  {
    // We are streaming, lock the TL parameters
    Result = pCamParams->SetIntegerValue( "TLParamsLocked" , 1 );
    if ( !Result.IsOK() )
    {
      SEND_DEVICE_ERR( "PleoraCamera::CameraStart - Can't set TLParamsLocked to 1 (%s)" ,
        Result.GetDescription().GetAscii() );
      TRACE( "\n!!!!!!!PleoraCamera::CameraStart - Can't set TLParamsLocked to 1 (%s)" ,
        Result.GetDescription().GetAscii() ) ;
      return false;
    }
    Result = pCamParams->ExecuteCommand( "AcquisitionStart" ) ;
    if ( !Result.IsOK() )
    {
      SEND_DEVICE_ERR( "PleoraCamera::CameraStart - Can't execute command AcquisitionStart (%s)" ,
        Result.GetDescription().GetAscii() );
      TRACE( "\n!!!!!!!PleoraCamera::CameraStart - Can't execute command AcquisitionStart (%s)" ,
        Result.GetDescription().GetAscii() ) ;
      return false;
    }
    m_bGrabLoopContinue = true;
  }
  else if ( ( lModeStr.Find( _T( "Multi" ) ) >= 0 ) ||
    ( lModeStr.Find( _T( "Single" ) ) >= 0 ) )
  {
    pCamParams->SetIntegerValue( "TLParamsLocked" , 1 );
    Result = pCamParams->ExecuteCommand( "AcquisitionStart" );
    if ( !Result.IsOK() )
    {
      SEND_DEVICE_ERR( "PleoraCamera::CameraStart - Can't execute command AcquisitionStart (%s)" ,
        Result.GetDescription().GetAscii() );
      TRACE( "\n!!!!!!!PleoraCamera::CameraStart - Can't execute command AcquisitionStart (%s)" ,
        Result.GetDescription().GetAscii() ) ;
      return false;
    }
    pCamParams->SetIntegerValue( "TLParamsLocked" , 0 );
  }
  m_bCameraRunning = true ;
  return true ;
  }
void PleoraCamera::CameraStop()
{
  PvGenParameterArray * pCamParams = m_Camera.GetGenParameters() ;
  pCamParams->ExecuteCommand( "AcquisitionStop" );
  pCamParams->SetIntegerValue( "TLParamsLocked" , 0 );

  m_bGrabLoopContinue = false ;
  if ( m_Pipeline.IsStarted() )
  {
    m_Pipeline.Stop();
    Sleep(100) ;
//     m_Pipeline.Reset() ;
    m_Stream.Close() ;
    m_bStreamProgrammed = false ;
  }

  m_CameraStatus = CameraStopped ;
  m_bCameraRunning = false ;
  if ( m_hGrabThreadHandle )
  {
    DWORD dwRes = WaitForSingleObject( m_hGrabThreadHandle , 2000 ) ;
    ASSERT( dwRes == WAIT_OBJECT_0 ) ;
    ASSERT( m_dwGrabThreadId == 0 ) ;
  SetEvent( m_evFrameReady ) ;
    m_hGrabThreadHandle = NULL ;
  }
  TRACE( "%f PleoraCamera::CameraStop() for #%u\n " , GetHRTickCount() , m_dwSerialNumber );
}

CVideoFrame* PleoraCamera::CameraDoGrab( double* StartTime )
{
  CVideoFrame * pOut = NULL ;
  m_dLastStartTime = ::GetHRTickCount() ;
  *StartTime = m_dLastStartTime - m_dLastInCallbackTime ;
  if ( m_ReadyFrames.size() )
  {
    m_GrabLock.Lock() ;
    pOut = m_ReadyFrames.front() ;
    m_ReadyFrames.pop() ;
    m_GrabLock.Unlock() ;
  }
  else
    Sleep( 1 ) ;
  *StartTime -= m_dLastInCallbackTime ;
  return pOut ;
}

bool PleoraCamera::BuildPropertyList()
{
  CAMERA1394::Property P;
  if ( !m_Camera.IsConnected() )
    return false ;

  if ( GetHRTickCount() - m_dLastBuiltPropertyTime < 1000. )
    return true ;

  double dStart = GetHRTickCount() ;
  m_Properties.RemoveAll();
  m_PropertiesEx.RemoveAll();
  int iWhiteBalanceMode = GetColorMode() ; // 0 - no color, 1 - by program, 2 - by camera


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

  if ( !m_bInScanProperties && !m_SettingsLock.Lock( 1000 , _T( "BuildPropertyList" ) ) )
  {
    SENDERR( "BuildPropertyList: Can't lock settings for CAM $d " , m_dwSerialNumber ) ;
    return false ;
  };
  PvGenParameterArray * pCamParams = m_Camera.GetGenParameters() ;
  if ( !pCamParams )
  {
    SENDERR( "BuildPropertyList: Can't get camera $d parameters" , m_dwSerialNumber ) ;
    return false ;
  }
  m_nSensorWidth = m_SensorSize.cx = GetMaxWidth() ;
  m_nSensorHeight = m_SensorSize.cy = GetMaxHeight() ;
  PvResult Result ;
  SetCamPropertyData Value ;
  PvGenParameter * pParam = pCamParams->Get( "AcquisitionFrameRateRaw" ) ;
  if ( pParam )
  {
    Result = pParam->GetType( Value.m_Type ) ;
    if ( Result.IsOK() )
    {
      if ( Value.m_Type == PvGenTypeInteger )
      {
        PvGenInteger * pInt = pCamParams->GetInteger( "AcquisitionFrameRateRaw" ) ;
        if ( pInt )
        {
          PvInt64 i64Val = 0L ;
          Result = pInt->GetValue( i64Val ) ;
          int iVal = ( int )i64Val ;
        }
      }

    }
  }
  Value.Reset() ;
  bool bRes = GetPropertyValue( "PixelFormat" , Value ) ;
  PvInt64 i64PayloadSizeValue = 0 ;
  Result = pCamParams->GetIntegerValue( "PayloadSize" , i64PayloadSizeValue );
  bool bInputMode = false ;
  bool bOutputMode = false ;
  bool bTriggerMode = false ;
  m_TriggerMode = TrigNotSupported ;

  for ( int i = 0 ; i < m_iNProperties ; i++ )
  {
    FXString DlgFormat ;
    const CamProperty& Prop = m_pOrigProperties[ i ] ;
    CameraAttribute NewAttribute( Prop.pr , Prop.name , Prop.CamPropertyName ) ;
    if ( Prop.CamPropertyName /*&& bOutputLineProcessPass*/ )
    {
      if ( Prop.pr == FGP_USER_SET_SELECT )
      {
        DlgFormat.Format( "ComboBox(%s(NoOper(0),Save(1),Load(2)))" , Prop.name ) ;
        m_SaveSettingsCommand = Prop.CamPropertyName ;
        m_LoadSettingsCommand = Prop.AutoControl ;
        NewAttribute.m_Type = PvGenTypeInteger ;
      }
      else if ( Prop.pr == FGP_USER_SET_DEF )
      {
        m_UserSetSelector = Prop.name ;
        m_SetDefaultSettings = Prop.CamPropertyName ;
        m_bSetDefaultSettingsIsEnum = ( Prop.AutoControl != NULL ) ;
      }
      else
      {
        SetCamPropertyData Value ;
        bRes = GetPropertyValue( Prop.CamPropertyName , Value ) ;
        if ( bRes )
                {
          NewAttribute.m_Type = Value.m_Type ;

            switch ( NewAttribute.m_Type )
            {
              case PvGenTypeInteger:
          {
            NewAttribute.m_i64Val = Value.m_int64 ;
            NewAttribute.m_intVal = Value.m_int ;
            PvGenInteger * pInt = pCamParams->GetInteger( Prop.CamPropertyName ) ;
            if ( pInt )
            {
              Result = pInt->SetValue( Value.m_int64 ) ;
              Value.m_pParameter = pInt ;
              PvInt64 Min = 0L ;
              PvInt64 Max = 0L ;
              PvInt64 Increment = 0L ;
              Result = pInt->GetIncrement( Increment ) ;
              PvGenRepresentation Repr ;
              Result = pInt->GetRepresentation( Repr ) ;
              Result = pInt->GetMax( Max ) ;
              Result = pInt->GetMin( Min ) ;
              if ( Max == 0L )
                Max = 19997L ;
              NewAttribute.m_i64Range[ 0 ] = Min ;
              NewAttribute.m_i64Range[ 1 ] = Max ;
              NewAttribute.m_iRange[ 0 ] = ( int )Min;
              NewAttribute.m_iRange[ 1 ] = ( int )Max ;
                DlgFormat.Format( ( Prop.AutoControl ? 
                  "Spin&Bool(%s,%i,%i)" : "Spin(%s,%i,%i)" ) , Prop.name ,
                NewAttribute.m_iRange[ 0 ] , NewAttribute.m_iRange[ 1 ] ) ;
            }
          }
                break;
              case PvGenTypeFloat:
                if ( ( Prop.pr == FGP_TRIGGERDELAY  && !bTriggerMode )
                  || ( Prop.pr == FGP_LINE_DEBOUNCE  && !bInputMode ) )
                  break ;
            Value.m_pParameter = pCamParams->GetFloat( Prop.CamPropertyName ) ;
            ( ( PvGenFloat* )( Value.m_pParameter ) )->GetUnit( NewAttribute.m_Unit ) ;
            ( ( PvGenFloat* )( Value.m_pParameter ) )->GetMin( NewAttribute.m_dRange[ 0 ] ) ;
            ( ( PvGenFloat* )( Value.m_pParameter ) )->GetMax( NewAttribute.m_dRange[ 1 ] ) ;
            ( ( PvGenFloat* )( Value.m_pParameter ) )->GetValue( NewAttribute.m_dVal ) ;
                switch ( Prop.pr )
                {
                  case FGP_OUT_DELAY:
                    DlgFormat.Format( "Spin(%s,%i,%i)" , Prop.name ,
                ROUND( NewAttribute.m_dRange[ 0 ] ) , ROUND( NewAttribute.m_dRange[ 1 ] ) ) ;
                    NewAttribute.m_i64Val = ROUND( NewAttribute.m_dVal ) ;
                    break ;
                  case FGP_EXTSHUTTER:
                    DlgFormat.Format( "Spin&Bool(%s,%i,%i)" , Prop.name ,
                ROUND( NewAttribute.m_dRange[ 0 ] ) , ROUND( NewAttribute.m_dRange[ 1 ] ) ) ;
                    NewAttribute.m_i64Val = ROUND( NewAttribute.m_dVal ) ;
                    break ;
                  case FGP_GAIN:
                    DlgFormat.Format( "Spin&Bool(%s,%i,%i)" , Prop.name ,
                ROUND( NewAttribute.m_dRange[ 0 ] * 10. ) , ROUND( NewAttribute.m_dRange[ 1 ] * 10. ) ) ;
                    NewAttribute.m_i64Val = ROUND( NewAttribute.m_dVal ) ;
                    break ;
              //                 case FGP_WHITEBAL_RATIO:
              //                   DlgFormat.Format( "Spin&Bool(%s,%i,%i)" , Prop.name ,
              //                     ( int )( NewAttribute.m_dRange[ 0 ] * 100. ) , ( int )( NewAttribute.m_dRange[ 1 ] * 100. ) ) ;
              //                   NewAttribute.m_i64Val = ROUND( NewAttribute.m_dVal * 100. ) ;
              //                   break ;
                  case FGP_FRAME_RATE:
                  {
                    DlgFormat.Format( "Spin(%s,%i,%i)" , Prop.name ,
                ROUND( NewAttribute.m_dRange[ 0 ] * 10. ) , ROUND( NewAttribute.m_dRange[ 1 ] * 10. ) ) ;
                    NewAttribute.m_i64Val = ROUND( NewAttribute.m_dVal * 10. ) ;
                    m_FPSPropertyName = NewAttribute.m_CameraPropertyName ;
                  }
                  break ;
                  break ;
                  case FGP_TEMPERATURE_S:
                  {
                    DlgFormat.Format( "EditBox(%s)" , Prop.name ) ;
                  }
                  break ;
                  default:
                    DlgFormat.Format( "EditBox(%s)" , Prop.name ) ;
                    break ;
                }
                break;
              case PvGenTypeEnum :
              {
                bool bExists = true ;
                switch ( Prop.pr )
                {
                  case FGP_TRIGGERDELAY:
                  case FGP_LINE_SELECT:
                  case FGP_TRIGGER_POLARITY: bExists = ( m_TriggerMode == TriggerOn ) ; break ;
                  case FGP_LINE_SOURCE: bExists = bOutputMode ; break ;
                  case FGP_LINE_DEBOUNCE: bExists = bInputMode ; break ;
                }
                if ( !bExists )
                  break ;
            NewAttribute.m_enumVal = Value.m_szString ;
            Value.m_pParameter = pCamParams->GetEnum( Prop.CamPropertyName ) ;

            PvGenEnum * pEnum = ( PvGenEnum* )( Value.m_pParameter ) ;

                NewAttribute.m_EnumRange.RemoveAll() ;
                DlgFormat.Format( "ComboBox(%s(" , Prop.name ) ;
                switch ( Prop.pr )
                {
                  case FGP_TRIGGER_SOURCE:
                    m_TriggerSourceEnums.Format( _T( "%s(0)," ) , m_TriggerOff ) ;
                    DlgFormat += m_TriggerSourceEnums ;
                    NewAttribute.m_EnumRange.Add( m_TriggerOff ) ;
                    break ;
                }
                PvInt64 i64EnumCnt ;
            m_Result = pEnum->GetEntriesCount( i64EnumCnt ) ;
                for ( PvInt64 j = 0 ; j < i64EnumCnt ; j++ )
                {
                  const PvGenEnumEntry * pEnumEntry ;
              Result = pEnum->GetEntryByIndex( j , &pEnumEntry ) ;
                  if ( Result.IsOK() )
                  {
                    PvString Enum ;
                    pEnumEntry->GetName( Enum ) ;
                    FXString Addition ;
                Addition.Format( "%s(%d)%c" , Enum.GetAscii() , ( int )j ,
                      ( j == i64EnumCnt - 1 ) ? ')' : ',' ) ;
                DlgFormat += Addition ;
                NewAttribute.m_EnumRange.Add( Enum.GetAscii() ) ;
                  }
                  else
                  {
                    ASSERT( 0 ) ;
                  }
                }
            DlgFormat += ')' ;

            if ( Prop.pr == FGP_SYNC_SIGNAL )
            {
              m_iSyncSignalMode = ( NewAttribute.m_enumVal == "ARM" ) ? 0 : 1 ;
            }
            else if ( Prop.pr == FGP_TRIGGER_SOURCE )
                {
                  int iLast = NewAttribute.m_EnumRange.GetUpperBound() ;
                  for ( int i = 0 ; i < NewAttribute.m_EnumRange.GetCount() ; i++ )
                  {
                    //m_TriggerSourceEnums += NewAttribute.m_EnumRange[i] ;
                    FXString ItemIndex ;
                    ItemIndex.Format( "(%d)%s" , i + 1 , ( i < iLast ) ? "," : "" ) ;
                    m_TriggerSourceEnums += ItemIndex ;
                  }
                  m_SelectedTriggerSource = ( m_TriggerMode == TriggerOff ) ? m_TriggerOff : NewAttribute.m_enumVal ;
                  //DlgFormat.Empty() ; // this property will be included into FGP_TRIGGERONOFF
                  m_TriggerSourceName = Prop.CamPropertyName ;
                }
                //Prop.m_EnumerateNames.Copy( NewAttribute.m_EnumRange ) ;
                if ( Prop.pr == FGP_LINE_SELECT )
                {
              pEnum->SetValue( PvString( NewAttribute.m_enumVal ) ) ;
                  m_SelectedLine = NewAttribute.m_enumVal ;

                  FXString LineParams ;
                  SetCamPropertyData Data ;
                  m_bViewErrorMessagesOnGetSet = false ;
              if ( GetPropertyValue( pCamParams , "LineMode" , Data ) )
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
                  bTriggerMode = ( NewAttribute.m_enumVal != m_TriggerOff ) ;// != Off  or  No
                  m_TriggerMode = ( bTriggerMode ) ? TriggerOn : TriggerOff ;
                  m_TriggerOn = NewAttribute.m_EnumRange[ 1 ] ;
                  m_TriggerModeName = Prop.CamPropertyName ;
                  DlgFormat.Empty() ;
                }
              }
              break;
              case PvGenTypeString :
              {
            NewAttribute.m_stringVal = Value.m_szString ;
                DlgFormat.Format( "EditBox(%s)" , Prop.name ) ;
              }
              break;
              case PvGenTypeBoolean :
            NewAttribute.m_boolVal = Value.m_bBool ;
                DlgFormat.Format( "ComboBox(%s(false(0),true(1)))" , Prop.name ) ;
                break;
              default:
                ASSERT( 0 ) ;
                break ;
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
          //Prop.m_DataType = PvGenTypeString  ;
          break;
        case FGP_LOG:
          DlgFormat = _T( "EditBox(Log)" ) ;
          //Prop.m_DataType = PvGenTypeString  ;
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
      NewAttribute.m_DlgFormat = ( LPCTSTR )DlgFormat ;
      //       m_Properties.Add( NewProperty ) ;
      m_PropertiesEx.Add( NewAttribute ) ;
      //       Prop.m_bSupported = true ;
    }
    //     else
    //       Prop.m_bSupported = false ;
  }
  if ( !m_bInScanProperties )
    m_SettingsLock.Unlock() ;

  m_CameraStatus = PropertyListBuilt ;
  m_dLastBuiltPropertyTime = GetHRTickCount() ;
  TRACE( "\n    AVTBuildPropertyList: list built time is %g" , m_dLastBuiltPropertyTime - dStart ) ;
  //m_Camera.Close() ;
  return true;
}

bool PleoraCamera::GetCameraProperty( unsigned uiProp , int &value , bool& bauto )
{
  if ( !CheckAndAllocCamera() )
    return false;
  UINT i = GetPropertyIndex( ( FG_PARAMETER )uiProp ) ;
  CameraAttribute& Prop = m_PropertiesEx.GetAt( i ) ;

  SetCamPropertyData PropData ;
  LPCTSTR pName = ( LPCTSTR ) m_PropertiesEx[ i ].m_CameraPropertyName ;
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
      value = ( int )( LPCTSTR )sROI;
      return true;
    }
    case FGP_LOG:
      value = ( int )( LPCTSTR )m_PropertiesForLogAsString ;
      return true ;

    default:
    {
      SetCamPropertyData PropData ;
      if ( GetCameraPropertyEx( pName , PropData ) )
      {
        if ( Prop.m_AutoControl && PropData.m_bAuto )
          bauto = PropData.m_bBool ;
        else
          bauto = false ;
        switch ( Prop.m_Type )
        {
          case PvGenTypeInteger:  value = PropData.m_int ; return true ;
          case PvGenTypeFloat:
          {
            switch ( Prop.pr )
            {
              case FGP_OUT_DELAY:
              case FGP_EXTSHUTTER: m_dExtShutter = value = ROUND( PropData.m_double ) ; break ;
              case FGP_GAIN:       value = ROUND( PropData.m_double * 10. ) ; break ;
              case FGP_FRAME_RATE:
                if ( m_iFPSx10 == 0 )
                  m_iFPSx10 = value = ROUND( PropData.m_double * 10. ) ;
                else
                  value = m_iFPSx10 ;
                break ;
              default:
                m_TmpString.Format( "%g" , PropData.m_double ) ;
                value = ( int )( LPCTSTR )m_TmpString ;
                break ;
            }
          }
          return true ;
          case PvGenTypeEnum :
          {
            if ( Prop.pr == FGP_LINE_SELECT )
              TRACE( "\nGetProperty %s = %s, Invalidate = %d" ,
              ( LPCTSTR )Prop.m_Name , PropData.m_szString , ( int )PropData.m_bInvalidate ) ;

            for ( int iEnum = 0 ; iEnum < Prop.m_EnumRange.GetCount() ; iEnum++ )
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
          case PvGenTypeString :
          {
            m_TmpString = PropData.m_szString ;
            value = ( int )( LPCTSTR )m_TmpString ;
            return true ;
          }
          case PvGenTypeBoolean :
          {
            value = ( int )PropData.m_bBool ;
            return true ;
          }
        }
      }
    }
  }
  bauto = false;
  return false ;
}
bool PleoraCamera::SetCameraProperty( unsigned iIndex , int &value , bool& bauto , bool& Invalidate )
{
  if ( !CheckAndAllocCamera() )
    return false;

  bool bRes = false ;
  SetCamPropertyData& Data = m_PropertyData ;
  CameraAttribute& pProp = m_PropertiesEx.GetAt( iIndex ) ;
  strcpy( Data.m_Name , pProp.m_CameraPropertyName ) ;
  strcpy( m_TmpPropertyName , Data.m_Name ) ;
  if ( !pProp.m_AutoControl.IsEmpty() )
  {
    Data.m_bAuto = true ;
    Data.m_bBool = bauto ;
  }
  else
    Data.m_bAuto = false ;
  switch ( pProp.m_Type )
  {
    case PvGenTypeInteger:
    {
      Data.m_int = value ;
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
    case PvGenTypeFloat:
    {
      double dValue ;
      switch ( pProp.pr )
      {
        case FGP_OUT_DELAY:
        case FGP_EXTSHUTTER: dValue = ( double )( value ) ; break ;
        case FGP_GAIN:       dValue = ( double )value / 10. ; break ;
        case FGP_FRAME_RATE:
          dValue = ( double )value / 10. ;
          m_iFPSx10 = value ;
          break ;
        case FGP_WHITEBAL_RATIO: dValue = ( double )value / 100. ; break ;
        default: dValue = atof( ( LPCTSTR )value ) ; break ;
      }
      Data.m_double = dValue ;
      break ;
    }
    case PvGenTypeEnum :
    {
      if ( ( 0 <= value ) && ( ( int )value < pProp.m_EnumRange.GetCount() ) )
        strcpy_s( Data.m_szString , ( LPCTSTR )pProp.m_EnumRange[ ( int )value ] ) ;
      else
      {
        SENDERR( "Set Property %s bad enum index %d[0,%d]" , pProp.m_Name ,
          value , pProp.m_EnumRange.GetCount() ) ;
        return false ;
      }
      pProp.m_stringVal = Data.m_szString  ;
      if ( pProp.pr == FGP_LINE_SELECT )
        TRACE( "\nBefore SetPropertyEx %s = %s, Invalidate = %d" ,
        ( LPCTSTR )pProp.m_Name , Data.m_szString , ( int )Data.m_bInvalidate ) ;
      break ;
    }
    case PvGenTypeString :
    {
      strcpy_s( Data.m_szString , ( LPCTSTR )value ) ;
      pProp.m_stringVal = Data.m_szString ;
      break ; ;
    }
    break;
    case PvGenTypeBoolean :
    {
      Data.m_bBool = ( value != NULL ) ;
      pProp.m_boolVal = Data.m_bBool ;
      break ;
    }
    default:
      if ( pProp.pr == FGP_ROI )
      {
        strcpy_s( Data.m_szString , ( LPCTSTR )value ) ;
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
  if ( IsRunning() && m_bCameraRunning )
  {
    for ( int i = 0 ; i < ARR_SIZE( GrabStopProperties ) ; i++ )
    {
      if ( IsStopNecessary( pProp.pr ) )
      {
        CameraStop() ;
          m_bShouldBeReprogrammed = m_bWasStopped = true ;
          break ;
        }
    }
  }
//   if ( pProp.pr == FGP_LOG )
//   {
//     int iPos = 0 ;
//     m_LogLock.Lock() ;
//     m_PropertiesForLogAsString = ( LPCTSTR )value ;
//     FXString Token = m_PropertiesForLogAsString.Tokenize( _T( ", \t" ) , iPos ) ;
//     if ( Token.IsEmpty() )
//     {
//       m_dLogPeriod_ms = 0. ;
//     }
//     else
//     {
//       double dPeriod = atof( Token ) ;
//       if ( dPeriod <= 0.0 )
//         m_dLogPeriod_ms = 0. ;
//       else
//       {
//         m_dLogPeriod_ms = dPeriod ;
//         m_PropertiesForLog.RemoveAll() ;
//         while ( !( Token = m_PropertiesForLogAsString.Tokenize( _T( ", \t" ) , iPos ) ).IsEmpty() )
//           m_PropertiesForLog.Add( Token ) ;
//       }
//       m_LogOutString = m_PropertiesForLogAsString + _T( "\n" ) ;
//       m_iLogCnt = 0 ;
//       m_BusEvents |= CAM_EVT_LOG ;
//       SetEvent( m_evCameraControl ) ;
//     }
//     m_LogLock.Unlock() ;
//     bRes = true ;
//   }
//   else
//    bRes = OtherThreadSetProperty( iIndex , &Data , &Invalidate ) ;
  bRes = SetCameraPropertyEx( iIndex , &Data , Invalidate ) ;
  if ( bRes )
  {
    if ( pProp.pr == FGP_LINE_SELECT
      || pProp.pr == FGP_LINEIN_SELECT
      || pProp.pr == FGP_LINEOUT_SELECT )
    {
      SetCamPropertyData DataAfter ;
      TRACE( "\nAfter SetPropertyEx %s = %s, Invalidate = %d" ,
        ( LPCTSTR )pProp.m_Name , Data.m_szString , ( int )Data.m_bInvalidate ) ;
      PvGenParameterArray * pCamParams = m_Camera.GetGenParameters() ;
      if ( GetPropertyValue( ( LPCTSTR )pProp.m_CameraPropertyName , DataAfter ) )
      {
        TRACE( "\nReal Value for %s = %s" , ( LPCTSTR )pProp.m_Name , DataAfter.m_szString ) ;
      }
      else
        ASSERT( 0 ) ;
      m_iSelectedLineNumber = ( int )value ;
      Invalidate = true ;
    }
  }

  return bRes ;

}


bool PleoraCamera::PleoraSaveOrLoadSettings( int iMode )  // 0 - nothing to do, 1 - Save, 2 - Load
{
  if ( !m_Camera.IsConnected() )
    return false ;
  PvGenParameterArray * pCamParams = m_Camera.GetGenParameters() ;
  if ( !pCamParams )
    return false ;
  if ( iMode == 0 )
    return true ;
  if ( iMode < 0 || iMode > 2 )
    return false ;

  PvGenParameter*  PropPtr = pCamParams->Get( PvString( m_UserSetSelector ) ) ;
  if ( !PropPtr )
  {
    SENDERR( "PleoraSaveOrLoadSettings - Can't get property by name %s" ,
      ( LPCTSTR )m_UserSetSelector ) ;
    TRACE( "\nPleoraSaveOrLoadSettings - Can't get property by name %s" ,
      ( LPCTSTR )m_UserSetSelector ) ;
    return false ;
  }
//   Result = PropPtr->SetValue( 1 ) ; //allways set 1
//   if ( !Result.IsOK() )
//   {
//     SENDERR( "\nSetCameraPropertyEx - Can't set value %s to 1: %s" ,
//       ( LPCTSTR )m_UserSetSelector , Result.GetDescription().GetAscii() ) ;
//     TRACE( "\nSetCameraPropertyEx - Can't set value %s to 1: %s" ,
//       ( LPCTSTR )m_UserSetSelector , Result.GetDescription().GetAscii() ) ;
//     return false ;
//   }
// 
//   PvGenParameter*  pCommandFeature;
//   FXString Command = ( iMode == 1 ) ? m_SaveSettingsCommand : m_LoadSettingsCommand ;
  //   Result = pCamParams->Get(
//     ( LPCTSTR )Command , pCommandFeature ) ;
//   if ( !Result.IsOK() )
//   {
//     SENDERR( "\nSetCameraPropertyEx - Can't get command by name %s: %s" ,
//       ( LPCTSTR )Command , Result.GetDescription().GetAscii() ) ;
//     TRACE( "\nSetCameraPropertyEx - Can't get command by name %s: %s" ,
//       ( LPCTSTR )Command , Result.GetDescription().GetAscii() ) ;
//     return false ;
//   }
//   Result = pCommandFeature->RunCommand() ;
//   if ( !Result.IsOK() )
//   {
//     SENDERR( "\nSetCameraPropertyEx - Can't run command %s: %s" ,
//       ( LPCTSTR )Command , Result.GetDescription().GetAscii() ) ;
//     TRACE( "\nSetCameraPropertyEx - Can't run command %s: %s" ,
//       ( LPCTSTR )Command , Result.GetDescription().GetAscii() ) ;
//     return false ;
//   }
//   double dRunStartTime = GetHRTickCount() ;
//   bool bFinished = false ;
//   do
//   {
//     m_Result = pCommandFeature->IsCommandDone( bFinished ) ;
//     if ( !Result.IsOK() )
//     {
//       SENDERR( "\nSetCameraPropertyEx - Can't check finish run %s: %s" ,
//         ( LPCTSTR )Command , Result.GetDescription().GetAscii() ) ;
//       TRACE( "\nSetCameraPropertyEx - Can't check finish run %s: %s" ,
//         ( LPCTSTR )Command , Result.GetDescription().GetAscii() ) ;
//       return false ;
//     }
//   } while ( !bFinished && ( GetHRTickCount() - dRunStartTime < 2000. ) ) ;
//   if ( !bFinished )
//   {
//     SENDERR( "\nSetCameraPropertyEx - Command %s is not finshed in 2000 ms" ,
//       ( LPCTSTR )Command ) ;
//     TRACE( "\nSetCameraPropertyEx - Command %s is not finshed in 2000 ms" ,
//       ( LPCTSTR )Command ) ;
//     return false ;
//   }
// 
//   switch ( iMode )
//   {
//     case 0: return true ; // no operation
//     case 1:// save
//     {
  //       Result = pCamParams->Get( ( LPCTSTR )m_SetDefaultSettings , PropPtr ) ;
//       if ( !Result.IsOK() )
//       {
//         SENDERR( "\nSetCameraPropertyEx - Can't get property by name %s: %s" ,
//           ( LPCTSTR )m_SetDefaultSettings , Result.GetDescription().GetAscii() ) ;
//         TRACE( "\nSetCameraPropertyEx - Can't get property by name %s: %s" ,
//           ( LPCTSTR )m_SetDefaultSettings , Result.GetDescription().GetAscii() ) ;
//         return false ;
//       }
//       if ( m_bSetDefaultSettingsIsEnum )
//       {
//         AVT::VmbAPI::EnumEntryVector Enumerators ;
//         m_Result = PropPtr->GetEntries( Enumerators ) ;
//         if ( !Result.IsOK() )
//         {
//           SENDERR( "\nSetCameraPropertyEx - Can't get enums for %s: %s" ,
//             ( LPCTSTR )m_SetDefaultSettings , Result.GetDescription().GetAscii() ) ;
//           TRACE( "\nSetCameraPropertyEx - Can't get enums for %s: %s" ,
//             ( LPCTSTR )m_SetDefaultSettings , Result.GetDescription().GetAscii() ) ;
//           return false ;
//         }
//         if ( Enumerators.size() < 2 )
//         {
//           SENDERR( "\nSetCameraPropertyEx - Not enough enums for %s(%d): %s" ,
//             ( LPCTSTR )m_SetDefaultSettings , Enumerators.size() ,
//             Result.GetDescription().GetAscii() ) ;
//         }
//         PvString Enum ;
//         Enumerators.at( 1 ).GetName( Enum ) ;
//         m_Result = PropPtr->SetValue( Enum.GetAscii() ) ;
//         if ( !Result.IsOK() )
//         {
//           SENDERR( "\nSetCameraPropertyEx - Can't set %s to %s: %s" ,
//             ( LPCTSTR )m_SetDefaultSettings , Enum.GetAscii() ,
//             Result.GetDescription().GetAscii() ) ;
//           TRACE( "\nSetCameraPropertyEx - Can't set %s to %s: %s" ,
//             ( LPCTSTR )m_SetDefaultSettings , Enum.GetAscii() ,
//             Result.GetDescription().GetAscii() ) ;
//           return false ;
//         }
//       }
//       else  // Set default set is command, not enumerator operation
//       {
//         Result = PropPtr->RunCommand() ;
//         if ( !Result.IsOK() )
//         {
//           SENDERR( "\nSetCameraPropertyEx - Can't run command %s: %s" ,
//             ( LPCTSTR )m_SaveSettingsCommand , Result.GetDescription().GetAscii() ) ;
//           TRACE( "\nSetCameraPropertyEx - Can't run command %s: %s" ,
//             ( LPCTSTR )m_SaveSettingsCommand , Result.GetDescription().GetAscii() ) ;
//           return false ;
//         }
//       }
//       return true ;
//     }
//     case 2: // Load, all is done before
//       return true ;
//   }
  return false ;
}

bool PleoraCamera::SetCameraPropertyEx(
  unsigned iIndex , SetCamPropertyData * pData , bool& Invalidate )
{
  if ( !m_Camera.IsConnected() )
  {
    SENDERR( "\nSetCameraPropertyEx - camera if not connected for SN%u " , m_dwSerialNumber ) ;
    TRACE( "\nSetCameraPropertyEx - camera if not connected for SN%u " , m_dwSerialNumber ) ;
    return false ;
  }
  if ( iIndex >= ( unsigned )m_PropertiesEx.GetCount() )
  {
    SENDERR( "\nSetCameraPropertyEx - No property for index %i " , iIndex ) ;
    TRACE( "\nSetCameraPropertyEx - No property for index %i " , iIndex ) ;
    return false ;
  }

  PvGenParameterArray * pCamParams = m_Camera.GetGenParameters() ;

  PvResult Result ;
  //   CamProperty& Prop = m_pOrigProperties[iIndex] ;
  CameraAttribute& Prop = m_PropertiesEx.GetAt( iIndex ) ;
  strcpy_s ( pData->m_Name , (LPCTSTR)Prop.m_CameraPropertyName ) ;
  switch ( Prop.pr )
  {
    case FGP_ROI:
    {
      CRect rc;
      if ( sscanf( ( LPCTSTR )pData->m_szString , "%d,%d,%d,%d" , &rc.left , &rc.top , &rc.right , &rc.bottom ) == 4 )
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
        ( LPCTSTR )m_PropertiesForLogAsString ) ;
      return true ;
    case FGP_USER_SET_SELECT:
    {
      return PleoraSaveOrLoadSettings( pData->m_int ) ;
    }
    default:
    {
      bool bAutoOn = false ;
    switch ( Prop.m_Type )
      {
    case PvGenTypeInteger:
        {
      PvGenInteger*  pPropPtr = pCamParams->GetInteger( pData->m_Name ) ;
      if ( !pPropPtr )
        {
        SENDERR( "\nSetCameraPropertyEx - Can't get int property by name %s" ,
          ( LPCTSTR )Prop.m_Name ) ;
        TRACE( "\nSetCameraPropertyEx - Can't get int property by name %s" ,
          ( LPCTSTR )Prop.m_Name ) ;
          return false ;
        }
          if ( Prop.m_AutoControl.IsEmpty() || !bAutoOn )
          {
            PvInt64 Value64 = pData->m_int ;
        Result = pPropPtr->SetValue( Value64 ) ;
            if ( !Result.IsOK() )
            {
              SENDERR( "\nSetCameraPropertyEx - Can't set int data for property %s: %s" ,
                ( LPCTSTR )Prop.m_Name , Result.GetDescription().GetAscii() ) ;
              TRACE( "\nSetCameraPropertyEx - Can't set int data for property %s: %s" ,
                ( LPCTSTR )Prop.m_Name , Result.GetDescription().GetAscii() ) ;
              return false ;
            }
          }
          return true ;
        }
        case PvGenTypeFloat:
        {
      PvGenFloat*  pPropPtr = pCamParams->GetFloat( pData->m_Name ) ;
      if ( !pPropPtr )
              {
        SENDERR( "\nSetCameraPropertyEx - Can't get float property by name %s" ,
          ( LPCTSTR )Prop.m_Name ) ;
        TRACE( "\nSetCameraPropertyEx - Can't get float property by name %s" ,
          ( LPCTSTR )Prop.m_Name ) ;
        return false ;
            }
      if ( Prop.m_AutoControl.IsEmpty() || !bAutoOn )
      {
        Result = pPropPtr->SetValue( pData->m_double ) ;
            if ( !Result.IsOK() )
            {
              SENDERR( "\nSetCameraPropertyEx - Can't set float data %g for property %s: %s" ,
                pData->m_double , ( LPCTSTR )Prop.m_Name , Result.GetDescription().GetAscii() ) ;
              TRACE( "\nSetCameraPropertyEx - Can't set float data for property %s: %s" ,
                ( LPCTSTR )Prop.m_Name , Result.GetDescription().GetAscii() ) ;
              return false ;
            }
          }
          return true ;
        }
        case PvGenTypeEnum :
        {
      PvGenEnum*  pPropPtr = pCamParams->GetEnum( pData->m_Name ) ;
              if ( !pPropPtr )
              {
        SENDERR( "\nSetCameraPropertyEx - Can't get enum property by name %s" ,
          ( LPCTSTR )Prop.m_Name ) ;
        TRACE( "\nSetCameraPropertyEx - Can't get enum property by name %s" ,
          ( LPCTSTR )Prop.m_Name ) ;
            return false ;
          }
      Result = pPropPtr->SetValue( pData->m_szString ) ;
          if ( !Result.IsOK() )
          {
            SENDERR( "\nSetCameraPropertyEx - Can't set property %s to %s: %s" ,
              ( LPCTSTR )Prop.m_Name , pData->m_szString , Result.GetDescription().GetAscii() ) ;
            TRACE( "\nSetCameraPropertyEx - Can't set property %s to %s: %s" ,
          ( LPCTSTR )Prop.m_Name , pData->m_szString , Result.GetDescription().GetAscii() ) ;
            return false ;
          }
          return true ;
        }
    break;
    case PvGenTypeString:
{
      PvGenString*  pPropPtr = pCamParams->GetString( pData->m_Name ) ;
  if ( !pPropPtr )
  {
        SENDERR( "\nSetCameraPropertyEx - Can't get string property by name %s" ,
          ( LPCTSTR )Prop.m_Name ) ;
        TRACE( "\nSetCameraPropertyEx - Can't get string property by name %s" ,
          ( LPCTSTR )Prop.m_Name ) ;
    return false ;
  }
      Result = pPropPtr->SetValue( pData->m_szString ) ;
  if ( !Result.IsOK() )
  {
        SENDERR( "\nSetCameraPropertyEx - Can't set property %s to %s: %s" ,
          ( LPCTSTR )Prop.m_Name , pData->m_szString , Result.GetDescription().GetAscii() ) ;
        TRACE( "\nSetCameraPropertyEx - Can't set property %s to %s: %s" ,
          ( LPCTSTR )Prop.m_Name , pData->m_szString , Result.GetDescription().GetAscii() ) ;
        return false ;
      }
      return true ;
    }
    break ;
    case PvGenTypeBoolean:
    {
      PvGenBoolean*  pPropPtr = pCamParams->GetBoolean( pData->m_Name ) ;
      if ( !pPropPtr )
      {
        SENDERR( "\nSetCameraPropertyEx - Can't get bool property by name %s" ,
          ( LPCTSTR )Prop.m_Name ) ;
        TRACE( "\nSetCameraPropertyEx - Can't get bool property by name %s" ,
          ( LPCTSTR )Prop.m_Name ) ;
        return false ;
      }
      Result = pPropPtr->SetValue( pData->m_bBool ) ;
      if ( !Result.IsOK() )
      {
        SENDERR( "\nSetCameraPropertyEx - Can't set bool data for property %s: %s" ,
          ( LPCTSTR )Prop.m_Name , Result.GetDescription().GetAscii() ) ;
        TRACE( "\nSetCameraPropertyEx - Can't set bool data for property %s: %s" ,
          ( LPCTSTR )Prop.m_Name , Result.GetDescription().GetAscii() ) ;
        return false ;
      }
      return true ;
    }
    case PvGenTypeRegister:
    {
      PvGenRegister*  pPropPtr = pCamParams->GetRegister( pData->m_Name ) ;
      if ( !pPropPtr )
      {
        SENDERR( "\nSetCameraPropertyEx - Can't get register property by name %s" ,
          ( LPCTSTR )Prop.m_Name ) ;
        TRACE( "\nSetCameraPropertyEx - Can't get register property by name %s" ,
          ( LPCTSTR )Prop.m_Name ) ;
        return false ;
      }
      PvInt64 i64Len = 0 ;
      pPropPtr->GetLength( i64Len ) ;
      if ( ( int )i64Len != pData->m_int )
    {
        SENDERR( "SetCameraPropertyEx - Not Correct Register len %d!=%d for property %s" ,
          pData->m_int , ( int )i64Len , pData->m_Name ) ;
        TRACE( "SetCameraPropertyEx - Not Correct Register len %d!=%d for property %s" ,
          pData->m_int , ( int )i64Len , pData->m_Name ) ;
        return false ;
      }
      Result = pPropPtr->Set( pData->m_Buffer , i64Len ) ;
      if ( !Result.IsOK() )
      {
        SENDERR( "\nSetCameraPropertyEx - Can't set register data for property %s: %s" ,
          ( LPCTSTR )Prop.m_Name , Result.GetDescription().GetAscii() ) ;
        TRACE( "\nSetCameraPropertyEx - Can't set register data for property %s: %s" ,
          ( LPCTSTR )Prop.m_Name , Result.GetDescription().GetAscii() ) ;
        return false ;
      }
      return true ;
    }
  }
  }
  }
  return false ;
}

bool PleoraCamera::GetCameraPropertyEx( int iIndex , SetCamPropertyData& Value )
{

  CameraAttribute& Prop = m_PropertiesEx.GetAt( iIndex ) ;

  return GetCameraPropertyEx( Prop.m_CameraPropertyName , Value ) ;
}

bool PleoraCamera::GetCameraPropertyEx( LPCTSTR pszPropertyName , SetCamPropertyData& Value )
{
  PvGenParameterArray * pCamParams = m_Camera.GetGenParameters() ;

  return GetPropertyValue( pCamParams , pszPropertyName , Value ) ;
}

bool PleoraCamera::GetROI( CRect& rc )
{
  int iWidth = GetXSize() ;
  int iHeight = GetYSize() ;
  if ( iWidth != 0 && iHeight != 0 )
  {
    int iXl = GetXOffset() ;
    int iYt = GetYOffset() ;

    rc.left = iXl ;
    rc.top = iYt ;
    rc.right = iWidth ;
    rc.bottom = iHeight ;
    m_CurrentROI = rc ;
    return true ;
  }
  return false ;
}
bool PleoraCamera::SetROI( LPCTSTR pROIAsText )
{
  CRect rc;
  if ( sscanf( pROIAsText , "%d,%d,%d,%d" , &rc.left , &rc.top , &rc.right , &rc.bottom ) == 4 )
  {
    rc.right += rc.left ;
    rc.bottom += rc.top ;
    SetROI( rc );
    return true ;
  }
  return false ;
}

void PleoraCamera::SetROI( CRect& rc )
{
  PvGenParameterArray * pCamParams = m_Camera.GetGenParameters() ;
  if ( !pCamParams )
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


  SetXOffset( rc.left ) ;
  SetYOffset( rc.top ) ;
  SetWidth( rc.Width() ) ;
  SetHeight( rc.Height() ) ;

  m_CurrentROI = rc;

  return ;
}

bool PleoraCamera::SetStrobe( const FXString& StrobeDataAsText , int iIndex )
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
  //         m_Result = m_pGUICamera->SetStrobe( &SControl ) ;
  //         if ( m_!Result.IsOK() )
  //         {
  //           SEND_DEVICE_ERR("Set Strobe %d error: %s",iIndex,m_Result.GetDescription().GetDescription( ));
  //           return false ;
  //         }
  //         return true ;
  //       }

  return false ;
}

void PleoraCamera::GetStrobe( FXString& StrobeDataAsText , int iIndex )
{
  //       if ( !m_pGUICamera )
  //         return ;
  // 
  //       StrobeControl SControl ;
  //       SControl.source = iIndex ;
  //       m_Result = m_pGUICamera->GetStrobe( &SControl ) ;
  //       if ( m_!Result.IsOK() )
  //       {
  //         SEND_DEVICE_ERR("Get Strobe error: %s",m_Result.GetDescription().GetDescription( ));
  //       }
  //       else
  //       {
  //         StrobeDataAsText.Format( _T("%d,%d,%d,%d") ,
  //           SControl.onOff , SControl.polarity , 
  //           _ROUND(SControl.delay * 1.e6) , _ROUND( SControl.duration * 1.e6) ) ;
  //       }
}

void PleoraCamera::GetCamResolutionAndPixelFormat(
  unsigned int* rows , unsigned int* cols , PvPixelType* pixelFmt )
{
  //       if ( !m_pGUICamera )pParam
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


unsigned int PleoraCamera::GetBppFromPixelFormat( PvPixelType pixelFormat )
{
  switch ( pixelFormat )
  {
    case PvPixelMono8:
    case PvPixelBayerGR8:
      return 8;
      break;
    case PvPixelYUV411Packed:
    case PvPixelMono12:
    case PvPixelBayerGR12:
      return 12;
      break;
    case PvPixelMono16:
    case PvPixelYUV422Packed:
    case PvPixelBayerGR16:
      return 16;
      break;
    case PvPixelYUV444Packed:
    case PvPixelRGB8Packed:
    case PvPixelBGR8Packed:
      return 24;
    case PvPixelRGBA8Packed:
    case PvPixelBGRA8Packed:
      return 32;
    case PvPixelRGB16Packed:
      return 48;
    default:
      return 0;
      break;
  }
}

void PleoraCamera::LogError( LPCTSTR Msg )
{
  FXString GadgetName ;
  GetGadgetName( GadgetName ) ;
  FxSendLogMsg( MSG_ERROR_LEVEL , GetDriverInfo() , 0 ,
    _T( "%s - %s" ) , ( LPCTSTR )GadgetName , Msg );
}

bool PleoraCamera::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  double dStart = GetHRTickCount() ;
  // following parenthesis are for proper auto lock release
  {
    FXAutolock al( m_SettingsLock , "ScanProperties" ) ;
  m_bInScanProperties = true ;
  m_bWasStopped = m_bShouldBeReprogrammed = false ;

  DriverInit() ;
  FXString tmpS;
  FXPropertyKit pc( text );
  unsigned camSN = 0 ;
  if ( pc.GetInt( "Camera" , ( int& )camSN ) )
  {
    if ( camSN && ( camSN != 0xffffffff ) )
    {
      unsigned newCamnmb = SerialToNmb( camSN );;
      if ( newCamnmb < m_CamerasOnBus )
      {
        if ( ( m_dwSerialNumber != camSN ) || ( newCamnmb != m_CurrentCamera ) )
        {
          m_bWasStopped = IsRunning() ;
            if ( !IsSNLegal() )
          {
            m_dwSerialNumber = 0 ;
            m_dwConnectedSerialNumber = 0 ;
          }
          m_dwSerialNumber = camSN ;
          m_CurrentCamera = newCamnmb;
          if ( m_dwSerialNumber )
          {
//             OtherThreadCameraInit();
            CameraInit() ;
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
      if ( m_dwSerialNumber && ( m_dwSerialNumber != 0xffffffff ) )
      {
        //            bool bRes =CamCNTLDoAndWait( CAM_EVT_RELEASE , 2000) ;
        CameraStop() ;
        CameraClose() ;
        m_dwSerialNumber = 0 ;
        m_dwConnectedSerialNumber = 0 ;

        Invalidate = true ;
      }
      //       ASSERT( bRes ) ;
    }
  }
  if ( DriverValid() )
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
      if ( ( key == SETUP_COMBOBOX ) || ( key == SETUP_SPIN ) ) // ints result
      {
        FXString sValue ;
        int value ; // what will be passed for property set
        bool bauto = false ;
        if ( pc.GetString( Prop.m_Name , sValue ) )
        {
          switch ( Prop.m_Type )
          {
            case PvGenTypeInteger:
            {
              if ( sValue != _T( "auto" ) )
                value = atoi( ( LPCTSTR )sValue ) ;
              else
              {
                bauto = true ;
                value = (int)Prop.m_i64Val ;
              }
            }
            break ;
            case PvGenTypeFloat:
            {
              if ( sValue != _T( "auto" ) )
                value = atoi( ( LPCTSTR )sValue ) ;
              else
              {
                bauto = true ;
                value = ROUND( Prop.m_dVal ) ;
              }
            }
            break ;
            case PvGenTypeEnum :
            case PvGenTypeString :
            {
              value = atoi( ( LPCTSTR )sValue ) ;
              if ( Prop.m_EnumRange.GetCount() == 0 || value < 0 )
                value = -1 ;
              if ( value >= Prop.m_EnumRange.GetCount() )
                value = 0 ;
            }
            break ;
            case PvGenTypeBoolean :
              value = atoi( ( LPCTSTR )sValue ) != 0 ;
              break ;
          }

            if ( m_bStreamProgrammed && m_bCameraRunning )
            {
              if ( IsStopNecessary( Prop.pr ) )
              {
                CameraStop() ;
                m_bShouldBeReprogrammed = m_bWasStopped = true ;
              }
            }
          if ( !SetCameraProperty( i , value , bauto , Invalidate ) )
          {
            SEND_DEVICE_ERR( "Can't set property %s" , ( LPCTSTR )Prop.m_Name );
          }
        }
      }
      else if ( key == SETUP_SPINABOOL )
      {
        FXString tmpS;
        if ( pc.GetString( ( LPCTSTR )Prop.m_Name , sValue ) )
        {
          int value;
          bool bauto = ( sValue.CompareNoCase( "auto" ) == 0 ) ;
          if ( !bauto )
            value = atoi( sValue ) ;

          if ( !SetCameraProperty( i , value , bauto , Invalidate ) )
          {
            SEND_DEVICE_ERR( "Can't set property %s" , ( LPCTSTR )Prop.m_Name );
          }
        }
      }
      else if ( key == SETUP_EDITBOX )
      {
        int value; bool bauto = false;
        if ( pc.GetString( ( LPCTSTR )Prop.m_Name , sValue ) )
        {
          if ( Prop.pr == FGP_ROI )
          {
            if ( !SetROI( sValue ) )
            {
              SEND_DEVICE_ERR( "Can't set prop %s to %s" , ( LPCTSTR )Prop.m_Name , ( LPCTSTR )sValue );
            }
          }
          else
          {
          value = ( int )( ( LPCTSTR )sValue );
          if ( !SetCameraProperty( i , value , bauto , Invalidate ) )
          {
            SEND_DEVICE_ERR( "Can't set prop %s to %s" , ( LPCTSTR )Prop.m_Name , ( LPCTSTR )sValue );
          }
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
//    OtherThreadBuildPropertyList() ;
    BuildPropertyList() ;
    m_LastPrintedProperties.Empty() ;
    m_LastPrintedSettings.Empty() ;
  }
  }

  m_bInScanProperties = false ;

  if ( IsSNLegal() && m_PropertiesEx.GetCount() > 2 )
  {
  if ( m_bShouldBeReprogrammed )
  {
    if ( CameraInit() && m_bWasStopped )
      {
        if ( CameraStart() )
        {
        };
  }
    }
//     if ( !m_bStreamProgrammed )
//       m_bCameraRunning = CameraStart() ;
  }

  FXString GadgetName ;
  GetGadgetName( GadgetName ) ;
  double dBusyTime = GetHRTickCount() - dStart ;
  TRACE( "\nPleoraCamera::ScanProperties %s: Start %g , Busy %g" , ( LPCTSTR )GadgetName ,
    dStart , dBusyTime ) ;
  return true;
}

void PleoraCamera::OnBusArrival( void* pParam , LPCTSTR szSerNum )
{
  PleoraCamera* pPleora = static_cast< PleoraCamera* >( pParam );
//   if ( pPleora->m_szSerialNumber == szSerNum )
//   {
//     pPleora->m_BusEvents |= BUS_EVT_ARRIVED ;
//     SetEvent( pPleora->m_evCameraControl ) ;
//     FxSendLogMsg( MSG_WARNING_LEVEL ,
//       pPleora->GetDriverInfo() , 0 , "Camera %u(%s) is connected" ,
//       pPleora->m_dwSerialNumber , szSerNum ) ;
//     TRACE( "\nBus Arrival for Camera #%u(%s) " , pPleora->m_dwSerialNumber , szSerNum ) ;
//   }
// 
  pPleora->m_bRescanCameras = true ;
  pPleora->m_dwNArrivedEvents++ ;
}

void PleoraCamera::OnBusRemoval( void* pParam , LPCTSTR szSerNum )
{
  PleoraCamera* pPleora = static_cast< PleoraCamera* >( pParam );
//   if ( pPleora->m_szSerialNumber == szSerNum )
//   {
//     pPleora->m_BusEvents |= BUS_EVT_REMOVED ;
//     SetEvent( pPleora->m_evCameraControl ) ;
//     FxSendLogMsg( MSG_WARNING_LEVEL ,
//       pPleora->GetDriverInfo() , 0 , "Used Camera %u(%s) is disconnected" ,
//       pPleora->m_dwSerialNumber , szSerNum ) ;
//     TRACE( "\nCamera #%u(%s) Removed\n" , pPleora->m_dwSerialNumber , szSerNum ) ;
//   }
}


void PleoraCamera::OnBusReset( void* pParam , LPCTSTR szSerNum )
{
  PleoraCamera* pPleora = static_cast< PleoraCamera* >( pParam );
  if ( pPleora->m_szSerialNumber == szSerNum )
  {
    //     pPleora->m_BusEvents |= BUS_EVT_RESET ;
    //     SetEvent( pPleora->m_evCameraControl ) ;
    FxSendLogMsg( MSG_WARNING_LEVEL ,
      pPleora->GetDriverInfo() , 0 , "Bus Reset for Camera %u(%s)" ,
      pPleora->m_dwSerialNumber , szSerNum ) ;
    TRACE( "\nBus Reset for Camera #%u(%s) \n" , pPleora->m_dwSerialNumber , szSerNum ) ;
  }
  pPleora->m_bCamerasEnumerated = false ;
  //   bool bRes = pPleora->CamCNTLDoAndWait( BUS_EVT_BUS_RESET | CAM_EVT_INIT , 2000)  ;
  pPleora->m_bRescanCameras = true ;
  //   ASSERT( bRes ) ;
}



bool PleoraCamera::CheckAndAllocCamera( void )
{
  if ( !m_Camera.IsConnected() )
  {
    if ( !IsSNLegal() )
      return false ;
    if ( !CameraInit() )
      return false ;
  }
  return true ;
}


bool PleoraCamera::SetBMIH( void )
{
  m_BMIH.biSize = sizeof( BITMAPINFOHEADER );
  m_BMIH.biWidth = m_CurrentROI.Width() ;
  if ( m_BMIH.biWidth == -1 || m_BMIH.biWidth == 0 )
    m_BMIH.biWidth = m_SensorSize.cx ;
  m_BMIH.biHeight = m_CurrentROI.Height();
  if ( m_BMIH.biHeight == -1 || m_BMIH.biHeight == 0 )
    m_BMIH.biHeight = m_SensorSize.cy ;

  m_BMIH.biPlanes = 1;
  switch ( m_pixelFormat )
  {
    case PvPixelBayerRG8:
    case PvPixelBayerGB8:
    case PvPixelBayerBG8:
    case PvPixelBayerGR8:
    case PvPixelMono8:
      m_BMIH.biCompression = BI_Y8;
      m_BMIH.biBitCount = 8;
      m_BMIH.biSizeImage = m_BMIH.biWidth*m_BMIH.biHeight;
      break;
    case PvPixelYUV411Packed:
      m_BMIH.biCompression = BI_YUV9 ;
      m_BMIH.biBitCount = 12;
      m_BMIH.biSizeImage = 9 * m_BMIH.biWidth*m_BMIH.biHeight / 8;
      break;
    case PvPixelMono10:
    case PvPixelMono12:
    case PvPixelMono12Packed:
    case PvPixelMono14:
    case PvPixelMono16:
      m_BMIH.biCompression = BI_Y16;
      m_BMIH.biBitCount = 16;
      m_BMIH.biSizeImage = 2 * m_BMIH.biWidth*m_BMIH.biHeight;
      break;
    case PvPixelRGB8Packed:
    case PvPixelBGR8Packed:
    case PvPixelRGBA8Packed:   // the same  case PvPixelFormatRgba8 :
    case PvPixelBGRA8Packed:
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
    case PvPixelBayerGR16:
    case PvPixelBayerRG16:
    case PvPixelBayerGB16:
    case PvPixelBayerBG16:
    case PvPixelBayerGR12Packed:
    case PvPixelBayerRG12Packed:
    case PvPixelBayerGB12Packed:
    case PvPixelBayerBG12Packed:
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


bool PleoraCamera::ScanSettings( FXString& text )
{
  double dBegin = GetHRTickCount() ;
  FXAutolock al( m_SettingsLock , "ScanSettings" ) ;
  if ( !m_LastPrintedSettings.IsEmpty()
    && ( dBegin - m_dLastSettingsPrintTime < 100.0 ) )
  {
    text = m_LastPrintedSettings ;
    m_dLastSettingsPrintTime = dBegin ;
    return true ;
  }
  m_bInScanProperties = TRUE ;
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
      ( LPCTSTR )Model , m_CamInfo[ i ].serialnmb );
    camlist += tmpS;
    if ( i < m_CamerasOnBus - 1 )
      camlist += _T( ',' ) ;
  }
  if ( iCurrentCamera < 0 && IsSNLegal() )
  {
    if ( !camlist.IsEmpty() )
      camlist += _T( ',' ) ;
    tmpS.Format( "?%d(%d)" , m_dwSerialNumber , m_dwSerialNumber ) ;
    camlist += tmpS ;
    iCurrentCamera = m_CamerasOnBus ; // first after real cameras
  }
  tmpS.Format( "ComboBox(Camera(%s))" , camlist );
  paramlist += tmpS;
  if ( ( m_dwSerialNumber == m_dwConnectedSerialNumber ) && IsSNLegal() )
  {
    paramlist += _T( ',' ) ;

//    OtherThreadBuildPropertyList() ;
    BuildPropertyList() ;
    tmpS.Format( "ComboBox(StreamState(Run(0),Idle(1)))" );
    paramlist += tmpS;
    for ( int i = 0; i < m_PropertiesEx.GetCount() ; i++ )
    {
      paramlist += _T( ',' ) ;
      paramlist += m_PropertiesEx[ i ].m_DlgFormat ;
    }
  }
  m_bInScanProperties = FALSE ;
  text.Format( "template(%s)" , paramlist );
  m_LastPrintedSettings = text ;
  m_dLastSettingsPrintTime = GetHRTickCount() ;
  double dDutyTime = GetHRTickCount() - dBegin ;
  TRACE( "\n----------Settings Printed in %g ms" , dDutyTime ) ;
  return true;
}

bool PleoraCamera::PrintProperties( FXString& text )
{
  double dBegin = GetHRTickCount() ;
  FXAutolock al( m_SettingsLock , "PrintProperties" ) ;
  if ( !m_LastPrintedProperties.IsEmpty()
    && ( dBegin - m_dLastProperiesPrintTime < 100.0 ) )
  {
    text = m_LastPrintedProperties ;
    m_dLastProperiesPrintTime = dBegin ;
    return true ;
  }
  FXPropertyKit pc;
  if ( DriverValid() && IsSNLegal() )
  {
    pc.WriteInt( "Camera" , m_dwSerialNumber );
    pc.WriteInt( "StreamState" , m_bLocalStopped ) ;
    if ( m_dwConnectedSerialNumber != 0 )
    {
      for ( int i = 0; i < m_PropertiesEx.GetSize(); i++ )
      {
        int value; bool bauto;
        if ( GetCameraProperty( m_PropertiesEx[ i ].pr , value , bauto ) )
        {
          FXString key;
          FXParser param;
          m_PropertiesEx[ i ].m_DlgFormat.GetElementNo( 0 , key , param );
          if ( ( key == SETUP_COMBOBOX ) || ( key == SETUP_SPIN ) ) // ints result
          {
            pc.WriteInt( m_PropertiesEx[ i ].m_Name , value );
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
            FXString svalue = ( LPCTSTR )value;
            pc.WriteString( m_PropertiesEx[ i ].m_Name , svalue );
          }
          else
          {
            C1394_SENDERR_1( "Unsupported key '%s'in scanproperty" , key );
          }
        }
      }
      pc += text ;
    }
  }
  else
  {
    pc.WriteInt( "Camera" , -1 );
  }
  m_LastPrintedProperties = text = pc;
  m_dLastProperiesPrintTime = GetHRTickCount() ;
  double dDutyTime = GetHRTickCount() - dBegin ;
  TRACE( "\n----------Properties Printed in %g ms" , dDutyTime ) ;
  return true;
}

void PleoraCamera::SendValueToLog()
{
  FXString Addition ;

  m_LogLock.Lock() ;
  PvGenParameterArray * pCamParams = m_Camera.GetGenParameters() ;
  if ( !pCamParams )
    return ;
  if ( m_iLogCnt >= 0 && m_PropertiesForLog.GetCount() )
  {
    SetCamPropertyData Value ;
    if ( GetPropertyValue( pCamParams , m_PropertiesForLog[ m_iLogCnt ] , Value ) )
    {
      if ( m_iLogCnt == 0 )
      {
        double dT = GetHRTickCount() ;
        Addition.Format( " %8d.%3d" , ( int )( dT / 1000. ) , ( int )fmod( dT , 1000. ) ) ;
        m_LogOutString += Addition ;
      }
      switch ( Value.m_Type )
      {
        case PvGenTypeInteger:
          Addition.Format( " %d" , (int)Value.m_int64 ) ;
          break ;
        case PvGenTypeFloat:
          Addition.Format( " %g" , Value.m_double ) ;
          break ;
        case PvGenTypeEnum :
        case PvGenTypeString :
          Addition.Format( " %s" , Value.m_szString ) ;
          break ;
        case PvGenTypeBoolean :
          Addition.Format( " %s" , Value.m_bBool ? "true" : "false" ) ;
          break ;
      }
      if ( Addition.IsEmpty() )
        Addition.Format( " Can't show property %s" , ( LPCTSTR )m_PropertiesForLog[ m_iLogCnt ] ) ;
      m_LogOutString += Addition ;
    }
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
    m_BusEvents &= ~CAM_EVT_LOG ;
  };
  m_LogLock.Unlock() ;
}

DWORD WINAPI PleoraCamera::CameraGrabLoop( LPVOID pParam )
{
  TRACE( "---------Entry to CameraGrabLoop\n" ) ;
  PleoraCamera * pPleora = ( PleoraCamera* )pParam ;

  return pPleora->GrabLoop() ;
  }

DWORD WINAPI PleoraCamera::GrabLoop()
          {
  m_bGrabLoopContinue = true ;
  // Acquire images until the user instructs us to stop
  FXString ResultTxt , OpResultTxt ;
  while ( m_bGrabLoopContinue )
          {
    PvBuffer *pBuffer = NULL;
    PvResult lOperationResult;
    //     PvInt64 i64FrameCnt ;
    //     double dFrameRate , dBandWidth ;
    CVideoFrame * pOutFrame = NULL ;

    // Retrieve next buffer		
    if ( m_Pipeline.IsStarted() )
    {
      PvResult lResult = m_Pipeline.RetrieveNextBuffer( &pBuffer , 10 , &lOperationResult );
      if ( lResult.IsOK() && pBuffer )
        {
      if ( lOperationResult.IsOK() )
        {
        //         pCount->GetValue( i64FrameCnt );
        //         pFrameRate->GetValue( dFrameRate );
        //         pBandwidth->GetValue( dBandWidth );

        if ( pBuffer->GetPayloadType() == PvPayloadTypeImage )
        {
          // Get image specific buffer interface
          PvImage *pImage = pBuffer->GetImage();
          pOutFrame = ConvertPleoraToSHformat( pImage ) ;
        }
        }
        else
        {
          OpResultTxt = lOperationResult.GetDescription().GetAscii() ;
          char a = OpResultTxt[ 0 ] ;
        }
      // We have an image - do some processing (...) and VERY IMPORTANT,
      // re-queue the buffer in the stream object
        // m_Stream.QueueBuffer( pBuffer );
        m_Pipeline.ReleaseBuffer( pBuffer ) ;
      if ( pOutFrame )
          {

        m_GrabLock.Lock() ;
        while ( m_ReadyFrames.size() > 2 )
              {
          m_ReadyFrames.front()->Release() ;
          m_ReadyFrames.pop() ;
              }
        m_ReadyFrames.push( pOutFrame ) ;
        m_GrabLock.Unlock() ;
            }
          }
          else
        {
        bool bConnected = m_Camera.IsConnected() ;
        ResultTxt = lResult.GetDescription().GetAscii() ;
        char a = lResult[ 0 ] ;
      }
        }
    else
      Sleep( 10 ) ;
      }
  m_dwGrabThreadId = 0 ;
  return 0 ;
    }

// void PleoraCamera::OnParameterUpdate( PvGenParameter *aParameter )
//   {
//   bool bBufferResize = false;
//   PvString lName;
//   if ( !aParameter->GetName( lName ).IsOK() )
//     {
//     ASSERT( 0 ); // Totally unexpected	
//     return;
//     }
//   FXString Name = lName.GetAscii() ;
//   int iLen = lName.GetLength() ;
//   //   if ( ( lName == "AcquisitionMode" ) &&
//   //     ( mModeCombo.GetSafeHwnd() != 0 ) )
//   //           {
//   //     bool lAvailable = false , lWritable = false;
//   //     VERIFY( aParameter->IsAvailable( lAvailable ).IsOK() );
//   //     if ( lAvailable )
//   //   {
//   //       VERIFY( aParameter->IsWritable( lWritable ).IsOK() );
//   //     }
//   // 
//   //     mModeCombo.EnableWindow( lAvailable && lWritable );
//   // 
//   //     PvGenEnum *lEnum = dynamic_cast< PvGenEnum * >( aParameter );
//   //     ASSERT( lEnum != NULL );
//   // 
//   //     if ( lEnum != NULL )
//   //     {
//   //       PvInt64 lEEValue = 0;
//   //       VERIFY( lEnum->GetValue( lEEValue ) );
//   // 
//   //       for ( int i = 0; i < mModeCombo.GetCount(); i++ )
//   //     {
//   //         DWORD_PTR lData = mModeCombo.GetItemData( i );
//   //         if ( lData == lEEValue )
//   //     {
//   //           mModeCombo.SetCurSel( i );
//   //           break;
//   //       }
//   //       }
//   //     }
//   //   }
//   }

#pragma once

#include "stdafx.h"
#include <math/Intf_sup.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <afxmt.h>
#include <iostream>

//#include "mfutility.h"
#include <gadgets\gadbase.h>
#include <helpers\UserBaseGadget.h>

#include "gadgets\VideoFrame.h"
#include "gadgets\TextFrame.h"
#include "gadgets\WaveFrame.h"
#include "gadgets\QuantityFrame.h"
#include "gadgets\RectFrame.h"
#include "gadgets\FigureFrame.h"

#include <helpers/CameraData.h>
#include "USBCamInSystem.h"
#include <atlbase.h>
#include <thread>
#include <queue>
#include <mutex>
#include "Callback.h"

// properties [0,999] - camera control
// properties [1000,1999] - amplifier control
// property 10000 - camera id
typedef enum 
{
  VCapP_Pan = 0 ,
  VCapP_Tilt = (VCapP_Pan + 1) ,
  VCapP_Roll = (VCapP_Tilt + 1) ,
  VCapP_Zoom = (VCapP_Roll + 1) ,
  VCapP_Exposure = (VCapP_Zoom + 1) ,
  VCapP_Iris = (VCapP_Exposure + 1) ,
  VCapP_Focus = (VCapP_Iris + 1) ,

  VCapP_Brightness = 1000 ,
  VCapP_Contrast = (VCapP_Brightness + 1) ,
  VCapP_Hue = (VCapP_Contrast + 1) ,
  VCapP_Saturation = (VCapP_Hue + 1) ,
  VCapP_Sharpness = (VCapP_Saturation + 1) ,
  VCapP_Gamma = (VCapP_Sharpness + 1) ,
  VCapP_ColorEnable = (VCapP_Gamma + 1) ,
  VCapP_WhiteBalance = (VCapP_ColorEnable + 1) ,
  VCapP_BacklightCompensation = (VCapP_WhiteBalance + 1) ,
  VCapP_Gain = (VCapP_BacklightCompensation + 1) ,

  VCapP_CameraNum  = 10000  , // property with Id = 10000
  VCapP_FormatAndSize = 10001 , // property with Id = 10001  
  
  VCapP_Auto = 20000 , // for possibility for auto mode marking
  VCapP_Unknown = -1

} VideoCaptureProperties;


typedef enum 
{
  OF_YUV12 = 0 ,
  OF_YUV9 ,
  OF_Y800
} OutputFormat ;

bool EnumCameras( CamerasVector& AllCameras , ULONG& ulLastEnumDevicesNumber );

union PropertyValue
{
  bool     bBool ;
  int      iInt ;
  __int64  iInt64 ;
  double   dDouble ;
  char     szAsString[ 300 ] ;
};


class CamProperty
{
public:
  VideoCaptureProperties  m_pr;
  const char *            m_name;
  VideoCaptureProperties  m_AutoControlId ;
  void       *        m_pInfo ;
  bool                m_bStopAcquisition ;
  int                 m_DataType ;
  PropertyValue       m_LastValue ;
  FXParser            m_DlgFormat ;

  CamProperty( VideoCaptureProperties pr = VideoCaptureProperties(-1) ,
    LPCTSTR name = NULL ,
    VideoCaptureProperties AutoId = (VideoCaptureProperties) -1 ,
    LPCTSTR pInfo = NULL ,
    bool    bStopGrab = false )
  {
    memset( this , 0 , sizeof( *this ) - sizeof( FXParser ) ) ;
    m_pr = pr ;
    m_name = name ;
    m_AutoControlId = AutoId ;
    m_pInfo = (void*)pInfo ;
    m_bStopAcquisition = bStopGrab ;
  }
  CamProperty& operator=( CamProperty& Orig )
  {
    memcpy( this , &Orig , sizeof( *this ) - sizeof( FXParser ) ) ;
    m_DlgFormat = Orig.m_DlgFormat ;
    return *this ;
  }
} ;

class USBCamera :public UserCaptureBaseGadget
{
  IMFMediaSource  * m_pMediaSource ;
  IMFSourceReader * m_pSourceReader;
  CComObject<CCallback> * m_pCallback ;
  static CModule    m_Module ;
  static int        m_iNGadgetsInApplication ;
  bool              m_bWasFirst;


  CameraData    m_ThisCamera ;
  FXString      m_CamerasListForStdDlg ;
  FXString      m_CaptureModesListForStdDlg ;
  //ULONG         m_ulLastDevicesNumber ; // not only Video!!!
  //int         m_iCameraNum ;  // selected in STD dialog camera num 
  int         m_iIndex ;      // Index in global camera array
  __int64     m_CurrentLocation ;
  USBLocation m_Location ;
  FXString    m_CameraFriendlyName ;
  FXString    m_VID_PID_MIAsString ;
  bool        m_bRescanCameras ;

  int         m_iCaptureMode ; // index in m_ThisCamera.m_CaptureModes
  OutputFormat m_OutputFormat ;

  CSize     m_ImageSize ;
  FOURCC    m_Fourcc ;
  BITMAPINFOHEADER m_FromCameraFormat ;
  double    m_dGuid ;
  int       m_iOutputFormat ;

  static HANDLE m_ghAppMutex ;
//   static int                    m_iCamNum ;
//   static DWORD                  m_dwDefaultTimeout ;
//   static bool                   m_bSaveFullInfo ;
  FXLockObject  m_SettingsLock;
  //IntVector     m_AvailableCamIndexes ;
  std::thread * m_GrabThread = NULL ;
  std::queue<CVideoFrame * > m_CapturedQueue ;
  std::mutex    m_GrabLock ;
  CEvent        m_FrameReady ;
  FXString      m_LastPrintedProperties ;
  double        m_dLastProperiesPrintTime ;
  double        m_dLastBuiltPropertyTime ;
  FXString      m_LastPrintedSettings ;
  FXString      m_GadgetInfo ;
  double        m_dLastSettingsPrintTime ;
  std::vector<CamProperty> m_Properties;
  LPCTSTR GetGadgetInfo() { return (LPCTSTR) m_GadgetInfo ; } ;
  ULONG         m_ulLastEnumDevicesNumber ;

public:
  USBCamera() ;
  ~USBCamera() ;

  void PropertiesRegistration();
  void ConnectorsRegistration();
  int GetCamNumByIndex(__int64 i64Index);
  static UINT GrabThread( void * pGadget ) ;
  UINT GrabFunc() ;
  virtual bool DriverInit() ;
  virtual bool EnumCameras() ;
  virtual bool CameraInit() ;
  virtual void CameraClose() ;
  bool CheckAndAllocCamera( void ) ;
  virtual bool CameraStart();
  virtual void CameraStop() ;
  virtual void OnStart()
  {
    CameraStart() ;
    CCaptureGadget::OnStart() ;
    return ;
  }
  virtual void OnStop()
  {
    CameraStop() ;
    CCaptureGadget::OnStop() ;
    return ;
  }

  virtual bool IsCameraRunning() { return false ; }
  virtual bool BuildPropertyList() ;
  virtual void OnScanSettings() ;


  static void CameraChange( LPCTSTR pName , void* pObject ,
    bool& bInvalidate , bool& bRescan ) ;
  static void FormatChange( LPCTSTR pName , void* pObject ,
    bool& bInvalidate , bool& bRescan ) ;
  static void ControlChange( LPCTSTR pName , void* pObject ,
    int iId ) ; // Id is index in property array

  virtual bool GetCameraProperty( unsigned i , FXSIZE &value , bool& bauto ) ;
  virtual bool SetCameraProperty( unsigned i , FXSIZE &value , bool& bauto , bool& Invalidate ) ;

  virtual void CameraTriggerPulse( CDataFrame* pDataFrame );

  virtual CDataFrame* GetNextFrame( double * dStartTime );
  virtual int  GetInputsCount()
  {
    return UserGadgetBase::GetInputsCount();
  }
  virtual int GetOutputsCount()
  {
    return UserGadgetBase::GetOutputsCount();
  }
  virtual int GetDuplexCount()
  {
    return UserGadgetBase::GetDuplexCount();
  }

  virtual CInputConnector*		GetInputConnector( int n )
  {
    return UserGadgetBase::GetInputConnector( n );
  }
  virtual COutputConnector*	GetOutputConnector( int n )
  {
    return UserGadgetBase::GetOutputConnector( n );
  }
  virtual CDuplexConnector*	GetDuplexConnector( int n )
  {
    return UserGadgetBase::GetDuplexConnector( n );
  }
  // Capture gadget stuff
  virtual void ShutDown();

  bool CheckAvailableCameras( FXString& DlgFormat , FXString& OnlyItems ) ;
  bool IsMatch( LPCTSTR pLocationAndVPM ) ; // VPM - VID,PID,MI
  HRESULT GetSourceFromCaptureDevice( 
    DeviceType deviceType , UINT nDevice ) ;
  int ConvertSampleToImage( LPBYTE pData , int iLen ) ;// gadget knows current image format

  HRESULT SetMediaFormat( IMFMediaSource* pSource , DWORD dwFormatIndex ) ;
  int FilterFormats( CameraData& Camera ) ;
  bool ReleaseCameraGlobal( int iIndex ) ;


  static void CALLBACK fn_capture_trigger( CDataFrame* pDataFrame , void* pGadget , CConnector* lpInput )
  {
    ((USBCamera*) pGadget)->CameraTriggerPulse( pDataFrame );
  }
  DECLARE_RUNTIME_GADGET( USBCamera );
};


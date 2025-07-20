// Basler.h: interface for the Basler class.
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_Basler_H__0C071980_0F39_40DC_A144_B6BEDB69E5DC__INCLUDED_
#define AFX_Basler_H__0C071980_0F39_40DC_A144_B6BEDB69E5DC__INCLUDED_

#include <map>
#include <helpers\propertykitEx.h>
#include <fxfc/fxext.h>
#include <gadgets\CDevice.h>
#include <imageproc\VFrameEmbedInfo.h>
#include <pylon/PylonIncludes.h>
#include <pylon/PixelType.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif
#include <pylon/BaslerUniversalInstantCamera.h>
#include <pylon/_BaslerUniversalCameraParams.h>
#include <video/stdcodec.h>

//#include <pylon/usb/BaslerUsbInstantCamera.h>
// Namespace for using pylon objects.
using namespace Pylon;
using namespace GENAPI_NAMESPACE ;
using namespace Basler_UniversalCameraParams;
// Namespace for using cout.
using namespace std;

#include "BaslerComplexIO.h"  

#ifndef FULL_32BIT_VALUE
#define FULL_32BIT_VALUE 0x7FFFFFFF
#endif

typedef enum
{
  BRIGHTNESS , /**< Brightness. */
  AUTO_EXPOSURE , /**< Auto exposure. */
  SHARPNESS , /**< Sharpness */
  WHITE_BALANCE , /**< White balance. */
  HUE , /**< Hue. */
  SATURATION , /**< Saturation. */
  GAMMA , /**< Gamma. */
  IRIS , /**< Iris. */
  FOCUS , /**< Focus. */
  ZOOM , /**< Zoom. */
  PAN , /**< Pan. */
  TILT , /**< Tilt. */
  EXPOSURE_MODE , /* Timed or trigger width (for basler) */
  SHUTTER , /**< Shutter. */
  SHUTTER_AUTO ,
  GAIN , /**< Gain. */
  GAIN_AUTO ,
  TRIGGER_SELECTOR , // Burst or single
  TRIGGER_MODE , /**< Trigger mode. */
  TRIGGER_SOURCE ,
  TRIGGER_DELAY , /**< Trigger delay. */
  TRIGGER_ACTIVATE , // What front is active
  TRIGGER_SOFT_FIRE ,
  BURST_LENGTH ,  // How many frames after trigger
  FRAME_RATE , /**< Frame rate. */
  FRAME_RATE_ENABLE ,
  TEMPERATURE , /**< Temperature. */

  CAMERA_GIVEN_NAME ,
  FORMAT_MODE ,
  SETROI ,
  SETSTROBE0 ,
  SETSTROBE1 ,
  SETSTROBE2 ,
  SETSTROBE3 ,
  BASLER_PACKETSIZE ,
  BASLER_INTER_PACKET_DELAY ,
  WHITE_BAL_RED ,
  WHITE_BAL_BLUE ,
  SAVE_SETTINGS ,
  USER_SET_SELECTOR ,
  RESTART_TIMEOUT ,
  MAX_PACKET_SIZE ,
  EMBED_INFO ,
  LINE_SELECTOR ,
  LINE_MODE ,
  LINE_SOURCE ,
  LINE_INVERTER ,
  TIMER_SELECT ,
  TIMER_DELAY ,
  TIMER_DURATION ,
  TIMER_TRIGGER_SOURCE ,
  TIMER_TRIGGER_ACTIVATION ,
  TIMER_TRIGGER_POLARITY ,
  OUTPUT_SELECTOR ,
  OUTPUT_VALUE ,

  XOFFSET ,
  YOFFSET ,
  EVENT_SELECTOR ,
  EVENT_ENABLE ,
  EVENTS_STATUS ,

  N_ACTIVE_BUFFERS ,
  DONT_SHOW_ERRORS ,

  UNSPECIFIED_PROPERTY_TYPE , /**< Unspecified property type. */
  PROPERTY_TYPE_FORCE_32BITS = FULL_32BIT_VALUE
} FXCamPropertyType ;


typedef struct
{
  FXCamPropertyType Id ;
  LPCTSTR           pUIName ;
  LPCTSTR           ppNodesNames[ 5 ] ; // set of names with trailing NULL
} BaslerPropertyDefine ;

typedef enum
{
  TriggerModeError = -2 ,
  TrigNotSupported = -1 ,
  TriggerOff = 0 ,
  TriggerOn ,
  TriggerInv ,
  TriggerSoftware
} CamTriggerMode;


#define  BUS_EVT_ARRIVED       0x001 
#define  BUS_EVT_REMOVED       0x002 
#define  BUS_EVT_BUS_RESET     0x004
#define  BASLER_EVT_SHUTDOWN      0x100
#define  BASLER_EVT_START_GRAB    0x200
#define  BASLER_EVT_STOP_GRAB     0x400
#define  BASLER_EVT_RELEASE       0x800
#define  BASLER_EVT_RESTART       0x1000
#define  BASLER_EVT_INIT          0x2000
#define  BASLER_EVT_LOCAL_STOP    0x4000
#define  BASLER_EVT_LOCAL_START   0x8000
#define  BASLER_EVT_SET_PROP      0x10000
#define  BASLER_EVT_DRIVER_INIT   0x20000
#define  BASLER_EVT_BUILD_PROP    0x40000
#define  BASLER_EVT_GET_PROP      0x80000

#define COLOR_BW          0 // no color info
#define COLOR_BY_SOFTWARE 1 // RAW image received, convert to color
#define COLOR_BY_CAMERA   2 // Camera did color coding (YUV, RGB, BGR formats received)

// typedef struct tagVideoformat
// {
//   PixelFormatEnums vm;
//   const char *     name; 
// } Videoformat;

class BaslerCameraInfo
{
public:
  BaslerCameraInfo( LPCTSTR pSerNum = NULL , LPCTSTR pModelName = NULL )
  {
    m_SerialNumber = pSerNum ;
    m_ModelName = pModelName ;
  }
  FXString m_SerialNumber ;
  FXString m_ModelName ;
  BaslerCameraInfo& operator = ( BaslerCameraInfo& Orig )
  {
    m_SerialNumber = Orig.m_SerialNumber ;
    m_ModelName = Orig.m_ModelName ;
  }
};

class SetCamPropertyData
{
public:
  bool    m_bAuto ;
  int     m_iIndex ;
  FXCamPropertyType m_Type ;
  int     m_int ;
  int64_t m_int64 ;
  double  m_double ;
  bool    m_bBool ;
  bool    m_bInvalidate ;
  char    m_szString[ 100 ] ;
  SetCamPropertyData() { Reset() ; }
  void    Reset() { memset( &m_bAuto , 0 , ( LPBYTE ) m_szString - ( LPBYTE ) &m_bAuto + 1 ) ; }
  SetCamPropertyData& operator=( SetCamPropertyData& Orig )
  {
    memcpy( this , &Orig , ( LPBYTE ) m_szString - ( LPBYTE ) &m_bAuto )  ;
    strcpy_s( m_szString , Orig.m_szString ) ;
    //char * pDst = m_szString ;
    //char * pSrc = Orig.m_szString ;
    //while ( ((*(pDst++) = *(pSrc++)) != 0) 
    //  && ((pDst - m_szString) < sizeof(m_szString)) ) ;
    return *this ;
  }
};

class BaslerCamProperty
{
public:
  FXCamPropertyType   id ;
  bool                bSupported ;
  int                 AutoControl ; // index of auto control property
  PropertyDataType    m_DataType ;
  PropertyValue       LastValue ;
  bool                m_bAuto ;
  bool                m_bDontShow ;
  FXParser            m_GUIFormat ;
  FXString            name;
  FXString            m_AutoControlName ;
  vector<string>      m_EnumeratorsAsTexts ;
  vector<int64_t>     m_EnumeratorsAsValues ;
  FXString            m_InDeviceName ;


  BaslerCamProperty()
  {
    Reset() ;
  }
  virtual ~BaslerCamProperty()
  {
    Reset() ;
  } ;
  void Reset()
  {
    id = UNSPECIFIED_PROPERTY_TYPE ;
    bSupported = false ;
    AutoControl = -1 ; // in
    m_DataType = PDTUnknown ;
    LastValue.Reset() ;
    m_bAuto = false ;
    m_bDontShow = false ;
    m_GUIFormat.Empty() ;
    name.Empty() ;
    m_AutoControlName.Empty() ;
    m_EnumeratorsAsTexts.clear() ;
    m_EnumeratorsAsValues.clear() ;
    m_InDeviceName.Empty() ;
  } ;
  BaslerCamProperty& operator = ( const BaslerCamProperty& Other )
  {
    id = Other.id ;
    bSupported = Other.bSupported ;
    AutoControl = Other.AutoControl ; // in
    m_DataType = Other.m_DataType ;
    LastValue = Other.LastValue ;
    m_bAuto = Other.m_bAuto ;
    m_bDontShow = Other.m_bDontShow ;
    m_GUIFormat = Other.m_GUIFormat ;
    name = Other.name ;
    m_AutoControlName = Other.m_AutoControlName ;
    m_EnumeratorsAsTexts = Other.m_EnumeratorsAsTexts ;
    m_EnumeratorsAsValues = Other.m_EnumeratorsAsValues ;
    m_InDeviceName = Other.m_InDeviceName ;
    return *this ;
  }
} ;

class BaslerPropertiesArray : public FXArray<BaslerCamProperty> {} ;

typedef struct
{
  DWORD m_dwTimeStamp ;     //0
  DWORD m_dwFrameCntr ;     // 4
  DWORD m_dwStrobePattern ; // 8
  DWORD m_dwGPIOPinStae ;   // 12
  DWORD m_dwROIPos ;        // 16
  DWORD m_Reserved[ 2 ];    // 20
} FrameInfo , *pFrameInfo ;

inline int GetIndexForEnumProp( BaslerCamProperty * pProp , int64_t i64Val )
{
  for ( size_t i = 0 ; i < pProp->m_EnumeratorsAsValues.size() ; i++ )
  {
    if ( pProp->m_EnumeratorsAsValues[ i ] == i64Val )
      return ( int ) i ;
  }
  return -1 ;
}
inline int GetIndexForEnumProp( BaslerCamProperty * pProp , LPCTSTR pValAsString )
{
  for ( size_t i = 0 ; i < pProp->m_EnumeratorsAsValues.size() ; i++ )
  {
    if ( pProp->m_EnumeratorsAsTexts[ i ] == pValAsString )
      return ( int ) i ;
  }
  return -1 ;
}


class Basler ;

// Simple helper class to set the HeartbeatTimeout safely.
class CHeartbeatHelper
{
public:
  explicit CHeartbeatHelper( CInstantCamera& camera )
    : m_pHeartbeatTimeout( NULL )
  {
    // m_pHeartbeatTimeout may be NULL
    m_pHeartbeatTimeout = camera.GetTLNodeMap().GetNode( "HeartbeatTimeout" );
  }

  bool SetValue( int64_t NewValue )
  {
    // Do nothing if no heartbeat feature is available.
    if ( !m_pHeartbeatTimeout.IsValid() )
      return false;

    // Apply the increment and cut off invalid values if neccessary.
    int64_t correctedValue = NewValue - ( NewValue % m_pHeartbeatTimeout->GetInc() );

    m_pHeartbeatTimeout->SetValue( correctedValue );
    return true;
  }

  bool SetMax()
  {
    // Do nothing if no heartbeat feature is available.
    if ( !m_pHeartbeatTimeout.IsValid() )
      return false;

    int64_t maxVal = m_pHeartbeatTimeout->GetMax();
    return SetValue( maxVal );
  }

protected:
  GenApi::CIntegerPtr m_pHeartbeatTimeout; // Pointer to the node, will be NULL if no node exists.
};

typedef int( *CameraRemoveWithNameCB )( Basler * pGadget , LPCTSTR pszSN ) ;
// When using Device Specific Instant Camera classes there are 
// specific Configuration event handler classes available which can be used, for example
// Pylon::CBaslerGigEConfigurationEventHandler or Pylon::CBasler1394ConfigurationEventHandler
//Example of a configuration event handler that handles device removal events.
class CSampleConfigurationEventHandler : public Pylon::CConfigurationEventHandler
{
public:
  Basler * m_pGadget ;
  CameraRemoveWithNameCB m_CB ;

  CSampleConfigurationEventHandler( Basler * pGadget , CameraRemoveWithNameCB CB )
    : Pylon::CConfigurationEventHandler()
  {
    m_pGadget = pGadget ;
    m_CB = CB ;
  }
  // This method is called from a different thread when the camera device removal has been detected.
  void OnCameraDeviceRemoved( CInstantCamera& DisconnectedCamera )
  {
    if ( m_pGadget && m_CB )
    {
      m_CB( m_pGadget , DisconnectedCamera.GetDeviceInfo().GetSerialNumber().c_str() ) ;
    }
  }
};


class CSampleImageEventHandler : public CImageEventHandler
{
public:
  CSampleImageEventHandler( Basler * pHost = NULL )
  {
    m_pHost = pHost ;
  }
  virtual void OnImageGrabbed( CInstantCamera& camera ,
    const CGrabResultPtr& ptrGrabResult ) ;

  Basler * m_pHost ;
} ;

enum BaslerEvents
{
  BE_FrameStart = 100 ,
  BE_ExposureEnd = 200
} ;

class CameraEventHandler :
  public CBaslerUniversalCameraEventHandler ,
  public CBaslerUniversalImageEventHandler
{
public:
  CameraEventHandler( Basler * pHost = NULL , LPCTSTR pEventName = NULL )
  {
    m_pHost = pHost ;
    if ( pEventName )
      m_EventName = pEventName ;
    else
      m_EventName.Empty() ;
  }
  virtual void OnCameraEvent( CBaslerUniversalInstantCamera& camera ,
    intptr_t userProvidedId , GenApi::INode* pNode ) ;

  // This method is called when an image has been grabbed.
  virtual void OnImageGrabbed( CBaslerUniversalInstantCamera& camera ,
    const CBaslerUniversalGrabResultPtr& ptrGrabResult )
  {
    // An image has been received.
    uint16_t frameNumber = ( uint16_t ) ptrGrabResult->GetBlockID();

//     // Check whether the imaged item or the sensor head can be moved.
//     // This will be the case if the Exposure End has been lost or if the Exposure End is received later than the image.
//     if (frameNumber == m_nextFrameNumberForMove)
//     {
//       MoveImagedItemOrSensorHead();
//     }
// 
//     // Check for missing images.
//     if (frameNumber != m_nextExpectedFrameNumberImage)
//     {
//       throw RUNTIME_EXCEPTION("An image has been lost. Expected frame number is %d but got frame number %d.", m_nextExpectedFrameNumberImage, frameNumber);
//     }
//     IncrementFrameNumber(m_nextExpectedFrameNumberImage);
  }



  Basler * m_pHost ;
  FXString m_EventName ;
};

typedef FXArray<CameraEventHandler* , CameraEventHandler*> ActiveCameraEvents ;

class BaslerWatcher : public FXWorker
{
  FXLockObject m_Protect ;
  FXArray< Basler* >  m_WorkingGadgets ;
  FXStringArray       m_CurrentCameras ;
  FXStringArray       m_KnownCameras ;
  bool                m_bDidCheck ;
public:
  BaslerWatcher( Basler * pGadget = NULL ) ;
  BOOL Create() ;
//   FXString  GetNames( FXStringArray& src ) ;
  virtual int DoJob() ;
  void    RegisterGadget( Basler * pGadget ) ;
  bool    UnregisterGadget( Basler * pGadget ) ; // if returned true, it was last gadget
  bool    IsInitialized()
  {
    return ( m_dwTicksIdle > 10 ) ;
  }
  void SetChecked( bool bSet )
  {
    m_bDidCheck = bSet ;
  }
  bool IsChecked()
  {
    return m_bDidCheck ;
  }
};

typedef pair<string , DWORD> EventAndEnum;
typedef pair<string , DWORD> PixelTypeNameAndEnum;
typedef vector<PixelTypeNameAndEnum> vFormats;

class Basler : public CDevice
{
  friend class CSampleImageEventHandler ;
  friend class BaslerWatcher ;
  friend class CameraEventHandler ;

  TriggerModeEnums m_TriggerMode ;
  TriggerSelectorEnums m_TriggerSelector ;
  //static void NewImageCallBack( Image * pImage , const void * pCallbackData ) ;
  int  m_iTriggerModeIndex;
  bool m_bLoggingON = true ;
private:
  static FXLockObject           m_ConfigLock ;
  static Device::DeviceInfo     m_CamInfo[ MAX_DEVICESNMB ] ;
  static FXStringArray          m_BusyCameras ;
  static int                    m_iCamNum ;
  static CObservedMutex         m_ConfigMutex ;
  static int                    m_iGadgetCount ;
  static DWORD                  m_dwDefaultTimeout ;
  static bool                   m_bCamerasEnumerated ;
  static bool                   m_bRescanCameras ;
  static BaslerCameraInfo       m_GigECamerasInfo[ MAX_DEVICESNMB ] ;
  static UINT                   m_uiNGigECameras;
  static bool                   m_bSaveFullInfo ;
  static BaslerWatcher        * m_pBaslerWatcher ;
  static BaslerPropertyDefine   m_cBProperties[] ;

  COutputConnector * m_pEventsOutput ;
  HANDLE      m_WaitEventFrameArray[ 2 ] ;   // For m_evFrameReady and m_evExit
  // in CameraDoGrab function
  HANDLE      m_WaitEventBusChangeArr[ 2 ] ; // for m_evExit and m_evBusChange
  // in ControlLoop
  HANDLE      m_evFrameReady ;
  HANDLE      m_evBusChange ;
  HANDLE      m_evControlRequestFinished ;
  HANDLE      m_hGrabThreadHandle ;
  HANDLE      m_hCamAccessMutex ;
  DWORD       m_dwGrabThreadId ;
  vFormats    m_AvailableFormats;

  FXLockObject            m_LocalConfigLock ;
  FXLockObject            m_ScanPropLock ;
  FXLockObject            m_PropertyListLock ;
  FXLockObject            m_GrabLock;
  FXLockObject            m_SettingsLock;
  bool                    m_bSelectByGivenName = false ;
  //CInstantCamera          m_Camera;
  CBaslerUniversalInstantCamera          m_Camera;
  //ActiveCameraEvents      m_EventHandlers ; // all camera event handlers
  CameraEventHandler       * m_pEventHandler ;
  CSampleImageEventHandler * m_pImageEventHandle;
  map <string , EventSelectorEnums>  m_EventCorresp;

//  InterfaceType m_IntfType ;
  bool          m_bCameraRemoved ;
  bool          m_bInitialScanDone ;
  DWORD         m_BusEvents ;
  uint32_t      m_LastError;
  int           m_iNoCameraStatus ;
  int           m_iNNoSerNumberErrors ;
  int           m_iSaveSettingsState ;
  bool          m_bInitialized ;
  bool          m_bStopInitialized ;
  bool          m_bBaslerShutDown ;
  bool          m_bIsRunning ;
  int           m_iNSkipImages ;
  uint64_t      m_ui64LastTimeStamp;
  uint64_t      m_ui64PreviousTimeStamp;
  uint32_t      m_uiLastFrameInterval;
  VideoFrameEmbeddedInfo m_LastEmbedInfo ;

  DWORD       m_dwSerialNumber ;
  FXString    m_sSerialNumber ;
  FXString    m_sModelName ;
  FXString    m_sCameraGivenName;
  DWORD       m_dwConnectedSerialNumber ;
  FXString    m_sConnectedSerialNumber ;
  DWORD       m_dwInitializedSerialNumber ;
  FXString    m_sInitializedSerialNumber ;
  bool		    m_FormatNotSupportedDispayed;
  bool        m_bCameraExists = false ;
  double      m_dLastReportTime = 0. ;

  FXString    m_NewCameras ;
  FXString    m_DisconnectedCameras ;

  int                     m_iPropertyIndex ;  // **************
  SetCamPropertyData      m_PropertyData ;  // These 4 variables for 
  bool                    m_bAuto ;           // inter thread data passing
  bool                    m_bInvalidate ;     // on set property procedure
  SetCamPropertyData      m_PropertyDataReturn ;
  SetCamPropertyData      m_PropertyDataSend ;

  //Mode		         m_Mode;
  BITMAPINFOHEADER m_BMIH;
  BITMAPINFOHEADER m_RealBMIH;

  PixelFormatEnums  m_pixelFormat;
  UINT        m_uiPacketSize ;
  BOOL        m_bMaxPacketSize ;
  UINT        m_uiSavedPacketSize;
  float       m_fPercentage ;
  int         m_iFPSx10 ;
  int         m_iWBRed ;
  int         m_iWBlue ;
  int         m_iRestartTimeOut_ms ; // if < 1000 - no restart
  bool        m_bTimeOutDisable ;
  BOOL        m_bEmbedFrameInfo ;
  DWORD       m_dwPrevBusTime;
  DWORD       m_dwLastFramePeriod ;
  DWORD       m_dwPrevBusFrameCnt ;
  float       m_fLastRealFrameRate ;
  __int64     m_i64BurstLength ;

  CRect       m_CurrentROI;  // !!!right is width, bottom is height
  CSize       m_SensorSize ;
  FXString    m_CameraID;
  FXString    m_ControlThreadName ;
  FXString    m_CallbackThreadName ;
  FXString    m_Tabs ;
  //FXArray <StrobeInfo>  m_StrobeInfo ;
  volatile bool m_continueGrabThread;                 //
//  bool m_isSelectingNewCamera;
  bool m_disableEmbeddedTimeStamp;
  //EmbeddedImageInfo m_embeddedInfo;
//   unsigned int m_prevWidth;
//   unsigned int m_prevHeight;

  double m_dLastStartTime ;
  double m_dLastInCallbackTime ;
  double m_dLastFrameTime ;
  BaslerPropertiesArray  m_ThisCamProperty ;
  BaslerPropertiesArray  m_SavedCamProperty ;
  BaslerComplexIOs       m_ComplexIOs ;
  BaslerComplexIOs       m_SavedComplexIOs ;

  //  FXIntArray             m_NotShownProperties ;
  CVideoFrame * m_pNewFrame ;
  pTVFrame      m_pNewImage ;
#ifdef _DEBUG
  FXString m_MutexHolder ;
  int      m_iMutexTakeCntr ;
#endif

  DWORD       m_dwNArrivedEvents ;

  bool        m_bWasStopped ; // = true when camera was stopped on ScanProperties
                              //   or set properties trough control pin
  bool        m_bShouldBeReprogrammed ;
  BOOL        m_bLocalStopped ;
  bool        m_bTriedToRestart ;
  StringList_t m_TriggerSourcesAsList ;
  StringList_t m_LineSelectorsAsList ;
  StringList_t m_LineSourceAsList ;
  FXStringArray   m_LineSourcesAsStrings ;
  CSampleConfigurationEventHandler * m_pConfHandler ;

public:
  Basler();
  ~Basler() ;

  virtual int GetOutputsCount()
  {
    return 2 ;
  }
  virtual COutputConnector * GetOutputConnector( int n )
  {
    if ( n == 0 )
      return m_pOutput ;
    else if ( n == 1 )
      return m_pEventsOutput ;
    return NULL ;
  }
  virtual bool DriverInit();
  virtual bool EnumCameras();
  virtual bool CameraInit();
  virtual void CameraClose();

  virtual bool DriverValid();
  virtual bool DeviceStart();
  virtual void DeviceStop();

  virtual bool GetCameraProperty( FXCamPropertyType Type , INT_PTR &value , bool& bauto );
  virtual bool SetCameraProperty( FXCamPropertyType Type , INT_PTR &value , bool& bauto , bool& Invalidate );
  virtual bool SetCameraPropertyEx( FXCamPropertyType Type , INT_PTR &value , bool& bauto , bool& Invalidate );
  virtual bool GetCameraProperty( int iIndex , INT_PTR &value , bool& bauto );
  virtual bool SetCameraProperty( int iIndex , INT_PTR &value , bool& bauto , bool& Invalidate );
  virtual bool SetCameraPropertyEx( int iIndex , INT_PTR &value , bool& bauto , bool& Invalidate );

  virtual CVideoFrame* DeviceDoGrab( double* StartTime );
  virtual void AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame );
  virtual void LocalStreamStart() ;
  virtual void LocalStreamStop() ;
  virtual bool ScanSettings( FXString& text );
  bool         m_bExpAfterScanSettings = false ;
  bool         m_bGainAfterScanSettings = false ;
  virtual bool ScanProperties( LPCTSTR text , bool& Invalidate ) ;
  virtual bool PrintProperties( FXString& text );
  virtual void ShutDown();
  void         LogError( LPCTSTR Msg ) ;

  // Capture gadget stuff
  DECLARE_RUNTIME_GADGET( Basler );
private:
  bool BuildPropertyList();
  void GetROI( CRect& rc );
  void SetROI( CRect& rc );
  bool SetStrobe( const FXString& STrobeDataAsText , int iIndex ) ;
  void GetStrobe( FXString& StrobeDataAsText , int iIndex ) ;
  CamTriggerMode GetTriggerMode();
  void SetTriggerMode( CamTriggerMode Mode );
protected:
  unsigned int GetBppFromPixelFormat( EPixelType pixelFormat );                           //
  //bool CheckCameraPower(CInstantCamera * am);                                                                 //
  //bool EnableEmbeddedTimeStamp(CInstantCamera* pCam);                              //
  //bool DisableEmbeddedTimeStamp(CInstantCamera* cam);                             //

  // The object grab image loop.  Only executed from within the grab thread.
  static DWORD WINAPI ControlLoop( LPVOID pParam );
  bool IsGrabThreadRunning();        //

  ///** Camera arrival callback handle. */
  //FlyCapture2::CallbackHandle m_cbArrivalHandle;

  ///** Camera removal callback handle. */
  //FlyCapture2::CallbackHandle m_cbRemovalHandle;

  ///** Camera reset callback handle. */
  //FlyCapture2::CallbackHandle m_cbResetHandle;
  static int m_iRegisterCnt ;

  /** Register all relevant callbacks with the library. */
  void RegisterCallbacks();

  /** Unregister all relevant callbacks with the library. */
  void UnregisterCallbacks();

  /**
  * Bus arrival handler that is passed to BusManager::RegisterCallback().
  * This simply emits a signal that calls the real handler.
  *
  * @param pParam The parameter passed to the BusManager::RegisterCallback().
  */
  static void OnBusArrival( void* pParam , unsigned int serialNumber );

  /**
  * Bus removal handler that is passed to BusManager::RegisterCallback().
  * This simply emits a signal that calls the real handler.
  *
  * @param pParam The parameter passed to the BusManager::RegisterCallback().
  */
  static void OnBusRemoval( void* pParam , unsigned int serialNumber );

  static void OnBusReset( void* pParam , unsigned int serialNumber );
  void OnBusRemovalEvent();
  void OnBusArrivalEvent();
  virtual bool BaslerCamInit() ; // Main == true => capture camera
  //      == false => GUI camera
  virtual bool BaslerCameraStart();
  virtual void BaslerCameraStop();
  virtual void BaslerCamClose() ; // Main == true => capture camera
  //      == false => GUI camera
  virtual void BaslerDeleteCam() ;
  virtual void BaslerLocalStreamStart() ;
  virtual void BaslerLocalStreamStop() ;
  virtual bool BaslerBuildPropertyList( bool bOutside = false ) ;
  virtual bool BaslerGetCameraProperty( int iIndex , SetCamPropertyData * pData ) ;
  virtual bool BaslerSetCameraProperty( int iIndex , SetCamPropertyData * pData ) ;
  LPCTSTR GetGadgetInfo() { return ( LPCTSTR ) m_GadgetInfo ; } ;

  inline bool CamCNTLDoAndWait( DWORD EvtMask , DWORD dwTimeOut = m_dwDefaultTimeout )
  {
    if ( m_continueGrabThread )
    {
      FXAutolock al( m_LocalConfigLock ) ;
      m_BusEvents |= EvtMask ;
      ResetEvent( m_evControlRequestFinished ) ;
      SetEvent( m_evBusChange ) ;
      SetEvent( m_evFrameReady ) ;
      if ( m_bTimeOutDisable )
        dwTimeOut = INFINITE ;
#ifdef _DEBUG
      CamTriggerMode Before = TrigNotSupported ;
      CamTriggerMode SetTo = TrigNotSupported ;
      if ( !( EvtMask & BASLER_EVT_BUILD_PROP ) )
      {
        Before = GetTriggerMode();
        if ( EvtMask & BASLER_EVT_SET_PROP )
        {
          if ( m_cBProperties[ m_iPropertyIndex ].Id == TRIGGER_MODE )
            SetTo = ( CamTriggerMode ) m_PropertyData.m_int ;
        }
      }
#endif
      bool bRes = ( WaitForSingleObject( m_evControlRequestFinished , dwTimeOut ) == WAIT_OBJECT_0 ) ;
      if ( !bRes )
      {
        DEVICESENDERR_2( "Cam #%u Error: %s" , m_dwSerialNumber , ( LPCTSTR ) DecodeEvtMask( EvtMask ) ) ;
        TRACE( "Cam #%u Error: %s" , m_dwSerialNumber , ( LPCTSTR ) DecodeEvtMask( EvtMask ) ) ;
        m_BusEvents = 0 ;
      }
#ifdef _DEBUG
      if ( !( EvtMask & BASLER_EVT_BUILD_PROP ) )
      {
        CamTriggerMode After = GetTriggerMode();
        if ( SetTo != TrigNotSupported )
        {
          if ( After != SetTo )
            SEND_GADGET_ERR( "Trigger %d is not set to %d " , After , SetTo ) ;
        }
//         else if ( After != Before )
//         {
//           SEND_GADGET_ERR( "Trigger changed from %d to %d after EvMask 0x%X %s access" , Before , After ,
//             EvtMask , GetThisCamPropertyName( m_iPropertyIndex ) ) ;
//         }
      }
#endif
      return bRes ;
    }
    return true ;
  }
  bool BaslerCheckAndStopCapture() ;
  bool BaslerInitAndConditialStart() ;
  inline bool CheckAndStopCapture()
  {
    bool bWasRunning = IsRunning() ;
    if ( bWasRunning )
    {
      bool bRes = CamCNTLDoAndWait( BASLER_EVT_STOP_GRAB , 1000 ) ;
      Sleep( 100 ) ;
      m_bWasStopped = true ;
    }
    return bWasRunning ;
  }

  inline bool GetCamMutex(
#ifdef _DEBUG
    LPCTSTR MutexHolder = NULL
#endif
  )
  {
// #ifdef _DEBUG
//     ASSERT( m_iMutexTakeCntr == 0 ) ;
// #endif


    DWORD dwRes = WaitForSingleObject( m_hCamAccessMutex , 1000 ) ;
#ifdef _DEBUG
    if ( dwRes == WAIT_OBJECT_0 )
    {
      ASSERT( m_iMutexTakeCntr == 0 ) ;
      m_MutexHolder = MutexHolder ;
      m_iMutexTakeCntr++ ;
    }
#endif
    return ( dwRes == WAIT_OBJECT_0 ) ;
  }
  inline CInstantCamera * GetCamera(
#ifdef _DEBUG
    LPCTSTR MutexHolder = NULL
#endif
  )
  {
    if ( GetCamMutex(
#ifdef _DEBUG
      MutexHolder
#endif
    ) )
    {
      return &m_Camera ;
    }
    return NULL ; // can't take mutex
  }
  inline void ReleaseCamera()
  {
    ReleaseMutex( m_hCamAccessMutex ) ;
#ifdef _DEBUG
    ASSERT( ( --m_iMutexTakeCntr ) == 0 );
    m_MutexHolder.Empty();
#endif
  }
public:
  bool CheckAndAllocCamera( void );
  static int CameraRemovedCB( Basler * pGadget , LPCTSTR pszRemovedSN ) ;
  bool SetBMIH( void );
  int GetColorMode();
  int SetPacketSize( int iPacketSize_Or_FPSx10 , bool bFrameRate );
  int GetMaxPacketSize();
  int SetMaxPacketSize();
  virtual void DeviceTriggerPulse( CDataFrame* pDataFrame );
  bool OtherThreadSetProperty( int iIndex , SetCamPropertyData * pData , bool * bInvalidate )
  {
    FXAutolock al( m_SettingsLock ) ;
    pData->m_iIndex = iIndex ;
    pData->m_Type = m_ThisCamProperty[ iIndex ].id ;
    m_PropertyData = *pData ;
    m_iPropertyIndex = iIndex ;
    m_bInvalidate = false ;
    bool bRes = CamCNTLDoAndWait( BASLER_EVT_SET_PROP ) ;
    if ( bRes )
    {
      *bInvalidate |= m_PropertyData.m_bInvalidate ;
      *pData = m_PropertyData ;
      return true ;
    }
    return false ;
  }
  bool OtherThreadGetProperty( int iIndex , SetCamPropertyData * pData )
  {
    FXAutolock al( m_SettingsLock ) ;
    m_PropertyData.Reset() ;
    m_iPropertyIndex = iIndex ;
    pData->m_iIndex = iIndex ;
    pData->m_Type = m_ThisCamProperty[ iIndex ].id ;
    m_bInvalidate = false ;
    bool bRes = CamCNTLDoAndWait( BASLER_EVT_GET_PROP ) ;
    *pData = m_PropertyData ;
    return bRes ;
  }
  int GetThisCamPropertyCnt()
  {
    return ( int ) m_ThisCamProperty.GetCount() ;
  }
  LPCTSTR GetThisCamPropertyName( FXCamPropertyType Type ) ;
  LPCTSTR GetThisCamPropertyName( int iIndex ) ;
  FXCamPropertyType GetThisCamPropertyType( int iIndex ) ;
  int GetThisCamPropertyIndex( FXCamPropertyType Type ) ;
  LPCTSTR GetThisCamInDeviceName( FXCamPropertyType Type ) ;
  int GetThisCamPropertyIndex( LPCTSTR name ) ;
  BaslerCamProperty * GetThisCamProperty( FXCamPropertyType Type ) ;
  FXCamPropertyType GetThisCamPropertyId( LPCTSTR name );
  PropertyDataType GetPropertyDataType( LPCTSTR name ) ;
  PropertyDataType GetPropertyDataType( int iIndex ) ;
  PropertyDataType GetPropertyDataType( FXCamPropertyType Type ) ;
  int64_t Basler::GetIntValue( LPCTSTR pName ) ;
  int64_t Basler::GetIntValue( LPCTSTR * pName ) ;
  void SetXOffset( int64_t i64Val ) ;
  void SetYOffset( int64_t i64Val ) ;
  void SetIntValue( LPCTSTR pName , int64_t i64Val ) ;
  void SetIntValue( LPCTSTR * pName , int64_t i64Val ) ;
  INode * GetPropertyNode( LPCTSTR pNodeName ) ;
  INode * FindValidCameraPropertyName(
    BaslerPropertyDefine& Property , LPCTSTR * ppValidName ) ;
  INode * FindValidCameraPropertyName(
    FXCamPropertyType Type , LPCTSTR * ppValidName ) ;
  bool GetEnumeratorsAsText( INode * pEnumeratedNode ,
    FXString& Result , BaslerCamProperty& Prop ) ;
  bool GetEnumeratorsAsText( NodeList_t& entries ,
    FXString& Result , BaslerCamProperty& Prop ) ;



  FXString DecodeEvtMask( DWORD dwEVtMask )
  {
    FXString Answer ;
    if ( dwEVtMask & BASLER_EVT_SHUTDOWN )
      Answer += "ShutDown " ;
    if ( dwEVtMask & BASLER_EVT_DRIVER_INIT )
      Answer += "DrvInit " ;
    if ( dwEVtMask & BASLER_EVT_RELEASE )
      Answer += "Release " ;
    if ( dwEVtMask & BASLER_EVT_RESTART )
      Answer += "Restart " ;
    if ( dwEVtMask & BASLER_EVT_INIT )
      Answer += "CamInit " ;
    if ( dwEVtMask & BASLER_EVT_START_GRAB )
      Answer += "Start " ;
    if ( dwEVtMask & BASLER_EVT_STOP_GRAB )
      Answer += "Stop " ;
    if ( dwEVtMask & BASLER_EVT_SET_PROP )
      Answer += "SetProp " ;
    if ( dwEVtMask & BASLER_EVT_GET_PROP )
      Answer += "GetProp " ;
    if ( dwEVtMask & BASLER_EVT_BUILD_PROP )
      Answer += "BuildProp " ;
    if ( dwEVtMask & BASLER_EVT_LOCAL_START )
      Answer += "LocStart " ;
    if ( dwEVtMask & BASLER_EVT_LOCAL_STOP )
      Answer += "LocStop " ;
    return Answer ;
  }
  unsigned GetPropertyId( LPCTSTR name )
  {
    int i;
    unsigned retV = WRONG_PROPERTY;
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
  unsigned GetPropertyIndex( LPCTSTR name )
  {
    int i;
    unsigned retV = -1 ;
    for ( i = 0; i < m_ThisCamProperty.GetSize(); i++ )
    {
      if ( m_ThisCamProperty[ i ].name.CompareNoCase( name ) == 0 )
        return ( unsigned ) i ;
    }
    return -1 ;
  }

  bool SetFrameRate( double dFrameRate );
  bool GetFrameRate( double& dFrameRate );
  int GetGain_dBx10( BaslerCamProperty * pProp ) ;
  int SetGain_dBx10( BaslerCamProperty * pProp , int iGain_dBx10 , bool bAuto ) ;
  int GetShutter_us( BaslerCamProperty * pProp ) ;
  int SetShutter_us( BaslerCamProperty * pProp , int iVal_us , bool bAuto ) ;

  void CameraDisconnected( FXStringArray& DisconnectedCameras ) ;
  INodeMap* Basler::GetNodeMapWithCatching() ;
  LPCTSTR * _getPropertyInCamNames( int id ) ;
  LPCTSTR _getPropertyname( int id ) ;
  int _getPropertyCount() ;
  PropertyDataType GetType( INode * pNode ) ;
  void HandleNode( INode * pNode , FILE * pSaveFile ) ;
  void NewCameraCallback( FXStringArray& NewCameras ) ;
  int PrintCameraEventsProperties( FXPropertyKit& pk ) ;
  int PrintCameraTimersProperties( FXPropertyKit& pk , bool bOnlyComplexIOsFill = false ) ;
  int PrintCameraLinesProperties( FXPropertyKit& pk , bool bOnlyComplexIOsFill = false ) ;
  int PrintCameraUserOutputsProperties( FXPropertyKit& pk , bool bOnlyComplexIOsFill = false ) ;
  void SaveCameraInfo( const CInstantCamera& camera , int iIndex ) ;
  void SaveCameraInfo( const CDeviceInfo& DevInfo , int iIndex ) ;
  int ScanCameraEventsProperties( FXPropKit2& pk ) ;
  int ScanTimersProperties( LPCTSTR pTimerName , FXPropKit2& pc );
  int ScanLineProperties( LPCTSTR pLineName , FXPropKit2& pc );
  EventSelectorEnums ConvertNameToEnum( LPCTSTR pName );

  int SetComplexIOs( BaslerComplexIOs& Params );
  string GetPixelFormatName( DWORD FormatAsDWORD );
  int WriteToFile( char * pData , int iDataLen , FILE * pSaveFile ) ;
};


#endif // !defined(AFX_Basler_H__0C071980_0F39_40DC_A144_B6BEDB69E5DC__INCLUDED_)

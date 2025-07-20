// PleoraCamera.h: interface for the PleoraCamera class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PleoraCamera_H__0C071980_0F39_40DC_A144_B6BEDB69E5DC__INCLUDED_)
#define AFX_PleoraCamera_H__0C071980_0F39_40DC_A144_B6BEDB69E5DC__INCLUDED_

#include <queue>
#include <1394Camera.h>
#include <PvSystem.h>
#include <PvInterface.h>
#include <PvDevice.h>
#include <PvBuffer.h>
#include <PvStreamBase.h>
#include <PvStream.h>
#include <PvStreamRaw.h>
#include <PvPipeline.h>
#include "ErrorCodeToMessage.h"
#include "helpers\SharedMemBoxes.h"
#include "helpers\FramesHelper.h"
// namespace AVT {
//   namespace VmbAPI {

#define ARR_SIZE(x) ( sizeof(x)/sizeof(x[0]))
#define LENARR(x) (sizeof(x)/sizeof(x[0]))
#define N_FRAMES_FOR_ALLOCATE (20)

enum FG_COLORMODE  // declared for compatibility with AVT FireGrab
{
  CM_UNKNOWN = -1 ,
  CM_Y8=0,
  CM_YUV411,
  CM_YUV422,
  CM_YUV444,
  CM_RGB8,
  CM_Y16,
  CM_RGB16,
  CM_SY16,
  CM_SRGB16,
  CM_RAW8,
  CM_RAW16,
  CM_LAST
};

typedef struct tagVideoformats
{
  PvPixelType  vm;
  const char *        name; 
  bool                supported;
  double              dBytesPerPixel ;
  FG_COLORMODE        AVT1394_ColorMode ;
  unsigned            BMP_OutCompression ;

} Videoformats;

typedef struct 
{
  unsigned long ulFrameCnt ;
  long          lUserValue ;
  unsigned long ulExposure ;
  unsigned long ulGain     ;
  short         shSyncIn ;
  short         shSyncOut ;
  BYTE          Unknown[20] ;
  int           iChunkId ;
  int           iChunkLen ;
} AncillaryInfo ;




// Parameters enumerator taken from avt 1394 cameras SDK
// All 1394 bus parameters are excluded, except of packet size
//   which has the same meaning
enum FG_PARAMETER
{
  FGP_IMAGEFORMAT=0,                            // Compact image format
  FGP_ENUMIMAGEFORMAT,                          // Enumeration (Reset,Get)
  FGP_BRIGHTNESS,                               // Set image brightness
  FGP_AUTOEXPOSURE,                             // Set auto exposure
  FGP_SHARPNESS,                                // Set image sharpness
  FGP_WHITEBALCB,                               // Blue
  FGP_WHITEBALCR,                               // Red
  FGP_WHITEBAL_RATIO,                           // UV ratio - alternative white balance control
  FGP_WHITEBAL_SELECT,                          // Base colors selector - ------------"--------
  FGP_HUE,                                      // Set image hue
  FGP_SATURATION,                               // Set color saturation
  FGP_GAMMA,                                    // Set gamma
  FGP_SHUTTER,                                  // Shutter time
  FGP_GAIN,                                     // Gain
  FGP_SENSOR_GAIN,                              // Sensor Gain
  FGP_IRIS,                                     // Iris
  FGP_FOCUS,                                    // Focus
  FGP_TEMPERATURE,                              // Color temperature
  FGP_TRIGGER,                                  // Trigger
  FGP_TRIGGERDLY,                               // Delay of trigger
  FGP_WHITESHD,                                 // Whiteshade
  FGP_FRAMERATE,                                // Frame rate
  FGP_ZOOM,                                     // Zoom
  FGP_PAN,                                      // Pan
  FGP_TILT,                                     // Tilt
  FGP_OPTICALFILTER,                            // Filter
  FGP_CAPTURESIZE,                              // Size of capture
  FGP_CAPTUREQUALITY,                           // Quality
  FGP_PHYSPEED,                                 // Set speed for asy/iso

  FGP_LAST
};

#define FGP_FRAME_RATE        ((FG_PARAMETER)(FGP_IMAGEFORMAT-13))
#define FGP_RESOLUTION        ((FG_PARAMETER)(FGP_IMAGEFORMAT-2))
#define FGP_ROI               ((FG_PARAMETER)(FGP_IMAGEFORMAT-3))
#define FGP_TRIGGERONOFF      ((FG_PARAMETER)(FGP_IMAGEFORMAT-4))
#define FGP_EXTSHUTTER        ((FG_PARAMETER)(FGP_IMAGEFORMAT-5))
#define FGP_SEND_FINFO        ((FG_PARAMETER)(FGP_IMAGEFORMAT-6))
#define FGP_TRIGGERDELAY      ((FG_PARAMETER)(FGP_IMAGEFORMAT-7))
#define FGP_GRAB              ((FG_PARAMETER)(FGP_IMAGEFORMAT-8))
#define FGP_PACKET_SIZE       ((FG_PARAMETER)(FGP_IMAGEFORMAT-9))                               // Packet size
#define FGP_TRIG_EVENT        ((FG_PARAMETER)(FGP_IMAGEFORMAT-10)) 
#define FGP_ACQU_MODE         ((FG_PARAMETER)(FGP_IMAGEFORMAT-11)) 
#define FGP_BANDWIDTH         ((FG_PARAMETER)(FGP_IMAGEFORMAT-12)) 

#define FGP_EXP_AUTO_OUT      ((FG_PARAMETER)(FGP_IMAGEFORMAT-14))
#define FGP_EXP_AUTO_ALG      ((FG_PARAMETER)(FGP_IMAGEFORMAT-15))
#define FGP_GAIN_AUTO_TARG    ((FG_PARAMETER)(FGP_IMAGEFORMAT-16))
#define FGP_GAIN_AUTO_OUT     ((FG_PARAMETER)(FGP_IMAGEFORMAT-17))
#define FGP_GAIN_PV           ((FG_PARAMETER)(FGP_IMAGEFORMAT-18))
#define FGP_OUT_DELAY         ((FG_PARAMETER)(FGP_IMAGEFORMAT-19))
#define FGP_LINE_SELECT       ((FG_PARAMETER)(FGP_IMAGEFORMAT-20))
#define FGP_LINE_SOURCE       ((FG_PARAMETER)(FGP_IMAGEFORMAT-21))
#define FGP_LINE_INVERSE      ((FG_PARAMETER)(FGP_IMAGEFORMAT-22))
#define FGP_LINE_PARAMS       ((FG_PARAMETER)(FGP_IMAGEFORMAT-23))
#define FGP_LINE_DEBOUNCE     ((FG_PARAMETER)(FGP_IMAGEFORMAT-24))
#define FGP_TRIGGER_POLARITY  ((FG_PARAMETER)(FGP_IMAGEFORMAT-25))
#define FGP_LINEIN_SELECT     ((FG_PARAMETER)(FGP_IMAGEFORMAT-26))
#define FGP_LINEOUT_SELECT    ((FG_PARAMETER)(FGP_IMAGEFORMAT-27))
#define FGP_LINEOUT_POLARITY  ((FG_PARAMETER)(FGP_IMAGEFORMAT-28))
#define FGP_LINEOUT_SOURCE    ((FG_PARAMETER)(FGP_IMAGEFORMAT-29))
#define FGP_BINNING           ((FG_PARAMETER)(FGP_IMAGEFORMAT-30))
#define FGP_TRIGGER_SOURCE    ((FG_PARAMETER)(FGP_IMAGEFORMAT-31))
#define FGP_USER_SET_SELECT   ((FG_PARAMETER)(FGP_IMAGEFORMAT-32))
#define FGP_USER_SET_DEF      ((FG_PARAMETER)(FGP_IMAGEFORMAT-33))
#define FGP_TEMPERATURE_S     ((FG_PARAMETER)(FGP_IMAGEFORMAT-34))
#define FGP_LOG               ((FG_PARAMETER)(FGP_IMAGEFORMAT-35))
#define FGP_ACQUISITION_SRC   ((FG_PARAMETER)(FGP_IMAGEFORMAT-36))
#define FGP_SYNC_SIGNAL       ((FG_PARAMETER)(FGP_IMAGEFORMAT-37))
#define FGP_ARM_POLARITY      ((FG_PARAMETER)(FGP_IMAGEFORMAT-37))

typedef CPtrList PvFrames ;

#define  BUS_EVT_ARRIVED   0x001 
#define  BUS_EVT_REMOVED   0x002 
#define  BUS_EVT_BUS_RESET 0x004
#define  CAM_EVT_SHUTDOWN  0x100
#define  CAM_EVT_START_GRAB 0x200
#define  CAM_EVT_STOP_GRAB  0x400
#define  CAM_EVT_RELEASE    0x800
#define  CAM_EVT_RESTART    0x1000
#define  CAM_EVT_INIT       0x2000
#define  CAM_EVT_LOCAL_STOP  0x4000
#define  CAM_EVT_LOCAL_START 0x8000
#define  CAM_EVT_SET_PROP    0x10000
#define  CAM_EVT_DRIVER_INIT 0x20000
#define  CAM_EVT_BUILD_PROP  0x40000
#define  CAM_EVT_GET_PROP    0x80000
#define  CAM_EVT_LOG         0x100000

#define COLOR_BW          0 // no color info
#define COLOR_BY_SOFTWARE 1 // RAW image received, convert to color
#define COLOR_BY_CAMERA   2 // Camera did color coding (YUV, RGB, BGR formats received)

#define SEND_DEVICE_INFO(sz,...)        FxSendLogMsg(MSG_INFO_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)
#define SEND_DEVICE_TRACE(sz,...)        FxSendLogMsg(MSG_DEBUG_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)
#define SEND_DEVICE_WARN(sz,...)        FxSendLogMsg(MSG_WARNING_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)
#define SEND_DEVICE_ERR(sz,...)        FxSendLogMsg(MSG_ERROR_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)
#define SEND_DEVICE_FAIL(sz,...)        FxSendLogMsg(MSG_CRITICAL_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)

union PropertyValue
{
  bool     bBool ;
  int      iInt ;
  double   dDouble ;
  char     szAsString[300] ;
};

class CamProperty
{
public:
  FG_PARAMETER        pr;
  const char *        name;
  const char *        CamPropertyName ;
  const char *        AutoControl ;
  void       *        pInfo ;
  bool                m_bSupported ;
  PvGenParameter *    m_pInCamera ;
  PvGenType           m_DataType ;
//   PropertyValue       m_LastValue ;
//   FXStringArray       m_EnumerateNames ;
//   FXParser            m_DlgFormat ;

//   CamProperty() { memset( this , 0 , sizeof( ( LPBYTE )&m_EnumerateNames - ( LPBYTE )pr ) ) ; }
//   CamProperty& operator=( CamProperty& Orig )
//   {
//     memcpy( this , &Orig.pr , sizeof( ( LPBYTE )&m_EnumerateNames - ( LPBYTE )pr ) ) ;
//     m_EnumerateNames.Copy( Orig.m_EnumerateNames ) ;
//     m_DlgFormat = Orig.m_DlgFormat ;
//   }
} ;

struct CamTypeAndProperties
{
  LPCTSTR     CamType ;
  int         iNProperties ;
  CamProperty * Properties ;
  PvDeviceInfo * m_pDeviceInfo ;
}  ;

class PleoraCamInfo : public CAMERA1394::CameraInfo
{
public:
  PvGenParameterArray * m_Features ;
};

class SetCamPropertyData
{
public:
  SetCamPropertyData() { Reset() ;}
  void    Reset() { memset( &m_Type , 0 , (LPBYTE)m_szString - (LPBYTE)&m_Type + 1 ) ;}
  PvGenType m_Type ;
  char      m_Name[ 100 ] ;
  PvGenParameter * m_pParameter ;
  bool    m_bAuto ;
  PvInt32 m_int ;
  PvInt64 m_int64 ;
  double  m_double ;  
  bool    m_bBool ;
  bool    m_bInvalidate ;
  char    m_szString[300] ;
  PvUInt8 m_Buffer[ 100 ] ;
  SetCamPropertyData& operator=( SetCamPropertyData& Orig )
  {
    memcpy( this , &Orig , sizeof( *this ) - sizeof(m_szString) )  ;
    strcpy_s( m_szString , Orig.m_szString ) ;
    memcpy( m_Buffer , Orig.m_Buffer , sizeof( m_Buffer ) ) ;
    return *this ;
  }
};
class CameraAttribute
{
public:
  CameraAttribute( FG_PARAMETER init_pr = FGP_LAST , 
    LPCTSTR pName = NULL , LPCTSTR pCamName = NULL ) 
  { 
    memset( &m_Type , 0 , ( LPBYTE )&m_Name - ( LPBYTE )&m_Type ) ;
    m_CameraPropertyName = pCamName ;
    m_Name = pName ;
    pr = init_pr ;
  }
  PvGenType           m_Type ;
  FG_PARAMETER        pr;
  bool                m_bIsSupported ;
  int                 m_iRange[2] ;
  double              m_dRange[2] ;
  PvInt64             m_i64Range[2] ;
  PvInt64             m_i64Val ;
  bool                m_boolVal ;
  PvInt32             m_intVal ;
  double              m_dVal ;
  FXString            m_Name ;
  FXString            m_CameraPropertyName ;
  PvString            m_Info ;
  FXStringArray       m_EnumRange ;
  FXString            m_enumVal ;
  FXString            m_stringVal ;
  FXString            m_AutoControl ;
  PvString            m_Unit ;
  FXParser            m_DlgFormat ;
  FXString            m_Description ;
  CameraAttribute& operator = (const CameraAttribute& Orig)
  {
    memcpy( &m_Type , &Orig.m_Type , ( LPBYTE )&m_Name - ( LPBYTE )&m_Type ) ;
    m_Name = Orig.m_Name ;
    m_CameraPropertyName = Orig.m_CameraPropertyName ;
    m_Info = Orig.m_Info ;
    m_EnumRange.RemoveAll() ;
    m_EnumRange.Append( Orig.m_EnumRange );
    m_enumVal = Orig.m_enumVal ;
    m_stringVal = Orig.m_stringVal ;
    m_AutoControl = Orig.m_AutoControl ;
    m_Unit = Orig.m_Unit ;
    m_DlgFormat = Orig.m_DlgFormat ;
    m_Description = Orig.m_Description ;
    return *this ;
  }
  LPCTSTR GetEnumByIndex( int iIndex )
  {
    if ( iIndex >= 0  &&  iIndex < m_EnumRange.GetCount() )
    {
      return (LPCTSTR)m_EnumRange[ iIndex ] ;
    }
    return NULL ;
  }
  int GetEnumRangeLength() { return m_EnumRange.GetCount() ; }
};
typedef CArray<CameraAttribute> CamProperties ;

class PleoraCamera ;

class BusyCamera 
{
public:
  BusyCamera( DWORD dwSN = 0 , PleoraCamera * pGadget = NULL ) 
  {
    m_dwSerialNumber = dwSN ;
    m_pGadget = pGadget ;
  }
  DWORD  m_dwSerialNumber ;
  PleoraCamera * m_pGadget ;
};

BOOL __stdcall FrameDoneCB(const PvBuffer * pFrame , void * pClient ) ;
typedef FXArray<PvBuffer*> PvBufferArray ;

class PleoraCamera : public C1394Camera /*, PvGenEventSink*/ 
{
  CVideoFrame * ConvertPleoraToSHformat( const PvImage * pFrame  ) ;
  enum CamTriggerMode
  {
    TrigNotSupported = -1 ,
    TriggerOff ,
    TriggerOn
  } m_TriggerMode ;
public:  
  PleoraCamera();
  ~PleoraCamera() ;
  virtual bool DriverInit();
  virtual bool EnumCameras();
  virtual bool CameraInit();
  virtual void CameraClose();

  virtual bool DriverValid();
  virtual bool CameraStart();
  virtual void CameraStop();

  virtual bool GetCameraProperty( unsigned i , int &value , bool& bauto );
  virtual bool SetCameraProperty( unsigned i , int &value , bool& bauto , bool& Invalidate );
  virtual bool SetCameraPropertyEx( unsigned i , SetCamPropertyData * pData , bool& Invalidate );
  virtual bool GetCameraPropertyEx( int iIndex , SetCamPropertyData& Value );
  virtual bool GetCameraPropertyEx( LPCTSTR pszPropertyName , SetCamPropertyData& Value );

  virtual CVideoFrame* CameraDoGrab( double* StartTime );
  virtual void AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame );
  virtual bool ScanSettings( FXString& text );
  virtual bool ScanProperties( LPCTSTR text , bool& Invalidate ) ;
  virtual bool PrintProperties( FXString& text );
  virtual void ShutDown();
  bool         GetCameraInfoBySN( DWORD dwSN , PvDeviceInfo ** pDevInfo ) ;
  void         LogError( LPCTSTR Msg ) ;
  /**
  * Bus arrival handler that is passed to BusManager::RegisterCallback().
  * This simply emits a signal that calls the real handler.
  *
  * @param pParam The parameter passed to the BusManager::RegisterCallback().
  */
  static void OnBusArrival( void* pParam , LPCTSTR szSerNum );

  /**
  * Bus removal handler that is passed to BusManager::RegisterCallback().
  * This simply emits a signal that calls the real handler.
  *
  * @param pParam The parameter passed to the BusManager::RegisterCallback().
  */
  static void OnBusRemoval( void* pParam , LPCTSTR szSerNum );

  static void OnBusReset( void* pParam , LPCTSTR szSerNum );
  // Camera observer for camera connect-disconnect indication

  // Capture gadget stuff
  DECLARE_RUNTIME_GADGET( PleoraCamera );
  PvResult        StartUp();
  PvResult        StartContinuousImageAcquisition( const PvString &rStrCameraID );
  PvResult        StopContinuousImageAcquisition();

  PvUInt32        GetWidth() { return m_nWidth; }
  PvUInt32        GetHeight() { return m_nHeight; }
  PvPixelType     GetPixelFormat() ;
  PvBuffer *            GetFrame();
  void                ClearFrameQueue();
  PvResult        QueueFrame( PvBuffer * pFrame );
  void SaveCameraInfo( PvInterface * pInterf , 
    int iIndexOnInterf , int iEnumIndex , FILE * pFile = NULL ) ;
  bool            ReprogramStream() ;
  string_type         GetVersion() const;
  // PvGenEventSink implementation
  void OnParameterUpdate( PvGenParameter *aParameter );
private:
  COutputConnector *  m_pLogOutput ;

  // A Pleora system
  PvSystem      m_System;
  PvInterface * m_pInterface ;
  FXStringArray m_AvailableInterfaces ;
  
  // The currently streaming camera, stream and pipeline
  PvDevice       m_Camera ;
  PvDeviceInfo * m_pDeviceInfo ;
  PvStream       m_Stream ;
  PvPipeline     m_Pipeline ;
  PvInt64        m_i64Payload ;
  int            m_iNFramesForAllocation ;
  PvInt64        m_i64FrameCnt ;
  double         m_dFrameRate ;
  double         m_dBandwidth ;
  PvInt64   *    m_pi64Tmp ;
  PvInt64        m_i64Tmp ;


 
  // Instance counter
  static DWORD m_dwInstanceCount ;
  // Every camera has its own frame observer
//   AVT::VmbAPI::IFrameObserverPtr m_pFrameObserver;
  // The current width
  PvInt32 m_nWidth;
  // The current height
  PvInt32 m_nHeight;
  PvInt32 m_nSensorWidth ;
  PvInt32 m_nSensorHeight ;
  CamProperties           m_PropertiesEx ;
  CamProperties           m_AllCamProperties ;
  const CamProperty *     m_pOrigProperties ;
  int                     m_iNProperties ;
private:
  static FXLockObject           m_ConfigLock ;
  static CAMERA1394::CameraInfo m_CamInfo[ MAX_CAMERASNMB ] ;
  static FXArray<BusyCamera>    m_BusyCameras ;
  static int                    m_iCamNum ;
  static DWORD                  m_dwDefaultTimeout ;
  static bool                   m_bSaveFullInfo ;

  CAMERA1394::CameraInfo        m_CameraInfo ;
  FXLockObject            m_LocalConfigLock ;
  FXLockObject            m_CamControlLock ;
  FXLockObject            m_GrabLock;
  FXLockObject            m_SettingsLock;
  TCHAR                   m_TmpPropertyName[ 100 ] ;
  int                     m_iPropertyIndex ;      // **************
  SetCamPropertyData      m_PropertyData ;        // These 4 variables for 
  bool                    m_bAuto ;               // inter thread data passing
  bool                    m_bInvalidate ;         // on set property procedure

  bool                    m_bViewErrorMessagesOnGetSet ;

  int                     m_iSelectedLineNumber ; // for binary IO signals
  int                     m_iInSelectedLineNumber ; // for binary IO signals
  int                     m_iOutSelectedLineNumber ; // for binary IO signals
  FXString                m_SelectedLine ;
  FXString                m_InSelectedLine ;
  FXString                m_OutSelectedLine ;
  FXString                m_SelectedLineParams ;
  FXString                m_InSelectedLineParams ;
  FXString                m_OutSelectedLineParams ;
  FXString                m_TriggerSourceName ;
  FXString                m_TriggerModeName ;
  FXString                m_TriggerSourceEnums ;
  FXString                m_SelectedTriggerSource ;
  FXString                m_TriggerOn , m_TriggerOff ;
  double                  m_dTemperatures[ 10 ] ;

  FXString                m_PropertiesForLogAsString ;
  FXStringArray           m_PropertiesForLog ;
  double                  m_dLogPeriod_ms ;
  int                     m_iLogCnt ; // >= 0 - how many items checked for log
                                      // if == m_PropertiesForLog.GetUpperBound()
                                      // log string will be sent out
  FXString                m_LogOutString ;
  FXLockObject            m_LogLock ;
  double                  m_dLastLogTime_ms ;

  FXString                m_SaveSettingsCommand ;
  FXString                m_LoadSettingsCommand ;
  int                     m_iNTemperatures ;
  FXString                m_UserSetSelector ;
  FXString                m_SetDefaultSettings ;
  bool                    m_bSetDefaultSettingsIsEnum ;

  FXString                m_LastPrintedProperties ;
  double                  m_dLastProperiesPrintTime ;
  FXString                m_LastPrintedSettings ;
  FXString                m_GadgetInfo ;
  double                  m_dLastSettingsPrintTime ;
  double                  m_dLastBuiltPropertyTime ;

  DWORD               m_BusEvents ;
  PvResult            m_Result;
  enum CameraStatus
  {
    NotInitialized = 0 ,
    CantGetCameras ,
    CantOpenCameraForRead ,
    CantGetFeatures ,
    DriverInitialized ,
    CameraClosed ,
    CameraAlreadyClosed ,
    CameraConnected ,
    PropertyListBuilt ,
    CantInit ,
    ClosedAfterFaultOnStart ,
    CameraStarted , 
    CameraStopped
  } m_CameraStatus ;
  int         m_iNNoSerNumberErrors ;
  bool        m_bInitialized ;
  bool        m_bStopInitialized ;
  bool		    m_FormatNotSupportedDispayed;
  bool        m_bCamerasEnumerated ;
  bool        m_bRescanCameras ;

  bool        m_bWasStopped ; // = true when camera was stopped on ScanProperties
  //   or set properties trough control pin
  bool        m_bShouldBeReprogrammed ;
  BOOL        m_bLocalStopped ;
  bool        m_disableEmbeddedTimeStamp ;
  bool        m_bIsOpened ;
  bool        m_bGrabLoopContinue ;
  bool        m_bStreamProgrammed ;

  FXString    m_szSerialNumber ;
  DWORD       m_dwSerialNumber ;
  DWORD       m_dwConnectedSerialNumber ;

//   Mode		    m_Mode;
  BITMAPINFOHEADER m_BMIH;
  BITMAPINFOHEADER m_RealBMIH;

  // The current pixel format
//   PvInt64  m_nPixelFormat;
  PvPixelType m_pixelFormat; // necessary pixel format
  UINT        m_uiPacketSize ;
  CSize       m_SensorSize ;
  float       m_fPercentage ;
  int         m_iFPSx10 ;
  FXString    m_FPSPropertyName ;
  double      m_dExtShutter ;
  int         m_iWBRed ;
  int         m_iWBlue ;
  int         m_iSyncSignalMode ; // 0 - ARM, i.e. sync in

  std::queue<CVideoFrame*> m_ReadyFrames ;
  PvBuffer *  m_pBuffers ;
  UINT        m_uiNBuffers ;
  
  
  CRect       m_CurrentROI;
  FXString    m_CameraID;
  FXString    m_CallbackThreadName ;
  FXString    m_TmpString ;
  //FXArray <StrobeInfo>  m_StrobeInfo ;
//  EmbeddedImageInfo m_embeddedInfo;
//   unsigned int m_prevWidth;
//   unsigned int m_prevHeight;

  double m_dLastStartTime ;
  double m_dLastInCallbackTime ;
  CVideoFrame * m_pNewFrame ;
  PvImage     * m_pNewImage ;
  HANDLE      m_WaitEventFrameArray[2] ;   // For m_evFrameReady and m_evExit
                                           // in CAmeraDoGrab function
  HANDLE      m_WaitEventBusChangeArr[3] ; // for m_evCameraControl , m_evExit, m_evBusChange, 
                                           // in DoGrabLoop
  HANDLE      m_evFrameReady ;
  HANDLE      m_evBusChange ;
  HANDLE      m_hCamAccessMutex ;
  HANDLE      m_hGrabThreadHandle ;
  DWORD       m_dwGrabThreadId ;
#ifdef _DEBUG
      FXString m_MutexHolder ;
      int      m_iMutexTakeCntr ;
#endif

  DWORD       m_dwNArrivedEvents ;
public:
private:
  // The camera control thread function
  static DWORD WINAPI CameraGrabLoop( LPVOID pParam );
  DWORD WINAPI GrabLoop() ;
  bool PleoraSaveOrLoadSettings( int iMode ) ; // 0 - nothing to do, 1 - Save, 2 - Load
  void LocalStreamStart( void ) ;
  void LocalStreamStop( void ) ;
  bool BuildPropertyList();
  bool GetROI(CRect& rc);
  void SetROI(CRect& rc);
  bool SetROI( LPCTSTR pROIAsText );
  bool SetStrobe( const FXString& STrobeDataAsText ,  int iIndex ) ;
  void GetStrobe( FXString& StrobeDataAsText , int iIndex ) ;

protected:
  void GetCamResolutionAndPixelFormat(unsigned int* rows,
    unsigned int* cols, PvPixelType* pixelFmt); //
  unsigned int GetBppFromPixelFormat( PvPixelType pixelFormat );                           //

  bool         GetULongFeature( /*PvGenParameterArray * Pars ,*/
    LPCTSTR pFeatureName , PvUInt32& ulValue ) ;
  bool         SetULongFeature( /*PvGenParameterArray * Pars ,*/
    LPCTSTR pFeatureName , PvUInt32 ulValue ) ;
  bool         GetIntFeature( /*PvGenParameterArray * Pars ,*/
    LPCTSTR pFeatureName , PvInt32& ulValue ) ;
  bool         SetIntFeature( /*PvGenParameterArray * Pars ,*/
    LPCTSTR pFeatureName , PvInt32 ulValue ) ;
  bool SetPropertyValue( PvGenParameterArray * Pars ,
    SetCamPropertyData& Value ) ;
  bool SetPropertyValue( PvGenParameterArray * Pars ,
    LPCTSTR Name , SetCamPropertyData& Value ) ;
  bool         SetPropertyValue( SetCamPropertyData& Value ) ;
  bool         SetPropertyValue(
    LPCTSTR pName , SetCamPropertyData& Value ) ;
  bool GetPropertyValue( PvGenParameterArray * Pars ,
    LPCTSTR Name , SetCamPropertyData& Value ) ;
  bool         GetPropertyValue( LPCTSTR Name ,
    SetCamPropertyData& Value ) ;
  bool GetPropertyValue(
    PvGenParameterArray * pProperties , SetCamPropertyData& Value ) ;
  bool         GetPropertyValue( SetCamPropertyData& Value ) ;
  PvUInt32 GetXSize()  ;
  void SetWidth( PvUInt32 Width ) ;
  PvUInt32 GetYSize() ;
  void SetHeight( PvUInt32 Height ) ;
  PvUInt32 GetXOffset() ;
  void SetXOffset( PvUInt32 XOffset ) ;
  PvUInt32 GetYOffset() ;
  void SetYOffset( PvUInt32 YOffset ) ;
  PvUInt32 GetMaxWidth() ;
  PvUInt32 GetMaxHeight() ;
  CVideoFrame * ConvertAVTtoSHformat( 
    const PvBuffer * pFrame , PleoraCamera * pGadget ) ;
  void OnBusRemovalEvent();
  void OnBusArrivalEvent();
public:
  virtual COutputConnector * GetOutputConnector(int iConnNum) 
  {
    if ( iConnNum == 0 )
      return m_pOutput ;
    else if ( iConnNum == 1 )
      return m_pLogOutput ;
    return NULL ;
  }
  virtual int GetOutputsCount() { return 2 ; }
  FXString GetCameraId( DWORD dwSerialNumber , DWORD& dwIndex ) ;
  LPCTSTR GetGadgetInfo() { return (LPCTSTR)m_GadgetInfo ; } ;
  bool CheckAndAllocCamera(void);
  bool SetBMIH(void);
  int GetColorMode()
  {
    int iPixelMode = m_pixelFormat  & 0xff ;
    if ( iPixelMode >= 8 && iPixelMode <= 0x59 )
    {
    }
    if ( m_pixelFormat == PvPixelBayerGR8
      || m_pixelFormat == PvPixelBayerRG8
      || m_pixelFormat == PvPixelBayerGR16
      || m_pixelFormat == PvPixelBayerGR12
      || m_pixelFormat == PvPixelBayerGR10
      || m_pixelFormat == PvPixelBayerRG16
      || m_pixelFormat == PvPixelBayerRG12 )
      return COLOR_BY_SOFTWARE ; // colors by program
    if ( m_pixelFormat == PvPixelYUV411Packed
      || m_pixelFormat == PvPixelYUV422Packed
      || m_pixelFormat == PvPixelYUV444Packed
       )
      return COLOR_BY_CAMERA ; // color by camera
    return COLOR_BW ; // bw mode
  }
  unsigned SerialToNmb(unsigned serial)
  {
    unsigned retV=-1;
    for (unsigned i=0; i < m_CamerasOnBus; i++)
    {
      if (m_CamInfo[i].serialnmb==serial)
        return i;
    }
    return retV;
  }
  int GetPropertyIndex( LPCSTR name ) ;
  int GetPropertyIndex( FG_PARAMETER id ) ;

  void SendValueToLog() ;
  bool IsSNLegal() { return ( (m_dwSerialNumber != 0) && (m_dwSerialNumber != 0xffffffff) ) ; }
};

// 
// }} // AVT::VmbAPI 


#endif // !defined(AFX_PleoraCamera_H__0C071980_0F39_40DC_A144_B6BEDB69E5DC__INCLUDED_)

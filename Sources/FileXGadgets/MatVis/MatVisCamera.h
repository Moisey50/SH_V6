// MatVis.h: interface for the MatVis class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MatVis_H__0C071980_0F39_40DC_A144_B6BEDB69E5DC__INCLUDED_)
#define AFX_MatVis_H__0C071980_0F39_40DC_A144_B6BEDB69E5DC__INCLUDED_

#include <queue>
#include <1394Camera.h>
#include <mvIMPACT_CPP/mvIMPACT_acquire.h>
#include <mvIMPACT_CPP/mvIMPACT_acquire_GenICam.h>

#include "helpers\propertykitEx.h"
#include "helpers\SharedMemBoxes.h"
#include "helpers\FramesHelper.h"
// namespace AVT {
//   namespace VmbAPI {

#define ARR_SIZE(x) ( sizeof(x)/sizeof(x[0]))
#define LENARR(x) (sizeof(x)/sizeof(x[0]))
#define N_FRAMES_FOR_ALLOCATE (10)

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
  TImageBufferPixelFormat  vm;
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


enum GrabMode
{
  GM_Unknown = 0 ,
  GM_Continuous ,
  GM_Multi ,
  GM_Single
};

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

  FGP_LAST ,
  FGP_WRONG
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
#define FGP_BINNINGV          ((FG_PARAMETER)(FGP_IMAGEFORMAT-36))
#define FGP_DECIMATION        ((FG_PARAMETER)(FGP_IMAGEFORMAT-37))
#define FGP_DECIMATIONV       ((FG_PARAMETER)(FGP_IMAGEFORMAT-38))

#define FGP_ADCGAIN           ((FG_PARAMETER)(FGP_IMAGEFORMAT-39))  // Matrix Vision Specific
#define FGP_VRAMP             ((FG_PARAMETER)(FGP_IMAGEFORMAT-40))  // Matrix Vision Specific
#define FGP_BLACKLEVEL        ((FG_PARAMETER)(FGP_IMAGEFORMAT-41))  // Matrix Vision Specific; with auto
#define FGP_DIGITALGAINOFFSET ((FG_PARAMETER)(FGP_IMAGEFORMAT-42))  // Matrix Vision Specific
#define FGP_SAVECALIBDATA     ((FG_PARAMETER)(FGP_IMAGEFORMAT-43))  // Matrix Vision Specific


typedef CPtrList PvFrames ;

#define  BUS_EVT_ARRIVED   0x001 
#define  BUS_EVT_REMOVED   0x002 
#define  BUS_EVT_BUS_RESET 0x004
#define  MatVis_EVT_SHUTDOWN  0x100
#define  MatVis_EVT_START_GRAB 0x200
#define  MatVis_EVT_STOP_GRAB  0x400
#define  MatVis_EVT_RELEASE    0x800
#define  MatVis_EVT_RESTART    0x1000
#define  MatVis_EVT_INIT       0x2000
#define  MatVis_EVT_LOCAL_STOP  0x4000
#define  MatVis_EVT_LOCAL_START 0x8000
#define  MatVis_EVT_SET_PROP    0x10000
#define  MatVis_EVT_DRIVER_INIT 0x20000
#define  MatVis_EVT_BUILD_PROP  0x40000
#define  MatVis_EVT_GET_PROP    0x80000
#define  MatVis_EVT_LOG         0x100000
#define  MatVis_EVT_SET_SOFT_TRIGGER 0x200000

#define COLOR_BW          0 // no color info
#define COLOR_BY_SOFTWARE 1 // RAW image received, convert to color
#define COLOR_BY_CAMERA   2 // Camera did color coding (YUV, RGB, BGR formats received)

#define SEND_DEVICE_INFO(sz,...)        FxSendLogMsg(MSG_INFO_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)
#define SEND_DEVICE_TRACE(sz,...)        FxSendLogMsg(MSG_DEBUG_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)
#define SEND_DEVICE_WARN(sz,...)        FxSendLogMsg(MSG_WARNING_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)
#define SEND_DEVICE_ERR(sz,...)        FxSendLogMsg(MSG_ERROR_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)
#define SEND_DEVICE_FAIL(sz,...)        FxSendLogMsg(MSG_CRITICAL_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)

// template<class _Ty>
// class DisplayDictEntry : public std::unary_function<std::pair<std::string , _Ty> , void>
//   //-----------------------------------------------------------------------------
// {
// public:
//   void operator()( const std::pair<std::string , _Ty>& data ) const
//   {
//     std::cout << "  [" << data.second << "]: " << data.first << std::endl;
//   }
// };


union PropertyValue
{
  bool     bBool ;
  int      iInt ;
  double   dDouble ;
  char     szAsString[300] ;
};

typedef struct tagCamProperty
{
  FG_PARAMETER        pr;
  const char *        name;
  const char *        CamPropertyName ;
  const char *        AutoControl ;
  void       *        pInfo ;
  bool                m_bStopAcquisition ;
  int                 m_DataType ;
  FXStringArray       m_EnumerateNames ;
  FXParser            m_DlgFormat ;
  PropertyValue       m_LastValue ;
} CamProperty;


struct CamTypeAndProperties
{
  LPCTSTR     CamType ;
  int         iNProperties ;
  CamProperty * Properties ;
}  ;

class CameraAttribute
{
public:
  CameraAttribute( FG_PARAMETER init_pr = FGP_LAST ,
    LPCTSTR pName = NULL , LPCTSTR pCamName = NULL , HOBJ hObjHandle = INVALID_ID )
  {
    memset( &m_Type , 0 , ( LPBYTE ) &m_enumVal - ( LPBYTE ) &m_Type ) ;
    m_CameraPropertyName = pCamName ;
    m_Name = pName ;
    pr = init_pr ;
    m_hObjHandle = hObjHandle ;
  }
  FXString            m_Name ;
  FXString            m_CameraPropertyName ;
  TComponentType      m_Type ;
  HOBJ                m_hObjHandle ;
  FG_PARAMETER        pr;
  bool                m_bIsStopNecessary ;
  bool                m_bHasMinMax ;
  
  ULONG               m_ulRange[ 2 ] ;
  double              m_dRange[ 2 ] ;
  __int64             m_i64Range[ 2 ] ;
  double              m_dStep ;
  int                 m_intVal ;
  bool                m_boolVal ;
  DWORD               m_uintVal ;
  double              m_dVal ;
  __int64             m_int64Val ;

  FXString            m_enumVal ;
  FXString            m_stringVal ;
  FXString            m_AutoControl ;
  FXString            m_Description ;
  FXString            m_DisplayName ;
  FXString            m_AllEnumeratorsAsString ;
  FXString            m_FDescription ; // for output in EnumCameras
  Int64Vector  m_int64Enums ;
  IntVector    m_intEnums ;
  DoubleVector m_DblEnums ;
  StringVector m_StringEnums ;  

  FXParser            m_DlgFormat ;
  CameraAttribute& operator = ( const CameraAttribute& Orig )
  {
    m_Name = Orig.m_Name ;
    m_CameraPropertyName = Orig.m_CameraPropertyName ;
    memcpy( &m_Type , &Orig.m_Type , ( LPBYTE ) &m_enumVal - ( LPBYTE ) &m_Type ) ;
    m_StringEnums = Orig.m_StringEnums ;
    m_enumVal = Orig.m_enumVal ;
    m_stringVal = Orig.m_stringVal ;
    m_DlgFormat = Orig.m_DlgFormat ;
    m_AutoControl = Orig.m_AutoControl ;
    m_int64Enums = Orig.m_int64Enums ;
    m_intEnums = Orig.m_intEnums ;
    m_DblEnums = Orig.m_DblEnums ;
    m_AllEnumeratorsAsString = Orig.m_AllEnumeratorsAsString ;
    m_FDescription = Orig.m_FDescription ;
    m_DisplayName = Orig.m_DisplayName ;
//     m_intEnums = Orig.m_intEnums ;
//     m_int64Enums = Orig.m_int64Enums ;
//     m_DblEnums = Orig.m_DblEnums ;
//     m_intEnums.assign( Orig.m_intEnums.begin() , Orig.m_intEnums.end() );
//     m_int64Enums.assign( Orig.m_int64Enums.begin() , Orig.m_int64Enums.end() );
//     m_DblEnums.assign( Orig.m_DblEnums.begin() , Orig.m_DblEnums.end() );
    return *this ;
  }
  LPCTSTR GetEnumByIndex( int iIndex )
  {
    if ( iIndex >= 0 && iIndex < (int)m_StringEnums.size() )
    {
      return ( LPCTSTR ) m_StringEnums[ iIndex ].c_str() ;
    }
    return NULL ;
  }
  void SetType( TComponentType Type ) { m_Type = Type ; }
  int GetEnumRangeLength() { return (int)m_StringEnums.size() ; }
};

typedef CArray<CameraAttribute> CamProperties ;
typedef map<string , Property> StringPropMap;

class SetCamPropertyData
{
public:
  SetCamPropertyData() { Reset() ;}
  void    Reset() 
  { 
    m_Pr = ( FG_PARAMETER ) ( -1 ) ;
    memset( &m_bAuto , 0 , ( LPBYTE ) m_szString - ( LPBYTE ) &m_bAuto + 1 ) ; 
  }
  FG_PARAMETER m_Pr ;
  bool    m_bAuto ;
  TComponentType m_Type ;
  int     m_int ;
  __int64 m_int64 ;
  double  m_double ;  
  bool    m_bBool ;
  bool    m_bInvalidate ;
  char    m_szString[300] ; 
  SetCamPropertyData& operator=( SetCamPropertyData& Orig )
  {
    memcpy( this , &Orig , sizeof( *this ) - sizeof(m_szString) )  ;
    strcpy_s( m_szString , Orig.m_szString ) ;
    return *this ;
  }
};

class MatVisCamInfo
{
public:
  MatVisCamInfo() { m_dwSN = 0 ; m_Interface = (TDeviceInterfaceLayout) 0 ; }
  std::string m_ModelName ;
  std::string m_sSN ;
  DWORD       m_dwSN ;
  int         m_Id ;
  TDeviceInterfaceLayout m_Interface ;
};

class MatVis ;

class BusyCamera 
{
public:
  BusyCamera( DWORD dwSN = 0 , MatVis * pGadget = NULL ) 
  {
    m_dwSerialNumber = dwSN ;
    m_pGadget = pGadget ;
  }
  DWORD  m_dwSerialNumber ;
  MatVis * m_pGadget ;
};


BOOL __stdcall FrameDoneCB( const mvIMPACT::acquire::ImageBuffer *pFrame , void * pClient ) ;

class MatVis : public C1394Camera  
{
  friend unsigned int __stdcall GrabThread(  void * pData )  ;
  friend class CameraObserver ;
  static DeviceManager  * m_pDeviceManager ;
  static int              m_iNGadgets ; 
  static MatVisCamInfo m_CamInfo[ MAX_CAMERASNMB ] ;
  Device *             m_pDevice ;
  //CameraSettingsBlueCOUGAR * m_pCougarSettings ;
  std::string             m_InterfaceLayout ;
  const std::vector< std::string > m_ImageRequestControls ;


  FunctionInterface *     m_pFuncInt ;
//   GenICam::AcquisitionControl *    m_pAcqControl ;
//   GenICam::ImageFormatControl * m_pImageFormatControl ;
  FXStringArray        m_AcqModesAsStrings ;
  StringPropMap        m_PropertyMap ;
  bool                 m_bDebug ;
  enum CamTriggerMode
  {
    TrigNotSupported = -1 ,
    TriggerOff ,
    TriggerOn
  } m_TriggerMode ;
public:  
  CVideoFrame * ConvertMVtoSHformat( const ImageBuffer * pFrame ) ;
  bool        StartUp();
//   VmbErrorType        StartContinuousImageAcquisition( const std::string &rStrCameraID );
//   VmbErrorType        StopContinuousImageAcquisition();

  __int64          GetWidth() { return m_nWidth; }
  __int64          GetHeight() { return m_nHeight; }
  Device *         GetDevice() { return m_pDevice ; }
  TImageBufferPixelFormat  GetPixelFormat() ;
//   AVT::VmbAPI::CameraPtrVector     GetCameraList();
//   AVT::VmbAPI::FramePtr            GetFrame();
//   void                ClearFrameQueue();
//   VmbErrorType        QueueFrame( AVT::VmbAPI::FramePtr pFrame );
  void SaveCameraInfo( Device * camera , int iIndex ) ;
//   string_type ErrorCodeToMessage( VmbErrorType eErr ) const
//   {
//     return AVT::VmbAPI::Examples::ErrorCodeToMessage( eErr );
//   }    
  std::string         GetVersion() const;
private:
  COutputConnector *  m_pLogOutput ;

  // Instance counter
  static DWORD m_dwInstanceCount ;
  // The current width
  __int64 m_nWidth;
  // The current height
  __int64 m_nHeight;
  CamProperties           m_PropertiesEx ;
  const CamProperty *     m_pOrigProperties ;
  int                     m_iNProperties ;
private:
  static FXLockObject           m_ConfigLock ;
  static FXArray<BusyCamera>    m_BusyCameras ;
  static int                    m_iCamNum ;
  static DWORD                  m_dwDefaultTimeout ;
  static bool                   m_bSaveFullInfo ;

//   AVT::VmbAPI::CameraPtrVector  m_Cameras ;
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
  GrabMode                m_GrabMode ;
  int                     m_iNFramesForGrabbing ;

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
  FXString                m_TmpString ;
  TCHAR                   m_TempOutOfStack[100] ;


  FXString                m_PropertiesForLogAsString ;
  FXStringArray           m_PropertiesForLog ;
  FXString                m_PropertiesForTempAsString ;
  FXStringArray           m_PropertiesForTemp ;
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

  DWORD                   m_BusEvents ;
  DWORD                   m_LastError;
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
  bool        m_bIsStopped ;
  bool        m_bInScanSettings ;

  FXString    m_szSerialNumber ;
  DWORD       m_dwSerialNumber ;
  DWORD       m_dwConnectedSerialNumber ;

//   Mode		    m_Mode;
  BITMAPINFOHEADER m_BMIH;
  BITMAPINFOHEADER m_RealBMIH;

  // The current pixel format
  __int64 m_nPixelFormat;
  TImageBufferPixelFormat m_pixelFormat; // necessary pixel format
  UINT        m_uiPacketSize ;
  CSize       m_SensorSize ;
  float       m_fPercentage ;
  int         m_iFPSx10 ;
  FXString    m_FPSPropertyName ;
  double      m_dExtShutter ;
  int         m_iWBRed ;
  int         m_iWBlue ;

  std::queue<CVideoFrame*> m_ReadyFrames ;
  
  
  CRect       m_CurrentROI;
  int         m_iCurrentBinningOrDecimation ;
  FXString    m_CameraID;
  FXString    m_ControlThreadName ;
  FXString    m_CallbackThreadName ;
  //FXArray <StrobeInfo>  m_StrobeInfo ;
//  EmbeddedImageInfo m_embeddedInfo;
//   unsigned int m_prevWidth;
//   unsigned int m_prevHeight;

  double m_dLastStartTime ;
  double m_dLastInCallbackTime ;
  CVideoFrame * m_pNewFrame ;
  HANDLE      m_WaitEventFrameArray[2] ;   // For m_evFrameReady and m_evExit
                                           // in CAmeraDoGrab function
  HANDLE      m_WaitEventBusChangeArr[4] ; // for m_evCameraControl , m_evExit, m_evBusChange, 
                                           // in DoGrabLoop
  HANDLE      m_evFrameReady ;
  HANDLE      m_evBusChange ;
  HANDLE      m_evCameraControl ;
  HANDLE      m_evControlRequestFinished ;
  HANDLE      m_hCamAccessMutex ;
  HANDLE      m_hCameraControlThreadHandle ;
  DWORD       m_dwCameraControlThreadId ;
  HANDLE      m_hGrabThreadHandle ;
  HANDLE      m_hevGrabEvt ;
  UINT        m_uiGrabThreadId ;
  bool        m_bContinueCameraControl ;
#ifdef _DEBUG
      FXString m_MutexHolder ;
      int      m_iMutexTakeCntr ;
#endif

  DWORD       m_dwNArrivedEvents ;
public:
  MatVis();
  ~MatVis() ;
  virtual bool DriverInit();
  virtual bool EnumCameras();
  virtual bool CameraInit();
  virtual void CameraClose();

  virtual bool DriverValid();
  virtual bool CameraStart();
  virtual void CameraStop();

  virtual bool GetCameraProperty(unsigned i, FXSIZE &value, bool& bauto);
  virtual bool SetCameraProperty(unsigned i, FXSIZE &value, bool& bauto, bool& Invalidate);
  virtual bool SetCameraPropertyEx(unsigned i, SetCamPropertyData * pData, bool& Invalidate);
  virtual bool GetCameraPropertyEx( int iIndex , SetCamPropertyData * pData );
  virtual bool GetCameraPropertyEx( LPCTSTR pszPropertyName , SetCamPropertyData * pData );

  virtual CVideoFrame* CameraDoGrab(double* StartTime);
  virtual void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
  virtual bool ScanSettings(FXString& text);
  virtual bool ScanProperties(LPCTSTR text, bool& Invalidate) ;
  virtual bool PrintProperties(FXString& text);
  virtual void ShutDown();
  virtual bool SetSoftwareTriggerMode( bool bSet ) ;
  void         LogError( LPCTSTR Msg ) ;
  /**
  * Bus arrival handler that is passed to BusManager::RegisterCallback().
  * This simply emits a signal that calls the real handler.
  *
  * @param pParam The parameter passed to the BusManager::RegisterCallback().
  */
  static void OnBusArrival( void* pParam, LPCTSTR szSerNum );

  /**
  * Bus removal handler that is passed to BusManager::RegisterCallback().
  * This simply emits a signal that calls the real handler.
  *
  * @param pParam The parameter passed to the BusManager::RegisterCallback().
  */
  static void OnBusRemoval( void* pParam, LPCTSTR szSerNum );

  static void OnBusReset( void* pParam, LPCTSTR szSerNum );
  // Camera observer for camera connect-disconnect indication
  CameraObserver  * m_pCameraObserver ;

  // Capture gadget stuff
  DECLARE_RUNTIME_GADGET(MatVis);
private:
  // The camera control thread function
  static DWORD WINAPI CameraControlLoop( LPVOID pParam );

  bool MatVisDriverInit();
  bool MatVisCameraInit();
  void MatVisCameraClose();
  bool MatVisCameraStart();
  void MatVisCameraStop() ;
  bool MatVisBuildPropertyList();
  bool MatVisSaveOrLoadSettings( int iMode ) ; // 0 - nothing to do, 1 - Save, 2 - Load
  bool MatVisSetSoftwareTriggerMode( bool bSet ) ;
  bool MatVisSetCameraPropertyEx( LPCTSTR pName , SetCamPropertyData * pData );
  bool MatVisSetAndCheckPropertyEx( LPCTSTR pName , SetCamPropertyData * pData );
  bool MatVisGetCameraPropertyEx( LPCTSTR pName , SetCamPropertyData * pData );
  bool MatVisGetCameraPropertyEx( SetCamPropertyData * pData );
  bool MatVisGetCameraPropertyEx( Property * pProperty , SetCamPropertyData * pData );
  void LocalStreamStart( void ) ;
  void LocalStreamStop( void ) ;
  bool BuildPropertyList();
  bool GetROI(CRect& rc);
  void SetROI(CRect& rc);
  bool SetStrobe( const FXString& STrobeDataAsText ,  int iIndex ) ;
  void GetStrobe( FXString& StrobeDataAsText , int iIndex ) ;

protected:
  void GetCamResolutionAndPixelFormat(unsigned int* rows,
    unsigned int* cols, TImageBufferPixelFormat* pixelFmt); //
  unsigned int GetBppFromPixelFormat( TImageBufferPixelFormat pixelFormat );                           //

  bool GetULongFeature( LPCTSTR pFeatureName , DWORD& ulValue ) ;
  bool SetULongFeature( LPCTSTR pFeatureName , DWORD ulValue ) ;
  bool GetPropertyValue( LPCTSTR Name , SetCamPropertyData& Value ) ;
  bool SetPropertyValue( LPCTSTR Name , SetCamPropertyData& Value ) ;
  bool GetFeatureIntValue( LPCTSTR pFeatureName, __int64 & value ) ;
  bool SetFeatureIntValue( LPCTSTR pFeatureName , __int64 value ) ;
  bool SetFeatureValueAsEnumeratorString( LPCTSTR pFeatureName , LPCTSTR pValue ) ;
  bool RunCameraCommand( LPCTSTR pCommand ) ;

  DWORD GetXSize()  ;
  void SetWidth( DWORD Width ) ;
  DWORD GetYSize() ;
  void SetHeight( DWORD Height ) ;
  DWORD GetXOffset() ;
  void SetXOffset( DWORD XOffset ) ;
  DWORD GetYOffset() ;
  void SetYOffset( DWORD YOffset ) ;
  DWORD GetSensorWidth() ;
  DWORD GetSensorHeight() ;
  DWORD GetMaxWidth() ;
  DWORD GetMaxHeight() ;

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
  bool SetBMIH( LPCTSTR pPixelFormat = NULL );
//   int GetColorMode()
//   {
//     int iPixelMode = m_pixelFormat  & 0xff ;
//     if ( iPixelMode >= 8 && iPixelMode <= 0x59 )
//     {
//     }
//     if ( m_pixelFormat == ibpfRGBx888Packed //bgr
//       || m_pixelFormat == ibpfRGB888Packed
//       || m_pixelFormat == ibpfBGR888Packed
//       || m_pixelFormat == VmbPixelFormatBayerGR12
//       || m_pixelFormat == VmbPixelFormatBayerGR10
//       || m_pixelFormat == ibpfRGB161616Packed //bgr
//       || m_pixelFormat == ibpfRGB141414Packed //bgr
//       || m_pixelFormat == ibpfRGB121212Packed
//       || m_pixelFormat == ibpfRGB101010Packed )
//       return COLOR_BY_SOFTWARE ; // colors by program
//     if ( m_pixelFormat == VmbPixelFormatYuv411
//       || m_pixelFormat == ibpfYUV422Packed
//       || m_pixelFormat == VmbPixelFormatYuv444
// //       || m_pixelFormat == PIXEL_FORMAT_422YUV8_JPEG
//       || m_pixelFormat == VmbPixelFormatRgb8
//       || m_pixelFormat == VmbPixelFormatRgb16
// //      || m_pixelFormat == PIXEL_FORMAT_S_RGB16
//       || m_pixelFormat == VmbPixelFormatBgra8
// //      || m_pixelFormat == PIXEL_FORMAT_BGR16
//       || m_pixelFormat == VmbPixelFormatBgr8 )
//       return COLOR_BY_CAMERA ; // color by camera
//     return COLOR_BW ; // bw mode
//   }
  unsigned SerialToNmb(unsigned serial)
  {
    unsigned retV=-1;
    for (unsigned i=0; i < m_CamerasOnBus; i++)
    {
      if ( m_CamInfo[i].m_dwSN == serial )
        return i;
    }
    return retV;
  }
  int GetPropertyIndex( LPCTSTR name ) ;
  int GetInCameraPropertyIndex( LPCTSTR name ) ;
  int GetPropertyIndex( FG_PARAMETER id ) ;
  FG_PARAMETER MatVis::GetPropertyID( LPCTSTR name ) ;

  int SetPacketSize( int iPacketSize_Or_FPSx10 , bool bFrameRate );

  inline bool CamCNTLDoAndWait( DWORD EvtMask , DWORD dwTimeOut = m_dwDefaultTimeout )
  {
    if ( m_bContinueCameraControl )
    {
      FXAutolock al( m_CamControlLock ) ;
      m_BusEvents |= EvtMask ;
      ResetEvent( m_evControlRequestFinished ) ;
      SetEvent( m_evCameraControl ) ;
      bool bRes = ( WaitForSingleObject( m_evControlRequestFinished , dwTimeOut ) == WAIT_OBJECT_0 ) ;
      if ( !bRes )
      {
        SEND_DEVICE_ERR( "Cam #%u Error: %s" , m_dwSerialNumber , 
          (LPCTSTR) DecodeEvtMask( EvtMask ) ) ;
        TRACE( "Cam #%u Error: %s" , m_dwSerialNumber , (LPCTSTR) DecodeEvtMask( EvtMask ) ) ;
        m_BusEvents = 0 ;
      }
      return bRes ;
    }
    return true ;
  }

  inline bool CheckAndStopCapture()
  {
    bool bWasRunning = IsRunning() ;
    if ( bWasRunning )
    {
      bool bRes =CamCNTLDoAndWait( MatVis_EVT_STOP_GRAB , 1000) ;
      Sleep(100) ;
      m_bWasStopped = true ; 
    }
    return bWasRunning ;
  }
  bool OtherThreadDriverInit() { return CamCNTLDoAndWait( MatVis_EVT_DRIVER_INIT , 5000) ; }
  bool OtherThreadCameraInit() { return CamCNTLDoAndWait( MatVis_EVT_INIT ) ; }
  bool OtherThreadBuildPropertyList() { return CamCNTLDoAndWait( MatVis_EVT_BUILD_PROP , m_bDebug ? 3000000 : 10000 ) ; }
  bool OtherThreadCameraClose() { return CamCNTLDoAndWait( MatVis_EVT_RELEASE ) ; }
  bool OtherThreadCameraStart() { return CamCNTLDoAndWait( MatVis_EVT_START_GRAB ) ; }
  bool OtherThreadCameraStop() { return CamCNTLDoAndWait( MatVis_EVT_STOP_GRAB , m_bDebug ? 300000 : 5000) ; }
  bool OtherThreadCameraShutDown() { return CamCNTLDoAndWait( MatVis_EVT_SHUTDOWN ) ; }
  bool OtherThreadLocalStart() { return CamCNTLDoAndWait( MatVis_EVT_LOCAL_START ) ; }
  bool OtherThreadLocalStop() { return CamCNTLDoAndWait( MatVis_EVT_LOCAL_STOP ) ; }
  bool OtherThreadSetProperty( int iIndex , SetCamPropertyData * pData , bool * bInvalidate )
  {
    m_PropertyData = *pData ;
    m_iPropertyIndex = iIndex ;
    m_bInvalidate = false ;
    bool bRes = CamCNTLDoAndWait( MatVis_EVT_SET_PROP ) ;
    if ( bRes )
    {
      *bInvalidate |= m_bInvalidate ;
      return true ;
    }
    return false ;
  }
  bool OtherThreadGetProperty( int iIndex , SetCamPropertyData * pData ,
    TCHAR * pName = NULL ) ;
  bool OtherThreadSetSoftwareTrigger( bool bSet )
  {
    m_PropertyData.Reset() ;
    m_PropertyData.m_bBool = bSet ;
    bool bRes = CamCNTLDoAndWait( MatVis_EVT_SET_SOFT_TRIGGER ) ;
    return bRes ;
  }

  FXString DecodeEvtMask( DWORD dwEVtMask ) ;
  LPCTSTR DecodePropertyType( TComponentType Type ) ;
  TImageBufferPixelFormat GetDecodedPixelFormat( LPCTSTR pPixelFormatName = NULL ) ;

  CVideoFrame * ConvertMVFrame(
    const mvIMPACT::acquire::ImageBuffer * pFrame , UINT OutputFormat ) ;
  void SendValueToLog() ;
  Property GetPropertyByName( LPCTSTR pName ) ;
  Component FindInList( ComponentIterator iter , LPCTSTR pName , const string& path=_T("") ) ;
  int PopulateDevicePropertyMap( Device * pDev , StringPropMap& map ) ;
  bool FormDialogData( mvIMPACT::acquire::Property& Comp , CameraAttribute& CompValues );
  bool ReorderDialogData() ; // find and replace setup dialog data
                             // initially done for TriggerMode and TriggerSource merging
  bool SetCheckRegionAndBinningOrDecimation( CRect& AOI ,
    char * pValueAsString , bool bBinning = true) ;// if not binning - decimation
  // Get grab mode and # rest frames
  bool GetGrabConditions( GrabMode& Mode , int& iNRestFrames ) ;
  // Set grab mode and # necessary to grab frames
  bool SetGrab( int iNFrames ) ;
  bool SetNFramesAndAcquisitionMode( int iNFrames ) ;
  int SetBinningAndDecimation( int iBinning , int iDecimation );
};

// 
// }} // AVT::VmbAPI 


#endif // !defined(AFX_MatVis_H__0C071980_0F39_40DC_A144_B6BEDB69E5DC__INCLUDED_)

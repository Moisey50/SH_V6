// PtGrey2_0.h: interface for the PtGrey2_0 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PtGrey2_0_H__0C071980_0F39_40DC_A144_B6BEDB69E5DC__INCLUDED_)
#define AFX_PtGrey2_0_H__0C071980_0F39_40DC_A144_B6BEDB69E5DC__INCLUDED_

#include <gadgets\CDevice.h>
#include <FlyCapture2.h>

using namespace FlyCapture2;

#define FORMAT_MODE ((FlyCapture2::PropertyType)(BRIGHTNESS-1))
#define SETROI ((FlyCapture2::PropertyType)(BRIGHTNESS-2))
#define SETSTROBE0 ((FlyCapture2::PropertyType)(BRIGHTNESS-3))
#define SETSTROBE1 ((FlyCapture2::PropertyType)(BRIGHTNESS-4))
#define SETSTROBE2 ((FlyCapture2::PropertyType)(BRIGHTNESS-5))
#define SETSTROBE3 ((FlyCapture2::PropertyType)(BRIGHTNESS-6))
#define PACKETSIZE ((FlyCapture2::PropertyType)(BRIGHTNESS-7))
#define WHITE_BAL_RED  ((FlyCapture2::PropertyType)(BRIGHTNESS-8))
#define WHITE_BAL_BLUE ((FlyCapture2::PropertyType)(BRIGHTNESS-9))
#define SAVE_SETTINGS ((FlyCapture2::PropertyType)(BRIGHTNESS-10))
#define RESTART_TIMEOUT   ((FlyCapture2::PropertyType)(BRIGHTNESS-11))
#define MAX_PACKET_SIZE ((FlyCapture2::PropertyType)(BRIGHTNESS-12))  
#define EMBED_INFO      ((FlyCapture2::PropertyType)(BRIGHTNESS-13))  


#define  BUS_EVT_ARRIVED       0x001 
#define  BUS_EVT_REMOVED       0x002 
#define  BUS_EVT_BUS_RESET     0x004
#define  PGR_EVT_SHUTDOWN      0x100
#define  PGR_EVT_START_GRAB    0x200
#define  PGR_EVT_STOP_GRAB     0x400
#define  PGR_EVT_RELEASE       0x800
#define  PGR_EVT_RESTART       0x1000
#define  PGR_EVT_INIT          0x2000
#define  PGR_EVT_LOCAL_STOP    0x4000
#define  PGR_EVT_LOCAL_START   0x8000
#define  PGR_EVT_SET_PROP      0x10000
#define  PGR_EVT_DRIVER_INIT   0x20000
#define  PGR_EVT_BUILD_PROP    0x40000
#define  PGR_EVT_GET_PROP      0x80000

#define COLOR_BW          0 // no color info
#define COLOR_BY_SOFTWARE 1 // RAW image received, convert to color
#define COLOR_BY_CAMERA   2 // Camera did color coding (YUV, RGB, BGR formats received)


typedef struct tagVideoformat
{
  FlyCapture2::PixelFormat vm;
  const char *        name; 
}Videoformat;


class SetCamPropertyData
{
public:
  SetCamPropertyData() { Reset() ;}
  void    Reset() { memset( &m_bAuto , 0 , (LPBYTE)m_szString - (LPBYTE)&m_bAuto + 1 ) ;}
  bool    m_bAuto ;
  int     m_int ;
  double  m_double ;  
  bool    m_bBool ;
  bool    m_bInvalidate ;
  char    m_szString[100] ; 
  SetCamPropertyData& operator=( SetCamPropertyData& Orig )
  {
    memcpy( this , &Orig , (LPBYTE)m_szString - (LPBYTE)&m_bAuto )  ;
    char * pDst = m_szString ;
    char * pSrc = Orig.m_szString ;
    while ( ((*(pDst++) = *(pSrc++)) != 0) 
      && ((pDst - m_szString) < sizeof(m_szString)) ) ;
    return *this ;
  }
};

class PgrCamProperty
{
public:
  PropertyType        id ;
  bool                bSupported ;
  int                 AutoControl ; // index of auto control property
  PropertyDataType    m_DataType ;
  PropertyValue       LastValue ;
  FXParser            m_GUIFormat ;
  FXString            name; 

  PgrCamProperty() { Reset() ; }
  ~PgrCamProperty() {} ;
  void Reset() 
  { 
    memset( this , 0 , (LPBYTE)&m_GUIFormat - (LPBYTE)this ) ;
    m_GUIFormat.Empty() ;
    name.Empty() ;
  } ;
  PgrCamProperty& operator = (const PgrCamProperty& Other)
  {
    memcpy_s( this , sizeof(*this) - sizeof(FXString) , &Other , (LPBYTE)&m_GUIFormat - (LPBYTE)this ) ;
    m_GUIFormat = Other.m_GUIFormat ;
    name = Other.name ;
    return *this ;
  }

} ;

class PgrPropertiesArray: public FXArray<PgrCamProperty> {} ;

typedef struct 
{
  DWORD m_dwTimeStamp ;     //0
  DWORD m_dwFrameCntr ;     // 4
  DWORD m_dwStrobePattern ; // 8
  DWORD m_dwGPIOPinStae ;   // 12
  DWORD m_dwROIPos ;        // 16
  DWORD m_Reserved[ 2 ];    // 20
} FrameInfo , * pFrameInfo ;

class PtGrey2_0 : public CDevice  
{
  enum CamTriggerMode
  {
    TrigNotSupported = -1 ,
    TriggerOff = 0 ,
    TriggerOn ,
    TriggerInv 
  } m_TriggerMode ;
  static void NewImageCallBack( Image * pImage , const void * pCallbackData ) ;
private:
  static FXLockObject           m_ConfigLock ;
  static Device::DeviceInfo     m_CamInfo[MAX_DEVICESNMB] ;
  static FXArray<DWORD>         m_BusyCameras ;
  static int                    m_iCamNum ;
  static HANDLE                 m_hConfigMutex ;
  static int                    m_iGadgetCount ;
  static DWORD                  m_dwDefaultTimeout ;
  static bool                   m_bCamerasEnumerated ;
  static bool                   m_bRescanCameras ;
  static CameraInfo             m_GigECamerasInfo[MAX_DEVICESNMB] ;
  static UINT                   m_uiNGigECameras;


  HANDLE      m_WaitEventFrameArray[2] ;   // For m_evFrameReady and m_evExit
  // in CameraDoGrab function
  HANDLE      m_WaitEventBusChangeArr[2] ; // for m_evExit and m_evBusChange
  // in ControlLoop
  HANDLE      m_evFrameReady ;
  HANDLE      m_evBusChange ;
  HANDLE      m_evControlRequestFinished ;
  HANDLE      m_hGrabThreadHandle ;
  HANDLE      m_hCamAccessMutex ;
  DWORD       m_dwGrabThreadId ;

  FXLockObject            m_LocalConfigLock ;
  FXLockObject            m_ScanPropLock ;
  FXLockObject            m_PropertyListLock ;
  FXLockObject            m_GrabLock;
  FXLockObject            m_SettingsLock;
  FlyCapture2::CameraInfo m_CameraInfo ;
  FlyCapture2::BusManager m_BusManager ;
  FlyCapture2::CameraBase    * m_pCamera ;


  InterfaceType m_IntfType ;
  DWORD         m_BusEvents ;
  Error         m_LastError;
  int           m_iNoCameraStatus ;
  int           m_iNNoSerNumberErrors ;
  int           m_iSaveSettingsState ;
  bool          m_bInitialized ;
  bool          m_bStopInitialized ;
  bool          m_bIsRunning ;
  int           m_iNSkipImages ;

  PGRGuid		  m_CurrentCameraGUID;
  DWORD       m_dwSerialNumber ;
  DWORD       m_dwConnectedSerialNumber ;
  DWORD       m_dwInitializedSerialNumber ;
  bool		    m_FormatNotSupportedDispayed;

  int                     m_iPropertyIndex ;  // **************
  SetCamPropertyData      m_PropertyData ;  // These 4 variables for 
  bool                    m_bAuto ;           // inter thread data passing
  bool                    m_bInvalidate ;     // on set property procedure
  SetCamPropertyData      m_PropertyDataReturn ;
  SetCamPropertyData      m_PropertyDataSend ;

  Mode		    m_Mode;
  BITMAPINFOHEADER m_BMIH;
  BITMAPINFOHEADER m_RealBMIH;

  PixelFormat m_pixelFormat;
  UINT        m_uiPacketSize ;
  BOOL        m_bMaxPacketSize ;
  UINT        m_uiSavedPacketSize;
  Format7ImageSettings m_f7is ; 
  Format7PacketInfo    m_f7pi ;
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

  CRect       m_CurrentROI;
  FXString    m_CameraID;
  FXString    m_ControlThreadName ;
  FXString    m_CallbackThreadName ;
  FXArray <StrobeInfo>  m_StrobeInfo ;
  volatile bool m_continueGrabThread;                 //
//  bool m_isSelectingNewCamera;
  bool m_disableEmbeddedTimeStamp;
  EmbeddedImageInfo m_embeddedInfo;
//   unsigned int m_prevWidth;
//   unsigned int m_prevHeight;

  double m_dLastStartTime ;
  double m_dLastInCallbackTime ;
  double m_dLastFrameTime ;
  PgrPropertiesArray  m_ThisCamProperty ;
  CVideoFrame * m_pNewFrame ;
  FlyCapture2::Image * m_pNewImage ;
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
public:
  PtGrey2_0();
  ~PtGrey2_0() ;
  virtual bool DriverInit();
  virtual bool EnumCameras();
  virtual bool CameraInit();
  virtual void CameraClose();

  virtual bool DriverValid();
  virtual bool DeviceStart();
  virtual void DeviceStop();

  virtual bool GetCameraProperty(PropertyType Type , FXSIZE &value, bool& bauto);
  virtual bool SetCameraProperty(PropertyType Type , FXSIZE &value, bool& bauto, bool& Invalidate);
  virtual bool SetCameraPropertyEx(PropertyType Type , FXSIZE &value, bool& bauto, bool& Invalidate);
  virtual bool GetCameraProperty(int iIndex , FXSIZE &value, bool& bauto);
  virtual bool SetCameraProperty(int iIndex , FXSIZE &value, bool& bauto, bool& Invalidate);
  virtual bool SetCameraPropertyEx(int iIndex , FXSIZE &value, bool& bauto, bool& Invalidate);

  virtual CVideoFrame* DeviceDoGrab(double* StartTime);
  virtual void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
  virtual void LocalStreamStart() ;
  virtual void LocalStreamStop() ;
  virtual bool ScanSettings(FXString& text);
  virtual bool ScanProperties(LPCTSTR text, bool& Invalidate) ;
  virtual bool PrintProperties(FXString& text);
  virtual void ShutDown();
  void         LogError( LPCTSTR Msg ) ;

  int ReceiveImage() ; // if OK returns 0
  // Capture gadget stuff
  DECLARE_RUNTIME_GADGET(PtGrey2_0);
private:
  bool BuildPropertyList();
  void GetROI(CRect& rc);
  void SetROI(CRect& rc);
  bool SetStrobe( const FXString& STrobeDataAsText ,  int iIndex ) ;
  void GetStrobe( FXString& StrobeDataAsText , int iIndex ) ;

protected:
  void GetCamResolutionAndPixelFormat(unsigned int* rows, unsigned int* cols, PixelFormat* pixelFmt); //
  BOOL GetDimensionsFromVideoMode(VideoMode mode, unsigned int* rows, unsigned int* cols); //
  BOOL GetPixelFormatFromVideoMode(VideoMode mode, bool stippled, PixelFormat* pixFormat); //
  unsigned int GetBppFromPixelFormat( PixelFormat pixelFormat );                           //
  bool CheckCameraPower(FlyCapture2::CameraBase * am);                                                                 //
  bool EnableEmbeddedTimeStamp(FlyCapture2::CameraBase* pCam);                              //
  bool DisableEmbeddedTimeStamp(FlyCapture2::CameraBase* cam);                             //

  // The object grab image loop.  Only executed from within the grab thread.
  static DWORD WINAPI ControlLoop( LPVOID pParam );
  bool IsGrabThreadRunning();        //

  /** Camera arrival callback handle. */
  FlyCapture2::CallbackHandle m_cbArrivalHandle;

  /** Camera removal callback handle. */
  FlyCapture2::CallbackHandle m_cbRemovalHandle;

  /** Camera reset callback handle. */
  FlyCapture2::CallbackHandle m_cbResetHandle;
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
  static void OnBusArrival( void* pParam, unsigned int serialNumber );

  /**
  * Bus removal handler that is passed to BusManager::RegisterCallback().
  * This simply emits a signal that calls the real handler.
  *
  * @param pParam The parameter passed to the BusManager::RegisterCallback().
  */
  static void OnBusRemoval( void* pParam, unsigned int serialNumber );

  static void OnBusReset( void* pParam, unsigned int serialNumber );
  void OnBusRemovalEvent();
  void OnBusArrivalEvent();
  virtual bool PtGreyCamInit() ; // Main == true => capture camera
  //      == false => GUI camera
  virtual bool PtGreyCameraStart();
  virtual void PtGreyCameraStop();
  virtual void PtGreyCamClose() ; // Main == true => capture camera
  //      == false => GUI camera
  virtual void PtGreyDeleteCam( FlyCapture2::CameraBase* pCam ) ;
  virtual void PtGreyLocalStreamStart() ;
  virtual void PtGreyLocalStreamStop() ;
  virtual bool PtGreyBuildPropertyList( bool bOutside = false ) ;
  virtual bool PtGreyGetCameraProperty( int iIndex , SetCamPropertyData * pData ) ;
  virtual bool PtGreySetCameraProperty( int iIndex , SetCamPropertyData * pData ) ;
  LPCTSTR GetGadgetInfo() { return (LPCTSTR)m_GadgetInfo ; } ;

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
      
      bool bRes = ( WaitForSingleObject( m_evControlRequestFinished , dwTimeOut ) == WAIT_OBJECT_0 ) ;
      if ( !bRes )
      {
        DEVICESENDERR_2( "Cam #%u Error: %s" , m_dwSerialNumber , (LPCTSTR) DecodeEvtMask( EvtMask ) ) ;
        TRACE( "Cam #%u Error: %s" , m_dwSerialNumber , (LPCTSTR) DecodeEvtMask( EvtMask ) ) ;
        m_BusEvents = 0 ;
      }
      return bRes ;
    }
    return true ;
  }
  bool PtGreyCheckAndStopCapture() ;
  bool PtGreyInitAndConditialStart() ;
  inline bool CheckAndStopCapture()
  {
    bool bWasRunning = IsRunning() ;
    if ( bWasRunning )
    {
      bool bRes =CamCNTLDoAndWait( PGR_EVT_STOP_GRAB , 1000) ;
      Sleep(100) ;
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
  inline FlyCapture2::CameraBase * GetCamera(
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
      return m_pCamera ;
    }
    return NULL ; // can't take mutex
  }
  inline void ReleaseCamera()
  {
    ReleaseMutex( m_hCamAccessMutex ) ;
#ifdef _DEBUG
    ASSERT( (--m_iMutexTakeCntr)==0 );
    m_MutexHolder.Empty();
#endif
  }
public:
  bool CheckAndAllocCamera(void);
  bool SetBMIH(void);
  int GetColorMode()
  {
    if ( m_pixelFormat == PIXEL_FORMAT_RAW8
      || m_pixelFormat == PIXEL_FORMAT_RAW16
      || m_pixelFormat == PIXEL_FORMAT_RAW12 )
      return COLOR_BY_SOFTWARE ; // colors by program
    if ( m_pixelFormat == PIXEL_FORMAT_411YUV8
      || m_pixelFormat == PIXEL_FORMAT_422YUV8
      || m_pixelFormat == PIXEL_FORMAT_444YUV8
      || m_pixelFormat == PIXEL_FORMAT_422YUV8_JPEG
      || m_pixelFormat == PIXEL_FORMAT_RGB8
      || m_pixelFormat == PIXEL_FORMAT_RGB16
      || m_pixelFormat == PIXEL_FORMAT_S_RGB16
      || m_pixelFormat == PIXEL_FORMAT_BGRU16
      || m_pixelFormat == PIXEL_FORMAT_BGR16
      || m_pixelFormat == PIXEL_FORMAT_BGR )
      return COLOR_BY_CAMERA ; // color by camera
    return COLOR_BW ; // bw mode
  }
  int SetPacketSize( int iPacketSize_Or_FPSx10 , bool bFrameRate );
  int GetMaxPacketSize();
  int SetMaxPacketSize();
  bool OtherThreadSetProperty( int iIndex , SetCamPropertyData * pData , bool * bInvalidate )
  {
    FXAutolock al( m_SettingsLock ) ;
    m_PropertyData = *pData ;
    m_iPropertyIndex = iIndex ;
    m_bInvalidate = false ;
    bool bRes = CamCNTLDoAndWait( PGR_EVT_SET_PROP ) ;
    if ( bRes )
    {
      *bInvalidate |= m_PropertyData.m_bInvalidate ;
      *pData = m_PropertyData ;
      return true ;
    }
    return false ;
  }
  bool OtherThreadGetProperty(int iIndex , SetCamPropertyData * pData  )
  {
    FXAutolock al( m_SettingsLock ) ;
    m_iPropertyIndex = iIndex ;
    m_bInvalidate = false ;
    bool bRes = CamCNTLDoAndWait( PGR_EVT_GET_PROP ) ;
    *pData = m_PropertyData ;
    return bRes ;
  }
  int GetThisCamPropertyCnt() { return (int) m_ThisCamProperty.GetCount() ; }
  LPCTSTR GetThisCamPropertyName( PropertyType Type ) ;
  LPCTSTR GetThisCamPropertyName( int iIndex ) ;
  PropertyType GetThisCamPropertyType( int iIndex ) ;
  int GetThisCamPropertyIndex( PropertyType Type ) ;
  int GetThisCamPropertyIndex( LPCTSTR name ) ;
  PropertyType GetThisCamPropertyId(LPCTSTR name);
  PropertyDataType GetPropertyDataType( LPCTSTR name ) ;
  PropertyDataType GetPropertyDataType( int iIndex ) ;
  PropertyDataType GetPropertyDataType( PropertyType Type ) ;
  FXString DecodeEvtMask( DWORD dwEVtMask )
  {
    FXString Answer ;
    if ( dwEVtMask & PGR_EVT_SHUTDOWN )
      Answer += "ShutDown " ;
    if ( dwEVtMask & PGR_EVT_DRIVER_INIT )
      Answer += "DrvInit " ;
    if ( dwEVtMask & PGR_EVT_RELEASE )
      Answer += "Release " ;
    if ( dwEVtMask & PGR_EVT_RESTART )
      Answer += "Restart " ;
    if ( dwEVtMask & PGR_EVT_INIT )
      Answer += "CamInit " ;
    if ( dwEVtMask & PGR_EVT_START_GRAB )
      Answer += "Start " ;
    if ( dwEVtMask & PGR_EVT_STOP_GRAB )
      Answer += "Stop " ;
    if ( dwEVtMask & PGR_EVT_SET_PROP )
      Answer += "SetProp " ;
    if ( dwEVtMask & PGR_EVT_GET_PROP )
      Answer += "GetProp " ;
    if ( dwEVtMask & PGR_EVT_BUILD_PROP )
      Answer += "BuildProp " ;
    if ( dwEVtMask & PGR_EVT_LOCAL_START )
      Answer += "LocStart " ;
    if ( dwEVtMask & PGR_EVT_LOCAL_STOP )
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
  int GetPropertyIndex( LPCTSTR name )
  {
    int i;
    unsigned retV = -1 ;
    for ( i = 0; i < m_ThisCamProperty.GetSize(); i++ )
    {
      if ( m_ThisCamProperty[ i ].name.CompareNoCase( name ) == 0 )
        return  i ;
    }
    return -1 ;
  }

  bool SetFrameRate( double dFrameRate );
  bool GetFrameRate( double& dFrameRate );
};


#endif // !defined(AFX_PtGrey2_0_H__0C071980_0F39_40DC_A144_B6BEDB69E5DC__INCLUDED_)

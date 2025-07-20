// IP_OnvifCam_.h: interface for the IP_OnvifCam class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IP_OnvifCam_H__0C071980_0F39_40DC_A144_B6BEDB69E5DC__INCLUDED_)
#define AFX_IP_OnvifCam_H__0C071980_0F39_40DC_A144_B6BEDB69E5DC__INCLUDED_

#include "gadgets/1394Camera.h"
#include <helpers\SharedMemBoxes.h>

//#include <gadgets\IPCamera.h>


#define  BUS_EVT_ARRIVED   0x001 
#define  BUS_EVT_REMOVED   0x002 
#define  BUS_EVT_BUS_RESET 0x004
#define  PGR_EVT_SHUTDOWN  0x100
#define  PGR_EVT_START_GRAB 0x200
#define  PGR_EVT_STOP_GRAB  0x400
#define  PGR_EVT_RELEASE    0x800
#define  PGR_EVT_RESTART    0x1000
#define  PGR_EVT_INIT       0x2000
#define  PGR_EVT_LOCAL_STOP  0x4000
#define  PGR_EVT_LOCAL_START 0x8000

#define COLOR_BW          0 // no color info
#define COLOR_BY_SOFTWARE 1 // RAW image received, convert to color
#define COLOR_BY_CAMERA   2 // Camera did color coding (YUV, RGB, BGR formats received)

#ifndef FULL_32BIT_VALUE
#define FULL_32BIT_VALUE 0x7FFFFFFF
#endif 

typedef UINT Error ; // for Onvif

/** Pixel formats available for Format7 modes. */
enum PixelFormat
{        
  PIXEL_FORMAT_MONO8     = 0x80000000, /**< 8 bits of mono information. */
  PIXEL_FORMAT_411YUV8   = 0x40000000, /**< YUV 4:1:1. */
  PIXEL_FORMAT_422YUV8   = 0x20000000, /**< YUV 4:2:2. */
  PIXEL_FORMAT_444YUV8   = 0x10000000, /**< YUV 4:4:4. */
  PIXEL_FORMAT_RGB8      = 0x08000000, /**< R = G = B = 8 bits. */
  PIXEL_FORMAT_MONO16    = 0x04000000, /**< 16 bits of mono information. */
  PIXEL_FORMAT_RGB16     = 0x02000000, /**< R = G = B = 16 bits. */
  PIXEL_FORMAT_S_MONO16  = 0x01000000, /**< 16 bits of signed mono information. */
  PIXEL_FORMAT_S_RGB16   = 0x00800000, /**< R = G = B = 16 bits signed. */
  PIXEL_FORMAT_RAW8      = 0x00400000, /**< 8 bit raw data output of sensor. */
  PIXEL_FORMAT_RAW16     = 0x00200000, /**< 16 bit raw data output of sensor. */
  PIXEL_FORMAT_MONO12    = 0x00100000, /**< 12 bits of mono information. */
  PIXEL_FORMAT_RAW12     = 0x00080000, /**< 12 bit raw data output of sensor. */
  PIXEL_FORMAT_BGR       = 0x80000008, /**< 24 bit BGR. */
  PIXEL_FORMAT_BGRU      = 0x40000008, /**< 32 bit BGRU. */
  PIXEL_FORMAT_RGB       = PIXEL_FORMAT_RGB8, /**< 24 bit RGB. */
  PIXEL_FORMAT_RGBU      = 0x40000002, /**< 32 bit RGBU. */
  PIXEL_FORMAT_BGR16     = 0x02000001, /**< R = G = B = 16 bits. */
  PIXEL_FORMAT_BGRU16    = 0x02000002, /**< 64 bit BGRU. */
  PIXEL_FORMAT_422YUV8_JPEG      = 0x40000001, /**< JPEG compressed stream. */
  NUM_PIXEL_FORMATS	   =  20, /**< Number of pixel formats. */
  UNSPECIFIED_PIXEL_FORMAT = 0 /**< Unspecified pixel format. */
};

enum PropertyType
{
  BRIGHTNESS, /**< Brightness. */
  AUTO_EXPOSURE, /**< Auto exposure. */
  SHARPNESS, /**< Sharpness */
  WHITE_BALANCE, /**< White balance. */
  HUE, /**< Hue. */
  SATURATION, /**< Saturation. */
  GAMMA, /**< Gamma. */
  IRIS, /**< Iris. */
  FOCUS, /**< Focus. */
  ZOOM, /**< Zoom. */
  PAN, /**< Pan. */
  TILT, /**< Tilt. */
  SHUTTER, /**< Shutter. */
  GAIN, /**< Gain. */
  TRIGGER_MODE, /**< Trigger mode. */
  TRIGGER_DELAY, /**< Trigger delay. */
  FRAME_RATE, /**< Frame rate. */
  TEMPERATURE, /**< Temperature. */
  UNSPECIFIED_PROPERTY_TYPE, /**< Unspecified property type. */
  PROPERTY_TYPE_FORCE_32BITS = FULL_32BIT_VALUE

};


class IP_OnvifCam : public C1394Camera 
{
  enum CamTriggerMode
  {
    TrigNotSupported = -1 ,
    TriggerOff ,
    TriggerOn
  } m_TriggerMode ;
  static void NewImageCallBack( void * pImage , const void * pCallbackData ) ;
private:
  static FXLockObject           m_ConfigLock ;
  //static CAMERA1394::CameraInfo m_CamInfo[MAX_CAMERASNMB] ;
  static FXArray<DWORD>          m_BusyCameras ;
  static unsigned int                    m_iCamNum ;

  FXLockObject            m_LocalConfigLock ;
//   FlyCapture2::CameraInfo m_CameraInfo ;
//   FlyCapture2::BusManager m_BusManager ;
//   FlyCapture2::CameraBase    * m_pCamera ;
//   FlyCapture2::CameraBase    * m_pGUICamera ;
  bool                    m_bGUIReconnect ;

//   InterfaceType m_IntfType ;
//   PGRGuid		  m_CurrentCameraGUID;
//   Mode		    m_Mode;
//   Format7ImageSettings m_f7is ; 
//   Format7PacketInfo    m_f7pi ;
//   FXArray <StrobeInfo>  m_StrobeInfo ;
//   EmbeddedImageInfo m_embeddedInfo;
  //   FlyCapture2::Image * m_pNewImage ;
  CInputConnector *             m_pStreamInput ;
  COutputConnector *            m_pStreamOutput ;
  void * m_pNewImage ; // for Onvif (look above)
  FXArray <PropertyType,PropertyType> m_PropForChange ;
  PixelFormat m_pixelFormat;
  Error         m_LastError;
  DWORD         m_BusEvents ;
  int           m_iNoCameraStatus ;
  int           m_iNNoSerNumberErrors ;
  bool          m_bInitialized ;
  bool          m_bStopInitialized ;

  unsigned int  m_uiSerialNumber ;
  unsigned int  m_uiConnectedSerialNumber ;
  bool		    m_FormatNotSupportedDispayed;

  BITMAPINFOHEADER m_BMIH;
  BITMAPINFOHEADER m_RealBMIH;

  UINT        m_uiPacketSize ;
  float       m_fPercentage ;
  int         m_iFPSx10 ;
  int         m_iWBRed ;
  int         m_iWBlue ;

  static FXLockObject m_ConnectionLock;
  static FXLockObject m_GrabLock;
  FXLockObject m_SettingsLock;

  static FXLockObject  m_IPCamArrLock ;
  
  CRect       m_CurrentROI;
  FXString    m_CameraID;
  FXString    m_ControlThreadName ;
  FXString    m_CallbackThreadName ;
  volatile bool m_continueGrabThread;                 //
//  bool m_isSelectingNewCamera;
  bool m_disableEmbeddedTimeStamp;
//   unsigned int m_prevWidth;
//   unsigned int m_prevHeight;

  double m_dLastStartTime ;
  double m_dLastInCallbackTime ;
  CVideoFrame * m_pNewFrame ;
  HANDLE      m_WaitEventFrameArray[2] ;   // For m_evFrameReady and m_evExit
                                           // in CAmeraDoGrab function
  HANDLE      m_WaitEventBusChangeArr[2] ; // for m_evExit and m_evBusChange
                                           // in DoGrabLoop
  HANDLE      m_evFrameReady ;
  HANDLE      m_evBusChange ;
  HANDLE      m_hGrabThreadHandle ;
  HANDLE      m_hCamAccessMutex ;
  DWORD       m_dwGrabThreadId ;
#ifdef _DEBUG
      FXString m_MutexHolder ;
      int      m_iMutexTakeCntr ;
#endif

  DWORD       m_dwNArrivedEvents ;
  bool        m_bCamerasEnumerated ;
  bool        m_bRescanCameras ;

  bool        m_bWasStopped ; // = true when camera was stopped on ScanProperties
                              //   or set properties trough control pin
  bool        m_bShouldBeReprogrammed ;
  BOOL        m_bLocalStopped ;
public:
  IP_OnvifCam();
  ~IP_OnvifCam() ;
  virtual int GetInputsCount() { return 2 ; }
  virtual int GetOutputsCount() { return 2 ; }
  virtual CInputConnector * GetInputConnector( int iNum )
  {
    if ( iNum == 0 )
      return m_pInputTrigger ;
    else if ( iNum == 1 )
      return m_pStreamInput ;
    return NULL ;
  }
  virtual COutputConnector * GetOutputConnector( int iNum )
  {
    if ( iNum == 0 )
      return m_pOutput ;
    else if ( iNum == 1 )
      return m_pStreamOutput ;
    return NULL ;
  }
  virtual bool DriverInit();
  virtual bool EnumCameras();
  virtual bool CameraInit();
  virtual void CameraClose();

  virtual bool DriverValid();
  virtual bool CameraStart();
  virtual void CameraStop();

  virtual bool GetCameraProperty(unsigned i, FXSIZE &value, bool& bauto);
  virtual bool SetCameraProperty(unsigned i, FXSIZE &value, bool& bauto, bool& Invalidate);
  virtual bool SetCameraPropertyEx(unsigned i, FXSIZE &value, bool& bauto, bool& Invalidate);

  virtual CVideoFrame* CameraDoGrab(double* StartTime);
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
  DECLARE_RUNTIME_GADGET(IP_OnvifCam);
private:
  bool BuildPropertyList();
  void GetROI(CRect& rc);
  void SetROI(CRect& rc);
  bool SetStrobe( const FXString& STrobeDataAsText ,  int iIndex ) ;
  void GetStrobe( FXString& StrobeDataAsText , int iIndex ) ;

  LPCTSTR GetGadgetInfo() { return (LPCTSTR)m_GadgetInfo ; }

   ///////////////////////////////////////////////////////////////////////////////////////
  public:
	static IP_OnvifCam* m_pIPCamGadgets[MAX_CAMERASNMB];
	void SetRelevantCamera(LONG lLoginID, char *pchDVRIP);
	unsigned char* m_pImage ;
	DWORD m_dwPacketSize;
	HANDLE  m_evGrabbed;
	HANDLE  m_evReconnect;
	bool CopyBuffer(unsigned char* data , DWORD size);
	void ReConnect(LONG lLoginID, char *pchDVRIP,  LONG nDVRPort);
	//void OpenStream();
	//void CloseStream();
	long m_LogIn;
    bool m_bReconnect;
    int	m_nIndex;
	CVideoFrame* DoH264VideoFrame( LPBYTE pData , UINT uiDataLen );
private:
	BOOL InitSDK();
	static DEV_INFO m_CamInfo[MAX_CAMERASNMB] ;
	static SDK_CONFIG_NET_COMMON m_CamSearchData[MAX_CAMERASNMB];
	static unsigned int m_iNCam ;
	DEV_INFO *pData;
	long m_nPlaydecHandle;
	int Stream(DEV_INFO *pDev, int nChannel);
	int Connect();
	void UpdatePlayhandle();
    long m_iPlayhandle;	//play handle
	DEV_INFO * GetSelectedCamData();
	int IP_OnvifCam::SerialToNmb(int nserial);

	IPCamInfo  m_IPCamBusInfo;
	CSharedMemBoxes m_IPCamShared;
	
	//////////////////////////////////////////////////////////////////////////////////////


protected:
//   void GetCamResolutionAndPixelFormat(unsigned int* rows, unsigned int* cols, PixelFormat* pixelFmt); //
//   BOOL GetDimensionsFromVideoMode(VideoMode mode, unsigned int* rows, unsigned int* cols); //
//   BOOL GetPixelFormatFromVideoMode(VideoMode mode, bool stippled, PixelFormat* pixFormat); //
//   unsigned int GetBppFromPixelFormat( PixelFormat pixelFormat );                           //
//   bool CheckCameraPower(FlyCapture2::CameraBase * am);                                                                 //
//   bool EnableEmbeddedTimeStamp(FlyCapture2::CameraBase* pCam);                              //
//   bool DisableEmbeddedTimeStamp(FlyCapture2::CameraBase* cam);                             //
//   /** Camera arrival callback handle. */
//   FlyCapture2::CallbackHandle m_cbArrivalHandle;
// 
//   /** Camera removal callback handle. */
//   FlyCapture2::CallbackHandle m_cbRemovalHandle;
// 
//   /** Camera reset callback handle. */
//   FlyCapture2::CallbackHandle m_cbResetHandle;
//   virtual void IP_Onvif_DeleteCam( FlyCapture2::CameraBase* pCam ) ;

  // The object grab image loop.  Only executed from within the grab thread.
  static DWORD WINAPI DoGrabLoop( LPVOID pParam );
  bool IsGrabThreadRunning();        //

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
  virtual bool IP_Onvif_CamInit() ; // Main == true => capture camera
  //      == false => GUI camera
  virtual void IP_Onvif_CameraStop();
  virtual void IP_Onvif_CamClose() ; // Main == true => capture camera
  //      == false => GUI camera
  virtual void IP_Onvif_LocalStreamStart() ;
  virtual void IP_Onvif_LocalStreamStop() ;


  inline bool IP_Onvif_DoAndWait( DWORD EvtMask , DWORD dwTimeOut = 1000 )
  {
    if ( m_continueGrabThread )
    {
      m_BusEvents |= EvtMask ;
      SetEvent( m_evBusChange ) ;
      SetEvent( m_evFrameReady ) ;
      DWORD dwTicks = GetTickCount() ;
      while ( (GetTickCount() - dwTicks) < dwTimeOut ) 
      {
        if ( !(m_BusEvents & EvtMask) )
          return true ;
        Sleep(10) ;
      }
      return false ;
    }
    return true ;
  }
  inline bool CheckAndStopCapture()
  {
    bool bWasRunning = IsRunning() ;
    if ( bWasRunning )
    {
      bool bRes =IP_Onvif_DoAndWait( PGR_EVT_STOP_GRAB , 1000) ;
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
//   inline FlyCapture2::CameraBase * GetCamera(
// #ifdef _DEBUG
//     LPCTSTR MutexHolder = NULL
// #endif
//    )
//   {
//     if ( GetCamMutex(
// #ifdef _DEBUG
//       MutexHolder
// #endif
//       ) )
//     {
//       return m_pCamera ;
//     }
//     return NULL ; // can't take mutex
//   }
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
//   int GetColorMode()
//   {
//     if ( m_pixelFormat == PIXEL_FORMAT_RAW8
//       || m_pixelFormat == PIXEL_FORMAT_RAW16
//       || m_pixelFormat == PIXEL_FORMAT_RAW12 )
//       return COLOR_BY_SOFTWARE ; // colors by program
//     if ( m_pixelFormat == PIXEL_FORMAT_411YUV8
//       || m_pixelFormat == PIXEL_FORMAT_422YUV8
//       || m_pixelFormat == PIXEL_FORMAT_444YUV8
//       || m_pixelFormat == PIXEL_FORMAT_422YUV8_JPEG
//       || m_pixelFormat == PIXEL_FORMAT_RGB8
//       || m_pixelFormat == PIXEL_FORMAT_RGB16
//       || m_pixelFormat == PIXEL_FORMAT_S_RGB16
//       || m_pixelFormat == PIXEL_FORMAT_BGRU16
//       || m_pixelFormat == PIXEL_FORMAT_BGR16
//       || m_pixelFormat == PIXEL_FORMAT_BGR )
//       return COLOR_BY_CAMERA ; // color by camera
//     return COLOR_BW ; // bw mode
//   }
  static friend void CALLBACK InputStreamProcessor(CDataFrame* pDataFrame, void* pGadget, CConnector* lpInput)
  {
    ((IP_OnvifCam*)pGadget)->ProcessNextInputStreamChunk(pDataFrame);
  }
  void ProcessNextInputStreamChunk( CDataFrame * pFrame) ;
};





#endif // !defined(AFX_IP_OnvifCam_H__0C071980_0F39_40DC_A144_B6BEDB69E5DC__INCLUDED_)

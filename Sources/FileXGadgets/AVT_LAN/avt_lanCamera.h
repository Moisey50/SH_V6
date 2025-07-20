// avt2_11Camera.h: interface for the avt2_11 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AVT2_11CAMERA_H__84FCD8FA_3B15_4BDE_99A7_EC03545294C4__INCLUDED_)
#define AFX_AVT2_11CAMERA_H__84FCD8FA_3B15_4BDE_99A7_EC03545294C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <PvAPI.h>
#include <..\..\gadgets\cameras\common\gadgets\1394Camera.h>

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
  FGP_HUE,                                      // Set image hue
  FGP_SATURATION,                               // Set color saturation
  FGP_GAMMA,                                    // Set gamma
  FGP_SHUTTER,                                  // Shutter time
  FGP_GAIN,                                     // Gain
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

typedef CPtrList PvFrames ;

enum CamEvents
{
  EventAcquisitionStart = 40000 ,
  EventAcquisitionEnd ,
  EventFrameTrigger ,
  EventExposureEnd ,
  EventAcquisitionRecordTrigger ,
  EventSyncIn1Rise = 40010 ,
  EventSyncIn1Fall ,
  EventSyncIn2Rise ,
  EventSyncIn2Fall ,
  EventOverflow = 65534 ,
  EventError = 65535
};

// #define ACK_START_MODE "AcqStartTriggerMode"
// #define ACK_START_ASYNC "Disabled"
// #define ACK_START_EVT  "AcqStartTriggerEvent"

class CameraAttribute
{
public:
  CameraAttribute() 
  { 
    memset(&m_Info, 0, (LPBYTE)&m_EnumRange - (LPBYTE)&m_Info) ; 
  }
  FXString             m_Name ;
  tPvAttributeInfo    m_Info ;
  bool                m_bIsSupported ;
  ULONG               m_ulRange[2] ;
  float               m_fRange[2] ;
  tPvInt64            m_i64Range[2] ;
  tPvInt32            m_intVal ;
  tPvBoolean          m_boolVal ;
  tPvUint32           m_uintVal ;
  tPvFloat32          m_floatVal ;
  tPvInt64            m_int64Val ;
  FXStringArray        m_EnumRange ;
  FXString             m_enumVal ;
  FXString             m_stringVal ;
  CameraAttribute& operator = (const CameraAttribute& Orig)
  {
    m_Name = Orig.m_Name ;
    memcpy( &m_Info , &Orig.m_Info , (LPBYTE)&m_EnumRange - (LPBYTE)&m_Info ) ;
    m_EnumRange.RemoveAll() ;
    m_EnumRange.Append( Orig.m_EnumRange );
    m_enumVal = Orig.m_enumVal ;
    m_stringVal = Orig.m_stringVal ;
    return *this ;
  }
};

typedef CArray<CameraAttribute> CamAttribs ;

class avt_lan : public C1394Camera  
{
  friend void PVDECL FrameDoneCB( tPvFrame * pFrame ) ;
  friend void PVDECL CameraLinkEventCB( void* Context,
    tPvInterface Interface,
    tPvLinkEvent Event,
    unsigned long UniqueId ) ;
  friend void PVDECL CameraEventCB(void* Context,
    tPvHandle hCamera,
    const tPvCameraEvent* EventList,
    unsigned long EventListLength) ;
protected:
  static bool       m_bAvtLanDriverReady ;
  static bool       m_bCamLANStatusChanged ;
  static long       m_lAvtLanInstCntr ;
  static FXLockObject m_avtLANInitLock ;

  tPvHandle         m_Handle ; // camera handle 
  tPvIpSettings     m_IpSttings ;
  tPvErr            m_LastError ;
  int               m_iLast1394Format ;
  tPvBayerPattern   m_BayerPattern ;
  float             m_fFrameRate;
  CamAttribs        m_CamAttribs ;
//   tPvAttrListPtr    m_pAttributesList ; // will be received from camera driver
//   unsigned long     m_ulNAttributes ;
  BITMAPINFOHEADER  m_BMIH;
  BITMAPINFOHEADER  m_RealBMIH;
  CSize             m_SensorSize ;
  ULONG             m_ulImageSize ;
  CRect             m_CurrentROI;
  FXLockObject       m_Lock;
  FXString           m_CameraID;
  bool              FrameInfoOn;
  tPvCameraInfoEx   m_nodesinfo[MAX_CAMERASNMB];
  PvFrames          m_FreeFrames ;
  PvFrames          m_Readyframes ;
  HANDLE            m_hFrameReady ;
  CUIntArray        m_AvailableCameras ;
  int               m_iAllocatedFrames ;
  FXStringArray      m_SupportedFormats ;
  int               m_iAvailableExtTriggers ;

  bool              m_bMono , m_bMonoPrev ;
  bool              m_bExpAuto , m_bExpAutoPrev ;
  bool              m_bGainAuto ;
  bool              m_bFixedRate , m_bPrevFixedRate ;
  bool              m_bContinuous ;
  bool              m_bWhiteBalAuto ;
  bool              m_bEventCBExists ;
  int               m_iHardTrigger ;
  tPvImageFormat    m_Format , m_PrevFormat ;
  bool              m_bAcqusitionStoped ;

public:
	avt_lan();
    // HW stuff
  virtual bool DriverInit();
  virtual bool Init() ;
  virtual bool CloseDriver() ;
  virtual int  DiscoverCameras( double dTimeOut = 2000. ) ; // returns amount of cameras
  virtual bool ScanSettings( FXString& text);

  virtual bool CameraInit();
  virtual void CameraClose();

  virtual bool CameraStart();
  virtual void CameraStop();

  bool GetCameraProperty(unsigned i, int &value, bool& bauto);
  bool SetCameraProperty(unsigned i, int &value, bool& bauto, bool& Invalidate);

  virtual CVideoFrame* CameraDoGrab();
    // Capture gadget stuff
  virtual void ShutDown();

  DECLARE_RUNTIME_GADGET(avt_lan);
private: // helpers
        bool            BuildPropertyList();
        bool            CameraConnected() { return (m_Handle != NULL); };
        bool            IsTriggerMode();
        int             GetTriggerMode( char * bufOut , int bufOutLen );
        void            SetTriggerMode(int iMode);
        void            GetROI(CRect& rc);
        bool            SetROI(CRect& rc);
        bool            GetROIFromCamera( CRect& rc ) ;
        ULONG             GetLongExposure() ;
        void            SetLongExposure( int iExp ) ;
        void            SetSendFrameInfo( int iSend ) ;
        bool            IsFrameInfoAvailable() ; // also availability check
        ULONG           GetTriggerDelay() ;
        void            SetTriggerDelay( ULONG iDelay_uS ) ;
        bool            SetGrab( int iNFrames ) ;
        bool            GetGrabConditions( 
          bool& bContinuous , int& iNRestFrames ) ;
        ULONG           GetMaxHeight() ;
        ULONG           GetMaxWidth() ;
        ULONG           GetImageSize( tPvImageFormat PixelFormat ,
                          CRect ROI = CRect(0,0,0,0) ) ;
        bool            IsPropertySupported( const char * PropName ) ;
        bool            GetAttrULONG( const char * PropName , ULONG& ulVal ) ;
        bool            SetAttrULONG( const char * PropName , ULONG ulVal ) ;
        bool            GetRangeULONG( const char * PropName , 
                          ULONG& ulMin , ULONG& ulMax ) ;
        bool            GetAttrInt64( const char * PropName , tPvInt64& i64Val ) ;
        bool            SetAttrInt64( const char * PropName , tPvInt64 i64Val ) ;
        bool            GetRangeInt64( const char * PropName , 
                          tPvInt64& i64Min , tPvInt64& i64Max ) ;
        bool            GetAttrBool( const char * PropName , tPvBoolean& bVal ) ;
        bool            SetAttrBool( const char * PropName , tPvBoolean bVal ) ;
        bool            GetAttrFloat( const char * PropName , float& fVal ) ;
        bool            SetAttrFloat( const char * PropName , float fVal ) ;
        bool            GetRangeFloat( const char * PropName , 
                          float& fMin , float& fMax ) ;
        bool            GetAttrEnum( const char * PropName , 
                          char * EnumString , ULONG ulLen , ULONG& WrittenLen ) ;
        bool            GetAttrEnum( const char * PropName , FXString& EnumString) ;
        bool            SetAttrEnum( const char * PropName , char * EnumString ) ;
        bool            RunCamCommand( const char * CmdName ) ;
        bool            GetEnumsForAttr( const char * PropName , 
                          char * csv , ULONG ulLen , ULONG& WrittenLen ) ;
        bool            GetAttrString( const char * PropName , 
                          char * ValString , ULONG ulLen , ULONG& WrittenLen ) ;

        tPvFrame *      AllocateCameraFrame() ;
        tPvFrame *      AllocateCameraFrame( ULONG ulSize ) ;
        int             ReleaseCameraFrame( tPvFrame * pFrame ) ;
        CVideoFrame *   ConvertPvToSH( tPvFrame * pFrame ) ;
        int             ReallocFramesIfNecessary() ;
        bool            ReleaseAllFrames() ;

        inline bool     avt_lan::ReallocFrame( tPvFrame * pFrame , ULONG ulSize )
        {
          if ( pFrame->ImageBufferSize != ulSize )
          {
            delete pFrame->ImageBuffer ;
            pFrame->ImageBuffer = new BYTE[ulSize] ;
            if ( pFrame->ImageBuffer )
            {
              pFrame->ImageBufferSize = ulSize ;
              return true ;
            }
            else
              return false ;
          }
          return true ;
        }


        bool            QueueFreeFrames( ULONG ulImageSize ) ;
};

#endif // !defined(AFX_AVT2_11CAMERA_H__84FCD8FA_3B15_4BDE_99A7_EC03545294C4__INCLUDED_)

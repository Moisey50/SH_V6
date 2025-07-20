// IDSCamera.h: interface for the IDS class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IDSCAMERA_H__84FCD8FA_3B15_4BDE_99A7_EC03545294C4__INCLUDED_)
#define AFX_IDSCAMERA_H__84FCD8FA_3B15_4BDE_99A7_EC03545294C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "uEye.h"
#include <..\..\gadgets\cameras\common\gadgets\1394Camera.h>

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
  FGP_XSIZE,                                    // Image XSize
  FGP_YSIZE,                                    // Image YSize
  FGP_XPOSITION,                                // Image x position
  FGP_YPOSITION,                                // Image y position
  FGP_PACKETSIZE,                               // Packet size
  FGP_DMAMODE,                                  // DMA mode (continuous or limp)
  FGP_BURSTCOUNT,                               // Number of images to produce
  FGP_FRAMEBUFFERCOUNT,                         // Number of frame buffers
  FGP_USEIRMFORBW,                              // Allocate bandwidth or not (IsoRscMgr)
  FGP_ADJUSTPARAMETERS,                         // Adjust parameters or fail
  FGP_STARTIMMEDIATELY,                         // Start bursting immediately
  FGP_FRAMEMEMORYSIZE,                          // Read only: Frame buffer size
  FGP_COLORFORMAT,                              // Read only: Colorformat
  FGP_IRMFREEBW,                                // Read only: Free iso bytes for 400MBit
  FGP_DO_FASTTRIGGER,                           // Fast trigger (no ACK)
  FGP_DO_BUSTRIGGER,                            // Broadcast trigger
  FGP_RESIZE,                                   // Start/Stop resizing
  FGP_USEIRMFORCHN,                             // Get channel over isochronous resource manager
  FGP_CAMACCEPTDELAY,                           // Delay after writing values
  FGP_ISOCHANNEL,                               // Iso channel
  FGP_CYCLETIME,                                // Read cycle time
  FGP_DORESET,                                  // Reset camera
  FGP_DMAFLAGS,                                 // Flags for ISO DMA
  FGP_R0C,                                      // Ring 0 call gate
  FGP_BUSADDRESS,                               // Exact bus address
  FGP_CMDTIMEOUT,                               // Global bus command timeout
  FGP_CARD,                                     // Card number of this camera (set before connect)
  FGP_LICENSEINFO,                              // Query license information
  FGP_PACKETCOUNT,                              // Read only: Packet count
  FGP_DO_MULTIBUSTRIGGER,                       // Do trigger on several busses
  FGP_CARDRESET,                                // Do reset on card (for hard errors)

  FGP_LAST
};


class IDS : public C1394Camera  
{
protected:
    HIDS         m_hCam ;
    UINT         m_LastError;
    char *       m_pImgMem ;
    int          m_iMemId ;
    CSize        m_SensorSize ;
    int          m_iMemPitch ;
    int          m_iCurrentFormat ;
    int          m_iCurrentResolution ;
    int          m_iShutterPercChange ;
    IMAGE_FORMAT_LIST * m_pImageFormats ;
    SENSORINFO   m_SensorInfo ;
    UEYE_AUTO_INFO m_AutoInfo ;
    int          m_Format ;  // color format
//     FG_COLORMODE    m_Format;
//     FG_FRAMERATE    m_FrameRate;
    BITMAPINFOHEADER m_BMIH;
    BITMAPINFOHEADER m_RealBMIH;
    CRect           m_CurrentROI;
    int             m_iPixelDepth ;
    FXLockObject     m_Lock;
    FXString         m_CameraID;
    bool            m_bFrameInfoOn;
    HANDLE          m_hGrabEndEvent ;
    bool            m_bLiveVideo ;
    bool            m_bInGrab ;
    int             m_iNorderedFrames ;
public:
	IDS();
    // HW stuff
    virtual bool DriverInit();
    virtual bool CameraInit();
    virtual void CameraClose();

    virtual bool CameraStart();
    virtual void CameraStop();

    bool GetCameraProperty(unsigned i, FXSIZE &value, bool& bauto);
    bool SetCameraProperty(unsigned i, FXSIZE &value, bool& bauto, bool& Invalidate);

    virtual CVideoFrame* GetCapturedFrame( double * pdStartTime );
    // Capture gadget stuff
    virtual void ShutDown();
    virtual CDataFrame* GetNextFrame(double* StartTime) ;
    DECLARE_RUNTIME_GADGET(IDS);
private: // helpers
        bool            BuildPropertyList();
        bool            CameraConnected() { return (m_hCam!=NULL); };
        bool            IsTriggerByInputPin();
        int             GetTriggerMode();
        void            SetTriggerMode(int iMode);
        void            GetROI(CRect& rc);
        bool            SetROI(CRect& rc);
        int             GetLongExposure( bool& bAuto ) ;
        void            SetLongExposure( int iExp , bool bAuto ) ;
        int             GetGain( bool& bAuto) ;
        void            SetGain( int iGain , bool bAuto ) ;
        void            SetSendFrameInfo( int iSend ) ;
        DWORD           IsFrameInfoAvailable() ; // also availability check
        int             GetTriggerDelay() ;
        void            SetTriggerDelay( int iDelay_uS ) ;
        bool            SetGrab( int iNFrames ) ;
        bool            GetGrabConditions( bool& bContinuous , int& iNRestFrames ) ;
        bool            AllocateImage() ;
        bool            CorrectBMPIH() ;
};

#endif // !defined(AFX_IDSCAMERA_H__84FCD8FA_3B15_4BDE_99A7_EC03545294C4__INCLUDED_)

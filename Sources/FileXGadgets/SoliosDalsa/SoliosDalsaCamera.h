// SoliosDalsaCamera.h: interface for the SoliosDalsa class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SoliosDalsaCAMERA_H__84FCD8FA_3B15_4BDE_99A7_EC03545294C4__INCLUDED_)
#define AFX_SoliosDalsaCAMERA_H__84FCD8FA_3B15_4BDE_99A7_EC03545294C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <..\..\gadgets\cameras\common\gadgets\1394Camera.h>
#include "SMSoliosRequest.h"
#include "imcntl.h"

#ifndef BASETYPE_H
  #define BASETYPE_H

  #ifndef UINT32
  #define UINT32 unsigned long
  #endif

  #ifndef SINT32
  #define SINT32 long
  #endif

  #ifndef UINT16
  #define UINT16 unsigned short
  #endif

  #ifndef SINT16
  #define SINT16 short
  #endif

  #ifndef UINT8
  #define UINT8 unsigned char
  #endif

  #ifndef SINT8
  #define SINT8 char
  #endif

  #ifndef TRUE
  #define TRUE            1
  #endif

  #ifndef FALSE
  #define FALSE           0
  #endif

  #ifndef NULL
  #define NULL 0
  #endif

  #ifndef NIL
  #define NIL  ((void*)-1)
  #endif

  #ifndef _UINT32HL
    typedef struct
    {
      UINT32        Low;
      UINT32        High;
    }UINT32HL;
    #define _UINT32HL
  #endif
#endif



// Parameters
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

typedef struct tagCamProperties
{
  FG_PARAMETER        pr;
  const char *        name; 
}CamProperties;

typedef struct                                  // Info for a device
{
  UINT32HL      Guid;                           // GUID of this device
  UINT8         CardNumber;                     // Card number
  UINT8         NodeId;                         // Depends on bus topology
  UINT8         Busy;                           // Actually busy
}FGNODEINFO;


class SoliosDalsa : public C1394Camera  
{
protected:
    UINT           m_LastError;
    BITMAPINFOHEADER m_BMIH;
    BITMAPINFOHEADER m_RealBMIH;
    CRect           m_CurrentROI;
    FXLockObject     m_Lock;
    FXString         m_CameraID;
    FXString         m_ControlProgram ;
    bool            m_bCamApplWasstarted ;
    IImCNTL         m_ImCNTL ;;
    CSMSoliosRequest  * m_pCamRequest ;
    int             m_iFramesPerSecond ;
    BOOL            m_bTriggerMode ;  // true - external trigger
    int             m_iShutter_us ;
    int             m_iGain ; // index 0->1 1->2 2->4
    BOOL            m_bLiveVideo ;
    BOOL            m_bTransferImages ;
public:
	SoliosDalsa();
    // HW stuff
    virtual bool DriverInit();
    virtual bool CameraInit();
    virtual void CameraClose();

    virtual bool CameraStart();
    virtual void CameraStop();

    bool GetCameraProperty(unsigned i, int &value, bool& bauto);
    bool SetCameraProperty(unsigned i, int &value, bool& bauto, bool& Invalidate);

    virtual CVideoFrame* CameraDoGrab(double* dStartTime);
    // Capture gadget stuff
    virtual void ShutDown();

    DECLARE_RUNTIME_GADGET(SoliosDalsa);
private: // helpers
    bool            BuildPropertyList();
    bool            ScanSettings(FXString& text) ;
    virtual bool    PrintProperties(FXString& text);
    virtual bool    ScanProperties(LPCTSTR text, bool& Invalidate);
    bool            CameraConnected() { return (m_CurrentCamera != -1); };
    int             GetTriggerMode();
    void            SetTriggerMode(int iMode);
    virtual bool    IsTriggerByInputPin() { return (m_bTriggerMode != FALSE) ; } ;
    void            GetROI(CRect& rc);
    bool            SetROI(CRect& rc) { return true ; } ;
    int             GetLongExposure() ;
    void            SetLongExposure( int iExp ) ;
    bool            SetGrab( int iNFrames ) ;
};

#endif // !defined(AFX_SoliosDalsaCAMERA_H__84FCD8FA_3B15_4BDE_99A7_EC03545294C4__INCLUDED_)

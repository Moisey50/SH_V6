// avt3_11Camera.h: interface for the AVTF class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AVTFCAMERA_H__84FCD8FA_3B15_4BDE_99A7_EC03545294C4__INCLUDED_)
#define AFX_AVTFCAMERA_H__84FCD8FA_3B15_4BDE_99A7_EC03545294C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <FGCamera.h>
#include <gadgets\CDevice.h>
#include <math\Intf_sup.h>
#include <VimbaImageTransform/Include/VmbTransform.h>
#include <VimbaImageTransform/Include/VmbCommonTypes.h>

struct Corresp
{
  DWORD dwSerial ;
  int   iCB_Index ;
  int   iUsing_Index ;
};

inline LPCTSTR ErrCodeToMessage( VmbError_t eError )
{
  switch( eError )
  {
  case VmbErrorSuccess:           return ( "Success." );
  case VmbErrorInternalFault:     return ( "Unexpected fault in VmbApi or driver." ) ;
  case VmbErrorApiNotStarted:     return ( "API not started." ) ;
  case VmbErrorNotFound:          return ( "Not found." ) ;
  case VmbErrorBadHandle:         return ( "Invalid handle " ) ;
  case VmbErrorDeviceNotOpen:     return ( "Device not open." ) ;
  case VmbErrorInvalidAccess:     return ( "Invalid access." ) ;
  case VmbErrorBadParameter:      return ( "Bad parameter." ) ;
  case VmbErrorStructSize:        return ( "Wrong DLL version." ) ;
  case VmbErrorMoreData:          return ( "More data returned than memory provided." ) ;
  case VmbErrorWrongType:         return ( "Wrong type." ) ;
  case VmbErrorInvalidValue:      return ( "Invalid value." ) ;
  case VmbErrorTimeout:           return ( "Timeout." ) ;
  case VmbErrorOther:             return ( "TL error." ) ;
  case VmbErrorResources:         return ( "Resource not available." ) ;
  case VmbErrorInvalidCall:       return ( "Invalid call." ) ;
  case VmbErrorNoTL:              return ( "TL not loaded." ) ;
  case VmbErrorNotImplemented:    return ( "Not implemented." ) ;
  case VmbErrorNotSupported:      return ( "Not supported." ) ;
  default:                        return ( "Unknown" ) ;
  }
}

class AVTF : public CDevice  
{
  friend void __stdcall AVTSystemCallback ( void* pContext , UINT32 uiParam , void * lParam ) ;
protected:
  CFGCamera       m_Camera;
  UINT            m_LastError;
  FG_COLORMODE    m_ColorMode ;
  UINT            m_ColorModeIndex ;
  FG_FRAMERATE    m_FrameRate;
  FG_RESOLUTION   m_Resolution;
  BITMAPINFOHEADER m_BMIH;
  BITMAPINFOHEADER m_RealBMIH;
  CRect            m_CurrentROI;
  FXLockObject     m_Lock;
  FXString         m_CameraID;
  bool             m_bFrameInfoOn;
  FGPINFO          m_PacketSize ;
  int              m_iNGrabSet;
  static FGINIT    m_InitData ;
  static FXArray<AVTF* , AVTF*> m_AVTGadgets ;
  static FXLockObject m_avtInitLock ;
  static bool         m_avtDrvReady ;
public:
  AVTF();
  // HW stuff
  virtual bool DriverInit();
  virtual bool EnumDevices();
  virtual bool DeviceInit();
  virtual void DeviceClose();
  virtual bool BuildPropertyList();

  virtual bool DeviceStart();
  virtual void DeviceStop();

  bool GetDeviceProperty(unsigned i, INT_PTR &value, bool& bauto);
  bool SetDeviceProperty(unsigned i, INT_PTR value, bool& bauto, bool& Invalidate);

  virtual CVideoFrame* DeviceDoGrab(double* StartTime);
  // Capture gadget stuff
  virtual void ShutDown();
  static void BusCallbackProcess( void* pContext , UINT32 uiParam , void * lParam ) ;
  static FXLockObject * GetInitLockObject() { return &m_avtInitLock ; } ;

  DECLARE_RUNTIME_GADGET(AVTF);
private: // helpers
  bool            IsDeviceConnected() { return (m_Camera.GetPtrDCam()!=NULL); };
  bool            IsTriggerByInputPin();
  int             GetTriggerMode();
  void            SetTriggerMode(int iMode);
  void            GetROI(CRect& rc);
  bool            SetROI(CRect& rc);
  int             GetLongExposure() ;
  void            SetLongExposure( int iExp ) ;
  void            SetSendFrameInfo( int iSend ) ;
  DWORD           IsFrameInfoAvailable() ; // also availability check
  int             GetTriggerDelay() ;
  void            SetTriggerDelay( int iDelay_uS ) ;
  bool            SetGrab( int iNFrames ) ;
  bool            GetGrabConditions( bool& bContinuous , INT_PTR& iNRestFrames ) ;
  static  LPCTSTR       GetErrorMessage( UINT32 errCode ) ;
  static bool avtInit( AVTF * pGadget ) ;
  static void avtDone( AVTF * pGadget ) ;

public:
  bool UpdateBMIH(void);
  bool GetBitCountAndSizeCoeff( FG_COLORMODE ColorMOde , WORD& iBitCount, double& dSizeCoeff);
  virtual bool RestartDevice(void);
  double m_dRestartRequested;
  double m_dRestartedTime ;
  double m_dLastFrameTime ;
  double m_dAverageFrameInterval ;
  double m_dAverageConstant ;
  int    m_iRestartRetryCnt ;
  double m_dLastBusCallbackTime ;
  int    m_iNLastMessagesInBurst ;
};

#endif // !defined(AFX_AVTFCAMERA_H__84FCD8FA_3B15_4BDE_99A7_EC03545294C4__INCLUDED_)

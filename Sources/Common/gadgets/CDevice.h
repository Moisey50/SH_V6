// CDevice.h: interface for the CDevice class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_Device_H__2D237E9D_6C31_4347_A976_91CF227172B3__INCLUDED_)
#define AFX_Device_H__2D237E9D_6C31_4347_A976_91CF227172B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <gadgets\VideoFrame.h>
#include <gadgets\gadbase.h>
#include <helpers\SharedMemBoxes.h>

#ifndef _UINT32HL
typedef struct
{
  UINT32        Low;
  UINT32        High;
}UINT32HL;
#define _UINT32HL
#endif


namespace Device
{
#define SYNC_INPUT 1
#define CTRL_INPUT 2

#define WRONG_PROPERTY (-1000)

  class DeviceInfo
  {
  public:
    DeviceInfo()
    {
      serialnmb = 0; ulProcessId = 0 ;
    }
    unsigned long serialnmb ;
    unsigned long ulCardId ;
    unsigned long ulProcessId ;
    unsigned long ulInterfaceType;
    UINT32HL      Guid;                           // GUID of this device
    TCHAR         Id[ 50 ] ;
    TCHAR         model[ 50 ] ;
    TCHAR         szSerNum[ 50 ] ;
    TCHAR         InterfaceId[ 50 ] ;
    TCHAR         m_sGivenName[ 256 ] ;
  } ;

  typedef struct tagProperty
  {
    unsigned    id;
    FXString    name;
    FXParser    property;
  } Property;

  class GlobalDeviceInfo
  {
  public:
    DWORD       dwProcessId ;
    DWORD       dwSerialNumber ;
    int         iCardNumber ;
    UINT32HL    Guid;                           // GUID of this device
    int         iMatchedWithNew ;
    char        szSerialNumber[ 50 ] ;
    char        szModelName[ 50 ] ;
    char        szCameraName[ 50 ] ;
    char        szInterfaceOrClass[ 50 ] ;

    GlobalDeviceInfo()
    {
      memset( this , 0 , sizeof( *this ) ) ;
    }
  } ;
};

enum PropertyDataType
{
  PDTUnknown = 0 ,
  PDTBool ,
  PDTInt ,
  PDTFloat ,
  PDTString ,
  PDTEnum ,
  PDTCommand ,
  PDTCategory ,
  PDTExternal
};

LPCTSTR GetTypeName( PropertyDataType Type ) ;

class PropertyValue
{
public:
  bool     bBool ;
  int      iInt ;
  long long i64Int ;
  double   dDouble ;
  TCHAR    szAsString[ 300 ] ;
  double   dMin ;
  double   dMax ;

  PropertyValue()
  {
    Reset() ;
  }

  void Reset()
  {
    memset( this , 0 , sizeof( *this ) ) ;
  }
  PropertyValue& operator=( const PropertyValue& Other )
  {
    memcpy( this , &Other , sizeof( *this ) ) ;
    return *this ;
  }
};


typedef struct tagCamProperties
{
  DWORD               pr;
  const char *        name;
  bool                bSupported ;
  int                 AutoControl ; // index of auto control property
  PropertyDataType    TypeInCamera ;
  PropertyDataType    TypeInSetup ;
  PropertyValue       LastValue ;
  PropertyValue       Ordered ;
} CamProperties;

inline bool IsAuto( FXString& sValue , INT_PTR& iVal )
{
  bool bRes = false ;
  FXString SmallLetters = sValue.MakeLower() ;
  if ( SmallLetters.Find( "auto" ) == 0 )
  {
    bRes = true;
    SmallLetters.Delete( 0 , 4 ) ;
    SmallLetters = SmallLetters.Trim() ;
    if ( SmallLetters.GetLength() <= 0 )
      return bRes ;
  }
  iVal = atoi( SmallLetters ) ;

  return bRes ;
}
// 
// const DWORD MS_VC_EXCEPTION = 0x406D1388;
// 
// #pragma pack(push,8)
// typedef struct tagTHREADNAME_INFO
// {
//   DWORD dwType; // Must be 0x1000.
//   LPCSTR szName; // Pointer to name (in user addr space).
//   DWORD dwThreadID; // Thread ID (-1=caller thread).
//   DWORD dwFlags; // Reserved for future use, must be zero.
// } THREADNAME_INFO;
// #pragma pack(pop)
// 
// inline void SetThreadName( LPCSTR pName , DWORD ThreadId )
// {
//   THREADNAME_INFO info;
//   info.dwType = 0x1000;
//   info.dwFlags = 0;
//   info.dwThreadID = ThreadId;
//   info.szName = pName;
//   __try
//   {
//     RaiseException( MS_VC_EXCEPTION , 0 , sizeof( info ) / sizeof( ULONG_PTR ) , (ULONG_PTR*) &info );
//   }
//   __except ( EXCEPTION_EXECUTE_HANDLER )
//   {}
// }


#define MAX_DEVICESNMB 64

#define DEVICESENDINFO_0(sz)        FxSendLogMsg(MSG_INFO_LEVEL,GetDriverInfo(),0,sz)
#define DEVICESENDINFO_1(sz,a)      FxSendLogMsg(MSG_INFO_LEVEL,GetDriverInfo(),0,sz,a)
#define DEVICESENDINFO_2(sz,a,b)    FxSendLogMsg(MSG_INFO_LEVEL,GetDriverInfo(),0,sz,a,b)
#define DEVICESENDINFO_3(sz,a,b,c)  FxSendLogMsg(MSG_INFO_LEVEL,GetDriverInfo(),0,sz,a,b,c)

#define DEVICESENDTRACE_0(sz)       FxSendLogMsg(MSG_DEBUG_LEVEL,GetDriverInfo(),0,sz)
#define DEVICESENDTRACE_1(sz,a)     FxSendLogMsg(MSG_DEBUG_LEVEL,GetDriverInfo(),0,sz,a)
#define DEVICESENDTRACE_2(sz,a,b)   FxSendLogMsg(MSG_DEBUG_LEVEL,GetDriverInfo(),0,sz,a,b)
#define DEVICESENDTRACE_3(sz,a,b,c) FxSendLogMsg(MSG_DEBUG_LEVEL,GetDriverInfo(),0,sz,a,b,c)

#define DEVICESENDWARN_0(sz)        FxSendLogMsg(MSG_WARNING_LEVEL,GetDriverInfo(),0,sz)
#define DEVICESENDWARN_1(sz,a)      FxSendLogMsg(MSG_WARNING_LEVEL,GetDriverInfo(),0,sz,a)
#define DEVICESENDWARN_2(sz,a,b)    FxSendLogMsg(MSG_WARNING_LEVEL,GetDriverInfo(),0,sz,a,b)
#define DEVICESENDWARN_3(sz,a,b,c)  FxSendLogMsg(MSG_WARNING_LEVEL,GetDriverInfo(),0,sz,a,b,c)

#define DEVICESENDERR_0(sz)         FxSendLogMsg(MSG_ERROR_LEVEL,GetDriverInfo(),0,sz)
#define DEVICESENDERR_1(sz,a)       FxSendLogMsg(MSG_ERROR_LEVEL,GetDriverInfo(),0,sz,a)
#define DEVICESENDERR_2(sz,a,b)     FxSendLogMsg(MSG_ERROR_LEVEL,GetDriverInfo(),0,sz,a,b)
#define DEVICESENDERR_3(sz,a,b,c)   FxSendLogMsg(MSG_ERROR_LEVEL,GetDriverInfo(),0,sz,a,b,c)

#define DEVICESENDFAIL_0(sz)        FxSendLogMsg(MSG_CRITICAL_LEVEL,GetDriverInfo(),0,sz)
#define DEVICESENDFAIL_1(sz,a)      FxSendLogMsg(MSG_CRITICAL_LEVEL,GetDriverInfo(),0,sz,a)
#define DEVICESENDFAIL_2(sz,a,b)    FxSendLogMsg(MSG_CRITICAL_LEVEL,GetDriverInfo(),0,sz,a,b)
#define DEVICESENDFAIL_3(sz,a,b,c)  FxSendLogMsg(MSG_CRITICAL_LEVEL,GetDriverInfo(),0,sz,a,b,c)

#define SEND_DEVICE_INFO(sz,...)        FxSendLogMsg(MSG_INFO_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)
#define SEND_DEVICE_TRACE(sz,...)        FxSendLogMsg(MSG_DEBUG_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)
#define SEND_DEVICE_WARN(sz,...)        FxSendLogMsg(MSG_WARNING_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)
#define SEND_DEVICE_ERR(sz,...)        FxSendLogMsg(MSG_ERROR_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)
#define SEND_DEVICE_FAIL(sz,...)        FxSendLogMsg(MSG_CRITICAL_LEVEL,GetGadgetInfo(),0,sz,##__VA_ARGS__)



class CDevice : public CCaptureGadget ,
  public FXInstanceCntr
{
protected: // gadget stuff
  FXString          m_GadgetInfo;
  CInputConnector*  m_pInputTrigger;
  CDuplexConnector* m_pControl;
  HANDLE			      m_evSWTriggerPulse;
  HANDLE            m_TriggerEvents[ 2 ] ;
  FXString          m_Status;
  bool              m_bDeviceRunning ;
  bool              m_bWasStopped ;
  BOOL              m_bInScanProperties ;
  bool              m_bAttachedAndShouldBeRestarted ;
  bool              m_bGadgetInputConnected ;
  bool              m_bSoftwareTriggerMode ;
  bool              m_bHardwareTrigger ;
  bool              m_bWasTriggerOnPin = false ;
protected: // HW specific stuff
  unsigned int    m_FrameNmb;
  unsigned int    m_CurrentDevice;   // real connected serial number (-1 if not connected)
  unsigned int    m_dwSerialNumber ; // Assigned by graph script or by Async control
  CArray<Device::Property , Device::Property&> m_Properties;
  bool            m_bEnumerated ;

  FXPropertyKit m_InitialProperties ;  // property kit for initial download set
  bool          m_bInitialOK ;         // Was initial download done
protected:
  int         SerialToNmb( unsigned serial );
  int         SerialToNmb( LPCTSTR pSerial );
  int         SaveDevInfo( Device::GlobalDeviceInfo& Info , int iIndex ) ;
  bool        IsDigitField( unsigned ID );
public:
  static CSharedMemBoxes     m_Devices ;
  static Device::DeviceInfo  m_DevicesInfo[ MAX_DEVICESNMB ];
  static ULONG               m_NConnectedDevices;
  static bool                m_bInBusCallback ;
public:
  CDevice( DWORD Inputs = SYNC_INPUT | CTRL_INPUT ,
    int iNBoxes = 64 ,
    int iBoxSizeBytes = 4 ,
    LPCTSTR pAreaName = _T( "SH_DEVICE" ) );
  // helpers
  unsigned GetPropertyId( LPCTSTR name );
  // HW stuff
  virtual bool DriverInit()
  {
    return false;
  }
  virtual bool EnumDevices()
  {
    return false;
  }
  virtual bool DeviceInit()
  {
    return false;
  }
  virtual bool BuildPropertyList()
  {
    return false ;
  }
  virtual bool IsDeviceConnected()
  {
    return false ;
  }
  virtual void DeviceClose()
  {}

  virtual bool DriverValid();
  virtual bool DeviceStart();
  virtual void DeviceStop()
  {};
  virtual bool IsDeviceRunning()
  {
    return false ;
  }

  virtual bool IsRunning();

  virtual bool GetDeviceProperty( unsigned i , INT_PTR &value , bool& bauto )
  {
    return false;
  }
  virtual bool SetDeviceProperty( unsigned i , INT_PTR value , bool& bauto , bool& Invalidate )
  {
    return false;
  }

  virtual void DeviceTriggerPulse( CDataFrame* pDataFrame );

  virtual CVideoFrame* DeviceDoGrab( double* dStartTime );

  // Capture gadget stuff
  virtual void ShutDown();

  virtual void OnStart()
  {
    DeviceStart();
    CCaptureGadget::OnStart();
  }
  virtual void OnStop()
  {
    DeviceStop(); CCaptureGadget::OnStop();
  }

  virtual bool PrintProperties( FXString& text );
  virtual bool ScanProperties( LPCTSTR text , bool& Invalidate );
  virtual bool ScanSettings( FXString& text );
  virtual bool ShouldBeStoppedOnReprogramming()
  {
    return false ;
  }

  virtual CDataFrame* GetNextFrame( double * dStartTime );
  virtual bool IsSNLegal( FXString& SN )
  {
    return (!SN.IsEmpty() && (SN != _T( "-1" )) && ( SN != _T( "-2" ) ) ) ;
  }

  virtual int GetInputsCount()
  {
    return (m_pInputTrigger) ? 1 : 0;
  }
  virtual CInputConnector* GetInputConnector( int n )
  {
    return (n == 0) ? m_pInputTrigger : NULL;
  }
  virtual int GetDuplexCount()
  {
    return (m_pControl) ? 1 : 0;
  }
  virtual CDuplexConnector* GetDuplexConnector( int n )
  {
    return (n == 0) ? m_pControl : NULL;
  }

  virtual void AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame );

  virtual LPCTSTR GetDriverInfo()
  {
    return m_GadgetInfo;
  }
  bool ReleaseDevice() ;
  bool TakeDevice( DWORD dwSerialNUmber ) ;

  virtual bool SetSoftwareTriggerMode( bool bSet )
  {
    m_bSoftwareTriggerMode = bSet ;
    return true ;
  }
  bool IsTriggerByInputPin()
  {
    return ((m_pInputTrigger) && (m_pInputTrigger->IsConnected()));
  }
  static friend void CALLBACK cdevice_fn_capture_trigger( CDataFrame* pDataFrame , void* pGadget , CConnector* lpInput )
  {
    ((CDevice*) pGadget)->DeviceTriggerPulse( pDataFrame );
  }
  DECLARE_RUNTIME_GADGET( CDevice );
  virtual bool ReastartDevice( void );
};

#endif // !defined(AFX_Device_H__2D237E9D_6C31_4347_A976_91CF227172B3__INCLUDED_)

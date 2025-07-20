// WDMCaptureGadget.h: interface for the WDMCapture class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WDMCAPTUREGADGET_H__223100C3_7BA5_46C0_ACE1_095BA792BF53__INCLUDED_)
#define AFX_WDMCAPTUREGADGET_H__223100C3_7BA5_46C0_ACE1_095BA792BF53__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include <gadgets\VideoFrame.h>
#include <DShow.h>
#include "WdmSetup.h"
#include "VFilter.h"
#include "crossbar.h"
#include <helpers\Hic.h>

//#if (WINVER < 0x0500)

#define DBT_DEVTYP_DEVICEINTERFACE      0x00000005  // device interface class
#define DEVICE_NOTIFY_WINDOW_HANDLE     0x00000000
typedef  PVOID           HDEVNOTIFY;

//#endif //#if (WINVER < 0x0500)

extern "C"
{
  typedef BOOL(/* WINUSERAPI */ WINAPI *PUnregisterDeviceNotification )(
    IN HDEVNOTIFY Handle
    );

  typedef HDEVNOTIFY(/* WINUSERAPI */ WINAPI *PRegisterDeviceNotificationA )(
    IN HANDLE hRecipient ,
    IN LPVOID NotificationFilter ,
    IN DWORD Flags
    );

  typedef HDEVNOTIFY(/* WINUSERAPI */ WINAPI *PRegisterDeviceNotificationW )(
    IN HANDLE hRecipient ,
    IN LPVOID NotificationFilter ,
    IN DWORD Flags
    );
}

#ifdef UNICODE
#define PRegisterDeviceNotification  PRegisterDeviceNotificationW
#else
#define PRegisterDeviceNotification  PRegisterDeviceNotificationA
#endif // !UNICODE

typedef struct _tag_capstuff
{
  // Interfaces
  ICaptureGraphBuilder2  *s_pBuilder;
  IMoniker               *s_rgpmVideoMenu[ 30 ];
  IMoniker               *s_pmVideo;
  IBaseFilter            *s_pVCap;
  IBaseFilter            *s_pRenderer;
  IGraphBuilder          *s_pFg;
  IAMDroppedFrames       *s_pDF;
  //    IFileSinkFilter        *s_pSink;
  IMediaControl          *s_pMediaControl;
  IConfigAviMux          *s_pConfigAviMux;
  IAMVideoCompression    *s_pVC;
  IAMStreamConfig        *s_pVSC;      // for video cap
  IAMVfwCaptureDialogs   *s_pDlg;
  IMediaEventEx          *s_pME;
  CVFilter               *s_VideoRenderer;
  // State and settings
  LPBITMAPINFOHEADER      s_lpBMIH;
  int                     s_iNumVCapDevices;
  int                     s_iDevNumSelected;
  bool                    s_bUseFrameRate;
  bool                    s_fCapturing;
  bool                    s_fCCAvail;
  bool                    s_fCapCC;
  bool                    s_fDeviceMenuPopulated;
  bool                    s_fPreviewing;
  bool                    s_fCaptureGraphBuilt;
  bool                    s_fPreviewGraphBuilt;
  bool                    s_fWantPreview;
  bool                    s_fWantCapture;
  long                    s_lDroppedBase;
  long                    s_lNotBase;
  double                  s_FrameRate;
  LONG                    s_NumberOfVideoInputs;
  // data
  WCHAR                   s_wachFriendlyName[ 120 ];
  CCrossbar              *s_pCrossbar;
  // notificators
  //    PUnregisterDeviceNotification s_gpUnregisterDeviceNotification;
  //    PRegisterDeviceNotification   s_gpRegisterDeviceNotification;
  //    HDEVNOTIFY                    s_ghDevNotify;
  CWnd*                         s_NotifyWnd;
}_capstuff;

class WDMCapture : public CCaptureGadget ,
  public _tag_capstuff
{
  friend bool InitCapFilters( WDMCapture* drv );
  friend HRESULT BuildCaptureGraph( WDMCapture* drv );
  friend void CALLBACK WDMInputCalback( CDataFrame* lpData , void* lpParam , CConnector* lpInput );
  friend class CWdmSetup;
private:
  CArray<CString , CString&> m_Devices;
  ULONG                    m_ulNUSBDevices ; // not only video in
  CInputConnector*         m_pInputTrigger;
  CMediaType *             m_pMediaType;
  BITMAPINFOHEADER         m_BMIH;
  DWORD                    m_OutputFormat;
  CString                  m_szVideoDisplayName;
  CString                  m_Location ;
  CString                  m_VID_PID_MI ;
  CInputConnector*		     m_pInput;
  HANDLE				           m_InputWanted;
  BOOL                     m_PauseMode;
  CvidcHic                 m_MJPGHic;
  FXLockObject             m_StateLock;
private:
  HRESULT EnumDevices();
  HRESULT SelectDevice( const char *szVideo );
  HRESULT SelectDevice( IMoniker *pmVideo );
  void    TearDownGraph();
  void	OnInput( CDataFrame* lpData );
public:
  void DoSend( pTVFrame inFrame );
  WDMCapture();
  virtual void ShutDown();
  int		DoJob();
  bool    Init();
  void    Close();

  virtual int GetInputsCount() { return 1; }
  CInputConnector* GetInputConnector( int n ) { return ((n == 0) ? m_pInput : NULL); }
  BOOL    SyncMode() { return m_pInput->IsConnected(); }

  virtual void OnStart();
  virtual void OnStop();
  virtual bool PrintProperties( FXString& text );
  virtual bool ScanProperties( LPCTSTR text , bool& Invalidate );
  virtual bool ScanSettings( FXString& text );

  void    RestoreGraph();
  void    DestroyGraph();

  LRESULT OnFGNotify( WPARAM wParam , LPARAM lParam );
  void    NotifyMediaType( CMediaType *pMediaType );
  HRESULT DoRenderSample( IMediaSample *pMediaSample );
  const char * GetCfgDialogs( SetupID id );

  CString GetDeviceName( int ItemNo ) { return(m_Devices[ ItemNo ]); };

  bool    CanDecompressMJPG() { return m_MJPGHic.CanDeCompress(); }

protected:
  bool ShowSetupDialog( CPoint& point );
  DECLARE_RUNTIME_GADGET( WDMCapture );
  friend HRESULT StartCapture( WDMCapture* drv );
};

#endif // !defined(AFX_WDMCAPTUREGADGET_H__223100C3_7BA5_46C0_ACE1_095BA792BF53__INCLUDED_)

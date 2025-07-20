// WDMCaptureGadget.cpp: implementation of the WDMCapture class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WDM.h"
#include "WDMCaptureGadget.h"
#include <video\shvideo.h>
//#include <imageproc/simpleip.h>
#include <Dvdmedia.h>
#include <video\mediasample2tvframe.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#ifdef THIS_MODULENAME
#undef THIS_MODULENAME
#endif

#define THIS_MODULENAME "WDM_Capt"

#if (WINVER < 0x0500)
#pragma message( "Warning! WinVer<0x0500 build" )
#endif

typedef struct _DEV_BROADCAST_DEVICEINTERFACE_A
{
  DWORD       dbcc_size;
  DWORD       dbcc_devicetype;
  DWORD       dbcc_reserved;
  GUID        dbcc_classguid;
  char        dbcc_name[ 1 ];
} DEV_BROADCAST_DEVICEINTERFACE_A , *PDEV_BROADCAST_DEVICEINTERFACE_A;

typedef struct _DEV_BROADCAST_DEVICEINTERFACE_W
{
  DWORD       dbcc_size;
  DWORD       dbcc_devicetype;
  DWORD       dbcc_reserved;
  GUID        dbcc_classguid;
  wchar_t     dbcc_name[ 1 ];
} DEV_BROADCAST_DEVICEINTERFACE_W , *PDEV_BROADCAST_DEVICEINTERFACE_W;

#ifdef UNICODE
typedef DEV_BROADCAST_DEVICEINTERFACE_W   DEV_BROADCAST_DEVICEINTERFACE;
typedef PDEV_BROADCAST_DEVICEINTERFACE_W  PDEV_BROADCAST_DEVICEINTERFACE;
#else
typedef DEV_BROADCAST_DEVICEINTERFACE_A   DEV_BROADCAST_DEVICEINTERFACE;
typedef PDEV_BROADCAST_DEVICEINTERFACE_A  PDEV_BROADCAST_DEVICEINTERFACE;
#endif // UNICODE

//#endif // WINVER

extern "C"
{
  typedef PVOID HDEVINFO;

  typedef struct _DEVICE_GUID_LIST
  {
    HDEVINFO   DeviceInfo;
    LIST_ENTRY ListHead;
  } DEVICE_GUID_LIST , *PDEVICE_GUID_LIST;

  extern DEVICE_GUID_LIST gHubList;
  extern DEVICE_GUID_LIST gDeviceList;

  VOID InitializeListHead( _Out_ PLIST_ENTRY ListHead )
  {
    ListHead->Flink = ListHead->Blink = ListHead;
  }

  VOID EnumerateHostControllers(
    HTREEITEM  hTreeParent ,
    ULONG     *DevicesConnected
    );

  const char * GetDevices() ;
}


class CNtfWnd : public CWnd
{
protected:
  WDMCapture* m_Server;
public:
  virtual BOOL Create( WDMCapture* Server );
  //{{AFX_MSG(CNtfWnd)
  afx_msg LRESULT OnFGNotify( WPARAM wParam , LPARAM lParam );
  afx_msg void OnTimer( UINT_PTR nIDEvent );
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP( CNtfWnd , CWnd )
  //{{AFX_MSG_MAP(CNtfWnd)
  ON_WM_TIMER()
  //}}AFX_MSG_MAP
  ON_MESSAGE( UM_FGNOTIFY , OnFGNotify )
END_MESSAGE_MAP()

#define BKGRND (RGB(0,0,0))

BOOL CNtfWnd::Create( WDMCapture* Server )
{
  m_Server = Server;
  RECT rect = {4, 1, 6, 7};
  CWnd *MainWnd = CWnd::FindWindow( "Progman" , NULL );
  LPCTSTR lpszNtfVClass = AfxRegisterWndClass( CS_PARENTDC , LoadCursor( NULL , IDC_ARROW ) , NULL );

  if ( CWnd::CreateEx( 0 , lpszNtfVClass , "Notify window" , WS_POPUP , rect , MainWnd , 0 , NULL ) )
    return(TRUE);
  return(FALSE);
}

LRESULT CNtfWnd::OnFGNotify( WPARAM wParam , LPARAM lParam )
{
  return (m_Server->OnFGNotify( wParam , lParam ));
}

void CNtfWnd::OnTimer( UINT_PTR nIDEvent )
{
  CWnd::OnTimer( nIDEvent );
}

void IMonRelease( IMoniker *&pm )
{
  if ( pm )
  {
    pm->Release();
    pm = 0;
  }
}

CString Wide2CStr( BSTR bstr )
{
  CString retVal;
  if ( !bstr ) return(retVal);
  int size = 0;
  size = WideCharToMultiByte( CP_ACP , 0 , bstr , -1 , NULL , size , 0 , 0 );
  if ( size == 0 ) return retVal;
  char* pntr = retVal.GetBuffer( size );
  WideCharToMultiByte( CP_ACP , 0 , bstr , -1 , pntr , size , 0 , 0 );
  retVal.ReleaseBuffer();
  return (retVal);
}

bool StopPreview( WDMCapture* drv )
{
  // way ahead of you
  if ( !drv->s_fPreviewing ) return false;

  // stop the graph
  IMediaControl *pMC = NULL;
  HRESULT hr = drv->s_pFg->QueryInterface( IID_IMediaControl , (void **) &pMC );
  if ( SUCCEEDED( hr ) )
  {
    hr = pMC->Stop();
    pMC->Release();
  }
  if ( FAILED( hr ) )
  {
    SENDERR_1( "--- CWDMDriver:: Error %x: StopPreview(): Cannot stop preview graph" , hr );
    return false;
  }
  drv->s_fPreviewing = FALSE;

  return true;
}

void NukeDownstream( WDMCapture* drv , IBaseFilter *pf )
{
  //DbgLog((LOG_TRACE,1,TEXT("Nuking...")));

  IPin *pP , *pTo;
  ULONG u;
  IEnumPins *pins = NULL;
  PIN_INFO pininfo;
  HRESULT hr = pf->EnumPins( &pins );
  pins->Reset();
  while ( hr == NOERROR )
  {
    hr = pins->Next( 1 , &pP , &u );
    if ( hr == S_OK && pP )
    {
      pP->ConnectedTo( &pTo );
      if ( pTo )
      {
        hr = pTo->QueryPinInfo( &pininfo );
        if ( hr == NOERROR )
        {
          if ( pininfo.dir == PINDIR_INPUT )
          {
            NukeDownstream( drv , pininfo.pFilter );
            drv->s_pFg->Disconnect( pTo );
            drv->s_pFg->Disconnect( pP );
            drv->s_pFg->RemoveFilter( pininfo.pFilter );
          }
          pininfo.pFilter->Release();
        }
        pTo->Release();
      }
      pP->Release();
    }
  }
  if ( pins )
    pins->Release();
}


void TearDownGraph( WDMCapture* drv )
{
  if ( drv->s_pRenderer ) drv->s_pRenderer->Release(); drv->s_pRenderer = NULL;
  //if (drv->s_VideoRenderer) delete drv->s_VideoRenderer; drv->s_VideoRenderer=NULL;
  if ( drv->s_pDF )     drv->s_pDF->Release();     drv->s_pDF = NULL;
  if ( drv->s_pVCap )   NukeDownstream( drv , drv->s_pVCap );
  drv->s_fCaptureGraphBuilt = FALSE;
  drv->s_fPreviewGraphBuilt = FALSE;
}

bool StopCapture( WDMCapture* drv )
{
  // way ahead of you
  if ( !drv->s_fCapturing ) return false;
  if ( !drv->s_pFg ) return false;
  // stop the graph
  HRESULT hr;
  ASSERT( drv->s_pMediaControl != NULL );
  if ( drv->s_pMediaControl )
  {
    hr = drv->s_pMediaControl->Stop();
    drv->s_pMediaControl->Release();
    drv->s_pMediaControl = NULL;
  }
  if ( FAILED( hr ) )
  {
    SENDERR_1( "--- CWDMDriver:: Error %x: Cannot stop graph" , hr );
    return false;
  }

  // no more status bar updates
  if ( drv )
  {
    if ( drv->s_NotifyWnd ) drv->s_NotifyWnd->KillTimer( 1 );
    drv->s_fCapturing = false;
  }

  return true;
}

HRESULT StartCapture( WDMCapture* drv )
{
  bool HasStreamControl;
  HRESULT hr;

  if ( drv->s_fCapturing )  return NOERROR;
  if ( drv->s_fPreviewing ) StopPreview( drv );
  if ( !drv->s_fCaptureGraphBuilt ) return E_FAIL;

  drv->s_lDroppedBase = 0;
  drv->s_lNotBase = 0;

  REFERENCE_TIME start = MAX_TIME , stop = MAX_TIME;
  hr = drv->s_pBuilder->ControlStream( &PIN_CATEGORY_CAPTURE , NULL , NULL , &start , NULL , 0 , 0 );
  HasStreamControl = SUCCEEDED( hr );
  //IMediaControl *pMC = NULL;

  ASSERT( drv->s_pMediaControl == NULL );

  hr = drv->s_pFg->QueryInterface( IID_IMediaControl , (void **) &drv->s_pMediaControl );
  if ( FAILED( hr ) )
  {
    SENDERR_1( "Error %x: Cannot get IMediaControl" , hr );
    return E_FAIL;
  }
  if ( HasStreamControl )
    hr = drv->s_pMediaControl->Run();
  else
    hr = drv->s_pMediaControl->Pause();
  if ( FAILED( hr ) )
  {
    // stop parts that started
    drv->s_pMediaControl->Stop();
    drv->s_pMediaControl->Release();
    drv->s_pMediaControl = NULL;
    SENDERR_1( "Error %x: Cannot start graph" , hr );
    return E_FAIL;
  }

  if ( HasStreamControl )
  {
    // we may not have this yet
    if ( !drv->s_pDF )
    {
      hr = drv->s_pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE , &MEDIATYPE_Interleaved ,
        drv->s_pVCap , IID_IAMDroppedFrames , (void **) &drv->s_pDF );
      if ( hr != NOERROR )
        hr = drv->s_pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE , &MEDIATYPE_Video ,
        drv->s_pVCap , IID_IAMDroppedFrames , (void **) &drv->s_pDF );
    }
    hr = drv->s_pBuilder->ControlStream( &PIN_CATEGORY_CAPTURE , NULL , NULL , NULL , &stop , 0 , 0 );
    if ( drv->s_pDF )
    {
      drv->s_pDF->GetNumDropped( &drv->s_lDroppedBase );
      drv->s_pDF->GetNumNotDropped( &drv->s_lNotBase );
    }

  }
  else
  {
    hr = drv->s_pMediaControl->Run();
    if ( FAILED( hr ) )
    {
      // stop parts that started
      drv->s_pMediaControl->Stop();
      drv->s_pMediaControl->Release();
      drv->s_pMediaControl = NULL;
      SENDERR_1( "Error %x: Cannot run graph" , hr );
      return E_FAIL;
    }
  }
  //pMC->Release();
  if ( drv->s_NotifyWnd )
    drv->s_NotifyWnd->SetTimer( 1 , 33 , NULL );
  drv->s_fCapturing = true;

  DbgLog( (LOG_TRACE , 0 , TEXT( "==================================================\n\n" )) );
  DumpGraph( drv->s_pFg , 0 );
  DbgLog( (LOG_TRACE , 0 , TEXT( "\n\n==================================================" )) );

  return hr;
}

HRESULT BuildCaptureGraph( WDMCapture* drv )
{
  HRESULT hr = NOERROR;

  if ( drv->s_fCaptureGraphBuilt ) return hr;
  if ( drv->s_fCapturing || drv->s_fPreviewing ) return hr;

  if ( drv->s_pVCap == NULL ) return E_FAIL;
  if ( drv->s_fPreviewGraphBuilt ) TearDownGraph( drv );

  drv->s_VideoRenderer = new CVFilter( NULL , &hr , drv );
  if ( (!drv->s_VideoRenderer) || (hr != NOERROR) ) return false;
  hr = drv->s_VideoRenderer->QueryInterface( IID_IBaseFilter , (void **) &drv->s_pRenderer );
  hr = drv->s_pFg->AddFilter( drv->s_pRenderer , NULL );

  hr = drv->s_pBuilder->RenderStream( &PIN_CATEGORY_CAPTURE , &MEDIATYPE_Interleaved , drv->s_pVCap , NULL , drv->s_pRenderer );
  if ( hr != NOERROR )
  {
    hr = drv->s_pBuilder->RenderStream( &PIN_CATEGORY_CAPTURE , &MEDIATYPE_Video , drv->s_pVCap , NULL , drv->s_pRenderer );
    if ( hr != NOERROR )
    {
      goto SetupCaptureFail;
    }
  }
  drv->s_fCaptureGraphBuilt = TRUE;
  DbgLog( (LOG_TRACE , 0 , TEXT( "==================================================\n\n" )) );
  DumpGraph( drv->s_pFg , 1 );
  DbgLog( (LOG_TRACE , 0 , TEXT( "\n\n==================================================" )) );
  return NOERROR;
SetupCaptureFail:
  drv->TearDownGraph();
  return E_FAIL;
}


void FreeCapFilters( WDMCapture* drv )
{
  if ( drv->s_pFg )   drv->s_pFg->Release();         drv->s_pFg = NULL;
  if ( drv->s_pBuilder ) drv->s_pBuilder->Release(); drv->s_pBuilder = NULL;
  if ( drv->s_pVCap ) drv->s_pVCap->Release();       drv->s_pVCap = NULL;
  if ( drv->s_pVC )   drv->s_pVC->Release();         drv->s_pVC = NULL;
  if ( drv->s_pVSC )  drv->s_pVSC->Release();        drv->s_pVSC = NULL;
  if ( drv->s_lpBMIH ) free( drv->s_lpBMIH );          drv->s_lpBMIH = NULL;
  if ( drv->s_pDlg )  drv->s_pDlg->Release();        drv->s_pDlg = NULL;
  if ( drv->s_pCrossbar )
  {
    delete drv->s_pCrossbar; drv->s_pCrossbar = NULL;
  }
}

// Make a graph builder object we can use for capture graph building
//
bool MakeBuilder( WDMCapture* drv )
{
  // we have one already
  if ( drv->s_pBuilder ) return true;

  HRESULT hr = ::CoCreateInstance( CLSID_CaptureGraphBuilder2 , NULL ,
    CLSCTX_INPROC , IID_ICaptureGraphBuilder2 ,
    (void **) &drv->s_pBuilder );
  return (hr == NOERROR);
}

// Make a graph object we can use for capture graph building
//
bool MakeGraph( WDMCapture* drv )
{
  HRESULT hr = NOERROR;
  // we have one already
  if ( drv->s_pFg ) return (hr == NOERROR);

  hr = CoCreateInstance( CLSID_FilterGraph , NULL , CLSCTX_INPROC , IID_IGraphBuilder , (void **) &drv->s_pFg );

  return (hr == NOERROR);
}

bool InitCapFilters( WDMCapture* drv )
{
  HRESULT hr = NOERROR;
  UINT uIndex = 0;

  drv->s_fCCAvail = false;	// assume no closed captioning support
  //
  // make a filtergraph, give it to the graph builder and put the video
  // capture filter in the graph
  //
  if ( !MakeGraph( drv ) )
  {
    SENDERR_0( "--- CWDMDriver:: Cannot instantiate filtergraph" );
    goto InitCapFiltersFail;
  }
  if ( !MakeBuilder( drv ) )
  {
    SENDERR_0( "--- CWDMDriver:: Cannot instantiate graph builder" );
    return false;
  }

  hr = drv->s_pBuilder->SetFiltergraph( drv->s_pFg );
  if ( hr != NOERROR )
  {
    SENDERR_0( "--- CWDMDriver:: Cannot give graph to builder" );
    goto InitCapFiltersFail;
  }

  //
  // Next, we need a Video Capture filter, and some interfaces
  //

  drv->s_pVCap = NULL;
  if ( drv->s_pmVideo != 0 )
  {
    IPropertyBag *pBag;
    drv->s_wachFriendlyName[ 0 ] = 0;
    hr = drv->s_pmVideo->BindToStorage( 0 , 0 , IID_IPropertyBag , (void **) &pBag );
    if ( SUCCEEDED( hr ) )
    {
      VARIANT var;
      var.vt = VT_BSTR;
      hr = pBag->Read( L"FriendlyName" , &var , NULL );
      if ( hr == NOERROR )
      {
        lstrcpyW( drv->s_wachFriendlyName , var.bstrVal );
        SysFreeString( var.bstrVal );
      }
      pBag->Release();
    }

    hr = drv->s_pmVideo->BindToObject( 0 , 0 , IID_IBaseFilter , (void**) &drv->s_pVCap );
    if ( drv->s_pVCap )
    {
      IPersistPropertyBag* iPPB;
      drv->s_pVCap->QueryInterface( IID_IPersistPropertyBag , (void**) &iPPB );
      if ( iPPB )
      {
        TRACE( "Success" );
        //iPPB->
      }
    }
  }

  if ( drv->s_pVCap == NULL )
  {
    SENDERR_1( "--- CWDMDriver:: Error %x: Cannot create video capture filter" , hr );
    goto InitCapFiltersFail;
  }

  hr = drv->s_pFg->AddFilter( drv->s_pVCap , NULL );
  if ( hr != NOERROR )
  {
    SENDERR_1( "--- CWDMDriver:: Error %x: Cannot add vidcap to filtergraph" , hr );
    goto InitCapFiltersFail;
  }

  // Calling FindInterface below will result in building the upstream
  // section of the capture graph (any WDM TVTuners or Crossbars we might
  // need).

  // we use this interface to get the name of the driver
  // Don't worry if it doesn't work:  This interface may not be available
  // until the pin is connected, or it may not be available at all.
  // (eg: interface may not be available for some DV capture)
  hr = drv->s_pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE , &MEDIATYPE_Interleaved ,
    drv->s_pVCap , IID_IAMVideoCompression ,
    (void **) &drv->s_pVC );
  if ( hr != S_OK )
  {
    hr = drv->s_pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE , &MEDIATYPE_Video ,
      drv->s_pVCap , IID_IAMVideoCompression ,
      (void **) &drv->s_pVC );
  }

  // !!! What if this interface isn't supported?
  // we use this interface to set the frame rate and get the capture size
  hr = drv->s_pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE , &MEDIATYPE_Interleaved ,
    drv->s_pVCap , IID_IAMStreamConfig ,
    (void **) &drv->s_pVSC );
  if ( hr != NOERROR )
  {
    hr = drv->s_pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE , &MEDIATYPE_Video ,
      drv->s_pVCap , IID_IAMStreamConfig ,
      (void **) &drv->s_pVSC );
    if ( hr != NOERROR )
    {
      // this means we can't set frame rate (non-DV only)
      SENDERR_1( "--- CWDMDriver:: Error %x: Cannot find VCapture:IAMStreamConfig" , hr );
    }
  }
  AM_MEDIA_TYPE *pmt;
  if ( drv->s_pVSC && drv->s_pVSC->GetFormat( &pmt ) == S_OK )
  {
    // DV capture does not use a VIDEOINFOHEADER
    if ( pmt->formattype == FORMAT_VideoInfo )
    {
      ASSERT( drv->s_lpBMIH == NULL );
      drv->s_lpBMIH = (LPBITMAPINFOHEADER) malloc( HEADER( pmt->pbFormat )->biSize );
      memcpy( drv->s_lpBMIH , HEADER( pmt->pbFormat ) , HEADER( pmt->pbFormat )->biSize );
    }
    DeleteMediaType( pmt );
  }
  // we use this interface to bring up the 3 dialogs
  // NOTE:  Only the VfW capture filter supports this.  This app only brings
  // up dialogs for legacy VfW capture drivers, since only those have dialogs
  hr = drv->s_pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE ,
    &MEDIATYPE_Video , drv->s_pVCap ,
    IID_IAMVfwCaptureDialogs , (void **) &drv->s_pDlg );

  // Use the crossbar class to help us sort out all the possible video inputs
  // The class needs to be given the capture filters ANALOGVIDEO input pin
  {
    IPin            *pP = 0;
    IEnumPins       *pins;
    ULONG           n;
    PIN_INFO        pinInfo;
    BOOL            Found = FALSE;
    IKsPropertySet  *pKs;
    GUID            guid;
    DWORD           dw;
    BOOL            fMatch = FALSE;

    drv->s_pCrossbar = NULL;

    if ( SUCCEEDED( drv->s_pVCap->EnumPins( &pins ) ) )
    {
      while ( !Found && (S_OK == pins->Next( 1 , &pP , &n )) )
      {
        if ( S_OK == pP->QueryPinInfo( &pinInfo ) )
        {
          if ( pinInfo.dir == PINDIR_INPUT )
          {
            // is this pin an ANALOGVIDEOIN input pin?
            if ( pP->QueryInterface( IID_IKsPropertySet , (void **) &pKs ) == S_OK )
            {
              if ( pKs->Get( AMPROPSETID_Pin , AMPROPERTY_PIN_CATEGORY , NULL , 0 , &guid , sizeof( GUID ) , &dw ) == S_OK )
              {
                if ( guid == PIN_CATEGORY_ANALOGVIDEOIN )
                  fMatch = TRUE;
              }
              pKs->Release();
            }

            if ( fMatch )
            {
              drv->s_pCrossbar = new CCrossbar( pP );
              hr = drv->s_pCrossbar->GetInputCount( &drv->s_NumberOfVideoInputs );
              Found = TRUE;
            }
          }
          pinInfo.pFilter->Release();
        }
        pP->Release();
      }
      pins->Release();
    }
  }

  IPin *pPin;
  hr = drv->s_pBuilder->FindPin( drv->s_pVCap , PINDIR_OUTPUT , &PIN_CATEGORY_VBI , NULL , FALSE , 0 , &pPin );
  if ( hr != S_OK )
  {
    hr = drv->s_pBuilder->FindPin( drv->s_pVCap , PINDIR_OUTPUT , &PIN_CATEGORY_CC , NULL , FALSE , 0 , &pPin );
  }
  if ( hr == S_OK )
  {
    pPin->Release();
    drv->s_fCCAvail = true;
  }
  else
  {
    drv->s_fCapCC = false;	// can't capture it, then
  }

  // potential debug output - what the graph looks like
  DumpGraph( drv->s_pFg , 1 );

  return (hr == NOERROR);
InitCapFiltersFail:
  FreeCapFilters( drv );
  return false;
}

void CALLBACK WDMInputCalback( CDataFrame* lpData , void* lpParam , CConnector* lpInput )
{
  ((WDMCapture*) lpParam)->OnInput( lpData );
}

IMPLEMENT_RUNTIME_GADGET_EX( WDMCapture , CCaptureGadget , "Video.capture" , TVDB400_PLUGIN_NAME );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WDMCapture::WDMCapture() :
  m_pMediaType( NULL ) ,
  m_PauseMode( FALSE ) ,
  m_MJPGHic( BI_MJPG )
{
  CoInitialize( NULL );
  DbgInitialise( AfxGetApp()->m_hInstance );
  ZeroMemory( (_capstuff*) this , sizeof( _tag_capstuff ) );
  ZeroMemory( s_rgpmVideoMenu , sizeof( s_rgpmVideoMenu ) );
  s_fDeviceMenuPopulated = false;
  s_iDevNumSelected = 0;
  s_NotifyWnd = NULL;
  m_InputWanted = ::CreateEvent( NULL , false , FALSE , NULL );
  m_pOutput = new COutputConnector( vframe );
  m_pInput = new CInputConnector( transparent , WDMInputCalback , this );
  m_SetupObject = new CWdmSetup( this , NULL );
}

void WDMCapture::ShutDown()
{
  CCaptureGadget::ShutDown();
  Close();
  CloseHandle( m_InputWanted ); m_InputWanted = NULL;
  ASSERT( s_pMediaControl == NULL );
  if ( m_pOutput ) delete m_pOutput;	m_pOutput = NULL;
  if ( m_pInput )  delete m_pInput;	    m_pInput = NULL;
  m_Devices.RemoveAll();
  DbgTerminate();
  CoUninitialize();
}

int WDMCapture::DoJob()
{
  CDataFrame* pDataFrame = NULL;
  ASSERT( m_pStatus );
  switch ( m_pStatus->GetStatus() )
  {
    case CExecutionStatus::STOP:
      m_PauseMode = FALSE;
      if ( m_bRun )
      {
        m_bRun = FALSE;
        OnStop();
        pDataFrame = CDataFrame::Create( transparent );
        Tvdb400_SetEOS( pDataFrame );
        m_FrameCounter = 0;
        break;
      }
      else
      {
        HANDLE pEvents[] = {m_evExit, m_pStatus->GetStartHandle()};
        DWORD cEvents = sizeof( pEvents ) / sizeof( HANDLE );
        DWORD retVal = ::WaitForMultipleObjects( cEvents , pEvents , FALSE , INFINITE );
        return WR_CONTINUE;
      }
    case CExecutionStatus::PAUSE:
    {

      if ( !m_pInput->IsConnected() )
        m_PauseMode = TRUE;
      else
      {
        m_PauseMode = FALSE;
        break;
      }
      HANDLE pEvents[] = {m_evExit, m_pStatus->GetStartHandle(),m_pStatus->GetStpFwdHandle()};
      DWORD cEvents = sizeof( pEvents ) / sizeof( HANDLE );
      DWORD retVal = ::WaitForMultipleObjects( cEvents , pEvents , FALSE , INFINITE );
      if ( retVal == 2 )
        ::SetEvent( m_InputWanted );
      break;
    }
    case CExecutionStatus::RUN:
    {
      m_PauseMode = FALSE;
      if ( !m_bRun )
        OnStart();
      m_bRun = TRUE;
      HANDLE pEvents[] = {m_evExit, m_pStatus->GetStopHandle(),m_pStatus->GetPauseHandle()};
      DWORD cEvents = sizeof( pEvents ) / sizeof( HANDLE );
      DWORD retVal = ::WaitForMultipleObjects( cEvents , pEvents , FALSE , INFINITE );
      break;
    }
    case CExecutionStatus::EXIT:
      return WR_EXIT;
    default:
      ASSERT( FALSE );
      return WR_CONTINUE;
  }
  if ( pDataFrame ) // send EOS
  {
    if ( (!m_pOutput) || (!m_pOutput->Put( pDataFrame )) )
      pDataFrame->Release();
  }
  return WR_CONTINUE;
}

bool WDMCapture::Init()
{
  Close();
  HRESULT hr = NOERROR;
  UINT uIndex = 0;
  s_NotifyWnd = new CNtfWnd;

  ((CNtfWnd*) s_NotifyWnd)->Create( this );

  hr |= EnumDevices();

  SelectDevice( m_szVideoDisplayName );
  DEV_BROADCAST_DEVICEINTERFACE filterData;
  ZeroMemory( &filterData , sizeof( DEV_BROADCAST_DEVICEINTERFACE ) );

  filterData.dbcc_size = sizeof( DEV_BROADCAST_DEVICEINTERFACE );
  filterData.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
  filterData.dbcc_classguid = AM_KSCATEGORY_CAPTURE;

  s_NotifyWnd->SendMessage( UM_FGNOTIFY , 0 , 0 );

  return true;
}

void    WDMCapture::Close()
{
  FXAutolock al( m_StateLock );
  if ( s_NotifyWnd )
  {
    if ( s_NotifyWnd->GetSafeHwnd() )
      s_NotifyWnd->DestroyWindow();
    delete s_NotifyWnd; s_NotifyWnd = NULL;
  }
  ::StopCapture( this );
  ::TearDownGraph( this );
  FreeCapFilters( this );
  IMonRelease( s_pmVideo );
  for ( int i = 0; i < NUMELMS( s_rgpmVideoMenu ); i++ )
  {
    IMonRelease( s_rgpmVideoMenu[ i ] ); s_rgpmVideoMenu[ i ] = 0;
  }
  if ( s_pBuilder ) s_pBuilder->Release();
  s_pBuilder = NULL;
  m_bRun = false;
}

void	WDMCapture::OnInput( CDataFrame* lpData )
{
  ::SetEvent( m_InputWanted );
  lpData->Release( lpData );
}

void WDMCapture::OnStart()
{
  FXAutolock al( m_StateLock );
  if ( s_fPreviewing ) ::StopPreview( this );
  if ( s_fPreviewGraphBuilt ) ::TearDownGraph( this );
  s_fWantCapture = true;
  BuildCaptureGraph( this );
  if ( SUCCEEDED( ::StartCapture( this ) ) )
  {
    m_bRun = true;
    s_fWantCapture = true;
    CWdmSetup* pSetupDlg = (CWdmSetup*) m_SetupObject;
    if ( pSetupDlg->GetSafeHwnd() )
    {
      pSetupDlg->OnChangeStatus();
    }
    return;
  }
  SENDERR_1( "Can't start wdm source '%s' for capture" , Wide2CStr( s_wachFriendlyName ) );
  DbgLog( (LOG_TRACE , 0 , TEXT( "==================================================\n\n" )) );
  DumpGraph( s_pFg , 1 );
  DbgLog( (LOG_TRACE , 0 , TEXT( "\n\n==================================================" )) );
  return;
}

void WDMCapture::OnStop()
{
  FXAutolock al( m_StateLock );
  m_bRun = false;
  s_fWantCapture = false;
  ::StopCapture( this );
  //::TearDownGraph(this);
  CWdmSetup* pSetupDlg = (CWdmSetup*) m_SetupObject;
  if ( pSetupDlg->GetSafeHwnd() )
  {
    pSetupDlg->OnChangeStatus();
  }
}

void WDMCapture::DestroyGraph()
{
  FXAutolock al( m_StateLock );
  if ( s_fPreviewing ) ::StopPreview( this );
  if ( s_fCapturing )  ::StopCapture( this );
  if ( (s_fPreviewGraphBuilt) || (s_fCaptureGraphBuilt) ) ::TearDownGraph( this );
}

void WDMCapture::RestoreGraph()
{
  CWaitCursor wc;
  if ( s_fWantCapture )
  {
    ::BuildCaptureGraph( this );
    ::StartCapture( this );
  }
  if ( s_fWantPreview )
  {
    //::BuildPreviewGraph();
    //::StartPreview(this);
  }
}

bool WDMCapture::PrintProperties( FXString& text )
{
  CCaptureGadget::PrintProperties( text );
  EnumDevices();
  FXPropertyKit pc;
  pc.WriteInt( "useframerate" , (int) s_bUseFrameRate );
  pc.WriteInt( "outputformat" , (int) m_OutputFormat );
  WCHAR *wszDisplayName = NULL;
  CString tmpS;
  if ( s_pmVideo )
  {
    if ( SUCCEEDED( s_pmVideo->GetDisplayName( 0 , 0 , &wszDisplayName ) ) )
    {
      if ( wszDisplayName )
      {
        pc.WriteString( "VID_PID_MI" , m_VID_PID_MI , false ) ;
        pc.WriteString( "Location" , m_Location , false ) ;
        m_szVideoDisplayName = Wide2CStr( wszDisplayName );
        CoTaskMemFree( wszDisplayName );
        pc.WriteString( "VideoDevice2s" ,(LPCTSTR) m_szVideoDisplayName , m_szVideoDisplayName.GetLength() + 1 );
        pc.WriteBinary( "VideoDevice2" , (LPBYTE) (LPCTSTR) m_szVideoDisplayName , m_szVideoDisplayName.GetLength() + 1 );
      }
      if ( s_pVSC )
      {
        AM_MEDIA_TYPE *pmt;
        s_pVSC->GetFormat( &pmt );
        pc.WriteBinary( "AM_MEDIA_TYPE" , (LPBYTE) pmt , sizeof( AM_MEDIA_TYPE ) );
        pc.WriteBinary( "pbFormat" , (LPBYTE) (pmt->pbFormat) , pmt->cbFormat );
        DeleteMediaType( pmt );
      }
    }
  }
  text += pc;
  return true;
}

bool WDMCapture::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  if ( text )
  {
    if ( !s_NotifyWnd )
    {
      CString tmpS;
      FXPropertyKit pc( text );
      //pc.GetInt("currentvideomode", (int&)m_CurrentVideoMode);
      if ( !pc.GetInt( "outputformat" , (int&) m_OutputFormat ) )
        m_OutputFormat = BI_YUV9;
      if ( !pc.GetInt( "useframerate" , (int&) s_bUseFrameRate ) )
        s_bUseFrameRate = 1;
      int units_per_frame;
      if ( !pc.GetInt( "framerate" , units_per_frame ) )
        units_per_frame = 666667;
      s_FrameRate = 10000000. / units_per_frame;
      s_FrameRate = (int) (s_FrameRate * 100) / 100.;
      // reasonable default
      if ( s_FrameRate <= 0. ) 
        s_FrameRate = 15.0;
      FXString TmpFX ;
      if ( pc.GetString( "VID_PID_MI" , TmpFX ) )
        m_VID_PID_MI = TmpFX ;
      if ( pc.GetString( "Location" , TmpFX ) )
        m_Location = TmpFX ;

      CString encoded;
      LPBYTE data; 
      FXSIZE size;
      if ( pc.GetBinary( "VideoDevice2" , data , size ) && (size) && (data) )
      {
        if ( data[ size - 1 ] != 0 )
        {
          data = (LPBYTE) realloc( data , size + 1 );
          data[ size ] = 0;
        }
        m_szVideoDisplayName = data;
        free( data );
      }
      Init();
      AM_MEDIA_TYPE *pmt;
      FXSIZE n;
      if ( (pc.GetBinary( "AM_MEDIA_TYPE" , (LPBYTE&) pmt , n )) && (n != 0) && (pmt != NULL) )
      {
        if ( (pc.GetBinary( "pbFormat" , (LPBYTE&) (pmt->pbFormat) , n )) && (n != 0) && (pmt->pbFormat != NULL) )
        {
          if ( s_pVSC )
            s_pVSC->SetFormat( pmt );
          delete[] pmt->pbFormat;
        }
        delete[] pmt;
      }
    }
    else
    {
      FXPropertyKit pc( text );
      pc.GetInt( "outputformat" , (int&) m_OutputFormat );
    }
  }
  else
  {

  }
  return true;
}

bool WDMCapture::ScanSettings( FXString& text )
{
  text = _T( "calldialog(true)" );
  return true;
}

void    WDMCapture::NotifyMediaType( CMediaType *pMediaType )
{
  m_pMediaType = pMediaType;
}

__forceinline void w2a( WCHAR* w , CString&a )
{
  size_t len = wcslen( w );
  char* tmpS = a.GetBuffer( (int)len + 1 );
  wcstombs( tmpS , w , len );
  a.ReleaseBuffer( (int)len );
}


void WDMCapture::DoSend( pTVFrame inFrame )
{
  if ( m_Devices.GetCount() > s_iDevNumSelected )
  {
    pTVFrame pFrame = new TVFrame;
    pFrame->lpBMIH = inFrame->lpBMIH;
    pFrame->lpData = inFrame->lpData;
    CString label = m_Devices[ s_iDevNumSelected ];
  //   w2a( s_wachFriendlyName , label );
    CVideoFrame* vf = CVideoFrame::Create( pFrame );
    vf->SetLabel( label );
    vf->SetTime( GetGraphTime() * 1.e-3 );
    vf->ChangeId( m_FrameCounter++ );
    if ( (!m_pOutput) || (!m_pOutput->Put( vf )) )
      vf->RELEASE( vf );
  }
  return;
}


HRESULT WDMCapture::DoRenderSample( IMediaSample *pMediaSample )
{
  double ts = GetHRTickCount();
  if ( ((!m_pInput->IsConnected()) && (!m_PauseMode)) || (::WaitForSingleObject( m_InputWanted , 0 ) == WAIT_OBJECT_0) )
  {
    pTVFrame result = mediasample2tvframe( m_pMediaType , pMediaSample , m_OutputFormat );
    if ( result )
    {
      DoSend( result );
      free( result );
    }
  }
  AddCPUUsage( GetHRTickCount() - ts );
  return NOERROR;
}

LRESULT WDMCapture::OnFGNotify( WPARAM wParam , LPARAM lParam )
{
  return 0;
}

// synonym of void AddDevicesToMenu() of the sample
#define IFENUMDEV_RETURN(hr,mes) if (hr!=NOERROR) { SENDERR_1("--- CWDMDriver::EnumDevices(): %s",mes); return hr; }
HRESULT WDMCapture::EnumDevices()
{
  HRESULT hr;
  ULONG cFetched;
  IMoniker *pM;
  IEnumMoniker *pEm;
  ICreateDevEnum *pCreateDevEnum;
  UINT    uIndex = 0;

  //    if(s_fDeviceMenuPopulated) return NOERROR;
  s_fDeviceMenuPopulated = true;
  s_iNumVCapDevices = 0;

  // Clean up all
  m_Devices.RemoveAll();
  gHubList.DeviceInfo = INVALID_HANDLE_VALUE;
  InitializeListHead( &gHubList.ListHead );
  gDeviceList.DeviceInfo = INVALID_HANDLE_VALUE;
  InitializeListHead( &gDeviceList.ListHead );
  EnumerateHostControllers( NULL , &m_ulNUSBDevices ) ;
  const char * pAllDevices = GetDevices() ;; // list as text
  CString AllUSBDevices( pAllDevices ) ;
  for ( int i = 0; i < NUMELMS( s_rgpmVideoMenu ); i++ )
  {
    if ( s_rgpmVideoMenu[ i ] ) 
    {
      IMonRelease( s_rgpmVideoMenu[ i ] );
      s_rgpmVideoMenu[ i ] = NULL;
    }
  }

  hr = ::CoCreateInstance( CLSID_SystemDeviceEnum , NULL , 
    CLSCTX_INPROC_SERVER , IID_ICreateDevEnum , (void**) &pCreateDevEnum );
  IFENUMDEV_RETURN( hr , "Error Creating Device Enumerator" );

  hr = pCreateDevEnum->CreateClassEnumerator( CLSID_VideoInputDeviceCategory , &pEm , 0 );
  IFENUMDEV_RETURN( hr , "There are no video capture hardware" );

  pEm->Reset();
  while ( hr = pEm->Next( 1 , &pM , &cFetched ) , hr == S_OK )
  {
    IPropertyBag *pBag;
    hr = pM->BindToStorage( 0 , 0 , IID_IPropertyBag , (void **) &pBag );
    if ( SUCCEEDED( hr ) )
    {
      VARIANT var , var2 ;
      var.vt = VT_BSTR;
      var2.vt = VT_BSTR ;
      hr = pBag->Read( L"FriendlyName" , &var , NULL );
      if ( hr == NOERROR )
      {
        CString Name = Wide2CStr( var.bstrVal ) ;
        //m_Devices.Add( Wide2CStr( var.bstrVal ) );
        if ( s_pmVideo != 0 && (S_OK == s_pmVideo->IsEqual( pM )) ) 
          s_iDevNumSelected = uIndex;
        SysFreeString( var.bstrVal );
        ASSERT( s_rgpmVideoMenu[ uIndex ] == 0 );
        s_rgpmVideoMenu[ uIndex ] = pM;
        pM->AddRef();
        hr = pBag->Read( L"DevicePath" , &var2 , NULL );
        bool bDeviceAdded = false ;
        if ( hr == NOERROR )
        {
          CString Location ;
          CString Id = Wide2CStr( var2.bstrVal ) ;
          int i1stHashPos = Id.Find( '#' ) ;
          int i2ndHashPos = Id.Find( '#' , i1stHashPos + 1) ;
          int i3rdHashPos = Id.Find( '#' , i2ndHashPos + 1) ;
          if ( i2ndHashPos > 0 && i3rdHashPos > 0 )
          {
            CString VID_PID_MI = Id.Mid( i1stHashPos + 1 , i2ndHashPos - i1stHashPos - 1 ).MakeUpper() ;
            CString Instance = Id.Mid( i2ndHashPos + 1 , i3rdHashPos - i2ndHashPos - 1 ).MakeUpper() ;
            int iPos = AllUSBDevices.Find( Instance ) ;
            if ( iPos >= 0 )
            {
              int iNextCR = AllUSBDevices.Find( '\n' , iPos ) ;
              CString OurString = AllUSBDevices.Left( iNextCR ) ;
              int iPrevCR = OurString.ReverseFind( '\n' ) ;
              if ( iPrevCR > 0 )
                OurString = OurString.Mid( iPrevCR + 1 ) ;
              int i1stSlash = OurString.Find( '/' ) ;
              int i2ndSlash = OurString.Find( '/' , i1stSlash + 1 ) ;
              Location = OurString.Mid( i1stSlash , i2ndSlash - i1stSlash + 1 ) ;
              Name += Location ;
            }
            m_Devices.Add( Name ) ;
            bDeviceAdded = true ;
            CString NameInCapital = m_szVideoDisplayName ;
            NameInCapital.MakeUpper() ;
            if ( NameInCapital.Find( VID_PID_MI ) >= 0 )
            {
              if ( NameInCapital.Find( Instance ) >= 0 )
              {
                m_VID_PID_MI = VID_PID_MI ;
                if ( !Location.IsEmpty() )
                  m_Location = Location ;
              }
              else
              {
                if ( Location == m_Location )
                {
                  m_szVideoDisplayName = Id ;
                  m_szVideoDisplayName.Insert( 0 , "@device:pnp:" ) ;
                }
              }
              
            }
          }
          SysFreeString( var2.bstrVal );
        }
        if ( !bDeviceAdded )
          m_Devices.Add( Name + "-Not Detected" ) ;
        {
        }
      }
      pBag->Release();
    }
    pM->Release();
    uIndex++;
  }
  pEm->Release();
  s_iNumVCapDevices = uIndex;
  pCreateDevEnum->Release();
  return NOERROR;
}

HRESULT WDMCapture::SelectDevice( const char *szVideo )
{
  HRESULT hr = NOERROR;
  WCHAR wszVideo[ 2048 ];
  IBindCtx *lpBC;
  IMoniker *pmVideo = 0;
  BOOL bFound = FALSE;

  MultiByteToWideChar( CP_ACP , 0 , szVideo , -1 , wszVideo , NUMELMS( wszVideo ) );
  hr = CreateBindCtx( 0 , &lpBC );
  if ( SUCCEEDED( hr ) )
  {
    DWORD dwEaten;
    hr = MkParseDisplayName( lpBC , wszVideo , &dwEaten , &pmVideo );
    lpBC->Release();
  }
  if ( pmVideo != NULL )
  {
    for ( int i = 0; i < NUMELMS( s_rgpmVideoMenu ); i++ )
    {
      if ( s_rgpmVideoMenu[ i ] != NULL && S_OK == s_rgpmVideoMenu[ i ]->IsEqual( pmVideo ) )
      {
        TRACE( "+++ Found device '%s'\n" , szVideo );
        bFound = TRUE;
        s_iDevNumSelected = i;
        break;
      }
    }
  }

  if ( !bFound )
  {
    TRACE( "!!! Device '%s' not found, load default\n" , szVideo );
    if ( s_iNumVCapDevices > 0 )
    {
      IMonRelease( pmVideo );
      ASSERT( s_rgpmVideoMenu[ 0 ] != NULL );
      pmVideo = s_rgpmVideoMenu[ 0 ];
      pmVideo->AddRef();
    }
    else
    {
      SENDERR_0( "--- CWDMDriver::SelectDevice() - fault" );
      goto CleanUp;
    }
  }
  SelectDevice( pmVideo );
CleanUp:
  IMonRelease( pmVideo );
  return hr;
}

//void ChooseDevices(IMoniker *pmVideo, IMoniker *pmAudio)
HRESULT WDMCapture::SelectDevice( IMoniker *pmVideo )
{
  HRESULT hr = NOERROR;
  if ( s_pmVideo != pmVideo )
  {
    if ( pmVideo ) pmVideo->AddRef();
    IMonRelease( s_pmVideo );
    s_pmVideo = pmVideo;
    if ( s_fPreviewing ) StopPreview( this );
    if ( s_fCaptureGraphBuilt || s_fPreviewGraphBuilt ) ::TearDownGraph( this );
    FreeCapFilters( this );
    InitCapFilters( this );
  }
  return hr;
}

void WDMCapture::TearDownGraph()
{
  if ( s_fPreviewGraphBuilt ) ::TearDownGraph( this );
}

const char * WDMCapture::GetCfgDialogs( SetupID id )
{
  LPCTSTR retVal = NULL;
  HRESULT hr;
  switch ( id )
  {
    // If this device supports the old legacy UI dialogs, offer them
    case     VfwCaptureDialogSource:
      if ( s_pDlg && !s_pDlg->HasDialog( VfwCaptureDialog_Format ) )
        retVal = "Video Format...";
      break;
    case     VfwCaptureDialogFormat:
      if ( s_pDlg && !s_pDlg->HasDialog( VfwCaptureDialog_Source ) )
        retVal = "Video Source...";
      break;
    case     VfwCaptureDialogDisplay:
      if ( s_pDlg && !s_pDlg->HasDialog( VfwCaptureDialog_Display ) )
        retVal = "Video Display...";
      break;

      // New WDM devices support new UI and new interfaces.
      // Your app can use some default property
      // pages for UI if you'd like (like we do here) or if you don't like our
      // dialog boxes, feel free to make your own and programmatically set 
      // the capture options through interfaces like IAMCrossbar, IAMCameraControl
      // etc.

      // There are 9 objects that might support property pages.  Let's go through
      // them.
    case WdmVideoCaptureFilter:
      if ( s_pVCap )
      {
        ISpecifyPropertyPages *pSpec;
        CAUUID cauuid;

        // 1. the video capture filter itself

        hr = s_pVCap->QueryInterface( IID_ISpecifyPropertyPages , (void **) &pSpec );
        if ( hr == S_OK )
        {
          hr = pSpec->GetPages( &cauuid );
          if ( hr == S_OK && cauuid.cElems > 0 )
          {
            retVal = "Video Capture Filter...";
            CoTaskMemFree( cauuid.pElems );
          }
          pSpec->Release();
        }
      }
      break;
      // 2.  The video capture capture pin
    case WdmVideoCapturePin:
      if ( (s_pVCap) && (s_pBuilder) )
      {
        ISpecifyPropertyPages *pSpec;
        CAUUID cauuid;

        IAMStreamConfig *pSC;
        hr = s_pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE ,
          &MEDIATYPE_Interleaved ,
          s_pVCap , IID_IAMStreamConfig , (void **) &pSC );
        if ( hr != S_OK )
          hr = s_pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE ,
          &MEDIATYPE_Video , s_pVCap ,
          IID_IAMStreamConfig , (void **) &pSC );
        if ( hr == S_OK )
        {
          hr = pSC->QueryInterface( IID_ISpecifyPropertyPages , (void **) &pSpec );
          if ( hr == S_OK )
          {
            hr = pSpec->GetPages( &cauuid );
            if ( hr == S_OK && cauuid.cElems > 0 )
            {
              retVal = "Video Capture Pin...";
              CoTaskMemFree( cauuid.pElems );
            }
          }
          pSpec->Release();
        }
        pSC->Release();
      }
      break;

      // 3.  The video capture preview pin.
      // This basically sets the format being previewed.  Typically, you
      // want to capture and preview using the SAME format, instead of having to
      // enter the same value in 2 dialog boxes.  For a discussion on this, see
      // the comment above the MakePreviewGraph function.
    case WdmVideoPreviewPin:
      if ( (s_pVCap) && (s_pBuilder) )
      {
        ISpecifyPropertyPages *pSpec;
        IAMStreamConfig *pSC;
        CAUUID cauuid;

        hr = s_pBuilder->FindInterface( &PIN_CATEGORY_PREVIEW ,
          &MEDIATYPE_Interleaved , s_pVCap ,
          IID_IAMStreamConfig , (void **) &pSC );
        if ( hr != NOERROR )
          hr = s_pBuilder->FindInterface( &PIN_CATEGORY_PREVIEW ,
          &MEDIATYPE_Video , s_pVCap ,
          IID_IAMStreamConfig , (void **) &pSC );
        if ( hr == S_OK )
        {
          hr = pSC->QueryInterface( IID_ISpecifyPropertyPages , (void **) &pSpec );
          if ( hr == S_OK )
          {
            hr = pSpec->GetPages( &cauuid );
            if ( hr == S_OK && cauuid.cElems > 0 )
            {
              retVal = "Video Preview Pin...";
              CoTaskMemFree( cauuid.pElems );
            }
            pSpec->Release();
          }
          pSC->Release();
        }
      }
      break;

      // 4 & 5.  The video crossbar, and a possible second crossbar
    case WdmVideoCrossbar:
    case WdmSecondCrossbar:
      if ( (s_pVCap) && (s_pBuilder) )
      {
        ISpecifyPropertyPages *pSpec;
        CAUUID cauuid;
        IAMCrossbar *pX/*, *pX2*/;
        IBaseFilter *pXF;

        hr = s_pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE ,
          &MEDIATYPE_Interleaved , s_pVCap ,
          IID_IAMCrossbar , (void **) &pX );
        if ( hr != S_OK )
          hr = s_pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE ,
          &MEDIATYPE_Video , s_pVCap ,
          IID_IAMCrossbar , (void **) &pX );
        if ( hr == S_OK )
        {
          hr = pX->QueryInterface( IID_IBaseFilter , (void **) &pXF );
          if ( hr == S_OK )
          {
            hr = pX->QueryInterface( IID_ISpecifyPropertyPages , (void **) &pSpec );
            if ( (hr == S_OK) && (id == WdmVideoCrossbar) )
            {
              hr = pSpec->GetPages( &cauuid );
              if ( hr == S_OK && cauuid.cElems > 0 )
              {
                retVal = "Video Crossbar...";
                CoTaskMemFree( cauuid.pElems );
              }
              pSpec->Release();
            }
            else if ( (hr == S_OK) && (id == WdmSecondCrossbar) )
            {
              /*hr = s_pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, pXF,IID_IAMCrossbar, (void **)&pX2);
              if (hr == S_OK)
              {
                  hr = pX2->QueryInterface(IID_ISpecifyPropertyPages,(void **)&pSpec);
                  if (hr == S_OK)
                  {
                      hr = pSpec->GetPages(&cauuid);
                      if (hr == S_OK && cauuid.cElems > 0)
                      {
                          retVal="Second Crossbar...";
                          CoTaskMemFree(cauuid.pElems);
                      }
                      pSpec->Release();
                  }
                  pX2->Release();
              }*/
            }
            pXF->Release();
          }
          pX->Release();
        }
      }
      break;

      // 6.  The TVTuner
    case WdmTVTuner:
      if ( (s_pVCap) && (s_pBuilder) )
      {
        ISpecifyPropertyPages *pSpec;
        CAUUID cauuid;
        IAMTVTuner *pTV;

        hr = s_pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE ,
          &MEDIATYPE_Interleaved , s_pVCap ,
          IID_IAMTVTuner , (void **) &pTV );
        if ( hr != S_OK )
          hr = s_pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE ,
          &MEDIATYPE_Video , s_pVCap ,
          IID_IAMTVTuner , (void **) &pTV );
        if ( hr == S_OK )
        {
          hr = pTV->QueryInterface( IID_ISpecifyPropertyPages , (void **) &pSpec );
          if ( hr == S_OK )
          {
            hr = pSpec->GetPages( &cauuid );
            if ( hr == S_OK && cauuid.cElems > 0 )
            {
              retVal = "TV Tuner...";
              CoTaskMemFree( cauuid.pElems );
            }
            pSpec->Release();
          }
          pTV->Release();
        }
      }
  } // case (id)
  return retVal;
}


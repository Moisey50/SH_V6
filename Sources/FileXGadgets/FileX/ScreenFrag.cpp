#include "stdafx.h"
// #include <windows.h>
// #include <gdiplusinit.h>
#include "CMouseHook.h"
#include "ScreenFrag.h"
#include "helpers/FramesHelper.h"
#include "fxfc/CSystemMonitorsEnumerator.h"
#define THIS_MODULENAME "ScreenFrag"

//USER_FILTER_RUNTIME_GADGET( ScreenFrag , "Video.capture" );
IMPLEMENT_RUNTIME_GADGET( UserCaptureBaseGadget , CCaptureGadget , "Video.capture" )

IMPLEMENT_RUNTIME_GADGET_EX( ScreenFrag , UserCaptureBaseGadget , "Video.capture" , TVDB400_PLUGIN_NAME )

 

void WindowSelectCallBack( int iMouseX , int iMouseY , HWND hSelected , void * pHost )
{
  ScreenFrag * pGadget = ( ScreenFrag * ) pHost ;
  pGadget->m_hWindow = hSelected ;
  pGadget->m_hWindowAsText.Format( "0x%p" , hSelected ) ;
  CStdSetupDlg * pSetup = ( CStdSetupDlg *)pGadget->GetSetupObject() ;
  if ( pSetup )
  {
    pSetup->SetCellText( "hWindow" , (LPCTSTR)pGadget->m_hWindowAsText ) ;
  }
  if ( pGadget->m_Hook )
  {
    delete pGadget->m_Hook ;
    pGadget->m_Hook = NULL ;
  }
  pGadget->m_bInRreprogram = false ;
}
ScreenFrag::ScreenFrag()
{
  //m_OutputMode = modeReplace;
  m_CaptureArea = CRect( 100 , 100 , 500 , 500 ) ;
  m_OutputFormat = SF_Y800 ;
  m_CaptureMode = SFCM_Absolute ;
  m_Timing = SFT_ByTrigger ;
  m_hWindow = NULL ;
  m_iDisplay = 0 ;
  m_Hook = NULL ;
  m_bInRreprogram = false ;
  init() ;
};


void ScreenFrag::ShutDown()
{
  CCaptureGadget::ShutDown();
};

static const char * pCaptureMode = "Idle;Absolute;Display;WindowName;WindowHandle;" ;
static const char * pOutFormats = "Unknown;Y800;RGB;YUV9;YUV12;" ;
static const char * pTiming = "ByTrigger;ByTimer;" ;

void ScreenFrag::CaptureModeChange(
  LPCTSTR pName , void* pObject , bool& bInvalidate , bool& bRescan )
{
  bInvalidate = true ;
  ScreenFrag * pGadget = ( ScreenFrag* ) pObject ;
  if ( _tcsicmp( pName , "CaptureMode" ) == 0 )
  {
    switch ( pGadget->m_CaptureMode )
    {
      case SFCM_WindowName:
        if ( !pGadget->m_WindowName.IsEmpty() )
        {
          HWND hWnd = FindWindow( NULL , ( LPCTSTR ) pGadget->m_WindowName ) ;
          pGadget->m_hWindow = hWnd ;
        }
        break ;
      case SFCM_Handle:
        break ;
      case SFCM_ByCursor:
        break ;
    }
  }
  if ( _tcsicmp( pName , "Timing" ) == 0 )
  {
    bInvalidate = true ;
    bRescan = true ;
    pGadget->m_iNTrigger = 0 ;
  }
}

void ScreenFrag::PropertiesRegistration()
{
  addProperty( SProperty::COMBO , "CaptureMode" , &m_CaptureMode , SProperty::Long , pCaptureMode ) ;
  addProperty( SProperty::COMBO , "OutFormat" , &m_OutputFormat , SProperty::Long , pOutFormats ) ;
  addProperty( SProperty::COMBO , "Timing" , &m_Timing , SProperty::Long , pTiming ) ;
  SetChangeNotification( _T( "CaptureMode" ) , CaptureModeChange , this );
  SetChangeNotification( _T( "Timing" ) , CaptureModeChange , this );

  if ( m_Timing == SFT_ByTimer )
  {
    addProperty( SProperty::EDITBOX , "Period_ms" , &m_OutPeriod_ms , SProperty::Double ) ;
  }
  const CSystemMonitorsEnumerator * pMonitorsEnum =
    CSystemMonitorsEnumerator::GetMonitorEnums() ;
  switch ( m_CaptureMode )
  {
  case SFCM_Absolute:
    {
      addProperty( SProperty::SPIN , "XOff" , &m_CaptureArea.left , SProperty::Long , 
        pMonitorsEnum->m_FullDesktopRect.left , pMonitorsEnum->m_FullDesktopRect.right - 1 );
      addProperty( SProperty::SPIN , "YOff" , &m_CaptureArea.top , SProperty::Long , 
        pMonitorsEnum->m_FullDesktopRect.top , pMonitorsEnum->m_FullDesktopRect.bottom - 1 );
      addProperty( SProperty::SPIN , "Width" , &m_CaptureArea.right , SProperty::Long , 
        1 , pMonitorsEnum->m_FullDesktopRect.Width() );
      addProperty( SProperty::SPIN , "Height" , &m_CaptureArea.bottom , SProperty::Long , 
        1 , pMonitorsEnum->m_FullDesktopRect.Height() );
    }
    break ;
  case SFCM_Display:
    {
      int iNMonitors = (int)pMonitorsEnum->m_monitorArray.size() ;
      addProperty( SProperty::SPIN , "DisplayNumber" , &m_iDisplay , SProperty::Long , 1 , iNMonitors );
    }
    break ;
  case SFCM_WindowName:
    addProperty( SProperty::EDITBOX , "WindowName" , &m_WindowName , SProperty::String ) ;
    break ;
  case SFCM_Handle:
    {
      m_bInRreprogram = true ;
      m_hWindowAsText.Empty() ;
      addProperty( SProperty::EDITBOX , "hWindow" , &m_hWindowAsText , SProperty::String ) ;
      m_Hook = new CMouseHook( WindowSelectCallBack , this ) ;
    }
    break ;
  }
}

void ScreenFrag::ConnectorsRegistration()
{
  addInputConnector( transparent , "Sync" , fn_capture_trigger , this ); ;
  addOutputConnector( transparent , "OutImages" );
};

CDataFrame* ScreenFrag::GetScreeny( ) // 
{
  if (m_bInRreprogram)
    return NULL ;

  CRect CaptureArea ;
  switch (m_CaptureMode)
  {
    case SFCM_Idle: return NULL ;
    case SFCM_Absolute: 
      CaptureArea = m_CaptureArea ; 
      // Correct Capture Rectangle (original holds top-left and width-height)
      CaptureArea.right += CaptureArea.left ;
      CaptureArea.bottom += CaptureArea.top ;
      break ;
    case SFCM_Display: 
      CaptureArea = CSystemMonitorsEnumerator::GetMonitorEnums()
        ->m_monitorArray[ m_iDisplay - 1 ].rcWork ;
      break ;
    case SFCM_WindowName:
      if (!m_WindowName.IsEmpty())
      {
        HWND hWnd = FindWindow( NULL , ( LPCTSTR ) m_WindowName ) ;
        if (hWnd && IsWindow( hWnd ))
          ::GetWindowRect( hWnd , &CaptureArea ) ;
        else
          return NULL ;
      }
      else
        return NULL ;
      break ;
    case SFCM_Handle:
      if (m_hWindow && IsWindow( m_hWindow ))
        ::GetWindowRect( m_hWindow , &CaptureArea ) ;
      else
        return NULL ;
      break ;
  }

  // create the buffer for the screenshot
  DWORD iImageSize = CaptureArea.Width() * CaptureArea.Height() * 3 ;
  BITMAPINFO bmiCapture = {
    sizeof( BITMAPINFOHEADER ) , CaptureArea.Width() , CaptureArea.Height() ,
    1 , 24 , BI_RGB , iImageSize , 0 , 0 , 0 , 0 ,
  };

  HWND hMyWnd = NULL ;                // get full desktop DC
  HDC hdcCapture;
  LPBYTE lpCapture;
  int iRes = 0 ;
  CVideoFrame * pOutFrame = NULL ;
  HDC hDC = GetDC( hMyWnd );
 
  BITMAP BMIH ;
  memset( &BMIH , 0 , sizeof( BITMAP ) );

  HGDIOBJ hBitmap = GetCurrentObject( hDC , OBJ_BITMAP );
  GetObject( hBitmap , sizeof( BITMAP ) , &BMIH );

  int iWidth = BMIH.bmWidth ;
  int iHeight = BMIH.bmHeight ;
//   nBPP = GetDeviceCaps( hDC , BITSPIXEL );
  hdcCapture = CreateCompatibleDC( hDC );

  // create a container and take the screenshot
  HBITMAP hbmCapture = CreateDIBSection( hDC , &bmiCapture ,
    DIB_RGB_COLORS , (LPVOID *) &lpCapture , NULL , 0 );

  // failed to take it
  if ( hbmCapture )
  {
    int nCapture = SaveDC( hdcCapture );
    SelectObject( hdcCapture , hbmCapture );
    BitBlt( hdcCapture , 0 , 0 , CaptureArea.Width() , CaptureArea.Height() , 
      hDC , CaptureArea.left , CaptureArea.top , SRCCOPY );
    RestoreDC( hdcCapture , nCapture );

    pOutFrame = CVideoFrame::Create() ;
    
    pOutFrame->lpBMIH = (BITMAPINFOHEADER*)malloc( sizeof( BITMAPINFOHEADER ) + iImageSize ) ;
    memcpy( pOutFrame->lpBMIH , &bmiCapture , sizeof( BITMAPINFOHEADER ) ) ;
    pOutFrame->lpBMIH->biSizeImage = iImageSize ;
    pOutFrame->lpBMIH->biHeight = CaptureArea.Height() ;

    LPBYTE lpImageBits = (LPBYTE) (pOutFrame->lpBMIH + 1) ;

    GetDIBits( hdcCapture , hbmCapture , 0 , CaptureArea.Height() , lpImageBits , &bmiCapture , DIB_RGB_COLORS ) ;
    LPBITMAPINFOHEADER pConverted = NULL ;

    switch ( m_OutputFormat )
    {
    case SF_RGB:     break ;
    case SF_Y800: pConverted = rgb24_to_y8( pOutFrame->lpBMIH ) ; break ;
    case SF_YUV9: pConverted = rgb24yuv9( pOutFrame->lpBMIH ) ; break ;
    case SF_YUV12: pConverted = rgb24yuv12( pOutFrame->lpBMIH ) ; break ;
    }

    if ( pConverted )
    {
      free( pOutFrame->lpBMIH ) ;
      pOutFrame->lpBMIH = pConverted ;
    }

    m_dLastFrameTime_ms = GetHRTickCount() ;
    DeleteObject( hbmCapture );
  }
  else
    SENDERR( "failed to take the screenshot. err: %s\n" , (LPCTSTR)FxLastErr2Mes());

  // copy the screenshot buffer
  DeleteDC( hdcCapture );
  ReleaseDC( hMyWnd , hDC );

//   GdiplusShutdown( gdiplusToken );
  return pOutFrame ;

}

//CDataFrame * ScreenFrag::DoProcessing( const CDataFrame * pDataFrame )
CDataFrame * ScreenFrag::GetNextFrame( double * dStartTime )
{
  if ( m_GadgetName.IsEmpty() )
    GetGadgetName( m_GadgetName );

  switch ( m_Timing )
  {
    case SFT_ByTrigger:
      if ( m_iNTrigger <= 0 )
        Sleep( 1 ) ;
      else
      {
        m_iNTrigger-- ;
        return GetScreeny() ;
      }
      break ;
    case SFT_ByTimer:
      if ( GetHRTickCount() - m_dLastFrameTime_ms < m_OutPeriod_ms - 11. )
        Sleep( 1 ) ;
      else
        return GetScreeny() ;
      break ;
    default:
      ASSERT( 0 ) ;
      break ;
  }
  return NULL ;
}


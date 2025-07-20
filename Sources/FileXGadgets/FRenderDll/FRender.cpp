// FRender.h : Implementation of the FRender class


#include "StdAfx.h"
#include "Resource.h"
#include "FRender.h"
#include <imageproc\imagebits.h>
#include <gadgets\TextFrame.h>
#include <Gadgets\RectFrame.h>
#include <helpers/propertykitEx.h>
#include <helpers\FramesHelper.h>
#include <fxfc\CSystemMonitorsEnumerator.h>
#include <math\hbmath.h>

#ifdef THIS_MODULENAME
#undef THIS_MODULENAME 
#endif
#define THIS_MODULENAME _T("FRender")

static const UINT VM_TVDB400_SET_GADGET_PROPERTIES = ::RegisterWindowMessage( _T( "Tvdb400_SetGadgetProperties" ) );

static int g_iDebugModeForFrender = 0 ;
// !!!!!!!!!!!!!!!!!!!    Removed work with IP cameras and Spy window
//
//HANDLE FRender::m_CallBackLock = NULL ;
//FXArray<FRender*> FRender::m_AllRenderPtrs ;
//WNDPROC FRender::m_MainWindowDefFunc = NULL ;
//DWORD   FRender::m_dwMessageId = 0 ;
//DWORD   FRender::m_dwProcessId = 0 ;
//HWND    FRender::m_hTopWnd = NULL ;
//bool		FRender::m_IPIndexes[ MAX_IP_CAMERAS ] = {FALSE};
//LRESULT CALLBACK FRender::FRenderWindowProc(
//  HWND hwnd , UINT uMsg , WPARAM wParam , LPARAM lParam )
//{
//  if ( uMsg == m_dwMessageId )
//  {
//    FRender * pFRender = (FRender*) lParam ;
////     FXString GadgetName ;
////     pFRender->GetGadgetName( GadgetName ) ;
////     GadgetName += "_WindowProc" ;
////     FXAutolock al( m_CallBackLock , (LPCTSTR)GadgetName ) ;
//    DWORD dwRes = WaitForSingleObject( m_CallBackLock , 1000 ) ;
//    ASSERT( dwRes == WAIT_OBJECT_0 ) ;
//
//   // looking for matching render
//   // If it's not existing, render was deleted between post message and callback
//    for ( int i = 0 ; i < m_AllRenderPtrs.GetCount() ; i++ )
//    {
//      if ( pFRender == m_AllRenderPtrs[ i ] )
//      {  // render exists
//        FXPropertyKit Out ;
//        Out.WriteInt( _T( "hTargetWindow" ) , (int) wParam ) ;
//        bool Invalidate ;
//        pFRender->ScanProperties( Out , Invalidate ) ;
//        ReleaseMutex( m_CallBackLock ) ;
//        return 1 ;
//      }
//    }
//    ReleaseMutex( m_CallBackLock ) ;
//  }
//  else if ( m_MainWindowDefFunc != NULL )
//    return m_MainWindowDefFunc( hwnd , uMsg , wParam , lParam ) ;
//
//  return 0 ;
//};
//

TCHAR * ScalesAsNames[]=
{
  "Fit_window" , "x1" , "x2" , "x4" , "x8" , "x16" 
} ;

FXString& GetScalesForCombo()
{
  static FXString ForCombo ;
  ForCombo = ScalesAsNames[ 0 ] ;
  FXString Add ;
  int i = 1 ;
  for ( ; i < ARRSZ(ScalesAsNames) ; i++ )
  {
    Add.Format( "(%d),%s" , i - 1 , ScalesAsNames[ i ] ) ;
    ForCombo += Add ;
  }
  Add.Format( "(%d)" , i - 1 ) ;
  return ForCombo ;
}

int GetScaleNumber( LPCTSTR pScaleAsText )
{
  for ( int i = 0 ; i < ARRSZ( ScalesAsNames ) ; i++ )
  {
    if ( _tcscmp( pScaleAsText , ScalesAsNames[ i ] ) == 0 )
      return i ;
  }
  return -1 ;
}
void FRender_DibEvent( int Event , void *Data , void *pParam , CDIBViewBase* wParam )
{
  FRender* vrg = (FRender*) pParam;
  vrg->DibEvent( Event , Data );
}

void FRender_SpyEvent( int Event , WPARAM wParam , void * pParam )
{
  FRender * pHost = (FRender*) pParam ;
  pHost->OnSpyMessage( Event , wParam ) ;
}

BOOL CALLBACK FRender::EnumWindowsProc( HWND hwnd , LPARAM lParam )
{
  if ( hwnd  &&  lParam )
  {
    DWORD dwProcessId = 0 ;
    DWORD dwWndThreadId = GetWindowThreadProcessId( hwnd , &dwProcessId ) ;
    FRender * pFRender = (FRender*) lParam ;
    // !!!!!!!!!!!!!!!!!!!    Removed work with IP cameras and Spy window
    //
    //if ( pFRender->m_dwProcessId == dwProcessId )
    //{
    //  pFRender->m_hTopWnd = hwnd ;
    //  return FALSE ;
    //}
    return TRUE ;
  }
  return FALSE ;
};


IMPLEMENT_RUNTIME_GADGET_EX( FRender , CRenderGadget , "Video.renderers" , TVDB400_PLUGIN_NAME );

FRender::FRender( void ) :
  m_wndOutput( NULL ) ,
  m_Scale( DEFAULT_SCALE ) ,
  m_Monochrome( DEFAULT_MONOCHROME ) ,
  m_RectSelection( true ) ,
  m_bViewRGB( false ) ,
  m_PointOfInterest( -1 , -1 ) ,
  m_bSomeSelected( false ) ,
  m_bCntrlWasPressed( false ) ,
  m_bLButtonWasPressed( false ) ,
  m_pOwnWnd( NULL ) ,
  m_hExternalWnd( NULL ) ,
  m_pAttachedWnd( NULL ) ,
  m_iNAttachments( 0 ) ,
  //m_pSelectorWnd( NULL ) ,
  m_iRenderId( 0 ) ,
  m_bAddedToList( false ) ,
  m_nIndex( -1 ) ,
  m_nPlaydecHandle( -1 ) ,
  m_bWasCreatedbyMe( false ) ,
  m_ViewPos( 0 , 0 ) ,
  m_bShouldBeAddedToList( false ) ,
  m_ObjectSelection( ObjSel_Enabled ) ,
  m_bShowLabel( SL_SHOW_MAIN ) ,
  m_dScale( 1.0 ) ,
  m_cScale( 1. , 0. ) ,
  m_bcScaleCongugate( false ) ,
  m_sUnits( "um" )
{
  m_pInput = new CInputConnector( transparent );
  m_pOutput = new COutputConnector( text );
  m_pImageOutput = new COutputConnector( transparent );
  m_pControl = new CDuplexConnector( this , text , text ) ;
  //if ( !m_CallBackLock )
  //  m_CallBackLock = CreateMutex( NULL , FALSE , NULL ) ;
  LPCTSTR lpszDIBVClass = AfxRegisterWndClass(
    CS_HREDRAW | CS_VREDRAW | CS_PARENTDC ,
    LoadCursor( NULL , IDC_CROSS ) ,
    ( HBRUSH ) ::GetStockObject( DKGRAY_BRUSH ) );

  m_ScreenWnd.CreateEx( NULL , lpszDIBVClass , "Full SCreen View" ,
    /*WS_CAPTION | */WS_VISIBLE | WS_OVERLAPPED , CRect( 100 , 100 , 600 , 600 ),
    NULL , 0 ) ;
  m_ScreenWnd.ShowWindow( SW_HIDE ) ;

  Resume();
}

void FRender::ShutDown()
{
  Detach() ;

  CRenderGadget::ShutDown();
  //DWORD dwRes = WaitForSingleObject( m_CallBackLock , 1000 ) ;
  //ASSERT( dwRes == WAIT_OBJECT_0 || !m_CallBackLock ) ;
  //if ( m_bShouldBeAddedToList && m_dwProcessId )
  //{
  //  for ( int i = 0 ; i < m_AllRenderPtrs.GetCount() ; i++ )
  //  {
  //    if ( m_AllRenderPtrs[ i ] == this )
  //    {
  //      m_AllRenderPtrs.RemoveAt( i ) ;
  //      m_bAddedToList = false ;
  //      if ( m_AllRenderPtrs.GetCount() == 0 )
  //      {
  //        SetWindowLongPtr( m_hTopWnd , GWL_WNDPROC , (LONG_PTR) m_MainWindowDefFunc );
  //        m_dwProcessId = 0 ;
  //        m_dwMessageId = 0 ;
  //        m_hTopWnd = NULL ;
  //        ReleaseMutex( m_CallBackLock ) ;
  //        CloseHandle( m_CallBackLock ) ;
  //        m_CallBackLock = NULL ;
  //      }
  //      else
  //        ReleaseMutex( m_CallBackLock ) ;
  //    }
  //  }
  //}
  FXString GadgetName ;
  GetGadgetName( GadgetName ) ;
  GadgetName += "_ShutDown" ;

  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
  delete m_pImageOutput;
  m_pImageOutput = NULL;
  delete m_pControl ;
  m_pControl = NULL ;
  //if ( m_pSelectorWnd )
  //{
  //  m_pSelectorWnd->CloseWindow() ;
  //  m_pSelectorWnd->DestroyWindow() ;
  //  delete m_pSelectorWnd ;
  //  m_pSelectorWnd = NULL ;
  //}
  if ( m_wndOutput )
    m_wndOutput->DestroyWindow();
  delete m_wndOutput;
  m_wndOutput = NULL;
}

void FRender::Attach( CWnd* pWnd )
{
  if ( !pWnd )
    Detach();
  HWND hOwnWnd = (m_pOwnWnd) ?
    (::IsWindow( m_pOwnWnd->GetSafeHwnd() ) ? m_pOwnWnd->GetSafeHwnd() : (HWND) 0)
    : (HWND) (-1) ;
  if ( !pWnd || !::IsWindow( pWnd->GetSafeHwnd() ) )
  {
    SEND_GADGET_ERR( "No Window for Attachment. hOwn=0x%08X " , hOwnWnd ) ;
    return ;
  }
  BOOL bLocked = m_Lock.LockAndProcMsgs( INFINITE , "FRender::Attach" ) ;
  VERIFY( bLocked );
  if ( m_pAttachedWnd && (m_pAttachedWnd != m_pOwnWnd) 
    && (m_pAttachedWnd != &m_ScreenWnd) )
  {
    HWND hPrev = m_pAttachedWnd->GetSafeHwnd() ;
    if ( IsWindow( hPrev ) )
    {
      HWND hOldWnd = m_pAttachedWnd->Detach() ;
      m_pAttachedWnd->DestroyWindow() ;
    }
    m_pAttachedWnd = NULL ;
  }
  HWND hWnd = pWnd->GetSafeHwnd() ;
  if (!m_wndOutput)
  {
    m_wndOutput = new CDIBFRender(m_Monitor);
    GetGadgetName(m_GadgetInfo);
    FXString GadgetName = m_GadgetInfo + _T("_View");
    m_wndOutput->Create(pWnd, 0, 0, (LPCTSTR)GadgetName);
    m_wndOutput->SetScale((m_Scale == 0) ? -1 : m_Scale);
    SetScrollPos(m_ViewPos);
    m_wndOutput->SetMonochrome(m_Monochrome);
    m_wndOutput->SetShowLabel(m_bShowLabel);
    m_wndOutput->InitSelBlock(m_LengthViewMode, m_RectSelection);
    m_wndOutput->m_LengthViewMode = m_LengthViewMode ;
    m_wndOutput->SetCallback(::FRender_DibEvent, this);
    m_wndOutput->SetImagesDir( m_SaveImagesDir ) ;
    m_wndOutput->m_sUnits = m_sUnits;
    m_wndOutput->m_dLastLineLength = 0.;
    m_wndOutput->m_cScale = m_cScale ;
    m_wndOutput->m_bConjugateScaleConvert = m_bcScaleCongugate ;
  }
  else /*if ( hOwnWnd != hWnd )*/
  {
    long WinStyle = GetWindowLong( m_wndOutput->m_hWnd , GWL_STYLE ) ;
    long WinStyleEx = GetWindowLong( m_wndOutput->m_hWnd , GWL_EXSTYLE ) ;

    SetParent(m_wndOutput->m_hWnd, hWnd);
    CRect cr;
    pWnd->GetClientRect(&cr);
    DWORD dwSize = (cr.Height() << 16) | (cr.Width() & 0xffff);
    PostMessage(m_wndOutput->m_hWnd, WM_SIZE , 0 , dwSize );
  }
  //m_hWndForPOE = m_wndOutput->m_hWnd;
  //m_nIndex = GetNextAvIndex();
  //m_wndOutput->m_nIndex = m_nIndex;
  //SetHandle(m_wndOutput->m_hWnd);
  m_iNAttachments++;
  if ( m_pOwnWnd == NULL )
    m_pOwnWnd = pWnd ;
  m_pAttachedWnd = pWnd ;

  m_Lock.Unlock() ;

  TRACE( "\nAttached to hWnd 0x%08X, hOutWnd=0X%08X" , 
    hWnd , m_wndOutput->m_hWnd ) ;
  SEND_GADGET_INFO( "Attached to hWnd 0x%08X, hOutWnd=0X%08X", 
    hWnd, m_wndOutput->m_hWnd) ;
}

int	 FRender::GetNextAvIndex( int index , int playhandle )
{
  //int sz = m_IPCamShared.GetBoxSize();
  //m_IPCamBusInfo.m_ID = -1;
  //m_IPCamBusInfo.m_iIndex = playhandle;
  int nextFree = -1;
  //m_IPCamShared.SetToFirstFree( nextFree , (BYTE*) &m_IPCamBusInfo , sz );
  //m_IPCamBusInfo.m_ID = nextFree;
  //m_IPCamBusInfo.m_iIndex = playhandle;
  //m_IPCamShared.SetBoxContent( nextFree , (BYTE*) &m_IPCamBusInfo , sz );
  return nextFree;
  /*
  //int npos = atoi(fxLabel.MakeLower().Trim("cam"));
  //if(npos >= 0 && npos < MAX_IP_CAMERAS)
  //	return npos;
  for (int i = 0; i < MAX_IP_CAMERAS; i++)
  {
    if(m_IPIndexes[i] == false)
    {
      m_IPIndexes[i] = true;
      return i;
    }
  }
  return -1;
  */
}
/*
void FRender::OpenStream()
{
   BYTE byFileHeadBuf;
  if (H264_PLAY_OpenStream(m_nIndex, &byFileHeadBuf, 1, SOURCE_BUF_MIN*50))
  {
    H264_PLAY_SetStreamOpenMode(m_nIndex, STREAME_REALTIME);
  }
}
*/
void FRender::SetHandle( HWND hWnd )
{
  // !!!!!!!!!!!!!!!!!!!    Removed work with IP cameras
  //
  // BYTE byFileHeadBuf;
  //if ( H264_PLAY_OpenStream( m_nIndex , &byFileHeadBuf , 1 , SOURCE_BUF_MIN * 50 ) )
  //{
  //  H264_PLAY_SetStreamOpenMode( m_nIndex , STREAME_REALTIME );

  //  //RECT myRect;	
  //  //GetClientRect(m_wndOutput->m_hWnd,&myRect);
  //  //myRect.top = 55;
  //  //myRect.left = 0;
  //  //myRect.right = 1280;
  //  //myRect.bottom = 700;
  //  //bool ret=H264_PLAY_SetDisplayRegion(m_nIndex,0,&myRect,NULL,TRUE);
  //  if ( H264_PLAY_Play( m_nIndex , hWnd ) )
  //  {
  //    m_nPlaydecHandle = m_nIndex;
  //  }
  //  m_nPlaydecHandle = m_nIndex;
  //}
}
void FRender::Detach()
{
  BOOL bLocked = m_Lock.LockAndProcMsgs( INFINITE , "FRender::Detach" ) ;
  VERIFY( bLocked );
  HWND hWndBefore = NULL ;
  bool bExiting = false ;
  if ( m_wndOutput )
  {
    hWndBefore = m_wndOutput->m_hWnd ;
    if ( ::IsWindow( m_wndOutput->GetSafeHwnd() ) )
    {
      m_wndOutput->SetCallback( NULL , NULL );
      m_wndOutput->DestroyWindow();
      bExiting = true ;
    }
    delete m_wndOutput;
    m_wndOutput = NULL;
  }
  HWND hExtWnd = m_hExternalWnd ;
  if ( m_hExternalWnd )
  {
    if ( m_pAttachedWnd ) // if not correct window was sent or deleted
    {
      m_pAttachedWnd->Detach();
      delete m_pAttachedWnd;
      m_pAttachedWnd = NULL;
    }
    m_hExternalWnd = NULL ;
  }
  // !!!!!!!!!!!!!!!!!!!    Removed work with IP cameras
  //
  //if ( m_nPlaydecHandle >= 0 /*&& m_bWasCreatedbyMe*/ )
  //{
  //  H264_PLAY_CloseStream( m_nPlaydecHandle );
  //  H264_PLAY_Stop( m_nPlaydecHandle );
  //  m_nPlaydecHandle = -1;
  //  if ( m_nIndex >= 0 )
  //  {
  //    m_IPIndexes[ m_nIndex ] = false;
  //    m_IPCamShared.BoxClear( m_nIndex );
  //    m_nIndex = -1;
  //  }
  //  //m_bWasCreatedbyMe = false;
  //}
  TRACE( "Detach %s window 0x%08X. Ext Wnd was 0x%08X" , 
    bExiting ? "real" : "Unused" , hWndBefore , hExtWnd ) ;

  m_Lock.Unlock() ;
}

void FRender::Render( const CDataFrame* pDataFrame )
{
  if ( !pDataFrame->IsContainer() )
  {
    const CTextFrame * pCommand = pDataFrame->GetTextFrame();

    if ( pCommand )
    {
      LPCTSTR pLabel = pCommand->GetLabel() ;
      if ( _tcscmp( pLabel , _T( "Scale&Units" ) ) == 0 )
      {
        FXString ScaleAsString = pCommand->GetString() ;
        if ( !ScaleAsString.IsEmpty() )
          SetScaleAndUnits( ScaleAsString ) ;
        return ;
      }
      else if ( _tcscmp( pLabel , _T( "SetWndHandle" ) ) == NULL )
      {
        CWnd * pMainWnd = AfxGetApp()->GetMainWnd() ;
        FXString GadgetName ;
        if ( pMainWnd && GetGadgetName( GadgetName ) )
        {
          FXSIZE iPos = GadgetName.ReverseFind( _T( '.' ) ) ;
          if ( iPos >= 0 )
            GadgetName = GadgetName.Mid( iPos + 1 ) ;
          FXString NewHandleAsString = pCommand->GetString() ;
          FXString NotificationToMainWnd ;
          NotificationToMainWnd.Format( "Gadget=%s;Properties=(hTargetWindow=%s;);" ,
            (LPCTSTR) GadgetName , (LPCTSTR)NewHandleAsString ) ;
          int iAllocLen = (int)NotificationToMainWnd.GetLength() + 5 ;
          TCHAR * pMsg = new TCHAR[ iAllocLen ] ;
          _tcscpy_s( pMsg , iAllocLen , (LPCTSTR) NotificationToMainWnd ) ;
          pMainWnd->PostMessage( VM_TVDB400_SET_GADGET_PROPERTIES , 0 , (LPARAM) pMsg ) ;
        }
        return ;
      }

    }
  }
  BOOL bLocked = m_Lock.LockAndProcMsgs( INFINITE , "FRender::Render" ) ;
  VERIFY( bLocked );
  if ( bLocked && (m_wndOutput) 
    && (::IsWindow( m_wndOutput->GetSafeHwnd() )) )
  {
    // !!!!!!!!!!!!!!!!!!!    Removed work with IP cameras
    //
    //if ( Tvdb400_IsEOS( pDataFrame ) )
    //{
    //  m_nIndex = -1;
    //}
    //else if ( m_nIndex == -1 && !Tvdb400_IsEOS( pDataFrame ) )
    //{
    //  const CVideoFrame * pvf = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
    //  if ( pvf )
    //  {
    //    int inIndex = pvf->lpBMIH->biClrUsed ;
    //    int iPlayhandle = pvf->lpBMIH->biClrImportant;
    //
    //    m_nIndex = GetNextAvIndex( inIndex , iPlayhandle );
    //    m_wndOutput->m_nIndex = m_nIndex;
    //    SetHandle( m_wndOutput->m_hWnd );
    //
    //    int sz = m_IPCamShared.GetBoxSize();
    //    int handle = atoi( pDataFrame->GetLabel() );
    //    m_IPCamBusInfo.m_ID = m_nIndex;
    //    m_IPCamBusInfo.m_iIndex = handle;
    //  }
    //  else
    //  {
    //    m_Lock.Unlock() ;
    //    return ;
    //  }
    //
    //}
    //
    //if ( m_bShouldBeAddedToList && !m_bAddedToList )
    //{
    //  DWORD dwRes = WaitForSingleObject( m_CallBackLock , 1000 ) ;
    //  //ASSERT( dwRes == WAIT_OBJECT_0 ) ;
    //  if ( !m_dwProcessId )
    //  {
    //    m_dwMessageId = RegisterWindowMessage( _T( "FRenderMsg" ) ) ;
    //    m_dwProcessId = GetCurrentProcessId() ;
    //    EnumWindows( EnumWindowsProc , (LPARAM) this ) ;
    //    if ( m_hTopWnd )
    //    {
    //      m_MainWindowDefFunc = (WNDPROC) GetWindowLongPtr( m_hTopWnd , GWL_WNDPROC );
    //      SetWindowLongPtr( m_hTopWnd , GWL_WNDPROC , (LONG_PTR) FRenderWindowProc );
    //      m_AllRenderPtrs.Add( this ) ;
    //    }
    //  }
    //  else if ( m_hTopWnd )
    //    m_AllRenderPtrs.Add( this ) ;
    //  m_bAddedToList = true ;
    //  ReleaseMutex( m_CallBackLock ) ;
    //}
    int x , y;
    if ( (pDataFrame)
      && (pDataFrame->Attributes()->GetInt( "x" , x ))
      && (pDataFrame->Attributes()->GetInt( "y" , y )) )
    {
      m_wndOutput->ShiftPos( x , y );
    }
    else
      m_wndOutput->ShiftPos( -1 , -1 );

    if ( !m_bInitialized )
    {
      m_bInitialized = true ;
      m_wndOutput->m_dScaleTenthPerUnit = m_dScaleTenthPerUnit ;
    }
    m_wndOutput->Render( pDataFrame );
  }
  m_Lock.Unlock() ;
}

int FRender::DibEvent( int Event , void *Data )
{
  if ( ( Event & DIBVE_OUT_VFRAME ) && Data )
  {
    CDataFrame * pOutFrame = ( CDataFrame* ) Data ;
    PutFrame( m_pImageOutput , pOutFrame , 100 ) ;
    return 0 ;
  }
  if ( (Event & DIBVE_OUT_FRAME) && Data )
  {
    CDataFrame * pOutFrame = (CDataFrame*) Data ;
    PutFrame( m_pOutput , pOutFrame , 100 ) ;
    return 0 ;
  }

  pTVFrame ptv = m_wndOutput->GetFramePntr();
  if ( !ptv || !ptv->lpBMIH )
    return 0 ;
  FXString Out ;
  int bKbLCntrlPressed = (GetAsyncKeyState( VK_LCONTROL ) & 0x8000) != 0 ;
  int bKbRCntrlPressed = (GetAsyncKeyState( VK_RCONTROL ) & 0x8000) != 0 ;
  int bKbCntrlPressed = bKbLCntrlPressed || bKbRCntrlPressed ;
  int bKbLShiftPressed = (GetAsyncKeyState( VK_LSHIFT ) & 0x8000) != 0 ;
  int bKbRShiftPressed = (GetAsyncKeyState( VK_RSHIFT ) & 0x8000) != 0 ;
  int bKbShiftPressed = bKbLShiftPressed || bKbRShiftPressed ;
  int bMousLeftPressed = (GetAsyncKeyState( VK_LBUTTON ) & 0x8000) != 0 ;
  int BitMask = bKbCntrlPressed + (bKbShiftPressed << 1)
    + (bKbLCntrlPressed << 2) + (bKbRCntrlPressed << 3)
    + (bKbLShiftPressed << 4) + (bKbRShiftPressed << 5) ;

  CPoint pnt = *(POINT*) Data;
  
  SelectionByCursor iStyle = ( SelectionByCursor ) 
    (( m_wndOutput ) ? m_wndOutput->GetSelStyle() : 0) ; // get mouse selection style

  GraphicsData& GD = m_wndOutput->m_GraphicsData ;

  int iType = ( m_wndOutput ) ? GD.m_iSelectedType : 0 ;
  if ( g_iDebugModeForFrender )
  {
  TRACE( "\nEvent=0x%08X Type=%d LB=%d CNTL=%d Selected=%d" ,
    Event , iType , bMousLeftPressed , bKbCntrlPressed , m_wndOutput->m_bSomeSelected ) ;
  }

  if ( Event & DIBVE_LBUTTONDBL )
  {
    if ( GD.m_iSelectedType == SELECTED_RECT )
    {
      FXString ObjectName( GD.m_LastSelectedRectName ) ;
      if ( ObjectName.Find( _T( "ROI:" ) ) == 0 )
      {
        FXString outS;
        outS.Format( "selected=dbl;x=%d;y=%d;Keys=%d;rect_name=%s;" ,
          pnt.x , pnt.y , BitMask , (LPCTSTR) ObjectName.Mid( 4 ) ) ;
        CTextFrame* pMouseOutFrame = CreateTextFrame( outS , (LPCTSTR) NULL ) ;
        PutFrame( m_pOutput , pMouseOutFrame , 100 ) ;
        GD.m_iSelectedIndex = -1 ;
      }
    }
  }
  else if ( Event == DIBVE_DO_MAXIMIZE )
  {
    if ( !m_bMaximized )
    {
      CPoint OnScreen ;
      GetCursorPos( &OnScreen ) ;
      const CSystemMonitorsEnumerator * pMonitorsEnum =
        CSystemMonitorsEnumerator::GetMonitorEnums() ;

      for ( size_t i = 0 ; i < pMonitorsEnum->m_rcMonitors.size() ; i++ )
      {
        if ( pMonitorsEnum->m_rcMonitors[ i ].PtInRect( OnScreen ) )
        {
          m_pNormalView = m_pAttachedWnd/*->GetParent()*/ ;
          m_hNormalWnd = m_pNormalView->GetSafeHwnd() ;
          CRect rc = pMonitorsEnum->m_rcMonitors[ i ] ;
          ::SetWindowPos( m_ScreenWnd.GetSafeHwnd() , ( HWND ) 0 , 
            rc.left , rc.top , rc.Width() , rc.Height() , SWP_SHOWWINDOW ) ;
          Attach( &m_ScreenWnd ) ;
          m_bMaximized = true ;
        }
      }
    }
    else
    {
      Attach( m_pNormalView ) ;
      m_ScreenWnd.ShowWindow( SW_HIDE ) ;
      m_bMaximized = false ;
      m_hNormalWnd = NULL ;
    }

  }
  else if ( m_ObjectSelection == ObjSel_Enabled )
  {
    if ( Event & DIBVE_MOUSEMOVEEVENT )
    {
      if ( m_wndOutput )
      {
        CPoint pnt = *(POINT*) Data;
        if ( bMousLeftPressed )
        {
          if ( bKbCntrlPressed && m_wndOutput->m_bSomeSelected )
          {
            FXAutolock al( m_Lock , "FRender::DibEvent MouseMove" ) ;
            switch ( GD.m_iSelectedType )
            {
              case SELECTED_POINT:
                break ;
              case SELECTED_RECT:
              {
                if ( 
//                   GD.m_iSelectedIndex >= 0 
//                   && GD.m_iSelectedIndex < GD.m_Rects.GetCount() 
                  /*&&*/ GD.m_iSelectedForAdjustmentsIndex >= 0 
                  /*&& GD.m_iSelectedIndex == GD.m_iSelectedForAdjustmentsIndex */)
                {
//                   FXRectangle& SelectedRect =
//                     GD.m_Rects.GetAt( GD.m_iSelectedIndex ) ;
                  FXRectangle& SelectedRect =
                    GD.m_Rects.GetAt( GD.m_iSelectedForAdjustmentsIndex ) ;
                  CPoint ScrPtOfInterest( m_wndOutput->m_PointOfInterest ) ;
                  CSize Offset = pnt - ScrPtOfInterest ;
                  if ( g_iDebugModeForFrender )
                  {
                  TRACE( "\nMovePt(%d,%d) Off(%d,%d) R(%d,%d,%d,%d)" ,
                    pnt.x , pnt.y , Offset.cx , Offset.cy ,
                    SelectedRect.left , SelectedRect.top , SelectedRect.right , SelectedRect.bottom ) ;
                  }
                  m_wndOutput->m_PointOfInterest = pnt ;
                  if ( Offset.cx != 0 || Offset.cy != 0 )
                  {
                    switch ( GD.m_iSelectedSide )
                    {
                      case CURSOR_ON_WHOLE_SELECTION:
                        SelectedRect.OffsetRect( Offset ) ;
                        break ;
                      case CURSOR_ON_LTCORNER:
                        SelectedRect.left += Offset.cx ;
                        SelectedRect.top += Offset.cy ;
                        break ;
                      case CURSOR_ON_TOP_BORDER:
                        SelectedRect.top += Offset.cy ;
                        break ;
                      case CURSOR_ON_TRCORNER:
                        SelectedRect.right += Offset.cx ;
                        SelectedRect.top += Offset.cy ;
                        break ;
                      case CURSOR_ON_RIGHT_BORDER:
                        SelectedRect.right += Offset.cx ;
                        break ;
                      case CURSOR_ON_RBCORNER:
                        SelectedRect.right += Offset.cx ;
                        SelectedRect.bottom += Offset.cy ;
                        break ;
                      case CURSOR_ON_BOTTOM_BORDER:
                        SelectedRect.bottom += Offset.cy ;
                        break ;
                      case CURSOR_ON_BLCORNER:
                        SelectedRect.left += Offset.cx ;
                        SelectedRect.bottom += Offset.cy ;
                        break ;
                      case CURSOR_ON_LEFT_BORDER:
                        SelectedRect.left += Offset.cx ;
                        break ;
                      default:
                        m_wndOutput->m_bSomeSelected = false ;
                        return 0 ;
                    }
                    SelectedRect.AdjustCursorAreas() ;
                    if ( m_wndOutput->m_bSomeSelected )
                      GD.m_LastSelectedArea = SelectedRect ;
//                     FXRectangle& SelectedRect =
//                       GD.m_Rects.GetAt( GD.m_iSelectedForAdjustmentsIndex ) ;

                    m_wndOutput->Invalidate( TRUE ) ;
                  }
                }
              }
              break ;
              default:
                m_wndOutput->m_bSomeSelected = false ;
                m_wndOutput->m_PointOfInterest.x = m_wndOutput->m_PointOfInterest.y = -1 ;
                break ;
            }
          }
          else
          {
            bool bTextFormed = FormOutputText( pnt , BitMask ) ;

            m_wndOutput->m_PointOfInterest = CPoint( -1 , -1 );
          }
        }
        else
        {
          m_wndOutput->m_PointOfInterest = pnt ;
          if ( !BitMask )
          {
//             FXString TipString , ErrString ;
// //             HWND hTT = m_wndOutput->GetToolTipHandle() ;
//             if ( FormOutputText( pnt , 0 , &TipString ) )
//             {
// 
// //               CPoint pt (pnt) ;
// //               ((CDIBViewBase*) m_wndOutput)->Pic2Scr( pt ) ;
// //               m_wndOutput->m_ToolTip.SetWindowPos( NULL , pt.x + 5 , pt.y - 5 , 0 , 0 ,
// //                 SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOZORDER ) ;
// //               int iRes = m_wndOutput->m_ToolTip.SendMessage( WM_SETTEXT , 0 , (LPARAM) (LPCTSTR) TipString ) ;
// //               if ( 1 )
// //               {
// //                 iRes++ ;
// //               }
//               //m_wndOutput->m_ToolTip.SetWindowText( TipString ) ;
//               
// //              m_wndOutput->ClientToScreen( &pt ) ;
// //               ::SetWindowPos( hTT , HWND_TOPMOST , pt.x , pt.y , 0 , 0 ,
// //                 SWP_NOSIZE );
// //               CRect cr ;
// //               ::GetClientRect( hTT , &cr ) ;
// //               HDC hDC = ::GetDC( hTT ) ;
// //               CDC dc ;
// //               dc.Attach( hDC ) ;
// //               dc.FillSolidRect( cr , RGB( 255 , 255 , 0 ) ) ;
// //               CGdiObject * pOldPen = dc.SelectStockObject( BLACK_PEN ) ;
// //               dc.TextOut( 2 , 2 , (LPCTSTR) TipString , TipString.GetLength() ) ;
// //               dc.SelectObject( pOldPen ) ;
// //               dc.Detach() ;
// //               ::ReleaseDC( hTT , hDC ) ;
// 
// //               TOOLINFO * pTInfo = m_wndOutput->GetToolTipInfo() ;
// //               HWND hOut = m_wndOutput->m_hWnd ;
// //               TOOLINFO TTInfo ;
// //               TCHAR Buf[ 256 ] ;
// //               TTInfo.cbSize = sizeof( TOOLINFO ) ;
// //               TTInfo.hwnd = hOut ;
// //               TTInfo.uId = (UINT_PTR) hOut ;
// //               TTInfo.lpszText = Buf ;
// //               TTInfo.hinst = GetModuleHandle( NULL ) ;
// //               DWORD dwRes = ::SendMessage( hTT , TTM_GETTOOLINFO , 0 , (LPARAM) &TTInfo );
// //               if ( !dwRes )
// //                 ErrString = FxLastErr2Mes( "Get Tool Info Error: " , "" ) ;
// // 
// //               TTInfo.lpszText = (LPSTR)TipString.GetBuffer() ;
// //               TTInfo.lpReserved = NULL ;
// //               
// //               ::SendMessage( hTT , TTM_SETTOOLINFO , 0 , (LPARAM) &TTInfo );
// // 
// //               dwRes = ::SendMessage( hTT , TTM_GETTOOLINFO , 0 , (LPARAM) &TTInfo );
// //               if ( !dwRes )
// //                 ErrString = FxLastErr2Mes( "Get Tool Info Error: " , "" ) ;
// //               // Position the tooltip. The coordinates are adjusted so that the tooltip does not overlap the mouse pointer.
// // 
// //               ClientToScreen( hOut , &pt );
// //               ::SendMessage( hTT , TTM_TRACKPOSITION , 0 , (LPARAM) MAKELONG( pt.x + 10 , pt.y - 20 ) );
// //               TipString.ReleaseBuffer() ;
// //               //SendMessage( hTT , TTM_POPUP , 0 , 0 ) ;
//               //               TOOLINFO Info ;
// //               memset( &Info , 0 , sizeof( Info ) ) ;
// //               Info.cbSize = sizeof( Info ) ;
// //               Info.uFlags = TTF_ABSOLUTE | TTF_TRACK ;
// //               Info.hwnd = m_wndOutput->m_hWnd ;
// //               Info.rect = CRect( m_wndOutput->m_LastMousePos , CSize( 70 , 25 ) ) ;
// //               Info.lpszText = (LPSTR)((LPCTSTR) TipString) ;
// //               m_wndOutput->GetToolTip()->SetToolInfo( &Info ) ;
// //               m_wndOutput->GetToolTip()->Activate( TRUE ) ;
//             }
          }
        }

        if ( !bKbCntrlPressed )
          m_bCntrlWasPressed = false ;
        m_wndOutput->CheckCursorForm( true , pnt ) ;
      }
      //     m_wndOutput->m_PointOfInterest=*(LPPOINT)Data;
      //     FXString outS; 
      //     outS.Format("selected=false;x=%d;y=%d;", m_wndOutput->m_PointOfInterest.x,m_wndOutput->m_PointOfInterest.y );
      //     CTextFrame* pMouseOutFrame = CTextFrame::Create();
      //     pMouseOutFrame->SetTime(GetGraphTime() * 1.e-3());
      //     pMouseOutFrame->ChangeId(NOSYNC_FRAME);
      //     pMouseOutFrame->SetString(outS);
      //     if (!m_pOutput->Put(pMouseOutFrame))
      //       pMouseOutFrame->RELEASE(pMouseOutFrame);
    }
    if ( Event & DIBVE_LBUTTONDOWN )
    {
      if ( bKbShiftPressed )
      {
        if ( GD.m_iSelectedIndex >= 0 )
        {
          if ( g_iDebugModeForFrender )
          {
          TRACE( "\nSelectedPt(%d,%d) Type(%d) Side(%d)" ,
            m_wndOutput->m_PointOfInterest.x , m_wndOutput->m_PointOfInterest.y ,
            GD.m_iSelectedType , GD.m_iSelectedSide ) ;
          }
          if ( ( GD.m_iSelectedType == SELECTED_RECT ) 
            && ( GD.m_iSelectedIndex < GD.m_Rects.GetCount() ) )
          {
            if ( GD.m_Rects[ GD.m_iSelectedIndex ].m_bSelectedForAdjustment )
            {
              GD.m_Rects[ GD.m_iSelectedIndex ].m_bSelectedForAdjustment = false ;
              GD.m_SelectedForAdjustmentName.Empty() ;
              GD.m_iSelectedForAdjustmentsIndex = -1 ;
            }
            else
            {
              for ( int i = 0 ; i < GD.m_Rects.GetCount() ; i++ )
                GD.m_Rects[ i ].m_bSelectedForAdjustment = false ;
              GD.m_Rects[ GD.m_iSelectedIndex ].m_bSelectedForAdjustment = true ;
              GD.m_SelectedForAdjustmentName =
                GD.m_Rects[ GD.m_iSelectedIndex ].m_ObjectName ;
              GD.m_iSelectedForAdjustmentsIndex = GD.m_iSelectedIndex ;
            }
          }
        }
      }
      if ( bKbCntrlPressed )
      {
        if ( !m_bCntrlWasPressed )
        {
          m_wndOutput->m_PointOfInterest = *(LPPOINT) Data ;
  //         m_wndOutput->Pic2Scr( m_wndOutput->m_PointOfInterest ) ;
          m_wndOutput->m_bSomeSelected = true ;
          m_bCntrlWasPressed = true ;
          if ( GD.m_iSelectedIndex >= 0 )
          {
            if ( g_iDebugModeForFrender )
            {
            TRACE( "\nSelectedPt(%d,%d) Type(%d) Side(%d)" ,
              m_wndOutput->m_PointOfInterest.x , m_wndOutput->m_PointOfInterest.y ,
              GD.m_iSelectedType ,
              GD.m_iSelectedSide ) ;
            }
            GD.m_RectOriginForModification =
              GD.m_Rects[ GD.m_iSelectedIndex ] ;
          }
        }
      }
      else
      //if ( m_wndOutput->GetSelStyle() == SEL_NOTHING )
      {
        if ( m_bCntrlWasPressed )
        {
          m_wndOutput->m_bSomeSelected = false ;
          m_bCntrlWasPressed = false ;
          GD.m_iSelectedIndex = -1 ;
        }
        if ( (GetAsyncKeyState( 'L' ) & 0x8000) )  // enable/disable line selection
        {
          m_LengthViewMode = (LengthViewMode)((((int) m_LengthViewMode) + 1) % ( ( int ) LVM_ViewBoth + 1)) ;
          m_wndOutput->m_LengthViewMode = m_LengthViewMode ;
        }
        else if ( (GetAsyncKeyState( 'R' ) & 0x8000) ) // enable/disable rect selection
        {
          m_wndOutput->SetLocalDrawRectEnabled( m_RectSelection = !m_RectSelection );
        }
        else
        {
          m_wndOutput->m_dLastLineLength = 0. ;
          bool bTextFormed = FormOutputText( m_wndOutput->m_PointOfInterest , BitMask ) ;
          m_wndOutput->m_PointOfInterest = CPoint( -1 , -1 );
          m_bLButtonWasPressed = true ;
        }

      }
    }
    else if ( Event & DIBVE_LINESELECTEVENT )
    {
      CRect rc = (m_wndOutput->m_rcLastSelectedLine) = *(CRect*) Data;
      //rc.NormalizeRect() ;
      int iWidth = abs( rc.Width()) ;
      int iHeight = abs( rc.Height()) ;
      CPoint Cent = rc.CenterPoint() ;
    
      if ( !iWidth && !iHeight )
      {
        FormOutputText( (CPoint&) m_wndOutput->m_PointOfInterest , BitMask ) ;
        m_wndOutput->m_dLastLineLength = 0. ;
      }
      else
      {
        if ( m_wndOutput->GetSelection()->LineEnable )
        {
          m_wndOutput->m_dLastLineLength = 
            sqrt( (double) (iWidth * iWidth + iHeight * iHeight) ) * m_dScale ;
          CPoint LineLengthTextPos = Cent ;
          if ( iWidth > iHeight * 2 )
            LineLengthTextPos += CSize( -iWidth / 4 , -20 - iHeight ) ;
          else
            LineLengthTextPos += CSize( 10 , 0 ) ;

//           CPoint LineLengthTextPos(
//             ((rc.top > rc.bottom) ? rc.right : rc.left ) - 20,
//             (((rc.top > rc.bottom) ? rc.bottom : rc.top ) - 12 ) ) ;
          m_wndOutput->m_LastLineLengthTextPos = LineLengthTextPos ;

          FXString outS;
          outS.Format( "Line=%d,%d,%d,%d;Keys=%d;L=%.2f%s;" , 
            rc.left , rc.top , rc.right , rc.bottom , BitMask , 
            m_wndOutput->m_dLastLineLength , (LPCTSTR)m_sUnits );
          CTextFrame* pMouseOutFrame = CreateTextFrame( outS , (LPCTSTR)NULL );
          PutFrame( m_pOutput , pMouseOutFrame , 100 ) ;
        }
      }
    }
    else if ( (Event & DIBVE_RECTSELECTEVENT) && m_wndOutput->GetSelection()->RectEnable )
    {
      CRect rc = *(RECT*) Data;
      rc.NormalizeRect() ;
      FXString outS;
      outS.Format( "Rect=%d,%d,%d,%d;Keys=%d;" , 
        rc.left , rc.top , rc.right , rc.bottom , BitMask );
      CTextFrame* pMouseOutFrame = CreateTextFrame( outS , (LPCTSTR)NULL );
      PutFrame( m_pOutput , pMouseOutFrame , 100 ) ;
      //m_wndOutput->ResetSelection() ;
    }
    else if ( (Event & DIBVE_LBUTTONUP)  /*&&  (iStyle == SEL_NOTHING)*/ )
    {
      if ( bKbCntrlPressed )
      {
        CPoint pnt = *(POINT*) Data;
        FXAutolock al( m_Lock , "FRender::DibEvent LButtonUp" ) ;
        if ( m_wndOutput->m_bSomeSelected && (m_wndOutput->m_PointOfInterest.x >= 0)
          && GD.m_iSelectedIndex >= 0 &&
          (GD.m_iSelectedIndex < GD.m_Rects.GetCount()) )
        {
          FXRectangle& SelectedRect =
            GD.m_Rects.GetAt( GD.m_iSelectedIndex ) ;
          FXString PureObjectName( SelectedRect.m_ObjectName ) ;
          int iColonPos = (int) PureObjectName.Find( _T( ':' ) ) ;
          if ( iColonPos >= 0 )
            PureObjectName.Delete( 0 , iColonPos + 1 ) ;

          bool bCorrected = GD.CorrectRect(
            SelectedRect , m_wndOutput->m_PointOfInterest , pnt , 
            ( SelectionByCursor ) SelectedRect.m_iSelectedSide ) ;
          CRect ForWrite( SelectedRect ) ;
          GD.m_Rects[ GD.m_iSelectedIndex ] = ForWrite ;
          ForWrite.left -= GD.m_RectOriginForModification.left ;
          ForWrite.top -= GD.m_RectOriginForModification.top ;
          // Replace right to width and bottom to height
          ForWrite.right = SelectedRect.Width() 
            - GD.m_RectOriginForModification.Width() ;
          ForWrite.bottom = SelectedRect.Height()
            - GD.m_RectOriginForModification.Height() ;
          FXPropertyKit SetROICommand ;
          WriteRect( SetROICommand , PureObjectName , ForWrite ) ;
          SetROICommand.WriteInt( "Id" , m_wndOutput->m_iTmpFrameId ) ;
          SetROICommand.Insert( 0 , _T( "setroidiff " ) ) ;
          TRACE( " %s" , (LPCTSTR) SetROICommand ) ;
          CTextFrame * pTF = CTextFrame::Create( SetROICommand ) ;
          pTF->SetLabel( SelectedRect.m_ObjectName ) ;
          pTF->SetTime( (GetGraphTime() * 1.e-3));
          pTF->ChangeId( NOSYNC_FRAME );
          if ( !m_pOutput->Put( pTF ) )
            pTF->RELEASE( pTF );
        }

        m_wndOutput->m_PointOfInterest.x = m_wndOutput->m_PointOfInterest.y = -1 ;
        //m_bCntrlWasPressed = false ;
       // GD.m_iSelectedIndex = -1 ;
      }
      else
      {
        m_bCntrlWasPressed = false ;
        CPoint pnt = *(POINT*) Data;
        FXString outS;
        //         outS.Format("LButtonUp=%d,%d;",pnt.x,pnt.y);
        outS.Format( "selected=false;x=%d;y=%d;Keys=%d;" , pnt.x , pnt.y , BitMask );
        CTextFrame* pMouseOutFrame = CreateTextFrame( outS , (LPCTSTR) NULL );
        PutFrame( m_pOutput , pMouseOutFrame , 100 ) ;
      }
      m_wndOutput->m_bSomeSelected = false ;
      GD.m_iSelectedIndex = -1 ;
      m_bLButtonWasPressed = false ;
    }
    else if ( (Event & DIBVE_RBUTTONUP) && (iStyle == SEL_NOTHING) )
    {
      CPoint pnt = *(POINT*) Data;
      FXString outS;
      outS.Format( "selected=false;x=%d;y=%d;Keys=%d;" , pnt.x , pnt.y , BitMask );
      CTextFrame* pMouseOutFrame = CTextFrame::Create();
      pMouseOutFrame->SetTime( (GetGraphTime() * 1.e-3));
      pMouseOutFrame->ChangeId( NOSYNC_FRAME );
      pMouseOutFrame->SetString( outS );
      if ( !m_pOutput->Put( pMouseOutFrame ) )
        pMouseOutFrame->RELEASE( pMouseOutFrame );
    }
    else if ( Event & DIBVE_RBUTTONDOWN )
    {
      m_wndOutput->m_dLastLineLength = 0. ;
    }
    else if ( Event & DIBVE_RBUTTONUP )
    {
      m_wndOutput->m_dLastLineLength = 0. ;
    }

  }  //   if ( m_ObjectSelection == ObjSel_Enabled )
  else
  {
    if ( Event & DIBVE_LBUTTONDOWN )
    {
      bool bTextFormed = FormOutputText( pnt , BitMask ) ;
    }
    else if ( (Event & DIBVE_LBUTTONUP) )
    {
      FXString outS;
      outS.Format( "selected=false;x=%d;y=%d;Keys=%d;" , pnt.x , pnt.y , BitMask );
      CTextFrame* pMouseOutFrame = CTextFrame::Create();
      pMouseOutFrame->SetTime( (GetGraphTime() * 1.e-3));
      pMouseOutFrame->ChangeId( NOSYNC_FRAME );
      pMouseOutFrame->SetString( outS );
      PutFrame( m_pOutput , pMouseOutFrame , 100 ) ;
      GD.m_iSelectedIndex = -1 ;
    }
    else if ( Event & DIBVE_MOUSEMOVEEVENT )
    {
      if ( m_wndOutput && bMousLeftPressed )
      {
        if ( bKbCntrlPressed )
        {
        }
        else
        {
          bool bTextFormed = FormOutputText( pnt , BitMask ) ;
          m_wndOutput->m_PointOfInterest = CPoint( -1 , -1 );
        }
      }
      else
      {

      }
    }
    else if ( Event & DIBVE_RBUTTONDOWN )
    {
      m_wndOutput->m_dLastLineLength = 0. ;
    }
    else if ( Event & DIBVE_RBUTTONUP )
    {
      m_wndOutput->m_dLastLineLength = 0. ;
    }
  }
  return 0 ;
}

bool FRender::PrintProperties( FXString& text )
{
  CRenderGadget::PrintProperties( text );
  FXPropertyKit pc;
  pc.WriteInt( "Monochrome" , m_Monochrome );
  if ( m_wndOutput )
  {
    double dScale = m_wndOutput->GetScale() ;
    if ( dScale < 0. )
      m_Scale = 0 ;
    else if ( dScale <= 1. )
      m_Scale = 1 ;
    else if ( dScale <= 2. )
      m_Scale = 2 ;
    else if ( dScale <= 4. )
      m_Scale = 3 ;
    else if ( dScale <= 8. )
      m_Scale = 4 ;
    else
      m_Scale = 5 ;
    CPoint ScrollPos( m_wndOutput->GetScrollPos( SB_HORZ ) ,
      m_wndOutput->GetScrollPos( SB_VERT ) ) ;
    m_ViewPos = ScrollPos ;
    WritePoint( pc , _T( "offset" ) , ScrollPos ) ;
  }
  //pc.WriteString( "Scale" , ScalesAsNames[ m_Scale ] );
  pc.WriteInt( "Scale" , m_Scale );

  pc.WriteInt( "LineSelection" , m_LengthViewMode );
  pc.WriteBool( "RectSelection" , m_RectSelection );
  if ( m_wndOutput )
    pc.WriteInt( "CutOverlap" , m_wndOutput->m_bCutOverlapMode ) ;
  pc.WriteBool( "ViewRGB" , m_bViewRGB ) ;
  FXString TargetAsString ;
  TargetAsString.Format( "0x%0X" , m_hExternalWnd ) ;
  if ( m_wndOutput && m_wndOutput->m_hWnd && !m_hExternalWnd )
  {
    CWnd * pWnd = m_wndOutput->GetParent();
    if ( pWnd )
    {
      HANDLE hWnd = pWnd->m_hWnd;
      FXString AddInfo;
      AddInfo.Format(" - 0x%0X", hWnd);
      AddInfo += (m_hExternalWnd == NULL) ? "  Own" : "  Ext" ;
      TargetAsString += AddInfo;
    }
  }
  pc.WriteString( _T( "hTargetWindow" ) , TargetAsString ) ;
  pc.WriteInt( _T( "Id" ) , m_iRenderId ) ;
  pc.WriteInt( _T( "ObjectSelection" ) , (int) m_ObjectSelection ) ;
  pc.WriteInt( _T( "ShowLabel" ) , m_bShowLabel ) ;
  FXString ScaleAsString ;
  ScaleAsString.Format( "%.6f,%s" , m_dScale , (LPCTSTR) m_sUnits ) ;
  if ( (m_cScale.real() != 1.) || (m_cScale.imag() != 0.) )
  {
    FXString cScaleAsString ;
    cScaleAsString.Format( ",%.7f,%.7f,%d" ,
      m_cScale.real() , m_cScale.imag() , m_bcScaleCongugate ? 1 : 0 ) ;
    ScaleAsString += cScaleAsString ;
  }
  pc.WriteString( "Scale&Units" , ScaleAsString ) ;
  pc.WriteString( "SaveImagesDir" , m_SaveImagesDir ) ;

  text += pc;
  return true;
}

bool FRender::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  GetGadgetName( m_GadgetInfo ) ;
  FXString tmpS;
  CRenderGadget::ScanProperties( text , Invalidate );
  FXPropertyKit pk( text );
  if ( pk.GetString( _T( "hTargetWindow" ) , tmpS ) ) 
  { 
    // // if handle exists, it should be the only property in input string
    FXSIZE iMaxPropLenAsText = ((FXSIZE) strlen( _T( "hTargetWindow" ) ) + tmpS.GetLength() + 5) ;
    if ( pk.GetLength() < iMaxPropLenAsText ) // if handle exists, check for absence of another items
    {
      HWND hTarget = (HWND) ConvToBinary( tmpS );
      if ( hTarget != m_hExternalWnd )
      {
        Detach();

        if ( hTarget == NULL )
        {
          if ( m_pOwnWnd && m_pOwnWnd->GetSafeHwnd() )
            Attach( m_pOwnWnd );
        }
        else
        {
          CWnd * pAttachedWnd = new CWnd;
          double dNow = GetHRTickCount();
          if ( pAttachedWnd )
          {
            if ( pAttachedWnd->FromHandlePermanent( hTarget ) != NULL )
              SENDERR( "Window h=0x%x is already attached, ATTACHMENT ABORTED" , hTarget );
            else
            {
              if ( pAttachedWnd->Attach( hTarget ) )
              {
                Attach( pAttachedWnd );
                m_hExternalWnd = hTarget;
              }
              else
                SENDERR( "Can't attach to external window h=0x%x, ATTACHMENT FAILED" , hTarget );
            }
          }
        }
      }
    }
  }
  FXString AsString ;
  pk.GetString( "Scale" , AsString ); // Zoom switching
  if ( isdigit( AsString[ 0 ] ) )
    m_Scale = atoi( AsString ) ;
  else 
  {
    int iScale = GetScaleNumber( AsString ) ;
    if ( iScale >= 0 )
      m_Scale = iScale ;
  }
//  pk.GetInt( "Scale" , m_Scale ); // Zoom switching
  pk.GetInt( "Monochrome" , m_Monochrome );
  if ( pk.GetInt( "LineSelection" , (int&)m_LengthViewMode ) )
  {
    if ( m_wndOutput )
      m_wndOutput->m_LengthViewMode = m_LengthViewMode ;
  }
  pk.GetBool( "RectSelection" , m_RectSelection ) ;
  if (pk.GetBool( "ViewRGB" , m_bViewRGB ) && m_wndOutput )
    m_wndOutput->m_bShowRGB = m_bViewRGB ;
  pk.GetInt( _T( "ObjectSelection" ) , (int&) m_ObjectSelection ) ;
  if ( m_wndOutput )
    pk.GetInt( "CutOverlap" , m_wndOutput->m_bCutOverlapMode ) ;
  pk.GetInt( _T( "Id" ) , m_iRenderId ) ;
  pk.GetInt( _T( "ShowLabel" ) , m_bShowLabel ) ;
  if ( GetPoint( pk , _T( "offset" ) , m_ViewPos ) )
    SetScrollPos( m_ViewPos ) ;
  if ( pk.GetString( "Scale&Units" , AsString ) )
  {
    SetScaleAndUnits( AsString.Trim() ) ;
    if ( m_wndOutput )
    {
      FXAutolock al( m_wndOutput->m_Lock , "FRender::ScanProperties" ) ;
      m_wndOutput->m_sUnits = m_sUnits ;
      m_wndOutput->m_dLastLineLength = 0. ;
    }
  }
  if ( pk.GetString( "SaveImagesDir" , m_SaveImagesDir ) )
  {
    if ( !m_SaveImagesDir.IsEmpty() )
    {
      TCHAR cLast = m_SaveImagesDir[ m_SaveImagesDir.GetLength() - 1 ] ;
      if ( cLast != _T( '\\' ) && cLast != _T( '/' ) )
        m_SaveImagesDir += _T( '/' ) ;
    }
  }
  if ( m_wndOutput )
  {
    double dScale = -1 ;
    switch ( m_Scale )
    {
      case 1: dScale = 1. ; break ;
      case 2: dScale = 2. ; break ;
      case 3: dScale = 4. ; break ;
      case 4: dScale = 8. ; break ;
      case 5: dScale = 16. ; break ;
    }
    m_wndOutput->SetScale( dScale );
    m_wndOutput->SetMonochrome( m_Monochrome );
    m_wndOutput->InitSelBlock( m_LengthViewMode , m_RectSelection );
    m_wndOutput->SetShowLabel( m_bShowLabel ) ;
    m_wndOutput->SetImagesDir( m_SaveImagesDir ) ;
  }
  return true;
}

bool FRender::ScanSettings( FXString& text )
{
  text = _T( "template(ComboBox(Scale(" + GetScalesForCombo() + "))"
    ",ComboBox(Monochrome(False(0),True(1),RAW_Mono(2),RAW_Color(3)))"
    ",ComboBox(LineSelection(False(0),View(1),View_Tenth(2),View_Both(3)))"
    ",ComboBox(RectSelection(False(false),True(true)))"
    ",ComboBox(CutOverlap(False(0),True(1)))"
    ",ComboBox(ViewRGB(False(false),True(true)))"
    ",Spin(Id,-1,1000000000)"
    ",EditBox(hTargetWindow)"
    ",ComboBox(ObjectSelection(Enabled(1),Disabled(0)))"
    ",ComboBox(ShowLabel(False(0),True(1)))"
    ",EditBox(Scale&Units)"
    ",EditBox(SaveImagesDir)"
    ")" );
  return true;
}

void FRender::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  if ( !pParamFrame )
    return ;
  CTextFrame* pTextFrame = pParamFrame->GetTextFrame( DEFAULT_LABEL );
  if ( pTextFrame )
  {
    LPCTSTR pLabel = pTextFrame->GetLabel();
    if ( _tcscmp( pLabel , _T( "SetCenter" ) ) == 0 )
    {
      FXPropKit2 Content = pTextFrame->GetString() ;
      CPoint NewCenter ;
      if ( Content.GetInt( "Xc" , NewCenter.x )
        && Content.GetInt( "Yc" , NewCenter.y ) 
        && m_wndOutput )
      {
        m_wndOutput->m_NewImageCenter = NewCenter ;
      }
    }

    FXParser Parser = (LPCTSTR) pTextFrame->GetString();
    FXString Prop , cmd ;
    FXString param;
    FXSIZE pos = 0;
    Parser.GetWord( pos , cmd );

    if ( cmd.CompareNoCase( "get" ) == 0 )
    {
      if ( Parser.GetWord( pos , Prop ) )
      {
        if ( Prop.CompareNoCase( _T( "hTargetWindow" ) ) )
        {
          Prop.Format( _T( "0x%X" ) , m_hExternalWnd ) ;
        }
        else
          Prop = _T( "Error" ) ;
      }
      else
        PrintProperties( Prop ) ;
    }
    else if ( (cmd.CompareNoCase( "set" ) == 0) )
    {
      if ( ( Parser.GetWord( pos , Prop )) && ( Parser.GetParamString( pos , param )) )
      {
/*        if ( Prop.Compare( _T( "hTargetWindow" ) ) == 0 )
        {
          int hWnd ;
          if ( ConvToBinary( param , hWnd ) )
          {
            if ( m_hTopWnd &&  m_dwMessageId )
            {
              PostMessage( m_hTopWnd , m_dwMessageId , (WPARAM) hWnd , (LPARAM) this ) ;
              Prop = _T( "OK" ) ;
            }
            else
              Prop = _T( "Error" ) ;
          }
        }
        else */
        if ( Prop.CompareNoCase( _T( "reset" ) ) == 0 )
        {
          if ( (m_wndOutput) && (::IsWindow( m_wndOutput->GetSafeHwnd() )) )
          {
            m_wndOutput->LoadFrame( NULL , /*NULL ,*/ true ) ;
            Prop = _T( "OK" ) ;
          }
        }
        else if ( Prop.CompareNoCase( _T( "outimage" ) ) == 0 ) // send formed image to second output
        {
          m_wndOutput->m_iSendOutImage = atoi( param ) ;
        }
      }
      else
      {
        FXPropertyKit pk = Parser.Mid( 4 ) ;
        bool bInvalidate = false ;
        if ( ScanProperties( pk , bInvalidate ) )
        {
          if ( bInvalidate )
          {
            CSetupObject * pSetup = GetSetupObject() ;
            if ( pSetup && pSetup->IsOn() )
              pSetup->Update() ;
          }
        }
        else
          Prop = _T( "Error" ) ;
      }
    }
    else
    {
      Prop = "List of available commands:\r\n"
        "list - return list of properties\r\n"
        "get hTargetWindow - get rendering window handle\r\n"
        "set hTargetWindow(<value>) - change rendering window handle (decimal or hex)\r\n"
        "set Reset - clear output window\n"
        "set OutImage(<# of images>) - send viewed image to second output as video frame"
        "String as property kit for parameters changing"
        ;

    }
    CTextFrame* retV = CTextFrame::Create( Prop );
    retV->ChangeId( NOSYNC_FRAME );
    PutFrame( m_pControl , retV , 100 ) ;
  }
  pParamFrame->Release() ;
}

int FRender::OnSpyMessage( int Event , WPARAM wParam )
{
  //switch ( Event )
  //{
  //  case MSG_SWITCH_HANDLE:
  //  {
  //    if ( m_pSelectorWnd )
  //    {
  //      m_pSelectorWnd->ShowWindow( SW_HIDE ) ;
  //      int hNewWnd = (int) wParam ;
  //      FXPropertyKit Out ;
  //      Out.WriteInt( _T( "hTargetWindow" ) , hNewWnd ) ;
  //      bool Invalidate ;
  //      ScanProperties( Out , Invalidate ) ;
  //    }
  //  }
  //}
  return 0 ;
}

// int FRender::CheckCursorForm( bool bMouseMOve , CPoint& CursorPos )
// {
//   if ( m_wndOutput )
//   {
//     CPoint pnt= CursorPos ;
// 
//     m_wndOutput->m_LastCursor = cmplx( pnt.x , pnt.y ) ;
//     m_wndOutput->m_LastCalculatedCursor = NULL ;
//     m_wndOutput->m_iLastNearest = 100000 ;
//     m_wndOutput->m_Lock.Lock() ;
//     for ( int i = 0 ; i < m_wndOutput->m_GraphicsData.m_Points.GetCount() ; i++ )
//     {
//       CGPoint& pt = m_wndOutput->m_GraphicsData.m_Points.GetAt(i) ;
//       //           if ( pt.IsSelectable() )  // could be commented for debugging
//       {
//         cmplx cmplxpt( pt.x , pt.y ) ;
//         int iDist = ROUND(abs( cmplxpt - m_wndOutput->m_LastCursor )) ;
//         if ( iDist <= CURSOR_TOLERANCE   &&  iDist < m_wndOutput->m_iLastNearest )
//         {
//           //             if ( !pt.IsSelected() )
//           //             {
//           //               pt.SetSelected( true ) ;
//           //               m_wndOutput->m_LastCalculatedCursor = IDC_SIZEALL ;
//           //             }
//           pt.m_iDistToCursor = iDist ;
//           m_wndOutput->m_iLastNearest = iDist ;
//         }
//         //           else if ( pt.IsSelected() )
//         //           {
//         //             pt.SetSelected( false ) ;
//         //             m_wndOutput->m_LastCalculatedCursor = IDC_CROSS ;
//         //           }
//       }
//     }
//     for ( int i = 0 ; i < m_wndOutput->m_GraphicsData.m_Rects.GetCount() ; i++ )
//     {
//       FXRectangle& Rect = m_wndOutput->m_GraphicsData.m_Rects.GetAt(i) ;
//       if ( Rect.IsSelectable() )  // could be commented for debugging
//       {
//         int iDist = 100000 ;
//         int iCursSelect = Rect.CheckCursor( pnt , 
//           CURSOR_TOLERANCE /*/ m_ScrScale*/ , &iDist ) ;
//         if ( (iCursSelect != CURSOR_NO_SELECTION)  &&  (iDist < m_wndOutput->m_iLastNearest) )
//         {
//           m_wndOutput->m_iLastNearest = iDist ;
//           Rect.m_iDistToCursor = iDist ;
//           LPCSTR CursorForm = 0 ;
//           switch ( iCursSelect )
//           {
//           case CURSOR_ON_LEFT_BORDER:
//           case CURSOR_ON_RIGHT_BORDER:  CursorForm = IDC_SIZEWE ; break;
//           case CURSOR_ON_TOP_BORDER:
//           case CURSOR_ON_BOTTOM_BORDER: CursorForm = IDC_SIZENS ; break;
//           case CURSOR_ON_LTCORNER:
//           case CURSOR_ON_RBCORNER:      CursorForm = IDC_SIZENWSE ; break;
//           case CURSOR_ON_TRCORNER:
//           case CURSOR_ON_BLCORNER:      CursorForm = IDC_SIZENESW ; break;
//           case CURSOR_ON_WHOLE_SELECTION:CursorForm = IDC_SIZEALL ; break;
//           }
//           Rect.m_CursorForm = CursorForm ;
//           //             m_wndOutput->m_LastCalculatedCursor = CursorForm ;
//           //             Rect.SetSelected( true ) ;
//         }
//         //           else
//         //           {
//         //             if ( Rect.IsSelected() )
//         //             {
//         //               m_wndOutput->m_LastCalculatedCursor = IDC_CROSS ;
//         //               Rect.SetSelected( false ) ;
//         //             }
//         //           }
//       }
//     }
//     bool bThereIsUnselected = false , bThereIsSelected = false ;
//     for ( int i = 0 ; i < m_wndOutput->m_GraphicsData.m_Points.GetCount() ; i++ )
//     {
//       CGPoint& pt = m_wndOutput->m_GraphicsData.m_Points.GetAt(i) ;
//       if ( m_wndOutput->m_LastCalculatedCursor == NULL
//         && pt.m_iDistToCursor == m_wndOutput->m_iLastNearest )
//       {
//         if ( !pt.IsSelected() )
//         {
//           pt.SetSelected( true ) ;
//           m_wndOutput->m_LastCalculatedCursor = IDC_SIZEALL ;
//         }
//         else
//           bThereIsSelected = true ;
//       }
//       else 
//       {
//         if ( pt.IsSelected() )
//         {
//           pt.SetSelected( false ) ;
//           bThereIsUnselected = true ;
//         }
//         pt.m_iDistToCursor = 100001 ;
//       }
//     }
//     for ( int i = 0 ; i < m_wndOutput->m_GraphicsData.m_Rects.GetCount() ; i++ )
//     {
//       FXRectangle& Rect = m_wndOutput->m_GraphicsData.m_Rects.GetAt(i) ;
//       if ( m_wndOutput->m_LastCalculatedCursor == NULL
//         && Rect.m_iDistToCursor == m_wndOutput->m_iLastNearest )
//       {
//         if ( !Rect.IsSelected()  
//           ||  (m_wndOutput->m_LastCalculatedCursor != m_wndOutput->m_CurrentCursor) )
//         {
//           Rect.SetSelected( true ) ;
//           m_wndOutput->m_LastCalculatedCursor = Rect.m_CursorForm ;
//         }
//         else
//           bThereIsSelected = true ;
//       }
//       else 
//       {
//         if ( Rect.IsSelected() )
//         {
//           Rect.SetSelected( false ) ;
//           bThereIsUnselected = true ;
//         }
//         Rect.m_iDistToCursor = 100001 ;
//       }
//     }
// 
//     m_wndOutput->m_Lock.Unlock() ;
//     if ( bThereIsUnselected )
//       m_wndOutput->m_LastCalculatedCursor = IDC_CROSS ;
//     HCURSOR hCurrentCursor = GetCursor() ;
//     if ( !m_wndOutput->m_LastCalculatedCursor )
//     {
//       HCURSOR hCrossCursor = LoadCursor( NULL , IDC_CROSS ) ;
//       if ( hCrossCursor != hCurrentCursor )
//         m_wndOutput->m_LastCalculatedCursor = IDC_CROSS ;
//     }
//     if ( bMouseMOve )
//     {
//       if ( m_wndOutput->m_LastCalculatedCursor &&  !bThereIsSelected )
//       {
//         m_wndOutput->Invalidate( FALSE ) ;
//       }
//     }
//     else
//     {
//       SetCursor( LoadCursor(NULL , m_wndOutput->m_LastCalculatedCursor) ) ;
//       m_wndOutput->m_CurrentCursor = m_wndOutput->m_LastCalculatedCursor ;
//     }
// 
// }


// For DIB events processing
bool FRender::FormOutputText( CPoint& OnImagePt , DWORD dwBitMask , FXString * pOutString )
{
  FXString OutS ;

  int iRes = m_wndOutput->FormToolTipText( OnImagePt , dwBitMask , OutS ,
    true , m_bViewRGB = m_wndOutput->m_bShowRGB , true ) ;

  if ( !OutS.IsEmpty() )
  {
    if ( pOutString )
    {
      if ( (m_cScale.real()) != 1.0 || (m_cScale.imag() != 0.) )
      {
        CPoint Tmp( OnImagePt ) ;
//i        cmplx cPtOnImage = m_wndOutput->Scr2PicCmplx( Tmp ) ;
        m_wndOutput->Scr2Pic( Tmp ) ;
      }
      *pOutString = OutS ;
      m_wndOutput->Invalidate( FALSE ) ;// simple redraw
      return true ;
    }

    HWND hParent = m_wndOutput->GetSafeHwnd() ;
    HWND hParent2 = GetParent( hParent ) ;
    while ( hParent2 )
    {
      hParent = hParent2 ;
      hParent2 = GetParent( hParent ) ;
    }
    if ( hParent )
    {
      CString name;
      CWnd::FromHandle( hParent )->GetWindowText( name );
      int pos;
      if ( (pos = name.Find( "-" )) >= 0 )
      {
        name = name.Left( pos );
      }
      name += CString( "- " ) + (LPCTSTR)OutS;
      SetWindowText( hParent , name ) ;
    }
    //end of Moisey's addon

    CTextFrame* pMouseOutFrame = CreateTextFrame( (LPCTSTR)OutS , (LPCTSTR)NULL );
    PutFrame( m_pOutput , pMouseOutFrame , 100 ) ;
  }
  return false ;
}


cmplx FRender::SetScaleAndUnits( FXString& AsString )
{
  FXSIZE iPos = 0 ;
  AsString = AsString.Trim() ;
  FXString Token = AsString.Tokenize( " \t,;:" , iPos ) ;
  if ( !Token.IsEmpty() )
  {
    double dScale = atof( AsString ) ;
    if ( dScale != 0. )
    {
      m_dScale = dScale ;
      if ( m_sUnits == "um" || m_sUnits == "micron" )
        m_dScaleTenthPerUnit = 1. / 2.54 ;
      else if ( m_sUnits == "mm" )
        m_dScaleTenthPerUnit = 1000. / 2.54 ;
      else
        m_dScaleTenthPerUnit = 0. ; // not necessary to show tenth

      Token = AsString.Tokenize( " \t,;:" , iPos ) ;
      if ( !Token.IsEmpty() )
      {
        m_sUnits = Token ;
        Token = AsString.Tokenize( " \t,;:" , iPos ) ;
        FXString Units = m_sUnits ;
        Units.MakeLower() ;
        if ( Units == "um" || Units == "micron" || Units == "microns" )
          m_dScaleTenthPerUnit = 1. / 2.54 ;
        else if ( Units == "mm" )
          m_dScaleTenthPerUnit = 1000. / 2.54 ;
        else
          m_dScaleTenthPerUnit = 0. ; // not necessary to show tenth
        if ( m_wndOutput )
          m_wndOutput->m_dScaleTenthPerUnit = m_dScaleTenthPerUnit ;

        if ( !Token.IsEmpty() )
        {
          double dScaleX = atof( Token ) ;
          Token = AsString.Tokenize( " \t,;:" , iPos ) ;
          if ( !Token.IsEmpty() )
          {
            double dScaleY = atof( Token ) ;
            m_cScale = cmplx( dScaleX , dScaleY ) ;
            m_dScale = abs( m_cScale ) ;
            if ( m_wndOutput )
            {
              m_wndOutput->m_cScale = m_cScale ;
              m_wndOutput->m_sUnits = m_sUnits ;
            }
            FXString Tok = AsString.Tokenize( " \t,;:" , iPos ) ;
            if ( !Tok.IsEmpty() )
            {
              m_bcScaleCongugate = atoi( Tok ) != 0 ;
              if ( m_wndOutput )
                m_wndOutput->m_bConjugateScaleConvert = m_bcScaleCongugate ;
            }
          }
        }
      }
      else
        m_sUnits.Empty() ;
    }

    if ( Token.IsEmpty() )
      m_cScale = cmplx( 1. , 0. ) ;
  }
  return m_cScale;
}

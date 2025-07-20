// WdmSetup.cpp : implementation file
//

#include "stdafx.h"
#include "WDM.h"
#include "WdmSetup.h"
#include "WDMCaptureGadget.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "CWdmSetup"

inline void _resetCmdList( Buttons* b )
{
  for ( int i = 0; i < BUTTONS_NMB; i++ )
  {
    b[ i ].s_Enable = false;
    b[ i ].s_Name = "";
    b[ i ].s_SetupID = VfwNope;
  }
}


/////////////////////////////////////////////////////////////////////////////
// CWdmSetup dialog

CWdmSetup::CWdmSetup( CGadget* pGadget , CWnd* pParent /*=NULL*/ )
  : CGadgetSetupDialog( pGadget , CWdmSetup::IDD , pParent )
  , m_OutputFormat( 0 )
{
  //{{AFX_DATA_INIT(CWdmSetup)
    // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
  m_Buttons[ 0 ].s_ID = IDC_SETUP1;
  m_Buttons[ 1 ].s_ID = IDC_SETUP2;
  m_Buttons[ 2 ].s_ID = IDC_SETUP3;
  m_Buttons[ 3 ].s_ID = IDC_SETUP4;
  m_Buttons[ 4 ].s_ID = IDC_SETUP5;
  m_Buttons[ 5 ].s_ID = IDC_SETUP6;
  m_Buttons[ 6 ].s_ID = IDC_SETUP7;
  m_Buttons[ 7 ].s_ID = IDC_SETUP8;
  m_Buttons[ 8 ].s_ID = IDC_SETUP9;
  m_Buttons[ 9 ].s_ID = IDC_SETUP10;
  m_OutputFormat = 0;
  m_UpdateReq = true;
}


void CWdmSetup::DoDataExchange( CDataExchange* pDX )
{
  CGadgetSetupDialog::DoDataExchange( pDX );
  //{{AFX_DATA_MAP(CWdmSetup)
  DDX_Control( pDX , IDC_CAP_DEVICELIST , m_DeviceList );
  //}}AFX_DATA_MAP

  DDX_Radio( pDX , IDC_OUT_YUV9 , m_OutputFormat );
}


BEGIN_MESSAGE_MAP( CWdmSetup , CGadgetSetupDialog )
  //{{AFX_MSG_MAP(CWdmSetup)
  ON_WM_SHOWWINDOW()
  ON_BN_CLICKED( IDC_SETUP1 , OnSetup1 )
  ON_BN_CLICKED( IDC_SETUP2 , OnSetup2 )
  ON_BN_CLICKED( IDC_SETUP3 , OnSetup3 )
  ON_BN_CLICKED( IDC_SETUP4 , OnSetup4 )
  ON_BN_CLICKED( IDC_SETUP5 , OnSetup5 )
  ON_BN_CLICKED( IDC_SETUP6 , OnSetup6 )
  ON_BN_CLICKED( IDC_SETUP7 , OnSetup7 )
  ON_BN_CLICKED( IDC_SETUP8 , OnSetup8 )
  ON_BN_CLICKED( IDC_SETUP9 , OnSetup9 )
  ON_BN_CLICKED( IDC_SETUP10 , OnSetup10 )
  ON_CBN_SELCHANGE( IDC_CAP_DEVICELIST , OnSelchangeCapDevicelist )
  ON_WM_TIMER()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWdmSetup message handlers

bool CWdmSetup::Show( CPoint point , LPCTSTR uid )
{
  FXString DlgHead;
  DlgHead.Format( "%s Setup Dialog" , uid );
  if ( !m_hWnd )
  {
    FX_UPDATERESOURCE fur( pThisDll->m_hResource );
    if ( !Create( IDD_SETUPDIALOG , NULL ) )
    {
      SENDERR_0( "Failed to create Setup Dialog" );
      return false;
    }
  }
  SetWindowText( DlgHead );
  SetWindowPos( NULL , point.x , point.y , 0 , 0 , SWP_NOSIZE | SWP_NOZORDER );
  ShowWindow( SW_SHOWNORMAL );
  return true;
}

void CWdmSetup::UploadParams()
{
  bool Invalidate = false;
  UpdateData( TRUE );
  FXPropertyKit pk;
  CString dName;
  int nIndex = m_DeviceList.GetCurSel();
  if ( (nIndex != CB_ERR) && (m_DeviceList.GetLBTextLen( nIndex ) != CB_ERR) && (m_DeviceList.GetLBTextLen( nIndex ) != 0) )
  {
    m_DeviceList.GetLBText( m_DeviceList.GetCurSel() , dName );
    if ( dName.GetLength() )
    {
      pk.WriteString( "Device" , dName );

      switch ( m_OutputFormat )
      {
        case 0:
          pk.WriteInt( "outputformat" , BI_YUV9 );
          break;
        case 1:
          pk.WriteInt( "outputformat" , BI_YUV12 );
          break;
        case 2:
          pk.WriteInt( "outputformat" , BI_Y8 );
          break;
      }
      m_pGadget->ScanProperties( pk , Invalidate );
      CGadgetSetupDialog::UploadParams();
    }
  }
  KillTimer( m_Timer );
}

void CWdmSetup::OnChangeStatus()
{
  if ( ((CCaptureGadget*) m_pGadget)->IsRun() )
  {
    GetDlgItem( IDC_OUT_YUV9 )->EnableWindow( FALSE );
    GetDlgItem( IDC_OUT_YUV12 )->EnableWindow( FALSE );
    GetDlgItem( IDC_OUT_Y8 )->EnableWindow( FALSE );
  }
  else
  {
    GetDlgItem( IDC_OUT_YUV9 )->EnableWindow( TRUE );
    GetDlgItem( IDC_OUT_YUV12 )->EnableWindow( TRUE );
    GetDlgItem( IDC_OUT_Y8 )->EnableWindow( TRUE );
  }
}

void CWdmSetup::OnShowWindow( BOOL bShow , UINT nStatus )
{
  CGadgetSetupDialog::OnShowWindow( bShow , nStatus );
  if ( bShow )
  {
    FXPropertyKit pk;

    m_pGadget->PrintProperties( pk );
    int outformat;
    pk.GetInt( "outputformat" , outformat );
    switch ( outformat )
    {
      case BI_YUV9:
        m_OutputFormat = 0;
        break;
      case BI_YUV12:
        m_OutputFormat = 1;
        break;
      case BI_Y8:
        m_OutputFormat = 2;
        break;
    }
    OnChangeStatus();
    UpdateData( FALSE );
    WDMCapture* Driver = (WDMCapture*) m_pGadget;
    m_DeviceList.ResetContent();
    int i;
    for ( i = 0; i < Driver->s_iNumVCapDevices; i++ )
    {
      CString Name = Driver->GetDeviceName( i ) ;
      if ( Name.Find( "-Not Detected") < 0 )
        m_DeviceList.AddString( Name );
    }
    if ( Driver->s_pmVideo != 0 ) m_DeviceList.SetCurSel( Driver->s_iDevNumSelected );
    int btnNmb = 0;
    _resetCmdList( m_Buttons );
    for ( int id = VfwCaptureDialogSource; id <= WdmVideoInput2; id++ )
    {
      const char *nm = Driver->GetCfgDialogs( (SetupID) id );
      if ( nm )
      {
        m_Buttons[ btnNmb ].s_Enable = true;
        m_Buttons[ btnNmb ].s_Name = nm;
        m_Buttons[ btnNmb ].s_SetupID = (SetupID) id;
        btnNmb++;
      }
      if ( btnNmb >= BUTTONS_NMB ) break;
    }
    for ( i = 0; i < BUTTONS_NMB; i++ )
    {
      if ( m_Buttons[ i ].s_Enable )
      {
        GetDlgItem( m_Buttons[ i ].s_ID )->SetWindowText( m_Buttons[ i ].s_Name );
        GetDlgItem( m_Buttons[ i ].s_ID )->EnableWindow( TRUE );
        GetDlgItem( m_Buttons[ i ].s_ID )->ShowWindow( SW_SHOW );
      }
      else
      {
        GetDlgItem( m_Buttons[ i ].s_ID )->EnableWindow( FALSE );
        GetDlgItem( m_Buttons[ i ].s_ID )->ShowWindow( SW_HIDE );
      }
    }
  }
  m_Timer = SetTimer( 10 , 100 , NULL );
}

void CWdmSetup::OnSetup( int nmb )
{
  HRESULT hr;
  if ( !m_Buttons[ nmb ].s_Enable ) return;
  WDMCapture* Driver = (WDMCapture*) m_pGadget;
  switch ( m_Buttons[ nmb ].s_SetupID )
  {
    case VfwCaptureDialogSource:
    {
      // this dialog will not work while previewing

      Driver->DestroyGraph();

      HRESULT hrD;
      hrD = Driver->s_pDlg->ShowDialog( VfwCaptureDialog_Format , this->m_hWnd );
      // Oh uh!  Sometimes bringing up the FORMAT dialog can result
      // in changing to a capture format that the current graph 
      // can't handle.  It looks like that has happened and we'll
      // have to rebuild the graph.
      if ( hrD == VFW_E_CANNOT_CONNECT )
      {
        DbgLog( (LOG_TRACE , 1 , TEXT( "DIALOG CORRUPTED GRAPH!" )) );
        Driver->TearDownGraph();	// now we need to rebuild
        // !!! This won't work if we've left a stranded h/w codec
      }

      // Resize our window to be the same size that we're capturing
      if ( Driver->s_pVSC )
      {
        AM_MEDIA_TYPE *pmt;
        // get format being used NOW
        hr = Driver->s_pVSC->GetFormat( &pmt );
        // DV capture does not use a VIDEOINFOHEADER
        if ( hr == NOERROR )
        {
          if ( pmt->formattype == FORMAT_VideoInfo )
          {
            // resize our window to the new capture size
            //ResizeWindow(HEADER(pmt->pbFormat)->biWidth,abs(HEADER(pmt->pbFormat)->biHeight));
          }
          DeleteMediaType( pmt );
        }
      }
      Driver->RestoreGraph();
    }
    break;
    case VfwCaptureDialogFormat:
      break;
    case VfwCaptureDialogDisplay:
      break;
    case WdmVideoCaptureFilter:
    {
      ISpecifyPropertyPages *pSpec;
      CAUUID cauuid;
      hr = Driver->s_pVCap->QueryInterface( IID_ISpecifyPropertyPages ,
        (void **) &pSpec );
      if ( hr == S_OK )
      {
        hr = pSpec->GetPages( &cauuid );
        hr = OleCreatePropertyFrame( this->m_hWnd , 30 , 30 , NULL , 1 , (IUnknown **) &Driver->s_pVCap , cauuid.cElems , (GUID *) cauuid.pElems , 0 , 0 , NULL );
        CoTaskMemFree( cauuid.pElems );
        pSpec->Release();
      }
    }
    break;
    case WdmVideoCapturePin:
    {
      IAMStreamConfig *pSC;
      ISpecifyPropertyPages *pSpec;
      CAUUID cauuid;

      Driver->DestroyGraph();

      if ( Driver->s_fCaptureGraphBuilt || Driver->s_fPreviewGraphBuilt )
      {
        DbgLog( (LOG_TRACE , 1 , TEXT( "Tear down graph for dialog" )) );
        Driver->TearDownGraph();	// graph could prevent dialog working
      }
      hr = Driver->s_pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE ,
        &MEDIATYPE_Interleaved , Driver->s_pVCap ,
        IID_IAMStreamConfig , (void **) &pSC );
      if ( hr != NOERROR )
        hr = Driver->s_pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE ,
        &MEDIATYPE_Video , Driver->s_pVCap ,
        IID_IAMStreamConfig , (void **) &pSC );
      hr = pSC->QueryInterface( IID_ISpecifyPropertyPages , (void **) &pSpec );
      if ( hr == S_OK )
      {
        hr = pSpec->GetPages( &cauuid );
        hr = OleCreatePropertyFrame( this->m_hWnd , 30 , 30 , NULL , 1 , (IUnknown **) &pSC , cauuid.cElems , (GUID *) cauuid.pElems , 0 , 0 , NULL );

        // !!! What if changing output formats couldn't reconnect
        // and the graph is broken?  Shouldn't be possible...

        if ( Driver->s_pVSC )
        {
          AM_MEDIA_TYPE *pmt;
          // get format being used NOW
          hr = Driver->s_pVSC->GetFormat( &pmt );
          // DV capture does not use a VIDEOINFOHEADER
          if ( hr == NOERROR )
          {
            if ( pmt->formattype == FORMAT_VideoInfo )
            {
              // resize our window to the new capture size
              //ResizeWindow(HEADER(pmt->pbFormat)->biWidth,abs(HEADER(pmt->pbFormat)->biHeight));
            }
            DeleteMediaType( pmt );
          }
        }
        CoTaskMemFree( cauuid.pElems );
        pSpec->Release();
      }
      pSC->Release();
      Driver->RestoreGraph();
    }
    break;
    case WdmVideoPreviewPin:
    {
      // this dialog may not work if the preview pin is connected
      // already, because the downstream filter may reject a format
      // change, so we better kill the graph. (EG: We switch from 
                  // capturing RGB to some compressed fmt, and need to pull in
                  // a decompressor)

      Driver->DestroyGraph();

      IAMStreamConfig *pSC;
      // This dialog changes the preview format, so it might affect
      // the format being drawn.  Our app's window size is taken
      // from the size of the capture pin's video, not the preview
      // pin, so changing that here won't have any effect. All in all,
      // this probably won't be a terribly useful dialog in this app.
      hr = Driver->s_pBuilder->FindInterface( &PIN_CATEGORY_PREVIEW ,
        &MEDIATYPE_Interleaved , Driver->s_pVCap ,
        IID_IAMStreamConfig , (void **) &pSC );
      if ( hr != NOERROR )
        hr = Driver->s_pBuilder->FindInterface( &PIN_CATEGORY_PREVIEW ,
        &MEDIATYPE_Video , Driver->s_pVCap ,
        IID_IAMStreamConfig , (void **) &pSC );
      ISpecifyPropertyPages *pSpec;
      CAUUID cauuid;
      hr = pSC->QueryInterface( IID_ISpecifyPropertyPages ,
        (void **) &pSpec );
      if ( hr == S_OK )
      {
        hr = pSpec->GetPages( &cauuid );
        hr = OleCreatePropertyFrame( this->m_hWnd , 30 , 30 , NULL , 1 , (IUnknown **) &pSC , cauuid.cElems , (GUID *) cauuid.pElems , 0 , 0 , NULL );
        CoTaskMemFree( cauuid.pElems );
        pSpec->Release();
      }
      pSC->Release();

      Driver->RestoreGraph();

    }
    break;
    case WdmVideoCrossbar:
    {
      IAMCrossbar *pX;
      hr = Driver->s_pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE ,
        &MEDIATYPE_Interleaved , Driver->s_pVCap ,
        IID_IAMCrossbar , (void **) &pX );
      if ( hr != NOERROR )
        hr = Driver->s_pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE , &MEDIATYPE_Video , Driver->s_pVCap , IID_IAMCrossbar , (void **) &pX );
      ISpecifyPropertyPages *pSpec;
      CAUUID cauuid;
      hr = pX->QueryInterface( IID_ISpecifyPropertyPages , (void **) &pSpec );
      if ( hr == S_OK )
      {
        hr = pSpec->GetPages( &cauuid );
        hr = OleCreatePropertyFrame( this->m_hWnd , 30 , 30 , NULL , 1 , (IUnknown **) &pX , cauuid.cElems , (GUID *) cauuid.pElems , 0 , 0 , NULL );
        CoTaskMemFree( cauuid.pElems );
        pSpec->Release();
      }
      pX->Release();
    }
    break;
    case WdmSecondCrossbar:
      break;
    case WdmTVTuner:
    {
      IAMTVTuner *pTV;
      hr = Driver->s_pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE ,
        &MEDIATYPE_Interleaved , Driver->s_pVCap ,
        IID_IAMTVTuner , (void **) &pTV );
      if ( hr != NOERROR )
        hr = Driver->s_pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE ,
        &MEDIATYPE_Video , Driver->s_pVCap ,
        IID_IAMTVTuner , (void **) &pTV );
      ISpecifyPropertyPages *pSpec;
      CAUUID cauuid;
      hr = pTV->QueryInterface( IID_ISpecifyPropertyPages , (void **) &pSpec );
      if ( hr == S_OK )
      {
        hr = pSpec->GetPages( &cauuid );
        hr = OleCreatePropertyFrame( this->m_hWnd , 30 , 30 , NULL , 1 , (IUnknown **) &pTV , cauuid.cElems , (GUID *) cauuid.pElems , 0 , 0 , NULL );
        CoTaskMemFree( cauuid.pElems );
        pSpec->Release();
      }
      pTV->Release();
    }
    break;
    case WdmVideoInput0:
      break;
    case WdmVideoInput1:
      break;
    case WdmVideoInput2:
      break;
  }
}

void CWdmSetup::UpdateButtons()
{
  for ( int i = 0; i < BUTTONS_NMB; i++ )
  {
    if ( m_Buttons[ i ].s_Enable )
    {
      GetDlgItem( m_Buttons[ i ].s_ID )->SetWindowText( m_Buttons[ i ].s_Name );
      GetDlgItem( m_Buttons[ i ].s_ID )->EnableWindow( TRUE );
      GetDlgItem( m_Buttons[ i ].s_ID )->ShowWindow( SW_SHOW );
    }
    else
    {
      GetDlgItem( m_Buttons[ i ].s_ID )->EnableWindow( FALSE );
      GetDlgItem( m_Buttons[ i ].s_ID )->ShowWindow( SW_HIDE );
    }
  }
  m_UpdateReq = false;
}


void CWdmSetup::GetCmdList()
{
  WDMCapture* Driver = (WDMCapture*) m_pGadget;
  int btnNmb = 0;
  _resetCmdList( m_Buttons );
  for ( int id = VfwCaptureDialogSource; id <= WdmVideoInput2; id++ )
  {
    const char *nm = Driver->GetCfgDialogs( (SetupID) id );
    if ( nm )
    {
      m_Buttons[ btnNmb ].s_Enable = true;
      m_Buttons[ btnNmb ].s_Name = nm;
      m_Buttons[ btnNmb ].s_SetupID = (SetupID) id;
      btnNmb++;
    }
    if ( btnNmb >= BUTTONS_NMB ) break;
  }
}


void CWdmSetup::OnSelchangeCapDevicelist()
{
  WDMCapture* Driver = (WDMCapture*) m_pGadget;
  Driver->DestroyGraph();
  if ( Driver->SelectDevice( Driver->s_rgpmVideoMenu[ m_DeviceList.GetCurSel() ] ) != NOERROR )
  {
    CString DevName;
    m_DeviceList.GetLBText( m_DeviceList.GetCurSel() , DevName );
    CString message;
    message.Format( "Error: can't set '%s' as input device" , DevName );
    SENDERR_0( message );
    if ( Driver->s_pmVideo != 0 )
      m_DeviceList.SetCurSel( Driver->s_iDevNumSelected );
  }
  else
  {
    GetCmdList();
    Driver->s_iDevNumSelected = m_DeviceList.GetCurSel();
  }
  Driver->RestoreGraph();
  UpdateButtons();
}

void CWdmSetup::OnTimer( UINT_PTR nIDEvent )
{
  CGadgetSetupDialog::OnTimer( nIDEvent );
  if ( m_UpdateReq ) UpdateButtons();
}


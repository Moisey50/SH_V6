// TunerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Tuner.h"
#include "TunerDlg.h"
#include <files\imgfiles.h>
#include <files\futils.h>
#include <files\FileList.h>
#include <files\fileerrormessages.h>
#include <messages.h>
#include "AviFrameList.h"
#include "FeatureDetectorDsp.h"
#include "HistogramView.h"
#include "DiffrerenceView.h"
#include "UPCReaderDsp.h"
#include "Code39Reader.h"
#include "Code128Reader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTunerDlg dialog

CTunerDlg::CTunerDlg( CWnd* pParent /*=NULL*/ )
  : CDialog( CTunerDlg::IDD , pParent ) ,
  m_pFL( NULL ) ,
  m_DispDlg( NULL ) ,
  m_Status( TRUE ) ,
  m_SliderChangePos( false ) ,
  m_StatePlay( 0 )
{
  //{{AFX_DATA_INIT(CTunerDlg)
  m_FileName = _T( "" );
  m_FileNmbInfo = _T( "" );
  m_SaveAsFileName1 = _T( "" );
  m_DspMethod = -1;
  //}}AFX_DATA_INIT
  m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
}

void CTunerDlg::DoDataExchange( CDataExchange* pDX )
{
  CDialog::DoDataExchange( pDX );
  //{{AFX_DATA_MAP(CTunerDlg)
  DDX_Control( pDX , IDC_CLIPSLIDER , m_ClipSlider );
  DDX_Text( pDX , IDC_FNAME , m_FileName );
  DDX_Text( pDX , IDC_FILE_NUMBER , m_FileNmbInfo );
  DDX_Text( pDX , IDC_SAVEFILENAME1 , m_SaveAsFileName1 );
  DDX_Radio( pDX , IDC_FEATUREDETECTORS , m_DspMethod );
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP( CTunerDlg , CDialog )
  //{{AFX_MSG_MAP(CTunerDlg)
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_BN_CLICKED( IDC_FILEOPEN , OnFileopen )
  ON_WM_DESTROY()
  ON_BN_CLICKED( IDC_FILE_NEXT , OnFileNext )
  ON_BN_CLICKED( IDC_FILE_PREVIOUS , OnFilePrevious )
  ON_BN_CLICKED( IDC_RUN_TVDB , OnRunTvdb )
  ON_WM_TIMER()
  ON_BN_CLICKED( IDC_HOME , OnHome )
  ON_BN_CLICKED( IDC_END , OnEnd )
  ON_BN_CLICKED( IDC_FILE_DELETE , OnFileDelete )
  ON_WM_HSCROLL()
  ON_BN_CLICKED( IDC_FILE_PLAY , OnFilePlay )
  ON_BN_CLICKED( IDC_SAVEAS1 , OnSaveas1 )
  ON_BN_CLICKED( IDC_HISTOGRAM , OnHistogram )
  ON_BN_CLICKED( IDC_FEATUREDETECTORS , OnFeaturedetectors )
  ON_BN_CLICKED( IDC_DIFFRENCE , OnDiffrence )
  ON_BN_CLICKED( IDC_ZONEDETECTOR , OnZonedetector )
  ON_BN_CLICKED( IDC_SEEKNUMBER , OnSeeknumber )
  ON_BN_CLICKED( IDC_UPCREADER , OnUpcreader )
  ON_BN_CLICKED( IDC_CODE39 , OnCode39 )
  ON_BN_CLICKED( IDC_CODE128 , OnCode128 )
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTunerDlg message handlers

BOOL CTunerDlg::OnInitDialog()
{
  CDialog::OnInitDialog();
  LoadSettings();
  SetIcon( m_hIcon , TRUE );			// Set big icon
  SetIcon( m_hIcon , FALSE );		// Set small icon
  UseFileFilters();
  AVIFilesSupported();

  m_Display.Create( GetDlgItem( IDC_VIEWFRAME ) );
  m_Display.SetScale( -1 );
  m_AppName.LoadString( IDS_APP_NAME );
  m_hAccel = LoadAccelerators( AfxGetInstanceHandle() ,
    MAKEINTRESOURCE( IDR_TUNER_ACCELERATOR ) );
  m_TranslateAccel = true;
  SetTimer( 13 , 20 , NULL );
  if ( m_CmdLine.GetLength() )
  {
    FileFiltersDone();
    UseFileFilters();
    m_fName = m_CmdLine;
    CFileList *FL = new CFileList;
    m_pFL = FL;
    m_LastDir = FxExtractPath( m_fName );
    FL->Dir( m_LastDir , GetAllFilesExtensions() );
    FL->SetPosition( m_fName );
    if ( !LoadFile( m_fName ) )
      AfxMessageBox( ERROR_CANTREADFILE );
    AVIFilesSupported();
    UpdateData( FALSE );
  }
  UpdateData( FALSE );
  return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTunerDlg::OnDestroy()
{
  FileFiltersDone();
  if ( m_pFL ) delete m_pFL;
  SaveSettings();
  if ( m_DispDlg )
  {
    m_DispDlg->DestroyWindow();
    delete m_DispDlg;
  }
  CDialog::OnDestroy();
}

void CTunerDlg::LoadSettings()
{
  m_LastDir = AfxGetApp()->GetProfileString( "Root" , "LastDir" , ".\\" );
  m_SaveAsFileName1 = AfxGetApp()->GetProfileString( "Root" , "SaveAsDir1" , ".\\" );
  m_DspMethod = AfxGetApp()->GetProfileInt( "Root" , "DispMethod" , 0 );
  UpdateData( FALSE );
  switch ( m_DspMethod )
  {
    case 0:
      OnFeaturedetectors();
      break;
    case 1:
      OnZonedetector();
      break;
    case 2:
      OnHistogram();
      break;
    case 3:
      OnDiffrence();
      break;
    case 4:
      OnUpcreader();
      break;
    case 5:
      OnCode39();
      break;
    case 6:
      OnCode128();
      break;
  }
}

void CTunerDlg::SaveSettings()
{
  UpdateData( TRUE );
  AfxGetApp()->WriteProfileString( "Root" , "LastDir" , m_LastDir );
  AfxGetApp()->WriteProfileString( "Root" , "SaveAsDir1" , m_SaveAsFileName1 );
  AfxGetApp()->WriteProfileInt( "Root" , "DispMethod" , m_DspMethod );
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTunerDlg::OnPaint()
{
  if ( IsIconic() )
  {
    CPaintDC dc( this ); // device context for painting

    SendMessage( WM_ICONERASEBKGND , (WPARAM) dc.GetSafeHdc() , 0 );

    // Center icon in client rectangle
    int cxIcon = GetSystemMetrics( SM_CXICON );
    int cyIcon = GetSystemMetrics( SM_CYICON );
    CRect rect;
    GetClientRect( &rect );
    int x = (rect.Width() - cxIcon + 1) / 2;
    int y = (rect.Height() - cyIcon + 1) / 2;

    // Draw the icon
    dc.DrawIcon( x , y , m_hIcon );
  }
  else
  {
    CDialog::OnPaint();
  }
}

bool CTunerDlg::LoadDIB( BITMAPINFOHEADER *bmih )
{
  bool result = m_Display.LoadDIB( bmih );
  if ( m_DispDlg ) m_DispDlg->LoadDIB( bmih );
  return result;
}

HCURSOR CTunerDlg::OnQueryDragIcon()
{
  return (HCURSOR) m_hIcon;
}

void CTunerDlg::OnFileopen()
{
  if ( m_pFL ) delete m_pFL; m_pFL = NULL;
  CFileDialog FD( TRUE , "bmp" , NULL , OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT , GetInputFileFilter() , NULL );
  FD.m_ofn.lpstrInitialDir = m_LastDir.GetBuffer( _MAX_PATH );
  if ( FD.DoModal() == IDOK )
  {
    CWaitCursor wt;
    if ( m_DispDlg ) m_DispDlg->Reset();
    m_LastDir.ReleaseBuffer();
    if ( FD.GetFileExt().CompareNoCase( "AVI" ) == 0 )
    {
      m_fName = FD.GetPathName();
      m_FileName = FD.GetFileName();
      CAviFrameList *FL = new CAviFrameList;
      m_pFL = FL;
      m_LastDir = m_fName.Left( m_fName.GetLength() - strlen( FD.GetFileName() ) );
      if ( !FL->Open( m_fName ) )
      {
        delete m_pFL; m_pFL = NULL;
        AfxMessageBox( ERROR_CANTREADFILE );
        UpdateData( FALSE );
        return;
      }
      m_pFL->SetFirst();
      LoadFile( m_pFL->Get() );
      UpdateData( FALSE );
      return;
    }
    else
    {
      FileFiltersDone();
      UseFileFilters();
      m_fName = FD.GetPathName();
      CFileList *FL = new CFileList;
      m_pFL = FL;
      m_LastDir = m_fName.Left( m_fName.GetLength() - strlen( FD.GetFileName() ) );
      FL->Dir( m_LastDir , GetAllFilesExtensions() );
      FL->SetPosition( m_fName );
      if ( !LoadFile( FD.GetPathName() ) )
        AfxMessageBox( ERROR_CANTREADFILE );
      AVIFilesSupported();
      UpdateData( FALSE );
      return;
    }
  }
  m_LastDir.ReleaseBuffer();
  UpdateData( FALSE );
}

bool CTunerDlg::LoadFile( const char *fName )
{
  CString wndName;
  LPBITMAPINFOHEADER bmih = ::loadDIB( fName );
  if ( bmih )
  {
    if ( m_pFL )
    {
      if ( m_pFL->IsStream() )
      {
        wndName.Format( m_AppName , m_FileName );
        SetWindowText( wndName );
      }
      else
      {
        wndName.Format( m_AppName , fName );
        SetWindowText( wndName );
        m_FileName = GetFileName( fName );
      }
      m_FileNmbInfo.Format( "%5d:%5d" , m_pFL->GetItemNo() + 1 , m_pFL->GetSize() );
      m_ClipSlider.SetRange( 0 , m_pFL->GetSize() - 1 );
      m_ClipSlider.SetPos( m_pFL->GetItemNo() );
    }
    UpdateData( FALSE );
    LoadDIB( bmih ); free( bmih );
    return true;
  }
  LoadDIB( NULL );
  wndName.Format( m_AppName , "none" );
  SetWindowText( wndName );
  m_FileName = "";
  m_FileNmbInfo = "0:0";
  UpdateData( FALSE );
  return false;
}

void CTunerDlg::OnRunTvdb()
{
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory( &si , sizeof( si ) );
  si.cb = sizeof( si );

  CString execpath = GetStartDir();
  execpath += "tvdb300.exe";
  execpath += " \"";
  execpath += m_pFL->Get();
  execpath += "\"";
  char a[ _MAX_PATH ];
  if ( execpath.GetLength() > _MAX_PATH - 1 )
  {
    AfxMessageBox( ERROR_UNKNOWN );
    return;
  }
  strcpy( a , execpath );
  if ( !CreateProcess( NULL , a , NULL , NULL , FALSE , 0 , NULL , NULL , &si , &pi ) )
  {
    AfxMessageBox( ERROR_CANTSTART_EXE );
    return;
  }
  WaitForSingleObject( pi.hProcess , INFINITE );

  // Close process and thread handles. 
  CloseHandle( pi.hProcess );
  CloseHandle( pi.hThread );
}

void CTunerDlg::OnTimer( UINT_PTR nIDEvent )
{
  BOOL status = (m_pFL != NULL) && (!m_StatePlay);
  if ( status != m_Status )
  {
    bool isStream = ((m_pFL) && (m_pFL->IsStream()));
    GetDlgItem( IDC_FILEOPEN )->EnableWindow( !m_StatePlay );
    GetDlgItem( IDC_FILE_PREVIOUS )->EnableWindow( status );
    GetDlgItem( IDC_FILE_NEXT )->EnableWindow( status );
    GetDlgItem( IDC_RUN_TVDB )->EnableWindow( status );
    GetDlgItem( IDC_HOME )->EnableWindow( status );
    GetDlgItem( IDC_END )->EnableWindow( status );
    GetDlgItem( IDC_CLIPSLIDER )->EnableWindow( status );
    GetDlgItem( IDC_SAVEAS1 )->EnableWindow( status );
    GetDlgItem( IDC_FILE_PLAY )->EnableWindow( m_pFL != NULL );
    GetDlgItem( IDC_FILE_DELETE )->EnableWindow( status && (!isStream) );
    m_Status = status;
  }
  if ( (m_pFL) && (!m_pFL->IsEmpty()) )
  {
    if ( m_SliderChangePos )
    {
      m_SliderChangePos = false;
      DWORD pos = m_ClipSlider.GetPos();
      m_pFL->SeekTo( pos );
      LoadFile( m_pFL->Get() );
    }
    if ( m_StatePlay )
    {
      OnFileNext();
    }
  }
  CDialog::OnTimer( nIDEvent );
}

BOOL CTunerDlg::PreTranslateMessage( MSG* pMsg )
{
  if ( m_TranslateAccel && m_hAccel != NULL && ::TranslateAccelerator( m_hWnd , m_hAccel , pMsg ) ) return TRUE;
  return CDialog::PreTranslateMessage( pMsg );
}

void CTunerDlg::OnFileNext()
{
  if ( (!m_pFL) || (m_pFL->IsEmpty()) ) return;
  if ( !m_pFL->Next() ) m_pFL->SetFirst();
  LoadFile( m_pFL->Get() );

}

void CTunerDlg::OnFilePrevious()
{
  if ( (!m_pFL) || (m_pFL->IsEmpty()) ) return;
  if ( !m_pFL->Previous() ) m_pFL->SetLast();
  LoadFile( m_pFL->Get() );
}


void CTunerDlg::OnHome()
{
  if ( (!m_pFL) || (m_pFL->IsEmpty()) ) return;
  m_pFL->SetFirst();
  LoadFile( m_pFL->Get() );
}

void CTunerDlg::OnEnd()
{
  if ( (!m_pFL) || (m_pFL->IsEmpty()) ) return;
  m_pFL->SetLast();
  LoadFile( m_pFL->Get() );
}

void CTunerDlg::OnFileDelete()
{
  if ( AfxMessageBox( PROMPT_DELETFILE , MB_YESNO ) == IDYES )
  {
    if ( !m_pFL->IsStream() )
    {
      CFile::Remove( m_FileName );
      m_pFL->Delete();
      if ( m_pFL->IsEmpty() )
      {
        delete m_pFL;
        m_pFL = NULL;
      }
      else LoadFile( m_pFL->Get() );
    }
    else AfxMessageBox( WARRNING_NOTIMPLEMENTED4STREAMS );
  }
}

void CTunerDlg::OnHScroll( UINT nSBCode , UINT nPos , CScrollBar* pScrollBar )
{
  m_SliderChangePos = true;
  CDialog::OnHScroll( nSBCode , nPos , pScrollBar );
}

void CTunerDlg::OnFilePlay()
{
  m_StatePlay = !m_StatePlay;
  if ( m_StatePlay )
    GetDlgItem( IDC_FILE_PLAY )->SetWindowText( "&Stop" );
  else
  {
    GetDlgItem( IDC_FILE_PLAY )->SetWindowText( "&Play" );
    GetDlgItem( IDC_FEATUREDETECTORS )->EnableWindow( true );
    GetDlgItem( IDC_ZONEDETECTOR )->EnableWindow( true );
    GetDlgItem( IDC_HISTOGRAM )->EnableWindow( true );
    GetDlgItem( IDC_DIFFRENCE )->EnableWindow( true );
  }
}

void CTunerDlg::OnSeeknumber()
{
  m_StatePlay = !m_StatePlay;
  if ( m_StatePlay )
  {
    GetDlgItem( IDC_FILE_PLAY )->SetWindowText( "&Stop" );
    m_StatePlay |= 2;
    GetDlgItem( IDC_FEATUREDETECTORS )->EnableWindow( false );
    GetDlgItem( IDC_ZONEDETECTOR )->EnableWindow( false );
    GetDlgItem( IDC_HISTOGRAM )->EnableWindow( false );
    GetDlgItem( IDC_DIFFRENCE )->EnableWindow( false );

  }
  else
  {
    GetDlgItem( IDC_FILE_PLAY )->SetWindowText( "&Play" );
    GetDlgItem( IDC_FEATUREDETECTORS )->EnableWindow( true );
    GetDlgItem( IDC_ZONEDETECTOR )->EnableWindow( true );
    GetDlgItem( IDC_HISTOGRAM )->EnableWindow( true );
    GetDlgItem( IDC_DIFFRENCE )->EnableWindow( true );
  }
}

void CTunerDlg::OnSaveas1()
{
  UpdateData( TRUE );
  CString path2save = m_SaveAsFileName1;
  if ( path2save.GetLength() == 0 )
  {
    AfxMessageBox( WARRNING_PATHNOTDEFINED );
    return;
  }
  if ( path2save.GetAt( path2save.GetLength() - 1 ) != '\\' ) path2save += '\\';
  path2save += m_pFL->GetFileName();
  if ( !saveDIB( path2save , m_Display.GetFramePntr() ) )
  {
    CreateDirectory( m_SaveAsFileName1 , NULL );
    if ( !saveDIB( path2save , m_Display.GetFramePntr() ) )
      AfxMessageBox( ERROR_CANTWRITEFILE );
  }
}

void CTunerDlg::OnHistogram()
{
  UpdateData( TRUE );
  if ( m_DispDlg )
  {
    if ( m_DispDlg->IsType( IDD_HISTORGAMVIEW ) ) return;
    m_DispDlg->DestroyWindow();
    delete m_DispDlg;
  }
  m_DispDlg = new CHistogramView( GetDlgItem( IDC_PLUGINPARENT ) );
  m_DispDlg->Create( GetDlgItem( IDC_PLUGINPARENT ) );
}

void CTunerDlg::OnFeaturedetectors()
{
  UpdateData( TRUE );
  if ( m_DispDlg )
  {
    if ( m_DispDlg->IsType( ID_FEATUREDETECTOR ) ) return;
    m_DispDlg->DestroyWindow();
    delete m_DispDlg;
  }
  m_DispDlg = new CFeatureDetectorDsp( GetDlgItem( IDC_PLUGINPARENT ) );
  m_DispDlg->Create( GetDlgItem( IDC_PLUGINPARENT ) );
}

void CTunerDlg::OnDiffrence()
{
  UpdateData( TRUE );
  if ( m_DispDlg )
  {
    if ( m_DispDlg->IsType( IDD_DIFFERENCEVIEW ) ) return;
    m_DispDlg->DestroyWindow();
    delete m_DispDlg;
  }
  m_DispDlg = new CDiffrerenceView( GetDlgItem( IDC_PLUGINPARENT ) );
  m_DispDlg->Create( GetDlgItem( IDC_PLUGINPARENT ) );
}

void CTunerDlg::OnZonedetector()
{
  UpdateData( TRUE );
  if ( m_DispDlg )
  {
    if ( m_DispDlg->IsType( IDD_DIFFERENCEVIEW ) ) return;
    m_DispDlg->DestroyWindow();
    delete m_DispDlg;
  }
  m_DispDlg = new CDiffrerenceView( GetDlgItem( IDC_PLUGINPARENT ) );
  m_DispDlg->Create( GetDlgItem( IDC_PLUGINPARENT ) );
}


void CTunerDlg::OnUpcreader()
{
  UpdateData( TRUE );
  if ( m_DispDlg )
  {
    if ( m_DispDlg->IsType( ID_UPCREADER ) ) return;
    m_DispDlg->DestroyWindow();
    delete m_DispDlg;
  }
  m_DispDlg = new CUPCReaderDsp( GetDlgItem( IDC_PLUGINPARENT ) );
  m_DispDlg->Create( GetDlgItem( IDC_PLUGINPARENT ) );
}


void CTunerDlg::OnCode39()
{
  UpdateData( TRUE );
  if ( m_DispDlg )
  {
    if ( m_DispDlg->IsType( ID_CODE39 ) ) return;
    m_DispDlg->DestroyWindow();
    delete m_DispDlg;
  }
  m_DispDlg = new CCode39Reader( GetDlgItem( IDC_PLUGINPARENT ) );
  m_DispDlg->Create( GetDlgItem( IDC_PLUGINPARENT ) );
}

void CTunerDlg::OnCode128()
{
  UpdateData( TRUE );
  if ( m_DispDlg )
  {
    if ( m_DispDlg->IsType( ID_CODE128 ) ) return;
    m_DispDlg->DestroyWindow();
    delete m_DispDlg;
  }
  m_DispDlg = new CCode128Reader( GetDlgItem( IDC_PLUGINPARENT ) );
  m_DispDlg->Create( GetDlgItem( IDC_PLUGINPARENT ) );
}

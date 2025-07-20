// SplashWnd.cpp : implementation file
//

#include "stdafx.h"
#include <security\basesecurity.h>
#include <userinterface\SplashWnd.h>

#define TIME_TO_SHOW 4000
#define TRANSPARENTCOLOR RGB(255, 0, 255);

// CSplashWnd dialog
BOOL         CSplashWnd::c_bShowSplashWnd;
CSplashWnd*  CSplashWnd::c_pSplashWnd;

IMPLEMENT_DYNAMIC( CSplashWnd , CDialog )

CSplashWnd::CSplashWnd( CWnd* pParent /*=NULL*/ )
  : CDialog( CSplashWnd::IDD , pParent ) ,
  m_IsAboutToClose( false )
{
  ASSERT( m_hBitmap != NULL );
}

CSplashWnd::~CSplashWnd()
{
  if ( m_hBitmap ) DeleteObject( m_hBitmap );
  ASSERT( c_pSplashWnd == this );
  c_pSplashWnd = NULL;
}

void CSplashWnd::DoDataExchange( CDataExchange* pDX )
{
  CDialog::DoDataExchange( pDX );
}


BEGIN_MESSAGE_MAP( CSplashWnd , CDialog )
  ON_WM_DESTROY()
  ON_WM_TIMER()
  ON_WM_PAINT()
  ON_WM_SIZE()
END_MESSAGE_MAP()


// CSplashWnd message handlers

void CSplashWnd::EnableSplashScreen( BOOL bEnable )
{
  c_bShowSplashWnd = bEnable;
}

void CSplashWnd::ShowSplashScreen( bool Demo , CWnd* pParentWnd )
{
  if ( !c_bShowSplashWnd || c_pSplashWnd != NULL )
    return;
  // Allocate a new splash screen, and create the window.
  c_pSplashWnd = new CSplashWnd;
  BOOL ret = c_pSplashWnd->Create( IDD , pParentWnd );
  ASSERT( ret != FALSE );
}

void CSplashWnd::WaitEnd()
{
  MSG msg;
  while ( c_pSplashWnd != NULL )
  {
    if ( ::PeekMessage( &msg , NULL , 0 , 0 , PM_REMOVE ) )
    {
      ::TranslateMessage( &msg );
      ::DispatchMessage( &msg );
    }
  }
}

BOOL CSplashWnd::OnInitDialog()
{
  CDialog::OnInitDialog();
  m_hTimer = SetTimer( IDD , TIME_TO_SHOW , NULL );
  CFont* pFont = GetFont();
  LOGFONT lf;
  if ( NULL != pFont )
  {
    pFont->GetLogFont( &lf );
    lf.lfHeight = -MulDiv( 20 , GetDeviceCaps( GetDC()->m_hDC , LOGPIXELSY ) , 72 );
    VERIFY( m_VersionFont.CreateFontIndirect( &lf ) );
    lf.lfHeight = -MulDiv( 12 , GetDeviceCaps( GetDC()->m_hDC , LOGPIXELSY ) , 72 );
    VERIFY( m_LicenseFont.CreateFontIndirect( &lf ) );
  }
  return TRUE;
}

void CSplashWnd::OnDestroy()
{
  if ( m_IsAboutToClose )
  {
    KillTimer( m_hTimer );
    CDialog::OnDestroy();
    delete c_pSplashWnd;
    c_pSplashWnd = NULL;
  }
}

void CSplashWnd::OnTimer( UINT_PTR nIDEvent )
{
  if ( nIDEvent == IDD )
  {
    m_IsAboutToClose = true;
    SendMessage( WM_DESTROY );
  }
  else
    CDialog::OnTimer( nIDEvent );
}

void CSplashWnd::OnPaint()
{
  CPaintDC dc( this );
  CRect textPos , wndRect;
  GetWindowRect( wndRect );
  CString owner;
  CSize textextent;

  GetDlgItem( IDC_TEXTPOS )->GetWindowRect( textPos );
  textPos.OffsetRect( -wndRect.left , -wndRect.top );
  HDC hMemDC = ::CreateCompatibleDC( NULL );
  SelectObject( hMemDC , m_hBitmap );
  ::BitBlt( dc.m_hDC , 0 , 0 , m_Bitmap.bmWidth , m_Bitmap.bmHeight , hMemDC , 0 , 0 , SRCCOPY );
  ::DeleteDC( hMemDC );

  CString tmpS ;
  getowner( owner ) ;
  bool bLocalMachine = true ;
  char cIsRestricted = isevaluation( bLocalMachine ) ;
  if ( bLocalMachine || (cIsRestricted == EOL_Evaluation) )
  {
    CString days;
    days.Format( "%d" , daysremain() );
    tmpS.Format( "Demo version, %s days left" , days );
  }
  else if ( cIsRestricted == EOL_License )
  {
    CString days;
    days.Format( "%d" , daysremain() );
    tmpS.Format( "Licensed to %s , %s days left" , owner , days );
  }
  else // no restriction
  {
    tmpS.Format( "Licensed to: '%s'" , owner );
  }

  if ( bLocalMachine )
    tmpS += " (HKLM)" ;

  dc.SetBkMode( TRANSPARENT );
  COLORREF oldColor = dc.SetTextColor( RGB( 255 , 255 , 255 ) );
  CFont* oldFont = dc.SelectObject( &m_VersionFont );
  textextent = dc.GetTextExtent( "A" , 1 );
  CString fname = CString( AfxGetApp()->m_pszExeName ) + ".exe";
  CString version; version.Format( "Version: %s" , ( LPCTSTR ) FxGetProductVersion( fname ) );
  dc.TextOut( textPos.left , textPos.top , version );

  textPos.OffsetRect( 0 , ( int ) ( textextent.cy * 1 ) );
  dc.SetTextColor( RGB( 251 , 176 , 59 ) );
  dc.SelectObject( &m_LicenseFont );
  dc.TextOut( textPos.left , textPos.top , tmpS );

  dc.SelectObject( oldFont );
  dc.SetTextColor( oldColor );
  CDialog::OnPaint();
}

void CSplashWnd::OnSize( UINT nType , int cx , int cy )
{
  CDialog::OnSize( nType , cx , cy );
  m_hBitmap = LoadBitmap( GetModuleHandle( NULL ) , MAKEINTRESOURCE( IDB_NEWSPLASH ) );
  if ( m_hBitmap == NULL )
  {
    MessageBox( "Error loading bitmap" );
    return;
  }
  GetObject( m_hBitmap , sizeof( m_Bitmap ) , &m_Bitmap );
  CPaintDC dc( this );
  CDC dcMem;
  dcMem.CreateCompatibleDC( &dc );
  CBitmap* pOldBitmap = dcMem.SelectObject( CBitmap::FromHandle( m_hBitmap ) );
  CRgn crRgn , crRgnTmp;
  crRgn.CreateRectRgn( 0 , 0 , 0 , 0 );
  COLORREF crTransparent = TRANSPARENTCOLOR;
  int iX = 0;
  int iY = 0;
  for ( ; iY < m_Bitmap.bmHeight; iY++ )
  {
    do
    {
      while ( iX <= m_Bitmap.bmWidth && dcMem.GetPixel( iX , iY ) == crTransparent )
        iX++;
      int iLeftX = iX;
      while ( iX <= m_Bitmap.bmWidth && dcMem.GetPixel( iX , iY ) != crTransparent )
        ++iX;
      if ( iLeftX != iX )
      {
        crRgnTmp.CreateRectRgn( iLeftX , iY , iX , iY + 1 );
        crRgn.CombineRgn( &crRgn , &crRgnTmp , RGN_OR );
        crRgnTmp.DeleteObject();
      }
    } while ( iX < m_Bitmap.bmWidth );
    iX = 0;
  }
  VERIFY( SetWindowRgn( crRgn , TRUE ) != 0 );
  iX = ( GetSystemMetrics( SM_CXSCREEN ) ) / 2 - ( m_Bitmap.bmWidth / 2 );
  iY = ( GetSystemMetrics( SM_CYSCREEN ) ) / 2 - ( m_Bitmap.bmHeight / 2 );
  SetWindowPos( &wndTopMost , iX , iY , m_Bitmap.bmWidth , m_Bitmap.bmHeight , NULL );

  dcMem.SelectObject( pOldBitmap );
  dcMem.DeleteDC();
  crRgn.DeleteObject();
}

// no close on Enter or Esc
void CSplashWnd::OnOK()
{}

void CSplashWnd::OnCancel()
{}


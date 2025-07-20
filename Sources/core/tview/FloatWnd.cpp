// FloatWnd.cpp : implementation file
//

#include "stdafx.h"
#include <Gadgets\tview.h>
#include "FloatWnd.h"
#include "fxfc/FXRegistry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFloatWnd dialog


CFloatWnd::CFloatWnd( CString name , CWnd* pParent /*=NULL*/ )
  : CDialog( CFloatWnd::IDD , pParent ) ,
  m_pActiveWnd( NULL ) ,
  m_Name( name ) ,
  m_OkToDestroy( FALSE ) ,
  m_ParentWnd( pParent )
{
  //{{AFX_DATA_INIT(CFloatWnd)
  //}}AFX_DATA_INIT
}


void CFloatWnd::DoDataExchange( CDataExchange* pDX )
{
  CDialog::DoDataExchange( pDX );
  //{{AFX_DATA_MAP(CFloatWnd)
  DDX_Control( pDX , IDC_DISP_TAB , m_DispTab );
  //}}AFX_DATA_MAP
}

BOOL CFloatWnd::Create( UINT nIDTemplate , CWnd* pParentWnd )
{
  BOOL res = CDialog::Create( nIDTemplate , pParentWnd );
  WINDOWPLACEMENT wp ;
  DWORD wpSize = sizeof( wp );
  FXRegistry Reg( "TheFileX\\SHStudio" ) ;
  if ( Reg.GetRegiBinary( "wndpos" , m_Name , ( LPBYTE ) &wp , &wpSize )
    && ( wpSize == sizeof( WINDOWPLACEMENT ) ) )
  {
    int iCaptionSize = GetSystemMetrics( SM_CYCAPTION ) ;
    if ( (wp.rcNormalPosition.bottom - wp.rcNormalPosition.top) < iCaptionSize * 2 )
    {
      if ( wp.rcNormalPosition.top < 400 )
        wp.rcNormalPosition.bottom += iCaptionSize * 3 ;
      else
        wp.rcNormalPosition.top -= iCaptionSize * 3 ;
    }
    SetWindowPlacement( &wp );
  }
//   AfxGetApp()->GetProfileBinary( "wndpos" , m_Name , (LPBYTE*) &wp , &wpSize );
//   if ( wp && (wpSize == sizeof( WINDOWPLACEMENT )) )
//   {
//     SetWindowPlacement( wp );
//     RECT rc;
//     GetWindowRect( &rc );
//     HMONITOR hMonitor = MonitorFromRect( &rc , MONITOR_DEFAULTTONULL );
//     if ( !hMonitor )
//       SetWindowPos( NULL , 0 , 0 , 0 , 0 , SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
//   }
//   if ( wp ) delete wp;
  SetWindowText( m_Name );
  return res;
}

void CFloatWnd::OnDestroy()
{
  WINDOWPLACEMENT wp;
  GetWindowPlacement( &wp );
  FXRegistry Reg( "TheFileX\\SHStudio" ) ;
  Reg.WriteRegiBinary( "wndpos" , m_Name , ( LPBYTE ) &wp , sizeof( wp ) );
  CDialog::OnDestroy();
}

BOOL CFloatWnd::DestroyChannel( LPCTSTR uid )
{
  CWnd* pWnd = NULL;
  if ( m_Channels.Lookup( uid , (LPVOID&) pWnd ) && pWnd )
  {
    CString key;
    m_Channels.RemoveKey( uid );
    m_DispTab.DeleteItem( SeekTabItem( uid ) );
    if ( pWnd == m_pActiveWnd )
    {
      m_pActiveWnd = NULL;
      POSITION pos = m_Channels.GetStartPosition();
      while ( pos )
      {
        m_Channels.GetNextAssoc( pos , key , (LPVOID&) pWnd );
        if ( pWnd && SetActiveChannel( key ) )
          break;
      }
    }
    return TRUE;
  }
  else
    return FALSE;
}
BOOL CFloatWnd::RenameChannel( LPCTSTR OldUID , LPCTSTR NewUID )
{
  BOOL bRes = FALSE ;
  int iItemNum = SeekTabItem( OldUID ) ;
  if ( iItemNum >= 0 )
  {
    bRes = SetTabItemName( iItemNum , NewUID ) ;
    if ( m_MainViewed == OldUID )
      m_MainViewed = NewUID ;
    SetActiveChannel( m_MainViewed , TRUE ) ;

  }
  return bRes ;
}

void CFloatWnd::InsertChannel(LPCTSTR uid, CWnd* pGlyphFrame)
{
  DestroyChannel(uid);
  m_Channels.SetAt(uid, pGlyphFrame);
  m_DispTab.InsertItem((int)m_Channels.GetCount(), uid);
  pGlyphFrame->SetParent(&m_DispTab);
  bool bActive = ((m_Channels.GetCount() == 1) && (m_MainViewed.IsEmpty()))
    || (m_MainViewed == uid) ;
  if ( bActive )
    SetActiveChannel(uid , ( m_MainViewed == uid ) );
  else
    ShowChannel(uid, FALSE);
  CorrectSize();
}

BOOL CFloatWnd::SetActiveChannel( LPCTSTR uid , bool bMainViewed )
{
  if ( m_pActiveWnd )
    m_pActiveWnd->ShowWindow( SW_HIDE );
  CWnd* pWnd = NULL;
  if ( m_Channels.Lookup( uid , (LPVOID&) pWnd ) && (pWnd) && ::IsWindow( pWnd->m_hWnd ) )
  {
    TRACE( "Activate window %s\n" , m_MainViewed );
    if ( bMainViewed )
      m_MainViewed = uid ;
    m_pActiveWnd = pWnd;
    m_pActiveWnd->ShowWindow( SW_SHOW );
    if ( SeekTabItem( m_MainViewed ) >= 0 ) 
    {
      m_DispTab.SetCurSel( SeekTabItem( m_MainViewed ) );
      if ( (m_pActiveWnd) && (m_ParentWnd) )
        m_ParentWnd->SendMessage( UM_FLTWND_CHANGED , (WPARAM) this , (LPARAM) ((LPCTSTR) m_Name) );
    }
    return TRUE;
  }
  else
  {
    if ( m_Channels.GetSize() )
    {
      POSITION Pos = m_Channels.GetStartPosition() ;
      CString FirstName ;
      m_Channels.GetNextAssoc( Pos , FirstName , (LPVOID&) pWnd ) ;
      if ( bMainViewed )
        m_MainViewed = uid ;
      m_pActiveWnd = pWnd;
      m_pActiveWnd->ShowWindow( SW_SHOW );
      if ( SeekTabItem( m_MainViewed ) >= 0 )
      {
        m_DispTab.SetCurSel( SeekTabItem( m_MainViewed ) );
        if ( (m_pActiveWnd) && (m_ParentWnd) )
          m_ParentWnd->SendMessage( UM_FLTWND_CHANGED , (WPARAM) this , (LPARAM) ((LPCTSTR) m_Name) );
      }
    }
  }
  return FALSE;
}

CWnd* CFloatWnd::ShowChannel( LPCTSTR uid , BOOL Show )
{
  CWnd* pWnd = NULL;
  if ( m_Channels.Lookup( uid , (LPVOID&) pWnd ) && (pWnd) && ::IsWindow( pWnd->m_hWnd ) )
  {
    pWnd->ShowWindow( (Show) ? SW_SHOW : SW_HIDE );
    return pWnd;
  }
  return NULL;
}

BOOL CFloatWnd::IsEmpty()
{
  return (m_Channels.GetCount() == 0);
}

BEGIN_MESSAGE_MAP( CFloatWnd , CDialog )
  //{{AFX_MSG_MAP(CFloatWnd)
  ON_WM_SIZE()
  ON_NOTIFY( TCN_SELCHANGE , IDC_DISP_TAB , OnSelchangeDispTab )
  ON_WM_DESTROY()
  ON_WM_CLOSE()
  //}}AFX_MSG_MAP
  ON_WM_MOVE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFloatWnd message handlers

void CFloatWnd::CorrectSize()
{
  CRect rect;
  if ( m_DispTab )
  {
    GetClientRect( rect );
    m_DispTab.MoveWindow( &rect , TRUE );
    m_DispTab.AdjustRect( FALSE , &rect );

    POSITION pos = m_Channels.GetStartPosition();
    while ( pos )
    {
      CString key;
      CWnd* pWnd;
      m_Channels.GetNextAssoc( pos , key , (LPVOID&) pWnd );
      if ( pWnd && ::IsWindow( pWnd->m_hWnd ) )
        pWnd->MoveWindow( rect , (pWnd == m_pActiveWnd) );
    }
  }
}

void CFloatWnd::OnSelchangeDispTab( NMHDR* pNMHDR , LRESULT* pResult )
{
  CString uid = GetCurrentTabSelection();
  if ( uid.GetLength() )
    SetActiveChannel( uid , true );
  *pResult = 0;
}

CString CFloatWnd::GetCurrentTabSelection()
{
  return GetTabItemName( m_DispTab.GetCurSel() );
}

CString CFloatWnd::GetTabItemName( int n )
{
#define MAXSTRSIZE 1024

  CString tmpS;
  if ( n != -1 )
  {
    TCITEM item;
    memset( &item , 0 , sizeof( item ) );
    item.mask = TCIF_TEXT;
    item.pszText = tmpS.GetBuffer( MAXSTRSIZE );
    item.cchTextMax = MAXSTRSIZE;
    m_DispTab.GetItem( n , &item );
    tmpS.ReleaseBuffer();
  }
  return tmpS;
}

BOOL CFloatWnd::SetTabItemName( int n , LPCTSTR pNewName )
{
#define MAXSTRSIZE 1024

  if ( (0 <= n) && (n < m_DispTab.GetItemCount()) )
  {
    CString tmpS;
    TCITEM item;
    memset( &item , 0 , sizeof( item ) );
    item.mask = TCIF_TEXT;
    item.pszText = tmpS.GetBuffer( MAXSTRSIZE );
    item.cchTextMax = MAXSTRSIZE;
    m_DispTab.GetItem( n , &item );
    strcpy_s( item.pszText , MAXSTRSIZE , pNewName ) ;
    m_DispTab.SetItem( n , &item ) ;
    tmpS.ReleaseBuffer();
    return TRUE ;
  }
  return FALSE;
}

int CFloatWnd::SeekTabItem( LPCTSTR name )
{
  if ( !m_DispTab ) return -1;
  int i = 0;
  while ( i < m_DispTab.GetItemCount() )
  {
    if ( GetTabItemName( i ) == name ) 
      break;
    i++;
  }
  if ( i == m_DispTab.GetItemCount() ) 
    return -1;
  return i;
}

void CFloatWnd::OnClose()
{
  if ( !m_OkToDestroy ) return;
  CDialog::OnClose();
}

void CFloatWnd::OnOK()
{}

void CFloatWnd::OnCancel()
{}

BOOL CFloatWnd::PreTranslateMessage( MSG* pMsg )
{
  if ( pMsg->message == WM_SYSKEYDOWN )
  {
    if ( GetKeyState( VK_MENU ) < 0 )
    {
      AfxGetApp()->GetMainWnd()->SetActiveWindow();
    }
  }
  return CDialog::PreTranslateMessage( pMsg );
}

void CFloatWnd::OnMove( int x , int y )
{
  CDialog::OnMove( x , y );
  if ( (m_pActiveWnd) && (m_ParentWnd) )
    m_ParentWnd->SendMessage( UM_FLTWND_CHANGED , (WPARAM) this , (LPARAM) ((LPCTSTR) m_Name) );
}

void CFloatWnd::OnSize( UINT nType , int cx , int cy )
{
  CDialog::OnSize( nType , cx , cy );
  CorrectSize();
  if ( (m_pActiveWnd) && (m_ParentWnd) )
    m_ParentWnd->SendMessage( UM_FLTWND_CHANGED , (WPARAM) this , (LPARAM) ((LPCTSTR) m_Name) );
}

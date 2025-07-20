// LogView.cpp : implementation file
//

#include "stdafx.h"
#include "tvdb400.h"
#include "logview.h"
#include <gadgets\shkernel.h>
#include <fxfc\/FXRegistry.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

LPCTSTR ColumnNames[] = {"Time", "Level", "Source","Message"};
LPCTSTR ColumnMessage[] = {"07/21/08 12:24:39", "Level", "IGraphbuilder.Load","Message"};
const int ColumnsNmb = sizeof( ColumnNames ) / sizeof( char* );

/////////////////////////////////////////////////////////////////////////////
// CLogView

CLogView::CLogView() :
  m_ColumnsSumWidth( 0 ) ,
  m_LogMsgQueue( 4000 ) ,
  m_Level( 0 ) ,
  m_WriteInFile( false )
{
  FXRegistry Reg( "TheFileX\\SHStudio" ) ;
  m_Level = Reg.GetRegiInt( "LogView" , "level" , 0 );
}

CLogView::~CLogView()
{
  FXRegistry Reg( "TheFileX\\SHStudio" ) ;
  Reg.WriteRegiInt( "LogView" , "level" , m_Level );
  m_PrintLock.Lock();
  m_ListCtrl.Detach();
  m_PrintLock.Unlock();
  if ( m_File.m_hFile != CFile::hFileNull )
    m_File.Close();
}


BEGIN_MESSAGE_MAP( CLogView , CWorkspaceBar )
  //{{AFX_MSG_MAP(CLogView)
  ON_WM_TIMER()
  ON_WM_DESTROY()
  ON_WM_SIZE()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLogView message handlers

BOOL CLogView::Create( CWnd* pParentWnd )
{

  BOOL res = CWorkspaceBar::Create( pParentWnd , "Log view" , WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC | CBRS_GRIPPER , LOG_BAR_ID );
  if ( !res )
    return FALSE;
  DWORD dwExStyle = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES /*| LVS_EX_SUBITEMIMAGES |*/
    /* LVS_EX_HEADERDRAGDROP | LVS_EX_TRACKSELECT*/;

  CRect rect;
  GetClientRect( rect );
  m_ListCtrl.Create( WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT , rect , this , IDC_LOGVIEW );
  m_ListCtrl.SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE , 0 , LPARAM( dwExStyle ) );
  m_ColumnsSumWidth = 0;
  for ( int i = 0; i < ColumnsNmb; i++ )
  {
    int w = m_ListCtrl.GetStringWidth( ColumnMessage[ i ] ) + 20;
    if ( (i + 1) < ColumnsNmb )
      m_ColumnsSumWidth += w;
    m_ListCtrl.InsertColumn( i , ColumnNames[ i ] , LVCFMT_LEFT , w );
  }
  m_PrintTimer = SetTimer( IDD_LOGVIEWDLG , 100 , NULL );
  m_pSite = &m_ListCtrl;
#ifdef TVDB_WORKSPACE_AS_REBAR
  CReBar::AddBar( m_pSite );
#endif
  return res;
}

void CLogView::OnDestroy()
{
  KillTimer( m_PrintTimer );
  while ( m_LogMsgQueue.ItemsInQueue() )
  {
    CLogMsg* lm;
    if ( m_LogMsgQueue.GetQueueObject( lm ) )
    {
      TRACE( "+++ Warrning! The messages remains in LogMsgQueue: '%s'\n" , lm->msgText );
      delete lm;
    }
  }
  CWorkspaceBar::OnDestroy();
}

void CLogView::PrintMsg( int msgLevel , LPCTSTR src , int msgId , LPCTSTR  msgText )
{
  CLogMsg* lm = new CLogMsg( msgLevel , src , msgId , msgText );
  if ( !m_LogMsgQueue.PutQueueObject( lm ) )
  {
    TRACE( "!!!! LOGMSG QUEUE IS FULL!\n" );
    TRACE( "\tORIGINAL MESSAGE WAS TVDB400 ALARM LOG >> Level=%d, src=%s ID=%d, text=\"%s\"\n" , msgLevel , src , msgId , msgText );
    delete lm;
  }
}

void CLogView::OnTimer( UINT_PTR nIDEvent )
{
  if ( m_WriteInFile )
  {
    if ( m_File.m_hFile == CFile::hFileNull )
    {
      CFileException e;
      if ( !m_File.Open( m_LogFileName ,
        CFile::modeCreate | CFile::modeNoTruncate | CFile::typeText
        | CFile::modeWrite | CFile::shareDenyWrite | CFile::osWriteThrough , &e ) )
      {
        TCHAR szError[ 1024 ];
        e.GetErrorMessage( szError , 1024 );

        PrintMsg( 7 , "LogView" , 0 , szError );
        m_WriteInFile = false;
      }
      else
      {
        m_File.SeekToEnd();
      }
    }
  }
  else
  {
    if ( m_File.m_hFile != CFile::hFileNull )
      m_File.Close();
  }
  if ( nIDEvent == IDD_LOGVIEWDLG )
  {
    if ( (::IsWindow( m_hWnd )) && (GetDlgItem( IDC_LOGVIEW )) )
    {
      if ( m_ColumnsSumWidth )
      {
        CLogMsg* lm;
        while ( m_LogMsgQueue.ItemsInQueue() )
        {
          if ( m_LogMsgQueue.GetQueueObject( lm ) && (lm) )
          {
            if ( lm->msgLevel >= m_Level )
            {
              WriteLogMessage( lm );
              int i = m_ListCtrl.GetItemCount();

              FILETIME ltime;
              SYSTEMTIME stime;
              FileTimeToLocalFileTime(&(lm->TimeStampAsFileTime), &ltime);//convert to local time and store in ltime
              FileTimeToSystemTime(&ltime, &stime);//convert in system time and store in stime

              CString TimeAsString;

              TimeAsString.Format("%02d%02d_%02d:%02d:%02d.%03d",
                stime.wMonth,stime.wDay, stime.wHour, stime.wMinute, 
                stime.wSecond, stime.wMilliseconds);

              m_ListCtrl.InsertItem( i , TimeAsString);
              CString tmpS; 
              tmpS.Format( "%d" , lm->msgLevel );
              m_ListCtrl.SetItemText( i , 1 , tmpS );
              //tmpS; tmpS.Format("%d",lm.m_Source);
              m_ListCtrl.SetItemText( i , 2 , lm->m_Source );
              m_ListCtrl.SetItemText( i , 3 , lm->msgText );
              m_ListCtrl.EnsureVisible( i , false );
              if ( lm->msgLevel >= MSG_CRITICAL_LEVEL )
              {
                KillTimer( m_PrintTimer );
                AfxMessageBox( lm->msgText );
                m_PrintTimer = SetTimer( IDD_LOGVIEWDLG , 100 , NULL );
              }
            }
            delete lm;
          }
        }
      }
    }
  }
  CWorkspaceBar::OnTimer( nIDEvent );
}

void CLogView::OnSize( UINT nType , int cx , int cy )
{
  CWorkspaceBar::OnSize( nType , cx , cy );
  if ( !::IsWindow( m_ListCtrl.GetSafeHwnd() ) )
    return;
  CRect rc;
  m_ListCtrl.GetClientRect( rc );
  int wLastCol = rc.Width() - m_ColumnsSumWidth;
  if ( wLastCol == 0 )
    wLastCol = 20;
  LVCOLUMN col;
  col.mask = LVCF_WIDTH;
  col.cx = wLastCol;
  m_ListCtrl.SetColumn( 3 , &col );
}

void CLogView::SetLogFileName( LPCTSTR name )
{
  if ( (m_LogFileName != name) && (m_File.m_hFile != CFile::hFileNull) )
    m_File.Close();
  if ( ::FxIsFullPath( name ) )
    m_LogFileName = name;
  else
    m_LogFileName = ::FxGetAppPath() + _T( '\\' ) + name;
  m_LogFileName = ::FxFullPath( m_LogFileName );
}

bool CLogView::WriteLogMessage( CLogMsg *lm )
{
  if ( m_File.m_hFile == CFile::hFileNull ) 
    return false;

  FILETIME ltime;
  SYSTEMTIME stime;
  FileTimeToLocalFileTime( &(lm->TimeStampAsFileTime) , &ltime);//convert to local time and store in ltime
  FileTimeToSystemTime( &ltime , &stime );//convert in system time and store in stime

  CString message;

  message.Format( "%04d%02d%02d_%02d:%02d:%02d.%03d\t%d\t%s\t%s\n" , 
    stime.wYear , stime.wMonth, stime.wDay,
    stime.wHour, stime.wMinute, stime.wSecond, stime.wMilliseconds,
    lm->msgLevel, lm->m_Source, lm->msgText);

  m_File.WriteString( message );
  m_File.Flush() ;

  return true;
}

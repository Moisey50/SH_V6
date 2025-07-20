// FloatWndManager.cpp: implementation of the CFloatWndManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FloatWnd.h"
#include "FloatWndManager.h"
#include "sketchview.h"
#include <fxfc/CSystemMonitorsEnumerator.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFloatWndManager::CFloatWndManager()
{

}

CFloatWndManager::~CFloatWndManager()
{
  while ( m_Monitors.GetSize() )
  {
    CFloatWnd* pWnd = (CFloatWnd*) m_Monitors.GetAt( 0 );
    m_Monitors.RemoveAt( 0 );
    pWnd->DestroyWindow();
    delete pWnd;
  }
}

void CFloatWndManager::EnumMonitors( CStringArray& Names )
{
  Names.RemoveAll();
  for ( int i = 0; i < m_Monitors.GetSize(); i++ )
  {
    CFloatWnd* pWnd = (CFloatWnd*) m_Monitors.GetAt( i );
    Names.Add( pWnd->GetName() );
  }
}

CFloatWnd* CFloatWndManager::CreateChannel( CSketchView* pSketchView , CString& monitor , LPCTSTR uid , CRect& rect )
{
  CFloatWnd* pWnd = (CFloatWnd*) FindMonitor( monitor );
  if ( !pWnd )
  {
    monitor = ((monitor.IsEmpty()) ? GenerateNewMonitorName() : monitor);
    pWnd = new CFloatWnd( monitor , pSketchView );
    if ( !pWnd->Create( IDD_FLOAT_WND , pSketchView ) )
    {
      delete pWnd;
      return NULL;
    }
    int cMonitors = (int) m_Monitors.GetSize();
    m_Monitors.Add( pWnd );
    CFloatWnd* PrevMonitor = ((cMonitors) ? (CFloatWnd*) m_Monitors.GetAt( cMonitors - 1 ) : NULL);
    double x , y , w , h;
    FXString SelectedPane ;
    if ( pSketchView->GetBuilder()->GetFloatWnd( monitor , x , y , w , h , &SelectedPane ) )
    {
      const CSystemMonitorsEnumerator * pMonitorsEnum =
        CSystemMonitorsEnumerator::GetMonitorEnums() ;
      CRect rcDesktop = pMonitorsEnum->m_FullDesktopRect , rc ;
      if ( (fabs( w ) <= 1.) && (fabs( h ) <= 1.) ) // Normalized coordinates
      {
        rc.left = ( int ) ( ( double ) rcDesktop.Width() * x ) + rcDesktop.left ;
        rc.top = ( int ) ( ( double ) rcDesktop.Height() * y ) + rcDesktop.top ;
        rc.right = rc.left + ( int ) ( ( double ) rcDesktop.Width() * w );
        rc.bottom = rc.top + ( int ) ( ( double ) rcDesktop.Height() * h );
      }
      else
        rc = CRect( CPoint(( int ) x , (int) y) , CSize( (int)w , (int) h ) ) ;

      if ( !rcDesktop.PtInRect( rc.TopLeft() ) )
      {
        rc = CRect( rcDesktop.TopLeft() + CSize( 50 , 50 ) , CSize( 200 , 200 ) ) ;
      }

      pWnd->SetWindowPos( NULL , rc.left , rc.top , rc.Width() , rc.Height() , SWP_NOACTIVATE | SWP_NOZORDER );
      pWnd->SetActiveChannel( SelectedPane , ( SelectedPane == uid ) ) ;
    }
  }
  else
  {
    double x , y , w , h;
    FXString SelectedPane ;
    if ( pSketchView->GetBuilder()->GetFloatWnd( monitor , x , y , w , h , &SelectedPane ) )
    {
      if ( SelectedPane == uid )
        pWnd->SetActiveChannel( SelectedPane , true ) ;
    }
  }
  return pWnd;
}

void CFloatWndManager::DestroyChannel( LPCTSTR uid )
{
  for ( int i = 0; i < m_Monitors.GetSize(); i++ )
  {
    CFloatWnd* Monitor = (CFloatWnd*) m_Monitors.GetAt( i );
    if ( (Monitor->GetSafeHwnd()) && (Monitor->DestroyChannel( uid )) )
    {
      if ( Monitor->IsEmpty() )
      {
        m_Monitors.RemoveAt( i );
        Monitor->DestroyWindow();
        delete Monitor;
      }
      return;
    }
  }
}
CString CFloatWndManager::RenameChannel( LPCTSTR pOldChannel , LPCTSTR pNewChannel )
{
  for ( int i = 0; i < m_Monitors.GetSize(); i++ )
  {
    CFloatWnd* Monitor = (CFloatWnd*) m_Monitors.GetAt( i );
    if ( Monitor->RenameChannel( pOldChannel , pNewChannel ) )
      return Monitor->GetName() ;
  }
  return CString() ;
}

void CFloatWndManager::SetChannelFrame( LPCTSTR monitor , LPCTSTR uid , CWnd* pGlyphFrame )
{
  CFloatWnd* pWnd = (CFloatWnd*) FindMonitor( monitor );
  ASSERT( pWnd );
  pWnd->InsertChannel( uid , pGlyphFrame );
}

bool CFloatWndManager::PtInside( CPoint& pt )
{
  CRect rc;
  for ( int i = 0; i < m_Monitors.GetSize(); i++ )
  {
    CWnd* Monitor = (CWnd*) m_Monitors.GetAt( i );
    Monitor->GetWindowRect( rc );
    if ( rc.PtInRect( pt ) )
      return true;
  }
  return false;
}

// --- private --- //

CFloatWnd* CFloatWndManager::FindMonitor( LPCTSTR name )
{
  for ( int i = 0; i < m_Monitors.GetSize(); i++ )
  {
    CFloatWnd* pWnd = (CFloatWnd*) m_Monitors.GetAt( i );
    CString title = pWnd->GetName();
    if ( !title.Compare( name ) )
      return pWnd;
  }
  return NULL;
}

static int MonitorsCreated = 0;
CString CFloatWndManager::GenerateNewMonitorName()
{
  CString name;
  int i = 0;
  do
  {
    name.Format( "Float window %d" , ++i );
  } while ( FindMonitor( name ) );
  return name;
}


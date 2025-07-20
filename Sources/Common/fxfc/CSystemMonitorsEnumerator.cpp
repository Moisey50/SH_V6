#include "stdafx.h"
#include "CSystemMonitorsEnumerator.h"

CSystemMonitorsEnumerator CSystemMonitorsEnumerator::g_SysMonitorsEnum ;

BOOL CALLBACK MyInfoEnumProc( 
  HMONITOR hMonitor , HDC hdcMonitor , LPRECT lprcMonitor , LPARAM dwData )
{
  MONITORINFOEX MonitorInfo;
  MonitorInfo.cbSize = sizeof( MONITORINFOEX );
  if ( GetMonitorInfo( hMonitor , &MonitorInfo ) )
  {
    if ( MonitorInfo.dwFlags == DISPLAY_DEVICE_MIRRORING_DRIVER )
      return true;
    CSystemMonitorsEnumerator * pClass = (CSystemMonitorsEnumerator *) dwData ;
    pClass->m_monitorArray.push_back( MonitorInfo );
    CRect MonRect( MonitorInfo.rcMonitor ) ;
    CRect FullRectNow = pClass->m_FullDesktopRect ;
    pClass->m_rcMonitors.push_back( MonRect ) ;
    pClass->m_FullDesktopRect.UnionRect( &MonRect , &FullRectNow ) ;
    return true;
  }
  return false ;
}

CSystemMonitorsEnumerator::CSystemMonitorsEnumerator()
  :
  m_FullDesktopRect( CRect(0,0,0,0) )
{
  
  ::EnumDisplayMonitors( NULL , NULL , &MyInfoEnumProc , (LPARAM)this );
}

const MONITORINFOEX * CSystemMonitorsEnumerator::GetMonitorInfo( size_t iMonitor )
{
  if ( iMonitor < m_monitorArray.size() )
    return &(m_monitorArray[ iMonitor ]) ;
  return NULL ;
};

CRect CSystemMonitorsEnumerator::GetMonitorRect( size_t iMonitor ) const
{
  if ( iMonitor >= m_rcMonitors.size() )
    iMonitor = m_rcMonitors.size() - 1 ;
  if ( iMonitor < m_rcMonitors.size() )
    return m_rcMonitors[ iMonitor ] ;
  return CRect( 0 , 0 , 0 , 0 ) ;
};



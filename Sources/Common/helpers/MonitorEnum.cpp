#include "stdafx.h"
#include "MonitorEnum.h"


BOOL CALLBACK MonitorEnumProc(
  HMONITOR hMonitor ,
  HDC hDC ,
  LPRECT pRect ,
  LPARAM Par
)
{
  Monitors * pMonitors = (Monitors*) Par ;
  MONITORINFO info;
  info.cbSize = sizeof( info );
  if ( GetMonitorInfo( hMonitor , &info ) )
  {
    MonitorData NewMonitor( hMonitor , hDC , *pRect , info.dwFlags ) ;
    pMonitors->push_back( NewMonitor ) ;
  }
  return TRUE ;
}



CMonitorEnum::CMonitorEnum()
{
  m_Monitors.clear() ;
  EnumDisplayMonitors( NULL , NULL , MonitorEnumProc , (LPARAM) &m_Monitors ) ;
}


CMonitorEnum::~CMonitorEnum()
{}

int CMonitorEnum::GetMonitorForPt( CPoint Pt )
{
  for ( int i = 0 ; i < (int)m_Monitors.size() ; i++ )
  {
    if ( m_Monitors[ i ].m_MonitorRect.PtInRect( Pt ) )
      return i ;
  }
  return -1 ;
}

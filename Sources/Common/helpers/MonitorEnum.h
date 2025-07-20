#pragma once

#include <vector>

class MonitorData
{

public:
  MonitorData( HMONITOR hMon = NULL , HDC hMonDC = NULL ,
    CRect MonRect = CRect( 0 , 0 , 0 , 0 ) , DWORD dwFlags = 0 )
  {
    m_hMonitor = hMon ;
    m_hMonitorDC = hMonDC ;
    m_MonitorRect = MonRect ;
    m_Flags = dwFlags ;
  }
  HMONITOR m_hMonitor ;
  HDC      m_hMonitorDC ;
  CRect    m_MonitorRect ;
  DWORD    m_Flags ;
};

typedef std::vector<MonitorData> Monitors ;

class CMonitorEnum
{
public:
  CMonitorEnum();
  ~CMonitorEnum();

  int GetMonitorForPt( CPoint Pt ) ;
  inline int GetNumberOfMonitors()
  {
    return (int)m_Monitors.size(); 
  }

  inline CRect GetMonitorRect( int iIndex )
  {
    if ( iIndex < 0 || iIndex >= (int)m_Monitors.size() )
      iIndex = 0 ;
    return m_Monitors[ iIndex ].m_MonitorRect ;
  }
  inline int GetMonitorIndex( HMONITOR hMon )
  {
    for ( int i = 0 ; i < (int) m_Monitors.size() ; i++ )
    {
      if ( hMon == m_Monitors[ i ].m_hMonitor )
        return i ;
    }
    return -1 ;
  }

  Monitors m_Monitors ;
};


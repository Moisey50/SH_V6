#pragma once

#include <fxfc/fxfc.h>
#include <vector>

struct ScreenArrayInfo
{
  MONITORINFOEX *Array;
  int Count;
  int MaxCount;
};

BOOL CALLBACK MyInfoEnumProc(
  HMONITOR hMonitor , HDC hdcMonitor , LPRECT lprcMonitor , LPARAM dwData ) ;

class FXFC_EXPORT CSystemMonitorsEnumerator
{
  friend  BOOL CALLBACK MyInfoEnumProc( HMONITOR hMonitor , HDC hdcMonitor , 
    LPRECT lprcMonitor , LPARAM dwData ) ;
  static CSystemMonitorsEnumerator g_SysMonitorsEnum ;
public:
  std::vector<CRect>   m_rcMonitors;
  std::vector<MONITORINFOEX> m_monitorArray;
  CRect                m_FullDesktopRect ;
  CSystemMonitorsEnumerator() ;

  const MONITORINFOEX * GetMonitorInfo( size_t iMonitor ) ;
  CRect GetMonitorRect ( size_t iMonitor ) const ;
  CRect GetFullDesktopRectangle() const { return m_FullDesktopRect ; } ;
  static const CSystemMonitorsEnumerator * GetMonitorEnums() { return &g_SysMonitorsEnum ; } ;
};



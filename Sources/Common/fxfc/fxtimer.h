// Timer.h: interface for the FXTimer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TIMER_H__884CEA60_4837_44F3_94B9_89C0B10DDF90__INCLUDED_)
#define AFX_TIMER_H__884CEA60_4837_44F3_94B9_89C0B10DDF90__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

extern double pf; // stored in ctimerstatics.cpp


inline __int64 get_int_time()
{
  LARGE_INTEGER pc;
  QueryPerformanceCounter( &pc );
  return pc.QuadPart ;
}

class FXFC_EXPORT FXTimer
{
private:
  volatile bool m_Alarm;
  HANDLE	 m_hAlarmThread;		// the main thread
  HANDLE   m_hExit;
  DWORD    m_Interval;
  FXString m_Name;
  __int64  m_StartTimeID;
  double   m_BaseFreq;
  HANDLE	 m_evDelay;
  HANDLE	 m_evResetPause;
  HANDLE   m_hTimer ;
  volatile bool m_bPause;
  /// main waiting thread
protected:
  int      AlarmLoop();
public:
  FXTimer( LPCTSTR Name );
  virtual ~FXTimer();
  void     SetAlarmFreq( double Freq );
  void	 AlarmOff();
  void	 AlarmOn();
  bool	 AlarmState() { return m_Alarm; };
  double   GetAlarmFreq() { return 1000. / ( double )m_Interval; };
  bool     Pause( DWORD delay );
  void     ResetPause();
  virtual void     OnAlarm( DWORD TimeID );
  //////////////////////////////////////////////////////////
  friend DWORD WINAPI Alarm_ProcFunc( void* pThis );
};

double FXFC_EXPORT GetHRTickCount();

#endif // !defined(AFX_TIMER_H__884CEA60_4837_44F3_94B9_89C0B10DDF90__INCLUDED_)

// Averager.h: interface for the CAverager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AVERAGER_H__6016A8B6_E552_4EC0_9D49_B406300A6E43__INCLUDED_)
#define AFX_AVERAGER_H__6016A8B6_E552_4EC0_9D49_B406300A6E43__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <video\tvframe.h>

class CAverager  
{
  FXLockObject m_Lock;
  int          m_Mode;
  pTVFrame     m_pFrame;
  DWORD*       m_pData;
  LPBYTE       m_SliderBuffer;
  int          m_AvgVal;
  int          m_cFrames;
  int          m_FramesRange;
  DWORD        m_WrkFormat;
  CSize        m_FrameSize;
  int          m_iLastMin ;
  int          m_iLastMax ;
  int          m_iAllowedMax ;
  bool         m_bAllowedMaxChanged ;
  int          m_iAddMax ;
  int          m_iAddCnt ;
public:
  enum { 
    AVG_INFINITE_UNIFORM, 
    AVG_INFINITE_PASTFADING, 
    AVG_SLIDEWINDOW, 
    AVG_ADD_AND_NORMALIZE,
    AVG_SIMPLE_ADD,
    AVG_AFTER_LAST
  };

  CAverager(int mode = AVG_SLIDEWINDOW);
  virtual ~CAverager();

  pTVFrame GetAvgFrame();
  int GetAvgValue() const { return m_AvgVal; };
  int GetMode() const { return m_Mode; };
  int GetAllowedMax() { return m_iAllowedMax ; }
  void SetAllowedMax( int iNewMax ) { m_iAllowedMax = iNewMax ; }
  void AllowedMaxChanged() { m_bAllowedMaxChanged = true ; }
  int GetAllowedAddCnt() { return m_iAddMax ; }
  void SetAllowedAddCnt( int iNewMax ) { m_iAddMax = iNewMax ; m_iAddCnt = 0 ; }
  void SetMode( int mode );
  int GetFramesRange() const { return m_FramesRange; };
  void SetFramesRange(int range);
  LPCTSTR GetModeName(int i);
  void Reset( int iAverageFactor = 0 );
  void AddFrame(pTVFrame Frame);
};

#endif // !defined(AFX_AVERAGER_H__6016A8B6_E552_4EC0_9D49_B406300A6E43__INCLUDED_)

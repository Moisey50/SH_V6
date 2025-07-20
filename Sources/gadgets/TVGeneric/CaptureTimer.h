// CaptureTimer.h: interface for the CaptureTimer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAPTURETIMER_H__4C3A0D49_FA63_481E_8749_FCA073F69D51__INCLUDED_)
#define AFX_CAPTURETIMER_H__4C3A0D49_FA63_481E_8749_FCA073F69D51__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Gadgets\gadbase.h>

#ifndef ROUND
#define ROUND(x) ((int)(x+0.5))
#endif
class CaptureTimer : public CCaptureGadget  
{
	int m_FrameRate;
  double m_dFramePeriod ;
  int  m_NSendOnStart;
  DWORD  m_NSentFrames;
protected:
	CaptureTimer();
public:
	virtual void ShutDown();
	virtual bool PrintProperties(FXString& text);
	virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
    virtual bool ScanSettings(FXString& text);
private:
    virtual CDataFrame* GetNextFrame(double* StartTime);
	bool InRunningMode();
	friend class CaptureTimerDialog;
	DECLARE_RUNTIME_GADGET(CaptureTimer);
};

#endif // !defined(AFX_CAPTURETIMER_H__4C3A0D49_FA63_481E_8749_FCA073F69D51__INCLUDED_)

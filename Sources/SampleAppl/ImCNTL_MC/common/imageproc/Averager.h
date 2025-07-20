// Averager.h: interface for the CAverager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AVERAGER_H__6016A8B6_E552_4EC0_9D49_B406300A6E43__INCLUDED_)
#define AFX_AVERAGER_H__6016A8B6_E552_4EC0_9D49_B406300A6E43__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <video\tvframe.h>
#include <helpers\lockobject.h>

class CAverager  
{
    CLockObject m_Lock;
	int         m_Mode;
	pTVFrame    m_pFrame;
    DWORD*      m_pData;
    LPBYTE      m_SliderBuffer;
	int         m_AvgVal;
	int         m_cFrames;
	int         m_FramesRange;
    DWORD       m_WrkFormat;
public:
	enum { AVG_INFINITE_UNIFORM, AVG_INFINITE_PASTFADING, AVG_SLIDEWINDOW, };

	CAverager(int mode = AVG_INFINITE_UNIFORM);
	virtual ~CAverager();

	pTVFrame GetAvgFrame();
	int GetAvgValue() const { return m_AvgVal; };
	int GetMode() const { return m_Mode; };
	void SetMode(int mode); 
	int GetFramesRange() const { return m_FramesRange; };
	void SetFramesRange(int range);
	LPCTSTR GetModeName(int i);
	void Reset();
	void AddFrame(pTVFrame Frame);
};

#endif // !defined(AFX_AVERAGER_H__6016A8B6_E552_4EC0_9D49_B406300A6E43__INCLUDED_)

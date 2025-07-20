//  $File : MotionDetector.h: interface for the CMotionDetector class.
//  (C) Copyright HomeBuilt Group and File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)

#if !defined(AFX_MOTIONDETECTOR_H__CC699F21_F3B2_11D5_B0F2_000001360793__INCLUDED_)
#define AFX_MOTIONDETECTOR_H__CC699F21_F3B2_11D5_B0F2_000001360793__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <video\TVFrame.h>
#include <imageproc\averager.h>

class CMotionDetector  
{
private:
    CAverager      m_Averager;
    pTVFrame       m_FramePrev;
    pTVFrame       m_FrameLast;
    double         m_Threshold;
public:
	         CMotionDetector();
	         ~CMotionDetector();
    void     SetThreshold(double val) { m_Threshold=val;};
    void     ResetDetector()                  { if (m_FramePrev) freeTVFrame(m_FramePrev); m_FramePrev=NULL; m_Averager.Reset(); };
    double   GetThreshold()           { return m_Threshold;};
    pTVFrame GetFrameStored()         { if (m_FramePrev) return m_FramePrev; else return m_Averager.GetAvgFrame(); };
    pTVFrame GetFrameLast()           { return m_FrameLast; };
    double   GetMotionValue(pTVFrame frame);
	double GetMotionValue2(pTVFrame iframe);
};

#endif // !defined(AFX_MOTIONDETECTOR_H__CC699F21_F3B2_11D5_B0F2_000001360793__INCLUDED_)

//  $File : MotionDetector.cpp: implementation of the CMotionDetector class.
//  (C) Copyright HomeBuilt Group and File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)

#include "stdafx.h"
#include <ImageProc\MotionDetector.h>
#include <imageproc\resample.h>
#include <imageproc\motiondetectors.h>
#include <imageproc\fstfilter.h>
#include <imageproc\EdgeFilters.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMotionDetector::CMotionDetector():
        m_FramePrev(NULL),
        m_FrameLast(NULL),
        m_Threshold(1.0)
{
}

CMotionDetector::~CMotionDetector()
{
    if (m_FramePrev) freeTVFrame(m_FramePrev); m_FramePrev=NULL;
    if (m_FrameLast) freeTVFrame(m_FrameLast); m_FrameLast=NULL;
}

double CMotionDetector::GetMotionValue(pTVFrame iframe)
{
   if (!iframe) return 0;

   pTVFrame n_frame=makecopyTVFrame(iframe);
   _diminish(n_frame); 
   //_diminish(n_frame);
   _restoreOrgSize(n_frame);

   if (!m_FramePrev) 
   {
       m_FramePrev=n_frame;
       return 0;
   }
   double dif=compareframes_abs(m_FramePrev, n_frame);
   //double dif=compareframes_val(m_FramePrev, n_frame);
   //double dif=compareframes_max(m_FramePrev, n_frame);
   freeTVFrame(m_FramePrev); m_FramePrev=n_frame;
   dif=(dif/m_Threshold);
   return dif;
}

double CMotionDetector::GetMotionValue2(pTVFrame iframe)
{
   if (!iframe) return 0;

   pTVFrame n_frame=makecopyTVFrame(iframe);
   _diminish(n_frame); 
   _diminish(n_frame); 
   _get_edges(n_frame);
   n_frame=_lpass(n_frame,700);
   //_normalize(n_frame);
   _simplebinarize(n_frame);
   //_dilate(n_frame);
   //_diminish(n_frame);
   //_restoreOrgSize(n_frame);
   m_Averager.AddFrame(n_frame);
   if (!m_Averager.GetAvgFrame()) 
   {
       return 0;
   } 
   double dif=(double)(compareframes_abs(m_Averager.GetAvgFrame(), n_frame))/2.0;
   //double dif=compareframes_val(m_Averager.GetAveragedFrame(), n_frame);
   //double dif=compareframes_max(m_FramePrev, n_frame);
   //freeTVFrame(m_FramePrev); m_FramePrev=n_frame;
   dif=(dif/m_Threshold);
   if (m_FrameLast) freeTVFrame(m_FrameLast); m_FrameLast=n_frame;
   return dif;
}

// MotionEstimate.h: interface for the MotionEstimate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOTIONESTIMATE_H__0A832162_0950_4319_9CEC_C9C8BFAB0446__INCLUDED_)
#define AFX_MOTIONESTIMATE_H__0A832162_0950_4319_9CEC_C9C8BFAB0446__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>

class MotionEstimate : public CCollectorGadget
{
    int     m_ROI;
    BOOL    m_bPrediction;
    int     m_Base;
public:
	MotionEstimate();
    virtual bool PrintProperties(FXString& text);
    virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
    virtual bool ScanSettings(FXString& text);
    CDataFrame* DoProcessing(CDataFrame const*const* frames, int nmb);
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2);
    CDataFrame* DoProcessing2(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2, const CDataFrame* pDataFrame3);
    DECLARE_RUNTIME_GADGET(MotionEstimate);
};

#endif // !defined(AFX_MOTIONESTIMATE_H__0A832162_0950_4319_9CEC_C9C8BFAB0446__INCLUDED_)

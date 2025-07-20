// MotionEstimate.h: interface for the MotionEstimate class.
//
//////////////////////////////////////////////////////////////////////

//#if !defined(AFX_MOTIONESTIMATE_H__0A832162_0950_4319_9CEC_C9C8BFAB0446__INCLUDED_)
//#define AFX_MOTIONESTIMATE_H__0A832162_0950_4319_9CEC_C9C8BFAB0446__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>

class MotionBridge : public CCollectorGadget
{
    
public:
	MotionBridge();
    
	CDataFrame* DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2) {return NULL;};
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2, const CDataFrame* pDataFrame3);
    CDataFrame* DoProcessing(CDataFrame const*const* frames, int nmb);
    DECLARE_RUNTIME_GADGET(MotionBridge);
};

//#endif // !defined(AFX_MOTIONESTIMATE_H__0A832162_0950_4319_9CEC_C9C8BFAB0446__INCLUDED_)

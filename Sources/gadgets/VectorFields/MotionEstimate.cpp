// MotionEstimate.cpp: implementation of the MotionEstimate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MotionEstimate.h"
#include "SeekMotion.h"
#include <gadgets/videoframe.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_RUNTIME_GADGET_EX(MotionEstimate, CCollectorGadget, "VectorFields", TVDB400_PLUGIN_NAME);

MotionEstimate::MotionEstimate()
{
    m_ROI=DEF_AREA_SEARCH;
    m_bPrediction=FALSE;
    m_pOutput = new COutputConnector(userdata);
    CreateInputs(2,vframe);
    m_Base=BLCKSIZE_4x4;
    Resume();
}

CDataFrame* MotionEstimate::DoProcessing(CDataFrame const*const* frames, int nmb)
{
    if (nmb==2)
    {
        CDataFrame* pDataFrame=DoProcessing(frames[0], frames[1]);
        if (pDataFrame)
            pDataFrame->CopyAttributes(frames[0]);
        return pDataFrame;
    }
    else if (nmb==3)
    {
        CDataFrame* pDataFrame=DoProcessing2(frames[0], frames[1], frames[2]);
        if (pDataFrame)
            pDataFrame->CopyAttributes(frames[0]);
        return pDataFrame;
    }
    return NULL;
}

CDataFrame* MotionEstimate::DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2) 
{ 
	const CVideoFrame* VideoFrame1 = pDataFrame1->GetVideoFrame(DEFAULT_LABEL);
	const CVideoFrame* VideoFrame2 = pDataFrame2->GetVideoFrame(DEFAULT_LABEL);
    if ((!VideoFrame1) || (!VideoFrame2))
    {
        return NULL;
    }
    CSeekMotion sm(VideoFrame1, VideoFrame2);
    sm.SetSeekArea(m_ROI);
    if (sm.Calc(m_Base))
    {
        CVectorFieldFrame* vff=CVectorFieldFrame::Create();
        sm.GetVectors(*vff);
        return vff;
    }
    return NULL;
}

CDataFrame* MotionEstimate::DoProcessing2(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2, const CDataFrame* pDataFrame3)
{
	const CVideoFrame* VideoFrame1 = pDataFrame1->GetVideoFrame(DEFAULT_LABEL);
	const CVideoFrame* VideoFrame2 = pDataFrame2->GetVideoFrame(DEFAULT_LABEL);
    CVectorFieldFrame* FieldFrame = (CVectorFieldFrame*)pDataFrame3->GetUserDataFrame(VECTROFILELDNAME, DEFAULT_LABEL);
    if ((!VideoFrame1) || (!VideoFrame2) || (!FieldFrame))
    {
        return NULL;
    }
    CSeekMotion sm(VideoFrame1, VideoFrame2);
    sm.SetSeekArea(m_ROI);
    if (sm.Calc(m_Base,FieldFrame))
    {
        CVectorFieldFrame* vff=CVectorFieldFrame::Create();
        sm.GetVectors(*vff);
        return vff;
    }
    
    return NULL;
}

bool MotionEstimate::PrintProperties(FXString& text)
{
    FXPropertyKit pc;
    pc.WriteInt("ROI",m_ROI);
    pc.WriteInt("UsePrediction",m_bPrediction);
    pc.WriteInt("Base",m_Base);
    text=pc;
	return true;
}

bool MotionEstimate::ScanProperties(LPCTSTR text, bool& Invalidate)
{
    FXPropertyKit pc(text);
    pc.GetInt("ROI",m_ROI);
    pc.GetInt("UsePrediction",m_bPrediction);
    pc.GetInt("Base",m_Base);
    if (m_bPrediction)
        CreateInputs(3,transparent);
    else
        CreateInputs(2,vframe);
    return true;
}

bool MotionEstimate::ScanSettings(FXString& text)
{
    text="template(Spin(ROI,1,8),ComboBox(UsePrediction(FALSE(0),TRUE(1))),ComboBox(Base(8x8(1),4x4(2))))";
    return true;
}

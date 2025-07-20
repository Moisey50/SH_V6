#include "stdafx.h"
#include "Code128Reader.h"
#include <Gadgets\VideoFrame.h>
#include <Gadgets\TextFrame.h>
#include <imageproc\recognition\decode128.h>

#define PASSTHROUGH_NULLFRAME(vfr, fr)			\
{												\
	if (!(vfr) || ((vfr)->IsNullFrame()))		\
    {	                                        \
		return NULL;			                \
    }                                           \
}


IMPLEMENT_RUNTIME_GADGET_EX(Code128Reader, CFilterGadget, "Video.recognition", TVDB400_PLUGIN_NAME);


Code128Reader::Code128Reader(void)
{
    m_MultyCoreAllowed=true;
	m_pInput   =    new CInputConnector(vframe);
	m_pOutput  =    new COutputConnector(text);
	Resume();
}

void Code128Reader::ShutDown()
{
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

CDataFrame* Code128Reader::DoProcessing(const CDataFrame* pDataFrame)
{
	const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
    PASSTHROUGH_NULLFRAME(VideoFrame, pDataFrame);
	FXString retS;
	if (parse128(VideoFrame, retS))
	{
		retS.TrimLeft('*');
		retS.TrimRight('*');
		FXString retV; retV="Code128: "+retS;
		CTextFrame* retVal=CTextFrame::Create(retV);
            retVal->CopyAttributes(pDataFrame);;
		return retVal;
	}
	return NULL;
}


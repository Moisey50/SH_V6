#include "stdafx.h"
#include "Code39.h"
#include <Gadgets\VideoFrame.h>
#include <Gadgets\TextFrame.h>
#include <imageproc\recognition\decode39.h>

#define THIS_MODULENAME "UPCReader.cpp"

#define PASSTHROUGH_NULLFRAME(vfr, fr)			\
{												\
	if (!(vfr) || ((vfr)->IsNullFrame()))		\
    {	                                        \
		return NULL;			                \
    }                                           \
}


IMPLEMENT_RUNTIME_GADGET_EX(Code39Reader, CFilterGadget, "Video.recognition", TVDB400_PLUGIN_NAME);

Code39Reader::Code39Reader(void)
{
    m_MultyCoreAllowed=true;
	m_pInput   =    new CInputConnector(vframe);
	m_pOutput  =    new COutputConnector(text);
	Resume();
}

void Code39Reader::ShutDown()
{
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

CDataFrame* Code39Reader::DoProcessing(const CDataFrame* pDataFrame)
{
	const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
    PASSTHROUGH_NULLFRAME(VideoFrame, pDataFrame);
	FXString retS;
	if (parse39(VideoFrame, retS))
	{
		retS.TrimLeft('*');
		retS.TrimRight('*');
		CString retV; retV="Code39: "+retS;
		CTextFrame* retVal=CTextFrame::Create(retV);
            retVal->CopyAttributes(pDataFrame);;
		return retVal;
	}
	return NULL;
}


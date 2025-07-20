// UPCReader.cpp: implementation of the UPCReader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Gadgets\VideoFrame.h>
#include <Gadgets\TextFrame.h>
#include "barcodes.h"
#include "UPCReader.h"
#include <imageproc\recognition\decodeUPC.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "UPCReader.cpp"

#define PASSTHROUGH_NULLFRAME(vfr, fr)			\
{												\
	if (!(vfr) || ((vfr)->IsNullFrame()))		\
    {	                                        \
		return NULL;			                \
    }                                           \
}


IMPLEMENT_RUNTIME_GADGET_EX(UPCReader, CFilterGadget, "Video.recognition", TVDB400_PLUGIN_NAME);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UPCReader::UPCReader()
{
    m_MultyCoreAllowed=true;
	m_pInput   =    new CInputConnector(vframe);
	m_pOutput  =    new COutputConnector(text);
	Resume();
}

void UPCReader::ShutDown()
{
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

CDataFrame* UPCReader::DoProcessing(const CDataFrame* pDataFrame)
{
	const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
    PASSTHROUGH_NULLFRAME(VideoFrame, pDataFrame);
	FXString retS;
	if (parse(VideoFrame, retS))
	{
		FXString retV;
		retV=((retS.GetLength()==13)?"EAN-13: ":"EAN-8: ")+retS;
		CTextFrame* retVal=CTextFrame::Create(retV);
            retVal->CopyAttributes(pDataFrame);;
		return retVal;
	}
	return NULL;
}

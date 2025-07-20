// SquareFilter.h : Implementation of the SquareFilter class


#include "StdAfx.h"
#include "SquareFilter.h"
#include <Gadgets\VideoFrame.h>
#include <Gadgets\vftempl.h>

IMPLEMENT_RUNTIME_GADGET_EX(SquareFilter, CFilterGadget, "Video.frequency", TVDB400_PLUGIN_NAME);

SquareFilter::SquareFilter(void): m_Box(box7x7)
{
    m_pInput = new CInputConnector(vframe);
    m_pOutput = new COutputConnector(vframe);
    Resume();
}

void SquareFilter::ShutDown()
{
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

CDataFrame* SquareFilter::DoProcessing(const CDataFrame* pDataFrame)
{
    const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
	PASSTHROUGH_NULLFRAME(VideoFrame, pDataFrame);
    CVideoFrame* retVal=CVideoFrame::Create(_sqrfilter(VideoFrame, m_Box));
        retVal->CopyAttributes(pDataFrame);;
    return retVal;
}

bool SquareFilter::ScanProperties(LPCTSTR text, bool& Invalidate)
{
    FXPropertyKit pk(text);
    pk.GetInt("Box",(int&)m_Box);
	return true;
}

bool SquareFilter::PrintProperties(FXString& text)
{
  FXPropertyKit pc;
  pc.WriteInt("Box",m_Box);
  text=pc;
  return true;
}

bool SquareFilter::ScanSettings(FXString& text)
{
    text="template(ComboBox(Box(box3x3(0),box5x5(1),box7x7(2))))";
    return true;
}


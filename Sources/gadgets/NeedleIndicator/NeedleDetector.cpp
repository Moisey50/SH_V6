#include "stdafx.h"
#include "NeedleIndicator.h"
#include "NeedleDetector.h"
#include <gadgets\vftempl.h>
#include <gadgets/containerframe.h>
#include "NeedleSetup.h"

#define THIS_MODULENAME "NeedleDetector"

IMPLEMENT_RUNTIME_GADGET_EX(NeedleDetector, CGadget, "Video.recognition", TVDB400_PLUGIN_NAME);

NeedleDetector::NeedleDetector(void):
	m_ScanLine(172),
	m_delta(10),
	m_FstPointValue(100),
	m_FstPointX(100),
	m_SdPointValue(400),
	m_SdPointX(400)
{
	m_pInput  = new CInputConnector(vframe);
	m_pOutput = new COutputConnector(vframe);
	m_pTextOutput = new COutputConnector(text);
    m_SetupObject = new CNeedleSetup(this, NULL);
	Resume();
}

void NeedleDetector::ShutDown()
{
	CGadget::ShutDown();
	if (m_pInput) delete m_pInput;
	m_pInput = NULL;
	if (m_pOutput) delete m_pOutput;
	m_pOutput = NULL;
	if (m_pTextOutput) delete m_pTextOutput;
	m_pTextOutput =NULL;
}

double	NeedleDetector::GetValue(int pos)
{
	double ratio=(m_SdPointValue-m_FstPointValue)/(m_SdPointX-m_FstPointX);
	return ratio*(pos-m_FstPointX)+m_FstPointValue;
}

int NeedleDetector::SeekMax(const CVideoFrame* VideoFrame)
{
	LPBYTE data=GetData(VideoFrame);
	int w=VideoFrame->lpBMIH->biWidth;
	int h=VideoFrame->lpBMIH->biHeight;
	int slMin=m_ScanLine-m_delta; if (slMin<0) slMin=0;
	int slMax=m_ScanLine+m_delta; if (slMax>=h) slMax=h-1;
	int max=0;
	int maxpos=-1;
	for (int x=0; x<w; x++)
	{
		int min=255;
		for (int y=slMin; y<slMax; y++)
		{
			if (min>data[x+y*w]) min=data[x+y*w];
		}
		if (min>max)
		{
			max=min;
			maxpos=x;
		}
	}
	return maxpos;
}

CDataFrame* NeedleDetector::DoProcessing(const CDataFrame* pDataFrame)
{
	const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
	PASSTHROUGH_NULLFRAME(VideoFrame, pDataFrame);

	if (IsSetupOn())
		((CNeedleSetup*)m_SetupObject)->LoadFrame(VideoFrame);
	int maxpos=SeekMax(VideoFrame);
	double value=GetValue(maxpos);
	FXString tmpS;
	tmpS.Format("%f",value);
	CTextFrame* tf=CTextFrame::Create(tmpS);
    tf->CopyAttributes(pDataFrame);
	if (!m_pTextOutput->Put(tf))
		tf->Release(tf);

	if (!m_pOutput->IsConnected()) 
		return NULL;

	CContainerFrame* pResult = CContainerFrame::Create();
    pResult->CopyAttributes(pDataFrame);
	pResult->AddFrame(pDataFrame);

	CFigureFrame* bl=CFigureFrame::Create();
    bl->Add(CDPoint(0,m_ScanLine));
    bl->Add(CDPoint(VideoFrame->lpBMIH->biWidth,m_ScanLine));
    bl->Attributes()->WriteString("color","0xff0000");
    pResult->AddFrame(bl);

	CFigureFrame* bm=CFigureFrame::Create();
    bm->Add(CDPoint(maxpos,0));
	bm->Add(CDPoint(maxpos,VideoFrame->lpBMIH->biHeight));
    bm->Attributes()->WriteString("color","0x0000ff");
    pResult->AddFrame(bm);

	return pResult;
}

bool NeedleDetector::PrintProperties(FXString& text)
{
    FXPropertyKit pc;
    pc.WriteInt("ScanLinePos",m_ScanLine);
	pc.WriteDouble("FstPointValue",m_FstPointValue);
	pc.WriteInt("FstPointX",m_FstPointX);
	pc.WriteDouble("SdPointValue",m_SdPointValue);
	pc.WriteInt("SdPointX",m_SdPointX);
    text=pc;
	return true;
}

bool NeedleDetector::ScanProperties(LPCTSTR text, bool& Invalidate)
{
    FXPropertyKit pc(text);
    pc.GetInt("ScanLinePos",m_ScanLine);
	pc.GetDouble("FstPointValue",m_FstPointValue);
	pc.GetInt("FstPointX",m_FstPointX);
	pc.GetDouble("SdPointValue",m_SdPointValue);
	pc.GetInt("SdPointX",m_SdPointX);
    return true;
}

bool NeedleDetector::ScanSettings(FXString& text)
{
    text="calldialog(true)"; 
    return true;
}

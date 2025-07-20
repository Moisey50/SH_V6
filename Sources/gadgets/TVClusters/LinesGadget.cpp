// LinesGadget.cpp: implementation of the Lines class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Gadgets\vftempl.h>
#include <helpers\FramesHelper.h>
#include "tvclusters.h"
#include "LinesGadget.h"
#include <gadgets\ContainerFrame.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "LinesGadget.cpp"

IMPLEMENT_RUNTIME_GADGET_EX(Lines, CFilterGadget, "Video.recognition", TVDB400_PLUGIN_NAME);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Lines::Lines()
{
  m_pInput   =    new CInputConnector(vframe);
  m_pOutput  =    new COutputConnector(vframe * figure);
  m_FormatErrorProcessed=false;
  m_LastFormat=0;
  m_MultyCoreAllowed=true;
  m_OutputMode=modeReplace;
  Resume();
}

void Lines::ShutDown()
{
    CFilterGadget::ShutDown();
    delete m_pInput;
    m_pInput = NULL;
    delete m_pOutput;
    m_pOutput = NULL;
}

CDataFrame* Lines::DoProcessing(const CDataFrame* pDataFrame)
{
	const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
	PASSTHROUGH_NULLFRAME(VideoFrame, pDataFrame);
    if (m_LastFormat!=VideoFrame->lpBMIH->biCompression)
    {
        m_LastFormat=VideoFrame->lpBMIH->biCompression;
        m_FormatErrorProcessed=false;
    }
    if ((m_LastFormat!=BI_YUV9) && (m_LastFormat!=BI_Y8) && (m_LastFormat!=BI_YUV12)) 
    {
        if (!m_FormatErrorProcessed)
            SENDERR_0("Lines can only accept formats YUV9 and Y8");
        m_FormatErrorProcessed=true;
        return NULL;
    }
    CStrictLines LineDetector;
    LineDetector.ParseFrame(VideoFrame);
    CRect rc(0,0,0,0);

    CContainerFrame* retVal = CContainerFrame::Create();
        retVal->CopyAttributes(pDataFrame);;
    CContainerFrame* resultVal=CContainerFrame::Create();
    resultVal->CopyAttributes(pDataFrame);
    resultVal->SetLabel("Figures");

    CFigureFrame* ff=NULL;
    for (int i=0; i<LineDetector.GetClustersNmb(); i++)
    {
        plinefactors plf=LineDetector.GetFigure(i);
        if (plf->type==TYPE_RECTANGLE)
        {
            ff=CFigureFrame::Create();
            ff->CopyAttributes(pDataFrame);
            ff->SetLabel("Rectangle");
	        ff->AddPoint(plf->a);
            ff->AddPoint(plf->b);
            ff->AddPoint(plf->c);
            ff->AddPoint(plf->d);
        }
        else if ((plf->type==TYPE_VSEGMENT) || (plf->type==TYPE_HSEGMENT))
        {
            ff=CFigureFrame::Create();
            ff->CopyAttributes(pDataFrame);
            ff->SetLabel("Line");
	        ff->AddPoint(plf->a);
            ff->AddPoint(plf->b);
        }
        if (ff) 
        {
            ff->CopyAttributes(pDataFrame);
            ff->Attributes()->WriteString("color","0x0000ff");
            resultVal->AddFrame(ff);
        }
    }
    retVal->AddFrame(resultVal);
    retVal->AddFrame(pDataFrame);
	return retVal;
}

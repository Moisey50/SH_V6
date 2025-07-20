// ScanLine.cpp: implementation of the ScanLine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScanLine.h"
#include "TVVideo.h"
#include <Gadgets\vftempl.h>
#include <gadgets\VideoFrame.h>
#include <gadgets\TextFrame.h>
#include <gadgets\FigureFrame.h>
#include <gadgets\ContainerFrame.h>
#include <imageproc\colors.h>

IMPLEMENT_RUNTIME_GADGET_EX(ScanLine, CFilterGadget, LINEAGE_DIAGNOSTIC, TVDB400_PLUGIN_NAME)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ScanLine::ScanLine():
    m_Line(-1),
    m_Row(-1)
{
	m_pInput  = new CInputConnector(vframe);
	m_pOutput = new COutputConnector(figure);
    m_pControl = new CDuplexConnector(this, text, text);
    Resume();
}

void ScanLine::ShutDown()
{
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
	delete m_pControl;
	m_pControl = NULL;
}

int ScanLine::GetDuplexCount()
{
	return 1;
}

CDuplexConnector* ScanLine::GetDuplexConnector(int n)
{
	return ((!n) ? m_pControl : NULL);
}

void ScanLine::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
    FXAutolock al(m_Lock);
	ASSERT(pParamFrame);
    CTextFrame* TextFrame = pParamFrame->GetTextFrame(DEFAULT_LABEL);
    if (TextFrame)
    {
        FXPropertyKit pk=TextFrame->GetString();
        int posY;
        BOOL    selected;
        if (pk.GetBool("selected",selected))
        {
            if ( selected && pk.GetInt("y",posY))
            {
                m_Line=posY;
                pk.GetInt("x",(int&)m_Row);
                TRACE("+++ ScanLine::AsyncTransaction x=%d y=%d\n",m_Row,m_Line);
            }
        }
    }
    pParamFrame->RELEASE(pParamFrame);
}

CDataFrame* ScanLine::DoProcessing(const CDataFrame* pDataFrame)
{
    FXAutolock al(m_Lock);
    const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
    PASSTHROUGH_NULLFRAME(VideoFrame, pDataFrame);
    CContainerFrame* retV=CContainerFrame::Create();
    retV->CopyAttributes(pDataFrame);

    CFigureFrame* bl=CFigureFrame::Create();
    bl->Add(CDPoint(0,m_Line));
    bl->Add(CDPoint(VideoFrame->lpBMIH->biWidth,m_Line));
    // How to set color for figures
    bl->Attributes()->WriteString("color","0xff0000");
    retV->AddFrame(bl);

    CFigureFrame* ff=CFigureFrame::Create();
    ff->CopyAttributes(pDataFrame);
    if ((m_Line>=0) && (m_Line<VideoFrame->lpBMIH->biHeight))
    {
        // How to pass the text messages and set it's color and coordinates 1st version
        FXString mes="Here is a point you are clicked at.";
        CTextFrame* tf=CTextFrame::Create(mes);
        tf->CopyAttributes(pDataFrame);
        tf->Attributes()->WriteInt("x",m_Row);
        tf->Attributes()->WriteInt("y",m_Line);
        tf->Attributes()->WriteString("color","0xffffff"); 
        // End text messages
        retV->AddFrame(tf);
        for(unsigned i=0; i<(unsigned)(VideoFrame->lpBMIH->biWidth); i++)
        {
            CDPoint pt;
            if (is16bit(VideoFrame))
            {
                pt.x=i;
                pt.y=VideoFrame->lpBMIH->biHeight-(VideoFrame->lpBMIH->biHeight*getdata_I_XY(VideoFrame, i, m_Line))/65535;
            }
            else
            {
                pt.x=i;
                pt.y=VideoFrame->lpBMIH->biHeight-(VideoFrame->lpBMIH->biHeight*getdata_I_XY(VideoFrame, i, m_Line))/256;
            }
            ff->Add(pt);
        }
    }
    ff->Attributes()->WriteString("color","0x00ff00");
    CFigureFrame* ps=CFigureFrame::Create();
    ps->CopyAttributes(pDataFrame);
    ps->Attributes()->WriteString("color","0xff7f00");
    ps->Add(CDPoint(m_Row,m_Line));
    
    retV->AddFrame(ff);
    retV->AddFrame(ps);
    
    return retV;
}
// ShowVectorField.cpp: implementation of the ShowVectorField class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VectorFields.h"
#include "ShowVectorField.h"
#include <Gadgets\vftempl.h>
#include <gadgets\MetafileFrame.h>
#include <gadgets\VideoFrame.h>
#include "VectorFieldFrame.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_RUNTIME_GADGET_EX(ShowVectorField, CFilterGadget, "VectorFields", TVDB400_PLUGIN_NAME)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ShowVectorField::ShowVectorField()
{
    m_OutputMode=modeAppend;
	m_pInput = new CInputConnector(vframe);
	m_pOutput = new COutputConnector(vframe);
	Resume();
}

void ShowVectorField::ShutDown()
{
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

bool ShowVectorField::Pic2Scr(DPOINT& point, CPoint& res)
{
    res.x=(LONG)(SCALE*point.x);
    res.y=(LONG)(SCALE*point.y);
    return true;
}

CDataFrame* ShowVectorField::DoProcessing(const CDataFrame* pDataFrame) 
{ 
    const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
    const CUserDataFrame* uDFrame = pDataFrame->GetUserDataFrame(VECTROFILELDNAME, DEFAULT_LABEL);
    PASSTHROUGH_NULLFRAME(VideoFrame, pDataFrame);

    CMetafileFrame* wmf = NULL;
    if ((uDFrame) && (VideoFrame) && (uDFrame->GetUserType()==VECTROFILELDNAME))
    {
        CVectorFieldFrame* vff=(CVectorFieldFrame*)uDFrame;
        wmf=CMetafileFrame::Create();
        wmf->CopyAttributes(pDataFrame);
        CRect pictRect(0,0,VideoFrame->lpBMIH->biWidth*SCALE,VideoFrame->lpBMIH->biHeight*SCALE);
        CRgn   rgn;
        VERIFY(rgn.CreateRectRgn(pictRect.left,pictRect.top,pictRect.right,pictRect.bottom));
        int width =VideoFrame->lpBMIH->biWidth;

	    HDC hdc=wmf->StartDraw(pictRect); /////
        CDC dc; dc.Attach(hdc); dc.SetAttribDC(hdc);
        dc.SelectClipRgn(&rgn);
        CPen mP(PS_SOLID,1,RGB(255,0,0));
        CPen* oP=dc.SelectObject(&mP);

        for(int y=0; y<vff->sizeY; y++)
        {
            for (int x=0; x<vff->sizeX; x++)
            {
                CDPoint dPnt(x*vff->stepX+vff->stepX/2,y*vff->stepY+vff->stepY/2);
                CPoint pnt;
                Pic2Scr(dPnt,pnt);
                dc.MoveTo(pnt.x, pnt.y);
                dPnt.x+=vff->vfvectors[x+y*vff->sizeX].x;
                dPnt.y+=vff->vfvectors[x+y*vff->sizeX].y;
                Pic2Scr(dPnt,pnt);
                dc.LineTo(pnt.x, pnt.y);
            }
        }
        dc.SelectObject(oP);
        wmf->EndDraw(); 
    }
    return wmf; 
};

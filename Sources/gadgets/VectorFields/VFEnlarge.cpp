// VFEnlarge.cpp: implementation of the VFEnlarge class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VFEnlarge.h"
#include "VectorFieldFrame.h"
#include <math\intf_sup.h>

IMPLEMENT_RUNTIME_GADGET_EX(VFEnlarge, CFilterGadget, "VectorFields", TVDB400_PLUGIN_NAME);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VFEnlarge::VFEnlarge()
{
	m_pInput = new CInputConnector(userdata);
	m_pOutput = new COutputConnector(userdata);
	Resume();
}

void VFEnlarge::ShutDown()
{
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

CDataFrame* VFEnlarge::DoProcessing(const CDataFrame* pDataFrame)
{
    CVectorFieldFrame* retV=NULL;

	CVectorFieldFrame* vfFrame = (CVectorFieldFrame*)pDataFrame->GetUserDataFrame(VECTROFILELDNAME,DEFAULT_LABEL);
    if (vfFrame)
    {
        CVectorField vf;
        vf.stepX=vfFrame->stepX;
        vf.stepY=vfFrame->stepY;
        vf.sizeX=vfFrame->sizeX*2;
        vf.sizeY=vfFrame->sizeY*2;
        vf.vfvectors=(pvfvector)malloc(sizeof(vfvector)*vf.sizeX*vf.sizeY);
        memset(vf.vfvectors,0,sizeof(vfvector)*vf.sizeX*vf.sizeY);
        int x,y;
        for (y=0; y<vfFrame->sizeY-1; y++)
        {
            for (x=0; x<vfFrame->sizeX; x++)
            {
                vf.vfvectors[2*(x+y*vf.sizeX)].x=2*vfFrame->vfvectors[x+y*vfFrame->sizeX].x;
                vf.vfvectors[2*(x+y*vf.sizeX)].y=2*vfFrame->vfvectors[x+y*vfFrame->sizeX].y;

                vf.vfvectors[2*(x+y*vf.sizeX)+1].x=vfFrame->vfvectors[x+y*vfFrame->sizeX].x+vfFrame->vfvectors[x+y*vfFrame->sizeX+1].x;
                vf.vfvectors[2*(x+y*vf.sizeX)+1].y=vfFrame->vfvectors[x+y*vfFrame->sizeX].y+vfFrame->vfvectors[x+y*vfFrame->sizeX+1].y;

                vf.vfvectors[2*(x+y*vf.sizeX)+vf.sizeX].x=vfFrame->vfvectors[x+y*vfFrame->sizeX].x+vfFrame->vfvectors[x+(y+1)*vfFrame->sizeX].x;
                vf.vfvectors[2*(x+y*vf.sizeX)+vf.sizeX].y=vfFrame->vfvectors[x+y*vfFrame->sizeX].y+vfFrame->vfvectors[x+(y+1)*vfFrame->sizeX].y;

                vf.vfvectors[2*(x+y*vf.sizeX)+vf.sizeX+1].x=(vfFrame->vfvectors[x+y*vfFrame->sizeX].x+vfFrame->vfvectors[x+y*vfFrame->sizeX+1].x
                                                           +vfFrame->vfvectors[x+(y+1)*vfFrame->sizeX].x+vfFrame->vfvectors[x+(y+1)*vfFrame->sizeX+1].x)/2;
                vf.vfvectors[2*(x+y*vf.sizeX)+vf.sizeX+1].y=(vfFrame->vfvectors[x+y*vfFrame->sizeX].y+vfFrame->vfvectors[x+y*vfFrame->sizeX+1].y
                                                           +vfFrame->vfvectors[x+(y+1)*vfFrame->sizeX].y+vfFrame->vfvectors[x+(y+1)*vfFrame->sizeX+1].y)/2;
            }
        }
        vf.vfvectors[2*(x+y*vf.sizeX)].x=2*vfFrame->vfvectors[x+y*vfFrame->sizeX].x;
        vf.vfvectors[2*(x+y*vf.sizeX)].y=2*vfFrame->vfvectors[x+y*vfFrame->sizeX].y;

        vf.vfvectors[2*(x+y*vf.sizeX)+1].x=vfFrame->vfvectors[x+y*vfFrame->sizeX].x+vfFrame->vfvectors[x+y*vfFrame->sizeX+1].x;
        vf.vfvectors[2*(x+y*vf.sizeX)+1].y=vfFrame->vfvectors[x+y*vfFrame->sizeX].y+vfFrame->vfvectors[x+y*vfFrame->sizeX+1].y;

        retV=CVectorFieldFrame::Create(vf);
        retV->CopyAttributes(pDataFrame);
    } 
	return retV;
}

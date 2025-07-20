// TestVectorField.cpp: implementation of the TestVectorField class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VectorFields.h"
#include "TestVectorField.h"
#include "VectorFieldFrame.h"
#include <gadgets\VideoFrame.h>
#include <math\intf_sup.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_RUNTIME_GADGET_EX(TestVectorField, CFilterGadget, "VectorFields", TVDB400_PLUGIN_NAME);


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TestVectorField::TestVectorField()
{
	m_pInput = new CInputConnector(vframe);
	m_pOutput = new COutputConnector(userdata);
	Resume();
}

void TestVectorField::ShutDown()
{
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

int cnt=0;

CDataFrame* TestVectorField::DoProcessing(const CDataFrame* pDataFrame)
{
    CVectorFieldFrame* retV=NULL;

	const CVideoFrame* vFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
    if (vFrame)
    {
        int width=vFrame->lpBMIH->biWidth;
        int height=vFrame->lpBMIH->biHeight;
        CVectorField vf;
        vf.stepX=16;
        vf.stepY=16;
        vf.sizeX=(int)(width/vf.stepX);
        vf.sizeY=(int)(height/vf.stepY);
        vf.vfvectors=(pvfvector)malloc(sizeof(vfvector)*vf.sizeX*vf.sizeY);
        for (int y=0; y<vf.sizeY; y++)
        {
            for (int x=0; x<vf.sizeX; x++)
            {
                vf.vfvectors[x+y*vf.sizeX].x=8*sin(cnt*0.02);
                vf.vfvectors[x+y*vf.sizeX].y=8*cos(cnt*0.02);
                cnt++;
            }
        }
        retV=CVectorFieldFrame::Create(vf);
        retV->CopyAttributes(pDataFrame);
    }
	return retV;
}

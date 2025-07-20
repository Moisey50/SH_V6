// CreateMask.cpp: implementation of the CreateMask class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CreateMask.h"
#include <gadgets\VideoFrame.h>
#include <Gadgets\vftempl.h>
#include "VectorFieldFrame.h"

IMPLEMENT_RUNTIME_GADGET_EX(CreateMask, CFilterGadget, "VectorFields", TVDB400_PLUGIN_NAME)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CreateMask::CreateMask()
{
	m_pInput = new CInputConnector(userdata);
	m_pOutput = new COutputConnector(vframe);
	Resume();

}

void CreateMask::ShutDown()
{
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

CDataFrame* CreateMask::DoProcessing(const CDataFrame* pDataFrame)
{
    const CVectorFieldFrame* uDFrame = (CVectorFieldFrame*)pDataFrame->GetUserDataFrame(VECTROFILELDNAME, DEFAULT_LABEL);
    if (!uDFrame)
         return NULL;
    int width=(int)(uDFrame->sizeX*uDFrame->stepX);
    int height=(int)(uDFrame->sizeY*uDFrame->stepY);

    pTVFrame frame =(pTVFrame)malloc(sizeof(TVFrame));
    memset(frame,0,sizeof(TVFrame));
    
    frame->lpBMIH=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+ width*height);
    memset(frame->lpBMIH,0,sizeof(BITMAPINFOHEADER)+ width*height);

    frame->lpBMIH->biSize=sizeof(BITMAPINFOHEADER);
    frame->lpBMIH->biWidth=width;
    frame->lpBMIH->biHeight=height;
    frame->lpBMIH->biPlanes=1;
    frame->lpBMIH->biBitCount=8;
    frame->lpBMIH->biCompression=BI_Y8;
    frame->lpBMIH->biSizeImage=width*height;

    LPBYTE dst=GetData(frame);

    for (int y=0; y<uDFrame->sizeY; y++)
    {
        for (int x=0; x<uDFrame->sizeX; x++)
        {
            if ((uDFrame->vfvectors[x+y*uDFrame->sizeX].x!=0) || (uDFrame->vfvectors[x+y*uDFrame->sizeX].y!=0))
            {
                LPBYTE off=dst + (x+y*uDFrame->sizeX*(int)uDFrame->stepY)*(int)uDFrame->stepX;
                for (int i=0; i<uDFrame->stepY; i++)
                {
                    memset(off,255,(int)uDFrame->stepX);
                    off+=width;
                }
            }
        }
    }

    CVideoFrame* retV=CVideoFrame::Create(frame);
    retV->CopyAttributes(pDataFrame);
    return retV;
}
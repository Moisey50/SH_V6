// MotionEstimate.cpp: implementation of the MotionEstimate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MotionBridgeGadget.h"
#include "VectorFieldFrame.h"
#include <gadgets/videoframe.h>
#include <math\intf_sup.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_RUNTIME_GADGET_EX(MotionBridge, CCollectorGadget, "VectorFields", TVDB400_PLUGIN_NAME);

MotionBridge::MotionBridge()
{
    
    m_pOutput = new COutputConnector(vframe);
	CreateInputs(2, vframe);
	CreateInputs(3, userdata);
    Resume();    
}

CDataFrame* MotionBridge::DoProcessing(CDataFrame const*const* frames, int nmb)
{
        ASSERT(nmb==3);
        CDataFrame* pDataFrame= DoProcessing(frames[0], frames[1], frames[2]);
        if (pDataFrame)
            pDataFrame->CopyAttributes(pDataFrame);
        return pDataFrame;
}

CDataFrame* MotionBridge::DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2, const CDataFrame* pDataFrame3)
{
	int iW = 0; //blockstring counter
	int iH = 0; // counter
	int counter = 0;//
	int iPOffset = 0;
	int iPmove = 0;
	int iPmoveHalf = 0;
	int startHalf = 0;
	int startSecond = 0;
	int moveX = 0;
	int moveY = 0;
	int imageSize = 0;

	vfvector move = {0,0};
	const CVideoFrame* vf1 = pDataFrame1->GetVideoFrame(DEFAULT_LABEL);
	const CVideoFrame* vf2 = pDataFrame2->GetVideoFrame(DEFAULT_LABEL);
    CVectorFieldFrame* ff = (CVectorFieldFrame*)pDataFrame3->GetUserDataFrame(VECTROFILELDNAME, DEFAULT_LABEL);

	if ((!vf1) || (!vf2) || (!ff))
    {
        return NULL;
    }	
	
	LPBYTE bm2=GetData(vf2);
	
	CVideoFrame* vf= CVideoFrame::Create();
	vf->lpBMIH=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+ vf1->lpBMIH->biSizeImage);
    memcpy(vf->lpBMIH, vf2->lpBMIH, sizeof(BITMAPINFOHEADER)+ vf2->lpBMIH->biSizeImage);

	vf->SetTime((pDataFrame1->GetTime() + pDataFrame2->GetTime()) / 2);
	LPBYTE bm=GetData(vf);
	
	imageSize = vf->lpBMIH->biHeight * vf->lpBMIH->biWidth;
	for(iW = 0; iW < ff->sizeX; iW++)
	{
		for(iH = 0; iH < ff->sizeY; iH++)
		{ 
			move = ff->vfvectors[iH * ff->sizeX + iW];
			moveX = (int) floor(move.x + 0.5);
			moveY = (int) floor(move.y + 0.5);
			if( abs(moveX) > 1 || abs(moveY) > 1)
			{
				//copy moved block;
				iPmove = moveX + moveY * vf1->lpBMIH->biWidth;
				iPmoveHalf = moveX / 2 + (moveY / 2) * vf1->lpBMIH->biWidth;
				iPOffset = (int) (iH * ff->stepY * vf1->lpBMIH->biWidth + iW * ff->stepX);

				if(iPOffset + iPmove < 0 || iPOffset + iPmove + ff->stepX + ff->stepY * vf->lpBMIH->biWidth >= imageSize)
						continue;

				startHalf = iPOffset + iPmoveHalf;
				startSecond = iPOffset + iPmove;
				for(counter = 0; counter < ff->stepY; counter++)
				{
					memcpy(bm + startHalf, bm2 + startSecond, (int) ff->stepX);
					startHalf += vf->lpBMIH->biWidth;
					startSecond += vf->lpBMIH->biWidth;
				}
			}
		}
	}
	
    return vf;
}


// VideoStereoSplitter.cpp: implementation of the VideoStereoSplitter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tvvideo.h"
#include "VideoStereoSplitter.h"
#include <Gadgets\VideoFrame.h>
#include <imageproc\cut.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_RUNTIME_GADGET_EX(VideoStereoSplitter, CGadget, "Video.stereo", TVDB400_PLUGIN_NAME);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VideoStereoSplitter::VideoStereoSplitter():
    m_pInput(0)
{
	m_pInput = new CInputConnector(vframe);
	m_pOutputs[0] = new COutputConnector(vframe);
    m_pOutputs[1] = new COutputConnector(vframe);
	Resume();
}

void VideoStereoSplitter::ShutDown()
{
    CGadget::ShutDown();

    delete m_pInput;
	m_pInput = NULL;
	delete m_pOutputs[0];
	m_pOutputs[0] = NULL;
	delete m_pOutputs[1];
	m_pOutputs[1] = NULL;
}

int VideoStereoSplitter::GetInputsCount()
{
	return 1;
}

int VideoStereoSplitter::GetOutputsCount()
{
	return 2;
}

CInputConnector* VideoStereoSplitter::GetInputConnector(int n)
{
	if ((n >= GetInputsCount()) || (n < 0))
		return NULL;
	return m_pInput;
}

COutputConnector* VideoStereoSplitter::GetOutputConnector(int n)
{
	if ((n >= 2) || (n < 0))
		return NULL;
	return m_pOutputs[n];
}

int VideoStereoSplitter::DoJob()
{
	CDataFrame* pDataFrame = NULL;
	while (m_pInput->Get(pDataFrame))
	{
		ASSERT(pDataFrame);
		CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
		if ((VideoFrame) && (VideoFrame->lpBMIH))
		{
            int width=VideoFrame->lpBMIH->biWidth;
            int height=VideoFrame->lpBMIH->biHeight;
            CRect rect[2];
            rect[0]=CRect(0,0,width/2,height);
            rect[1]=CRect(width/2+1,0,width,height);
			for (int i = 0; i < 2; i++)
			{
				CVideoFrame* ResultFrame = CVideoFrame::Create(_cut_rect(VideoFrame, rect[i]));
				ASSERT(ResultFrame);
                ResultFrame->CopyAttributes(pDataFrame);
				if (!m_pOutputs[i] || !m_pOutputs[i]->Put(ResultFrame))
					ResultFrame->Release(ResultFrame);
			} 
		}
		pDataFrame->Release();
	}
	return WR_CONTINUE;
}
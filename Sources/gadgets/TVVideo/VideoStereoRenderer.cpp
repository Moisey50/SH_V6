// VideoStereoRenderer.cpp: implementation of the VideoStereoRender class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TVVideo.h"
#include "VideoStereoRenderer.h"

#define THIS_MODULENAME "VideoStereoRenderer"

IMPLEMENT_RUNTIME_GADGET_EX(VideoStereoRender, CCollectorRenderer, "Video.renderers.stereo", TVDB400_PLUGIN_NAME);
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VideoStereoRender::VideoStereoRender():
                        m_wndOutput(NULL)
{
    m_FormatErrorProcessed=false;
    m_LastFormat=0;
    CreateInputs(2,vframe);
}

void VideoStereoRender::ShutDown()
{
    CCollectorRenderer::ShutDown();
    Detach();
}

void VideoStereoRender::Create()
{
}

void VideoStereoRender::Attach(CWnd* pWnd)
{
    Detach();
    m_wndOutput=new CStereoView(m_Monitor);
	m_wndOutput->Create(pWnd);
}

void VideoStereoRender::Detach()
{
	if (::IsWindow(m_wndOutput->GetSafeHwnd()))
    {
		m_wndOutput->DestroyWindow();
    }
    if (m_wndOutput)
        delete m_wndOutput; m_wndOutput=NULL;
}

void VideoStereoRender::Render(CDataFrame const*const* frames, int nmb)
{
    const CDataFrame *pDataFrame1=frames[0], *pDataFrame2 =frames[1];

    if (::IsWindow(m_wndOutput->GetSafeHwnd()))
    {

        if ((!Tvdb400_IsEOS(pDataFrame1)) && (!Tvdb400_IsEOS(pDataFrame2)))
        {
	        const CVideoFrame* Frame1 = pDataFrame1->GetVideoFrame(DEFAULT_LABEL);
            const CVideoFrame* Frame2 = pDataFrame2->GetVideoFrame(DEFAULT_LABEL);

            if (m_LastFormat!=Frame1->lpBMIH->biCompression)
            {
                m_LastFormat=Frame1->lpBMIH->biCompression;
                m_FormatErrorProcessed=false;
            }
            if ((m_LastFormat!=BI_YUV9) && (m_LastFormat!=BI_Y8) 
              && (Frame1->lpBMIH->biCompression!=Frame2->lpBMIH->biCompression)) 
            {
                if (!m_FormatErrorProcessed)
                    SENDERR_0("VideoStereoRender can only accept formats YUV9 and Y8");
                m_FormatErrorProcessed=true;
                return;
            }

            if ((Frame1) && (Frame1->lpBMIH) && (Frame2) && (Frame2->lpBMIH))
                m_wndOutput->LoadFrames(Frame1,Frame2);
            else
                m_wndOutput->LoadFrames(NULL,NULL);
        }
    }
}

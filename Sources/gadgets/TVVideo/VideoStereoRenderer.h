// VideoStereoRenderer.h: interface for the VideoStereoRender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIDEOSTEREORENDERER_H__0C574638_90FA_4D92_8F49_28D6F9977285__INCLUDED_)
#define AFX_VIDEOSTEREORENDERER_H__0C574638_90FA_4D92_8F49_28D6F9977285__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <Gadgets\gadbase.h>
#include <Gadgets\VideoFrame.h>
#include <video\stereoview.h>

#define GADGET_STEREOVIDEO_WIDTH	320
#define GADGET_STEREOVIDEO_HEIGHT	240


class VideoStereoRender : public CCollectorRenderer
{
private:
	CStereoView*m_wndOutput;
    bool        m_FormatErrorProcessed;
    DWORD       m_LastFormat;
public:
	VideoStereoRender();
	virtual void ShutDown();
    void    Create();
    void    Attach(CWnd* pWnd);
    void    Detach();
private:
    //virtual void    Render(CDataFrame* pDataFrame1,CDataFrame* pDataFrame2);
    virtual void    Render(CDataFrame const*const* frames, int nmb);
    virtual CWnd*   GetRenderWnd() { return m_wndOutput; }
    virtual void    GetDefaultWndSize (RECT& rc) { rc.left=rc.top=0; rc.right=GADGET_STEREOVIDEO_WIDTH; rc.bottom=GADGET_STEREOVIDEO_HEIGHT; }
	DECLARE_RUNTIME_GADGET(VideoStereoRender);
};

#endif // !defined(AFX_VIDEOSTEREORENDERER_H__0C574638_90FA_4D92_8F49_28D6F9977285__INCLUDED_)

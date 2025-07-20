// StVideoRenderer.h: interface for the CStVideoRenderer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STVIDEORENDERER_H__8B144638_90E2_4268_A523_E1C94028EEEE__INCLUDED_)
#define AFX_STVIDEORENDERER_H__8B144638_90E2_4268_A523_E1C94028EEEE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define GADGET_SVIDEO_WIDTH	    320
#define GADGET_SVIDEO_HEIGHT	240

#include <Gadgets\VideoFrame.h>

class CStVideoRenderer : public CRenderGadget  
{
private:
	CStVideoRenderer();
public:
	virtual ~CStVideoRenderer();
	virtual void Create(CWnd* pWnd);
private:
	virtual void Render(CDataFrame* pDataFrame);
    virtual CWnd*GetRenderWnd() { return NULL; }
    virtual void GetDefaultWndSize (RECT& rc) { rc.left=rc.top=0; rc.right=GADGET_SVIDEO_WIDTH; rc.bottom=GADGET_SVIDEO_HEIGHT; }
	DECLARE_RUNTIME_GADGET(CStVideoRenderer );
};

#endif // !defined(AFX_STVIDEORENDERER_H__8B144638_90E2_4268_A523_E1C94028EEEE__INCLUDED_)

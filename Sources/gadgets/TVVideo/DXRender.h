// BaseRenders.h: interface for the CBaseRenders class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DXRENDER_H__CA778094_866E_410D_8298_36CB79468438__INCLUDED_)
#define AFX_DXRENDER_H__CA778094_866E_410D_8298_36CB79468438__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <shbase\shbase.h>
#include <Gadgets\VideoFrame.h>
#include <userinterface\DIBRender3D.h>

#define DEFAULT_MONOCHROME false
#define DEFAULT_SCALE       (-1)

class DXVideoRender : public CRenderGadget
{
    FXLockObject     m_Lock;
	CDXRender       *m_wndOutput;
    COutputConnector* m_pOutput;
    int             m_Scale;
    bool            m_Monochrome;
    bool            m_LineSelection; // allows line selection
    bool            m_RectSelection; // allowos rectangle selection
    CPoint          m_PointOfInterest;
public:
    void        ShutDown();
    void        Attach(CWnd* pWnd);
    void        Detach();
    int         GetOutputsCount()                { return 1; }
    COutputConnector* GetOutputConnector(int n)   { return (n==0)?m_pOutput:NULL; }
	bool        PrintProperties(FXString& text);
	bool        ScanProperties(LPCTSTR text, bool& Invalidate);
    bool        ScanSettings(FXString& text);
    void        DibEvent(int Event, void *Data);
private:
	            DXVideoRender();
	void        Render(const CDataFrame* pDataFrame);
    CWnd*       GetRenderWnd() { return m_wndOutput; }
    void        GetDefaultWndSize (RECT& rc) { rc.left=rc.top=0; rc.right=GADGET_VIDEO_WIDTH; rc.bottom=GADGET_VIDEO_HEIGHT; }
	DECLARE_RUNTIME_GADGET(DXVideoRender);
};

#endif // !defined(AFX_DXRENDER_H__CA778094_866E_410D_8298_36CB79468438__INCLUDED_)

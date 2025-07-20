// BaseRenders.h: interface for the CBaseRenders class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIDEORENDERS_H__CA778094_866E_410D_8298_36CB79468438__INCLUDED_)
#define AFX_VIDEORENDERS_H__CA778094_866E_410D_8298_36CB79468438__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <shbase\shbase.h>
#include <Gadgets\VideoFrame.h>
#include <userinterface\DIBRender3D.h>

#define DEFAULT_MONOCHROME false
#define DEFAULT_SCALE       (-1)

class VideoRender : public CRenderGadget
{
    FXLockObject     m_Lock;
	CDIBRender     *m_wndOutput;
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
	            VideoRender();
	void        Render(const CDataFrame* pDataFrame);
    CWnd*       GetRenderWnd() { return m_wndOutput; }
    void        GetDefaultWndSize (RECT& rc) { rc.left=rc.top=0; rc.right=GADGET_VIDEO_WIDTH; rc.bottom=GADGET_VIDEO_HEIGHT; }
	DECLARE_RUNTIME_GADGET(VideoRender);
};

class Render3DPlot : public CRenderGadget
{
    FXLockObject     m_Lock;
    CDIBRender3D     *m_wndOutput;
    CDuplexConnector *m_pControl;
    COutputConnector *m_pOutput;
    int              m_Scale;
    bool             m_Monochrome;
    bool             m_LineSelection; // allows line selection
    bool             m_RectSelection; // allowos rectangle selection
    CPoint           m_PointOfInterest;
    int				       m_alpha, m_beta, m_rotZ;
    int				       m_grid;
    int              m_iViewLow , m_iViewHigh ;
public:
    virtual void ShutDown();
    virtual void Attach(CWnd* pWnd);
    virtual void Detach();
    virtual int  GetOutputsCount()                  { return 1; }
    COutputConnector* GetOutputConnector(int n)     { return (n==0)?m_pOutput:NULL; }
    virtual void    AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
    virtual int     GetDuplexCount()                { return 1; }
    virtual CDuplexConnector* GetDuplexConnector(int n) { return (n==0)?m_pControl:NULL; }
    virtual bool ScanSettings(FXString& text);
    virtual bool PrintProperties(FXString& text);
    virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
    void         DibEvent(int Event, void *Data);
private:
    Render3DPlot();
    virtual void Render(const CDataFrame* pDataFrame);
    virtual CWnd*GetRenderWnd() { return m_wndOutput; }
    virtual void GetDefaultWndSize (RECT& rc) { rc.left=rc.top=0; rc.right=GADGET_VIDEO_WIDTH; rc.bottom=GADGET_VIDEO_HEIGHT; }
    DECLARE_RUNTIME_GADGET(Render3DPlot);
};

#endif // !defined(AFX_BASERENDERS_H__CA778094_866E_410D_8298_36CB79468438__INCLUDED_)

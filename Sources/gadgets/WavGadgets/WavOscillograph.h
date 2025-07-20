#ifndef __WAV_OSCILLOGRAPH
#define __WAV_OSCILLOGRAPH

#include "UserInterface\DataFrameView.h"

class COscillWindow : public CDataFrameView
{
public:
	void Render(const CDataFrame* pDataFrame);
	BOOL IsValid() { return ::IsWindow(GetSafeHwnd()); }

};

__forceinline void COscillWindow::Render(const CDataFrame* pDataFrame)
{
	const CWaveFrame* wd=pDataFrame->GetWaveFrame(DEFAULT_LABEL);
    Reset();
    if (!wd || !((CWaveFrame*)wd)->GetData()->waveformat) // empty waveformat means reset
    {
        TRACE("+++ Reset oscillograph\n");
        return;
    }
	 SetDataFrame((CWaveFrame*)wd);
	 Invalidate();
}

class WavOscillograph : public CRenderGadget
{
// 	 CLockObject m_Lock;
    COscillWindow *m_Terminal;
    RenderCallBack m_rcb;
    void*          m_wParam;
public:
    virtual void ShutDown();
	virtual void Create();
	virtual void Attach(CWnd* pWnd);
    virtual void Detach();
    bool SetCallBack(RenderCallBack rcb, void* cbData) { m_rcb=rcb; m_wParam=cbData; return true; }
    CWnd*GetRenderWnd() { return m_Terminal; }
    void GetDefaultWndSize (RECT& rc) { rc.left=rc.top=0; rc.right=320; rc.bottom=240; }
private:
	WavOscillograph();
	virtual void Render(const CDataFrame* pDataFrame);

	DECLARE_RUNTIME_GADGET(WavOscillograph);
};

__forceinline WavOscillograph::WavOscillograph(): 
    m_Terminal(NULL),
    m_rcb(NULL),
    m_wParam(NULL)
{
	m_pInput = new CInputConnector(wave);
	Resume();
}

__forceinline void WavOscillograph::ShutDown()
{
    CRenderGadget::ShutDown();
	delete m_pInput;   m_pInput = NULL;
    delete m_Terminal; m_Terminal=NULL;
}

__forceinline void WavOscillograph::Render(const CDataFrame* pDataFrame)
{
//	 m_Lock.Lock();
    if (m_rcb)
    {
        m_rcb(pDataFrame,m_wParam);
//	 	  m_Lock.Unlock();
        return;
    }
	if ((!m_Terminal) || (!m_Terminal->IsValid())) return;

	m_Terminal->Render(pDataFrame);
//   m_Lock.Unlock();
}

__forceinline void WavOscillograph::Create()
{
    CRenderGadget::Create();
}

__forceinline void WavOscillograph::Attach(CWnd* pWnd)
{
    Detach();
    m_Terminal = new COscillWindow();
	 m_Terminal->Create(pWnd); 
    m_Terminal->SetScaleAuto(true);
    CRenderGadget::Create();
}

__forceinline void WavOscillograph::Detach()
{
	if ((m_Terminal) && (m_Terminal->IsValid()))
    {
		m_Terminal->DestroyWindow();
        delete m_Terminal;
        m_Terminal=NULL;
    }
}

#endif
// TiedDebugRender.h: interface for the CDebugRender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TIEDDEBUGRENDER_H__D354ECBB_B949_4635_86BB_C7C28F753F54__INCLUDED_)
#define AFX_TIEDDEBUGRENDER_H__D354ECBB_B949_4635_86BB_C7C28F753F54__INCLUDED_

#include <gadgets\shkernel.h>
#include <shbase\shbase.h>

class CTiedDebugRender : public CRenderGadget,
                         public FXTimer
{
public:
    enum Mode
    {
        Default,
        Text,
        Video,
        Container,
        FrameRate,
        FrameInfo
    };
private:
    CWnd*           m_pParentWnd;
	CDIBRender*     m_pView;
    CTextView*      m_pStaticView;
    CContainerView* m_pContainerView;
    CWnd*           m_CurrentView;
    Mode            m_DispMode;
    FXLockObject     m_Lock;
// for frame rate cntr
    double m_FrCnt;
    double m_LstTick;
public:
	void DispInfo(const CDataFrame *pDataFrame);
	void SetContainerView();
	virtual void ShutDown();
	virtual void Attach(CWnd* pWnd);
	virtual void Detach();
    virtual CWnd*GetRenderWnd() { return m_pParentWnd; }
            void SetDispMode(Mode mode);
            Mode GetDispMode() { return m_DispMode; };
	        void SetVideoView();
	        void SetStaticView();
    virtual void OnAlarm(DWORD TimeID);
private:
	CTiedDebugRender();
	virtual void Render(const CDataFrame* pDataFrame);

	DECLARE_RUNTIME_GADGET(CTiedDebugRender);
};

#endif //AFX_TIEDDEBUGRENDER_H__D354ECBB_B949_4635_86BB_C7C28F753F54__INCLUDED_
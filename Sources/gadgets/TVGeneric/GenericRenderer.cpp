#include "stdafx.h"
#include "GenericRenderer.h"

CGenericView::CGenericView(GenericRender* gr):
                            m_GR(gr)
{
}

CGenericView::~CGenericView()
{
}


BEGIN_MESSAGE_MAP(CGenericView, CWnd)
	//{{AFX_MSG_MAP(CGenericView)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGenericView message handlers

BOOL CGenericView::Create(CWnd* pParentWnd) 
{
	if (m_hWnd) return(TRUE);
    BOOL RESULT;
    RECT rc;
    pParentWnd->GetClientRect(&rc);
    RESULT=CWnd::Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, rc, pParentWnd, AFX_IDW_PANE_FIRST, NULL);
	UpdateWindow();
	return (RESULT);
}


void CGenericView::OnPaint() 
{
    CPaintDC dc(this);
	RECT rc;
	GetClientRect(&rc);
    dc.FillSolidRect(&rc, RGB(255,255,255));
    m_GR->Draw(&dc,rc);
}

IMPLEMENT_RUNTIME_GADGET_EX(GenericRender, CRenderGadget, LINEAGE_GENERIC, TVDB400_PLUGIN_NAME);

GenericRender::GenericRender(void): 
        m_View(NULL),
        m_Frame(NULL)
{
    m_pInput = new CInputConnector(transparent);
    Resume();
}

void GenericRender::ShutDown()
{
    CRenderGadget::ShutDown();
	delete m_pInput; m_pInput=NULL;
    
    FXAutolock al(m_Lock);
    if (m_Frame)
        m_Frame->Release(m_Frame);
    m_Frame=NULL;

}

void GenericRender::Attach(CWnd* pWnd)
{
    Detach();
	m_View=new CGenericView(this);
	m_View->Create(pWnd);
    m_View->Invalidate();
}

void GenericRender::Detach()
{
    VERIFY(m_Lock.LockAndProcMsgs());
	if (::IsWindow(m_View->GetSafeHwnd()))
	{
		m_View->DestroyWindow();
	}
	if (m_View) delete m_View; m_View=NULL;
    m_Lock.Unlock();
}

void GenericRender::GetDefaultWndSize (RECT& rc) 
{ 
    rc.left=rc.top=0; 
    rc.right=8*DEFAULT_GADGET_WIDTH; 
    rc.bottom=DEFAULT_GADGET_HEIGHT/2; 
}

void GenericRender::Render(const CDataFrame* pDataFrame)
{
    FXAutolock al(m_Lock);
    if (m_Frame)
        m_Frame->Release(m_Frame);
    m_Frame=pDataFrame->Copy();
    //m_Frame->AddRef();
    if (m_View)
        m_View->Invalidate(TRUE);
}

bool GenericRender::Draw(CDC * dc, RECT rc)
{
    FXAutolock al(m_Lock);
    if (m_Frame)
        return m_Frame->Draw(dc, rc);
    return false;
}


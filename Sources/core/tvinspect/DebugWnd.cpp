#include "stdafx.h"
#include "DebugWnd.h"
#include "DebugViewWnd.h"
#include "TvdbDebugView.h"

CDebugWnd::CDebugWnd(IGraphbuilder* pBuilder):
m_pBuilder(pBuilder),
m_pGadgetsBar(NULL),
m_pHostWnd(NULL),
m_pGraphView(NULL),
m_pClientFrame(NULL)
{
}

CDebugWnd::~CDebugWnd()
{
}

BOOL CDebugWnd::Create(CWnd* pHostWnd)
{
	m_pHostWnd = pHostWnd;
	LPCTSTR cName = ::AfxRegisterWndClass(CS_PARENTDC, ::LoadCursor(NULL, IDC_ARROW), NULL);
	LPCTSTR wName = _T("Debug window");
	DWORD dwStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE;
	if (!CFrameWnd::Create(cName, wName, dwStyle, rectDefault, pHostWnd))
		return FALSE;
	EnableDocking(CBRS_ALIGN_ANY);

	CCreateContext cc;
	::ZeroMemory(&cc, sizeof(cc));
	cc.m_pNewViewClass = RUNTIME_CLASS(CWnd);
	m_pClientFrame = CreateView(&cc);
	m_pClientFrame->ShowWindow(SW_HIDE);
	m_pGraphView = new CGraphViewDlg(this, NULL);
	m_pGraphView->Create(IDD_GRAPH_VIEW_DLG, this);
	m_pGraphView->OpenView(m_pBuilder);
	m_pGraphView->ShowWindow(SW_SHOW);
    m_pGraphView->SetParent(this);
    m_pGadgetsBar = Tvdb400_GetGadgetsBar();
	if (!m_pGadgetsBar->Create(this, "Gadgets", WS_CHILD | WS_VISIBLE | CBRS_LEFT
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC))
	{
		TRACE0("Failed to create gadgets bar\n");
		return -1;      // fail to create
	}
	GET_BAR(m_pGadgetsBar)->EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT );
    DockControlBar(GET_BAR(m_pGadgetsBar));
    m_pGadgetsBar->SetShowType(m_pGadgetsBar->IsTypeShown(), m_pBuilder);
	RecalcLayout();
  m_pHostWnd->SendMessage( WM_PARENTNOTIFY , WM_CREATE , (LPARAM) this->m_hWnd );	

	return TRUE;
}

BEGIN_MESSAGE_MAP(CDebugWnd, CFrameWnd)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_REGISTERED_MESSAGE(VM_TVDB400_REGISTERDRAGDROP, OnRegisterDragDrop)
//    ON_WM_CHANGEUISTATE()
//ON_WM_TIMER()
END_MESSAGE_MAP()

void CDebugWnd::OnDestroy()
{
  if ( m_pHostWnd && ::IsWindow( m_pHostWnd->GetSafeHwnd() ) )
    m_pHostWnd->SendMessage( WM_PARENTNOTIFY , WM_DESTROY , (LPARAM) m_hWnd );	
                                                  
  if ( m_pGadgetsBar )
		m_pGadgetsBar->Release();
	m_pGadgetsBar = NULL;
	CFrameWnd::OnDestroy();
	if (m_pClientFrame)
		delete m_pClientFrame;
}

void CDebugWnd::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);

	if (m_pGraphView && ::IsWindow(m_pGraphView->GetSafeHwnd()))
	{
		//RecalcLayout();
		CRect rc;
		m_pClientFrame->GetClientRect(rc);
		m_pClientFrame->ClientToScreen(rc);
		ScreenToClient(rc);
		m_pGraphView->MoveWindow(rc);
	}
}

LRESULT CDebugWnd::OnRegisterDragDrop(WPARAM wParam, LPARAM lParam)
{
	m_pGraphView->RegisterActiveDragDrop();
	return 0;
}

BOOL CDebugWnd::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    if (message==WM_NOTIFY)
    {
        // hacking!
        // somebody send notifycation abt change docking state, who knows...
        CRect rc;
        m_pClientFrame->GetClientRect(rc);
        this->SendMessage(WM_SIZE,rc.right,rc.bottom);

        //TRACE("0x%x\n",this);
        //TRACE("0x%x 0x%x 0x%x\n",message,wParam,lParam);
    }
    return CFrameWnd::OnWndMsg(message, wParam, lParam, pResult);
}



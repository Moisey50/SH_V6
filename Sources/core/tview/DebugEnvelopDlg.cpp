// DebugEnvelopDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DebugEnvelopDlg.h"
#include "TiedDebugRender.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDebugEnvelopDlg dialog


CDebugEnvelopDlg::CDebugEnvelopDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDebugEnvelopDlg::IDD, pParent),
	m_pOutFrame(NULL),
	m_pInFrame(NULL),
    m_pInGadget(NULL),
    m_pOutGadget(NULL),
    m_pGadget(NULL)
{
    m_hWndTop=FALSE;
	//{{AFX_DATA_INIT(CDebugEnvelopDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDebugEnvelopDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDebugEnvelopDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

typedef struct _tagModes
{
    unsigned ID;
    CTiedDebugRender::Mode mode;
}Modes;


Modes menuItems[]={
                    {IDM_DEFAULT,   CTiedDebugRender::Default},
                    {IDM_FRAMERATE, CTiedDebugRender::FrameRate},
                    {IDM_TEXT,      CTiedDebugRender::Text},
                    {IDM_VIDEO,     CTiedDebugRender::Video},
                    {IDM_CONTAINERVIEW,CTiedDebugRender::Container}
                  };

BEGIN_MESSAGE_MAP(CDebugEnvelopDlg, CDialog)
	//{{AFX_MSG_MAP(CDebugEnvelopDlg)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_RBUTTONUP()
	ON_COMMAND(IDM_DEFAULT, OnDefault)
	ON_COMMAND(IDM_FRAMERATE, OnFramerate)
	ON_COMMAND(IDM_TEXT, OnText)
	ON_COMMAND(IDM_VIDEO, OnVideo)
	ON_COMMAND(IDM_CONTAINERVIEW, OnContainerview)
    ON_MESSAGE(UM_CHECKSTATECHANGE, OnCheckStateChange) 
	ON_COMMAND(IDM_FRAMEINFO, OnFrameinfo)
	//}}AFX_MSG_MAP
    ON_WM_MOVE()
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void CDebugEnvelopDlg::ArrangeWindows()
{
	CRect rc;
	GetClientRect(rc);
	if (m_pOutFrame && m_pOutFrame->IsWindowVisible())
	{
		if (m_pInFrame && m_pInFrame->IsWindowVisible())
		{
			CRect rc1 = rc;
			rc1.right /= 2;
			m_pOutFrame->MoveWindow(rc1);
			rc.left = rc1.right + 1;
			m_pInFrame->MoveWindow(rc);
		}
		else
			m_pOutFrame->MoveWindow(rc);
	}
	else if (m_pInFrame && m_pInFrame->IsWindowVisible())
	{
		m_pInFrame->MoveWindow(rc);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDebugEnvelopDlg message handlers

void CDebugEnvelopDlg::OnCancel() 
{
	DestroyWindow();
}

void CDebugEnvelopDlg::OnOK() 
{
}

void CDebugEnvelopDlg::PostNcDestroy() 
{
	delete this;
}

BOOL CDebugEnvelopDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
    CRect rc;
    GetClientRect(rc);
	m_pOutFrame = new CStaticFrame;
	m_pOutFrame->Create("",WS_CHILD|WS_BORDER|WS_VISIBLE,rc,this,0);
	//m_pOutFrame->SetScale(-1);
	m_pInFrame = new CStaticFrame;
	m_pInFrame->Create("",WS_CHILD|WS_BORDER|WS_VISIBLE,rc,this,1);
	//m_pInFrame->SetScale(-1);
    
    VERIFY(m_Menu.LoadMenu(IDR_TIREDDLGMENU));

	ArrangeWindows();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDebugEnvelopDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	m_pOutFrame->DestroyWindow();
    delete m_pOutFrame;
    m_pOutFrame=NULL;
	
	m_pOutFrame = NULL;
	m_pInFrame->DestroyWindow();
	delete m_pInFrame;
	m_pInFrame = NULL;
}

void CDebugEnvelopDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	ArrangeWindows();
}


void CDebugEnvelopDlg::OnRButtonUp(UINT nFlags, CPoint point) 
{
//    TOOLINFO ti;
    //OnToolHitTest(point,&ti);
    if (((HWND)(size_t)nFlags)==m_pInFrame->m_hWnd)
    {
        m_pGadget=m_pInGadget;
        TRACE("+++ m_pInGadget\n");
    }
    else if (((HWND) (size_t) nFlags)==m_pOutFrame->m_hWnd)
    {
        m_pGadget=m_pOutGadget;
        TRACE("+++ m_pOutGadget\n");
    }
    else
    {
        m_pGadget=NULL;
        CRect inRect,outRect,Rect;
        m_pInFrame->GetWindowRect(inRect);
        m_pOutFrame->GetWindowRect(outRect);
        GetWindowRect(Rect);
        inRect-=CPoint(Rect.left,Rect.top);
        outRect-=CPoint(Rect.left,Rect.top);
        if ((inRect.PtInRect(point)) && m_pInGadget)
            m_pGadget=m_pInGadget;
        else if (outRect.PtInRect(point))
            m_pGadget=m_pOutGadget;
        else
        {
            TRACE("Unknown region fo RButtonUp in CDebugEnvelopDlg\n");
        }
    }

    CMenu* pPopup = m_Menu.GetSubMenu(0);
    
    ASSERT(pPopup != NULL);
    
    if(m_pGadget==NULL) return;

    int menuItmCnt=sizeof(menuItems)/sizeof(Modes);
    for (int i=0; i<menuItmCnt; i++)
    {
        pPopup->CheckMenuItem(menuItems[i].ID,(menuItems[i].mode==m_pGadget->GetDispMode())?MF_CHECKED:MF_UNCHECKED);
    }
    CRect rc;
    GetWindowRect(rc);
    CPoint pos=point+CPoint(rc.left,rc.top);
    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, this);

	CDialog::OnRButtonUp(nFlags, point);
}

void CDebugEnvelopDlg::OnDefault() 
{
    if (m_pGadget)
        m_pGadget->SetDispMode(CTiedDebugRender::Default);
}

void CDebugEnvelopDlg::OnFramerate() 
{
    if (m_pGadget)
        m_pGadget->SetDispMode(CTiedDebugRender::FrameRate);
}

void CDebugEnvelopDlg::OnText() 
{
    if (m_pGadget)
        m_pGadget->SetDispMode(CTiedDebugRender::Text);
}

void CDebugEnvelopDlg::OnVideo() 
{
    if (m_pGadget)
        m_pGadget->SetDispMode(CTiedDebugRender::Video);
}

void CDebugEnvelopDlg::OnContainerview() 
{
    if (m_pGadget)
        m_pGadget->SetDispMode(CTiedDebugRender::Container);
}

void CDebugEnvelopDlg::OnFrameinfo() 
{
    if (m_pGadget)
        m_pGadget->SetDispMode(CTiedDebugRender::FrameInfo);
}

LRESULT CDebugEnvelopDlg::OnCheckStateChange(WPARAM wParam, LPARAM lParam)
{
    HTREEITEM   hItem =(HTREEITEM)lParam;
    //TRACE("Item %d (%s) will be changed (%s)\n",hItem,GetItemText(hItem),(GetCheck(hItem))?"Checked":"Unchecked");
    //GetParent()->SendMessage(UM_CHECKSTATECHANGE, wParam, lParam);
    return 0;
}

void CDebugEnvelopDlg::OnMove(int x, int y)
{
    CDialog::OnMove(x, y);
    Invalidate();
}

void CDebugEnvelopDlg::OnMouseMove(UINT nFlags, CPoint point)
{
    BringWindowToTop();
    Invalidate();
    CDialog::OnMouseMove(nFlags, point);
}

// GraphViewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ImCNTLDlg.h"
#include "GraphViewDlg.h"


DlgItem GVDI[] =
{
    {IDC_TABVIEW,BOTTOM_ALIGN|RIGHT_ALIGN,{0,0,0,0},0xffffffff},
    {IDC_RUN,BOTTOM_MOVE,{0,0,0,0}, 0xffffffff },
    {IDC_STOP,BOTTOM_MOVE,{0,0,0,0}, 0xffffffff },
    {IDC_CLOSE_TAB,BOTTOM_MOVE,{0,0,0,0}, 0xffffffff }
};

// CGraphViewDlg dialog

IMPLEMENT_DYNAMIC(CGraphViewDlg, CDialog)

CGraphViewDlg::CGraphViewDlg(CWnd* pParent /*=NULL*/, IGraphbuilder* Builder)
	: CResizableDialog(CGraphViewDlg::IDD, pParent),
	m_bOLEInitialized(FALSE)
{
}

CGraphViewDlg::~CGraphViewDlg()
{
}

void CGraphViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TABVIEW, m_Tabs);
}


BEGIN_MESSAGE_MAP(CGraphViewDlg, CResizableDialog)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_RUN, &CGraphViewDlg::OnBnClickedRun)
	ON_BN_CLICKED(IDC_STOP, &CGraphViewDlg::OnBnClickedStop)
	ON_REGISTERED_MESSAGE(VM_TVDB400_CREATENEWVIEW, OnCreateNewView)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TABVIEW, &CGraphViewDlg::OnTcnSelchangeTabview)
	ON_NOTIFY(TCN_SELCHANGING, IDC_TABVIEW, &CGraphViewDlg::OnTcnSelchangingTabview)
	ON_BN_CLICKED(IDC_CLOSE_TAB, &CGraphViewDlg::OnBnClickedCloseTab)
END_MESSAGE_MAP()


// CGraphViewDlg message handlers

void CGraphViewDlg::OnCancel()
{
	DestroyWindow();
}

void CGraphViewDlg::OnDestroy()
{
	CResizableDialog::OnDestroy();
	int cItems = m_DebugTemplates.GetSize();
	while (cItems--)
	{
		CDebugDocTemplate ddt = m_DebugTemplates.GetAt(cItems);
		ddt.pView->Release();
		ddt.pBuilder->Release();
		m_DebugTemplates.RemoveAt(cItems);
	}
	if (m_bOLEInitialized)
		OleUninitialize();
}

void CGraphViewDlg::PostNcDestroy()
{
	CResizableDialog::PostNcDestroy();
	delete this;
}

BOOL CGraphViewDlg::OnInitDialog()
{
	CResizableDialog::OnInitDialog(GVDI, sizeof(GVDI) / sizeof(DlgItem));

	EnableFitting();
	FitDialog();

	return TRUE;
}

void CGraphViewDlg::OnSize(UINT nType, int cx, int cy)
{
	CResizableDialog::OnSize(nType, cx, cy);
	if (::IsWindow(m_Tabs.GetSafeHwnd()))
	{
		ISketchView* View = GetActiveView();
		if (View)
			RepositionView(View);
	}
}

void CGraphViewDlg::OpenView(IGraphbuilder* pBuilder, LPCTSTR title)
{
	CString Name = title;
	if (Name.IsEmpty())
		Name = pBuilder->GetID();
	if (Name.IsEmpty())
		Name = _T("Root");
	ISketchView* pView = Tvdb400_GetSketchView();
	if (!pView)
		return;
	if (!GET_WND(pView)->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		pView->Release();
		return;
	}
	pBuilder->AddRef();
	pView->SetBuilder(pBuilder);
	pView->ComposeGraph();
	pView->SetTargetWnd(this);

	CDebugDocTemplate ddt;
	ddt.pBuilder = pBuilder;
	ddt.pView = pView;
	m_DebugTemplates.Add(ddt);

	ISketchView* LastView = GetActiveView();
	if (LastView)
		GET_WND(LastView)->ShowWindow(SW_HIDE);

	int nItem = m_Tabs.GetItemCount();
	m_Tabs.InsertItem(nItem, Name);
	m_Tabs.SetCurSel(nItem);
	GetDlgItem(IDC_CLOSE_TAB)->EnableWindow(nItem);

	RepositionView(pView);
}

void CGraphViewDlg::RepositionView(ISketchView* View)
{
	CRect rc;
	m_Tabs.GetClientRect(rc);
	m_Tabs.AdjustRect(FALSE, rc);
	m_Tabs.ClientToScreen(rc);
	ScreenToClient(rc);
	GET_WND(View)->MoveWindow(rc);
}

ISketchView* CGraphViewDlg::GetActiveView()
{
	int nSel = m_Tabs.GetCurSel();
	if (nSel < 0 || nSel >= m_DebugTemplates.GetSize())
		return NULL;
	return m_DebugTemplates.GetAt(nSel).pView;
}

IGraphbuilder* CGraphViewDlg::GetActiveBuilder()
{
	int nSel = m_Tabs.GetCurSel();
	if (nSel < 0 || nSel >= m_DebugTemplates.GetSize())
		return NULL;
	return m_DebugTemplates.GetAt(nSel).pBuilder;
}

void CGraphViewDlg::RegisterActiveDragDrop()
{
	if (m_bOLEInitialized)
		OleUninitialize();
	OleInitialize(0);
	ISketchView* View = GetActiveView();
	if (View)
		View->RegisterSketchDragDrop();
	m_bOLEInitialized = TRUE;
}

void CGraphViewDlg::OnBnClickedRun()
{
	IGraphbuilder* pBuilder = GetActiveBuilder();
	if (pBuilder)
		pBuilder->Start();
}


void CGraphViewDlg::OnBnClickedStop()
{
	IGraphbuilder* pBuilder = GetActiveBuilder();
	if (pBuilder)
		pBuilder->Stop();
}

LRESULT CGraphViewDlg::OnCreateNewView(WPARAM wParam, LPARAM lParam)
{
	if (wParam == IGraphbuilder::TVDB400_GT_COMPLEX)
	{
		CString path = ((LPCREATEGRAPHVIEW)lParam)->path;
		IGraphbuilder* pBuilder = ((LPCREATEGRAPHVIEW)lParam)->builder;
		OpenView(pBuilder, path);
	}
	return 0;
}

void CGraphViewDlg::OnTcnSelchangeTabview(NMHDR *pNMHDR, LRESULT *pResult)
{
	ISketchView* View = GetActiveView();
	if (View)
		GET_WND(View)->ShowWindow(SW_SHOW);
	int iItem = m_Tabs.GetCurSel();
	GetDlgItem(IDC_CLOSE_TAB)->EnableWindow(iItem);
	*pResult = 0;
}

void CGraphViewDlg::OnTcnSelchangingTabview(NMHDR *pNMHDR, LRESULT *pResult)
{
	ISketchView* View = GetActiveView();
	if (View)
		GET_WND(View)->ShowWindow(SW_HIDE);
	*pResult = 0;
}

void CGraphViewDlg::OnBnClickedCloseTab()
{
	int iItem = m_Tabs.GetCurSel();
	if (iItem < 0)
		return;
	m_Tabs.DeleteItem(iItem);
	ASSERT(iItem < m_DebugTemplates.GetCount());
	CDebugDocTemplate ddt = m_DebugTemplates.GetAt(iItem);
	ddt.pView->Release();
	ddt.pBuilder->Release();
	m_DebugTemplates.RemoveAt(iItem);
	ASSERT(iItem > 0);
	m_Tabs.SetCurSel(iItem - 1);
	ISketchView* View = GetActiveView();
	if (View)
		GET_WND(View)->ShowWindow(SW_SHOW);
}

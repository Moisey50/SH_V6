// GadgetsTreeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GadgetsTreeDlg.h"
#include "SketchView.h"
#include "DragDrop.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGadgetsTreeDlg dialog


CGadgetsTreeDlg::CGadgetsTreeDlg(CWnd* pParent /*=NULL*/, BOOL bUserLibrary)
	: CDialog(CGadgetsTreeDlg::IDD, pParent),
	m_pDragItem(NULL),
	m_hDragged(NULL),
	m_IView(NULL),
	m_bUserLibrary(bUserLibrary)
{
	//{{AFX_DATA_INIT(CGadgetsTreeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CGadgetsTreeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGadgetsTreeDlg)
	DDX_Control(pDX, IDC_GADGETS_TREE, m_GadgetsTree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGadgetsTreeDlg, CDialog)
	//{{AFX_MSG_MAP(CGadgetsTreeDlg)
	ON_NOTIFY(TVN_BEGINDRAG, IDC_GADGETS_TREE, OnBegindragGadgetsTree)
	ON_NOTIFY(TVN_SELCHANGED, IDC_GADGETS_TREE, OnSelchangedGadgetsTree)
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGadgetsTreeDlg message handlers

void CGadgetsTreeDlg::OnBegindragGadgetsTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMTREEVIEW pInfo = (LPNMTREEVIEW)pNMHDR;
	HTREEITEM hItem = pInfo->itemNew.hItem;
	if (!m_GadgetsTree.IsGroup(hItem))
	{
			//OLEDRAGDROP
			CFrameWnd* pFrame = GetParentFrame();
			pFrame->SendMessage(VM_TVDB400_REGISTERDRAGDROP, NULL, NULL);
			m_hDragged = hItem;

			FORMATETC fmtetc = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
			STGMEDIUM stgmed = { TYMED_HGLOBAL, { 0 }, 0 };

			if (!m_bUserLibrary)
			{
				CString Class = m_GadgetsTree.GetItemText(m_hDragged);
				stgmed.hGlobal = StringToHandle(Class.GetBuffer(), -1);
			}
			else
			{
				LPCTSTR path = (LPCTSTR)m_GadgetsTree.GetItemData(m_hDragged);
				CString uid = m_GadgetsTree.GetItemText(m_hDragged);
				uid += _T("1");
				//uid = uid.Mid(1);
				CString params;
				params.Format("Complex(%s):\"%s\"", uid, path);
				stgmed.hGlobal = StringToHandle(params.GetBuffer(), -1);
			}

			IDataObject *pDataObject;
			IDropSource *pDropSource;
			CDropSource::CreateDropSource(&pDropSource);
			CDataObject::CreateDataObject(&fmtetc, &stgmed, 1, &pDataObject);
			DWORD dwResult, dwEffect;
			dwResult = DoDragDrop(pDataObject, pDropSource, DROPEFFECT_COPY, &dwEffect);
			pDropSource->Release();
			pDataObject->Release();
			ReleaseStgMedium(&stgmed);
	}
	*pResult = 0;
}

void CGadgetsTreeDlg::OnSelchangedGadgetsTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	*pResult = 0;
}

void CGadgetsTreeDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	if (!::IsWindow(m_GadgetsTree.GetSafeHwnd()))
		return;
	CRect rc;
	GetClientRect(rc);
	m_GadgetsTree.MoveWindow(rc);
}

void CGadgetsTreeDlg::PostNcDestroy() 
{
	if (m_pDragItem)
		delete m_pDragItem;
	delete this;
}

void CGadgetsTreeDlg::OnCancel()
{
}

BOOL CGadgetsTreeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if (m_bUserLibrary)
		m_GadgetsTree.ShowGroupByMedia(true);

	if (m_GadgetsTree.CreateItemIcons())
		m_GadgetsTree.CreateRootItems();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGadgetsTreeDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_pDragItem)
	{
		ClientToScreen(&point);
		m_pDragItem->DragMove(point);
	}
	CDialog::OnMouseMove(nFlags, point);
}

void CGadgetsTreeDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_pDragItem)
	{
		m_pDragItem->DragLeave(NULL);
		m_pDragItem->EndDrag();
		ReleaseCapture();
		ShowCursor(TRUE);
		delete m_pDragItem;
		m_pDragItem = NULL;
		if (m_hDragged && m_IView)
		{
			ClientToScreen(&point);
			CSketchView* pView = (CSketchView*)GET_WND(m_IView);
			if (!m_bUserLibrary)
			{
				CString Class = m_GadgetsTree.GetItemText(m_hDragged);
				if (pView)
					pView->InsertGadget(Class, point);
			}
			else
			{
				LPCTSTR path = (LPCTSTR)m_GadgetsTree.GetItemData(m_hDragged);
				CString uid = m_GadgetsTree.GetItemText(m_hDragged);
				uid += _T("1");
				uid = uid.Mid(1);
				if (pView)
				{
					CString params;
					params.Format("\"%s\"", path);
					pView->InsertGadget("Complex", point, params, uid);
				}
			}
		}
	}
	CDialog::OnLButtonUp(nFlags, point);
}

BOOL CGadgetsTreeDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    HTREEITEM hItem = m_GadgetsTree.GetSelectedItem();
    if ((hItem != NULL) && (!m_GadgetsTree.ItemHasChildren(hItem)))
    {
        CString uid = m_GadgetsTree.GetItemText(hItem);
        HTREEITEM prev=m_GadgetsTree.GetParentItem(hItem);
        while (prev)
        {
            CString prevID = m_GadgetsTree.GetItemText(prev);
            uid=prevID+"."+uid;
            prev=m_GadgetsTree.GetParentItem(prev);
        }
        ShowHelp(ID_CONTEXT_HELP,uid);
        return TRUE;
    }
	return FALSE;
}

// SelectGadgetClassDlg.cpp : implementation file
//

#include "stdafx.h"
#include <gadgets\shkernel.h>
#include "SelectGadgetClassDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectGadgetClassDlg dialog


CSelectGadgetClassDlg::CSelectGadgetClassDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectGadgetClassDlg::IDD, pParent),
	m_GadgetClass(""),
	m_pGadgetTypes(NULL),
	m_pGadgetClasses(NULL),
	m_pGadgetLineages(NULL)
{
	//{{AFX_DATA_INIT(CSelectGadgetClassDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CSelectGadgetClassDlg::SetGadgetsInfo(CUIntArray* Types, CStringArray* Classes, CStringArray* Lineages)
{
	m_pGadgetTypes = Types;
	m_pGadgetClasses = Classes;
	m_pGadgetLineages = Lineages;
}

void CSelectGadgetClassDlg::BuildTree()
{
	ASSERT(m_pGadgetTypes->GetSize() == m_pGadgetClasses->GetSize());
	ASSERT(m_pGadgetTypes->GetSize() == m_pGadgetLineages->GetSize());
	for (int i = 0; i < m_pGadgetTypes->GetSize(); i++)
	{
		CString Class = m_pGadgetClasses->GetAt(i);
		CString Lineage = m_pGadgetLineages->GetAt(i);
		switch (m_pGadgetTypes->GetAt(i))
		{
		case IGraphbuilder::TVDB400_GT_CAPTURE:
			m_GadgetsTree.InsertItem(CGadgetsTree::GTI_CAPTURES, Lineage, Class);
			break;
		case IGraphbuilder::TVDB400_GT_FILTER:
			m_GadgetsTree.InsertItem(CGadgetsTree::GTI_FILTERS, Lineage, Class);
			break;
		case IGraphbuilder::TVDB400_GT_CTRL:
			m_GadgetsTree.InsertItem(CGadgetsTree::GTI_CTRLS, Lineage, Class);
			break;
		default:
			m_GadgetsTree.InsertItem(CGadgetsTree::GTI_OTHER, Lineage, Class);
			break;
		}
	}
}

void CSelectGadgetClassDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectGadgetClassDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectGadgetClassDlg, CDialog)
	//{{AFX_MSG_MAP(CSelectGadgetClassDlg)
	ON_NOTIFY(TVN_SELCHANGED, IDC_GADGETS_TREE, OnGadgetsTreeSelChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectGadgetClassDlg message handlers

BOOL CSelectGadgetClassDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CRect rc;
	GetDlgItem(IDC_TREE_FRAME)->GetClientRect(rc);
//	m_GadgetsTree.Create(this, rc);
//	if (m_pGadgetTypes && m_pGadgetClasses && m_pGadgetLineages)
//		BuildTree();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectGadgetClassDlg::OnGadgetsTreeSelChanged(NMHDR* pNotifyStruct, LRESULT* result)
{
	LPNMTREEVIEW pInfo = (LPNMTREEVIEW)pNotifyStruct;
	HTREEITEM hItem = pInfo->itemNew.hItem;
	BOOL isGroupItem = m_GadgetsTree.IsGroup(hItem);
	if (isGroupItem)
		m_GadgetsTree.SetItemState(hItem, 0, TVIS_SELECTED);
	else
		m_GadgetClass = m_GadgetsTree.GetItemText(pInfo->itemNew.hItem);
	GetDlgItem(IDOK)->EnableWindow(!isGroupItem);
	*result = 0;
}

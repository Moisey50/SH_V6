// GadgetsTree.cpp : implementation file
//

#include "stdafx.h"
#include "GadgetsTree.h"
#include "resource.h"
#include "fxfc/FXRegistry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGadgetsTree

CGadgetsTree::CGadgetsTree()
{
	ZeroMemory(m_hRootItem, sizeof(m_hRootItem));
	m_ItemLabel[GTI_CAPTURES] = "Captures";
	m_ItemLabel[GTI_FILTERS] = "Filters";
	m_ItemLabel[GTI_CTRLS] = "Controls";
	m_ItemLabel[GTI_SPLITTERS] = "Splitters";
	m_ItemLabel[GTI_OTHER] = "Other";

  FXRegistry Reg( "TheFileX\\SHStudio" ) ;
  m_GroupByMedia=(Reg.GetRegiInt("GadgetTree","ShowTypes",TRUE)!=FALSE);
}

CGadgetsTree::~CGadgetsTree()
{
  FXRegistry Reg( "TheFileX\\SHStudio" ) ;
  Reg.WriteRegiInt( "GadgetTree" , "ShowTypes" , m_GroupByMedia ) ;
//   AfxGetApp()->WriteProfileInt( "GadgetTree" , "ShowTypes" , m_GroupByMedia );
}


BEGIN_MESSAGE_MAP(CGadgetsTree, CTreeCtrl)
	//{{AFX_MSG_MAP(CGadgetsTree)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

HTREEITEM CGadgetsTree::EnsureGroup(HTREEITEM hRoot, LPCTSTR group)
{
	HTREEITEM hItem = FindSubGroup(hRoot, group);
	if (!hItem)
	{
		hItem = CTreeCtrl::InsertItem(group, GTI_COUNT, GTI_COUNT, hRoot);
		SortChildren(hRoot);
	}
	return hItem;
}

HTREEITEM CGadgetsTree::FindSubGroup(HTREEITEM hRoot, LPCTSTR group)
{
	HTREEITEM hItem = GetChildItem(hRoot);
	while (hItem)
	{
		if ((GetItemText(hItem) == group) && IsGroup(hItem))
			return hItem;
		hItem = GetNextItem(hItem, TVGN_NEXT);
	}
	return NULL;
}

HTREEITEM CGadgetsTree::FindItem(HTREEITEM hRoot, LPCTSTR name, LPVOID lpData)
{
	HTREEITEM hItem = GetChildItem(hRoot);
	while (hItem)
	{
		if (!IsGroup(hItem))
		{
			if ((GetItemText(hItem) == name) && (GetItemData(hItem) == (DWORD_PTR)lpData))
				return hItem;
		}
		hItem = GetNextItem(hItem, TVGN_NEXT);
	}
	return NULL;
}

BOOL CGadgetsTree::DeleteBranch(HTREEITEM hItem)
{
	HTREEITEM hGroup = GetParentItem(hItem);
	if (!hGroup)
		return TRUE;
	if (!CTreeCtrl::DeleteItem(hItem))
		return FALSE;
	if (ItemHasChildren(hGroup))
		return TRUE;
	return DeleteBranch(hGroup);
}

HTREEITEM CGadgetsTree::InsertItem(UINT type, LPCTSTR lineage, LPCTSTR item)
{
	if (type >= GTI_COUNT)
		return NULL;
	HTREEITEM hGroup = m_hRootItem[type];
	while (strlen(lineage))
	{
		CString group(lineage);
		int dotPos = group.Find('.');
		if (dotPos >= 0)
		{
			group = group.Left(dotPos);
			lineage += dotPos + 1;
		}
		else
			lineage += strlen(lineage);
		hGroup = EnsureGroup(hGroup, group);
		if (!hGroup)
			return NULL;
	}
	//ASSERT(hGroup);
	if (!hGroup)
		hGroup = TVI_ROOT;
	HTREEITEM hItem = CTreeCtrl::InsertItem(item, type, type + GTI_COUNT + 1, hGroup);
	SortChildren(hGroup);
	return hItem;
}

BOOL CGadgetsTree::DeleteItem(UINT type, LPCTSTR lineage, LPCTSTR item, LPVOID lpData)
{
	if (type >= GTI_COUNT)
		return NULL;
	HTREEITEM hGroup = m_hRootItem[type];
	while (hGroup && strlen(lineage))
	{
		CString group(lineage);
		int dotPos = group.Find('.');
		if (dotPos >= 0)
		{
			group = group.Left(dotPos);
			lineage += dotPos + 1;
		}
		else
			lineage += strlen(lineage);
		hGroup = FindSubGroup(hGroup, group);
	}
	if (!hGroup)
		return FALSE;
	HTREEITEM hItem = FindItem(hGroup, item, lpData);
	if (!hItem)
		return FALSE;
	return DeleteBranch(hItem);
}

BOOL CGadgetsTree::DeleteItem(LPCTSTR lineage, LPCTSTR item, LPVOID lpData)
{
	for (UINT type = 0; type < GTI_COUNT; type++)
		if (DeleteItem(type, lineage, item, lpData))
			return TRUE;
	return FALSE;
}

BOOL CGadgetsTree::DeleteAllItems()
{
    memset(m_hRootItem,0,sizeof(m_hRootItem));
    return CTreeCtrl::DeleteAllItems();
}

BOOL CGadgetsTree::IsGroup(HTREEITEM hItem)
{
	for (int i = 0; i < GTI_COUNT; i++)
		if (hItem == m_hRootItem[i])
			return TRUE;
	return ItemHasChildren(hItem);
}

/////////////////////////////////////////////////////////////////////////////
// CGadgetsTree message handlers
/*
BOOL CGadgetsTree::Create(CWnd* pParentWnd, const RECT& rect) 
{
	if (!m_Icons.Create(IDB_GADGET_ICONS, 16, 5, RGB(0, 128, 128)))
		return FALSE;
	DWORD style = TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;
	style |= WS_VISIBLE | WS_TABSTOP | WS_CHILD | WS_BORDER;
	if (!CTreeCtrl::Create(style, rect, pParentWnd, IDC_GADGETS_TREE))
		return FALSE;
	SetImageList(&m_Icons, TVSIL_NORMAL);
    CreateRootItems();
	TRACE(" ------ CGadgetTree::Create ------\n");
	TRACE("  pParentWnd = 0x%x (HWND = 0x%x)\n", pParentWnd, pParentWnd->GetSafeHwnd());
	return TRUE;
}
*/

BOOL CGadgetsTree::CreateItemIcons()
{
	if (!m_Icons.Create(IDB_GADGET_ICONS, 16, 5, RGB(0, 128, 128)))
		return FALSE;
	SetImageList(&m_Icons, TVSIL_NORMAL);
	return TRUE;
}


void CGadgetsTree::CreateRootItems()
{
    if (!m_GroupByMedia)
    {
	    for (int i = 0; i < GTI_COUNT; i++)
	    {
		    m_hRootItem[i] = CTreeCtrl::InsertItem(m_ItemLabel[i], i, i + GTI_COUNT + 1);
		    if (!m_hRootItem[i])
			    return;
	    } 
    }
}

BOOL CGadgetsTree::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style &= ~TVS_DISABLEDRAGDROP;
	return CTreeCtrl::PreCreateWindow(cs);
}

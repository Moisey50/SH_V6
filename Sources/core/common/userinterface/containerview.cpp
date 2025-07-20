// ContainerView.cpp : implementation file
//

#include "stdafx.h"
#include <shbase\shbase.h>
#include <windowsx.h>
#include <gadgets\arrayframe.h>
#include <gadgets\containerframe.h>
#include <gadgets\quantityframe.h>
#include <gadgets\rectframe.h>
#include <gadgets\textframe.h>
#include <gadgets\videoframe.h>
#include <gadgets\waveframe.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_CONTAINERTREE 1000


/////////////////////////////////////////////////////////////////////////////
// CContainerView

CContainerView::CContainerView():
    m_IsAboutToQuit(false)
{
}

CContainerView::~CContainerView()
{
}


BEGIN_MESSAGE_MAP(CContainerView, CTreeCtrl)
	//{{AFX_MSG_MAP(CContainerView)
	ON_WM_DESTROY()
	ON_WM_RBUTTONDOWN()
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
    ON_MESSAGE(UM_CHECKSTATECHANGE, OnCheckStateChange) 
	//}}AFX_MSG_MAP
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CContainerView message handlers

BOOL CContainerView::Create(CWnd* pParentWnd)
{
	CRect rc;
	pParentWnd->GetClientRect(rc);
	return CTreeCtrl::Create(WS_VISIBLE | WS_TABSTOP | WS_CHILD | WS_BORDER | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_CHECKBOXES, rc, pParentWnd, IDC_CONTAINERTREE);
}

void CContainerView::OnDestroy( )
{
    m_Lock.LockAndProcMsgs();
    m_IsAboutToQuit=true;
	CTreeCtrl::OnDestroy();
    m_Lock.Unlock();
}

void CContainerView::DescribeFrame(const CDataFrame* pDataFrame, CString& txt, CFramesIterator*& Iterator)
{
	Iterator = pDataFrame->CreateFramesIterator(nulltype);
	if (Iterator)
		txt.Format("CContainerFrame #%d %s", pDataFrame->GetId(), pDataFrame->GetLabel());
	else
	{
		txt.Empty();
		CString info;
		datatype dtype = pDataFrame->GetDataType();
		if (Tvdb400_TypesCompatible(dtype, arraytype))
		{
			const CArrayFrame* ArrayFrame = pDataFrame->GetArrayFrame(pDataFrame->GetLabel());
			CString strCount = _T("some");
			if (ArrayFrame)
				strCount.Format("%d", ArrayFrame->GetCount());
			dtype /= arraytype;
			info.Format("CArrayFrame #%d %s  {%s of %s}", pDataFrame->GetId(), pDataFrame->GetLabel(), strCount, Tvdb400_TypeToStr(dtype));
			txt += info;
		}
        else if (dtype==transparent || dtype == nulltype)
        {
			info.Format("CDataFrame #%d %s ", pDataFrame->GetId(), pDataFrame->GetLabel());
			txt += info;
        }
		else if (Tvdb400_TypesCompatible(dtype, vframe))
		{
            char fcc[5]; 
            memcpy(fcc,&(pDataFrame->GetVideoFrame(DEFAULT_LABEL)->lpBMIH->biCompression),4);
            fcc[4]=0;
			info.Format("CVideoFrame #%d(%s) %s ", pDataFrame->GetId(),fcc,pDataFrame->GetLabel());
			txt += info;
		}
		else if (Tvdb400_TypesCompatible(dtype, text))
		{
			info.Format("CTextFrame #%d %s [\"%s\"] ", pDataFrame->GetId(), pDataFrame->GetLabel(), pDataFrame->GetTextFrame(NULL)->GetString());
			txt += info;
		}
		else if (Tvdb400_TypesCompatible(dtype, wave))
		{
			info.Format("CWaveFrame #%d %s ", pDataFrame->GetId(), pDataFrame->GetLabel());
			txt += info;
		}
		else if (Tvdb400_TypesCompatible(dtype, quantity))
		{
			info.Format("CQuantityFrame #%d %s [%s] ", pDataFrame->GetId(), pDataFrame->GetLabel(), pDataFrame->GetQuantityFrame(NULL)->ToString());
			txt += info;
		}
		else if (Tvdb400_TypesCompatible(dtype, figure))
		{
			info.Format("CFigureFrame #%d %s [%s] ", pDataFrame->GetId(), pDataFrame->GetLabel(), pDataFrame->GetFigureFrame(NULL)->ToString());
			txt += info;
		}
		else if (Tvdb400_TypesCompatible(dtype, logical))
		{
			info.Format("CBooleanFrame #%d %s [%s] ", pDataFrame->GetId(), pDataFrame->GetLabel(), (pDataFrame->GetBooleanFrame(NULL)->operator bool() ? "true" : "false"));
			txt += info;
		}
		else if (Tvdb400_TypesCompatible(dtype, rectangle))
		{
			CRect rc = pDataFrame->GetRectFrame(NULL);
			info.Format("CRectFrame #%d %s [(%d, %d) - (%d, %d)] ", pDataFrame->GetId(), pDataFrame->GetLabel(), rc.left, rc.top, rc.right, rc.bottom);
			txt += info;
		}
		else if (Tvdb400_TypesCompatible(dtype, metafile))
		{
			info.Format("CMetafileFrame #%d", pDataFrame->GetId());
			txt += info;
		}
		else if (Tvdb400_TypesCompatible(dtype, userdata))
		{
            const CUserDataFrame* udf=NULL;
			if (udf=pDataFrame->GetUserDataFrame(NULL,DEFAULT_LABEL))
			{
				info.Format("CUserDataFrame (%s) #%d", udf->GetUserType(), pDataFrame->GetId());
				txt += info;
			}
		}
	}
}

bool CContainerView::CompareDescriptions(LPCTSTR dsc1, LPCTSTR dsc2)
{
	char* pos1 = (char*)strchr(dsc1, (int)' ');
	char* pos2 = (char*)strchr(dsc2, (int)' ');
	if (!pos1 || !pos2)
		return true;
	FXSIZE len1 = pos1 - dsc1;
	FXSIZE len2 = pos2 - dsc2;
	if (len1 != len2)
		return true;
	if (!len1)
		return false;
	return (memcmp(dsc1, dsc2, len1) != 0);
}

void CContainerView::CutUnselected(CDataFrame* pDataFrame, CFramesIterator* Iterator, HTREEITEM hItem)
{
	HTREEITEM hChild = GetChildItem(hItem);
	CDataFrame* pFrame = Iterator->Next(NULL);
	CPtrArray CutFrames;
	while (pFrame)
	{
		ASSERT(hChild);
		if (!GetCheck(hChild))
		{
			CFramesIterator* pChildIterator = pFrame->CreateFramesIterator(nulltype);
			if (pChildIterator)
			{
				CutUnselected(pFrame, pChildIterator, hChild);
				delete pChildIterator;
			}
			else
				CutFrames.Add(pFrame);
		}
		pFrame = Iterator->NextChild(NULL);
		hChild = GetNextItem(hChild, TVGN_NEXT);
	}
	while (CutFrames.GetSize())
	{
		pFrame = (CDataFrame*)CutFrames.GetAt(0);
		CutFrames.RemoveAt(0);
		pDataFrame->Release(pFrame);
	}
}

HTREEITEM CContainerView::DoRender(HTREEITEM hParentItem, HTREEITEM hItem, CDataFrame* pDataFrame)
{
	CString txt;
	CFramesIterator* Iterator = NULL;
	DescribeFrame(pDataFrame, txt, Iterator);
	while (hItem)
	{
		if (!CompareDescriptions(txt, GetItemText(hItem)))
			break;
		HTREEITEM tmpItem = GetNextItem(hItem, TVGN_NEXT);
		DeleteItem(hItem);
		hItem = tmpItem;
	}
	if (hItem)
		SetItemText(hItem, txt);
	else
		hItem = InsertItem(txt, hParentItem);
	if (Iterator)
	{
		CDataFrame* pChild = Iterator->NextChild(NULL);
		HTREEITEM hChild = GetChildItem(hItem);
		while (pChild)
		{
			hChild = GetNextItem(DoRender(hItem, hChild, pChild), TVGN_NEXT);
			pChild = Iterator->NextChild(NULL);
		}
		delete Iterator;
		if (hChild)
		{
			HTREEITEM tmpItem = GetNextItem(hChild, TVGN_NEXT);
			while (tmpItem)
			{
				DeleteItem(tmpItem);
				tmpItem = GetNextItem(hChild, TVGN_NEXT);
			}
			DeleteItem(hChild);
		}
	}
	return hItem;
}

void CContainerView::Render(const CDataFrame* pDataFrame)
{
    m_Lock.LockAndProcMsgs();
	if ((m_IsAboutToQuit) || (!GetSafeHwnd()))
	{
        m_Lock.Unlock();
		return;
	}
	if (Tvdb400_IsEOS(pDataFrame))
		DeleteAllItems();
	else
	{
		CString txt;
		CFramesIterator* Iterator = NULL;
		DescribeFrame(pDataFrame, txt, Iterator);
		HTREEITEM hRoot = GetRootItem();
		if (hRoot)
		{
			if (CompareDescriptions(txt, GetItemText(hRoot)))
			{
				DeleteAllItems();
				hRoot = NULL;
			}
			else
				SetItemText(hRoot, txt);
		}
		if (!hRoot)
			hRoot = InsertItem(txt, NULL);
		if (Iterator)
		{
			CDataFrame* pChild = Iterator->NextChild(NULL);
			HTREEITEM hItem = GetChildItem(hRoot);
			while (pChild)
			{
				hItem = GetNextItem(DoRender(hRoot, hItem, pChild), TVGN_NEXT);
				pChild = Iterator->NextChild(NULL);
			}
			delete Iterator;
			if (hItem)
			{
				HTREEITEM tmpItem = GetNextItem(hItem, TVGN_NEXT);
				while (tmpItem)
				{
					DeleteItem(tmpItem);
					tmpItem = GetNextItem(hItem, TVGN_NEXT);
				}
				DeleteItem(hItem);
			}
		}
		Expand(hRoot,TVE_EXPAND);
		SetCheck(hRoot);
	}
    m_Lock.Unlock();
}

CDataFrame* CContainerView::CutUnselected(const CDataFrame* pDataFrame)
{
	HTREEITEM hRoot = GetRootItem();
	if (!hRoot)
		return NULL;
    CDataFrame* retV=pDataFrame->Copy();
	if (GetCheck(hRoot))
		return retV;
	CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(nulltype);
	if (!Iterator)
		return NULL;
	CutUnselected(retV, Iterator, hRoot);
	delete Iterator;
	return retV;
}

BOOL CContainerView::DestroyWindow() 
{
	return CTreeCtrl::DestroyWindow();
}

void CContainerView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	GetParent()->SendMessage(WM_RBUTTONUP,nFlags,MAKELPARAM(point.x,point.y)); // Forward message up to the parent
	CTreeCtrl::OnRButtonDown(nFlags, point);
}

void CContainerView::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
    TVHITTESTINFO ht = {0};
	if (pNMHDR->idFrom==IDC_CONTAINERTREE)
    {
        DWORD dwpos = GetMessagePos();

        // include <windowsx.h> and <windows.h> header files
        ht.pt.x = GET_X_LPARAM(dwpos);
        ht.pt.y = GET_Y_LPARAM(dwpos);
        ::MapWindowPoints(HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1);

        TreeView_HitTest(pNMHDR->hwndFrom, &ht);
        if(TVHT_ONITEMSTATEICON & ht.flags)
        {
            PostMessage(UM_CHECKSTATECHANGE, 0, (LPARAM)ht.hItem);
        }
    }
	*pResult = 0;
}

LRESULT CContainerView::OnCheckStateChange(WPARAM wParam, LPARAM lParam)
{
    HTREEITEM   hItem =(HTREEITEM)lParam;
    //TRACE("Item %d (%s) will be changed (%s)\n",hItem,GetItemText(hItem),(GetCheck(hItem))?"Checked":"Unchecked");
    return GetParent()->SendMessage(UM_CHECKSTATECHANGE, wParam, lParam);
}

void CContainerView::OnMouseMove(UINT nFlags, CPoint point)
{
    GetParent()->SendMessage(WM_MOUSEMOVE,nFlags,MAKELPARAM(point.x,point.y)); // Forward message up to the parent
    CTreeCtrl::OnMouseMove(nFlags, point);
}

// GraphTreeComposer.h: interface for the CGraphTreeComposer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRAPHTREECOMPOSER_H__E4CB866B_7669_4667_99C0_9606C01DFA30__INCLUDED_)
#define AFX_GRAPHTREECOMPOSER_H__E4CB866B_7669_4667_99C0_9606C01DFA30__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GraphComposer.h"
#include "StringTree.h"
#include "TreeTools.h"

// Helpers

__forceinline BOOL RemoveElement(CString& item, CStringArray& array)
{
	for (int i = 0; i < array.GetSize(); i++)
	{
		if (item == array.GetAt(i))
		{
			array.RemoveAt(i);
			return TRUE;
		}
	}
	return FALSE;
}


class CGraphTreeComposer : public CGraphComposer
{
public:
	CGraphTreeComposer() {};
	virtual void ComposeGraph(IGraphbuilder* GraphBuilder, CStringArray* SrcGadgets = NULL, CStringArray* DstGadgets = NULL);

private:
	void AddTreeItems(CStringArray& srcGadgets, CStringArray& dstGadgets, IGraphbuilder* GraphBuilder, CStringTree* Tree);
	void BuildGraphTree(IGraphbuilder* GraphBuilder, CStringTree* Tree, CStringArray* SrcGadgets, CStringArray* DstGadgets);
	void ArrangeGraphTree(CStringTree* Tree, int curDepth, int& maxDepth, int& maxWidth);
};

__forceinline void CGraphTreeComposer::ComposeGraph(IGraphbuilder* GraphBuilder, CStringArray* SrcGadgets, CStringArray* DstGadgets)
{
	CStringTree GraphTree;
	BuildGraphTree(GraphBuilder, &GraphTree, SrcGadgets, DstGadgets);
	TREE_DumpTree(&GraphTree);
	int maxDepth = 0, maxWidth = 0;
	ArrangeGraphTree(&GraphTree, 0, maxDepth, maxWidth);
}

__forceinline void CGraphTreeComposer::AddTreeItems(CStringArray& srcGadgets, CStringArray& dstGadgets, IGraphbuilder* GraphBuilder, CStringTree* Tree)
{
	CStringArray curGadgets;
	curGadgets.Append(srcGadgets);
	while(srcGadgets.GetSize())
	{
		Tree->AddSibling(srcGadgets[0]);
		srcGadgets.RemoveAt(0);
	}
	while(curGadgets.GetSize())
	{
		while(curGadgets.GetSize())
		{
			FXString uidGadget = (FXString) curGadgets.GetAt(0);
			CStringArray uidRecievers;
			GraphBuilder->EnumGadgetLinks(uidGadget, uidRecievers);
			CStringTree* Branch = Tree->FindItem(uidGadget);
			while (uidRecievers.GetSize())
			{
				CString uidChild = uidRecievers.GetAt(0);
				if (RemoveElement(uidChild, dstGadgets))
				{
					srcGadgets.Add(uidChild);
					Branch->AddChild(uidGadget, uidChild);
				}
				uidRecievers.RemoveAt(0);
			}
			curGadgets.RemoveAt(0);
		}
		curGadgets.Append(srcGadgets);
		srcGadgets.RemoveAll();
	}
}

__forceinline void CGraphTreeComposer::BuildGraphTree(IGraphbuilder* GraphBuilder, CStringTree* Tree, CStringArray* SrcGadgets, CStringArray* DstGadgets)
{
	CStringArray srcGadgets;
	CStringArray dstGadgets;
	if (!SrcGadgets || !DstGadgets)
		GraphBuilder->EnumGadgets(srcGadgets, dstGadgets);
	else
	{
		srcGadgets.Append(*SrcGadgets);
		dstGadgets.Append(*DstGadgets);
	}
	// add regular branches
	AddTreeItems(srcGadgets, dstGadgets, GraphBuilder, Tree);
	// add cyclic and broken branches if any
	while (dstGadgets.GetSize())
	{
		srcGadgets.Add(dstGadgets.GetAt(0));
		dstGadgets.RemoveAt(0);
		AddTreeItems(srcGadgets, dstGadgets, GraphBuilder, Tree);
	}
}

__forceinline void CGraphTreeComposer::ArrangeGraphTree(CStringTree* Tree, int curDepth, int& maxDepth, int& maxWidth)
{
	CStringArray siblings;
	Tree->EnumSiblings(siblings);
	while (siblings.GetSize())
	{
		CString item = siblings.GetAt(0);
		siblings.RemoveAt(0);
		if (curDepth > maxDepth)
			AddGadgetPosition(item, new CPoint(maxDepth = curDepth, maxWidth));
		else
			AddGadgetPosition(item, new CPoint(maxDepth = curDepth, ++maxWidth));
		CStringTree* Children = Tree->GetChildren(item);
		if (Children)
			ArrangeGraphTree(Children, curDepth + 1, maxDepth, maxWidth);
	}
}

#endif // !defined(AFX_GRAPHTREECOMPOSER_H__E4CB866B_7669_4667_99C0_9606C01DFA30__INCLUDED_)

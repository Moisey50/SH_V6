#ifndef _TREE_TOOLS_INCLUDED
#define _TREE_TOOLS_INCLUDED

#include "StringTree.h"


__forceinline int TREE_GetLevel(CStringTree* Tree)
{
	int level = 0;
	while (Tree->GetParent())
	{
		level++;
		Tree = Tree->GetParent();
	}
	return level;
}

__forceinline bool TREE_AreForefatherAndOffspring(CStringTree* Tree1, CStringTree* Tree2)
{
	Tree2 = Tree2->GetParent();
	while (Tree2)
	{
		if (Tree1 == Tree2)
			return true;
		Tree2 = Tree2->GetParent();
	}
	return false;
}

#ifdef _DEBUG
inline void TREE_DumpTree(CStringTree* Tree)
{
	CStringArray siblings;
	Tree->EnumSiblings(siblings);
	int level = TREE_GetLevel(Tree);
	CString tab(' ', level);
	for (int i = 0; i < siblings.GetSize(); i++)
	{
		CString sibling = siblings.GetAt(i);
		TRACE("%s-%s\n", tab, sibling);
		CStringTree* Children = Tree->GetChildren(sibling);
		if (Children)
			TREE_DumpTree(Children);
	}
}
#else
 #define TREE_DumpTree(Tree)
#endif

#endif
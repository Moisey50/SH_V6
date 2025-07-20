// StringTree.cpp: implementation of the CStringTree class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StringTree.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStringTree::CStringTree(CStringTree* pParentTree):
m_pParent(pParentTree)
{
}

CStringTree::~CStringTree()
{
	RemoveAll();
}

CStringTree* CStringTree::GetRoot()
{
	CStringTree* Root = this;
	while (Root->GetParent())
		Root = Root->GetParent();
	return Root;
}

void CStringTree::EnumSiblings(CStringArray& siblings)
{
	siblings.RemoveAll();
	POSITION pos = m_Siblings.GetStartPosition();
	CString name;
	void* children;
	while (pos)
	{
		m_Siblings.GetNextAssoc(pos, name, children);
		siblings.Add(name);
	}
}

CStringTree* CStringTree::GetChildren(LPCTSTR sibling)
{
	CStringTree* Children = NULL;
	if (!m_Siblings.Lookup(sibling, (void*&)Children))
		return NULL;
	return Children;
}

CStringTree* CStringTree::FindItem(LPCTSTR sibling)
{
	CStringTree* Children;
	if (m_Siblings.Lookup(sibling, (void*&)Children))
		return this;
	POSITION pos = m_Siblings.GetStartPosition();
	while (pos)
	{
		CString name;
		m_Siblings.GetNextAssoc(pos, name, (void*&)Children);
		if (!Children)
			continue;
		CStringTree* Result = Children->FindItem(sibling);
		if (Result)
			return Result;
	}
	return NULL;
}

BOOL CStringTree::AddSibling(LPCTSTR sibling)
{
	if (GetChildren(sibling))
		return FALSE;
	m_Siblings.SetAt(sibling, NULL);
	return TRUE;
}

BOOL CStringTree::AddChild(LPCTSTR sibling, LPCTSTR child)
{
	CStringTree* Children = GetChildren(sibling);
	if (!Children)
	{
		Children = new CStringTree(this);
		m_Siblings.SetAt(sibling, Children);
	}
	return Children->AddSibling(child);
}

void CStringTree::RemoveSibling(LPCTSTR sibling)
{
	RemoveChildren(sibling);
	m_Siblings.RemoveKey(sibling);
}

void CStringTree::RemoveChildren(LPCTSTR sibling)
{
	CStringTree* Children = GetChildren(sibling);
	if (Children)
	{
		delete Children;
		m_Siblings.SetAt(sibling, NULL);
	}
}

void CStringTree::RemoveChild(LPCTSTR sibling, LPCTSTR child)
{
	CStringTree* Children = GetChildren(sibling);
	if (Children)
		Children->RemoveSibling(child);
}

void CStringTree::RemoveAll()
{
	POSITION pos = m_Siblings.GetStartPosition();
	CString sibling;
	CStringTree* Children;
	while (pos)
	{
		m_Siblings.GetNextAssoc(pos, sibling, (void*&)Children);
		if (Children)
			delete Children;
		m_Siblings.SetAt(sibling, NULL);
	}
	m_Siblings.RemoveAll();
}

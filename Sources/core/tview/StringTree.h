// StringTree.h: interface for the CStringTree class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRINGTREE_H__141C300D_A4E6_4F3C_B367_7DA966795308__INCLUDED_)
#define AFX_STRINGTREE_H__141C300D_A4E6_4F3C_B367_7DA966795308__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CStringTree
{
private:
	CStringTree*	m_pParent;
	CMapStringToPtr	m_Siblings;
public:
	CStringTree(CStringTree* pParentTree = NULL);
	virtual ~CStringTree();

	CStringTree*	GetParent() { return m_pParent; };
	CStringTree*	GetRoot();
	void			EnumSiblings(CStringArray& siblings);
	CStringTree*	GetChildren(LPCTSTR sibling);
	CStringTree*	FindItem(LPCTSTR sibling);

	BOOL			AddSibling(LPCTSTR sibling);
	BOOL			AddChild(LPCTSTR sibling, LPCTSTR child);
	void			RemoveSibling(LPCTSTR sibling);
	void			RemoveChildren(LPCTSTR sibling);
	void			RemoveChild(LPCTSTR sibling, LPCTSTR child);
	void			RemoveAll();
};

#endif // !defined(AFX_STRINGTREE_H__141C300D_A4E6_4F3C_B367_7DA966795308__INCLUDED_)

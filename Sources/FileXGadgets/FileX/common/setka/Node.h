// Node.h: interface for the CNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NODE_H__0D45E42D_516E_403B_AF4A_9134262D6E86__INCLUDED_)
#define AFX_NODE_H__0D45E42D_516E_403B_AF4A_9134262D6E86__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Lines.h"
#include "AfxTempl.h"

typedef struct tagLINENODE
{
	double	m_x;
	double	m_y;
	CLine*	m_VLine;
	CLine*	m_HLine;
	double	m_refX;
	double	m_refY;
}NODE;

class CNodes
{
public:
	CNodes();
	virtual ~CNodes();

	void	AddNode(double x, double y, CLine* VLine, CLine* m_HLine);
	int		GetCount();
	NODE	GetAt(int index);
	void	SetAt(int index, NODE& node);
	void	RemoveAll();

private:
	CArray<NODE, NODE&> m_Nodes;
};

#endif // !defined(AFX_NODE_H__0D45E42D_516E_403B_AF4A_9134262D6E86__INCLUDED_)

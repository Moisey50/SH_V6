// Node.cpp: implementation of the CNode class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Node.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNodes::CNodes()
{
	m_Nodes.SetSize(0, 1);
}

CNodes::~CNodes()
{
}

void CNodes::AddNode(double x, double y, CLine* VLine, CLine* HLine)
{
	NODE node;
	::ZeroMemory(&node, sizeof(node));
	node.m_x = x;
	node.m_y = y;
	node.m_VLine = VLine;
	node.m_HLine = HLine;
	m_Nodes.Add(node);
}

int CNodes::GetCount()
{
	return (int)m_Nodes.GetSize();
}

NODE CNodes::GetAt(int index)
{
	return m_Nodes.GetAt(index);
}

void CNodes::SetAt(int index, NODE& node)
{
	m_Nodes.SetAt(index, node);
}

void CNodes::RemoveAll()
{
	m_Nodes.RemoveAll();
}

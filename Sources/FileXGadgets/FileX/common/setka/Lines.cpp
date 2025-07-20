// Lines.cpp: implementation of the CLines class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Lines.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLines::CLines()
{
	m_Lines.SetSize(0, 1);
}

CLines::~CLines()
{
	RemoveAll();
}

CLine* CLines::AddLine()
{
	CLine* Line = new CLine;
	Line->SetSize(0, 1);
	m_Lines.Add(Line);
	return Line;
}

void CLines::MoveLine(int indexSrc, int indexDst)
{
	CLine* Line = m_Lines.GetAt(indexSrc);
	if (indexSrc > indexDst)
	{
		m_Lines.RemoveAt(indexSrc);
		m_Lines.InsertAt(indexDst, Line);
	}
	else
	{
		m_Lines.InsertAt(indexDst, Line);
		m_Lines.RemoveAt(indexSrc);
	}
}

CLine* CLines::GetAt(int index)
{
	return m_Lines.GetAt(index);
}

int CLines::Index(CLine* Line)
{
	for (int i = 0; i < GetCount(); i++)
		if (GetAt(i) == Line)
			return i;
	return -1;
}

void CLines::RemoveAt(int index, int count)
{
	while (count--)
	{
		CLine* Line = GetAt(index);
		delete Line;
		m_Lines.RemoveAt(index);
	}
}

int CLines::GetCount()
{
	return (int) m_Lines.GetSize();
}

void CLines::RemoveAll()
{
	RemoveAt(0, GetCount());
}

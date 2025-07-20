// DibRect.cpp: implementation of the CDibRect class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TVVideo.h"
#include "DibRect.h"
#include "VideoCutRectDialog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDibRect::CDibRect():
m_bHasRect(FALSE),
m_pClient(NULL)
{
}

CDibRect::~CDibRect()
{
}

BEGIN_MESSAGE_MAP(CDibRect, CDibVSq)
	//{{AFX_MSG_MAP(CDibVSq)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CDibRect::AddSquare(CSquare* Square)
{
	if (m_bHasRect)
	{
		delete Square;
		return;
	}
	m_bHasRect = TRUE;
	CDibVSq::AddSquare(Square);
}

void CDibRect::SetClient(void* pClient)
{
	m_pClient = pClient;
}


void CDibRect::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDibVSq::OnLButtonDown(nFlags, point);
	if (m_pClient)
	{
		CSquare* Square = GetSquare(0);
		CRect rect = Square->GetRect();
		((VideoCutRectDialog*)m_pClient)->OnRectChanged(&rect);
	}
}

void CDibRect::OnMouseMove(UINT nFlags, CPoint point)
{
	CDibVSq::OnMouseMove(nFlags, point);
	if (m_pClient)
	{
		CSquare* Square = GetSquare(0);
		CRect rect = Square->GetRect();
		((VideoCutRectDialog*)m_pClient)->OnRectChanged(&rect);
	}
}

void CDibRect::OnLButtonUp(UINT nFlags, CPoint point)
{
	CDibVSq::OnLButtonUp(nFlags, point);
	if (m_pClient)
	{
		CSquare* Square = GetSquare(0);
		CRect rect = Square->GetRect();
		((VideoCutRectDialog*)m_pClient)->OnRectChanged(&rect);
	}
}

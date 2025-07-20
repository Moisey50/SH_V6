// DibVSq.cpp : implementation file
//

#include "stdafx.h"
#include "DibVSq.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BKGRND (RGB(50,50,80))

/////////////////////////////////////////////////////////////////////////////
// CDibVSq

CDibVSq::CDibVSq():
m_pSDI(NULL),
m_bDrawSquares(TRUE),
m_CurHitType(HTNOWHERE),
CDIBView("CDibVSq")
{
	InitSelBlock(false, false);
	m_Font.CreateFont(-10, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS, "Arial");
}

CDibVSq::~CDibVSq()
{
	m_Font.DeleteObject();
}


BEGIN_MESSAGE_MAP(CDibVSq, CDIBView)
	//{{AFX_MSG_MAP(CDibVSq)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CDibVSq::GetSquareLabelXRect(CRect& Rect, CSquare* Square, CDC& dc)
{
	CRect rc = Square->GetRect();
	CFont* oldFont = dc.SelectObject(&m_Font);
	CString text;
	text.Format("X0 = %d", rc.left);
	CSize sz = dc.GetTextExtent(text);
	int width = sz.cx;
	int height = sz.cy;
	text.Format("Y0 = %d", rc.top);
	sz = dc.GetTextExtent(text);
	if (sz.cx > width) width = sz.cx;
	dc.SelectObject(oldFont);
	RectPic2Scr(rc);
	Rect.left = rc.left - 3 * width / 4;
	Rect.top = rc.top - height - sz.cy;
	Rect.right = Rect.left + width;
	Rect.bottom = Rect.top + height;
}

void CDibVSq::GetSquareLabelYRect(CRect& Rect, CSquare* Square, CDC& dc)
{
	CRect rc = Square->GetRect();
	CFont* oldFont = dc.SelectObject(&m_Font);
	CString text;
	text.Format("X0 = %d", rc.left);
	CSize sz = dc.GetTextExtent(text);
	int width = sz.cx;
	int height = sz.cy;
	text.Format("Y0 = %d", rc.top);
	sz = dc.GetTextExtent(text);
	if (sz.cx > width) width = sz.cx;
	dc.SelectObject(oldFont);
	RectPic2Scr(rc);
	Rect.left = rc.left - 3 * width / 4;
	Rect.top = rc.top - sz.cy;
	Rect.right = Rect.left + sz.cx;
	Rect.bottom = Rect.top + sz.cy;
}

void CDibVSq::GetSquareLabelWRect(CRect& Rect, CSquare* Square, CDC& dc)
{
	CRect rc = Square->GetRect();
	CFont* oldFont = dc.SelectObject(&m_Font);
	CString text;
	text.Format("%d", rc.Width());
	CSize sz = dc.GetTextExtent(text);
	dc.SelectObject(oldFont);
	RectPic2Scr(rc);
	Rect.top = rc.bottom;
	Rect.bottom = Rect.top + sz.cy;
	Rect.left = (rc.left + rc.right - sz.cx) / 2;
	Rect.right = Rect.left + sz.cx;
	Rect.OffsetRect(0, 3);
}

void CDibVSq::GetSquareLabelHRect(CRect& Rect, CSquare* Square, CDC& dc)
{
	CRect rc = Square->GetRect();
	CFont* oldFont = dc.SelectObject(&m_Font);
	CString text;
	text.Format("%d", rc.Height());
	CSize sz = dc.GetTextExtent(text);
	dc.SelectObject(oldFont);
	RectPic2Scr(rc);
	Rect.left = rc.right;
	Rect.right = Rect.left + sz.cx;
	Rect.top = (rc.top + rc.bottom - sz.cy) / 2;
	Rect.bottom = Rect.top + sz.cy;
	Rect.OffsetRect(3, 0);
}

void CDibVSq::InvalidateSquare(CSquare* Square, CDC& dc, BOOL bErase)
{
	CRect rc = Square->GetRect();
	InvalidateRect(rc, bErase);
	GetSquareLabelXRect(rc, Square, dc);
	CWnd::InvalidateRect(rc, bErase);
	GetSquareLabelYRect(rc, Square, dc);
	CWnd::InvalidateRect(rc, bErase);
	GetSquareLabelWRect(rc, Square, dc);
	CWnd::InvalidateRect(rc, bErase);
	GetSquareLabelHRect(rc, Square, dc);
	CWnd::InvalidateRect(rc, bErase);
}

void CDibVSq::DrawSquare(CSquare* Square, CDC& dc)
{
	CRect rc = Square->GetRect();
	int x = rc.left, y = rc.top, w = rc.Width(), h = rc.Height();
	COLORREF color = Square->GetColor();
	CPen* OldPen = dc.SelectObject(m_GDI.GetPen(PS_SOLID, 1, color));
	CFont* OldFont = dc.SelectObject(&m_Font);
	int bkMode = dc.SetBkMode(TRANSPARENT);
	COLORREF txtColor = dc.SetTextColor(color);
	RectPic2Scr(rc);

    CPoint pl[5];
    pl[0]=rc.TopLeft();
    pl[1]=CPoint(rc.right, rc.top);
    pl[2]=rc.BottomRight();
    pl[3]=CPoint(rc.left, rc.bottom);
    pl[4]=rc.TopLeft();
    dc.Polyline(pl,5);

	CString text;
	GetSquareLabelXRect(rc, Square, dc);
	text.Format("X0 = %d", x);
	dc.TextOut(rc.left, rc.top, text);
	GetSquareLabelYRect(rc, Square, dc);
	text.Format("Y0 = %d", y);
	dc.TextOut(rc.left, rc.top, text);
	GetSquareLabelWRect(rc, Square, dc);
	text.Format("%d", w);
	dc.TextOut(rc.left, rc.top, text);
	GetSquareLabelHRect(rc, Square, dc);
	text.Format("%d", h);
	dc.TextOut(rc.left, rc.top, text);
	dc.SetTextColor(txtColor);
	dc.SetBkMode(bkMode);
	dc.SelectObject(OldFont);
	dc.SelectObject(OldPen);
}

void CDibVSq::UpdateCursor(UINT hitType)
{
	if (hitType == m_CurHitType)
		return;
	m_CurHitType = hitType;
	switch (hitType)
	{
	case HTNOWHERE:
		SetClassLongPtr(GetSafeHwnd(), GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_ARROW));
		break;
	case HTCLIENT:
		SetClassLongPtr(GetSafeHwnd(), GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_SIZEALL));
		break;
	case HTTOPLEFT:
	case HTBOTTOMRIGHT:
		SetClassLongPtr(GetSafeHwnd(), GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_SIZENWSE));
		break;
	case HTTOPRIGHT:
	case HTBOTTOMLEFT:
		SetClassLongPtr(GetSafeHwnd(), GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_SIZENESW));
		break;
	case HTTOP:
	case HTBOTTOM:
		SetClassLongPtr(GetSafeHwnd(), GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_SIZENS));
		break;
	case HTLEFT:
	case HTRIGHT:
		SetClassLongPtr(GetSafeHwnd(), GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_SIZEWE));
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDibVSq message handlers

void CDibVSq::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	RECT rc;
	bool wasDrawn=false;


	GetClientRect(&rc);
	wasDrawn=Draw(dc.m_hDC,rc);
	if (!wasDrawn) 
		dc.FillSolidRect(&rc, BKGRND);
	else if (m_bDrawSquares)
	{
		CRect rc(0, 0, 0, 0);
        GetImageSize(rc.right, rc.bottom);
		RectPic2Scr(rc);
		CRgn rgn;
		rgn.CreateRectRgnIndirect(rc);
		dc.SelectClipRgn(&rgn);
		for (int i = 0; i < m_Squares.GetSize(); i++)
		{
			CSquare* Square = m_Squares.GetAt(i);
			if (Square)
				DrawSquare(Square, dc);
		}
		dc.SelectClipRgn(NULL);
		rgn.DeleteObject();
	}
	if (m_DrawEx) m_DrawEx(dc.m_hDC,rc,this,m_DrawExParam);
}

void CDibVSq::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_pSDI)
		delete m_pSDI;
	m_pSDI = NULL;
	CPoint pt = point;
	Scr2Pic(pt);
	CSquare* Square = NULL;
	UINT hitType = HTBOTTOMRIGHT;
	BOOL bNewSquare = (nFlags & MK_CONTROL);
	if (bNewSquare)
	{
		CRect rc(pt.x - 1, pt.y - 1, pt.x + 1, pt.y + 1);
		Square = new CSquare(rc, RGB(0, 0, 255));
		//m_Squares.Add(Square);
		AddSquare(Square);
		Invalidate(FALSE);
	}
	else	
		Square = m_Squares.HitTest(pt, hitType);
	if (Square)
	{
		m_pSDI = new CSquareDragInfo(Square, hitType, pt, bNewSquare);
		UpdateCursor(hitType);
		SetCapture();
	}
	CDIBView::OnLButtonDown(nFlags, point);
}

void CDibVSq::OnMouseMove(UINT nFlags, CPoint point) 
{
	CPoint pt = point;
	Scr2Pic(pt);
	if (m_pSDI)
	{
		CClientDC dc(this);
		CSquare* Square = m_pSDI->GetSquare();
		InvalidateSquare(Square, dc, FALSE);
		Square->Transform(m_pSDI->GetShift(pt), m_pSDI->GetHitType());
		CRect rc(0, 0, 0, 0);
        GetImageSize(rc.right, rc.bottom);
		RectPic2Scr(rc);
		CRgn rgn;
		rgn.CreateRectRgnIndirect(rc);
		dc.SelectClipRgn(&rgn);
		DrawSquare(Square, dc);
		dc.SelectClipRgn(NULL);
		rgn.DeleteObject();
	}
	else
	{
		UINT hitType;
		m_Squares.HitTest(pt, hitType);
		UpdateCursor(hitType);
	}
	CDIBView::OnMouseMove(nFlags, point);
}

void CDibVSq::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_pSDI)
	{
		ReleaseCapture();
		if (m_pSDI->SquareIsNew())
		{
			CColorDialog cd(m_pSDI->GetSquare()->GetColor());
			if (cd.DoModal() == IDOK)
			{
				m_pSDI->GetSquare()->SetColor(cd.GetColor());
				Invalidate(FALSE);
			}
		}
		delete m_pSDI;
		m_pSDI = NULL;
	}
	CDIBView::OnLButtonUp(nFlags, point);
}

void CDibVSq::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (nFlags & MK_CONTROL)
	{
		Scr2Pic(point);
		m_Squares.RemoveSquareAt(point);
		Invalidate(FALSE);
		UpdateCursor(HTNOWHERE);
	}
}

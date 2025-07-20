#if !defined(AFX_DIBVSQ_H__DDA68D59_9AD8_4478_9952_5C462C702765__INCLUDED_)
#define AFX_DIBVSQ_H__DDA68D59_9AD8_4478_9952_5C462C702765__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DibVSq.h : header file
//

#include <video\dibview.h>
#include "AfxTempl.h"
#include <shbase\shbase.h>

#define DEFAULT_SQUARE_WIDTH	100
#define DEFAULT_SQUARE_HEIGHT	100
#define SQUARE_MARGIN_WIDTH		5

/////////////////////////////////////////////////////////////////////////////
// CDibVSq window

class CDibVSq : public CDIBView
{
public:
	class CSquare
	{
		CRect m_Rect;
		COLORREF m_Color;
	public:
		CSquare(CRect& rc, COLORREF color) { m_Rect = rc; m_Color = color; };
		UINT HitTest(CPoint& pt)
		{
			CRect rc = m_Rect;
			if (!rc.PtInRect(pt))
				return HTNOWHERE;
			rc.DeflateRect(SQUARE_MARGIN_WIDTH, SQUARE_MARGIN_WIDTH);
			if (rc.PtInRect(pt))
				return HTCLIENT;
			rc = CRect(m_Rect.TopLeft(), CSize(SQUARE_MARGIN_WIDTH, SQUARE_MARGIN_WIDTH));
			if (rc.PtInRect(pt))
				return HTTOPLEFT;
			rc.OffsetRect(m_Rect.Width() - SQUARE_MARGIN_WIDTH, 0);
			if (rc.PtInRect(pt))
				return HTTOPRIGHT;
			rc.OffsetRect(0, m_Rect.Height() - SQUARE_MARGIN_WIDTH);
			if (rc.PtInRect(pt))
				return HTBOTTOMRIGHT;
			rc.OffsetRect(-m_Rect.Width() + SQUARE_MARGIN_WIDTH, 0);
			if (rc.PtInRect(pt))
				return HTBOTTOMLEFT;
			rc = CRect(m_Rect.TopLeft(), CSize(m_Rect.Width(), SQUARE_MARGIN_WIDTH));
			if (rc.PtInRect(pt))
				return HTTOP;
			rc.OffsetRect(0, m_Rect.Height() - SQUARE_MARGIN_WIDTH);
			if (rc.PtInRect(pt))
				return HTBOTTOM;
			rc = CRect(m_Rect.TopLeft(), CSize(SQUARE_MARGIN_WIDTH, m_Rect.Height()));
			if (rc.PtInRect(pt))
				return HTLEFT;
			rc.OffsetRect(m_Rect.Width() - SQUARE_MARGIN_WIDTH, 0);
			if (rc.PtInRect(pt))
				return HTRIGHT;
			ASSERT(FALSE);
			return 0;
		}
		void Transform(CPoint offset, UINT hitType)
		{
			switch (hitType)
			{
			case HTCLIENT:
				m_Rect.OffsetRect(offset);
				break;
			case HTTOPLEFT:
				m_Rect.TopLeft() += offset;
				break;
			case HTTOPRIGHT:
				m_Rect.TopLeft().y += offset.y;
				m_Rect.BottomRight().x += offset.x;
				break;
			case HTBOTTOMRIGHT:
				m_Rect.BottomRight() += offset;
				break;
			case HTBOTTOMLEFT:
				m_Rect.TopLeft().x += offset.x;
				m_Rect.BottomRight().y += offset.y;
				break;
			case HTTOP:
				m_Rect.TopLeft().y += offset.y;
				break;
			case HTBOTTOM:
				m_Rect.BottomRight().y += offset.y;
				break;
			case HTLEFT:
				m_Rect.TopLeft().x += offset.x;
				break;
			case HTRIGHT:
				m_Rect.BottomRight().x += offset.x;
				break;
			}
		}
		CRect GetRect() const { return m_Rect; };
		COLORREF GetColor() const { return m_Color; };
		void SetColor(COLORREF color) { m_Color = color; };
		void SetRect(CRect& rc) { m_Rect = rc; };
	};

	class CSquares : public CArray<CSquare*, CSquare*>
	{
	public:
		CSquares() { SetSize(0, 1); };
		virtual ~CSquares() { RemoveAll(); };

		void RemoveAt(int index, int count = 1)
		{
			while (count--)
			{
				CSquare* Square = GetAt(index);
				if (Square) delete Square;
				CArray<CSquare*, CSquare*>::RemoveAt(index);
			}
		}
		void RemoveAll()
		{
			RemoveAt(0, (int)GetSize());
		}
		BOOL Load(const char* fileName)
		{
			RemoveAll();
			CStdioFile file;
			if (!file.Open(fileName, CFile::modeRead))
				return FALSE;
			CString line;
			while (file.ReadString(line))
			{
				CSquare* Square = CreateSquare(line);
				if (Square)
					Add(Square);
			}
			file.Close();
			return TRUE;
		}
		BOOL Save(const char* fileName)
		{
			CStdioFile file;
			if (!file.Open(fileName, CFile::modeWrite | CFile::modeCreate))
				return FALSE;
			for (int i = 0; i < GetSize(); i++)
			{
				CSquare* Square = GetAt(i);
				if (Square)
				{
					CString line = DescribeSquare(Square);
					file.WriteString(line);
				}
			}
			file.Close();
			return TRUE;
		}
		CSquare* HitTest(CPoint& pt, UINT& hitType)
		{
			for (int i = (int)GetSize() - 1; i >= 0; i--)
			{
				CSquare* Square = GetAt(i);
				if (Square)
				{
					hitType = Square->HitTest(pt);
					if (hitType != HTNOWHERE)
						return Square;
				}
			}
			return NULL;
		}
		void RemoveSquareAt(CPoint& pt)
		{
			for (int i = (int) GetSize() - 1; i >= 0; i--)
			{
				CSquare* Square = GetAt(i);
				if (!Square || (Square->HitTest(pt) == HTNOWHERE))
					continue;
				RemoveAt(i);
				return;
			}
		}
	private:
		CSquare* CreateSquare(CString& line)
		{
			line.TrimLeft();
			CString item = line.SpanExcluding("(");
			if (item.CompareNoCase("rect"))
				return NULL;
			line = line.Mid(item.GetLength() + 1);
			line.TrimLeft();
			item = line.SpanExcluding(",;");
			if (item.SpanIncluding("0123456789") != item)
				return NULL;
			int x = atoi(item);
			line = line.Mid(item.GetLength() + 1);			
			line.TrimLeft();
			item = line.SpanExcluding(",;");
			if (item.SpanIncluding("0123456789") != item)
				return NULL;
			int y = atoi(item);
			line = line.Mid(item.GetLength() + 1);			
			line.TrimLeft();
			item = line.SpanExcluding(",;");
			if (item.SpanIncluding("0123456789") != item)
				return NULL;
			int w = atoi(item);
			line = line.Mid(item.GetLength() + 1);			
			line.TrimLeft();
			item = line.SpanExcluding(")");
			if (item.SpanIncluding("0123456789") != item)
				return NULL;
			int h = atoi(item);
			line = line.Mid(item.GetLength() + 1);			
			line.TrimLeft();
			item = line.SpanExcluding("(");
			if (item.CompareNoCase("color"))
				return NULL;
			line = line.Mid(item.GetLength() + 1);			
			line.TrimLeft();
			item = line.SpanExcluding(",;");
			if (item.SpanIncluding("0123456789") != item)
				return NULL;
			int r = atoi(item);
			line = line.Mid(item.GetLength() + 1);			
			line.TrimLeft();
			item = line.SpanExcluding(",;");
			if (item.SpanIncluding("0123456789") != item)
				return NULL;
			int g = atoi(item);
			line = line.Mid(item.GetLength() + 1);			
			line.TrimLeft();
			item = line.SpanExcluding(")");
			if (item.SpanIncluding("0123456789") != item)
				return NULL;
			int b = atoi(item);
			return new CSquare(CRect(x, y, w + x, h + y), RGB(r, g, b));
		}
		CString DescribeSquare(CSquare* Square)
		{
			CString str;
			CRect rc = Square->GetRect();
			COLORREF color = Square->GetColor();
			str.Format("RECT(%d, %d, %d, %d) COLOR(%d, %d, %d)\n", rc.left, rc.top, rc.Width(), rc.Height(),
				GetRValue(color), GetGValue(color), GetBValue(color));
			return str;
		}
	};

	class CSquareDragInfo
	{
		CSquare*	m_pSquare;
		UINT		m_HitType;
		CPoint		m_ptDragEnter;
		BOOL		m_bNewSquare;
	public:
		CSquareDragInfo(CSquare* Square, UINT hit, CPoint& pt, BOOL bNewSquare = FALSE) : m_pSquare(Square), m_HitType(hit), m_ptDragEnter(pt), m_bNewSquare(bNewSquare) {};
		CPoint GetShift(CPoint& pt) { CPoint shift = pt - m_ptDragEnter; m_ptDragEnter = pt; return shift; };
		CSquare* GetSquare() const { return m_pSquare; };
		UINT GetHitType() const { return m_HitType; };
		BOOL SquareIsNew() { return m_bNewSquare; };
	};
protected:
	CSquares	m_Squares;
	BOOL		m_bDrawSquares;
	CSquareDragInfo* m_pSDI;
	CFont		m_Font;
	CGDIBank	m_GDI;
	UINT		m_CurHitType;

	void GetSquareLabelXRect(CRect& rc, CSquare* Square, CDC& dc);
	void GetSquareLabelYRect(CRect& rc, CSquare* Square, CDC& dc);
	void GetSquareLabelWRect(CRect& rc, CSquare* Square, CDC& dc);
	void GetSquareLabelHRect(CRect& rc, CSquare* Square, CDC& dc);
	void InvalidateSquare(CSquare* Square, CDC& dc, BOOL bErase = TRUE);
	void DrawSquare(CSquare* Square, CDC& dc);
	void UpdateCursor(UINT hitType);

// Construction
public:
	CDibVSq();

// Attributes
public:
	void AddSquare(LPRECT rc, COLORREF color) { CSquare* Square = new CSquare(CRect(rc), color); AddSquare(Square); };
	virtual void AddSquare(CSquare* Square) { m_Squares.Add(Square); };
	void SetDrawSquares(BOOL bDraw) { m_bDrawSquares = bDraw; };
	BOOL IsDrawingSquares() { return m_bDrawSquares; };
	CSquare* GetSquare(int i) { return m_Squares.GetAt(i); };

// Operations
public:
	void RectScr2Pic(CRect& rc) { Scr2Pic(rc.TopLeft()); Scr2Pic(rc.BottomRight()); };
	void RectPic2Scr(CRect& rc)
	{
		Pic2Scr(rc.TopLeft());
		Pic2Scr(rc.BottomRight());
	};
	BOOL LoadSquares(const char* fileName) { return m_Squares.Load(fileName); };
	BOOL SaveSquares(const char* fileName) { return m_Squares.Save(fileName); };

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDibVSq)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDibVSq();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDibVSq)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIBVSQ_H__DDA68D59_9AD8_4478_9952_5C462C702765__INCLUDED_)

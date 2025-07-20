// PointsMap.h: interface for the CPointsMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POINTSMAP_H__1016C6A7_782B_4BF1_AFFD_FD058F0D33B3__INCLUDED_)
#define AFX_POINTSMAP_H__1016C6A7_782B_4BF1_AFFD_FD058F0D33B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPointsMap  
{
	double m_Scale;
	CPoint m_Offset;
public:
	CPointsMap() { m_Scale = 1; m_Offset = CPoint(0, 0); };
	virtual ~CPointsMap() {};

	double GetScale() const { return m_Scale; };
	void SetScale(double scale) { m_Scale = scale; };
	CPoint& GetOffset() { return m_Offset; };
	void SetOffset(CPoint offset) { m_Offset = offset; };
  void MoveMap(int dx, int dy)  { m_Offset.x+=dx; m_Offset.y+=dy; };
  void ResetOffsetX()           { m_Offset.x=0; };
  void ResetOffsetY()           { m_Offset.y=0; };
	void AbsToView(LPPOINT pt);
	void AbsToViewScale(LPPOINT pt);
	void ViewToAbs(LPPOINT pt);
	void AbsToView(LPRECT rc);
	void AbsToViewScale(LPRECT rc);
	void ViewToAbs(LPRECT rc);
	CRgn* AbsToView(CRgn* pRgn);
	CRgn* ViewToAbs(CRgn* pRgn);
};

__forceinline void CPointsMap::AbsToView(LPPOINT pt)
{
//	pt->x = (int)((double)(pt->x - m_Offset.x) / m_Scale);
//	pt->y = (int)((double)(pt->y - m_Offset.y) / m_Scale);
	pt->x = (int)((double)pt->x / m_Scale) + m_Offset.x;
	pt->y = (int)((double)pt->y / m_Scale) + m_Offset.y;
}


__forceinline void CPointsMap::AbsToViewScale(LPPOINT pt)
{
//	pt->x = (int)((double)(pt->x - m_Offset.x) / m_Scale);
//	pt->y = (int)((double)(pt->y - m_Offset.y) / m_Scale);
	pt->x = (int)((double)pt->x / m_Scale);
	pt->y = (int)((double)pt->y / m_Scale);
}

__forceinline void CPointsMap::ViewToAbs(LPPOINT pt)
{
	pt->x = (int)((double)(pt->x - m_Offset.x) * m_Scale);
	pt->y = (int)((double)(pt->y - m_Offset.y) * m_Scale);
}

__forceinline void CPointsMap::AbsToView(LPRECT rc)
{
	CPoint pt1(rc->left, rc->top);
	AbsToView(&pt1);
	CPoint pt2(rc->right, rc->bottom);
	AbsToView(&pt2);
	rc->left = pt1.x;
	rc->top = pt1.y;
	rc->right = pt2.x;
	rc->bottom = pt2.y;
}

__forceinline void CPointsMap::AbsToViewScale(LPRECT rc)
{
	CPoint pt1(rc->left, rc->top);
	AbsToViewScale(&pt1);
	CPoint pt2(rc->right, rc->bottom);
	AbsToViewScale(&pt2);
	rc->left = pt1.x;
	rc->top = pt1.y;
	rc->right = pt2.x;
	rc->bottom = pt2.y;
}

__forceinline void CPointsMap::ViewToAbs(LPRECT rc)
{
	CPoint pt1(rc->left, rc->top);
	ViewToAbs(&pt1);
	CPoint pt2(rc->right, rc->bottom);
	ViewToAbs(&pt2);
	rc->left = pt1.x;
	rc->top = pt1.y;
	rc->right = pt2.x;
	rc->bottom = pt2.y;
}

__forceinline CRgn* CPointsMap::AbsToView(CRgn* pRgn)
{
	int cbData = pRgn->GetRegionData(NULL, 0);
	RGNDATA* lpData = (RGNDATA*)malloc(cbData);
	pRgn->GetRegionData(lpData, cbData);
	FLOAT scale = (FLOAT)(1. / m_Scale);
	XFORM xf = { scale, 0, 0, scale, 0, 0 };
	CRgn* Rgn = new CRgn;
	Rgn->CreateFromData(&xf, cbData, lpData);
	free(lpData);
	Rgn->OffsetRgn(m_Offset.x, m_Offset.y);
	return Rgn;
}

__forceinline CRgn* CPointsMap::ViewToAbs(CRgn* pRgn)
{
	pRgn->OffsetRgn(-m_Offset.x, -m_Offset.y);
	int cbData = pRgn->GetRegionData(NULL, 0);
	RGNDATA* lpData = (RGNDATA*)malloc(cbData);
	pRgn->GetRegionData(lpData, cbData);
	FLOAT scale = (FLOAT)m_Scale;
	XFORM xf = { scale, 0, 0, scale, 0, 0 };
	CRgn* Rgn = new CRgn;
	Rgn->CreateFromData(&xf, cbData, lpData);
	free(lpData);
	return Rgn;
}

#endif // !defined(AFX_POINTSMAP_H__1016C6A7_782B_4BF1_AFFD_FD058F0D33B3__INCLUDED_)

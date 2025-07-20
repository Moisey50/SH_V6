// Mapper.cpp: implementation of the CMapper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mapper.h"
#include <math\intf_sup.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define SIMPLE_MODE
#define SIMPLIFY_COEFFS { ax = cx = ay = cy = 0; }

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMapper::CMapper():
m_axisX(CMatrix(6, 1)),
m_axisY(CMatrix(6, 1)),
m_bValid(false)
{
}

CMapper::~CMapper()
{
}

#ifdef SIMPLE_MODE
bool CMapper::BuildMap(CNodes* Nodes)
{
	Reset();
	double X = 0, Y = 0, x = 0, y = 0;
	double Xx = 0, Xy = 0, Yx = 0, Yy = 0, x2 = 0, y2 = 0, xy = 0;
	double Xx2 = 0, Xxy = 0, Xy2 = 0, Yx2 = 0, Yxy = 0, Yy2 = 0, x3 = 0, x2y = 0, xy2 = 0, y3 = 0;
	double x4 = 0, x3y = 0, x2y2 = 0, xy3 = 0, y4 = 0;

	for (int i = 0; i < Nodes->GetCount(); i++)
	{
		NODE node = Nodes->GetAt(i);
		X += node.m_refX;
		Y += node.m_refY;
		x += node.m_x;
		y += node.m_y;
		Xx += node.m_refX * node.m_x;
		Xy += node.m_refX * node.m_y;
		Yx += node.m_refY * node.m_x;
		Yy += node.m_refY * node.m_y;
		x2 += node.m_x * node.m_x;
		y2 += node.m_y * node.m_y;
		xy += node.m_x * node.m_y;
		Xx2 += node.m_refX * node.m_x * node.m_x;
		Xxy += node.m_refX * node.m_x * node.m_y;
		Xy2 += node.m_refX * node.m_y * node.m_y;
		Yx2 += node.m_refY * node.m_x * node.m_x;
		Yxy += node.m_refY * node.m_x * node.m_y;
		Yy2 += node.m_refY * node.m_y * node.m_y;
		x3 += node.m_x * node.m_x * node.m_x;
		x2y += node.m_x * node.m_x * node.m_y;
		xy2 += node.m_x * node.m_y * node.m_y;
		y3 += node.m_y * node.m_y * node.m_y;
		x4 += node.m_x * node.m_x * node.m_x * node.m_x;
		x3y += node.m_x * node.m_x * node.m_x * node.m_y;
		x2y2 += node.m_x * node.m_x * node.m_y * node.m_y;
		xy3 += node.m_x * node.m_y * node.m_y * node.m_y;
		y4 += node.m_y * node.m_y * node.m_y * node.m_y;
	}
	double n = (double)Nodes->GetCount();
	double mx[] =
	{
		x2y2, x2y, xy2, xy,
		x2y, x2, xy, x,
		xy2, xy, y2, y,
		xy, x, y, n
	};
	double cX[] = { Xxy, Xx, Xy, X };
	double cY[] = { Yxy, Yx, Yy, Y };
	CMatrix M(4, 4, mx);
	if (determinant(M) == 0)
		return false;
	CMatrix I = Inverse(M);
	CMatrix Cx(4, 1, cX);
	CMatrix Cy(4, 1, cY);

	CMatrix Ax = I * Cx;
	CMatrix Ay = I * Cy;
	m_axisX[0][0] = m_axisX[2][0] = 0;
	m_axisX[1][0] = Ax[0][0];
	m_axisX[3][0] = Ax[1][0];
	m_axisX[4][0] = Ax[2][0];
	m_axisX[5][0] = Ax[3][0];
	m_axisY[0][0] = m_axisY[2][0] = 0;
	m_axisY[1][0] = Ay[0][0];
	m_axisY[3][0] = Ay[1][0];
	m_axisY[4][0] = Ay[2][0];
	m_axisY[5][0] = Ay[3][0];
	m_bValid = true;
	return true;
}
#else
bool CMapper::BuildMap(CNodes* Nodes)
{
	Reset();
	double X = 0, Y = 0, x = 0, y = 0;
	double Xx = 0, Xy = 0, Yx = 0, Yy = 0, x2 = 0, y2 = 0, xy = 0;
	double Xx2 = 0, Xxy = 0, Xy2 = 0, Yx2 = 0, Yxy = 0, Yy2 = 0, x3 = 0, x2y = 0, xy2 = 0, y3 = 0;
	double x4 = 0, x3y = 0, x2y2 = 0, xy3 = 0, y4 = 0;

	for (int i = 0; i < Nodes->GetCount(); i++)
	{
		NODE node = Nodes->GetAt(i);
		X += node.m_refX;
		Y += node.m_refY;
		x += node.m_x;
		y += node.m_y;
		Xx += node.m_refX * node.m_x;
		Xy += node.m_refX * node.m_y;
		Yx += node.m_refY * node.m_x;
		Yy += node.m_refY * node.m_y;
		x2 += node.m_x * node.m_x;
		y2 += node.m_y * node.m_y;
		xy += node.m_x * node.m_y;
		Xx2 += node.m_refX * node.m_x * node.m_x;
		Xxy += node.m_refX * node.m_x * node.m_y;
		Xy2 += node.m_refX * node.m_y * node.m_y;
		Yx2 += node.m_refY * node.m_x * node.m_x;
		Yxy += node.m_refY * node.m_x * node.m_y;
		Yy2 += node.m_refY * node.m_y * node.m_y;
		x3 += node.m_x * node.m_x * node.m_x;
		x2y += node.m_x * node.m_x * node.m_y;
		xy2 += node.m_x * node.m_y * node.m_y;
		y3 += node.m_y * node.m_y * node.m_y;
		x4 += node.m_x * node.m_x * node.m_x * node.m_x;
		x3y += node.m_x * node.m_x * node.m_x * node.m_y;
		x2y2 += node.m_x * node.m_x * node.m_y * node.m_y;
		xy3 += node.m_x * node.m_y * node.m_y * node.m_y;
		y4 += node.m_y * node.m_y * node.m_y * node.m_y;
	}
	double n = (double)Nodes->GetCount();
	double mx[] =
	{
		x4, x3y, x2y2, x3, x2y, x2,
		x3y, x2y2, xy3, x2y, xy2, xy,
		x2y2, xy3, y4, xy2, y3, y2,
		x3, x2y, xy2, x2, xy, x,
		x2y, xy2, y3, xy, y2, y,
		x2, xy, y2, x, y, n
	};
	double cX[] = { Xx2, Xxy, Xy2, Xx, Xy, X };
	double cY[] = { Yx2, Yxy, Yy2, Yx, Yy, Y };
	CMatrix M(6, 6, mx);
	if (determinanterminant(M) == 0)
		return false;
	CMatrix I = Inverse(M);
	CMatrix Cx(6, 1, cX);
	CMatrix Cy(6, 1, cY);

	m_axisX = I * Cx;
	m_axisY = I * Cy;
	m_bValid = true;
	return true;
}
#endif

bool CMapper::Map(double& x, double& y)
{
	if (!m_bValid)
		return false;
	double ax = m_axisX[0][0];
	double bx = m_axisX[1][0];
	double cx = m_axisX[2][0];
	double dx = m_axisX[3][0];
	double ex = m_axisX[4][0];
	double fx = m_axisX[5][0];
	double ay = m_axisY[0][0];
	double by = m_axisY[1][0];
	double cy = m_axisY[2][0];
	double dy = m_axisY[3][0];
	double ey = m_axisY[4][0];
	double fy = m_axisY[5][0];

#ifdef SIMPLE_MODE
	SIMPLIFY_COEFFS;
#endif

	double X = ax * x * x + bx * x * y + cx * y * y + dx * x + ex * y + fx;
	double Y = ay * x * x + by * x * y + cy * y * y + dy * x + ey * y + fy;
	x = X;
	y = Y;
	return true;
}

bool CMapper::Project(double& x, double& y)
{
	if (!m_bValid)
		return false;
	double ax = m_axisX[0][0];
	double bx = m_axisX[1][0];
	double cx = m_axisX[2][0];
	double dx = m_axisX[3][0];
	double ex = m_axisX[4][0];
	double fx = m_axisX[5][0];
	double ay = m_axisY[0][0];
	double by = m_axisY[1][0];
	double cy = m_axisY[2][0];
	double dy = m_axisY[3][0];
	double ey = m_axisY[4][0];
	double fy = m_axisY[5][0];

#ifdef SIMPLE_MODE
	SIMPLIFY_COEFFS;
#endif

	double m[] = { dx, ex, dy, ey };
	CMatrix M(2, 2, m);
	CMatrix R = Inverse(M);
	if (!R.Size())
		return false;
	double c[] = { x - fx, y - fy };
	CMatrix C(2, 1, c);

	CMatrix S = R * C;
	CMatrix S1 = S;
	const double prec = 0.0001;
	int attempts = 10000;
	do
	{
		S = S1;
		x = S1[0][0];
		y = S1[1][0];
		double dx = ax * x * x + bx * x * y + cx * y * y;
		double dy = ay * x * x + by * x * y + cy * y * y;
		C[0][0] -= dx;
		C[1][0] -= dy;
		S1 = R * C;
		attempts--;
	}while (attempts && fabs(S1[0][0] - S[0][0]) < prec && fabs(S1[1][0] - S[1][0]) < prec);

	x = S1[0][0];
	y = S1[1][0];
	return true;
}

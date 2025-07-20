#ifndef _LINE_OPERATIONS_INCLUDED
#define _LINE_OPERATIONS_INCLUDED

#include <math\Matrix.h>
/*
inline bool Line_EstimateParameters(CLine* Line)
{
	double sumX = 0, sumX2 = 0, sumX3 = 0, sumX4 = 0, sumX5 = 0, sumX6 = 0, sumX8 = 0, sumY = 0, sumYX = 0, sumYX2 = 0, sumYX4 = 0;
	for (int i = 0; i < Line->GetSize(); i++)
	{
		POINT pt = Line->GetAt(i);
		double x = (double)pt.x;
		double y = (double)pt.y;
		sumX += x;
		sumX2 += (x * x);
		sumX3 += (x * x * x);
		sumX4 += (x * x * x * x);
		sumX5 += (x * x * x * x * x);
		sumX6 += (x * x * x * x * x * x);
		sumX8 += (x * x * x * x * x * x * x * x);
		sumY += y;
		sumYX += (y * x);
		sumYX2 += (y * x * x);
		sumYX4 += (y * x * x * x * x);
	}
	double mx[] =
	{
		sumX8, sumX6, sumX5, sumX4,
		sumX6, sumX4, sumX3, sumX2,
		sumX5, sumX3, sumX2, sumX,
		sumX4, sumX2, sumX, (double)Line->GetSize()
	};
	CMatrix M(4, 4, mx);
	double v[] = { sumYX4, sumYX2, sumYX, sumY };
	CMatrix C(4, 1, v);
	if (det(M) == 0)
	{
		LPLINEAPPROXPARAMS lap = Line->GetParams();
		lap->valid = false;
		return false;
	}
	CMatrix I = Inverse(M);
	CMatrix P = I * C;

	LPLINEAPPROXPARAMS lap = Line->GetParams();
	lap->valid = true;
	lap->k = P[0][0];
	lap->a = P[1][0];
	lap->b = P[2][0];
	lap->c = P[3][0];
	lap->argMin = (double)Line->GetAt(0).x;
	lap->argMax = (double)Line->GetAt(Line->GetSize() - 1).x;
	return true;
}*/

inline void Line_GetAvgPoint(CLine* Line, int index, int range, double& x, double& y)
{
	x = y = 0;
	for (int i = index - range / 2; i < index + range / 2; i++)
	{
		POINT pt = Line->GetAt(i);
		x += (double)pt.x;
		y += (double)pt.y;
	}
	x /= (double)range;
	y /= (double)range;
}

inline bool Line_EstimateParameters(CLine* Line)
{
	const int range = 10;
	if (Line->GetSize() < 2 * range)
	{
		LPLINEAPPROXPARAMS lap = Line->GetParams();
		lap->valid = false;
		return false;
	}
	double x1, y1;
	Line_GetAvgPoint(Line, range / 2, range, x1, y1);
	double x2, y2;
	Line_GetAvgPoint(Line, (int)Line->GetSize() / 4, range, x2, y2);
	double x3, y3;
	Line_GetAvgPoint(Line, (int) Line->GetSize() / 2, range, x3, y3);
	double x4, y4;
	Line_GetAvgPoint(Line, (int) Line->GetSize() / 4 * 3, range, x4, y4);
	double x5, y5;
	Line_GetAvgPoint(Line, (int) Line->GetSize() - range / 2, range, x5, y5);
	double mx[] =
	{
		x1 * x1, x1, 1,
		x3 * x3, x3, 1,
		x5 * x5, x5, 1
	};
	CMatrix M(3, 3, mx);
	double v[] = { y1, y3, y5 };
	CMatrix V(3, 1, v);
	CMatrix I = Inverse(M);
	CMatrix P1 = I * V;
	M[0][0] = x2 * x2;
	M[0][1] = x2;
	V[0][0] = y2;
	I = Inverse(M);
	CMatrix P2 = I * V;
	M[2][0] = x4 * x4;
	M[2][1] = x4;
	V[2][0] = y4;
	I = Inverse(M);
	CMatrix P3 = I * V;
	LPLINEAPPROXPARAMS lap = Line->GetParams();
	lap->valid = true;
	lap->k = 0;
	lap->a = (P1[0][0] + P2[0][0] + P3[0][0]) / 3.;
	lap->b = (P1[1][0] + P2[1][0] + P3[1][0]) / 3.;
	lap->c = (P1[2][0] + P2[2][0] + P3[2][0]) / 3.;
	lap->c += .5;
	lap->argMin = (double)Line->GetAt(0).x;
	lap->argMax = (double)Line->GetAt(Line->GetSize() - 1).x;
	double x = (lap->argMin + lap->argMax) / 2;
	lap->meanLevel = lap->a * x * x + lap->b * x + lap->c;
	return true;
}

inline bool Line_PreciseCross(CLine* VertLine, CLine* HorzLine, double& x, double& y)
{
	LPLINEAPPROXPARAMS Vlap = VertLine->GetParams();
	LPLINEAPPROXPARAMS Hlap = HorzLine->GetParams();
	double x1, y1, x2, y2, eps;
	do
	{
		x1 = x;
		y1 = Hlap->a * x1 * x1 + Hlap->b * x1 + Hlap->c;
		y2 = y1;
		x2 = Vlap->a * y2 * y2 + Vlap->b * y2 + Vlap->c;
		eps = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
		x = x2;
		y = y2;
	}
	while (eps > .01 && eps < 400.);
	return (eps < .01);
}

inline bool Line_FindCross(CLine* VertLine, CLine* HorzLine, double& x, double& y)
{
	LPLINEAPPROXPARAMS Vlap = VertLine->GetParams();
	LPLINEAPPROXPARAMS Hlap = HorzLine->GetParams();
	if (!Hlap->valid || !Vlap->valid)
		return false;
	double mx[] = { Hlap->b, -1, -1, Vlap->b };
	CMatrix M(2, 2, mx);
	double c[] = { -Hlap->c, -Vlap->c };
	CMatrix C(2, 1, c);
	CMatrix I = Inverse(M);
	CMatrix V = I * C;
	x = V[0][0];
	y = V[1][0];
	if (!Line_PreciseCross(VertLine, HorzLine, x, y))
		return false;
	const double delta = 0;
	return (x >= Hlap->argMin  - delta && x <= Hlap->argMax + delta && y >= Vlap->argMin - delta && y <= Vlap->argMax + delta);
}

inline int Line_GetIndex(CLine* Line)
{
	LPLINEAPPROXPARAMS lap = Line->GetParams();
	return lap->index;
}

#endif
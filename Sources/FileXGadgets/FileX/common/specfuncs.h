#ifndef SETKA_SPECIAL_IMAGEPROC_FUNCTIONS_INCLUDED
#define SETKA_SPECIAL_IMAGEPROC_FUNCTIONS_INCLUDED

#include "Distortion.h"

inline void _rectify(pTVFrame Frame)
{
	int width = Frame->lpBMIH->biWidth;
	int height = Frame->lpBMIH->biHeight;
	LPBYTE Data = GetData(Frame);
	LPBYTE Copy = (LPBYTE)malloc(width * height);
	memcpy(Copy, Data, width * height);
	LPBYTE Src = Data + width + 1;
	LPBYTE End = Data + width * (height - 1);
	LPBYTE Dst = Copy + width + 1;
	while (Src < End)
	{
		int vertNbrs = (*(Src - width) == 0) ? 1 : 0;
		if (*(Src + width) == 0)
			vertNbrs++;
		int horzNbrs = (*(Src - 1) == 0) ? 1 : 0;
		if (*(Src + 1) == 0)
			horzNbrs++;
		int diagNbrs = (*(Src - width - 1) == 0) ? 1 : 0;
		if (*(Src - width + 1) == 0)
			diagNbrs++;
		if (*(Src + width - 1) == 0)
			diagNbrs++;
		if (*(Src + width + 1) == 0)
			diagNbrs++;
		if (vertNbrs == 2 || horzNbrs == 2)
			*Dst = 0;
		else if (!vertNbrs && !horzNbrs)
			*Dst = 255;
		Src++;
		Dst++;
	}
	memcpy(GetData(Frame), Copy, width * height);
	free(Copy);
}

inline void _v_erode(pTVFrame Frame)
{
	int width = Frame->lpBMIH->biWidth;
	int height = Frame->lpBMIH->biHeight;
	LPBYTE Data = GetData(Frame);
	LPBYTE Copy = (LPBYTE)malloc(width * height);
	memcpy(Copy, Data, width * height);
	LPBYTE Src = Data + width;
	LPBYTE End = Data + width * (height - 1);
	LPBYTE Dst = Copy + width;
	while (Src < End)
	{
		if (*(Src - width) != 0 && *(Src - width - 1) != 0 && *(Src - width + 1) != 0 &&
			*(Src + width) != 0 && *(Src + width - 1) != 0 && *(Src + width + 1) != 0)
			*Dst = 255;
		Src++;
		Dst++;
	}
	memcpy(GetData(Frame), Copy, width * height);
	free(Copy);
}

inline void _h_erode(pTVFrame Frame)
{
	int width = Frame->lpBMIH->biWidth;
	int height = Frame->lpBMIH->biHeight;
	LPBYTE Data = GetData(Frame);
	LPBYTE Copy = (LPBYTE)malloc(width * height);
	memcpy(Copy, Data, width * height);
	LPBYTE Src = Data + 1;
	LPBYTE End = Data + width * height - 1;
	LPBYTE Dst = Copy + 1;
	while (Src < End)
	{
		if (*(Src - 1) != 0 && *(Src + 1) != 0)
			*Dst = 255;
		Src++;
		Dst++;
	}
	memcpy(GetData(Frame), Copy, width * height);
	free(Copy);
}

inline pTVFrame _logical_or(pTVFrame frame1, pTVFrame frame2)
{
	ASSERT(!memcmp(frame1->lpBMIH, frame2->lpBMIH, sizeof(BITMAPINFOHEADER)));
	pTVFrame Frame = makecopyTVFrame(frame1);
	LPBYTE Src = GetData(frame2);
	LPBYTE End = Src + frame2->lpBMIH->biWidth * frame2->lpBMIH->biHeight;
	LPBYTE Dst = GetData(Frame);
	while (Src < End)
	{
		if (*Src)
			*Dst = 255;
		Src++;
		Dst++;
	}
	return Frame;
}

inline void _mark_hmin(pTVFrame DstFrame, pTVFrame SrcFrame, POINT& focus, int delta = 5)
{
	LPBYTE Src = GetData(SrcFrame);
	LPBYTE Dst = GetData(DstFrame);
	int width = SrcFrame->lpBMIH->biWidth;
	int xs = focus.x - delta;
	if (xs < 0)
		xs = 0;
	int xe = focus.x + delta + 1;
	if (xe > width)
		xe = width;
	int xmin1 = xs, xmin2 = xs;
	while (++xs < xe)
	{
		if (Src[focus.y * width + xs] < Src[focus.y * width + xmin1])
			xmin1 = xs;
		if (Src[focus.y * width + xs] <= Src[focus.y * width + xmin2])
			xmin2 = xs;
	}
	focus.x = (xmin1 + xmin2) / 2;
	Dst[focus.y * width + focus.x] = 0;
}

inline void _mark_vmin(pTVFrame DstFrame, pTVFrame SrcFrame, POINT& focus, int delta = 5)
{
	LPBYTE Src = GetData(SrcFrame);
	LPBYTE Dst = GetData(DstFrame);
	int width = SrcFrame->lpBMIH->biWidth;
	int ys = focus.y - delta;
	if (ys < 0)
		ys = 0;
	int ye = focus.y + delta + 1;
	if (ye > SrcFrame->lpBMIH->biHeight)
		ye = SrcFrame->lpBMIH->biHeight;
	int ymin1 = ys, ymin2 = ys;
	while (++ys < ye)
	{
		if (Src[ys * width + focus.x] < Src[ymin1 * width + focus.x])
			ymin1 = ys;
		if (Src[ys * width + focus.x] <= Src[ymin2 * width + focus.x])
			ymin2 = ys;
	}
	focus.y = (ymin1 + ymin2) / 2;
	Dst[focus.y * width + focus.x] = 0;
}

inline void _index_lines(CLines* Lines)
{
	const double prec = 15;
    int i;
	for (i = 1; i < Lines->GetCount(); i++)
	{
		CLine* Line = Lines->GetAt(i);
		LPLINEAPPROXPARAMS lap = Line->GetParams();
		for (int j = 0; j < i; j++)
		{
			CLine* OrderedLine = Lines->GetAt(j);
			LPLINEAPPROXPARAMS lapOrdered = OrderedLine->GetParams();
			if (lap->meanLevel < lapOrdered->meanLevel)
			{
				Lines->MoveLine(i, j);
				break;
			}
		}
	}
	int index = 0;
	for (i = 0; i < Lines->GetCount(); i++)
	{
		CLine* Line = Lines->GetAt(i);
		LPLINEAPPROXPARAMS lap = Line->GetParams();
		if (!i)
		{
			lap->index = index;
			continue;
		}
		CLine* IndexedLine = Lines->GetAt(i - 1);
		LPLINEAPPROXPARAMS lapIndexed = IndexedLine->GetParams();
		if (lap->meanLevel - lapIndexed->meanLevel > prec)
			index++;
		lap->index = index;
	}
}

inline void _diagnose_nodes(CMapper* Mapper, CNodes* Nodes)
{
	for (int i = 0; i < Nodes->GetCount(); i++)
	{
		NODE node = Nodes->GetAt(i);
		double x1 = node.m_x;
		double y1 = node.m_y;
		Mapper->Map(x1, y1);
		double X1 = node.m_refX;
		double Y1 = node.m_refY;
		for (int j = i + 1; j < Nodes->GetCount(); j++)
		{
			node = Nodes->GetAt(j);
			double x2 = node.m_x;
			double y2 = node.m_y;
			Mapper->Map(x2, y2);
			double X2 = node.m_refX;
			double Y2 = node.m_refY;
			double dist = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
			double distModel = sqrt((X2 - X1) * (X2 - X1) + (Y2 - Y1) * (Y2 - Y1));
			TRACE("Nodes (%2d,%2d): distance %.2f (%.2f)\n", i, j, dist, distModel);
		}
	}
}

inline void _calculate_distortion(CMapper* Mapper, CDistortion* Dist, pTVFrame Frame)
{
	double x0 = (double)Frame->lpBMIH->biWidth / 2;
	double y0 = (double)Frame->lpBMIH->biHeight / 2;
	double X0 = x0;
	double Y0 = y0;
	Mapper->Map(X0, Y0);
	double r = 3 * (double)Frame->lpBMIH->biWidth / 8;
	double x = x0 + r;
	double y = y0;
	double X = x;
	double Y = y;
	Mapper->Map(X, Y);
	double R2 = (X - X0) * (X - X0) + (Y - Y0) * (Y - Y0);
	double Rmin = R2, Rmax = R2;
	Dist->AddNode(x, y, NULL, NULL);
	int iMin = 0, iMax = 0;
	for (double a = 1; a < 360; a++)
	{
		double cosA = cos(a / 180 * acos(-1.));
		double sinA = sin(a / 180 * acos(-1.));
		x = r * cosA + x0;
		y = r * sinA + y0;
		X = x;
		Y = y;
		Mapper->Map(X, Y);
		double R = (X - X0) * (X - X0) + (Y - Y0) * (Y - Y0);
		if (Rmin > R)
		{
			iMin = (int)a;
			Rmin = R;
		}
		if (Rmax < R)
		{
			iMax = (int)a;
			Rmax = R;
		}
		while (R < R2)
		{
			x += cosA;
			y += sinA;
			X = x;
			Y = y;
			Mapper->Map(X, Y);
			R = (X - X0) * (X - X0) + (Y - Y0) * (Y - Y0);
		}
		while (R > R2)
		{
			x -= cosA;
			y -= sinA;
			X = x;
			Y = y;
			Mapper->Map(X, Y);
			R = (X - X0) * (X - X0) + (Y - Y0) * (Y - Y0);
		}
		Dist->AddNode(x, y, NULL, NULL);
	}
/*	double rMin, rMax;
	for (int i = 0; i < Dist->GetCount() / 2; i++)
	{
		NODE node = Dist->GetAt(i);
		double r = (node.m_x - x0) * (node.m_x - x0) + (node.m_y - y0) * (node.m_y - y0);
		if (!i)
			rMin = rMax = r;
		if (rMin > r)
		{
			rMin = r;
			iMin = i;
		}
		if (rMax < r)
		{
			rMax = r;
			iMax = i;
		}
	}*/
	Dist->SetPoles(iMin, (iMin + Dist->GetCount() / 2) % 360, iMax, (iMax + Dist->GetCount() / 2) % 360);
}

#endif
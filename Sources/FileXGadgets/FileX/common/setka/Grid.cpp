// Grid.cpp: implementation of the CGrid class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Grid.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const char defaultGridFilename[] = _T("Calibration.txt");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGrid::CGrid():
m_CX(0),
m_CY(0)
{
	LoadMetric();
}

CGrid::~CGrid()
{
	m_Proc.Reset();
	m_Data.Reset();
}

BOOL CGrid::LoadMetric(LPCTSTR fileName)
{
	CMapper* Mapper = m_Data.GetMapper();
	CMatrix* XAxis = Mapper->GetXPolinomial();
	CMatrix* YAxis = Mapper->GetYPolinomial();
	double ax, bx, cx, dx, ex, fx, ay, by, cy, dy, ey, fy;
	FILE* file = (fileName) ? fopen(fileName, "r") : fopen(defaultGridFilename, "r");
	if (!file) return FALSE;
	char buf[80];
    fseek(file,0,SEEK_END);
    int size=ftell(file);
    if (size==0) return false; // fsize == 0, damaged file;;;
//    fseek(file,0,SEEK_SET);
//    char *data=(char*)malloc(size);
//    fread(data,1,size,file);
    fseek(file,0,SEEK_SET);
    int i=fscanf(file,"%s%s%lf%lf %lf %lf %lf %lf\r\n%s%s %lf %lf %lf %lf %lf %lf\r\n%s%s %lf\r\n%s%s %lf",
		buf,buf, &ax, &bx, &cx, &dx, &ex, &fx, buf, buf, &ay, &by, &cy, &dy, &ey, &fy, buf,buf, &m_CX, buf,buf, &m_CY);
	fclose(file);
//    free(data);
	(*XAxis)[0][0] = ax;
	(*XAxis)[1][0] = bx;
	(*XAxis)[2][0] = cx;
	(*XAxis)[3][0] = dx;
	(*XAxis)[4][0] = ex;
	(*XAxis)[5][0] = fx;
	(*YAxis)[0][0] = ay;
	(*YAxis)[1][0] = by;
	(*YAxis)[2][0] = cy;
	(*YAxis)[3][0] = dy;
	(*YAxis)[4][0] = ey;
	(*YAxis)[5][0] = fy;
	Mapper->SetValid();
	return TRUE;
}

BOOL CGrid::SaveMetric(LPCTSTR fileName)
{
	CMapper* Mapper = m_Data.GetMapper();
	CMatrix* XAxis = Mapper->GetXPolinomial();
	CMatrix* YAxis = Mapper->GetYPolinomial();
	double ax, bx, cx, dx, ex, fx, ay, by, cy, dy, ey, fy;
	ax = (*XAxis)[0][0];
	bx = (*XAxis)[1][0];
	cx = (*XAxis)[2][0];
	dx = (*XAxis)[3][0];
	ex = (*XAxis)[4][0];
	fx = (*XAxis)[5][0];
	ay = (*YAxis)[0][0];
	by = (*YAxis)[1][0];
	cy = (*YAxis)[2][0];
	dy = (*YAxis)[3][0];
	ey = (*YAxis)[4][0];
	fy = (*YAxis)[5][0];

	FILE* file = (fileName) ? fopen(fileName, "w") : fopen(defaultGridFilename, "w");
	if (!file)
		return FALSE;
	fprintf(file, "X polynomial %lf %lf %lf %lf %lf %lf\r\nY polynomial %lf %lf %lf %lf %lf %lf\r\nX center %lf\r\nY center %lf",
		ax, bx, cx, dx, ex, fx, ay, by, cy, dy, ey, fy, m_CX, m_CY);
	fclose(file);
	return TRUE;
}

BOOL CGrid::VerboseMetric(FXString& str)
{
	CMapper* Mapper = m_Data.GetMapper();
	if (!Mapper)
		return FALSE;
	CMatrix* XAxis = Mapper->GetXPolinomial();
	CMatrix* YAxis = Mapper->GetYPolinomial();
	if (!XAxis || !YAxis)
		return FALSE;
	double ax, bx, cx, dx, ex, fx, ay, by, cy, dy, ey, fy;
	ax = (*XAxis)[0][0];
	bx = (*XAxis)[1][0];
	cx = (*XAxis)[2][0];
	dx = (*XAxis)[3][0];
	ex = (*XAxis)[4][0];
	fx = (*XAxis)[5][0];
	ay = (*YAxis)[0][0];
	by = (*YAxis)[1][0];
	cy = (*YAxis)[2][0];
	dy = (*YAxis)[3][0];
	ey = (*YAxis)[4][0];
	fy = (*YAxis)[5][0];

	str.Format("X polynomial %lf %lf %lf %lf %lf %lf\r\nY polynomial %lf %lf %lf %lf %lf %lf\r\nX center %lf\r\nY center %lf",
		ax, bx, cx, dx, ex, fx, ay, by, cy, dy, ey, fy, m_CX, m_CY);
	return TRUE;
}

BOOL CGrid::VerboseMetricPK(FXString& str)
{
	CMapper* Mapper = m_Data.GetMapper();
	if (!Mapper)
		return FALSE;
	CMatrix* XAxis = Mapper->GetXPolinomial();
	CMatrix* YAxis = Mapper->GetYPolinomial();
	if (!XAxis || !YAxis)
		return FALSE;
	double ax, bx, cx, dx, ex, fx, ay, by, cy, dy, ey, fy;
	ax = (*XAxis)[0][0];
	bx = (*XAxis)[1][0];
	cx = (*XAxis)[2][0];
	dx = (*XAxis)[3][0];
	ex = (*XAxis)[4][0];
	fx = (*XAxis)[5][0];
	ay = (*YAxis)[0][0];
	by = (*YAxis)[1][0];
	cy = (*YAxis)[2][0];
	dy = (*YAxis)[3][0];
	ey = (*YAxis)[4][0];
	fy = (*YAxis)[5][0];

    FXPropertyKit pk;
    pk.WriteDouble("x0", ax);
    pk.WriteDouble("x1", bx);
    pk.WriteDouble("x2", cx);
    pk.WriteDouble("x3", dx);
    pk.WriteDouble("x4", ex);
    pk.WriteDouble("x5", fx);
    pk.WriteDouble("y0", ay);
    pk.WriteDouble("y1", by);
    pk.WriteDouble("y2", cy);
    pk.WriteDouble("y3", dy);
    pk.WriteDouble("y4", ey);
    pk.WriteDouble("y5", fy);
    pk.WriteDouble("xC", m_CX);
    pk.WriteDouble("yC", m_CY);
	str = pk;
	return TRUE;
}

BOOL CGrid::CalcMetric(LPCTSTR fileName)
{
	m_Data.Reset();
	m_Proc.Reset();
	m_CX = m_CY = 0;
	if (!m_Proc.Next(&m_Data, (void*)fileName))
		return FALSE;
	while (!m_Proc.IsLastStep())
		if (!m_Proc.Next(&m_Data, NULL))
			return FALSE;
	POINT pt;
	if (!m_Data.GetImgCenter(pt))
		return FALSE;
	return MapPoint((double)pt.x, (double)pt.y, &m_CX, &m_CY);
}

BOOL CGrid::CalcMetric(const pTVFrame Frame)
{
	m_Data.Reset();
	m_Proc.Reset();
	m_CX = m_CY = 0;
	if (!m_Data.LoadFrame(Frame))
		return FALSE;
	while (!m_Proc.IsLastStep())
		if (!m_Proc.Next(&m_Data, NULL))
			return FALSE;
	POINT pt;
	if (!m_Data.GetImgCenter(pt))
		return FALSE;
	return MapPoint((double)pt.x, (double)pt.y, &m_CX, &m_CY);
}

BOOL CGrid::MapPoint(double xScr, double yScr, double* x, double* y)
{
	CMapper* Mapper = m_Data.GetMapper();
	if (!Mapper)
		return FALSE;
	if (!Mapper->Map(xScr, yScr))
		return FALSE;
	*x = (xScr - m_CX);
	*y = (yScr - m_CY);
	return TRUE;
}

BOOL CGrid::ProjectPoint(double x, double y, double* xScr, double* yScr)
{
	CMapper* Mapper = m_Data.GetMapper();
	if (!Mapper)
		return FALSE;
	*xScr = (x + m_CX);
	*yScr = (y + m_CY);
	if (!Mapper->Project(*xScr, *yScr))
		return FALSE;
	return TRUE;
}

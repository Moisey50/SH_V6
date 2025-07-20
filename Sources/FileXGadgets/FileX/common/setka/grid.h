// Grid.h: interface for the CGrid class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRID_H__0BBDEA4B_9098_4A1E_AF41_BFE24B01CF4C__INCLUDED_)
#define AFX_GRID_H__0BBDEA4B_9098_4A1E_AF41_BFE24B01CF4C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Data.h"
#include "Processing.h"
#include <fxfc\fxfc.h>

class CGrid
{
private:
	CData		m_Data;
	CProcessing	m_Proc;
	double		m_CX, m_CY;	// image center coordinates
public:
	CGrid();
	virtual ~CGrid();

	BOOL	LoadMetric(LPCTSTR fileName = NULL);
	BOOL	SaveMetric(LPCTSTR fileName = NULL);
	BOOL	CalcMetric(LPCTSTR fileName);
	BOOL	CalcMetric(const pTVFrame Frame);
	BOOL	VerboseMetric(FXString& str);
    BOOL    VerboseMetricPK(FXString& str);
	BOOL	MapPoint(double xScr, double ySrc, double* x, double* y);		// screen coordinates to real coordinates
	BOOL	ProjectPoint(double x, double y, double* xScr, double* yScr);	// real coordinates to screen coordinates
};

#endif // !defined(AFX_GRID_H__0BBDEA4B_9098_4A1E_AF41_BFE24B01CF4C__INCLUDED_)

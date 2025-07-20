// Mapper.h: interface for the CMapper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAPPER_H__CAE18121_535B_4DFB_8357_6BCEA0C96FC2__INCLUDED_)
#define AFX_MAPPER_H__CAE18121_535B_4DFB_8357_6BCEA0C96FC2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <math\Matrix.h>
#include "Node.h"

class CMapper  
{
public:
	CMapper();
	virtual ~CMapper();

	bool	BuildMap(CNodes* Nodes);
	bool	Map(double& x, double& y);		// screen coordinates to real coordinates
	bool	Project(double& x, double& y);	// real coordinates to screen coordinates
	void	Reset()	{ m_bValid = false; };
	CMatrix*	GetXPolinomial() { return &m_axisX; };
	CMatrix*	GetYPolinomial() { return &m_axisY; };
	void	SetValid() { m_bValid = true; };
	bool	IsValid() { return m_bValid; };

private:
	CMatrix	m_axisX;
	CMatrix	m_axisY;
	bool	m_bValid;
};

#endif // !defined(AFX_MAPPER_H__CAE18121_535B_4DFB_8357_6BCEA0C96FC2__INCLUDED_)

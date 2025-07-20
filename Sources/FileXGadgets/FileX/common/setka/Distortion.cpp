// Circle.cpp: implementation of the CCircle class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Distortion.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDistortion::CDistortion():
m_idPoleN(0),
m_idPoleS(0),
m_idPoleW(0),
m_idPoleE(0)
{
}

CDistortion::~CDistortion()
{
}

void CDistortion::SetPoles(int iN, int iS, int iW, int iE)
{
	m_idPoleN = iN;
	m_idPoleS = iS;
	m_idPoleW = iW;
	m_idPoleE = iE;
}

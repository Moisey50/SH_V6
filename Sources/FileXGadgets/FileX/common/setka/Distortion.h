// Circle.h: interface for the CCircle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CIRCLE_H__8B58569E_439A_4A93_99F7_6BAADB9CD155__INCLUDED_)
#define AFX_CIRCLE_H__8B58569E_439A_4A93_99F7_6BAADB9CD155__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Node.h"

class CDistortion : public CNodes
{
public:
	CDistortion();
	virtual ~CDistortion();

	NODE	GetPoleN() { return GetAt(m_idPoleN); };
	NODE	GetPoleS() { return GetAt(m_idPoleS); };
	NODE	GetPoleW() { return GetAt(m_idPoleW); };
	NODE	GetPoleE() { return GetAt(m_idPoleE); };

	void	SetPoles(int iN, int iS, int iW, int iE);

private:
	int		m_idPoleN, m_idPoleS, m_idPoleW, m_idPoleE;
};

#endif // !defined(AFX_CIRCLE_H__8B58569E_439A_4A93_99F7_6BAADB9CD155__INCLUDED_)

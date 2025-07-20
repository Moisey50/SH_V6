// GridTransform.h: interface for the CGridTransform class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRIDTRANSFORM_H__8551B182_8B45_432E_93AA_8AD878F277E4__INCLUDED_)
#define AFX_GRIDTRANSFORM_H__8551B182_8B45_432E_93AA_8AD878F277E4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\ControledFilter.h>
#include <setka\Grid.h>

class CGridTransform : public CControledFilter  
{
	CGrid m_Grid;
protected:
	CGridTransform();
public:
	virtual ~CGridTransform();
private:
	virtual CDataFrame* DoProcessing(CDataFrame* pDataFrame, CDataFrame* pParamFrame);
	DECLARE_RUNTIME_GADGET(CGridTransform);
};

#endif // !defined(AFX_GRIDTRANSFORM_H__8551B182_8B45_432E_93AA_8AD878F277E4__INCLUDED_)

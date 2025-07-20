// PolyTransform.h: interface for the PolyTransform class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POLYTRANSFORM_H__51036198_A16E_43DC_8718_879B46F7298E__INCLUDED_)
#define AFX_POLYTRANSFORM_H__51036198_A16E_43DC_8718_879B46F7298E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include <Gadgets\ControledFilter.h>
#include "PolySetupDialog.h"

class PolyTransform : public CControledFilter
{
	double m_xPoly[6];
	double m_yPoly[6];
	double m_xC;
	double m_yC;
protected:
	PolyTransform();
public:
	virtual void ShutDown();
	virtual bool PrintProperties(FXString& text);
	virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
    virtual bool ScanSettings(FXString& text);
private:
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame, const CDataFrame* pParamFrame);
	void Transform(LPRECT rc);
	void Transform(double& x, double& y);
	DECLARE_RUNTIME_GADGET(PolyTransform);
};

#endif // !defined(AFX_POLYTRANSFORM_H__51036198_A16E_43DC_8718_879B46F7298E__INCLUDED_)

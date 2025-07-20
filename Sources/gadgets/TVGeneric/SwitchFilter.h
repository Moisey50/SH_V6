// SwitchFilter.h: interface for the SwitchFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SWITCHFILTER_H__270FE9E1_B1C5_4EF6_A1D5_195471B375D3__INCLUDED_)
#define AFX_SWITCHFILTER_H__270FE9E1_B1C5_4EF6_A1D5_195471B375D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include <gadgets\ControledFilter.h>

class Switch : public CControledFilter
{
	volatile BOOL   m_bOn;
protected:
	Switch();
private:
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame, const CDataFrame* pParamFrame);
    bool ScanSettings(FXString& txt);
    bool ScanProperties(LPCTSTR txt, bool& Invalidate);
    bool PrintProperties(FXString& txt);
	DECLARE_RUNTIME_GADGET(Switch);
};

#endif // !defined(AFX_SWITCHFILTER_H__270FE9E1_B1C5_4EF6_A1D5_195471B375D3__INCLUDED_)

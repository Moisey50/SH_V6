// AggregatorGadget.h: interface for the Aggregator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AGGREGATORGADGET_H__7D73EAF2_9174_4465_8AF6_9B66ADC331F0__INCLUDED_)
#define AFX_AGGREGATORGADGET_H__7D73EAF2_9174_4465_8AF6_9B66ADC331F0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets/gadbase.h>
#include <gadgets\ContainerFrame.h>

class Aggregator : public CCollectorGadget
{
	FXPropertyKit m_Config;
protected:
	Aggregator();
public:
	virtual void ShutDown();
	bool PrintProperties(FXString& text);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool ScanSettings(FXString& text);
protected:
	virtual CDataFrame* DoProcessing(CDataFrame const*const* frames, int nmb);
	DECLARE_RUNTIME_GADGET(Aggregator);
};

#endif // !defined(AFX_AGGREGATORGADGET_H__7D73EAF2_9174_4465_8AF6_9B66ADC331F0__INCLUDED_)

// NMEAParser.h: interface for the NMEAParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NMEAPARSER_H__C309F93C_1CBB_4118_B168_26CC920249BD__INCLUDED_)
#define AFX_NMEAPARSER_H__C309F93C_1CBB_4118_B168_26CC920249BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Gadgets\gadbase.h>
#include <Gadgets\TextFrame.h>

class NMEAParser : public CFilterGadget
{
public:
	NMEAParser();
	virtual void ShutDown();
    virtual  CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
    DECLARE_RUNTIME_GADGET(NMEAParser);
};

#endif // !defined(AFX_NMEAPARSER_H__C309F93C_1CBB_4118_B168_26CC920249BD__INCLUDED_)

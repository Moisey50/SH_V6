// UPCReader.h: interface for the UPCReader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UPCREADER_H__405BDCF9_434F_4B40_95F1_AD97A602BD6E__INCLUDED_)
#define AFX_UPCREADER_H__405BDCF9_434F_4B40_95F1_AD97A602BD6E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class UPCReader : public CFilterGadget  
{
public:
	UPCReader();

	virtual void ShutDown();
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	DECLARE_RUNTIME_GADGET(UPCReader);
};

#endif // !defined(AFX_UPCREADER_H__405BDCF9_434F_4B40_95F1_AD97A602BD6E__INCLUDED_)

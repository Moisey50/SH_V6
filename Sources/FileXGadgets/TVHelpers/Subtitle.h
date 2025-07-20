#pragma once

#include <Gadgets\vftempl.h>
#include <gadgets\gadbase.h>

class Subtitle : public CFilterGadget 
{
	FXString m_sFormatName;
	int m_xText;
	int m_yText;
	int m_size;
	FXString m_color;
	double prevTime;
	FXString m_AppendString;
	CInputConnector *m_pInput2;
	FXLockObject m_Lock;

	FXString GenerateName(const CDataFrame*);
public:
	virtual void ShutDown();
	void onText(CDataFrame *lpData);
	Subtitle();
	virtual int GetInputsCount() { return 2; }
    CInputConnector* GetInputConnector(int n) { return ((n<2)&&(n>=0))?((n==0)?m_pInput:m_pInput2):NULL; }
	CDataFrame *DoProcessing(const CDataFrame* pDataFrame);
	virtual bool PrintProperties(FXString& text);
	virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
	virtual bool ScanSettings(FXString& text);

	DECLARE_RUNTIME_GADGET(Subtitle);
};
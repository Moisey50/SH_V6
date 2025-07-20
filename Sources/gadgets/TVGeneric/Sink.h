// Sink.h: interface for the Sink class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SINK_H__AEA34284_EEBF_411D_953A_18B519B57484__INCLUDED_)
#define AFX_SINK_H__AEA34284_EEBF_411D_953A_18B519B57484__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets/gadbase.h>

class Sink : public CGadget  
{
private:
	COutputConnector* m_pOutput;
	CPtrArray		  m_Inputs;
    int               m_InputCnt;
	FXLockObject	  m_Lock, m_OutputLock;
public:
	Sink();
	virtual void ShutDown();
	virtual int GetInputsCount();
	virtual int GetOutputsCount();
	virtual CInputConnector*    GetInputConnector(int n);
	virtual COutputConnector*   GetOutputConnector(int n);
            void Input(CDataFrame* pDataFrame, CConnector* lpInput);
            void RemoveInputs();
            void CreateInputs(int n, basicdatatype type);

    bool PrintProperties(FXString& text);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool ScanSettings(FXString& text);

    static friend void __stdcall Sink_SendData(CDataFrame* pDataFrame, void* lParam, CConnector* lpInput) { ((Sink*)lParam)->Input(pDataFrame, lpInput); };
    DECLARE_RUNTIME_GADGET(Sink);
};

#endif // !defined(AFX_SINK_H__AEA34284_EEBF_411D_953A_18B519B57484__INCLUDED_)
